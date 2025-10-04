# Architecture: Neural Control of the Q-Resonator

This document outlines the integration plan for connecting the high-fidelity δ.net EEG processing stack with the theoretical **Q-Resonator** quantum computer. The goal is to establish a **Neural Command Layer** where a user's classified cognitive state directly controls the execution of quantum gates on the iCESugar FPGA.

## 1. High-Level Architecture Map

The integration transforms the δ.net stack from a simple signal viewer into a functional **Thought Interface**. The signal travels through a fully instrumented pipeline, from biological measurement to digital quantum actuation:

Electrode→ADS1256→ESP32-S3→Go Server (Classification)→USB Serial Bridge→iCESugar FPGA→AD9850 (Gate Pulse)

## 2. Component Roles in the Integrated System

The new architecture relies on specialized roles for each piece of hardware, leveraging the iCESugar's built-in serial functionality.

| Component                     | Role                                                                                                                                                                                                                            | Data In / Data Out                                                                       |
| ----------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------- |
| **ADS1256 (**24-bit ADC}**)** | **Sensor Frontend.** Acquires 8-channel 30kSPS raw EEG data from electrodes.                                                                                                                                                    | **Out:** Raw 24-bit digital voltage values.                                              |
| **ESP32-S3 WROOM**            | **Hardware Bridge.** Handles low-level SPI communication with the ADS1256 and posts data via Wi-Fi to the Go server.                                                                                                            | **Out:** JSON packets to Go Server (POST to `/api/v1/signal`).                           |
| **Go Server (Debian VPS)**    | **Cognitive Pattern Interpreter.** Receives raw EEG, performs filtering (60Hz notch, bandpass), FFT analysis, and Alpha Suppression classification.                                                                             | **Out:** Classified state (`ATTENTION` or `RELAXED`) available at **`/api/v1/control`**. |
| **iCESugar FPGA** (New)       | **Quantum Control Unit.** Executes the core Verilog logic (e.g., Deutsch-Jozsa) and generates the precise timing sequences for the AD9850. **The iCESugar receives the command to START/STOP via its USB** CDC **Serial Port.** | **In:** Single serial command character (e.g., '1' or '0').                              |
| **AD9850 DDS Module**         | **Actuator.** Receives SPI commands from the iCESugar to generate the exact RF pulse (duration, frequency, phase) required to actuate the quantum gate on the quartz crystal.                                                   | **In:** FPGA SPI clock/data.                                                             |

## 3. The Neural Command Layer Protocol

The goal of this layer is to translate the high-level psychological state (ATTENTION) into the low-level digital trigger (`start_algorithm` wire) on the iCESugar FPGA.

### 3.1 Establishing the USB Serial Bridge (Go → iCESugar)

Since the iCESugar appears as a standard USB CDC **Serial Port** on the host PC (your Debian VPS or a separate host), we must create a **Software Bridge** to send the classified command.

**Bridge Implementation Plan:**

1. **Poll Script:** A small daemon written in Python or Go runs continuously on the Debian VPS.
    
2. **API Check:** This script periodically queries the Go server's **`/api/v1/control`** endpoint to get the current state (`ATTENTION` or `RELAXED`).
    
3. **Serial Write:**
    
    - If the state is ATTENTION, the script opens the iCESugar's serial port (e.g., `/dev/ttyACM0`) and writes the single command character: **`S` (Start)**.
        
    - If the state is RELAXED, the script writes: **`I` (Idle)**.
        

### 3.2 Modifying the iCESugar Verilog Input

The FPGA's Verilog logic must be updated to read from the UART (Serial) port instead of using a physical button or a constant wire for the trigger.

|Verilog Module|Input/Logic Change|
|---|---|
|**New** UART **Receiver Module**|Create a small Verilog module that continuously reads the USB Serial input. When it sees the character **`S`**, it raises a dedicated internal wire called `eeg_start_trigger` for one clock cycle.|
|**`deutsch_jozsa_main`**|Replace the fixed trigger `start_algorithm` with the new wire: `input wire eeg_start_trigger`.|

**Result:** The quantum algorithm is now **triggered not by a programmer's clock cycle, but by the user achieving a sustained state of active** Alpha **suppression.**

## 4. Next Implementation Focus

The next critical steps are **Phase 4** and **Phase 5**, leading to the first demonstration of human thought directly controlling a theoretical quantum computation.

1. **Complete Phase 3 Processing:** Finalize the FFT and Alpha Suppression classification logic within the Go Server.
    
2. **Implement Phase 4 Bridge:** Deploy and test the dedicated **Python/Go Serial Bridge** that links the Go Server's API output to the iCESugar's USB CDC port.
