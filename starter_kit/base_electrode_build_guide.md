# Base Electrode Build Guide

## Introduction

This guide details how to build a simple **wet Ag/AgCl electrode** from scratch for a DIY single-channel EEG setup. This electrode uses pure silver wire chloridized to form a stable Ag/AgCl sensing tip, paired with saline-soaked gauze for low-impedance skin contact (~1-10kΩ). It's designed for basic brain wave detection (e.g., alpha rhythms) when interfaced with an amplification circuit like a TL084CN op-amp and ESP32 ADC.

**Safety Notes**:

- Wear gloves and eye protection during chloridization (involves chemicals/electricity).
- For educational/experimental use only—not medical-grade.
- Test on clean skin; discontinue if irritation occurs.
- Build 3 electrodes: 1 signal, 1 reference, 1 ground.

Estimated time: 45-60 minutes per electrode (plus 1-hour drying). Cost: ~$5-10 total for 3.

## Parts List

|Category|Item|Quantity (per electrode)|Notes/Sourcing|
|---|---|---|---|
|**Core Material**|999 Fine Silver Wire (24 gauge, dead soft, round)|10cm length|Filofly brand recommended; ~$10-15 for 5ft spool on Amazon/Etsy. Provides stable Ag/AgCl coating.|
|**Electrolyte**|3M NaCl solution (3g table salt in 10ml distilled water) or 5% household bleach|50ml|For chloridization; saline from grocery/pharmacy. Bleach as alternative (no power needed).|
|**Contact Medium**|Gauze square (2x2") or cotton ball|1|Absorbs saline for wet contact; pharmacy (~$2/pack).|
|**Housing**|Small plastic tube/cup (e.g., cut syringe barrel or vial cap)|1|Holds wire + gauze; repurpose household items.|
|**Connectors**|Alligator clip or 2mm snap connector|1|Attaches to shielded wire; from electronics kit (~$5).|
|**Sealing**|Hot glue or electrical tape|As needed|Secures assembly; hardware store.|

**For 3 Electrodes**: Scale quantities (e.g., 30cm wire total).

## Tools Required

- Sandpaper or emery cloth (fine grit, 400+).
- Wire cutters/pliers.
- Multimeter (for impedance/potential testing).
- Small beaker or glass dish.
- 9V battery or 3-5V DC power supply (for electrolytic method).
- Distilled water for rinsing.
- Soldering iron (optional, for wire splicing).

## Step-by-Step Build Instructions

### Step 1: Prepare the Silver Wire

1. Cut a 10cm length of Filofly 999 fine silver wire using wire cutters.
2. Sand the first 1cm (sensing tip) lightly with fine sandpaper to remove any oxidation—aim for a smooth, shiny surface.
3. Clean the entire wire: Soak in white vinegar for 5 minutes, then rinse thoroughly with distilled water. Pat dry with a lint-free cloth.
    - _Why?_ Ensures even chloridization without contaminants.

### Step 2: Chloridize the Sensing Tip (Form Ag/AgCl Layer)

Choose one method—electrolytic is preferred for uniformity on pure silver.

#### Option A: Electrolytic Method (Recommended, 5-10 min)

1. Prepare electrolyte: Dissolve 3g table salt in 10ml distilled water (3M NaCl).
2. Setup: Fill a small beaker with solution. Insert the sanded tip (anode, +) and a copper wire scrap (cathode, -).
3. Connect: Wire the silver tip to + on a 3-5V DC supply (or 9V battery via 100Ω resistor to limit current <10mA). Dip both in solution.
4. Anodize: Apply power for 5-10 minutes—bubbles form, and the tip turns gray-black (AgCl coating). Stop when current drops (<1mA).
5. Rinse the tip in distilled water; air-dry for 1 hour.

#### Option B: Bleach Dip Method (Simpler, 10-30 min)

1. Submerge the sanded tip in household bleach (5% NaOCl) for 10-30 minutes until gray-black coating forms.
2. Rinse thoroughly in distilled water; air-dry for 1 hour.
    - _Note_: Monitor for even coverage—pure silver works well here.

### Step 3: Assemble the Electrode

1. Prepare contact medium: Soak gauze square or cotton ball in 0.9% saline (1g salt in 100ml water).
2. Insert wire: Poke the coated tip through the center of the gauze, leaving ~0.5cm exposed.
3. House it: Slide the gauze + wire into the plastic tube/cup. Secure the back end (non-sensing) with hot glue or tape, ensuring the tip protrudes slightly for skin contact.
4. Attach connector: Solder or clip the wire's back end (~7cm from tip) to an alligator clip or snap. Add 20-50cm shielded wire here for noise reduction to your amp.
    - _Tip_: Label wires (e.g., red for signal).

### Step 4: Initial Testing

1. **Impedance Check**: Wet the tip with saline. Use multimeter (ohms mode) between tip and a ground point—should read <10kΩ on damp skin.
2. **Potential Stability**: Dip two electrodes (signal + ref) in saline bath. Measure DC mV difference with multimeter—<5mV stable for 5 minutes.
3. **Visual Inspection**: Coating should be uniform gray-black; re-chloridize if patchy.

## Integration Notes

- **Placement**: Signal on forehead (Fp1), reference on left earlobe (A1), ground on right earlobe (A2). Re-wet with saline before use.
- **Circuit Wiring**: Connect to TL084CN amp inputs (signal to IN+, ref to IN-, ground to GND). No changes to gain/filter.
- **Maintenance**: Store dry in ziplock bag. Re-chloridize every 5-10 uses if coating flakes. Avoid bending the tip sharply.
- **Troubleshooting**:
    - High impedance? Add more saline or clean skin.
    - Drift? Ensure pure silver and even coating.
    - Noise? Use shielded wire full-length.

This base electrode provides low-noise biopotential sensing for your EEG prototype. Scale up for multi-channel if needed. For questions, reference the full EEG build guide.

_Last Updated: October 1, 2025_
