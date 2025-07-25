/**
\brief This program shows the use of the "radio" bsp module.
*/

#include "stdint.h"
#include "string.h"
#include "board.h"
#include "radio.h"
#include "radio_df.h"
#include "leds.h"
#include "uart.h"
#include "sctimer.h"
#include "math.h"
#include "debugpins.h"

//=========================== defines =========================================

#define NUM_SAMPLES           SAMPLE_MAXCNT
#define NUM_SAMPLES_REFERENCE 64
#define LENGTH_PACKET         125+LENGTH_CRC // maximum length is 127 bytes
#define CHANNEL               0             // 24ghz: 11 = 2.405GHz, subghz: 11 = 865.325 in  FSK operating mode #1
#define FREQUENCY             
#define LENGTH_SERIAL_FRAME   ((NUM_SAMPLES*4)+10)   // length of the serial frame

#define DF_ENABLE             1
#define CALCULATE_ON_BOARD    1
#define INVAILD_ANGLE         361.0

#define ANGLE_HISTORY_LEN     8
#define INDEX_MASK            0x07

#define UART_DEBUG            1


//=========================== variables =======================================

typedef struct {
    uint8_t    num_compared;
} app_dbg_t;

app_dbg_t app_dbg;

//=========================== prototypes ======================================

void     cb_timer(void);

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {

    board_init();

    sctimer_set_callback(0,cb_timer);
    sctimer_setCompare(0,sctimer_readCounter() + (32768/200)*20);
    
    clocks_start();
    clocks_stop();


    while(1) {
        __WFE();
        __WFE();
    }
}

//=========================== private =========================================


//=========================== callbacks =======================================

void cb_timer() {
    leds_error_toggle();
    app_dbg.num_compared++;
    sctimer_setCompare(0,sctimer_readCounter() + (32768/200)*20);
}
