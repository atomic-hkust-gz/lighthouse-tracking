/**
 * brief TS4231 localization implementation.
 * This code implements the use of one TS4231 and one HTC lighthouse v1 for localization.
 * The lighthouse needs to be set up in a room, and the corresponding photodiode
 * BPW34S of the TS4231 needs to be able to "see" the lighthouse, with no shaded objects between them.
 *
 * For the Dec.3 2023 version, we only distinguish whether the light from the lighthouse
 * is in the X-axis or the Y-axis. Simply put, there are 8 different lengths of sync light,
 * and we only classify them into two categories according to the axis information.
 * \author Cheng Wang <cwang199@connect.hkust-gz.edu.cn>, Dec 2023.
 * \Reference to open source project: https://github.com/HiveTracker/firmware
 * \Reference to QingFeng BBS: http://www.qfv8.com/
 *
 * This file is also reference to OpenWSN ppi, timer driver: https://github.com/atomic-hkust-gz/openwsn-fw,
 * \author Manjiang Cao   <manjiang19@hkust-gz.edu.cn> Atomic. Nov, 2023
 * \author Tengfei Chang  <tengfeichang@hkust-gz.edu.cn> Atomic. Nov, 2023
 */

#include "ppi_gpiote.h"
#include "SEGGER_RTT.h"
#include "nrf52840.h"
#include "nrf52840_bitfields.h"
#include "stdbool.h"
#include "timer_ts4231.h"
#include "ts4231.h"

//=========================== define ==========================================

// we have use E:P(0,3) and D:P(0,4), and config them as input in ts4231.h

//=========================== public ==========================================

/**
 * Configure two GPIOTE interrupts for entering GPIOTE_IRQHandler() in this state.
 * This function must be executed after ts4231_init().
 *
 * The TS4231 is in the WATCH state with Epin normally high, so the signals are all low.
 * Therefore, a falling edge indicates the beginning of a signal and a rising edge
 * indicates the end of a signal. Considering that we need to calculate the signal duration,
 * both moments need to be able to trigger an interrupt, so we use two GPIOTE channels to
 * configure Epin's falling and rising edges to trigger an interrupt.
 */
void gpiote_init(void) {
  // A falling edge indicates the beginning of a wave
  // Specify a channel number [0], set to falling edge trigger, specify pin, set to event mode
  NRF_GPIOTE->CONFIG[0] =
      (GPIOTE_CONFIG_POLARITY_HiToLo << GPIOTE_CONFIG_POLARITY_Pos) |
      (TS4231_N1_E_PIN << GPIOTE_CONFIG_PSEL_Pos) |
      (GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos);
  // A rising edge indicates the end of a wave
  NRF_GPIOTE->CONFIG[1] =
      (GPIOTE_CONFIG_POLARITY_LoToHi << GPIOTE_CONFIG_POLARITY_Pos) |
      (TS4231_N1_E_PIN << GPIOTE_CONFIG_PSEL_Pos) |
      (GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos);
  // Enable interrupts
  NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_IN0_Set << GPIOTE_INTENSET_IN0_Pos;
  NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_IN1_Set << GPIOTE_INTENSET_IN1_Pos;
  NVIC_EnableIRQ(GPIOTE_IRQn);
}

//=========================== interrupt handlers ==============================

/**
 * The falling edge interrupt is the start of a signal and we start the clock with t_0_start,
 * being the start moment of the signal. We also set flag_start to 1. This is because
 * we want the falling edge to be the start of the function's logic, and we will
 * check flag_start in the rising edge interrupt to make sure that the falling edge
 * interrupt is executed first before calculating the process that follows. In the rising
 * edge interrupt, we store the current moment in t_0_end and set flag_start to 0 to indicate
 * that we have completed the acquisition of this signal. We also put the value of this moment
 * into t_1_start at the end, which will be used for the subsequent calculation of the interval
 * between the two signals.
 *
 * The difference between the two moments is the duration of the signal, t_opt_pulse, which is
 * classified as sync or sweep depending on the duration. If it is sync, it means that the next
 * time a falling edge occurs it will be sweep, so flag_sweep is set to 1. Meanwhile, it also means
 * that the next time a falling edge interrupt arrives the time difference between the two signals
 * needs to be calculated, for which cal_distance is also set to 1. We also need to know the
 * specifics of this sync, so we have consulted the data to give us a simple way to differentiate
 * between the XY axis and express the result via loca_x.
 *
 * Finally, when the next falling edge interrupt arrives, if cal_distance is 1, subtract the last
 * t_1_start from the current t_0_start to get loca_duration, and then determine whether we should
 * give L_X or L_Y according to the value of loca_x. In this way, a localization is finished.
 */
