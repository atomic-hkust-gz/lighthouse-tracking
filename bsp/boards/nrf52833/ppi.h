/**

Manjiang Cao   <manjiang19@hkust-gz.edu.cn> Atomic.
Tengfei Chang  <tengfeichang@hkust-gz.edu.cn> Atomic.

*/

#ifndef __PPI_H__
#define __PPI_H__

#include "board_info.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== module variables ================================

//=========================== prototypes ======================================

void ppi_enable(uint8_t channel);
void ppi_disable(uint8_t channel);

void ppi_radio_sof_timer_capture(uint8_t channel, uint8_t capture_id);
void ppi_radio_eof_timer_capture(uint8_t channel, uint8_t capture_id);
void ppi_timer_compare_radio_start(uint8_t channel, uint8_t compare_id) ;

void ppi_rtc_schedule_timer_capture(
    uint8_t channel,
    uint8_t compare_id,
    uint8_t capture_id
);

#endif // __ADC_SENSOR_H__
