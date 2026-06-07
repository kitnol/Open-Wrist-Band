# Embedded software
### Project files structure:
- `boards/` - custom board import into Zephyr OS based on the PCB design and required features, contains pin mapping definitions and enables individual subsystems (I2C, SPI, USB, UART)
- `src/` - all of the source code that is part of the embedded application
  - `main.c` - main code setup
  - `as7058_drv.c` - custom made driver for AS7058 bio chip based on its datasheet
  - `flash_drv.c` - flash layer for the onboard spi nor flash - littlefs implementation
  - `shell.c` - the implementation of shell commnads
  - `usb_drv.c` - the implementation of the usb interface management
- `sysbuild/` - mcuboot bootloader implementation and configuration
- `app.overlay` - configuration of specifc components present on the board
- `prj.conf` - project configuration enabling and configuring different zephyros features


Step-by-step guide on how to set up coding environment for embedded software development. The firmware was developed using nRF connect SDK v3.2.4 with nRF extensions enabled, using Visual studio code.

## Environment setup
1. Download Nordic semiconductor SDK for nrf52833 in Visual studio code
2. Open/import the project folder from github
3. Go into your SDK installation ncs\v3.2.4\bootloader\mcuboot\zephyr\main.c and implement https://github.com/mcu-tools/mcuboot/pull/1903 
This feature was added to allow entering DFU mode (for code flashing) after triggering a hardware reset, instead of entering DFU mode on every single after power-up. This improves power usage by not wasting time in DFU mode when not needed, while keeping the feature to allow for reflashing with non-functional software without a J-link debugger.

## Flashing of an empty board
1. (if needed) Create the build configuration, ensure sysbuild is enabled and the board target is correctly selected. Keep all of the settings on default.  
![Create build configuration](_doc/createbuild.png)  
![Build configration](_doc/buildconfig.png)
2. Build the project using nRF Connect SDK actions (or by *west build*)  
![Build complete](_doc/buildcomplete.png)
3. **IMPORTANT** - Ensure the Segger J-link device is set to **1.8V voltage level** as higher voltages will damage some of the PCB components
4. Connect the Segger J-link to the board using the pin interface, ensure that the "detect pin" on the Segger J-link device is pulled to ground, if needed, as the PCB itself supports only 5 pin J-link interface and does not interact with the "detect pin".
5. Flash the entire build (sysbuild) onto the nRF microcontroller using built in nRF Connect SDK actions (or by *west flash*)![Build flashing](_doc/flashing1.png)

## Flashing of a board with MCUboot present on it
Prerequisite: Download and open AuTerm as mentioned in course https://academy.nordicsemi.com/courses/nrf-connect-sdk-intermediate/ within Lesson 9 - Bootloaders and DFU 
1. Build the project using nRF Connect SDK actions (or by *west build*)
2. Locate the file in ``build/<folder_name>/zephyr/zephyr.signed.bin`` and open it in AuTerm's MCUmgr
3. Connect the board using USB
4. Press the reset button on the board (if present) or short the reset pin of J-link interface to the ground on the board. (TODO: implement command in shell interface to trigger DFU mode)
5. Find the device in AuTerm and open communication with it
6. Flash using the MCUmgr section  
![Flashing](_doc/flashing.png)

## Configurnig measurement parameters using AS7058 Sensor Driver
### Overview
This driver provides initialization and control for the AMS AS7058 multi-parameter biosensor used for PPG (photoplethysmography), ECG, and BioZ measurements. The sensor communicates via I2C and supports configurable LED modulation, photodiode selection, and FIFO-based data acquisition.

### Configuration

The driver uses preprocessor macros for configuration (defined in `as7058_drv.c`):

#### Measurement Sampling
- `PPG_FREQ`: PPG sampling frequency in Hz (default: 10 Hz)
- `ECG_FREQ`: ECG sampling frequency in Hz (default: 10 Hz)
- `FIFO_SAMPLE_COUNT`: Number of samples to buffer before FIFO threshold interrupt (default: 423, max: 511) This should be coordinated together with the byte structure that stores the offloaded data from FIFO

#### LED Configuration
- `PPG_MOD_1`, `PPG_MOD_2`: Enable/disable PPG modulators 1 and 2
- `LED_x_max_current`: Max current setting per LED (0-3, corresponding to 25mA, 150mA, 225mA, or 300mA)
- `LED_x_current`: Actual LED current in mA

**PCB LED Mapping:**
- LED1: IR_1, LED2: GREEN_1, LED3: IR_2
- LED5: RED_2, LED6: GREEN_2, LED7: RED_1

#### Photodiode Selection
- `LED_SUBx`: Configure which LEDs are active for measurement sub-sample x
- `PPG1_PDSELx`, `PPG2_PDSELx`: Configure which photodiodes are active for each sub-sample

Each register uses bit-field encoding:
- `LED_SUB` bits 0-3: Driver1 LED selection, bits 4-7: Driver2 LED selection
- `PDSEL` bits 0-7: Photodiode 1-8 enable/disable  

#### NOTE: For more detailed configuration please consult with the datasheet of AS7058 sensor available online, where all of the registers and their function/configuration is described in detail.