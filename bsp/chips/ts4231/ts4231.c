/**
\brief TS4231 driver.
\based on Official lib at https://github.com/TriadSemi/TS4231
\If you want to use different pins, please change these define:
\E_Pin P(0.3)
#define TS4231_N1_E_GPIO_PORT 0
#define TS4231_N1_E_GPIO_PIN 3
\D_Pin P(0,4)
#define TS4231_N1_D_GPIO_PORT 0
#define TS4231_N1_D_GPIO_PIN 4
\author Cheng Wang <cwang199@connect.hkust-gz.edu.cn>, Nov 2023.
\This file is also based on bmx160 driver, author Tengfei Chang <tengfei.chang@gmail.com>, Nov 2021.
*/

#include "ts4231.h"
#include "SEGGER_RTT.h"
#include "nrf52840.h"
#include "nrf52840_bitfields.h"
#include "stdbool.h"

//=========================== define ==========================================

typedef struct
{

  uint32_t E_pin;
  uint32_t D_pin;

  bool configured;

} TS4231_t;

TS4231_t ts4231_var = {
    .E_pin = TS4231_N1_E_PIN,
    .D_pin = TS4231_N1_D_PIN,
    .configured = false};

// wait 500ms(for 32768tick = 1s configured in sctimer)
uint16_t light_timeout = 32768 / 2;
bool Is_lighthouse = false;
uint8_t Config_result = 0x66;
uint8_t Origin_state = 0x66;
uint8_t Current_state = 0x66;
uint32_t temp_in = 0x01;

// may replace with RTC in future
void delay_ms(int16_t times) {
  // Calculate the number of clock cycles for the delay
  uint32_t start = DWT->CYCCNT;
  uint32_t cycles = times * (SystemCoreClock / 1000);
  // Using the DWT register for time delay
  while ((DWT->CYCCNT - start) < cycles) {
  }
}

// may replace with RTC in future
void delay_us(int16_t times) {
  // Calculate the number of clock cycles for the delay
  uint32_t start = DWT->CYCCNT;
  uint32_t cycles = times * (SystemCoreClock / 1000000);
  // Using the DWT register for time delay
  while ((DWT->CYCCNT - start) < cycles) {
  }
}

int8_t repeat_config = 0;
// use this function in mote_main() to initialize sensor
void ts4231_init() {
  ts_nrf_gpio_cfg_input(TS4231_N1_E_PIN);
  ts_nrf_gpio_cfg_input(TS4231_N1_D_PIN);

  Is_lighthouse = ts4231_waitForLight(light_timeout);
  if (Is_lighthouse == 0) {
    SEGGER_RTT_WriteString(0, "NO lighthouse detected\n");

  } else {
    // this is a segger RTT virtual uart
    SEGGER_RTT_WriteString(0, "lighthouse detected\n");
    Origin_state = ts4231_checkBus();

    // ts4231_goToWatch();
    Current_state = ts4231_checkBus();

    if (Current_state == WATCH_STATE) {
      SEGGER_RTT_WriteString(0, "already configured\n");
    } else {
      Config_result = ts4231_configDevice();
      // it is easy to get a WATCH_FAIL, repeat the config process until successful
      while ((Current_state != WATCH_STATE) && (Config_result != CONFIG_PASS) && (repeat_config < 10)) {

        switch (Config_result) {
        case CONFIG_PASS:
          SEGGER_RTT_WriteString(0, "config success\n");
          break;
        case BUS_FAIL:
          SEGGER_RTT_WriteString(0, "error:BUS_FAIL\n");
          break;
        case VERIFY_FAIL:
          SEGGER_RTT_WriteString(0, "error:VERIFY_FAIL\n");
          break;
        case WATCH_FAIL:
          SEGGER_RTT_WriteString(0, "error:WATCH_FAIL\n");
          break;
        default:
          SEGGER_RTT_WriteString(0, "Unknown:Program Execution ERROR\n");
          break;
        }
        repeat_config++;
        Config_result = ts4231_configDevice();

        ts4231_goToWatch();
        Current_state = ts4231_checkBus();
        if (Current_state == WATCH_STATE) {
          SEGGER_RTT_WriteString(0, "config success\n");
        }
        delay_ms(5);
      }
    }
  }
  // pulseSetup();
}

