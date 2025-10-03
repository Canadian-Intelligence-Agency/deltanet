#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// --- 1. NETWORK CONFIGURATION ---
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// CRITICAL: This IP must be the local network IP of the machine running the Go server.
// Do NOT use "localhost" or "127.0.0.1" here, as that refers to the ESP32 itself.
const char* GO_BRIDGE_HOST = "192.168.1.100"; 
const int GO_BRIDGE_PORT = 8080;
const char* GO_BRIDGE_PATH = "/api/v1/signal";

// --- 2. ADS HARDWARE CONFIGURATION ---
// Note: Pin assignments are placeholders. Adjust according to your wiring.
const int ADS_CS_PIN = 5;  // Chip Select
const int ADS_DRDY_PIN = 18; // Data Ready Interrupt Pin
const long ADS_SAMPLING_RATE = 30000; // 30kSPS
const float ADS_VOLTAGE_REF = 2.5; // Vref voltage in Volts (Check your ADS board spec)
const float ADS_GAIN = 1;      // PGA Gain (Check ADS configuration)
const int ADS_RESOLUTION = 24; // 24-bit ADC

// Define the 8 channels corresponding to the order read from the ADS
const char* CHANNEL_IDS[] = {"Fp1", "O1", "T3", "C3", "C4", "Pz", "Cz", "M1"};
const int NUM_CHANNELS = 8;

// --- 3. DATA PACKET & BUFFERING CONFIGURATION ---
// The ESP32 collects a batch of samples before sending a single HTTP request.
// This reduces Wi-Fi overhead and ensures the data packet has enough points for SPS calculation.
const int PACKET_POINTS = 50; // Points per channel per packet (matches Go server simulation)

// Raw buffer to hold 50 samples * 8 channels = 400 readings
long raw_data_buffer[NUM_CHANNELS * PACKET_POINTS]; 
int buffer_index = 0;

// --- Data Structures (Mirroring Go Server JSON) ---
// Note: This data is serialized on-the-fly, not stored in these structs, 
// but they illustrate the required format.

/*
[
  {
    "channelId": "Fp1",
    "points": [ {"t": 1700000000000, "v": 55.2}, ... ]
  }, ...
]
*/


// --- FUNCTION PLACEHOLDERS FOR HARDWARE INTEGRATION ---

/**
 * @brief Initializes the ADS chip (SPI, registers, PGA, rate).
 * * !!! CRITICAL INTEGRATION POINT !!!
 * Replace the contents of this function with the proper SPI/I2C and register
 * configuration calls for your specific 8-channel, 24-bit ADC chip.
 */
void setupADS() {
  Serial.println("Setting up ADS hardware...");
  pinMode(ADS_DRDY_PIN, INPUT_PULLUP);
  pinMode(ADS_CS_PIN, OUTPUT);
  digitalWrite(ADS_CS_PIN, HIGH);
  
  // Example SPI initialization (might need adjustment for specific clock speeds)
  // SPI.begin();
  
  // !!! Insert register write calls here to set gain, channels, and 30kSPS rate !!!
  // Example: writeRegister(CONFIG_REG, 0xXX); 
  
  Serial.println("ADS setup complete. Waiting for data ready...");
}

/**
 * @brief Reads one sample from all 8 channels of the ADS.
 * * !!! CRITICAL INTEGRATION POINT !!!
 * Replace the contents of this function with the logic that waits for the DRDY 
 * signal and reads the 24-bit data stream for all 8 channels in order.
 * * @param raw_values Array of size NUM_CHANNELS to store the 24-bit raw values.
 */
void readSingleSample(long* raw_values) {
  // Wait for DRDY to go LOW (data ready)
  // while (digitalRead(ADS_DRDY_PIN) == HIGH); 
  
  // !!! Insert SPI/I2C read commands to pull 8 channels of 24-bit data !!!
  for (int i = 0; i < NUM_CHANNELS; i++) {
    // raw_values[i] = read24BitValue(); // Custom hardware function
    // --- Simulation for testing until hardware is connected ---
    raw_values[i] = random(0, 0xFFFFFF); // Simulates a full range 24-bit reading
  }
}

/**
 * @brief Converts the raw 24-bit ADS value to voltage in microvolts (µV).
 * @param raw_value The raw 24-bit reading.
 * @return float Voltage in microvolts (µV).
 */
