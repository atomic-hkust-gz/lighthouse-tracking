# LIGHTHOUSE TRACKING with TS4231

A project based on openWSN, see ts4231 firmware at ./bsp/chips/ts4231.

## CONTENTS

### Main Project

- `./projects/nrf52840_dk_TS4231`

- [x] ### 1.TS4231 firmware

  #### Instructions

  This is a reimplementation of the official Arduino library, which is entirely C-based, runs on the NRF52840, and does not use the NRF SDK.

  #### Files

  - `./bsp/chips/ts4231/ts4231.h`
  - `./bsp/chips/ts4231/ts4231.c`

- [x] ### 2.TS4231 localization with A HTC lighthouse 1.0

  #### Instructions

  This is based on lighthouse positioning using a TS4231 for the receiver and a BPW34S for the photodiode, it is completely C based, runs on an NRF52840 and does not use the NRF SDK.

  (Dec. 2023) Single lighthouse and single TS4231.

  #### Files
  
  - `./bsp/chips/ts4231/ppi_gpiote.h`
  - `./bsp/chips/ts4231/ppi_gpiote.c`

### 3. In progress...



## REFERENCE

- OpenWSN firmware: stuff that runs on a mote
  - Part of UC Berkeley's OpenWSN project, http://www.openwsn.org/.
  - also our fork version, https://github.com/atomic-hkust-gz/openwsn-fw.

- Open source project: https://github.com/HiveTracker/firmware.

- QingFeng BBS: http://www.qfv8.com/