// essential step, when powering on the sensor.
bool ts4231_waitForLight(uint16_t light_timeout) {
  bool light = false;
  bool exit = false;
  unsigned long time0;

  if (ts4231_checkBus() == S0_STATE) {
    time0 = NRF_RTC0->COUNTER;

    while (exit == false) {
      if (ts_nrf_gpio_read_input(TS4231_N1_D_PIN) > 0) {
        while (exit == false) {
          if (ts_nrf_gpio_read_input(TS4231_N1_D_PIN) == 0) {
            exit = true;
            light = true;
          } else if ((NRF_RTC0->COUNTER) > (time0 + light_timeout)) {
            exit = true;
            light = false;
          } else {
            exit = false;
            light = false;
          }
        }
      } else if ((NRF_RTC0->COUNTER) > (time0 + light_timeout)) {
        exit = true;
        light = false;
      } else {
        exit = false;
        light = false;
      }
    }
  } else
    light = true; // if not in state S0_state, light has already been detected
  return light;
}

// uint16_t watch_readback = 0x00;
uint8_t Chip_state = 0x99;

uint8_t ts4231_configDevice() {
  uint16_t config_val = CFG_WORD;
  uint8_t config_success = 0x00;
  uint16_t readback;

  ts4231_var.configured = false;
  ts4231_pinMode(TS4231_N1_D_PIN, MODE_INPUT);
  ts4231_pinMode(TS4231_N1_E_PIN, MODE_INPUT);
  ts4231_digitalWrite(TS4231_N1_D_PIN, OUTPUT_LOW);
  ts4231_digitalWrite(TS4231_N1_E_PIN, OUTPUT_LOW);
  ts4231_pinMode(TS4231_N1_E_PIN, MODE_OUTPUT);
  delay_us(BUS_DRV_DLY);
  ts4231_digitalWrite(TS4231_N1_E_PIN, OUTPUT_HIGH);
  delay_us(BUS_DRV_DLY);
  ts4231_digitalWrite(TS4231_N1_E_PIN, OUTPUT_LOW);
  delay_us(BUS_DRV_DLY);
  ts4231_digitalWrite(TS4231_N1_E_PIN, OUTPUT_HIGH);
  delay_us(BUS_DRV_DLY);
  ts4231_pinMode(TS4231_N1_D_PIN, MODE_OUTPUT);
  delay_us(BUS_DRV_DLY);
  ts4231_digitalWrite(TS4231_N1_D_PIN, OUTPUT_HIGH);
  delay_us(BUS_DRV_DLY);
  ts4231_pinMode(TS4231_N1_E_PIN, MODE_INPUT);
  ts4231_pinMode(TS4231_N1_D_PIN, MODE_INPUT);
  Chip_state = ts4231_checkBus();
  if (Chip_state == S3_STATE) {
    ts4231_writeConfig(config_val);
    readback = ts4231_readConfig();
    // watch_readback = readback;
    if (readback == config_val) {
      // readback == config_val
      ts4231_var.configured = true;
      if (ts4231_goToWatch()) {
        config_success = CONFIG_PASS;
      } else
        config_success = WATCH_FAIL;
    } else
      config_success = VERIFY_FAIL;
  } else
    config_success = BUS_FAIL;

  return config_success;
}

bool ts4231_goToSleep() {

  bool sleep_success;

  if (ts4231_var.configured == false)
    sleep_success = false;
  else {
    switch (ts4231_checkBus()) {
    case S0_STATE:
      sleep_success = false;
      break;
    case SLEEP_STATE:
      sleep_success = true;
      break;
    case WATCH_STATE:
      ts4231_digitalWrite(TS4231_N1_E_PIN, OUTPUT_LOW);
      ts4231_pinMode(TS4231_N1_E_PIN, MODE_OUTPUT);
      delay_us(BUS_DRV_DLY);
      ts4231_pinMode(TS4231_N1_E_PIN, MODE_INPUT);
      delay_us(BUS_DRV_DLY);
      if (ts4231_checkBus() == SLEEP_STATE)
        sleep_success = true;
      else
        sleep_success = false;
      break;
    case S3_STATE:
      sleep_success = false;
      break;
    default:
      sleep_success = false;
      break;
    }
  }
  return sleep_success;
}

