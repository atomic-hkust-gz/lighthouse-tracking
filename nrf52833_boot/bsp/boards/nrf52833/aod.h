/**

Manjiang Cao   <manjiang19@hkust-gz.edu.cn> Atomic. 

*/
#ifndef __AOD_H__
#define __AOD_H__

//=========================== typedef =========================================
typedef struct {
    float    I;
    float    Q;
} IQ_sturcture;

typedef struct {
    int16_t        ref_I[8];
    int16_t        ref_Q[8];

    int16_t        ant0_I[10];
    int16_t        ant0_Q[10];

    int16_t        ant1_I[10];
    int16_t        ant1_Q[10];

    int16_t        ant2_I[10];
    int16_t        ant2_Q[10];

    int16_t        ant3_I[10];
    int16_t        ant3_Q[10];
} sample_array_int_t;

typedef  struct {
    float         ref_I_f[8];
    float         ref_Q_f[8];
    float         ant0_I_f[10];
    float         ant0_Q_f[10];
    float         ant1_I_f[10];
    float         ant1_Q_f[10];
    float         ant2_I_f[10];
    float         ant2_Q_f[10];

    float         ant3_I_f[10];
    float         ant3_Q_f[10];
} sample_array_float_t;

typedef  struct {
  float           ant0_I;
  float           ant0_Q;
  float           ant1_I;
  float           ant1_Q;
  float           ant2_I;
  float           ant2_Q;

} ant_mean_t;

typedef struct {
    double real; // 实部
    double imag; // 虚部
} Complex;

//=========================== prototypes ======================================

//===========================privacy===========================================
IQ_sturcture  normalization(float ant_I, float ant_Q);
float calculate_angle(float I1, float Q1, float I2, float Q2);
float angle_diff_per_us(sample_array_float_t sample_array_float);
IQ_sturcture rotate_vector(float I, float Q, float theta);
ant_mean_t cal_mean_phase(sample_array_float_t sample_array_float);

Complex complex_add(Complex a, Complex b);
Complex complex_multiply(Complex a, Complex b);
Complex complex_exponential(float theta);
float complex_norm(Complex a);
Complex* steering_vector(float alpha);
uint16_t DoA_algorithm(ant_mean_t ant_mean);


//=========================== public ==========================================
sample_array_float_t ant_IQ_norm(sample_array_int_t sample_array_int);
sample_array_float_t compensate_phase(sample_array_float_t sample_array_float, float angle_change_us);
uint16_t cal_angle(sample_array_int_t sample_array_int, Complex*, Complex*);

#endif