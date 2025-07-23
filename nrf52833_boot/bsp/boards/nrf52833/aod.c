/**
    \brief Definition of the nrf5340 aod algorithm.
    \author Manjiang Cao   <manjiang19@hkust-gz.edu.cn> Atomic. Nov, 2024
*/

#include <stdio.h>
#include <math.h>
#include "board.h"

//=========================== defines =========================================

#define ANT_NUM             4
#define SPEED_OF_LIGHT      299792458
#define ANGLE_RANGE         180
#define FREQUENCY           2400000000
#define ANT_INTERVAL        0.0375
#define PI                  3.1415926
#define ONE_ANT_CONSTANT    -1.89
#define TWO_ANT_CONSTANT    -3.78


//=========================== typedef =========================================
typedef struct {
    float    I;
    float    Q;
} IQ_sturcture;

IQ_sturcture IQ_sample;

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
sample_array_int_t sample_array_int;

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
sample_array_float_t sample_array_float;

typedef  struct {
  float           ant0_I;
  float           ant0_Q;
  float           ant1_I;
  float           ant1_Q;
  float           ant2_I;
  float           ant2_Q;

} ant_mean_t;
ant_mean_t ant_mean;

typedef struct {
    double real; // 实部
    double imag; // 虚部
} Complex;

//=========================== variables =======================================

//=========================== prototype =======================================

//===========================privacy===========================================

IQ_sturcture  normalization(float ant_I, float ant_Q) {
    float amplitude;
    float norm_I;
    float norm_Q;
    IQ_sturcture IQ_sample;
    amplitude = amplitude = sqrt(ant_I * ant_I + ant_Q * ant_Q);

    if (amplitude != 0) {
        norm_I = ant_I / amplitude; 
        norm_Q = ant_Q / amplitude; 
    } else {
        // if amplitude == 0, set I and Q to 0
        norm_I = 0; 
        norm_Q = 0;
    }
    IQ_sample.I = norm_I;
    IQ_sample.Q = norm_Q;
    return IQ_sample;
}

float calculate_angle(float I1, float Q1, float I2, float Q2) {
    float dot_product;
    float cross_product;
    dot_product = I1*I2 + Q1*Q2;

    if (dot_product > 1) {
        dot_product = 1;
    } else if (dot_product < -1) {
        dot_product = -1;
    }
    
    float theta;
    theta = acos(dot_product);
    
    cross_product = I1 * Q2 - Q1 * I2;
    if (cross_product > 0) {
        theta = theta;
    } else {
        theta = -theta;
    }

    return theta;
}

float angle_diff_per_us(sample_array_float_t sample_array_float) {
    float ref_theta_array[4];
    for (uint8_t i=0;i<4;i++) {
        ref_theta_array[i] = calculate_angle(sample_array_float.ref_I_f[i], sample_array_float.ref_Q_f[i], sample_array_float.ref_I_f[i+4], sample_array_float.ref_Q_f[i+4]);
    }
    float theta_sum;
    theta_sum = 0;
    for (uint8_t i=0;i<4;i++) {
        theta_sum += ref_theta_array[i];
    }
    return (theta_sum/4)/4;
}

IQ_sturcture rotate_vector(float I, float Q, float theta) {
    IQ_sample.I = I*cos(theta) - Q*sin(theta);
    IQ_sample.Q = I*sin(theta) + Q*cos(theta);
    return IQ_sample;
}

ant_mean_t cal_mean_phase(sample_array_float_t sample_array_float) {
    float total_I, total_Q;
    //cal mean ant0 phase
    total_I = 0;
    total_Q = 0;

    for (uint8_t i=0;i<10;i++) {
        total_I += sample_array_float.ant0_I_f[i];
        total_Q += sample_array_float.ant0_Q_f[i];
    }
    ant_mean.ant0_I = total_I / 10;
    ant_mean.ant0_Q = total_Q / 10;

    //cal mean ant1 phase
    total_I = 0;
    total_Q = 0;
    for (uint8_t i=0;i<10;i++) {
        total_I += sample_array_float.ant1_I_f[i];
        total_Q += sample_array_float.ant1_Q_f[i];
    }
    ant_mean.ant1_I = total_I / 10;
    ant_mean.ant1_Q = total_Q / 10;

    //cal mean ant2 phase
    total_I = 0;
    total_Q = 0;
    for (uint8_t i=0;i<10;i++) {
        total_I += sample_array_float.ant2_I_f[i];
        total_Q += sample_array_float.ant2_Q_f[i];
    }
    ant_mean.ant2_I = total_I / 10;
    ant_mean.ant2_Q = total_Q / 10;

    return ant_mean;
}

