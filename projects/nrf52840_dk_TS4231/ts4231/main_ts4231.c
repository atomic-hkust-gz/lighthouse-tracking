/**
 * brief ts4231 configuration and localization implementation.
 * This code implements the use of TS4231 and HTC lighthouse v1 for localization.
 *
 * \author Cheng Wang <cwang199@connect.hkust-gz.edu.cn>, Dec 2023.
 * \Reference to open source project: https://github.com/HiveTracker/firmware
 * \Reference to QingFeng BBS: http://www.qfv8.com/
 *
 * This file is also reference to OpenWSN bmx160 driver: https://github.com/atomic-hkust-gz/openwsn-fw,
 * \author Tengfei Chang <tengfei.chang@gmail.com>, Nov 2021
 */

#include "board.h"
#include "i2c.h"
#include "leds.h"
#include "ppi_gpiote.h"
#include "sctimer.h"
#include "stdint.h"
#include "stdio.h"
#include "string.h"
#include "timer_ts4231.h"
#include "ts4231.h"
#include "uart.h"

//=========================== defines =========================================

#define SAMPLE_PERIOD (32768 >> 4) // @32kHz = 1s
#define BUFFER_SIZE 0x08           // 2B*3 axises value + 2B ending with '\r\n'

//=========================== variables =======================================

typedef struct {
  uint16_t num_compare;
  bool sampling_now;
  uint8_t axes[6];
  float axis_x;
  float axis_y;
  float axis_z;

  uint8_t who_am_i;
  int16_t temp;
  float temp_f;

  // uart specific
  uint8_t uart_lastTxByteIndex;
  volatile uint8_t uartDone;
  volatile uint8_t uartSendNow;
  volatile uint8_t uartToSend[BUFFER_SIZE];
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void cb_compare(void);
void cb_uartTxDone(void);
uint8_t cb_uartRxCb(void);

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {

  int16_t tmp;
  uint8_t i;

  // initialize board.
  board_init();

  // setup UART
  uart_setCallbacks(cb_uartTxDone, cb_uartRxCb);
  uart_enableInterrupts();

  sctimer_set_callback(cb_compare);
  // sctimer_setCompare(sctimer_readCounter()+SAMPLE_PERIOD);

  // TS4231 init and configure
  ts4231_init();
  delay_ms(10);
  // timer,gpiote and ppi must be configured after the ts4231 has been initialized,
  // otherwise it will have an impact on them.
  TIMER_init();
  gpiote_init();
  ppi_init();

  // wait sensor change to watch satate
  delay_ms(10);

  while (1) {

    //
  }
}

//=========================== callbacks =======================================

void cb_compare(void) {

  // have main "task" send over UART
  app_vars.uartSendNow = 1;

  // schedule again
  sctimer_setCompare(sctimer_readCounter() + SAMPLE_PERIOD);
}

void cb_uartTxDone(void) {

  app_vars.uart_lastTxByteIndex++;
  if (app_vars.uart_lastTxByteIndex < sizeof(app_vars.uartToSend)) {
    uart_writeByte(app_vars.uartToSend[app_vars.uart_lastTxByteIndex]);
  } else {
    app_vars.uartDone = 1;
  }
}

uint8_t cb_uartRxCb(void) {

  uint8_t byte;

  // toggle LED
  leds_error_toggle();

  // read received byte
  byte = uart_readByte();

  // echo that byte over serial
  uart_writeByte(byte);

  return 0;
}