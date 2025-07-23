/**
\brief This program implement RTT on nrf52840.

\author Manjiang Cao <mcao999@connect.hkust-gz.edu.cn>, Novermber 2023.
*/

#include "stdint.h"
#include "string.h"
#include "board.h"
#include "ppi.h"
#include "sctimer.h"
#include "timer.h"
#include "leds.h"
#include "eui64.h"
#include "radio.h"
#include "uart.h"
#include "nrf52840.h"

#include "bmx160.h"
#include "i2c.h"
#include "stdio.h"
//=========================== defines =========================================

#define NUM_PPI_CHANNEL_USED  4

//#define TIMER_PERIOD          (0xffff>>4)    ///< 0xffff = 2s@32kHz
#define TIMER_PERIOD          0xA4    ///< 0xffff = 2s@32kHz

#define SENDER_ID_LAST_BYTE         0xad            //board id ---707
#define RECEIVER_ONE_ID_LAST_BYTE   0xac            //board id ---014
#define RECEIVER_TWO_ID_LAST_BYTE   0x01            //board id ---376
#define RECEIVER_THREE_ID_LAST_BYTE 0x9e          //board id ---265

#define RECEIVER_NUM          3                   // the number of receiver

#define EOF_CAPTURE_ID        0               //EOF_CAPTURE_ID using for EoF capture into EOF_CAPTURE_ID
#define SOF_CAPTURE_ID        1               //SOF_CAPTURE_ID using for SoF capture into SOF_CAPTURE_ID

//-------radio define-------
#define RECEIVER_ONE_CHANNEL                  14    //channel = anchor number + 13
#define RECEIVER_TWO_CHANNEL                  15
#define RECEIVER_THREE_CHANNEL                16
#define LENGTH_PACKET         20+LENGTH_CRC   //maximum length is 127 bytes
#define LEN_PKT_TO_SEND       20+LENGTH_CRC

//-------PPI define-------
#define EOF_CAPTURE_PPI_ID                   0  //EOF_CAPTURE_PPI_ID using for Radio EoF -> Timer Capture  app_vars.t1 into EOF_CAPTURE_ID
#define SOF_CAPTURE_PPI_ID                   1  //SOF_CAPTURE_PPI_ID using for Radio SoF -> Timer Capture  app_vars.t2 into SOF_CAPTURE_ID
#define TIMER_COMPARE_RADIO_PPI_ID           2  //TIMER_COMPARE_RADIO_PPI_ID using for Timer schedule -> Radio TX 

//------- timer define -------
#define TIMER_COMPARE_ID      0
#define RESPONSE_DELAY        16000           // 16000@16MHz = 1ms

//------

#define NUM_SAMPLES           64
#define NUM_SAMPLE_MASK       0x3f

//=========================== variables =======================================
//uint8_t stringToSend[]  = "+002 Ptest.24.00.12.-010\n";

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
    
    uint8_t              num_rx_startFrame;
    uint8_t              num_rx_endFrame;
} app_dbg_t;

app_dbg_t app_dbg;

typedef struct {
    uint8_t              txpk_buf[LENGTH_PACKET];
    uint8_t              txpk_len;
    //uint8_t              stringtosend[6];
    uint8_t              receiver_id;

    uint8_t              packet[LENGTH_PACKET];
    uint8_t              packet_len;
    int8_t               rxpk_rssi;
    uint8_t              rxpk_lqi;
    bool                 rxpk_crc;
                  
                  uint8_t         flags;
                  app_state_t     state;
                  uint8_t uart_lastTxByteIndex;
   volatile       uint8_t uartDone;
   volatile       uint8_t uartSendNow;
   volatile       uint8_t uartToSend[10];

                  uint32_t       t1;
                  uint32_t       t2;

                  uint16_t       diff_index;
                  uint32_t       diff[NUM_SAMPLES];

} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

int sender_main(void);
int receiver_main(uint8_t);

void cb_endFrame(PORT_TIMER_WIDTH timestamp);
void cb_startFrame(PORT_TIMER_WIDTH timestamp);

void     cb_uart_tx_done(void);
uint8_t  cb_uart_rx(void);

void cb_timer(void);

void delay_ms(int16_t times);