float convertRawToMicrovolts(long raw_value) {
  // Convert 24-bit signed value (Two's complement)
  if (raw_value & 0x800000) { // Check for negative sign
    raw_value |= 0xFF000000; // Extend sign to 32 bits
  }

  // Conversion formula: (Raw Value / (2^N * Gain * Vref)) -> Volts
  // Vref (2.5V) is 2,500,000 µV
  float voltage_microvolts = 
    ((float)raw_value * (ADS_VOLTAGE_REF * 1000000)) / 
    (pow(2, ADS_RESOLUTION - 1) * ADS_GAIN);
    
  // Since the web client displays a positive 0-120 µV scale, 
  // we add an arbitrary offset for visualization.
  return voltage_microvolts + 60.0; // Return with 60µV offset for visualization
}

// --- CORE PROGRAM LOGIC ---

void setup() {
  Serial.begin(115200);
  delay(100);
  
  // 1. Hardware Initialization
  setupADS();

  // 2. Wi-Fi Connection
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected to Wi-Fi.");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());
}

/**
 * @brief Collects a batch of samples, formats the JSON payload, and sends it 
 * to the Go server via HTTP POST.
 */
void sendDataPacket() {
  // Use StaticJsonDocument for stack allocation, optimizing memory usage
  // Calculate required size based on: 
  //   Array[8] of Objects {channelId: string, points: Array[50] of Objects {t:int, v:float}}
  // Estimated size: 10KB (This requires a reasonable stack size on ESP32)
  StaticJsonDocument<12000> doc; 

  // Create the main JSON array
  JsonArray main_array = doc.to<JsonArray>();

  // Use this timestamp for all points in the packet (or improve by using a microsecond timer)
  long packet_send_time_ms = millis(); 

  // 1. Iterate through each channel to build the 8 ChannelData objects
  for (int i = 0; i < NUM_CHANNELS; i++) {
    JsonObject channel_obj = main_array.add<JsonObject>();
    channel_obj["channelId"] = CHANNEL_IDS[i];
    JsonArray points_array = channel_obj.createNestedArray("points");

    // 2. Populate the 'points' array for the current channel (50 points)
    for (int j = 0; j < PACKET_POINTS; j++) {
      // Index in the flattened buffer: (j * NUM_CHANNELS) + i
      long raw_value = raw_data_buffer[(j * NUM_CHANNELS) + i];
      
      // Calculate the approximate timestamp for the client to determine SPS
      // Time is approximated relative to the packet_send_time_ms
      long point_timestamp = packet_send_time_ms - (PACKET_POINTS - 1 - j) * (1000 / ADS_SAMPLING_RATE);
      
      JsonObject point = points_array.add<JsonObject>();
      point["t"] = point_timestamp;
      point["v"] = convertRawToMicrovolts(raw_value); 
    }
  }

  // 3. Serialize JSON and Send HTTP POST
  String payload;
  serializeJson(doc, payload);

  HTTPClient http;
  String server_url = String("http://") + GO_BRIDGE_HOST + ":" + GO_BRIDGE_PORT + GO_BRIDGE_PATH;
  
  http.begin(server_url);
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(payload);

  if (httpResponseCode > 0) {
    Serial.printf("[HTTP] POST... code: %d\n", httpResponseCode);
  } else {
    Serial.printf("[HTTP] POST failed, error: %s\n", http.errorToString(httpResponseCode).c_str());
  }

  http.end();
}

void loop() {
  // 1. Read one full sample (8 channels)
  long current_sample[NUM_CHANNELS];
  readSingleSample(current_sample);

  // 2. Store the 8 channels sequentially in the raw buffer
  for (int i = 0; i < NUM_CHANNELS; i++) {
    raw_data_buffer[buffer_index * NUM_CHANNELS + i] = current_sample[i];
  }
  
  buffer_index++;

  // 3. Check if the buffer is full (50 points collected)
  if (buffer_index >= PACKET_POINTS) {
    Serial.println("Buffer full. Sending packet...");
    sendDataPacket();
    buffer_index = 0; // Reset buffer index for next packet
  }

  // CRITICAL: Maintain the 30kSPS rate by introducing a delay if needed.
  // Since the ADS is very fast, the loop overhead might be too slow.
  // For production, this loop should ideally be triggered by the ADS_DRDY_PIN 
  // via an interrupt to ensure perfect timing, not a delay.
  delay(1); // Small delay to prevent watchdog timer errors during testing.
}