Complex complex_add(Complex a, Complex b) {
    Complex result;
    result.real = a.real + b.real;
    result.imag = a.imag + b.imag;
    return result;
}

Complex complex_multiply(Complex a, Complex b) {
    Complex result;
    result.real = a.real * b.real - a.imag * b.imag; 
    result.imag = a.real * b.imag + a.imag * b.real;
    return result;
}

Complex complex_exponential(float theta) {
    Complex result;
    result.real = cos(theta);
    result.imag = sin(theta);
    return result;
}

float complex_norm(Complex a) {
    float result;
    result = sqrt(a.real*a.real + a.imag*a.imag);
    return result;
}

Complex* update_steering_vector1(Complex* steer_vector_array1) {
    for (int i = -90; i < 90; i++) {
        steer_vector_array1[i+90] = complex_exponential(-2*PI*FREQUENCY*ANT_INTERVAL/SPEED_OF_LIGHT*sin(i*(PI/180)));
    }
    return steer_vector_array1;
}

Complex* update_steering_vector2(Complex* steer_vector_array2) {
    for (int i = -90; i < 90; i++) {
        steer_vector_array2[i+90] = complex_exponential(-2*PI*FREQUENCY*2*ANT_INTERVAL/SPEED_OF_LIGHT*sin(i*(PI/180)));
    }
    return steer_vector_array2;
}


int8_t DoA_algorithm(ant_mean_t ant_mean, Complex* steer_vector_array1, Complex* steer_vector_array2) {
    float ant0_theta, ant1_theta, ant2_theta;
    ant0_theta = atan2(ant_mean.ant0_Q, ant_mean.ant0_I);
    ant1_theta = atan2(ant_mean.ant1_Q, ant_mean.ant1_I);
    ant2_theta = atan2(ant_mean.ant2_Q, ant_mean.ant2_I);

    ant1_theta = ant1_theta - ant0_theta;
    ant2_theta = ant2_theta - ant0_theta;
    ant0_theta = 0.0;

    Complex received_signal[3];
    received_signal[0] = complex_exponential(ant0_theta);
    received_signal[1] = complex_exponential(ant1_theta);
    received_signal[2] = complex_exponential(ant2_theta);

    // 角度列表
    double angle_list[ANGLE_RANGE];
    for (int i = -90; i < 90; i++) {
        angle_list[i + 90] = i; // 将角度范围从 -90 到 90 转换为 0 到 179
    }

    // 存储 y_alpha_values
    double y_alpha_list[ANGLE_RANGE];
    for (int i = 0; i < ANGLE_RANGE; i++) {
        double alpha = angle_list[i];
        alpha = alpha *(PI / 180);
        Complex steer_vector[3];
        steer_vector[0].real = 1;
        steer_vector[0].imag = 0;

        //steer_vector[1] = complex_exponential(-2*PI*FREQUENCY*ANT_INTERVAL*sin(alpha)/SPEED_OF_LIGHT);
        //steer_vector[2] = complex_exponential(-2*PI*FREQUENCY*2*ANT_INTERVAL*sin(alpha)/SPEED_OF_LIGHT);


        //没必要每个循环都计算steer vector， 可以打表


        steer_vector[1] = steer_vector_array1[i];
        steer_vector[2] = steer_vector_array2[i];


        Complex part1 = complex_multiply(steer_vector[0], received_signal[0]);
        Complex part2 = complex_multiply(steer_vector[1], received_signal[1]);
        Complex part3 = complex_multiply(steer_vector[2], received_signal[2]);
        
        Complex add1 = complex_add(part1, part2);
        Complex add_total = complex_add(add1, part3);

        y_alpha_list[i] = add_total.real;
    }

    // 找到 y_alpha_list 中的最大值的索引
    double max_value = y_alpha_list[0];
    int8_t best_angle_index = 0;
    for (int i = 1; i < ANGLE_RANGE; i++) {
        if (y_alpha_list[i] > max_value) {
            max_value = y_alpha_list[i];
            best_angle_index = i;
        }
    }
    return best_angle_index - 90;
}


//=========================== public ==========================================

