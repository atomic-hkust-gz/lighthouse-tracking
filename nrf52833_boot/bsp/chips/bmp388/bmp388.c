/**
\brief bmp388 driver.

\author Tengfei Chang <tengfeichang@hkust-gz.edu.cn>, Nov 2023.
*/


#include "math.h"
#include "toolchain_defs.h"
#include "string.h"
#include "i2c.h"
#include "bmp388.h"

//=========================== define ==========================================

typedef struct {
    bmp388_par_t bmp388_par;
    bmp388_nvm_par_t bmp388_nvm_par;
    float  temperature;
    float pressure;
}bmp388_vars_t;

//=========================== variables =======================================

bmp388_vars_t bmp388_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

// admin
uint8_t bmp388_who_am_i(void) {

    uint8_t chipid;
    i2c_read_bytes(BMP388_REG_ADDR_CHIPID, &chipid, 1);
    return chipid;
}

uint8_t bmp388_get_status(void) {

    uint8_t pmu_status;
    i2c_read_bytes(BMP388_REG_ADDR_STATUS, &pmu_status, 1);
    return pmu_status;
}

uint8_t bmp388_get_errorreg(void) {
    
    uint8_t error_reg;
    i2c_read_bytes(BMP388_REG_ADDR_ERR_REG, &error_reg, 1);
    return error_reg;
}

void    bmp388_set_cmd(uint8_t cmd) {
    i2c_write_bytes(BMP388_REG_ADDR_CMD, &cmd, 1);
}


void    bmp388_power_on(void) {
    
    uint8_t control;
    control = (1<<PWR_CTRL_PRESS_EN_OFFSET) |
              (1<<PWR_CTRL_TEMP_EN_OFFSET)  |
              (3<<PWR_CTRL_MODE_OFFSET);

    i2c_write_bytes(BMP388_REG_ADDR_PWR_CTRL, &control, 1);
}

void    bmp388_set_config(uint8_t coef) {
    
    i2c_write_bytes(BMP388_REG_ADDR_CONFIG, &coef, 1);
}

void    bmp388_set_osr(uint8_t osr_t, uint8_t osr_p) {
    
    uint8_t os;
    os = (osr_t<<OSR_OSR_T_OFFSET) |
         (osr_p<<OSR_OSR_P_OFFSET);

    i2c_write_bytes(BMP388_REG_ADDR_OSR, &os, 1);
}

// read
float bmp388_read_temperature(void) {
    
    uint8_t tmp;
    uint32_t temp;

    float partial_data1;
    float partial_data2;

    float temp_f;

    i2c_read_bytes(BMP388_REG_ADDR_DATA3, &tmp, 1);
    temp = tmp;
    i2c_read_bytes(BMP388_REG_ADDR_DATA4, &tmp, 1);
    temp |= (((uint32_t)tmp)<<8);
    i2c_read_bytes(BMP388_REG_ADDR_DATA5, &tmp, 1);
    temp |= (((uint32_t)tmp)<<16);

    partial_data1 = (float)(temp-bmp388_vars.bmp388_par.par_t1);
    partial_data2 = (float)(partial_data1*bmp388_vars.bmp388_par.par_t2);

    temp_f = partial_data2 + \
        (partial_data1*partial_data1)*bmp388_vars.bmp388_par.par_t3;
    
    bmp388_vars.temperature = temp_f;

    return temp_f;
}

