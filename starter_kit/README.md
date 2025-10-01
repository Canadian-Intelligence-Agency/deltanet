# Basic Neuro Interfacing Setup Guide

The goal of this guide is to demonstrate the **practical application of reading brain waves** using accessible, DIY components from the ground up. This project serves as a foundational exercise in **biopotential sensing**, providing essential, hands-on experience for those interested in the future of thought-driven connectivity as envisioned by **Delta Net ($\delta$.net)**.

By building this low-cost, single-channel Electroencephalography (EEG) setup, you will master the principles required to detect basic cerebral rhythms, such as **alpha waves**.

---

## Project Context: From DIY EEG to the $\delta$.net Vision

The $\delta$.net whitepaper outlines a **Neural Internet** based on semantic relationships and **Direct Neural Interfaces**. This Basic Neuro Interfacing Setup Guide represents the most fundamental step toward that vision: achieving stable, low-noise acquisition of neural signals (Section 2.2 of the whitepaper, *Thought Interface*, necessitates robust signal acquisition).

This guide focuses on three foundational elements:

1.  **Electrode Construction:** Building stable, low-impedance $\text{Ag/AgCl}$ wet electrodes.
2.  **Amplification Circuit:** Interfacing the electrodes with a simple op-amp circuit (e.g., TL084CN) to amplify $\mu\text{V}$-level brain signals.
3.  **Signal Acquisition:** Converting the amplified signal using an **ESP32 Analog-to-Digital Converter (ADC)** for digital processing.

---

## Essential First Step: Build Your Electrodes

The stability and low impedance of the sensing medium are critical for reducing noise and artifacts. We will construct custom wet $\text{Ag/AgCl}$ electrodes.

### Electrode Build Guide

Follow the detailed instructions below to construct the three necessary electrodes: **Signal, Reference, and Ground**.

* **[Base Electrode Build Guide](https://github.com/Canadian-Intelligence-Agency/deltanet/blob/main/starter_kit/base_electrode_build_guide.md)**

This resource covers the necessary **Parts List**, the **Chloridization** process (which forms the stable $\text{AgCl}$ sensing layer), and initial **Testing** procedures to ensure the final product meets the impedance target ($<10\text{k}\Omega$).

> **Safety Note:** This setup is for educational/experimental use only and is **not medical-grade**. Wear gloves and eye protection during the chemical/electrical chloridization process.

---

## Next Steps: Amplification and Data Acquisition

Once your three electrodes are built and tested, integrate them with the electronics for signal conditioning.

### Signal Conditioning

The differential voltage measured across the scalp is in the microvolt ($\mu\text{V}$) range. A **high-gain differential amplifier** (e.g., based on the **TL084CN** op-amp) is required to:

* **Amplify** the differential signal (Input - Reference).
* **Filter** out environmental noise (like $50\text{Hz}/60\text{Hz}$ mains hum) and biological artifacts (e.g., muscle activity).

### Data Interface

The analog output from the amplifier must be digitized for analysis:

* Connect the amplifier's output to an **ADC pin** on a low-cost microcontroller like the **ESP32**.
* Use a programming environment (e.g., Arduino IDE) to stream the raw, digitized data via **Serial** to a host computer.
* Visualization tools (such as Python with plotting libraries) can then be used to observe the neural patterns in real-time.

---

## Achieving the Goal: Detecting Alpha Rhythms

A successful setup should clearly demonstrate $\text{Alpha Waves}$ ($8-12\text{Hz}$), a common rhythm associated with the relaxed, waking state.

**Placement:** Typically, the Signal electrode is placed on the forehead (e.g., $\text{Fp1}$), the Reference on one earlobe ($\text{A1}$), and the Ground on the other earlobe ($\text{A2}$).

**Demonstration:**
1.  **Eyes Open:** Alpha rhythm activity should be less prominent (a phenomenon known as **alpha desynchronization**).
2.  **Eyes Closed:** The $\text{Alpha}$ rhythm should become significantly stronger, appearing as consistent, high-amplitude oscillations in your data plot.

---
*A project of the Delta Net Research Consortium. Last Updated: October 1, 2025*
