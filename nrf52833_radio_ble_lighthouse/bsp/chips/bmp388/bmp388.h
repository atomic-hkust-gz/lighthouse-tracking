/**
\brief registers address mapping of bmp388 sensor.

\author Tengfei Chang <tengfeichang@hkust-gz.edu.cn>, Nov 2023.
*/

#include "stdint.h"

//=========================== define ==========================================

#define BMP388_ADDR 0x76

//---- register addresses

#define BMP388_REG_ADDR_CHIPID          0x00
#define BMP388_REG_ADDR_ERR_REG         0x02
#define BMP388_REG_ADDR_STATUS          0x03

// sensor data

// presure sensor data
#define BMP388_REG_ADDR_DATA0           0x04
#define BMP388_REG_ADDR_DATA1           0x05
#define BMP388_REG_ADDR_DATA2           0x06

// temp sensor data
#define BMP388_REG_ADDR_DATA3           0x07
#define BMP388_REG_ADDR_DATA4           0x08
#define BMP388_REG_ADDR_DATA5           0x09

#define BMP388_REG_ADDR_SENSORTIME0     0x0c
#define BMP388_REG_ADDR_SENSORTIME1     0x0d
#define BMP388_REG_ADDR_SENSORTIME2     0x0e

#define BMP388_REG_ADDR_EVENT           0x10
#define BMP388_REG_ADDR_INT_STATUS      0x11

#define BMP388_REG_ADDR_FIFO_LENGTH0    0x12
#define BMP388_REG_ADDR_FIFO_LENGTH1    0x13
#define BMP388_REG_ADDR_FIFO_DATA       0x14
#define BMP388_REG_ADDR_FIFO_WTM_0      0x15
#define BMP388_REG_ADDR_FIFO_WTM_1      0x16
#define BMP388_REG_ADDR_FIFO_CONFIG_1   0x17
#define BMP388_REG_ADDR_FIFO_CONFIG_2   0x18

#define BMP388_REG_ADDR_INT_CTRL        0x19

#define BMP388_REG_ADDR_IF_CONF         0x1a

#define BMP388_REG_ADDR_PWR_CTRL        0x1b

#define BMP388_REG_ADDR_OSR             0x1c
#define BMP388_REG_ADDR_ODR             0x1d

#define BMP388_REG_ADDR_CONFIG          0x1f

#define BMP388_REG_NVM_PAR_T            0x31
#define BMP388_REG_NVM_PAR_P            0x36

#define BMP388_REG_ADDR_CMD             0x7e

//---- register values

#define PWR_CTRL_PRESS_EN_OFFSET        0
#define PWR_CTRL_TEMP_EN_OFFSET         1
#define PWR_CTRL_MODE_OFFSET            4

#define OSR_OSR_P_OFFSET                0
#define OSR_OSR_T_OFFSET                3

//---- calibration

BEGIN_PACK
typedef struct{
    
    uint16_t  nvm_par_t1;
    uint16_t  nvm_par_t2;
    uint8_t   nvm_par_t3;

    int16_t   nvm_par_p1;
    int16_t   nvm_par_p2;
    int8_t    nvm_par_p3;
    int8_t    nvm_par_p4;
    int16_t   nvm_par_p5;
    int16_t   nvm_par_p6;
    int8_t    nvm_par_p7;
    int8_t    nvm_par_p8;
    int16_t   nvm_par_p9;
    int8_t    nvm_par_p10;
    int8_t    nvm_par_p11;

}bmp388_nvm_par_t;
END_PACK


typedef struct{

    // par temperature
    float par_t1;
    float par_t2;
    float par_t3;

    // par pressure
    float par_p1;
    float par_p2;
    float par_p3;
    float par_p4;
    float par_p5;
    float par_p6;
    float par_p7;
    float par_p8;
    float par_p9;
    float par_p10;
    float par_p11;
}bmp388_par_t;

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

// admin
uint8_t bmp388_who_am_i(void);
uint8_t bmp388_get_status(void);
uint8_t bmp388_get_errorreg(void);

void    bmp388_power_on(void);
void    bmp388_set_cmd(uint8_t cmd);
void    bmp388_set_osr(uint8_t osr_t, uint8_t osr_p);

void    bmp388_set_config(uint8_t config);

void    bmp388_read_nvm_par(void);

bmp388_par_t* bmp388_get_par(void);

// read
float bmp388_read_temperature(void);
uint16_t bmp388_read_pressure(void);


//=========================== private =========================================