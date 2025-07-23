/**
\brief This program shows the use of the "radio" bsp module.

Since the bsp modules for different platforms have the same declaration, you
can use this project with any platform.

After loading this program, your board will switch on its radio on frequency
CHANNEL.

While receiving a packet (i.e. from the start of frame event to the end of
frame event), it will turn on its sync LED.

Every TIMER_PERIOD, it will also send a packet containing LENGTH_PACKET bytes
set to ID. While sending a packet (i.e. from the start of frame event to the
end of frame event), it will turn on its error LED.

This program is edited from 01bsp_radio_ble_rx. This program implement a ble tx side with direction finding.

\author Tengfei Chang <tengfei.chang@inria.fr>, August 2020.
\author Manjiang Cao <mcao999@connect.hkust-gz.edu.cn>, June 2024.
*/

#include "board.h"
#include "radio.h"
#include "leds.h"
#include "sctimer.h"
#include "uart.h"

//=========================== defines =========================================

#define LENGTH_BLE_CRC  3
#define LENGTH_PACKET   125+LENGTH_BLE_CRC  ///< maximum length is 127 bytes
#define CHANNEL         0              ///< 0~39
#define TIMER_PERIOD    (0xffff>>2)     ///< 0xffff = 2s@32kHz
#define TXPOWER         0xD5            ///< 2's complement format, 0xD8 = -40dbm

#define LENGTH_SERIAL_FRAME  127              // length of the serial frame
#define UART_LENGTH     6

#define ENABLE_DF       0

uint16_t length = 0;

const static uint8_t ble_device_addr[6] = { 
    0xaa, 0xbb, 0xcc, 0xcc, 0xbb, 0xaa
};

// get from https://openuuid.net/signin/:  a24e7112-a03f-4623-bb56-ae67bd653c73
const static uint8_t ble_uuid[16]       = {
    0xa2, 0x4e, 0x71, 0x12, 0xa0, 0x3f, 
    0x46, 0x23, 0xbb, 0x56, 0xae, 0x67,
    0xbd, 0x65, 0x3c, 0x73
};

//=========================== variables =======================================

enum {
    APP_FLAG_START_FRAME = 0x01,
    APP_FLAG_END_FRAME   = 0x02,
    APP_FLAG_TIMER       = 0x04,
};

typedef enum {
    APP_STATE_TX         = 0x01,
    APP_STATE_RX         = 0x02,
} app_state_t;

typedef struct {
    uint8_t              num_startFrame;
    uint8_t              num_endFrame;
    uint8_t              num_timer;
} app_dbg_t;

app_dbg_t app_dbg;

typedef struct {
                uint8_t         flags;
                app_state_t     state;
                uint8_t         packet[LENGTH_PACKET];
                uint8_t         packet_len;
                
                uint8_t         uart_buffer_to_send[UART_LENGTH];
                uint16_t        uart_lastTxByteIndex;
     volatile   uint8_t         uartDone;

                uint8_t         rxpk_done;
                uint8_t         rxpk_buf[LENGTH_PACKET];
                uint8_t         rxpk_freq_offset;
                uint8_t         rxpk_len;
                uint8_t         rxpk_num;
                int8_t          rxpk_rssi;
                uint8_t         rxpk_lqi;
                bool            rxpk_crc;

                int8_t          estimate_angle;
                uint8_t         antenna_array_id;
                uint8_t         node_id;


                uint8_t         uart_txFrame[LENGTH_SERIAL_FRAME];
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void     cb_startFrame(PORT_TIMER_WIDTH timestamp);
void     cb_endFrame(PORT_TIMER_WIDTH timestamp);
void     cb_timer(void);

void     cb_uartTxDone(void);
uint8_t  cb_uartRxCb(void);

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {
    uint16_t i;

    uint8_t freq_offset;
    uint8_t sign;
    uint8_t read;

    // clear local variables
    memset(&app_vars,0,sizeof(app_vars_t));

    // initialize board
    board_init();
    radio_ble_init();

    // add callback functions radio
    radio_setStartFrameCb(cb_startFrame);
    radio_setEndFrameCb(cb_endFrame);

    // prepare packet
    app_vars.packet_len = sizeof(app_vars.packet);

    uart_setCallbacks(cb_uartTxDone,cb_uartRxCb);
    uart_enableInterrupts();

    // prepare radio
    radio_rfOn();
    // freq type only effects on scum port
    radio_ble_setFrequency(CHANNEL);

    radio_rxEnable();
    radio_rxNow();

    while (1){

        // sleep while waiting for at least one of the rxpk_done to be set

        app_vars.rxpk_done = 0;
        while (app_vars.rxpk_done==0) {
            board_sleep();
        }
        
        leds_error_toggle();
        memset(&app_vars,0,sizeof(app_vars_t));
        // done receiving a packet
        app_vars.packet_len = sizeof(app_vars.packet);
        
        radio_getReceivedFrame(
                            app_vars.packet,
                            &app_vars.packet_len,
                            sizeof(app_vars.packet),
                            &app_vars.rxpk_rssi,
                            &app_vars.rxpk_lqi,
                            &app_vars.rxpk_crc
                        );

        app_vars.node_id = app_vars.packet[31];
        app_vars.estimate_angle = app_vars.packet[32];
        app_vars.antenna_array_id = app_vars.packet[33];

        radio_rxEnable();
        radio_rxNow();
        
        uint8_t i;
        i = 0;
        app_vars.uart_buffer_to_send[i++] = app_vars.node_id;
        app_vars.uart_buffer_to_send[i++] = app_vars.estimate_angle;
        app_vars.uart_buffer_to_send[i++] = app_vars.antenna_array_id;
        app_vars.uart_buffer_to_send[i++] = 0xff;
        app_vars.uart_buffer_to_send[i++] = 0xff;
        app_vars.uart_buffer_to_send[i++] = 0xff;

        app_vars.uart_lastTxByteIndex = 0;
        uart_writeByte(app_vars.uart_buffer_to_send[0]);

        // led
        //leds_error_off();
    }
}
  

//=========================== private =========================================

//=========================== callbacks =======================================

void cb_startFrame(PORT_TIMER_WIDTH timestamp) {
    // set flag
    //app_vars.flags |= APP_FLAG_START_FRAME;

    //leds_sync_on();
    // update debug stats
    app_dbg.num_startFrame++;
}

void cb_endFrame(PORT_TIMER_WIDTH timestamp) {
    bool     expectedFrame;
    uint8_t  i;

    // set flag
    //app_vars.flags |= APP_FLAG_END_FRAME;

    // update debug stats
    app_dbg.num_endFrame++;
    app_vars.rxpk_done = 1;
}


void cb_uartTxDone(void) {

   app_vars.uart_lastTxByteIndex++;
   if (app_vars.uart_lastTxByteIndex<UART_LENGTH) {
      uart_writeByte(app_vars.uart_buffer_to_send[app_vars.uart_lastTxByteIndex]);
   } else {
      app_vars.uartDone = 1;
   }
}

uint8_t cb_uartRxCb(void) {
   uint8_t byte;
   
   // read received byte
   byte = uart_readByte();
   
   // echo that byte over serial
   uart_writeByte(byte);
   
   return 0;
}