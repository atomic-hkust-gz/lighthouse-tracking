/**
 * brief TIMER implementation.
 * This code implements the function of initializing and enabling TIMER3 and TIMER4.
 * In order to improve the real-time performance of the code, some functions that
 * need to be called at interrupt are designed as static inline types.
 *
 * \author Cheng Wang <cwang199@connect.hkust-gz.edu.cn>, Dec 2023.
 * \Reference to open source project: https://github.com/HiveTracker/firmware
 * \Reference to QingFeng BBS: http://www.qfv8.com/
 *
 * This file is also reference to OpenWSN timer driver: https://github.com/atomic-hkust-gz/openwsn-fw,
 * \author Manjiang Cao   <manjiang19@hkust-gz.edu.cn> Atomic. Nov, 2023
 * \author Tengfei Chang  <tengfeichang@hkust-gz.edu.cn> Atomic. Nov, 2023
 */

#ifndef _TIMER_H_
#define _TIMER_H_

#include "nrf52840.h"

void TIMER_init(void);
void TIMER3_init(void);
void TIMER4_init(void);

__STATIC_INLINE void timer3_start(void) {

  NRF_TIMER3->TASKS_START = 1;
}

__STATIC_INLINE void timer3_stop(void) {

  NRF_TIMER3->TASKS_STOP = 1;
}

__STATIC_INLINE void timer4_start(void) {

  NRF_TIMER4->TASKS_START = 1;
}

__STATIC_INLINE void timer4_stop(void) {

  NRF_TIMER4->TASKS_STOP = 1;
}

/**
 * Every time the CAPTURE[n] task is triggered, the Counter value is copied to
 * the CC[n] register ( From Official Doc).Therefore, after setting up the
 * PPI channel at ppi_gpiote.c, you only need to read CC[n] to get the value of this moment.
 */
__STATIC_INLINE uint32_t timer3_getCapturedValue(uint8_t compare_id) {

  return NRF_TIMER3->CC[compare_id];
}
/**
 * Every time the CAPTURE[n] task is triggered, the Counter value is copied to
 * the CC[n] register ( From Official Doc).Therefore, after setting up the
 * PPI channel at ppi_gpiote.c, you only need to read CC[n] to get the value of this moment.
 */
__STATIC_INLINE uint32_t timer4_getCapturedValue(uint8_t compare_id) {

  return NRF_TIMER4->CC[compare_id];
}
#endif // _TIMER_H_