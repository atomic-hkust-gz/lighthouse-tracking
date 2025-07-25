
# brief 
    This program shows how to use lighthouse to obtain data and Bluetooth to transmit coordinates.
    It is a combination of program "bsp_radio_ble" and program "nrf52833_lighthouse". There are several functions for control.
1. "ts4231.h" Used to control the TS4231 pin
        // E_Pin P(0,xx)
        #define TS4231_N1_E_GPIO_PORT 0
        #define TS4231_N1_E_GPIO_PIN xx
        // D_Pin P(0,xx)
        #define TS4231_N1_D_GPIO_PORT 0
        #define TS4231_N1_D_GPIO_PIN xx

2. "ppi_gpiote.c"
      data[6]:  Contains the coordinates that need to be sent 
      test[500]:To test whether TS4231 is feasible, 
                it is necessary to ensure that the data is in ABB form, 
                for example: 200 87 86, which meets the law of small size and must require a group of three.

3. "nrf52833_lighthouse_radio_ble.c"
        Through the program to switch the radio frequency CHANNEL.

# author 
Zhoutao Bi <bizhoutao.terry@gmail.com>, july 2025