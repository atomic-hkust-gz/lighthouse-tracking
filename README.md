# LIGHTHOUSE TRACKING with TS4231

A project based on openWSN, see ts4231 firmware at ./bsp/chips/ts4231.

 It is completely C based, runs on an NRF52840 and does not use the NRF SDK(Currently).

## CONTENTS

### Main Project

- `./projects/nrf52840_dk_TS4231`

### 1. TS4231 firmware

  #### Instructions

  This is a reimplementation of the official Arduino library: [TriadSemi/TS4231 (github.com)](https://github.com/TriadSemi/TS4231/tree/master)..

  #### Files

  - `./bsp/chips/ts4231/ts4231.h`
  - `./bsp/chips/ts4231/ts4231.c`

### 2. TS4231 localization with A HTC lighthouse 1.0

  #### Instructions

  This is based on lighthouse positioning using a TS4231 for the receiver and a BPW34S for the photodiode.

  (Dec. 2023) Single lighthouse and single TS4231.

  #### Files

  - `./bsp/chips/ts4231/ppi_gpiote.h`
  - `./bsp/chips/ts4231/ppi_gpiote.c`

### 3. In progress...



## REFERENCE

- OpenWSN firmware: stuff that runs on a mote
  - Part of UC Berkeley's OpenWSN project, http://www.openwsn.org/.
  - also our fork version, [atomic-hkust-gz/openwsn-fw: OpenWSN firmware: stuff that runs on a mote (github.com)](https://github.com/atomic-hkust-gz/openwsn-fw).

- Open source project: [HiveTracker/firmware: nRF52 code: handles LightHouse pulse signals measured with TS4231 + IMU (github.com)](https://github.com/HiveTracker/firmware).
- Official TS4231 library: [TriadSemi/TS4231 (github.com)](https://github.com/TriadSemi/TS4231/tree/master).

- QingFeng BBS:[青风电子社区 - Powered by Discuz! (qfv8.com)](http://www.qfv8.com/forum.php)

