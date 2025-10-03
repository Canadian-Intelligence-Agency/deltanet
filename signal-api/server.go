package main

import (
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"time"
	// NOTE: You would import necessary hardware/serial libraries here, 
	// e.g., for SPI communication with the ADS chip.
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

// =====================================================================
// !!! HARDWARE INTEGRATION REQUIRED HERE !!!
// This function MUST be replaced by your actual ADS reading and conversion logic.
// It should read the latest batch of data (e.g., 50 samples) from the ADS chip
// and return it structured as an array of ChannelData objects.
// =====================================================================
func readADSAndFormatData() []ChannelData {
	
	// WARNING: Currently, this bridge returns an empty array, causing the 
	// Canvas status to show "ERROR - Bridge Offline" because no data is received.
	
	log.Println("WARNING: ADS bridge is running but readADSAndFormatData() returned no data. Please replace this function with hardware reading logic.")

	// Placeholder structure expected by the Canvas (returning empty):
	return []ChannelData{}
}

// signalHandler handles requests to the /api/v1/signal endpoint.
func signalHandler(w http.ResponseWriter, r *http.Request) {
	// Allow requests from any origin (required for localhost testing)
	w.Header().Set("Access-Control-Allow-Origin", "*")
	w.Header().Set("Content-Type", "application/json")

	signalData := readADSAndFormatData()

	if len(signalData) == 0 {
		// If no data is available, respond with HTTP 204 No Content
		w.WriteHeader(http.StatusNoContent)
		return
	}

	if err := json.NewEncoder(w).Encode(signalData); err != nil {
		http.Error(w, fmt.Sprintf("Failed to encode JSON response: %v", err), http.StatusInternalServerError)
		log.Printf("Error encoding JSON: %v", err)
		return
	}
	
	log.Printf("Served %d channel data packets.", len(signalData))
}

// main sets up the router and starts the server.
func main() {
	http.HandleFunc("/api/v1/signal", signalHandler)

	http.HandleFunc("/health", func(w http.ResponseWriter, r *http.Request) {
		w.WriteHeader(http.StatusOK)
		w.Write([]byte("OK"))
	})

	port := ":8080"
	log.Printf("--- Delta Net ADS Bridge (Go) Starting ---")
	log.Printf("Endpoint: http://localhost%s/api/v1/signal", port)
	log.Printf("-------------------------------------------------")

	if err := http.ListenAndServe(port, nil); err != nil {
		log.Fatalf("Server failed to start on port %s: %v", port, err)
	}
}
