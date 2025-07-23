/**
\brief nRF52840-specific definition of the "leds" bsp module.

\author Tengfei Chang <tengfei.chang@gmail.com>, July 2020.
*/

#include "nrf52833.h"
#include "nrf52833_bitfields.h"
#include "leds.h"

//=========================== defines =========================================

#define NRF_GPIO_PIN_MAP(port, pin) (((port) << 5) | ((pin) & 0x1F))

#define LED_1          4
#define LED_2          5
#define LED_3          9
//#define LED_4          NRF_GPIO_PIN_MAP(0,16) // p0.16


//=========================== variables =======================================

//=========================== prototypes ======================================

void nrf_gpio_cfg_output(uint32_t pin_number);

//=========================== public ==========================================

void leds_init() {

    NRF_P0->DIRSET = 1<<LED_1;
    NRF_P0->DIRSET = 1<<LED_2;
    NRF_P1->DIRSET = 1<<LED_3;
    //NRF_P0->DIRSET = 1<<LED_4;

    leds_all_off();
}

//==== error led 01

void leds_error_off(void) {
    NRF_P0->OUTCLR = 1<<LED_1;
}

void leds_error_on(void) {
    NRF_P0->OUTSET = 1<<LED_1;
}

void leds_error_toggle(void) {
    if ((NRF_P0->OUT & (1<<LED_1))!=0) {        
        NRF_P0->OUTCLR = 1<<LED_1;
    } else {
        NRF_P0->OUTSET = 1<<LED_1;
    }
}

uint8_t leds_error_isOn(void) {
    if (NRF_P0->OUT & (1<<LED_1)) {
        return 0;
    } else {
        return 1;
    }
}

//==== sync led 02

void leds_sync_off(void) {
    NRF_P0->OUTCLR = 1<<LED_2;
}

void leds_sync_on(void) {
    NRF_P0->OUTSET = 1<<LED_2;
}

void leds_sync_toggle(void) {
    if ((NRF_P0->OUT & (1<<LED_2))!=0) {        
        NRF_P0->OUTCLR = 1<<LED_2;
    } else {
        NRF_P0->OUTSET = 1<<LED_2;
    }
}

uint8_t leds_sync_isOn(void) {
    if (NRF_P0->OUT & (1<<LED_2)) {
        return 0;
    } else {
        return 1;
    }
}

//==== radio led 03

void leds_radio_off(void) {
    NRF_P1->OUTCLR = 1<<LED_3;
}

void leds_radio_on(void) {
    NRF_P1->OUTSET = 1<<LED_3;
}

void leds_radio_toggle(void) {
    if ((NRF_P1->OUT & (1<<LED_3))!=0) {        
        NRF_P1->OUTCLR = 1<<LED_3;
    } else {
        NRF_P1->OUTSET = 1<<LED_3;
    }
}

uint8_t leds_radio_isOn(void) {
    if (NRF_P1->OUT & (1<<LED_3)) {
        return 0;
    } else {
        return 1;
    }
}

//==== debug led


void leds_debug_off(void) {
    //NRF_P0->OUTSET = 1<<LED_4;
}

void leds_debug_on(void) {
    //NRF_P0->OUTCLR = 1<<LED_4;
}

void leds_debug_toggle(void) {
    //if ((NRF_P0->OUT & (1<<LED_4))!=0) {        
    //    NRF_P0->OUTCLR = 1<<LED_4;
    //} else {
    //    NRF_P0->OUTSET = 1<<LED_4;
    //}
}

uint8_t leds_debug_isOn(void) {
    //if (NRF_P0->OUT & (1<<LED_4)) {
    //    return 0;
    //} else {
    //    return 1;
    //}
}

//==== all leds

void leds_all_on(void) {
    leds_radio_on();
    leds_sync_on();
    //leds_debug_on();
    leds_error_on();
}

void leds_all_off(void) {
    leds_radio_off();
    leds_sync_off();
    //leds_debug_off();
    leds_error_off();
}

void leds_all_toggle(void) {
    leds_radio_toggle();
    leds_sync_toggle();
    //leds_debug_toggle();
    leds_error_toggle();
}

void leds_error_blink(void) {
    
    uint8_t i;
    uint32_t j;

    // turn all LEDs off
    leds_all_off();

    // blink error LED for ~10s
    for (i = 0; i < 100; i++) {
        leds_error_toggle();
        for(j=0;j<0x1ffff;j++);
    }
}

void leds_circular_shift(void) {

    uint32_t led_new_value;
    uint32_t led_read;
    uint32_t shift_bit;

    led_read = NRF_P0->OUT & 0x0001e000;
    shift_bit = (NRF_P0->OUT & 0x00010000)>>3;
    led_new_value = ((led_read<<1) & 0x0001e000) | shift_bit; 
    
    NRF_P0->OUTSET = led_new_value;
}

void leds_increment(void) {
    
    uint32_t led_new_value;
    uint32_t led_read;

    led_read = (NRF_P0->OUT & 0x0001e000)>>13;
    led_new_value = ((led_read+1) & 0x0000000f)<<13;

    NRF_P0->OUTSET = led_new_value;
}

//=========================== private =========================================

void nrf_gpio_cfg_output(uint32_t pin_number){
 

    NRF_P0->PIN_CNF[pin_number] =   \
           ((uint32_t)GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos)
         | ((uint32_t)GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos)
         | ((uint32_t)GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos)
         | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)
         | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);

}

//void nrf_gpio_pin_write(uint32_t pin_number, uint32_t value)
//{
//    if (value == 0)
//    {
//        nrf_gpio_pin_clear(pin_number);
//    }
//    else
//    {
//        nrf_gpio_pin_set(pin_number);
//    }
//}

//void nrf_gpio_pin_set(uint32_t pin_number)
//{
//    NRF_GPIO_Type * reg = nrf_gpio_pin_port_decode(&pin_number);

//    nrf_gpio_port_out_set(reg, 1UL << pin_number);
//}


//void nrf_gpio_pin_clear(uint32_t pin_number)
//{
//    NRF_GPIO_Type * reg = nrf_gpio_pin_port_decode(&pin_number);

//    nrf_gpio_port_out_clear(reg, 1UL << pin_number);
//}

//NRF_GPIO_Type * nrf_gpio_pin_port_decode(uint32_t * p_pin)
//{
//    //NRFX_ASSERT(nrf_gpio_pin_present_check(*p_pin));
//    if (*p_pin < P0_PIN_NUM)
//    {
//        return NRF_P0;
//    }
//    else
//    {
//        *p_pin = *p_pin & 0x1F;
//        return NRF_P1;
//    }
//}