//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {
    uint8_t board_eui[8];
    eui64_get(board_eui);
    
    uint8_t anchor_number;
    anchor_number = 1;
    while(1) {

        if (board_eui[7] == SENDER_ID_LAST_BYTE){
            sender_main();
        //when downloading, change the receiver last byte
        } else if (board_eui[7] == RECEIVER_ONE_ID_LAST_BYTE){
            receiver_main(anchor_number);
        }
    }  
}

//=========================== callbacks =======================================

int sender_main(void){


    uint32_t tmp;
    uint16_t i;
    
    uint8_t sign;
    uint8_t read;
    
    uint8_t receiver_id;

    board_init();
    timer_init();

    //config PPI

    //using PPI channel 0, EoF capture time value to EOF_CAPTURE_ID
    ppi_radio_eof_timer_capture(EOF_CAPTURE_PPI_ID, EOF_CAPTURE_ID);    
    ppi_enable(EOF_CAPTURE_PPI_ID);

    //using PPI channel 1, SoF capture time value to SOF_CAPTURE_ID  
    ppi_radio_sof_timer_capture(SOF_CAPTURE_PPI_ID, SOF_CAPTURE_ID);     
    ppi_enable(SOF_CAPTURE_PPI_ID);
    
    timer_clear();
    timer_start();

    // setup UART
    uart_setCallbacks(cb_uart_tx_done,cb_uart_rx);
    uart_enableInterrupts();

    app_vars.uartDone = 1;

    app_vars.receiver_id = 1; //start from first receiver
    

    radio_setStartFrameCb(cb_startFrame);
    radio_setEndFrameCb(cb_endFrame);

    // start bsp timer
    sctimer_set_callback(cb_timer);
    sctimer_setCompare(sctimer_readCounter()+TIMER_PERIOD);
    sctimer_enable();

    // prepare radio
    radio_rfOn();

    // freq type only effects on scum port, start from channel 14
    radio_setFrequency(RECEIVER_ONE_CHANNEL, FREQ_RX);
    
    radio_rxEnable();
    
    app_vars.state = APP_STATE_RX;
    
    // start by a transmit
    app_vars.flags |= APP_FLAG_TIMER;

    while(1) {

        while(app_vars.flags==0x00) {
            board_sleep();
        }

        while (app_vars.flags) {
            
            //===========  APP_FLAG_START_FRAME (TX or RX)
            if (app_vars.flags & APP_FLAG_START_FRAME) {
                // start of frame

                switch (app_vars.state) {
                    case APP_STATE_RX:
                        // started receiving a packet
                        app_vars.t2 = timer_getCapturedValue(SOF_CAPTURE_ID);
                        // led
                        leds_error_on();
                        break;
                    case APP_STATE_TX:
                        // started sending a packet

                        // led
                        leds_sync_on();
                    break;
                }

                // clear flag
                app_vars.flags &= ~APP_FLAG_START_FRAME;
            }

            //==== APP_FLAG_END_FRAME (TX or RX)
            if (app_vars.flags & APP_FLAG_END_FRAME) {
                //end of frame

                switch (app_vars.state) {
                    case APP_STATE_RX:
                        
                        // done receiving a packet
                        radio_getReceivedFrame(
                                                app_vars.packet,
                                                &app_vars.packet_len,
                                                sizeof(app_vars.packet),
                                                &app_vars.rxpk_rssi,
                                                &app_vars.rxpk_lqi,
                                                &app_vars.rxpk_crc
                                            );

                        tmp = app_vars.t2 - app_vars.t1;
                        app_vars.diff[app_vars.diff_index++&NUM_SAMPLE_MASK] = tmp;
                        
                        if (app_vars.packet[0] == app_vars.receiver_id){

                            uint8_t i = 0;
                            
                            app_vars.uartToSend[i++] = app_vars.receiver_id;
                            //send rtt time 
                            app_vars.uartToSend[i++] = (uint8_t)((tmp >> 24) & 0x000000ff);
                            app_vars.uartToSend[i++] = (uint8_t)((tmp >> 16) & 0x000000ff);
                            app_vars.uartToSend[i++] = (uint8_t)((tmp >> 8) & 0x000000ff);
                            app_vars.uartToSend[i++] = (uint8_t)((tmp >> 0) & 0x000000ff);
                            //send RSSI for response packet
                            sign = (app_vars.rxpk_rssi & 0x80) >> 7;
                            if (sign){
                                read = 0xff - (uint8_t)(app_vars.rxpk_rssi) + 1;
                            } else {
                                read = app_vars.rxpk_rssi;
                            }

                            if (sign) {
                                app_vars.uartToSend[i++] = '-';
                            } else {
                                app_vars.uartToSend[i++] = '+';
                            }
                            app_vars.uartToSend[i++] = '0'+read/100;
                            app_vars.uartToSend[i++] = '0'+read/10;
                            app_vars.uartToSend[i++] = '0'+read%10;
                            //get RSSI of request packet from response packet and send 
                            app_vars.uartToSend[i++] = app_vars.packet[1];
                            app_vars.uartToSend[i++] = app_vars.packet[2];
                            app_vars.uartToSend[i++] = app_vars.packet[3];
                            app_vars.uartToSend[i++] = app_vars.packet[4];

                            app_vars.uartToSend[i++] = '\r';
                            app_vars.uartToSend[i++] = '\n';

                            //uart_writeByte(app_vars.uartToSend[6]);

                            // send string over UART
                            if (app_vars.uartDone == 1) {
                                app_vars.uartDone              = 0;
                                app_vars.uart_lastTxByteIndex  = 0;
                                uart_writeByte(app_vars.uartToSend[app_vars.uart_lastTxByteIndex]);
                            }

                            leds_error_off();

                            app_vars.receiver_id = app_vars.receiver_id + 1;
                            if (app_vars.receiver_id > RECEIVER_NUM){
                                app_vars.receiver_id = 1;
                            }
                            radio_setFrequency(app_vars.receiver_id + 13, FREQ_RX);
                           
                        }
                        
                        break;
                    
                    case APP_STATE_TX:
                        // done sending a packet

                        // switch to RX mode
                        radio_rxEnable();
                        radio_rxNow();
                        app_vars.state = APP_STATE_RX;

                        app_vars.t1 = timer_getCapturedValue(EOF_CAPTURE_ID);

                        // led
                        leds_sync_off();
                        break;
                }
                // clear flag
                app_vars.flags &= ~APP_FLAG_END_FRAME;
            }

            //==== APP_FLAG_TIMER

            if (app_vars.flags & APP_FLAG_TIMER){
                // timer fired

                if (app_vars.state==APP_STATE_RX){
                    // stop listening
                    radio_rfOff();

                    // prepare packet
                    app_vars.txpk_buf[0] = app_vars.receiver_id; // Packet number or other data
                    app_vars.txpk_len = sizeof(app_vars.txpk_buf);

                    // send packet
                    radio_loadPacket(app_vars.txpk_buf, app_vars.txpk_len);
                    radio_txEnable();
                    radio_txNow();
                    app_vars.state = APP_STATE_TX;
                }

                app_vars.flags &= ~APP_FLAG_TIMER;
            }
        }
    }
}


