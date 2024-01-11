- [Overview](#overview)
  - [Firmware](#firmware)
    - [Common C Firmware Code](#common-c-firmware-code)
    - [SDL Based Implementation](#sdl-based-implementation)
    - [ESP32-S3 Implementation](#esp32-s3-implementation)
      - [Configuration](#configuration)
      - [Building, Flashing, and Monitoring](#building-flashing-and-monitoring)
  - [Hardware](#hardware)

# Overview

Microgpu is a project to create a Graphical Processing Unit (GPU) that can be
used by resource constrained microcontrollers to render high fidelity graphical
applications at a high framerate.

THe project consists of:
* A C library that can be used to create Microgpu firmwares for new MCUs
* A working implementation for the ESP32-S3
* Protocols to facilitate communication between MCUs and a Microgpu device
* C# driver to communicate with a Microgpu device from a Meadow device
* Hardware PCB designs for Microgpu devices.

## Firmware

The Microgpu firmware allows a device to receive and react to commands from a
microcontroller. While most Microgpu firmwares share a common core, each
firmware is tailored to each hardware device its implemented on, which displays
it can utilize, and which channels of communication it utilizes for receiving
commands.

There are currently two firmware implementations for Microgpu, an SDL one that
runs on PCs and an esp32-s3 one.


### Common C Firmware Code

The code that runs most Microgpu firmware implementations is a set of C header
and source files found in
[/firmware/microgpu-common](firmware/microgpu-common/).

This provides standard logic for deserializing and executing incoming
operations, as well as serializing any responses that may occur.

Most Microgpu firmware implementations can use the same logic for the majority
of the operations, but each implementation must uniquely define two concepts:

* Databus
  * The system responsible for receiving operations from a controlling device
    and sending responses back. Can be based on SPI, UART, or even TCP based. It
    just needs enough bandwidth to be able to send the desired commands in the
    desired time frame.
  * A databus is created by implementing the
    [Mgpu_Databus](firmware/microgpu-common/databus.h) type and its respective
    functions.
* Display
  * The system the Microgpu can use to render graphics to. It can be an attached
  LCD panel, VGA port, or even an SDL framebuffer. displays and databuses.
  * Implemented by creating an implementation of the
    [Mgpu_Display](firmware/microgpu-common/display.h) type and its respective
    functions.


### SDL Based Implementation

The [microgpu-sdl-fw folder](firmware/microgpu-sdl-fw/) contains a firmware
implemented for use on normal PC hardware. It uses the SDL library for window
management, SDL's framebuffer for rendering its graphics, and hosts a TCP server
for its databus channel.

The primary purpose of the SDL firmware is for quick iteration and testing of
common Microgpu functionality, without having to always deploy clients or
firmware builds to embedded systems all the time.

The SDL implementation's build is managed
[by its own cmake file](firmware/microgpu-sdl-fw/CMakeLists.txt), and relies on
vcpkg for referencing the SDL library.

It contains two targets, a `tcp` and `test` target. The `tcp` target creates a TCP listener for databus operations, and thus can be interacted with by an external process. The `test` target has an in memory databus that gives a fixed set of operations to execute, allowing for verification of functionality without an additional external controlling process.

### ESP32-S3 Implementation

The [esp32-s3 folder](firmware/microgpu-esp32-fw/) contains a firmware designed
to run on the ESP32-S3 microcontroller. It can theoretically run on other ESP32
variants, but the current i80 8-bit parallel display implementation requires the S3's LCD APIs.

Building the ESP32 firmware requires the
[ESP-IDF 5.1 tool set](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/index.html).
The ESP-IDF tooling needs to be activated (either via the IDE instructions or
running their script on the command line) in order to properly build, configure,
flash and deploy the firmware. All `idf.py` commands need to be executed from within the `/firmware/microgpu-esp32-fw/` directory.

The project is loaded via the  
[cmake file](firmware/microgpu-esp32-fw/CMakeLists.txt). 

#### Configuration

Once the ESP-IDF tooling has been activated, you need to tell the ESP-IDF
tooling that the ESP32 variant you will be deploying to is an S3. To do that you
need to execute `idf.py set-target esp32s3`.

the project then needs to be configured via the `idf.py menuconfig` command.
This brings up a graphical configuration manager that allows you to set local
build and flash configuration.

The important configuration options are:

* `Compiler Options -> Optimization Level -> Optimize for performance`
  * This compiles in Release mode for better performance.
* `Component config -> ESP System Settings -> CPU Frequency`
  * This sets the CPU frequency of the MCU. ESP32-S3 can safely be put at 240Mhz
* `Component config -> ESP PSRAM`
  * This enables PSRAM if you are deploying to a chip that contains connected
    PSRAM. If enabled, make sure to select the correct Quad vs Octal SPI
    settings, or the esp32-s3 may error on boot.
  * PSRAM is slower than internal RAM, though the practical difference hasn't
    been benchmarked yet. Most Microgpu demos currently do not require PSRAM to
    be activated.
* `Microgpu Config -> Board Type`
  * This allows selecting what type of board you are using. This mostly just
    selects reasonable defaults for Microgpu specific options.
* `Microgpu Config -> Databus Type`
  * This allows picking from the implemented databus types. Currently two have
    been implemented, a `SPI` based databus and a `Benchmark` based databus. The
    latter has an in-memory set of operations that get executed and is used to
    benchmark different scenarios.
* `Microgpu Config -> Display Type`
  * Which display implementation to use. Currently only one exists.
* `Microgpu Config -> SPI Databus Pins`
  * This allows setting which pins are used for SPI databus functionality
* `Microgpu Config -> Display Options`
  * This allows configuring display resolution, if the display has a read clock
    pin, and the pin numbers required to communicate with the display.


#### Building, Flashing, and Monitoring

* `idf.py build` can be executed to build the project.
* `idf.py -p COMX flash` deploys the current build to a connected ESP32
* `idf.py -p COMX monitor` connects to the connected ESP32 and retries stdout
  info

## Hardware

An initial hardware implementation of the Microgpu has been designed and built based on the ESP32 architecture.  

For version 1.0a:
* [EasyEDA Project](https://oshwlab.com/kalldrexx/microgpu-esp32-s3-embedded-v1)
* [Schematic](hardware/esp32-s3%20embedded/v1.0a/Schematic_Microgpu%20Esp32-s3%20Embedded_2024-01-10.pdf)
* [PCB Gerber](hardware/esp32-s3%20embedded/v1.0a/Gerber_PCB_Microgpu%20Esp32-s3%20Embedded.zip)
* [BOM](hardware/esp32-s3%20embedded/v1.0a/BOM_PCB_Microgpu%20Esp32-s3%20Embedded_2024-01-10.csv)

This PCB is designed to handle inputs from a controlling device via either
Mikrobus or a breadboard compatible (straight pinned) layout. Only one of these
should be used at a time.

Likewise, the device supports two different types of displays, an 8-bit parallel
display with an Arduino Uno style pinout (ILI9341 compatible), or a 16-bit
display with an Arduino Mega style pinout (e.g. ILI9488).

While the PCB contains a USB port for flashing and monitoring the ESP32-S3, **no
power will run through the USB port**. 3V power for the ESP32-S3 must come
through the 3V Mikrobus or Breadboard compatible pins.

Note that in the v1.0a design, the `Microgpu Reset` pin is a direct connection
to the ESP32-S3's `EN` pin. This means that the ESP32-S3 requires an active high
voltage through the `Microgpu Reset` pin in order for the ESP32-S3 to turn on.
Holding this pin low will turn the Microgpu off, and not setting an explicit
value through this pin risks leaving it floating, which may cause the Microgpu
to turn on and off randomly.
