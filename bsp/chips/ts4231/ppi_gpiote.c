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
 * The difference between the two moments is the duration of the signal, t_opt_pulse, which we
 * convert to us by dividing it by the number of ticks, and classify it into sync, sweep,
 * and skip_sync according to the duration (defined in ppi_gpiote.h as type_sync, type_sweep,
 * and type_skip_sync, respectively).
 *
 * Since the sync light of the two base stations alternates, i.e., when base station A transmits
 * 60-100us of light, base station B will follow with 100-150us of light. We choose to start the
 * logic from sweep light, which is convenient for us to distinguish the sequence of the two base
 * stations. In the rising edge interrupt, if we confirm that this light is a sweep, we set
 * flag_station to 1, so that when the next light comes, it must come from base station A. When
 * the next light is a skip_sync, we choose to give flag_station +1, so that the sync light after
 * the skip_sync knows that it is from base station B. In the sync light interrupt, we choose to
 * give flag_station +1, so that the sync light after the skip_sync knows that it is from base station B.
 * In the interrupt of the sync light, we represent the base station information with the value of
 * flag_A_station: according to flag_station == 1 (A),flag_station == 2 (B). Meanwhile, the falling
 * edge moment of sync is stored separately to t_d_start .
 *
 * Finally, the sweep appears, and we choose to update the data to A_X A_Y or B_X B_Y depending on
 * the value of flag_A_station. loca_duration is the difference between the previously stored t_d_start
 * and the falling edge moment of the sweep, t_0_start.
 *
 * This logic works fine in the presence of 1 or 2 lighthouses.
 */
uint32_t t_0_start = 0x00;
uint32_t t_0_end = 0x00;
uint32_t t_opt_pulse = 0x00;
uint32_t t_opt_pulse_us = 0x00;
uint32_t t_1_start = 0x00;
uint32_t t_d_start = 0x00;
uint8_t flag_start = 0;
// 0:sync;1:sweep;2:skip_sync
uint8_t flag_light_type = 0;

// after a sweep, wo is the first which is the station A
uint8_t flag_station = 0;
// 0：NULL,1:A,2:B
uint8_t flag_A_station = 0;
uint32_t loca_duration = 0;
uint8_t loca_x = 0;
uint32_t A_X = 0;
uint32_t A_Y = 0;
uint32_t B_X = 0;
uint32_t B_Y = 0;

void GPIOTE_IRQHandler(void) {
  // falling edge interrupt, first check and clear the interrupt flag bit
  if ((NRF_GPIOTE->EVENTS_IN[0] == 1) && (NRF_GPIOTE->INTENSET & GPIOTE_INTENSET_IN0_Msk)) {
    NRF_GPIOTE->EVENTS_IN[0] = 0;

    switch (flag_start) {
    case 0:
      t_0_start = timer3_getCapturedValue(0);

      flag_start = 1;
      break;
    case 1:
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
      // Dividing the two signals by 50us: 0.000,050/(1/16M) = 800 = 0x320,99us(1584) for skip/sync
      (t_opt_pulse < 800) ? (flag_light_type = type_sweep) : ((t_opt_pulse < 1584) ? (flag_light_type = type_sync) : (flag_light_type = type_skip_sync));
      t_opt_pulse_us = t_opt_pulse / 16;
      switch (flag_light_type) {
      // More than 50us then it must be sync, then the next falling edge interrupt will need to calculate position
      case type_sync:
        // If sync, distance measurement starts from this time.
        t_d_start = t_0_start;
        // It is only based on sweep that you can determine whether you are currently in A or B.
        if (flag_station >= 1) {
          //  Where an even ten digit number is the X-axis, an odd number is the Y-axis.
          ((t_opt_pulse_us / 10) % 2 == 0) ? (loca_x = 1) : (loca_x = 0);
        }
        // Determine whether this is an A or B base station based on the value of flag_station
        switch (flag_station) {
        case 0:
          break;
        case 1:
          flag_A_station = 1;
          break;
        case 2:
          flag_A_station = 2;
          break;
        default:
          break;
        }
        break;
      case type_sweep:
        //  0 ：NULL,1: next is A,2:next is B
        flag_station = 1;

        loca_duration = t_0_start - t_d_start;
        switch (flag_A_station) {
        case 0:
          break;
        // A
        case 1:
          if (loca_x == 1) {
            A_X = loca_duration;
          } else {
            A_Y = loca_duration;
          }
          flag_A_station = 0;
          break;
        // B
        case 2:
          if (loca_x == 1) {
            B_X = loca_duration;
          } else {
            B_Y = loca_duration;
          }
          flag_A_station = 0;
          break;
        default:
          break;
        }

        break;
      case type_skip_sync:
        if (flag_station >= 1) {
          flag_station++;
          // Exceeding 2 means that a sweep was not seen, which often happens.
          if (flag_station >= 3) {
            flag_station--;
          }
        }
        break;
      default:
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