/**
    \brief Definition of the nrf52480 Timer driver.
    \author Manjiang Cao   <manjiang19@hkust-gz.edu.cn> Atomic. Nov, 2023
    \author Tengfei Chang  <tengfeichang@hkust-gz.edu.cn> Atomic. Nov, 2023
*/

#include "nrf52833.h"
#include "nrf52833_bitfields.h"
#include "timer.h"

//=========================== defines =========================================

#define MODE_TIMER              0
#define MODE_COUNTER            1
#define MODE_LOWPOWER_COUNTER   2

#define BITMODE_16BIT           0
#define BITMODE_0BIT            1
#define BITMODE_24BIT           2
#define BITMODE_32BIT           3

#define OFFSET_INTENSET_CC_0    16  
#define OFFSET_INTENSET_CC_1    17  
#define OFFSET_INTENSET_CC_2    18  
#define OFFSET_INTENSET_CC_3    19  
#define OFFSET_INTENSET_CC_4    20  
#define OFFSET_INTENSET_CC_5    21     

#define NUM_TIMER0_COMPARE      6

//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {
 
    timer_cbt         cb[NUM_TIMER0_COMPARE];
}timer_vars_t;

timer_vars_t timer_vars;

//=========================== prototype =======================================

//=========================== public ==========================================

void timer_init(void) {

    NRF_TIMER0->MODE    = MODE_TIMER;
    NRF_TIMER0->BITMODE = BITMODE_32BIT;

    // set prescaler
    NRF_TIMER0->PRESCALER = 0x00;

    // configure interrupt
    NVIC->IP[TIMER0_IRQn]         = (uint8_t)((TIMER_PRIORITY << (8 - __NVIC_PRIO_BITS)) & (uint32_t)0xFF);
    NVIC->ISER[TIMER0_IRQn>>5]    = (uint32_t)(0x1 << (TIMER0_IRQn & 0x1f));

    // set compare interrupt for timer0
    //NRF_TIMER0->INTENSET = (1<<OFFSET_INTENSET_CC_0) |\
    //                       (1<<OFFSET_INTENSET_CC_1) |\
    //                       (1<<OFFSET_INTENSET_CC_2) |\
    //                       (1<<OFFSET_INTENSET_CC_3) |\
    //                       (1<<OFFSET_INTENSET_CC_4) |\
    //                       (1<<OFFSET_INTENSET_CC_5);

                           
    NRF_TIMER0->INTENSET = (1<<OFFSET_INTENSET_CC_0);
}

void timer_set_callback(uint8_t compare_id, timer_cbt cb) {

    timer_vars.cb[compare_id] = cb;
}

void timer_schedule(uint8_t compare_id, uint32_t value) {

    NRF_TIMER0->CC[compare_id] = value;
}

uint32_t timer_getCapturedValue(uint8_t compare_id) {

    return NRF_TIMER0->CC[compare_id];
}


void timer_clear(void) {
    NRF_TIMER0->TASKS_CLEAR = 1;
}


void timer_capture_now(uint8_t capture_id) {
    NRF_TIMER0->TASKS_CAPTURE[capture_id] = 1;
}


void timer_start(void) {

    NRF_TIMER0->TASKS_START = 1;
}

void timer_stop(void) {
    
    NRF_TIMER0->TASKS_STOP = 1;
}

//=========================== private =========================================

void TIMER0_IRQHandler(void) {

    uint8_t i;

    for(i=0;i<NUM_TIMER0_COMPARE;i++) {
        
        if (NRF_TIMER0->EVENTS_COMPARE[i]) {
        
            NRF_TIMER0->EVENTS_COMPARE[i] = 0;
            if (timer_vars.cb[i]!=NULL) {
                timer_vars.cb[i]();
            }
        }
    }
    

}