//normlize all antenna data
sample_array_float_t ant_IQ_norm(sample_array_int_t sample_array_int) {
    //normlize ref ant data
    for (uint8_t i=0;i<8;i++) {
        IQ_sample = normalization(sample_array_int.ref_I[i], sample_array_int.ref_Q[i]);
        sample_array_float.ref_I_f[i] = IQ_sample.I;
        sample_array_float.ref_Q_f[i] = IQ_sample.Q;
    }

    //normlize ant0 data
    for (uint8_t i=0;i<10;i++) {
        IQ_sample = normalization(sample_array_int.ant0_I[i], sample_array_int.ant0_Q[i]);
        sample_array_float.ant0_I_f[i] = IQ_sample.I;
        sample_array_float.ant0_Q_f[i] = IQ_sample.Q;

        //normlize ant1 data
        IQ_sample = normalization(sample_array_int.ant1_I[i], sample_array_int.ant1_Q[i]);
        sample_array_float.ant1_I_f[i] = IQ_sample.I;
        sample_array_float.ant1_Q_f[i] = IQ_sample.Q;
   
        //normlize ant2 data
        IQ_sample = normalization(sample_array_int.ant2_I[i], sample_array_int.ant2_Q[i]);
        sample_array_float.ant2_I_f[i] = IQ_sample.I;
        sample_array_float.ant2_Q_f[i] = IQ_sample.Q;
   
        //normlize ant3 data
        IQ_sample = normalization(sample_array_int.ant3_I[i], sample_array_int.ant3_Q[i]);
        sample_array_float.ant3_I_f[i] = IQ_sample.I;
        sample_array_float.ant3_Q_f[i] = IQ_sample.Q;
    }
    return sample_array_float;
}

sample_array_float_t compensate_phase(sample_array_float_t sample_array_float, float angle_change_us) {
    float rotate_angle;
    //double pi = acos(-1.0);

    for (uint8_t i=0;i<10;i++) {
        rotate_angle = i*(PI/2 + angle_change_us)*8;
        //compensate ant0
        IQ_sample = rotate_vector(sample_array_float.ant0_I_f[i], sample_array_float.ant0_Q_f[i], -rotate_angle);
        sample_array_float.ant0_I_f[i] = IQ_sample.I;
        sample_array_float.ant0_Q_f[i] = IQ_sample.Q;

        //compensate ant1
        IQ_sample = rotate_vector(sample_array_float.ant1_I_f[i], sample_array_float.ant1_Q_f[i], -rotate_angle);
        sample_array_float.ant1_I_f[i] = IQ_sample.I;
        sample_array_float.ant1_Q_f[i] = IQ_sample.Q;

        //compensate ant2
        IQ_sample = rotate_vector(sample_array_float.ant2_I_f[i], sample_array_float.ant2_Q_f[i], -rotate_angle);
        sample_array_float.ant2_I_f[i] = IQ_sample.I;
        sample_array_float.ant2_Q_f[i] = IQ_sample.Q;

        //compensate ant3
        //IQ_sample = rotate_vector(sample_array_float.ant3_I_f[i], sample_array_float.ant3_Q_f[i], -rotate_angle);
        //sample_array_float.ant3_I_f[i] = IQ_sample.I;
        //sample_array_float.ant3_Q_f[i] = IQ_sample.Q;
    }
    return sample_array_float;
}

uint16_t cal_angle(sample_array_int_t sample_array_int, Complex* steer_vector_array1, Complex* steer_vector_array2) {

    sample_array_float = ant_IQ_norm(sample_array_int);

    float angle_change_us;
    angle_change_us = angle_diff_per_us(sample_array_float);

    sample_array_float = compensate_phase(sample_array_float, angle_change_us);

    ant_mean = cal_mean_phase(sample_array_float);
    
    float rotate_angle_1_0;
    float rotate_angle_2_0;

    rotate_angle_1_0 = 2*(PI/2 + angle_change_us);
    rotate_angle_2_0 = 4*(PI/2 + angle_change_us);

    IQ_sample = rotate_vector(ant_mean.ant1_I, ant_mean.ant1_Q, -rotate_angle_1_0);
    ant_mean.ant1_I = IQ_sample.I;
    ant_mean.ant1_Q = IQ_sample.Q;

    IQ_sample = rotate_vector(ant_mean.ant2_I, ant_mean.ant2_Q, -rotate_angle_2_0);
    ant_mean.ant2_I = IQ_sample.I;
    ant_mean.ant2_Q = IQ_sample.Q;

    IQ_sample = normalization(ant_mean.ant0_I, ant_mean.ant0_Q);
    ant_mean.ant0_I = IQ_sample.I;
    ant_mean.ant0_Q = IQ_sample.Q;

    IQ_sample = normalization(ant_mean.ant1_I, ant_mean.ant1_Q);
    ant_mean.ant1_I = IQ_sample.I;
    ant_mean.ant1_Q = IQ_sample.Q;

    IQ_sample = normalization(ant_mean.ant2_I, ant_mean.ant2_Q);
    ant_mean.ant2_I = IQ_sample.I;
    ant_mean.ant2_Q = IQ_sample.Q;

    int8_t estimate_angle;
    estimate_angle = DoA_algorithm(ant_mean, steer_vector_array1, steer_vector_array2);

    return estimate_angle;
}