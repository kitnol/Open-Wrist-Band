# Hardware Documentation

## Overview

Here is presented a documentation for hardware, mechanical and electronics parts. Here described hardware manufacturing and assembly process.

## Table of Contents

1. [Specifications](#specifications)
2. [PCB ordering](#pcb-oredering)
3. [Bill of Materials (BOM)](#bill-of-materials-bom)
4. [Assembly](#assembly)
5. [PCB Design](#pcb-design)
6. [Connectivity](#connectivity)
7. [Sensors](#sensors)
8. [Mechanical Design](#mechanical-design)
9. [Testing & Validation](#testing--validation)


## Specifications

| Parameter | Value | Notes |
|-----------|-------|-------|
| Dimensions | 46.3 x 34 x 16.9 mm| |
| Sensors |PPG, EDA, IMU| For hear rate, SpO2, stress measurements and step counting|
| Weight | 42 grams | Weight with the strap|
| Operating Voltage | 3.6-4.2V | From the battery |
| Current Consumption | 1mA | When PPG and EDA measurements are on |
| Wrist circumference | 150-240 mm | |


## Assembly

### PCB Assembly
1. [STEP 1 - DESCRIBE ASSEMBLY PROCESS]
2. [STEP 2]
3. [STEP 3]

### Housing Assembly
[DESCRIBE HOW COMPONENTS ARE ASSEMBLED INTO THE ENCLOSURE]

### Testing After Assembly
[DESCRIBE ASSEMBLY TESTING PROCEDURES]

## PCB Design

- **Layout Files**: [/PCB/Altium/PCB1.PcBDoc](https://github.com/kitnol/Open-Wrist-Band/blob/main/Hardware/PCB/Altium/PCB1.PcbDoc)
- **Design Software**: Altium
- **Board Dimensions**: 30.3x30 mm
- **Number of Layers**: 4

### Schematics
[REFERENCE SCHEMATIC FILES OR ADD SCHEMATIC DESCRIPTION]

## Power Management

### Charging
- **Charging Method**: USB-C
- **Charging Current**: 100mA
- **Charging Time**: 3-7 hours (depending on a battery capacity)

### Battery Management
- **BMS Type**: [IF APPLICABLE, SPECIFY]
- **Low Battery Threshold**: [SPECIFY PERCENTAGE]
- **Power States**: [DESCRIBE DIFFERENT POWER STATES]

## Connectivity

### Data Interfaces
- **Serial Communication**: USB
- **Programming Interface**: USB or J-LINK
- **Programming voltage**: 1.8V

## Sensors

| Sensor | Type | Range | Purpose |
|--------|------|-------|---------|
| [SENSOR NAME] | [TYPE] | [RANGE] | [PURPOSE] |
| [ADD MORE] | | | |

## Mechanical Design

### Enclosure Material
- **Material**: PLA, PETG or any other non allergic materials

### Dimensions & Layout
[DESCRIBE LAYOUT AND DIMENSIONS]

### Durability Features
- **Water Resistance**: [IPX RATING]
- **Shock Resistance**: [SPECIFY LEVEL]
- **Material Durability**: [DESCRIBE]

## Testing & Validation

### Hardware Testing
- [ ] Short circuits test
- [ ] Power test
- [ ] Charging test (TODO)

#### Short circuits test

Pre-Power Short Circuit Test

Once the PCB is fully assembled, you must test it for short circuits before applying power. Set your multimeter to continuity mode and check the continuity between the following points:

- **GND** and the **1.8V** line

- **GND** and the **2.4V** line

- **1.8V** line and the **2.4V** line

If the multimeter does not beep (indicating no continuity), it is safe to power on the PCB. If continuity is detected, a short circuit exists. <u>Do not apply power until you locate and resolve the source of the short.</u>

#### Power testing

Before programming, power the PCB via USB or battery and use a multimeter to measure the 1.8V and 2.4V lines. If the voltages are correct, it is safe to proceed with programming.

**Troubleshooting Guide:**

- **Voltages are too low**: Check the power input to ensure the board is receiving either 5V (USB) or 3.7V–4.2V (battery).

- **1.8V line is too high**: Immediately disconnect power. Verify that resistor R6 is 47 kΩ. Note that if this line exceeded 1.98V, the AS7058 component is likely damaged and must be replaced.

- **2.4V line is too high**: Power off the PCB immediately and verify that resistor R7 is 47 kΩ. Component damage is unlikely on this line, so no replacements should be necessary once the resistor issue is fixed.

## Bill of Materials (BOM)

### Mechanical BOM

| Part number | Description  | Quantity | Unit Price | Source |
|-----------|--------------|-------|------|--------|
| AM4885 | Li-Pol battery 800mAh, 3.7V, 622540 | 1 |€6.55 | [AM4885](https://ampul.eu/en/battery/6303-li-pol-battery-800mah-37v-622540) |
| ACHLR-02V-S | Battery connector | 1 | €0.13 | [ACHLR-02V-S](https://www.digikey.de/en/products/detail/jst-sales-america-inc/achlr-02v-s/5272191) |
| ASACHLSACHL28W152 | Battery wires | 1 | €0.98 | [ASACHLSACHL28W152](https://www.digikey.de/en/products/detail/jst-sales-america-inc/ASACHLSACHL28W152/8107430?s=N4IgTCBcDaICwFYEFoDMYEHZkDkAiIAugL5A) |
| M2 3mm | Screw for lid | 4 | €5.88 | [M2 3mm](https://dk.rs-online.com/web/p/maskinskruer/9141560) |

**Total Cost**: €31.18

### Electronics BOM
| Designator | Description | Footprint | JLCPCB Part | Quantity | Unit price ($) | Total cost ($) |
|-------------|-------------|-----|-----------|---------|---------|---------|
| C1, C3, C4, C5, C6, C8, C10, C16, C21, C22, C32, C33, C34, C35 | 100nF | 0402 | C1525 | 14 | $0.0017 | $0.0238 |
| C2 | 10 µF | 0402 | C15525 | 1 | $0.0178 | $0.0178 |
| C7 | 1 µF | 0402 | C52923 | 1 | $0.0053 | $0.0053 |
| C9, C11, C14, C17, C25 | 1µF 40% 50V | 0603 | C15849 | 3 | $0.0156 | $0.0468 |
| C15, C19, C23, C24, C26 | 10 µF | 0603 | C19702 | 5 | $0.0149 | $0.0745 |
|  | 1 µF | 0603 | C15849 | 2 | $0.0108 | $0.0216 |
| C18 | 2.2 µF | 0603 | C49326366 | 1 | $0.0092 | $0.0184 |
| C20 | 100 pF | 0402 | C1546 | 1 | $0.0014 | $0.0014 |
| C27 | 4.7 µF | 0603 | C19666 | 1 | $0.0138 | $0.0138 |
| C28, C29, C30, C31 | 0.047 µF | 0402 | C82219 | 4 | $0.0024 | $0.0096 |
| C36, C37 | 12 pF | 0402 | C1547 | 2 | $0.0016 | $0.0032 |
| D1, D2 | SFH 7018A | SFH 7018 | C24570310 | 2 | $1.8124 | $3.6248 |
| D3, D4 | Blue LED | 0402 | C22355736 | 1 | $0.0087 | $0.0174 |
| D5, D6, D7, D8 | TVS DIODE | 0402 | C10676 | 4 | $0.0426 | $0.1704 |
| D9, D10, D11, D12 | SFH2705 | DFN175P2 | C6672359 | 4 | $1.7060 | $6.824 |
| J1 | USB-C Molex-2171790001 | SMD | C3197684 | 1 | $0.6895 | $0.6895 |
| J2 | Battery connector BM02B-ACHLKS | FP-BM02B | C594367 | 1 | $0.7975 |$0.7975 |
| J3 | M50-3530542 | Plugin,P=1.27mm| C6293302 | 1 | $0.3278 | $0.3278 |
| L1, L2 | CIGT201610LH2R2MNE | 0806 | C307626 | 2 | $0.1070 | $0.214 |
| R1, R2, R12, R13 | 5.1K | 0402 | C25905 | 4 | $0.0009 | $0.0036 |
| R3, R4, R5, R8, R11, R14, R19, R20, R21, R22, R23, R24, R25 | 10K | 0402 | C844452 | 17 | $0.0013 | $0.0221 |
| R6, R7 | 47K | 0402 | C25792 | 2 | $0.001 | $0.002 |
| R9, R10, R16, R17 | 100K | 0402 | C25741 | 4 | $0.0009 | $0.0036 | 
| R15, R26, R27, R28 | 4.7K | 0402 | C25900 | 4 | $0.001 | $0.004  |
| R18 | 1K | 0402 | C11702 | 1 | $0.001 | $0.001 |
| U1 | LIS2DS12TR | LGA-12(2x2) | C915682 | 1 | $2.0411 | $2.0411 |
| U2 | nRF52833 | QFN40_5X5 | C3007767 | 1 | $5.0552 | $5.0552 |
| U3 | NPM1304 | QFN32 | C49449665 | 1 | $4.3002 | $4.3002 | 
| U4 | RV-3028-C7 | FP-RV-3028-C7 | C3019759 | 1 | $2.2667 | $2.2667 |
| U5 | AS7058 | WLCSP-42(2.8x2.6) | C23782775 | 1 | $7.5763 | $7.5763 |
| U6 | IS25WP01GJ-RMLE-TY (128MB) | 16-SOIC | [DigiKey](https://www.digikey.de/en/products/detail/issi-integrated-silicon-solution-inc/IS25WP01GJ-RMLE-TY/29286651?s=N4IgTCBcDaIJIGUwFYDqAFADARgOICkBaAJQFkAZAUUIBUBNEAXQF8g) | 1 | $14.38360 | $14.38360 |
| U7 | USBLC6-2SC6 (ESD) | SOT-23-6 | C2827654 | 1 | $0.0410 | $0.0410 |
| Y1 | XYDCELNANF-32M | SMD2520-4P | C124330 | 1 | $0.3137 | $0.3137 |

**Total Cost**: $48.8926

---

**Last Updated**: 06-06-2026