int receiver_main(uint8_t anchor_number){
    board_init();
    timer_init();

    uint8_t sign;
    uint8_t read;
    
    //config PPI

    //using PPI channel 0, EoF capture time value to EOF_CAPTURE_ID    app_vars.t1
    ppi_radio_eof_timer_capture(EOF_CAPTURE_PPI_ID, EOF_CAPTURE_ID);    
    ppi_enable(EOF_CAPTURE_PPI_ID);

    ppi_timer_compare_radio_start(TIMER_COMPARE_RADIO_PPI_ID, TIMER_COMPARE_ID);
    ppi_enable(TIMER_COMPARE_RADIO_PPI_ID);

    timer_clear();
    timer_start();

    // prepare radio
    radio_rfOn();
    // freq type only effects on scum port
    radio_setFrequency(anchor_number+13, FREQ_RX);

    // prepare packet
    app_vars.txpk_buf[0] = anchor_number; // Packet number or other data
    app_vars.txpk_len = sizeof(app_vars.txpk_buf);

    radio_setStartFrameCb(cb_startFrame);
    radio_setEndFrameCb(cb_endFrame);

    radio_rxEnable();
    radio_rxNow();

    app_vars.state = APP_STATE_RX;
    
    while(1) {
        while(app_vars.flags == 0x00) {
            board_sleep();
        }

        while(app_vars.flags) {
            
            //==== APP_FLAG_START_FRAME (TX or RX)

            if (app_vars.flags & APP_FLAG_START_FRAME) {
                //start of frame
                switch (app_vars.state) {
                    case APP_STATE_RX:

                        // led
                        leds_error_on();
                        break;
                    case APP_STATE_TX:
                        // started sending a packet
    
                        // led
                        leds_sync_on();
                    break;
                }
                // clear flag
                app_vars.flags &= ~APP_FLAG_START_FRAME;
            }
            
            //==== APP_FLAG_END_FRAME (TX or RX)

            if (app_vars.flags & APP_FLAG_END_FRAME) {
                //end of frame
                switch (app_vars.state) {
                    case APP_STATE_RX:
                        
                        //done receiving a packet
                        radio_getReceivedFrame(
                                    app_vars.packet,
                                    &app_vars.packet_len,
                                    sizeof(app_vars.packet),
                                    &app_vars.rxpk_rssi,
                                    &app_vars.rxpk_lqi,
                                    &app_vars.rxpk_crc
                                );
                        
                        if (app_vars.packet[0] == app_vars.txpk_buf[0]){
                            timer_schedule(TIMER_COMPARE_ID, timer_getCapturedValue(EOF_CAPTURE_ID)+RESPONSE_DELAY);                        
                            
                            sign = (app_vars.rxpk_rssi & 0x80) >> 7;
                            if (sign){
                                read = 0xff - (uint8_t)(app_vars.rxpk_rssi) + 1;
                            } else {
                                read = app_vars.rxpk_rssi;
                            }
                            // the first byte of txpk_buf used to tell target that which anchor is responsing

                            
                            uint8_t i = 1;
                            if (sign) {
                                app_vars.txpk_buf[i++] = '-';
                            } else {
                                app_vars.txpk_buf[i++] = '+';
                            }
                            app_vars.txpk_buf[i++] = '0'+read/100;
                            app_vars.txpk_buf[i++] = '0'+read/10;
                            app_vars.txpk_buf[i++] = '0'+read%10;
                            
                            
                            app_vars.txpk_len = sizeof(app_vars.txpk_buf);

                            radio_loadPacket(app_vars.txpk_buf, app_vars.txpk_len);
                            radio_txEnable();
                        
                            app_vars.state = APP_STATE_TX;
                            // led
                            leds_error_off();
                            //memset(&app_vars.packet,0,sizeof(app_vars.packet));
                        } else {
                            radio_txEnable();
                            radio_txNow();
                           app_vars.state = APP_STATE_TX;
                        }
                        break;

                    case APP_STATE_TX:
                        // done sending a packet

                        // switch to RX mode
                        radio_rxEnable();
                        radio_rxNow();
                        app_vars.state = APP_STATE_RX;

                        // led
                        leds_sync_off();
                        break;
                }
                // clear flag
                app_vars.flags &= ~APP_FLAG_END_FRAME;
            }
        }
    }
}

