/**
\brief registers address mapping of TS4231 sensor.

\author Tengfei Chang <tengfei.chang@gmail.com>, Nov 2021.
*/

#include "stdint.h"
#include "stdbool.h"

//=========================== define ==========================================

#define BMX160_ADDR 0x68

//---- register addresses
// This file adapts the pinout for the HiveTracker V1.0

#ifndef _TS4231_
#define _TS4231_

#define BUS_DRV_DLY     1       //delay in microseconds between bus level changes
#define BUS_CHECK_DLY   500     //delay in microseconds for the checkBus() function
#define SLEEP_RECOVERY  100     //delay in microseconds for analog wake-up after exiting SLEEP mode
#define UNKNOWN_STATE   0x04    //checkBus() function state
#define S3_STATE        0x03    //checkBus() function state
#define WATCH_STATE     0x02    //checkBus() function state
#define SLEEP_STATE     0x01    //checkBus() function state
#define S0_STATE        0x00    //checkBus() function state
#define CFG_WORD        0x392B  //configuration value
#define BUS_FAIL        0x01    //configDevice() function status return value
#define VERIFY_FAIL     0x02    //configDevice() function status return value
#define WATCH_FAIL      0x03    //configDevice() function status return value
#define CONFIG_PASS     0x04    //configDevice() function status return value


//// Serial
//#undef PIN_SERIAL_RX
//#undef PIN_SERIAL_TX
//#define PIN_SERIAL_RX        (25) /* J4 - E */
//#define PIN_SERIAL_TX        (27) /* J4 - D */

//// I2C
//#undef PIN_WIRE_SDA
//#undef PIN_WIRE_SCL
//#define PIN_WIRE_SDA         (29)
//#define PIN_WIRE_SCL         (28)

// Photodiodes
//const int sensors_num = 1; // 0,  1,  2,  3
//const int sensors_E[] =     {22, 0, 0, 0}; // Envelope line
//const int sensors_D[] =     {24, 0, 0, 0}; // Data line - not used for now


#endif









//#define BMX160_REG_ADDR_CHIPID      0x00


// sensor data

//#define BMX160_REG_ADDR_DATA        0x04


//---- register values

//#define BMX160_CMD_PMU_ACC_SUSPEND        0x10


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

//config gpio to input
void ts_nrf_gpio_cfg_input(uint32_t pin_number);
//read a pin
uint32_t ts_nrf_gpio_read_input(uint32_t pin_number);

//uint8_t bmx160_who_am_i(void);
//uint8_t bmx160_power_down(void);
//uint8_t bmx160_get_pmu_status(void);

//void    bmx160_set_cmd(uint8_t cmd);
//void    bmx160_acc_config(uint8_t config);
//void    bmx160_gyr_config(uint8_t config);
//void    bmx160_mag_config(uint8_t config);

//void    bmx160_acc_range(uint8_t range);
//void    bmx160_gyr_range(uint8_t range);
//void    bmx160_mag_if(uint8_t interface);

// read

//config gpio in/out
void ts_nrf_gpio_cfg_input(uint32_t pin_number);
void ts_nrf_gpio_cfg_output(uint32_t pin_number);
//read a pin
uint32_t ts_nrf_gpio_read_input(uint32_t pin_number);


//void ts4231_pinMode(int pin, uint8_t mode);
//int16_t bmx160_read_temperature(void);

//int16_t bmx160_read_acc_x(void);
//int16_t bmx160_read_acc_y(void);
//int16_t bmx160_read_acc_z(void);

//int16_t bmx160_read_mag_x(void);
//int16_t bmx160_read_mag_y(void);
//int16_t bmx160_read_mag_z(void);

//int16_t bmx160_read_gyr_x(void);
//int16_t bmx160_read_gyr_y(void);
//int16_t bmx160_read_gyr_z(void);

//void    bmx160_read_9dof_data(void);

//=========================== private =========================================