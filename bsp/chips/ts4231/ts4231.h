/**
\brief registers address mapping of TS4231 sensor.

\based on author Tengfei Chang <tengfei.chang@gmail.com>, Nov 2021.
*/

#include "board_info.h"
#include "stdbool.h"
#include "stdint.h"

//=========================== define ==========================================

//---- register addresses
// This file adapts the pinout for the HiveTracker V1.0

#ifndef _TS4231_
#define _TS4231_

// E_Pin P(1,0)0.3
#define TS4231_N1_E_GPIO_PORT 0
#define TS4231_N1_E_GPIO_PIN 3
// D_Pin P(0,4)
#define TS4231_N1_D_GPIO_PORT 0
#define TS4231_N1_D_GPIO_PIN 4

#define TS4231_N1_E_PIN NRF_GPIO_PIN_MAP(TS4231_N1_E_GPIO_PORT, TS4231_N1_E_GPIO_PIN) // Clock signal pin P0.03
#define TS4231_N1_D_PIN NRF_GPIO_PIN_MAP(TS4231_N1_D_GPIO_PORT, TS4231_N1_D_GPIO_PIN) // Data signal pin P0.04
// for GPIO mode
#define MODE_INPUT 0
#define MODE_OUTPUT 1
// for GPIO output
#define OUTPUT_LOW 0
#define OUTPUT_HIGH 1

// if BUS_DRV_DLY = 2, it will as slow as arduino uno(ATMEL328P)
#define BUS_DRV_DLY 1      // delay in microseconds between bus level changes
#define BUS_CHECK_DLY 500  // delay in microseconds for the checkBus() function
#define SLEEP_RECOVERY 100 // delay in microseconds for analog wake-up after exiting SLEEP mode
#define UNKNOWN_STATE 0x04 // checkBus() function state
#define S3_STATE 0x03      // checkBus() function state
#define WATCH_STATE 0x02   // checkBus() function state
#define SLEEP_STATE 0x01   // checkBus() function state
#define S0_STATE 0x00      // checkBus() function state
#define CFG_WORD 0x392B    // configuration value
#define BUS_FAIL 0x01      // configDevice() function status return value
#define VERIFY_FAIL 0x02   // configDevice() function status return value
#define WATCH_FAIL 0x03    // configDevice() function status return value
#define CONFIG_PASS 0x04   // configDevice() function status return value

#endif

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

// admin

void ts4231_init(void);

bool ts4231_waitForLight(uint16_t light_timeout);
uint8_t ts4231_configDevice(void);
uint16_t ts4231_readConfig(void);
void ts4231_writeConfig(uint16_t config_val);

uint8_t ts4231_checkBus(void);
bool ts4231_goToSleep(void);
bool ts4231_goToWatch(void);

void ts4231_pinMode(uint32_t pin_number, uint8_t mode);
uint32_t ts4231_digitalRead(uint32_t pin_number);
void ts4231_digitalWrite(uint32_t pin_number, uint8_t output_mode);

void delay_us(int16_t times);
void delay_ms(int16_t times);

// read a pin
uint32_t ts_nrf_gpio_read_input(uint32_t pin_number);

// config gpio in/out
void ts_nrf_gpio_cfg_input(uint32_t pin_number);
void ts_nrf_gpio_cfg_output(uint32_t pin_number);

//=========================== private =========================================