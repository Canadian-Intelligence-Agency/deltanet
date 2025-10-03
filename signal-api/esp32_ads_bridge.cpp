#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SPI.h> // Include the standard SPI library for communication

// --- 1. NETWORK CONFIGURATION ---
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// CRITICAL: This IP must be the local network IP of the machine running the Go server.
const char* GO_BRIDGE_HOST = "192.168.1.100"; 
const int GO_BRIDGE_PORT = 8080;
const char* GO_BRIDGE_PATH = "/api/v1/signal";

// --- 2. ADS1256 HARDWARE CONFIGURATION ---
// Pin assignments specific to ESP32-S3 and ADS1256 wiring. Adjust as needed.
const int ADS_CS_PIN = 5;    // Chip Select (CS) - Active LOW
const int ADS_DRDY_PIN = 18; // Data Ready (DRDY) - Interrupt Pin
const int ADS_RST_PIN = 19;  // Reset Pin

const long ADS_SAMPLING_RATE = 30000; // Target 30kSPS
const float ADS_VOLTAGE_REF = 2.5; // Vref voltage (e.g., from an external reference chip like LM4040)
const float ADS_GAIN = 1;          // Initial PGA Gain (configurable in register)
const int ADS_RESOLUTION = 24;     // 24-bit ADC

// ADS1256 COMMANDS
#define ADS_CMD_WAKEUP 0x00
#define ADS_CMD_SYNC 0xFC
#define ADS_CMD_STANDBY 0xFD
#define ADS_CMD_RDATA 0x01   // Read Data (Single Read)
#define ADS_CMD_RREG(reg) (0x20 | reg) // Read Register (reg: 0-10)
#define ADS_CMD_WREG(reg) (0x50 | reg) // Write Register (reg: 0-10)

// ADS1256 REGISTERS (Addresses)
#define ADS_REG_STATUS 0x00
#define ADS_REG_MUX 0x01
#define ADS_REG_ADCON 0x02
#define ADS_REG_DRATE 0x03 // Data Rate control
#define ADS_REG_GND 0x07 // Not a real register, but placeholder for the common reference

// Channel Definitions
const char* CHANNEL_IDS[] = {"Fp1", "O1", "T3", "C3", "C4", "Pz", "Cz", "M1"};
const int NUM_CHANNELS = 8;
const int PACKET_POINTS = 50; 

// Raw buffer to hold 50 samples * 8 channels = 400 readings (long is 32-bit/4 bytes)
long raw_data_buffer[NUM_CHANNELS * PACKET_POINTS]; 
int buffer_index = 0;

// --- ADS1256 SPI LOW-LEVEL FUNCTIONS ---

/**
 * @brief Sends a single command byte to the ADS1256.
 */
void adsCommand(uint8_t cmd) {
  digitalWrite(ADS_CS_PIN, LOW);
  delayMicroseconds(2); // Wait time specified in datasheet
  SPI.transfer(cmd);
  digitalWrite(ADS_CS_PIN, HIGH);
}

/**
 * @brief Writes data to an ADS1256 register.
 * @param reg Register address.
 * @param val Value to write.
 */
void writeRegister(uint8_t reg, uint8_t val) {
  adsCommand(ADS_CMD_WREG(reg)); // Write Register command
  SPI.transfer(0x00);            // Send 1 register count (0 means 1 register)
  SPI.transfer(val);             // Send the data byte
}

/**
 * @brief Reads data from an ADS1256 register.
 * @param reg Register address.
 * @return uint8_t The register value.
 */
uint8_t readRegister(uint8_t reg) {
  adsCommand(ADS_CMD_RREG(reg));
  SPI.transfer(0x00); // Send 1 register count
  delayMicroseconds(5); // Wait time for RREG command
  uint8_t val = SPI.transfer(0x00);
  return val;
}

// --- CORE PROGRAM LOGIC (ADS1256 SPECIFIC) ---

/**
 * @brief Initializes the ADS1256 chip registers for 8-channel, 30kSPS operation.
 * * !!! CRITICAL INTEGRATION POINT !!!
 * Ensure SPI settings and register values match your ADS1256 breakout board Vref/Oscillator.
 */
void setupADS() {
  Serial.println("Setting up ADS1256 hardware...");
  
  // Pin setup
  pinMode(ADS_DRDY_PIN, INPUT_PULLUP);
  pinMode(ADS_CS_PIN, OUTPUT);
  pinMode(ADS_RST_PIN, OUTPUT);
  digitalWrite(ADS_CS_PIN, HIGH);

  // Reset sequence
  digitalWrite(ADS_RST_PIN, HIGH);
  delay(1);
  digitalWrite(ADS_RST_PIN, LOW);
  delay(1);
  digitalWrite(ADS_RST_PIN, HIGH);
  delay(20); // Wait for power-up

  // SPI setup (ADS1256 uses CPOL=0, CPHA=1)
  // ESP32 SPI setting for ADS1256 communication
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE1); 
  // Clock speed depends on oscillator. Use a safe speed like 1MHz initially.
  SPI.setClockDivider(SPI_CLOCK_DIV8); // Assuming 80MHz clock, 10MHz SPI speed

  // 1. Reset and Synchronization
  adsCommand(0xFE); // RESET command
  delay(5);
  adsCommand(ADS_CMD_SYNC); // SYNC command
  adsCommand(ADS_CMD_WAKEUP); // WAKEUP command

  // 2. Configure Registers:
  
  // DRATE Register (0x03): Set to 30kSPS (Max rate)
  // The DRATE value for 30kSPS depends on the oscillator frequency (e.g., 7.68MHz or 8MHz)
  // Assuming a value (0xF0) that corresponds to 30kSPS for a 7.68MHz clock
  writeRegister(ADS_REG_DRATE, 0xF0); 
  
  // MUX Register (0x01): Set MUX to AIN0/AINCOM for the first channel (will be changed on the fly in readSingleSample)
  // Since we are reading all 8, we will rely on continuously changing the MUX register.
  writeRegister(ADS_REG_MUX, 0x08); // Set positive input AIN0, negative input AINCOM (0x08)
  
  // ADCON Register (0x02): Set PGA gain (e.g., GAIN=1, CLKOUT off)
  // The ADS_GAIN defined above should match the value set here.
  writeRegister(ADS_REG_ADCON, 0x00); // 0x00 means PGA = 1 (GAIN=1)
  
  // STATUS Register (0x00): Read to ensure communication is okay
  Serial.printf("ADS1256 Status Register: 0x%X\n", readRegister(ADS_REG_STATUS));
  Serial.println("ADS1256 setup complete. Ready to acquire data...");
}

