/**
 * brief TS4231 localization implementation.
 *
 * \author Cheng Wang <cwang199@connect.hkust-gz.edu.cn>, Dec 2023.
 * \Reference to open source project: https://github.com/HiveTracker/firmware
 * \Reference to QingFeng BBS: http://www.qfv8.com/
 *
 * This file is also reference to OpenWSN ppi, timer driver: https://github.com/atomic-hkust-gz/openwsn-fw,
 * \author Manjiang Cao   <manjiang19@hkust-gz.edu.cn> Atomic. Nov, 2023
 * \author Tengfei Chang  <tengfeichang@hkust-gz.edu.cn> Atomic. Nov, 2023
 */

#include "stdbool.h"
#include "stdint.h"

//=========================== define ==========================================

#ifndef _PPIGPIOTE_
#define _PPIGPIOTE_

#endif
//indicate the type of light
#define type_sync 0
#define type_sweep 1
#define type_skip_sync 2
//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void ppi_init(void);
void gpiote_init(void);

//=========================== private =========================================