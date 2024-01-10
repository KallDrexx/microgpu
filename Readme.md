- [Overview](#overview)
  - [Firmware](#firmware)
    - [Common C Firmware Code](#common-c-firmware-code)
    - [SDL Based Implementation](#sdl-based-implementation)
    - [ESP32-S3 Implementation](#esp32-s3-implementation)

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
displays and databuses.

The displays is the system the Microgpu can use to render graphics to. It can be
an attached LCD panel, VGA port, or even an SDL framebuffer.

This is done by creating an implementation of the
[Mgpu_Display](firmware/microgpu-common/display.h) type and its respective
functions.

A databus is the system responsible for receiving operations from a controlling
device and sending responses back. Can be based on SPI, UART, or even TCP based.
It just needs enough bandwidth to be able to send the desired commands in the
desired time frame.

A databus is created by implementing the
[Mgpu_Databus](firmware/microgpu-common/databus.h) type and its respective
functions.

### SDL Based Implementation

The [microgpu-sdl-fw](firmware/microgpu-sdl-fw/) folder contains a firmware
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