bool ts4231_goToWatch(void) {

  bool watch_success;

  if (ts4231_var.configured == false)
    watch_success = false;
  else {
    switch (ts4231_checkBus()) {
    case S0_STATE:
      watch_success = false;
      break;
    case SLEEP_STATE:
      ts4231_digitalWrite(TS4231_N1_D_PIN, OUTPUT_HIGH);
      ts4231_pinMode(TS4231_N1_D_PIN, MODE_OUTPUT);
      ts4231_digitalWrite(TS4231_N1_E_PIN, OUTPUT_LOW);
      ts4231_pinMode(TS4231_N1_E_PIN, MODE_OUTPUT);
      ts4231_digitalWrite(TS4231_N1_D_PIN, OUTPUT_LOW);
      ts4231_pinMode(TS4231_N1_D_PIN, MODE_INPUT);
      ts4231_digitalWrite(TS4231_N1_E_PIN, OUTPUT_HIGH);
      ts4231_pinMode(TS4231_N1_E_PIN, MODE_INPUT);
      delay_us(SLEEP_RECOVERY);
      if (ts4231_checkBus() == WATCH_STATE)
        watch_success = true;
      else
        watch_success = false;
      break;
    case WATCH_STATE:
      watch_success = true;
      break;
    case S3_STATE:
      ts4231_digitalWrite(TS4231_N1_E_PIN, OUTPUT_HIGH);
      ts4231_pinMode(TS4231_N1_E_PIN, MODE_OUTPUT);
      ts4231_digitalWrite(TS4231_N1_D_PIN, OUTPUT_HIGH);
      ts4231_pinMode(TS4231_N1_D_PIN, MODE_OUTPUT);
      ts4231_digitalWrite(TS4231_N1_E_PIN, OUTPUT_LOW);
      ts4231_digitalWrite(TS4231_N1_D_PIN, OUTPUT_LOW);
      ts4231_pinMode(TS4231_N1_D_PIN, MODE_INPUT);
      ts4231_digitalWrite(TS4231_N1_E_PIN, OUTPUT_HIGH);
      ts4231_pinMode(TS4231_N1_E_PIN, MODE_INPUT);
      delay_us(SLEEP_RECOVERY);
      if (ts4231_checkBus() == WATCH_STATE)
        watch_success = true;
      else
        watch_success = false;
      break;
    default:
      watch_success = false;
      break;
    }
  }
  return watch_success;
}

uint8_t ts4231_checkBus(void) {

  uint8_t state;
  uint8_t E_state;
  uint8_t D_state;
  uint8_t S0_count = 0;
  uint8_t SLEEP_count = 0;
  uint8_t WATCH_count = 0;
  uint8_t S3_count = 0;

  for (uint8_t i = 0; i < 3; i++) {
    E_state = ts_nrf_gpio_read_input(ts4231_var.E_pin);
    D_state = ts_nrf_gpio_read_input(ts4231_var.D_pin);
    if (D_state == 1) {
      if (E_state == 1)
        S3_count++;
      else
        SLEEP_count++;
    } else {
      if (E_state == 1)
        WATCH_count++;
      else
        S0_count++;
    }
    delay_us(BUS_CHECK_DLY);
  }
  if (SLEEP_count >= 2)
    state = SLEEP_STATE;
  else if (WATCH_count >= 2)
    state = WATCH_STATE;
  else if (S3_count >= 2)
    state = S3_STATE;
  else if (S0_count >= 2)
    state = S0_STATE;
  else
    state = UNKNOWN_STATE;

  return state;
}

// simulate the pinMode() in arduino
void ts4231_pinMode(uint32_t pin_number, uint8_t mode) {
  switch (mode) {
  case 0:
    ts_nrf_gpio_cfg_input(pin_number);
    break;
  case 1:
    ts_nrf_gpio_cfg_output(pin_number);
    break;
  }
}

