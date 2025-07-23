/**
    \brief Definition of the nrf52480 Timer driver.
    \author Manjiang Cao   <manjiang19@hkust-gz.edu.cn> Atomic. Nov, 2023
    \author Tengfei Chang  <tengfeichang@hkust-gz.edu.cn> Atomic. Nov, 2023
*/

#include "nrf52833.h"
#include "nrf52833_bitfields.h"
#include "ppi.h"

//=========================== defines =========================================

#define NUM_PPI_CH    5

//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {

    uint32_t ch_eep[NUM_PPI_CH];
    uint32_t ch_tep[NUM_PPI_CH];

}ppi_vars_t;

ppi_vars_t ppi_vars;

//=========================== prototype =======================================

void ppi_get_ch_register_value(uint8_t channel);

//=========================== public ==========================================

void ppi_enable(uint8_t channel) {
    NRF_PPI->CHENSET = 1<<channel;
}
void ppi_disable(uint8_t channel) {

    NRF_PPI->CHENCLR = 1<<channel;
}

void ppi_radio_sof_timer_capture(uint8_t channel, uint8_t capture_id) {

    NRF_PPI->CH[channel].EEP = (uint32_t)&NRF_RADIO->EVENTS_FRAMESTART;
    NRF_PPI->CH[channel].TEP = (uint32_t)&NRF_TIMER0->TASKS_CAPTURE[capture_id];
    ppi_get_ch_register_value(channel);
}
void ppi_radio_eof_timer_capture(uint8_t channel, uint8_t capture_id) {

    NRF_PPI->CH[channel].EEP = (uint32_t)&NRF_RADIO->EVENTS_END;
    NRF_PPI->CH[channel].TEP = (uint32_t)&NRF_TIMER0->TASKS_CAPTURE[capture_id];
    ppi_get_ch_register_value(channel);
}
void ppi_timer_compare_radio_start(uint8_t channel, uint8_t compare_id) {

    NRF_PPI->CH[channel].EEP = (uint32_t)&NRF_TIMER0->EVENTS_COMPARE[compare_id];
    NRF_PPI->CH[channel].TEP = (uint32_t)&NRF_RADIO->TASKS_START;
    ppi_get_ch_register_value(channel);
}


void ppi_rtc_schedule_timer_capture(uint8_t channel, uint8_t compare_id, uint8_t capture_id) {

    NRF_PPI->CH[channel].EEP = (uint32_t)&NRF_RTC0->EVENTS_COMPARE[compare_id];
    NRF_PPI->CH[channel].TEP = (uint32_t)&NRF_TIMER0->TASKS_CAPTURE[capture_id];
    ppi_get_ch_register_value(channel);
}

//=========================== private =========================================


void ppi_get_ch_register_value(uint8_t channel) {

    ppi_vars.ch_eep[channel] = NRF_PPI->CH[channel].EEP;
    ppi_vars.ch_tep[channel] = NRF_PPI->CH[channel].TEP;
}
