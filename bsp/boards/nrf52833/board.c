/**
\brief nRF52840-specific definition of the "board" bsp module.

\author Tengfei Chang <tengfei.chang@gmail.com>, July 2020.
*/

#include "nrf52833.h"
#include "board.h"
// bsp modules
#include "debugpins.h"
#include "leds.h"
#include "uart.h"
#include "sctimer.h"
#include "radio.h"
#include "i2c.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

void clocks_start(void);
void enable_dcdc(void);

//=========================== main ============================================

extern int mote_main(void);

int main(void) {
   return mote_main();
}

//=========================== public ==========================================

void board_init(void) {

    clocks_start();

    // initialize bsp modules
    debugpins_init();
    leds_init();
    uart_init();
    radio_init();
    sctimer_init();
    i2c_init();

    enable_dcdc();
    
}

void board_sleep(void) {
    __WFE();
    __WFE();
}

void board_reset(void) {

    NVIC_SystemReset();
}

//=========================== private =========================================

void clocks_start( void ){

    // Start HFCLK and wait for it to start.
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART = 1;
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0);
}


void enable_dcdc(void) {

    uint32_t status; 

    status = NRF_POWER->MAINREGSTATUS;

    if (status == 0) {

        while (NRF_POWER->DCDCEN == 0){
            // in normal voltage mode: PS1.2, page 59
            NRF_POWER->DCDCEN = (uint32_t)1;
        }
    }
}