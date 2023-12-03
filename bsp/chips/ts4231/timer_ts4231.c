/**
 * brief TIMER implementation.
 * This code implements the function of initializing and enabling TIMER3 and TIMER4.
 *
 * \author Cheng Wang <cwang199@connect.hkust-gz.edu.cn>, Dec 2023.
 * \Reference to open source project: https://github.com/HiveTracker/firmware
 * \Reference to QingFeng BBS: http://www.qfv8.com/
 *
 * This file is also reference to OpenWSN timer driver: https://github.com/atomic-hkust-gz/openwsn-fw,
 * \author Manjiang Cao   <manjiang19@hkust-gz.edu.cn> Atomic. Nov, 2023
 * \author Tengfei Chang  <tengfeichang@hkust-gz.edu.cn> Atomic. Nov, 2023
 */

#include "timer_ts4231.h"
#include "nrf52840.h"
#include "nrf52840_bitfields.h"

void TIMER_init(void) {

  // 16MHz HFCLOCK enabled on board_init()

  TIMER3_init();
  // not use
  // TIMER4_init();
}

/**
 * TIMER3 is configured in timer mode without frequency division (16M/(2^0)), and the timer 
 * runs at a maximum frequency of 16M/s. The registers holding the timing values are 
 * set to the highest 32 bits. 
 */
void TIMER3_init(void) {

  NRF_TIMER3->MODE = TIMER_MODE_MODE_Timer;
  // 2^0 = 1,16MHz
  NRF_TIMER3->PRESCALER = 0;
  NRF_TIMER3->BITMODE = TIMER_BITMODE_BITMODE_32Bit;
  // clear timer before use
  NRF_TIMER3->TASKS_CLEAR = 1;
  timer3_start();
}

/**
 * TIMER4 is configured in timer mode without frequency division (16M/(2^0)), and the timer 
 * runs at a maximum frequency of 16M/s. The registers holding the timing values are 
 * set to the highest 32 bits. 
 * 
 */
void TIMER4_init(void) {

  NRF_TIMER4->MODE = TIMER_MODE_MODE_Timer;
  // 2^0 = 1,16MHz
  NRF_TIMER4->PRESCALER = 0;
  NRF_TIMER4->BITMODE = TIMER_BITMODE_BITMODE_32Bit;
  // clear timer before use
  NRF_TIMER4->TASKS_CLEAR = 1;
  timer4_start();
}