/**
 * @brief Reads a single 24-bit sample for a specified channel from the ADS1256.
 * * This function updates the MUX register for the specific channel being read.
 * @param channelIndex The 0-7 index of the channel to read.
 * @return long The raw 24-bit reading.
 */
long readChannel(int channelIndex) {
    // 1. Set MUX register for the desired channel (AINp = channel, AINn = AINCOM)
    uint8_t mux_setting = (channelIndex << 4) | 0x08; // e.g., AIN0/AINCOM is 0x08
    writeRegister(ADS_REG_MUX, mux_setting);
    
    // 2. Wait for DRDY to go LOW (data ready)
    // CRITICAL: We rely on the 30kSPS rate. The code must be fast enough, or use interrupts.
    // For now, poll the pin:
    while (digitalRead(ADS_DRDY_PIN) == HIGH); 
    
    // 3. Send RDATA command
    adsCommand(ADS_CMD_RDATA);
    delayMicroseconds(5); // Wait time for RDATA command

    // 4. Read the 24-bit result
    long raw_value = 0;
    raw_value |= (long)SPI.transfer(0x00) << 16;
    raw_value |= (long)SPI.transfer(0x00) << 8;
    raw_value |= (long)SPI.transfer(0x00) << 0;
    
    return raw_value;
}


/**
 * @brief Reads one sample from all 8 channels of the ADS1256.
 * @param raw_values Array of size NUM_CHANNELS to store the 24-bit raw values.
 */
void readSingleSample(long* raw_values) {
  // CRITICAL: Ensure the total time for reading 8 channels is less than (1/30000)s = 33µs
  // Reading 8 channels sequentially will significantly reduce the effective sampling rate.
  // For true 30kSPS/channel, you would need multiple ADS1256 chips or a lower global rate.
  // For now, we read sequentially:
  for (int i = 0; i < NUM_CHANNELS; i++) {
    raw_values[i] = readChannel(i);
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

  // Vref (2.5V) is 2,500,000 µV
  // Formula: (Raw Value / (2^23 - 1)) * (Vref / Gain) -> Volts
  float voltage_microvolts = 
    ((float)raw_value / (pow(2, ADS_RESOLUTION - 1) - 1)) * ((ADS_VOLTAGE_REF * 1000000) / ADS_GAIN);
    
  return voltage_microvolts;
}

// --- Data Sending and Wi-Fi Setup (Unchanged) ---

void sendDataPacket() {
  // Implementation for JSON serialization and HTTP POST remains here...
  // ... (omitted for brevity, assume it uses raw_data_buffer and sends data)
  StaticJsonDocument<12000> doc; 

  JsonArray main_array = doc.to<JsonArray>();

  long packet_send_time_ms = millis(); 

  for (int i = 0; i < NUM_CHANNELS; i++) {
    JsonObject channel_obj = main_array.add<JsonObject>();
    channel_obj["channelId"] = CHANNEL_IDS[i];
    JsonArray points_array = channel_obj.createNestedArray("points");

    for (int j = 0; j < PACKET_POINTS; j++) {
      long raw_value = raw_data_buffer[(j * NUM_CHANNELS) + i];
      
      long point_timestamp = packet_send_time_ms - (PACKET_POINTS - 1 - j) * (1000 / ADS_SAMPLING_RATE);
      
      JsonObject point = points_array.add<JsonObject>();
      point["t"] = point_timestamp;
      // Add a visual offset for the web client (since real EEG is bipolar)
      point["v"] = convertRawToMicrovolts(raw_value) + 60.0; 
    }
  }

  HTTPClient http;
  String server_url = String("http://") + GO_BRIDGE_HOST + ":" + GO_BRIDGE_PORT + GO_BRIDGE_PATH;
  
  http.begin(server_url);
  http.addHeader("Content-Type", "application/json");

  String payload;
  serializeJson(doc, payload);

  int httpResponseCode = http.POST(payload);

  if (httpResponseCode > 0) {
    Serial.printf("[HTTP] POST... code: %d\n", httpResponseCode);
  } else {
    Serial.printf("[HTTP] POST failed, error: %s\n", http.errorToString(httpResponseCode).c_str());
  }

  http.end();
}

void setup() {
  Serial.begin(115200);
  delay(100);
  
  // 1. Hardware Initialization (ADS1256 specific)
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

void loop() {
  // --- IMPORTANT NOTE ON SAMPLING RATE ---
  // The ADS1256 samples ALL 8 channels sequentially. If the global DRATE is 30kSPS, 
  // the effective rate *per channel* is 30kSPS / 8 = 3.75kSPS. 
  // We will assume the 30kSPS is the *global* data rate and adjust the timestamping logic.

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

  // Since we are polling, the loop runs as fast as possible.
  // The actual timing is governed by the DRDY pin and the SPI read time.
  // We rely on the DRDY waiting loop inside readChannel to maintain timing fidelity.
}
