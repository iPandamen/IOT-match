
#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#define TIM4_SRC	100000

typedef void (*ULTRASONIC_CALLBACK_FUNC)(uint32_t ultrasonic_new_dist);


//初始化TIM3 channel3为输入捕捉进行超声波测试
void TIM4_ch3_input_capture_ultrasonic_init(ULTRASONIC_CALLBACK_FUNC ult_update_notify_cb);
//读取超声波测到距离(单位为:ms)
uint16_t read_ultrasonic_dist(void);


#endif

