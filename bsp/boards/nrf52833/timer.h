/**

Manjiang Cao   <manjiang19@hkust-gz.edu.cn> Atomic. 
Tengfei Chang  <tengfeichang@hkust-gz.edu.cn> Atomic. 

*/

#ifndef __TIMER_H__
#define __TIMER_H__

#include "board_info.h"

//=========================== define ==========================================

//=========================== typedef =========================================

typedef void  (*timer_cbt)(void);

//=========================== module variables ================================

//=========================== prototypes ======================================

void timer_init(void);
void timer_set_callback(uint8_t compare_id, timer_cbt cb);

void timer_start(void);
void timer_stop(void);

void timer_clear(void);
void timer_capture_now(uint8_t capture_id) ;

void timer_schedule(uint8_t compare_id, uint32_t value);
uint32_t timer_getCapturedValue(uint8_t compare_id);

#endif // __ADC_SENSOR_H__
