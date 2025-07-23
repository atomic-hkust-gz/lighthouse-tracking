/**
\brief This program shows the use of the "PPI" bsp module.

Since the bsp modules for different platforms have the same declaration, you
can use this project with any platform.

\author Tengfei Chang <tengfeichang@hkust-gz.edu.cn>, Novermber 2023.
*/

#include "stdint.h"
#include "string.h"
#include "board.h"
#include "ppi.h"
#include "sctimer.h"
#include "timer.h"
#include "leds.h"

//=========================== defines =========================================

#define NUM_PPI_CHANNEL_USED  4

#define SCTIMER_PERIOD     32768 // @32kHz = 1s

//=========================== variables =======================================

typedef struct {
    uint16_t num_compare;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================


void cb_compare(void);

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {  

    uint8_t i;

    memset(&app_vars, 0, sizeof (app_vars_t));
   
    // initialize board. 
    board_init();

    sctimer_set_callback(cb_compare);
    sctimer_setCompare(sctimer_readCounter()+SCTIMER_PERIOD);

    timer_init();

    for (i=0;i<NUM_PPI_CHANNEL_USED;i++) {
        ppi_enable(i);
    }

    ppi_rtc_schedule_timer_capture(3, 0, 1);

    while (1) {
        board_sleep();
    }
}

//=========================== callbacks =======================================

void cb_compare(void) {
   
   // toggle error led
   leds_error_toggle();
   
   // increment counter
   app_vars.num_compare++;
   
   // schedule again
   sctimer_setCompare(sctimer_readCounter()+SCTIMER_PERIOD);

}