// simulate the digitalRead() in arduino
uint32_t ts4231_digitalRead(uint32_t pin_number) {

  NRF_GPIO_Type *NRF_Px_port;
  uint32_t nrf_pin_number;

  if (pin_number < 32) {

    NRF_Px_port = NRF_P0;
    nrf_pin_number = pin_number;
  } else {

    NRF_Px_port = NRF_P1;
    nrf_pin_number = pin_number & 0x1f;
  }

  return (NRF_Px_port->IN >> pin_number) & 1UL;
}

// simulate the digitalWrite() in arduino
void ts4231_digitalWrite(uint32_t pin_number, uint8_t output_mode) {

  NRF_GPIO_Type *NRF_Px_port;
  uint32_t nrf_pin_number;

  if (pin_number < 32) {

    NRF_Px_port = NRF_P0;
    nrf_pin_number = pin_number;
  } else {

    NRF_Px_port = NRF_P1;
    nrf_pin_number = pin_number & 0x1f;
  }

  if (output_mode == 1) {
    NRF_Px_port->OUTSET = (1UL << pin_number);
  } else {
    NRF_Px_port->OUTCLR = (1UL << pin_number);
  }

  // A short delay function can be inserted here to extend the time between writes to
  // the E and D outputs if TS4231 timing parameters are being violated.  Consult
  // the TS4231 datasheet for more information on timing parameters.  It is recommended
  // that any added delay be no longer than approximately 1us.
  delay_us(1);
}

void ts4231_writeConfig(uint16_t config_val) {

  ts4231_digitalWrite(TS4231_N1_E_PIN, OUTPUT_HIGH);
  ts4231_digitalWrite(TS4231_N1_D_PIN, OUTPUT_HIGH);
  ts4231_pinMode(TS4231_N1_E_PIN, MODE_OUTPUT);
  ts4231_pinMode(TS4231_N1_D_PIN, MODE_OUTPUT);
  delay_us(BUS_DRV_DLY);
  ts4231_digitalWrite(TS4231_N1_D_PIN, OUTPUT_LOW);
  delay_us(BUS_DRV_DLY);
  ts4231_digitalWrite(TS4231_N1_E_PIN, OUTPUT_LOW);
  delay_us(BUS_DRV_DLY);
  for (uint8_t i = 0; i < 15; i++) {
    config_val = config_val << 1;
    if ((config_val & 0x8000) > 0)
      ts4231_digitalWrite(TS4231_N1_D_PIN, OUTPUT_HIGH);
    else
      ts4231_digitalWrite(TS4231_N1_D_PIN, OUTPUT_LOW);
    delay_us(BUS_DRV_DLY);
    ts4231_digitalWrite(TS4231_N1_E_PIN, OUTPUT_HIGH);
    delay_us(BUS_DRV_DLY);
    ts4231_digitalWrite(TS4231_N1_E_PIN, OUTPUT_LOW);
    delay_us(BUS_DRV_DLY);
  }
  ts4231_digitalWrite(TS4231_N1_D_PIN, OUTPUT_LOW);
  delay_us(BUS_DRV_DLY);
  ts4231_digitalWrite(TS4231_N1_E_PIN, OUTPUT_HIGH);
  delay_us(BUS_DRV_DLY);
  ts4231_digitalWrite(TS4231_N1_D_PIN, OUTPUT_HIGH);
  delay_us(BUS_DRV_DLY);
  ts4231_pinMode(TS4231_N1_E_PIN, MODE_INPUT);
  ts4231_pinMode(TS4231_N1_D_PIN, MODE_INPUT);
}

