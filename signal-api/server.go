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
// 't' is the timestamp in milliseconds, 'v' is the simulated voltage in microvolts (µV).
type SignalPoint struct {
	Timestamp int64   `json:"t"`
	Voltage   float64 `json:"v"`
}

// Global variable to keep track of simulated time steps across requests
var timeIndex float64 = 0
var initialTimestamp time.Time

func init() {
	// Initialize random seed for noise generation
	rand.Seed(time.Now().UnixNano())
	// Set the initial timestamp for relative time tracking
	initialTimestamp = time.Now()
}

// generateSimulatedEEGData simulates a burst of EEG Alpha wave data (e.g., 200ms at 250 SPS).
// In a real implementation, this function would read directly from the ADS hardware driver.
func generateSimulatedEEGData(numPoints int) []SignalPoint {
	// Simulation Constants
	const alphaFrequency = 10.0 // 10 Hz (Alpha Wave)
	const amplitude = 50.0      // 50 µV peak-to-peak amplitude
	const samplingRate = 250.0  // Samples per second (SPS)
	const timeStep = 1.0 / samplingRate

	data := make([]SignalPoint, numPoints)

	for i := 0; i < numPoints; i++ {
		// Calculate the core sinusoidal voltage (Alpha rhythm)
		voltage := amplitude * math.Sin(2*math.Pi*alphaFrequency*timeIndex)

		// Add a simulated DC offset (like electrode polarization) and random noise
		const dcOffset = 50.0 // Simulated DC bias/offset (50 µV)
		noise := rand.Float64()*10 - 5 // Random noise between -5µV and 5µV

		data[i].Voltage = voltage + dcOffset + noise

		// Calculate the relative timestamp in milliseconds from the server start
		currentTimeMs := time.Since(initialTimestamp).Milliseconds()
		data[i].Timestamp = currentTimeMs + int64(i * 1000 / int(samplingRate))

		timeIndex += timeStep
	}

	return data
}

// signalHandler handles requests to the /api/v1/signal endpoint.
func signalHandler(w http.ResponseWriter, r *http.Request) {
	// Mandatory CORS headers for development/local testing with a separate frontend
	w.Header().Set("Access-Control-Allow-Origin", "*")
	w.Header().Set("Content-Type", "application/json")

	// Generate 50 points of simulated data (0.2 seconds at 250 SPS)
	const pointsToGenerate = 50
	signalData := generateSimulatedEEGData(pointsToGenerate)

	if err := json.NewEncoder(w).Encode(signalData); err != nil {
		http.Error(w, fmt.Sprintf("Failed to encode JSON response: %v", err), http.StatusInternalServerError)
		log.Printf("Error encoding JSON: %v", err)
		return
	}

	log.Printf("Served %d signal points. Latest timestamp: %d ms", pointsToGenerate, signalData[pointsToGenerate-1].Timestamp)
}

// main sets up the router and starts the server.
func main() {
	// Define the API route for raw signal data
	http.HandleFunc("/api/v1/signal", signalHandler)

	// Define a simple health check route
	http.HandleFunc("/health", func(w http.ResponseWriter, r *http.Request) {
		w.WriteHeader(http.StatusOK)
		w.Write([]byte("OK"))
	})

	port := ":8080"
	log.Printf("--- Delta Net Protocol Server (Go) Starting ---")
	log.Printf("Status: Ready to stream simulated neural data.")
	log.Printf("Access API Endpoint: http://localhost%s/api/v1/signal", port)
	log.Printf("-------------------------------------------------")

	if err := http.ListenAndServe(port, nil); err != nil {
		log.Fatalf("Server failed to start on port %s: %v", port, err)
	}
}
