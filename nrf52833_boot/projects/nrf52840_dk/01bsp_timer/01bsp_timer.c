/**
\brief This program shows the use of the "timer" bsp module.

Since the bsp modules for different platforms have the same declaration, you
can use this project with any platform.


\author Tengfei Chang <tengfeichang@hkust-gz.edu.cn>, Novermber 2023.
*/

#include "stdint.h"
#include "string.h"
#include "board.h"
#include "debugpins.h"
#include "leds.h"
#include "timer.h"

//=========================== defines =========================================

#define TIMER_PERIOD        16000000 // @16Mhz = 1s
#define TIMER_CAPTURE       

#define TIMER0_CC_SCHEDULE  0
#define TIMER0_CC_CAPTURE   1

#define HISTORY_LENGTH      16  // must be 2^x

//=========================== variables =======================================

typedef struct {
   uint32_t num_compare;

   uint8_t  last_value_index;
   uint32_t last_compare_value[HISTORY_LENGTH];
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void cb_compare(void);

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {  

    memset(&app_vars, 0, sizeof (app_vars_t));
   
    // initialize board. 
    board_init();

    timer_init();
    
    timer_set_callback(TIMER0_CC_SCHEDULE, cb_compare);
    timer_clear();
    timer_schedule(TIMER0_CC_SCHEDULE, TIMER_PERIOD);
    timer_start();
    
    while (1) {
        board_sleep();
    }
}

//=========================== callbacks =======================================

void cb_compare(void) {
   
   // toggle pin
   debugpins_frame_toggle();
   
   // toggle error led
   leds_error_toggle();

   timer_capture_now(TIMER0_CC_CAPTURE);
   app_vars.last_compare_value[app_vars.last_value_index++] = timer_getCapturedValue(TIMER0_CC_CAPTURE);
   app_vars.last_value_index &= (HISTORY_LENGTH-1);
   
   // increment counter
   app_vars.num_compare++;
   
   // schedule again
    timer_schedule(
        TIMER0_CC_SCHEDULE, 
        app_vars.last_compare_value[app_vars.last_value_index-1] + TIMER_PERIOD);
}