uint16_t ts4231_readConfig(void) {

  uint16_t readback;

  readback = 0x0000;
  ts4231_digitalWrite(TS4231_N1_E_PIN, OUTPUT_HIGH);
  ts4231_digitalWrite(TS4231_N1_D_PIN, OUTPUT_HIGH);
  ts4231_pinMode(TS4231_N1_E_PIN, MODE_OUTPUT);
  ts4231_pinMode(TS4231_N1_D_PIN, MODE_OUTPUT);
  delay_us(BUS_DRV_DLY);
  ts4231_digitalWrite(TS4231_N1_D_PIN, OUTPUT_LOW);
  delay_us(BUS_DRV_DLY);
  ts4231_digitalWrite(TS4231_N1_E_PIN, OUTPUT_LOW);
  delay_us(BUS_DRV_DLY);
  ts4231_digitalWrite(TS4231_N1_D_PIN, OUTPUT_HIGH);
  delay_us(BUS_DRV_DLY);
  ts4231_digitalWrite(TS4231_N1_E_PIN, OUTPUT_HIGH);
  delay_us(BUS_DRV_DLY);
  ts4231_pinMode(TS4231_N1_D_PIN, MODE_INPUT);
  delay_us(BUS_DRV_DLY);
  ts4231_digitalWrite(TS4231_N1_E_PIN, OUTPUT_LOW);
  delay_us(BUS_DRV_DLY);
  for (uint8_t i = 0; i < 14; i++) {
    ts4231_digitalWrite(TS4231_N1_E_PIN, OUTPUT_HIGH);
    delay_us(BUS_DRV_DLY);
    readback = (readback << 1) | (ts4231_digitalRead(TS4231_N1_D_PIN) & 0x0001);
    ts4231_digitalWrite(TS4231_N1_E_PIN, OUTPUT_LOW);
    delay_us(BUS_DRV_DLY);
  }
  ts4231_digitalWrite(TS4231_N1_D_PIN, OUTPUT_LOW);
  ts4231_pinMode(TS4231_N1_D_PIN, MODE_OUTPUT);
  delay_us(BUS_DRV_DLY);
  ts4231_digitalWrite(TS4231_N1_E_PIN, OUTPUT_HIGH);
  delay_us(BUS_DRV_DLY);
  ts4231_digitalWrite(TS4231_N1_D_PIN, OUTPUT_HIGH);
  delay_us(BUS_DRV_DLY);
  ts4231_pinMode(TS4231_N1_E_PIN, MODE_INPUT);
  ts4231_pinMode(TS4231_N1_D_PIN, MODE_INPUT);
  return readback;
}

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

//=========================== helper ==========================================

//=========================== private =========================================

// NRF-SDK interface, configure the pin as input
void ts_nrf_gpio_cfg_input(uint32_t pin_number) {

  NRF_GPIO_Type *NRF_Px_port;
  uint32_t nrf_pin_number;

  if (pin_number < 32) {

    NRF_Px_port = NRF_P0;
    nrf_pin_number = pin_number;
  } else {

    NRF_Px_port = NRF_P1;
    nrf_pin_number = pin_number & 0x1f;
  }
  // direction:input, PUll:float,
  NRF_Px_port->PIN_CNF[nrf_pin_number] =
      ((uint32_t)GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos) |
      ((uint32_t)GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
      ((uint32_t)GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
      ((uint32_t)GPIO_PIN_CNF_DRIVE_S0D1 << GPIO_PIN_CNF_DRIVE_Pos) |
      ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);
}

// NRF-SDK interface, configure the pin as output
void ts_nrf_gpio_cfg_output(uint32_t pin_number) {

  NRF_GPIO_Type *NRF_Px_port;
  uint32_t nrf_pin_number;

  if (pin_number < 32) {

    NRF_Px_port = NRF_P0;
    nrf_pin_number = pin_number;
  } else {

    NRF_Px_port = NRF_P1;
    nrf_pin_number = pin_number & 0x1f;
  }
  // direction:output, PUll:float,
  NRF_Px_port->PIN_CNF[nrf_pin_number] =
      ((uint32_t)GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos) |
      ((uint32_t)GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) |
      ((uint32_t)GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
      ((uint32_t)GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
      ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);
}

// NRF-SDK interface, read the pin input
uint32_t ts_nrf_gpio_read_input(uint32_t pin_number) {

  NRF_GPIO_Type *NRF_Px_port;
  uint32_t nrf_pin_number;

  if (pin_number < 32) {

    NRF_Px_port = NRF_P0;
    nrf_pin_number = pin_number;
  } else {

    NRF_Px_port = NRF_P1;
    nrf_pin_number = pin_number & 0x1f;
  }

  return (((NRF_Px_port->IN) >> pin_number) & 1UL);
}