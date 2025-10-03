# Delta Net (δ.net): Phase 1.5 - Signal Interpretation Core

This roadmap outlines the steps required to transition the project from a raw data acquisition setup (ADS 30kSPS via ESP32) to the initial **Neural Encoding** layer required for the δ.net architecture. The focus is on establishing a stable, clean signal and translating raw μV readings into usable **cognitive features**.

## Phase 1: Stabilization and Verification

**GOAL:** Achieve a stable, noise-free, and correctly timestamped 30kSPS data stream from the ADS to the Go Server.

### 1.1 Complete Hardware Bridge Integration

The highest priority is replacing the simulation in the Go server.

- **ESP32 Firmware Finalization:** Complete the low-level **SPI/I2C driver** within the `esp32_ads_bridge.cpp` to accurately configure the 24-bit ADS chip and continuously stream the 8 channels of data.
    
- **Timestamp Verification:** Ensure the ESP32 is generating accurate **timestamps (**t**)** for each data point so the client can reliably calculate the 30kSPS rate.
    
- **Bridge Robustness:** Implement **error handling and retry logic** within the ESP32's HTTP client to maintain consistent connection to the Go server.
    

### 1.2 Implement Real-Time Preprocessing (Go Server)

Raw EEG data is noisy. The Go server (`server.go`) will become the primary data cleaner before transmission to the web client.

- **Bandpass Filter:** Integrate a signal processing library in Go and apply a robust **Digital Bandpass Filter** (e.g., 0.5Hz to 45Hz) to isolate meaningful EEG rhythms from slow drift and high-frequency noise.
    
- **Notch Filter:** Implement a narrow **Notch Filter** at 60Hz (or 50Hz based on location) to eliminate power line interference, which is often the largest source of artifact.
    
- **Artifact Detection:** Implement a simple thresholding mechanism to detect and flag or remove large, sudden spikes corresponding to muscle movements or eye blinks.
    

## Phase 2: Feature Extraction (Neural Encoding)

**GOAL:** Translate clean time-series data into quantitative, semantic features (EEG Power Bands). This fulfills the first requirement of the δ.net **Neural Encoding** layer.

### 2.1 Develop Frequency Domain Analysis

- **Implement Fast Fourier Transform (FFT):** Integrate a high-performance FFT library within the Go server to convert the preprocessed time-series data for all 8 channels into the frequency domain in real time.
    
- **Calculate Power Bands:** Define functions to calculate the total power (or average magnitude) within the following standard EEG frequency bands for each channel:
    
    - **Delta** (0.5−4Hz) - Sleep/Deep Relaxation
        
    - **Theta** (4−8Hz) - Memory/Drowsiness
        
    - **Alpha** (8−12Hz) - Relaxation/Idle
        
    - **Beta** (13−30Hz) - Active Attention/Cognition
        
    - **Gamma** (30−45Hz) - High-level Processing
        

### 2.2 Define the Feature API Endpoint

- **New Go Endpoint:** Create a new API endpoint in `server.go`: `/api/v1/features`. This endpoint will serve the computed power band data (the feature vectors), not the raw voltage trace.
    
- **Update Client:** Modify the `index.html` to fetch data from this new endpoint and display the features (e.g., as a histogram or numerical values) instead of, or alongside, the raw signal.
    

## Phase 3: Thought Interface Pilot Command

**GOAL:** Implement the first binary neural command, demonstrating the δ.net **Thought Interface** principle through context interpretation.

### 3.1 Implement Alpha Suppression Classification

- **Define Baseline:** Establish a baseline Alpha Power value (eyes closed, relaxed state) for the Occipital (O1) channel.
    
- **Thresholding Logic:** Implement a classification function in Go that continuously compares the current O1 Alpha Power to the baseline.
    
    - **"Idle/Relaxed" State:** Alpha Power is high (Power>70% of baseline).
        
    - **"Attention/Active" State:** Alpha Power drops significantly (Alpha Suppression Ratio is low, Power<30% of baseline).
        

### 3.2 Create the Control Output API

- **Command Endpoint:** Create a final Go endpoint: `/api/v1/control`. This endpoint will return the current **classified cognitive state** (e.g., `{"state": "ATTENTION", "confidence": 0.85}`).
    
- **System Feedback:** The client (`index.html`) should fetch this control data and display a visual indicator (e.g., a green light for "Active", a blue light for "Idle"), completing the first functional BCI loop in the Delta Net prototype.
