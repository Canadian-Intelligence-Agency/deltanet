package main

import (
	"encoding/json"
	"fmt"
	"log"
	"math"
	"math/rand"
	"net/http"
	"time"
)

// SignalPoint represents a single data reading from a neural channel.
// 't' is the timestamp in milliseconds (from the ADS/Microcontroller), 
// 'v' is the voltage in microvolts (µV) calculated from the raw ADS reading.
type SignalPoint struct {
	Timestamp int64   `json:"t"`
	Voltage   float64 `json:"v"`
}

// ChannelData represents the data payload for a single EEG channel.
type ChannelData struct {
	ChannelID string        `json:"channelId"`
	Points    []SignalPoint `json:"points"`
}

// Global variable to simulate time progression for the sine wave
var timeIndex float64 = 0.0

// =====================================================================
// !!! Hardware Bridge Simulation / Integration Point !!!
// This function simulates the multi-channel data stream.
// Replace the contents of this function with your actual ESP32/ADS hardware 
// reading and data conversion logic when you integrate the chip.
// The ESP32 will likely send the data over a WebSocket or simple HTTP POST/GET.
// =====================================================================

func readADSAndFormatData(numPoints int) []ChannelData {
	// Simulation Constants (Updated for the 8-Channel, 30kSPS ADC)
	const samplingRate = 30000.0 // 30,000 Samples per second (SPS) from the new board
	const timeStep = 1.0 / samplingRate
	baseTimestamp := time.Now().UnixMilli()

	// Data structure for the EIGHT configured channels
	data := make([]ChannelData, 8)
	
	// Define the 8 channels using standard 10-20 EEG positions
	channelIDs := []string{"Fp1", "O1", "T3", "C3", "C4", "Pz", "Cz", "M1"}

	for i, id := range channelIDs {
		data[i] = ChannelData{ChannelID: id}
	}


	for i := 0; i < numPoints; i++ {
		currentSimTime := timeIndex
		// Ensure timestamp increases realistically for SPS calculation on client side
		currentTimeMs := baseTimestamp + (int64(i) * 1000 / int64(samplingRate))

		// Simulation Logic for 8 Channels (simplified but varied to show differentiation)
		for idx, id := range channelIDs {
			var v float64
			// Base DC offset + random noise
			noise := rand.Float64()*10 - 5
			baseOffset := 50.0 + float64(idx) * 5.0 // Slightly different DC offset for each channel
			
			switch id {
			case "Fp1": // Alpha + Slow Drift (Frontal - High Amplitude)
				v = 50.0*math.Sin(2*math.Pi*10.0*currentSimTime) + 20.0*math.Sin(2*math.Pi*2.0*currentSimTime)
			case "O1": // Theta/Visual (Occipital - Medium Amplitude)
				v = 30.0*math.Sin(2*math.Pi*4.0*currentSimTime) + 10.0*math.Sin(2*math.Pi*1.0*currentSimTime)
			case "T3": // Gamma Burst (Temporal - Low Amplitude/High Frequency)
				v = 10.0*math.Sin(2*math.Pi*30.0*currentSimTime) + 5.0*math.Sin(2*math.Pi*60.0*currentSimTime)
			case "C3", "C4": // Sensorimotor (Beta activity)
				v = 25.0*math.Sin(2*math.Pi*18.0*currentSimTime) + 5.0*rand.Float64() // 18 Hz Beta
			case "Pz", "Cz": // Central/Parietal (General Alpha/Theta mix)
				v = 40.0*math.Sin(2*math.Pi*8.0*currentSimTime) + 15.0*rand.Float64()
			case "M1": // Mastoid/Reference (Lowest Signal/Highest Noise)
				v = 5.0*math.Sin(2*math.Pi*60.0*currentSimTime) + 10.0*rand.Float64()
			default:
				v = 0
			}
			
			// Final voltage calculation, ensuring V >= 0 for current visualization scale
			finalV := v + baseOffset + noise
			data[idx].Points = append(data[idx].Points, SignalPoint{Timestamp: currentTimeMs, Voltage: math.Max(0, finalV)})
		}

		timeIndex += timeStep
	}
	return data
}

// signalHandler handles requests to the /api/v1/signal endpoint.
func signalHandler(w http.ResponseWriter, r *http.Request) {
	// CORS Headers: MUST be set to allow the client (Canvas/Browser) to fetch data from localhost
	w.Header().Set("Access-Control-Allow-Origin", "*")
	w.Header().Set("Content-Type", "application/json")
	
	// Simulate reading 50 points per fetch interval
	const pointsPerPacket = 50 
	signalData := readADSAndFormatData(pointsPerPacket)

	if len(signalData) == 0 {
		// Respond with 204 No Content if the bridge is up but has no data
		w.WriteHeader(http.StatusNoContent)
		return
	}

	if err := json.NewEncoder(w).Encode(signalData); err != nil {
		http.Error(w, fmt.Sprintf("Failed to encode JSON response: %v", err), http.StatusInternalServerError)
		log.Printf("Error encoding JSON: %v", err)
		return
	}
	
	log.Printf("Served %d channel data packets (%d points each).", len(signalData), pointsPerPacket)
}

// main sets up the router and starts the server.
func main() {
	// Seed the random number generator
	rand.Seed(time.Now().UnixNano()) 

	http.HandleFunc("/api/v1/signal", signalHandler)

	http.HandleFunc("/health", func(w http.ResponseWriter, r *http.Request) {
		w.WriteHeader(http.StatusOK)
		w.Write([]byte("OK"))
	})

	port := ":8080"
	log.Printf("--- Delta Net ADS Bridge (Go HTTP Server) Starting ---")
	log.Printf("Endpoint: http://localhost%s/api/v1/signal", port)
	log.Printf("-------------------------------------------------")

	if err := http.ListenAndServe(port, nil); err != nil {
		log.Fatalf("Server failed to start on port %s: %v", port, err)
	}
}