uint16_t bmp388_read_pressure(void) {
    
    uint8_t   tmp;
    uint32_t  pressure;
    float     pressure_f;

    float partial_data1;
    float partial_data2;
    float partial_data3;
    float partial_data4;
    float partial_out1;
    float partial_out2;

    i2c_read_bytes(BMP388_REG_ADDR_DATA0, &tmp, 1);
    pressure = tmp;
    i2c_read_bytes(BMP388_REG_ADDR_DATA1, &tmp, 1);
    pressure |= (((uint32_t)tmp)<<8);
    i2c_read_bytes(BMP388_REG_ADDR_DATA2, &tmp, 1);
    pressure |= (((uint32_t)tmp)<<16);

    partial_data1 = bmp388_vars.bmp388_par.par_p6*bmp388_vars.temperature;
    partial_data2 = bmp388_vars.bmp388_par.par_p7*(bmp388_vars.temperature*bmp388_vars.temperature);
    partial_data3 = bmp388_vars.bmp388_par.par_p8*(bmp388_vars.temperature*bmp388_vars.temperature*bmp388_vars.temperature);
    partial_out1  = bmp388_vars.bmp388_par.par_p5 + partial_data1 + partial_data2 + partial_data3;

    partial_data1 = bmp388_vars.bmp388_par.par_p2*bmp388_vars.temperature;
    partial_data2 = bmp388_vars.bmp388_par.par_p3*(bmp388_vars.temperature*bmp388_vars.temperature);
    partial_data3 = bmp388_vars.bmp388_par.par_p4*(bmp388_vars.temperature*bmp388_vars.temperature*bmp388_vars.temperature);
    partial_out2  = (float)pressure*(bmp388_vars.bmp388_par.par_p1+partial_data1 + partial_data2 + partial_data3);

    partial_data1 = (float)pressure*(float)pressure;
    partial_data2 = bmp388_vars.bmp388_par.par_p9*bmp388_vars.bmp388_par.par_p10*bmp388_vars.temperature;
    partial_data3 = partial_data1 * partial_data2;
    partial_data4 = partial_data3 + ((float)pressure*(float)pressure*(float)pressure)*bmp388_vars.bmp388_par.par_p11;

    pressure_f = partial_data1 + partial_data2 + partial_data4;

    bmp388_vars.pressure = pressure_f;

    return pressure_f;
}

void bmp388_read_nvm_par(void) {

    memset(
        &bmp388_vars.bmp388_nvm_par, 
        0, 
        sizeof(bmp388_nvm_par_t)
    );
  
    i2c_read_bytes(
        BMP388_REG_NVM_PAR_T, 
        (uint8_t*)&bmp388_vars.bmp388_nvm_par, 
        sizeof(bmp388_nvm_par_t)
    );
}

bmp388_par_t* bmp388_get_par(void) {

    memset(
        &bmp388_vars.bmp388_par, 
        0, 
        sizeof(bmp388_par_t)
    );

    bmp388_vars.bmp388_par.par_t1 = \
        bmp388_vars.bmp388_nvm_par.nvm_par_t1/(pow(2,-8));
        
    bmp388_vars.bmp388_par.par_t2 = \
        bmp388_vars.bmp388_nvm_par.nvm_par_t3/(pow(2,30));

    bmp388_vars.bmp388_par.par_t3 = \
        bmp388_vars.bmp388_nvm_par.nvm_par_t3/(pow(2,48));
        
    bmp388_vars.bmp388_par.par_p1 = \
        (bmp388_vars.bmp388_nvm_par.nvm_par_p1-pow(2,14))/(pow(2,20));
        
    bmp388_vars.bmp388_par.par_p2 = \
        (bmp388_vars.bmp388_nvm_par.nvm_par_p2-pow(2,14))/(pow(2,29));

    bmp388_vars.bmp388_par.par_p3 = \
        bmp388_vars.bmp388_nvm_par.nvm_par_p3/(pow(2,32));

    bmp388_vars.bmp388_par.par_p4 = \
        bmp388_vars.bmp388_nvm_par.nvm_par_p4/(pow(2,37));

    bmp388_vars.bmp388_par.par_p5 = \
        bmp388_vars.bmp388_nvm_par.nvm_par_p5/(pow(2,-3));

    bmp388_vars.bmp388_par.par_p6 = \
        bmp388_vars.bmp388_nvm_par.nvm_par_p6/(pow(2,6));

    bmp388_vars.bmp388_par.par_p7 = \
        bmp388_vars.bmp388_nvm_par.nvm_par_p7/(pow(2,8));
    
    bmp388_vars.bmp388_par.par_p8 = \
        bmp388_vars.bmp388_nvm_par.nvm_par_p8/(pow(2,15));

    bmp388_vars.bmp388_par.par_p9 = \
        bmp388_vars.bmp388_nvm_par.nvm_par_p9/(pow(2,48));
        
    bmp388_vars.bmp388_par.par_p10 = \
        bmp388_vars.bmp388_nvm_par.nvm_par_p10/(pow(2,48));

    bmp388_vars.bmp388_par.par_p11 = \
        bmp388_vars.bmp388_nvm_par.nvm_par_p11/(pow(2,65));

    return &bmp388_vars.bmp388_par;
}

//=========================== helper ==========================================

//=========================== private =========================================