uint32_t t_0_start = 0x00;
uint32_t t_0_end = 0x00;
uint32_t t_opt_pulse = 0x00;
uint32_t t_opt_pulse_us = 0x00;
uint32_t t_1_start = 0x00;
int8_t flag_start = 0;
int32_t flag_opt = 0;
bool flag_sweep = 0;
bool cal_distance = 0;
uint32_t loca_duration = 0;
int8_t loca_x = 0;
uint32_t L_X = 0;
uint32_t L_Y = 0;

void GPIOTE_IRQHandler(void) {
  // falling edge interrupt, first check and clear the interrupt flag bit
  if ((NRF_GPIOTE->EVENTS_IN[0] == 1) && (NRF_GPIOTE->INTENSET & GPIOTE_INTENSET_IN0_Msk)) {
    NRF_GPIOTE->EVENTS_IN[0] = 0;

    switch (flag_start) {
    case 0:
      t_0_start = timer3_getCapturedValue(0);

      flag_start++;

      if (cal_distance == 1) {
        loca_duration = t_0_start - t_1_start;

        if (loca_x == 1) {
          L_X = loca_duration;
        } else {
          L_Y = loca_duration;
        }
      }

      t_1_start = t_0_start;
      break;
    case 1:
      // currently of no practical use
      flag_opt++;
      break;
    default:
      break;
    }
  }

  // rising edge interrupt, first check and clear the interrupt flag bit
  if ((NRF_GPIOTE->EVENTS_IN[1] == 1) && (NRF_GPIOTE->INTENSET & GPIOTE_INTENSET_IN1_Msk)) {
    NRF_GPIOTE->EVENTS_IN[1] = 0;

    switch (flag_start) {
    case 0:
      break;
    case 1:
      t_0_end = timer3_getCapturedValue(1);
      flag_start = 0;
      t_opt_pulse = t_0_end - t_0_start;
      // Dividing the two signals by 50us: 0.000,050/(1/16M) = 800 = 0x320
      (t_opt_pulse < 800) ? (flag_sweep = 1) : (flag_sweep = 0);
      t_opt_pulse_us = t_opt_pulse / 16;
      switch (flag_sweep) {
      // More than 50us then it must be sync, then the next falling edge interrupt will need to calculate position
      case 0:
        cal_distance = 1;
        // Where an even ten digit number is the X-axis, an odd number is the Y-axis.
        ((t_opt_pulse_us / 10) % 2 == 0) ? (loca_x = 1) : (loca_x = 0);

        break;
      case 1:
        cal_distance = 0;
        break;
      }
      break;
    default:
      break;
    }
  }
}

//=========================== public ==========================================

/**
 * Since each GPIOTE interrupt needs to be timed simultaneously, we use PPI to
 * improve efficiency. This eliminates the process of grabbing the TIMER value in
 * each interrupt, reducing CPU involvement and improving real-time performance.
 * The CAPTURE task was used for each time grab, and the TIMER was not cleared,
 * restarted, or paused. This is because the overflow time for a 32bit register
 * is in the tens of seconds, well beyond our maximum counting time, and thus does
 * not need to be taken into account(see timer_ts4231.c).
 */
void ppi_init(void) {
  // Channel [0] connects the falling edge GPIOTE [0] interrupt event
  // and the TIMER3 grab current count value to CC [0] task together.
  NRF_PPI->CH[0].EEP = (uint32_t)(&NRF_GPIOTE->EVENTS_IN[0]);
  NRF_PPI->CH[0].TEP = (uint32_t)(&NRF_TIMER3->TASKS_CAPTURE[0]);

  // Channel [1] connects the rising edge GPIOTE [1] interrupt event
  // and the TIMER3 grab current count value to CC [1] task together.
  NRF_PPI->CH[2].EEP = (uint32_t)(&NRF_GPIOTE->EVENTS_IN[1]);
  NRF_PPI->CH[2].TEP = (uint32_t)(&NRF_TIMER3->TASKS_CAPTURE[1]);

  // enable channel 0,2
  NRF_PPI->CHENSET = 0x05;
}