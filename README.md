# LiteX FreeRTOS demos on GateMate

This repository contains demonstration projects for running FreeRTOS on LiteX
RISC-V SoCs targeting the Olimex GateMate A1 EVB.

> NOTE:
> The current demo setup uses a GateMate board, but the FreeRTOS/LiteX software
> parts can be adapted to any board supported by LiteX.

`demo_blinky/` is a FreeRTOS application that has currently been
tested with *NEORV32* and *VexRiscv* LiteX CPU variants.

`olimex_gatemate_a1_evb.py` is a copy of the
[litex-boards target](https://github.com/litex-hub/litex-boards/blob/master/litex_boards/targets/olimex_gatemate_a1_evb.py)
with a small change that uses BRAM as MAIN-RAM (16 KiB), since external PSRAM
support has not been implemented here yet.

## Cloning repository

This repository uses a Git submodule for the FreeRTOS kernel. Clone it with
submodules enabled:

```sh
git clone --recurse-submodules https://github.com/trabucayre/litex_freertos_demos.git
cd litex_freertos_demos
```

If the repository has already been cloned without submodules, initialize them
from the repository root:

```sh
git submodule update --init --recursive
```

## LiteX setup

See the
[LiteX installation guide](https://github.com/enjoy-digital/litex/wiki/Installation)
for the current LiteX setup instructions.

## Goal

The goal is to provide small, reproducible demonstrations that run the same
FreeRTOS blinky/queue workload on LiteX RISC-V soft CPUs while keeping
CPU-specific differences isolated.

The demo:

- installs the FreeRTOS RISC-V trap handler in `mtvec`;
- uses LiteX UART for console output;
- toggles a LiteX LED GPIO from a FreeRTOS task;
- uses either the CPU machine timer or LiteX `timer0` as the FreeRTOS tick
  source, depending on the generated SoC.

## CPU differences

NEORV32 and VexRiscv expose interrupts differently in these LiteX builds.

For NEORV32, `FreeRTOSConfig.h` enables the machine timer path by defining:

```c
configMTIME_BASE_ADDRESS
configMTIMECMP_BASE_ADDRESS
```

For VexRiscv, those addresses must remain zero. The demo then provides its own
`vPortSetupTimerInterrupt()` implementation, which configures LiteX `timer0` and
enables the LiteX external interrupt bit.

The most important rule is that the application must compile against the
generated headers for the matching CPU build. If a VexRiscv application
accidentally includes NEORV32 generated headers, FreeRTOS can select the wrong
timer path and fail to boot.

## Building

### Gateware

```bash
./olimex_gatemate_a1_evb.py --build [--load] [--cpu-type xxxx]
```

The `--load` option may be used at the same time or in a second step.
`--cpu-type` must be:
- `vexriscv` (default)
- `neorv32`

The gateware must be in place before compiling the demo.

### Software

Build the NEORV32 demo with the generated NEORV32 LiteX software tree:

```sh
cd demo_blink
make clean all
```

After the build completes, `demo.bin` is created in `demo_blink/`.
This file is used by `litex_term` to perform a serial boot.

## Running

At root directory:

```bash
litex_term --kernel demo_blink/demo.bin /dev/ttyACM0
```

The expected banner is:

```text

        __   _ __      _  __
       / /  (_) /____ | |/_/
      / /__/ / __/ -_)>  <
     /____/_/\__/\__/_/|_|
   Build your hardware, easily!

 (c) Copyright 2012-2026 Enjoy-Digital
 (c) Copyright 2007-2015 M-Labs

 BIOS built on Aug  6 2026 10:56:38
 BIOS CRC passed (e4f18c38)

 LiteX git sha1: c6a5f6a6f

--================ SoC =================--
CPU:            VexRiscv @ 24MHz
BUS:            wishbone 32-bit data/32-bit addr
CSR:            32-bit data big ordering
ROM:            128.0KiB
SRAM:           8.0KiB
MAIN RAM:       16.0KiB

--=========== Initialization ===========--
Memtest at 0x40000000 (16.0KiB)...
  Write: 0x40000000-0x40004000 16.0KiB
   Read: 0x40000000-0x40004000 16.0KiB
Memtest OK
Memspeed at 0x40000000 (Sequential, 16.0KiB)...
  Write speed: 35.7MiB/s
   Read speed: 18.5MiB/s
--================ Boot ================--
Booting from serial...
Press Q or ESC to abort boot completely.
sL5DdSMmkekro
[LITEX-TERM] Received firmware download request from the device.
[LITEX-TERM] Uploading demo_blink/demo.bin to 0x40000000 (15624 bytes)...
[LITEX-TERM] Upload calibration... (inter-frame:  0.00us, length: 64, window: 1)
[LITEX-TERM] Upload complete (9.4KB/s).
[LITEX-TERM] Booting the device.
[LITEX-TERM] Done.
Executing booted program at 0x40000000

--============== Liftoff! ==============--

<<< LiteX running FreeRTOS V11.1.0+ >>>
```

If the binary prints an exception message or hangs before the banner, first
check that the application was built against the generated headers for the same
CPU as the bitstream and BIOS.

## Contact

E-mail: Gwenhael Goavec-Merou <gwenhael.goavec-merou@trabucayre.com></br>
Copyright (C) <b>2025</b></br>
SPDX-License-Identifier: BSD-2-Clause</br>
