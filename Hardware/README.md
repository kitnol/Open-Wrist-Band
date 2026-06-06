# Hardware Documentation

## Overview

In this section is presented a documentation for hardware, mechanical and electronics parts. Here described hardware manufacturing and assembly process.

## Table of Contents

1. [Specifications](#specifications)
2. [Bill of Materials (BOM)](#bill-of-materials-bom)
3. [Assembly](#assembly)
4. [PCB Design](#pcb-design)
5. [Power Management](#power-management)
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

## Bill of Materials (BOM)

| Designator | Description | Footprint | LCSC Part | Comment |
|-------------|-------------|----------|-----------|---------|
| C1, C3, C4 | CAP CER 1 | 0402 | C527001 | 0.1 µF |
| C2 | CAP CER 1 | 0402 | C5272370 | 10 µF |
| C7 | CAP CER 1 | 0402 | C106254 | 1 µF |
| C9, C11, C | Chip Capa | 0603 | C15849 | Capacitor |
| C15, C19, CAP CER 1 | MURATA 0 | C2356601 | | 10 µF |
| C17, C25 | CAP CER 1 | CAP 0603 | C15849 | 1 µF |
| C18 | CAP CER 2 | CAP 0603 | C49326366 | 2.2 µF |
| C20 | CAP CER 1 | CAP 0402 | C541358 | 100 µF |
| C27 | CAP CER 4 | MURATA G | C342959 | 4.7 µF |
| C26, C29, C | CAP CER 0 | CAP 0402 | C107020 | 0.047 µF |
| C36, C37 | CAP CER 1 | CAP 0402 | C1822032 | 12 pF |
| D1, D2 | LED HI/RRs | SFH 7018D4 | C2585188 | SFH 7018A |
| D3 | LED GREEN | LED 0402_1005 | GREEN | Blue |
| D4 | LED GREEN | LED 0402_1005 | GREEN | Green |
| D5, D6, D7 | TVS DIODE | FP-PGB1-( | C10676 | PGB10100D4 |
| D8, D9, D10 | Photodiode | DFN175P2 | C667359 | SFH2705 |
| J1 | CONN RGH | FP-217179-( | C319764 | 217179-00 J1 |
| J2 | CONN HEM/F | FP-BM02B | C594367 | BM02B-AC J2 |
| L1 | 12.7mm(2) | HARWIN W | C923302 | M50-3530013 |
| L1, L2 | IND_CIGT2016 | SAM | | Description |
| R1, R2, R11 | RES 5.1K | RES 0402 | C25905 | 5.1 Kohms |
| R3, R4, R5 | RES 10K OHM | RES 0402 | C844452 | 10 kOhms |
| R6, R7 | RES 47K OHM | RES 0402 | C25792 | 47 kOhms |
| R8, R10, R | RES 100K OHM | YAGEO RE-| C313329 | 100 kOhms |
| R15, R26, R | RES SMD 4 | YAGEO RE-| C881369 | 4.7 kOhms |
| R18 | RES SMD 1M | YAGEO RE-| C176101 | 1 kOhms |
| SW1 | Tactile Swi | ESWI-TL33 | C913778 | - |
| U1 | ACCEL 2-1 | FP-LGA12 | C915682 | LIS2DS12T U1 |
| U2 | Description | QFN40_5X | C3007767 | NRF52833 U2 |
| U3 | NPM1304 | QFN50P500X500X90-| | NPM1304-U3 |
| U4 | Real Time | FP-RV-302 | C3019759 | RV-3028-C U4 |
| U5 | AS7058 Se | BGA42C4C | C2378277 | AS7058-BM U5 |
| U6 | Flash Mem | SOIC-8 | C7519 | IS25WP01J U6 |
| U7 | 5A 17V 6V | STM-SOT2 | C3197684 | USBLC6-2@ U7 |
| Y1 | CRYSTAL 3 | ABRACONI | C124330 | 32MHz Y1 |

**Total Cost**: [CALCULATE TOTAL]

---

**Last Updated**: [DATE]
**Maintained By**: [YOUR NAME/TEAM]
**Version**: [VERSION NUMBER]

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
[SPECIFY MATERIAL AND PROPERTIES]

### Dimensions & Layout
[DESCRIBE LAYOUT AND DIMENSIONS]

### Durability Features
- **Water Resistance**: [IPX RATING]
- **Shock Resistance**: [SPECIFY LEVEL]
- **Material Durability**: [DESCRIBE]

## Testing & Validation

### Hardware Testing
- [ ] [TEST 1 - DESCRIBE]
- [ ] [TEST 2 - DESCRIBE]
- [ ] [TEST 3 - DESCRIBE]

### Performance Validation
[DESCRIBE VALIDATION PROCEDURES AND RESULTS]

### Reliability Testing
[DESCRIBE RELIABILITY TEST METHODS AND RESULTS]

