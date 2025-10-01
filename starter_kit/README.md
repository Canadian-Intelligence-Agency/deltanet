# Basic Neuro Interfacing Setup Guide 🧠

The goal of this guide is to show the **practical application** of reading brain waves using accessible, **DIY components** from the ground up. You'll learn how to build your own low-cost, single-channel Electroencephalography (EEG) setup capable of detecting basic rhythms like **alpha waves**.

---

## 🛠️ Project Scope & Core Components

This project focuses on the foundational elements of a neuro-interfacing setup:

1.  **Electrode Construction:** Building stable, low-impedance Ag/AgCl wet electrodes.
2.  **Amplification Circuit:** Interfacing the electrodes with a simple op-amp circuit (e.g., TL084CN) to boost the minuscule brain signals.
3.  **Signal Acquisition:** Connecting the amplified signal to an Analog-to-Digital Converter (ADC), like one found on an **ESP32**, for data processing and visualization.

---

## 🔌 Essential First Step: Build Your Electrodes

The quality of your signal depends heavily on your electrodes. This project uses custom-built **wet Ag/AgCl electrodes** which offer a significant stability and noise reduction advantage over simple metal wires or commercial dry electrodes.

### 🔗 Electrode Build Guide

Follow the detailed instructions below to construct the three necessary electrodes (**Signal, Reference, and Ground**):

* **[Base Electrode Build Guide](https://github.com/Canadian-Intelligence-Agency/deltanet/blob/main/starter_kit/base_electrode_build_guide.md)**

This guide covers:
* **Parts List:** Featuring items like 999 Fine Silver Wire, table salt, and gauze.
* **Chloridization:** Two methods (electrolytic preferred) for creating the stable $\text{Ag/AgCl}$ sensing layer.
* **Assembly and Testing:** Step-by-step instructions for housing the electrode and initial impedance testing (aiming for $<10\text{k}\Omega$).

> **⚠️ Safety Note:** Remember to wear gloves and eye protection during the chloridization process, especially when dealing with chemicals or electricity. This setup is for educational/experimental use only—it is **not medical-grade**.

---

## 🏗️ Next Steps (Amplification & Acquisition)

Once you have your three electrodes built and tested, you're ready to integrate them with the rest of the electronics.

### ⚡️ Signal Conditioning

Brain wave signals are often in the microvolt $(\mu\text{V})$ range. You will need a **high-gain differential amplifier** (e.g., using a **TL084CN** op-amp) to:

* **Amplify** the difference between the Signal and Reference electrodes.
* **Filter** out unwanted noise (like mains hum or muscle artifacts).

*Further instructions on the op-amp circuit build will be provided in a subsequent document.*

### 🖥️ Data Interface

The amplified analog signal needs to be converted into a digital format that a computer can read.

* Connect the output of your amplifier to the **ADC pin** on a microcontroller like the **ESP32**.
* Use a programming environment (e.g., Arduino IDE) to read the ADC values and stream the raw data via **Serial** to a host computer.
* You can then use plotting software (like Python with Matplotlib or a dedicated serial plotter) to visualize the brain wave data in real-time.

---

## ✨ Achieving the Goal: Detecting Alpha Rhythms

With a properly built setup and low-noise electrodes, you should be able to clearly detect **Alpha Waves** ($8-12\text{Hz}$) by placing the signal electrode on the forehead (**Fp1**) and the reference/ground on the earlobes (**A1/A2**).

**Try this simple experiment:**
1.  Open your eyes: The alpha rhythm should be less prominent (a process called **alpha desynchronization**).
2.  Close your eyes: The alpha rhythm should become significantly stronger, appearing as distinct, consistent oscillations on your plot.

---
*Last Updated: October 1, 2025*