//=================callbacks function=====================
void cb_endFrame(PORT_TIMER_WIDTH timestamp) {

    // set flag
    app_vars.flags |= APP_FLAG_END_FRAME;

    // update debug stats
    app_dbg.num_endFrame++;

    if (app_vars.state == APP_STATE_RX) {
        app_dbg.num_rx_endFrame++;
    }
}

void cb_startFrame(PORT_TIMER_WIDTH timestamp) {
    // set flag
    app_vars.flags |= APP_FLAG_START_FRAME;
    
    // update debug stats
    app_dbg.num_startFrame++;

    if (app_vars.state == APP_STATE_RX) {
        app_dbg.num_rx_startFrame++;
    }
}

void cb_uart_tx_done(void) {
    app_vars.uart_lastTxByteIndex++;
    if (app_vars.uart_lastTxByteIndex<15) {
        uart_writeByte(app_vars.uartToSend[app_vars.uart_lastTxByteIndex]);
    } else {
        app_vars.uartDone = 1;
    }
}

uint8_t cb_uart_rx(void) {
    uint8_t byte;

    // toggle LED
    leds_error_toggle();

    // read received byte
    byte = uart_readByte();

    // echo that byte over serial
    uart_writeByte(byte);

    return 0;
}

void cb_timer(void) {
    // set flag
    app_vars.flags |= APP_FLAG_TIMER;

    // update debug stats
    app_dbg.num_timer++;

    sctimer_setCompare(sctimer_readCounter()+TIMER_PERIOD);
}
