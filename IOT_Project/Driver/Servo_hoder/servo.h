
#ifndef _HOLDER_H
#define _HOLDER_H

#include "Init.h"




/*********************************************************
*  TIM3: 
*		arr: 2000     pre: 720-1     period£∫50Hz (20ms)
*		compare value   |    pulse width
*			50			|		0.5ms
*			100			|		1.0ms
*			150			|		1.5ms
*			200			|		2.0ms
*			250			|		2.5ms
**********************************************************/

#define CLOCKWISE 		1	//clockwise À≥ ±’Î
#define	ANTICLOCKWISE	0	//anticlockwise ƒÊ ±’Î

#define X_AXIS		1

#define X_ANGLE_0_VAL		50
#define X_ANGLE_90_VAL		150
#define X_ANGLE_180_VAL		250

#define X_MIN_ANGLE		0
#define X_MAX_ANGLE		180

#define X_ANGLE_PER_VALUE	(180.0/(X_ANGLE_180_VAL-X_ANGLE_0_VAL))
#define X_VALUE_PER_ANGLE 	((X_ANGLE_180_VAL - X_ANGLE_0_VAL) / 180.0)

#define Y_AXIS		0

#define Y_ANGLE_0_VAL		50		
#define Y_ANGLE_90_VAL		150
#define Y_ANGLE_180_VAL		250

#define Y_MIN_ANGLE		0
#define Y_MAX_ANGLE		130

#define Y_ANGLE_PER_VALUE	(180.0/(X_ANGLE_180_VAL-X_ANGLE_0_VAL))
#define Y_VALUE_PER_ANGLE 	((Y_ANGLE_180_VAL - Y_ANGLE_0_VAL) / 180.0)



void Servo_Init(void);


double value_to_angle(uint16_t val);
uint16_t angle_to_value(double angle);

void Servo_Control_X(double angle);
void Servo_Control_Y(double angle);

double Servo_Get_X_Angle(void);
double Servo_Get_Y_Angle(void);


#endif	//_HOLDER_H


