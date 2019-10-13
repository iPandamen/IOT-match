
#include "servo.h"

/********************************************
*@brief		Holder Init
*@param		None
*@retval	None
*********************************************/

uint16_t current_servo_x_angle; 
uint16_t current_servo_y_angle; 

/***********************************************************
*@brief		servo init 
*@parma		None
*@retval 	None
************************************************************/
void Servo_Init(void)
{
	TIM3_Init(2000-1,720-1);
}



/***********************************************************
*@brief		将 定时器比较值 转换为 角度 
*@parma		val：定时器比较值
*@retval 	返回角度
************************************************************/
double value_to_angle(uint16_t val)
{
	if(val <= X_ANGLE_180_VAL && val >= X_ANGLE_0_VAL)	// 输入是否正确
		return (val - X_ANGLE_0_VAL) * X_ANGLE_PER_VALUE;
	else
	{
		DEBUG("Error: The value should be within %d to %d!",X_ANGLE_0_VAL,X_ANGLE_180_VAL);
		return 0;
	}
}

/***********************************************************
*@brief		将 角度 转换为 定时器比较值 
*@parma		angle: 角度
*@retval 	返回定时器比较值
************************************************************/
uint16_t angle_to_value(double angle)
{
	uint16_t val = 0;
		
	if(angle <= 180 && angle >= 0)
	{
		val = angle * X_VALUE_PER_ANGLE +50;
		return  val;
	}
	else
	{
		printf("Error: Angle must between 0 and 180 ! \r\n");
		return 0;
	}
	
}
/***********************************************************
*@brief		设定 servo X 方向当前的角度
*@parma		angle： 设定的角度
*@retval 	None
************************************************************/
void Servo_Control_X(double angle)
{
	if(angle > X_MAX_ANGLE || angle < X_MIN_ANGLE)
	{
		DEBUG("The ange must between %d and %d! \r\n",X_MIN_ANGLE,X_MAX_ANGLE);
		if(angle > X_MAX_ANGLE)
			angle=X_MAX_ANGLE;
		if(angle < X_MIN_ANGLE)
			angle=X_MIN_ANGLE;
	}
	current_servo_x_angle = angle;
	TIM_SetCompare1(TIM3, angle_to_value(angle));
}


/***********************************************************
*@brief		设定 servo Y 方向当前的角度
*@parma		angle： 设定的角度
*@retval 	None
************************************************************/
void Servo_Control_Y(double angle)
{
	if(angle > Y_MAX_ANGLE || angle < Y_MIN_ANGLE)
	{
		DEBUG("The ange must between %d and %d! \r\n",Y_MIN_ANGLE,Y_MAX_ANGLE);
		if(angle > Y_MAX_ANGLE)
			angle = Y_MAX_ANGLE;
		if(angle < Y_MIN_ANGLE)
			angle = Y_MIN_ANGLE;
	}
	current_servo_y_angle=angle;
	TIM_SetCompare2(TIM3, angle_to_value(angle));
}


/*********************************************
*@brief		获取当前舵机 X 方向的角度
*@param		None
*@retval	角度
**********************************************/
double Servo_Get_X_Angle(void)
{
	uint16_t value = 0;
	double angle = 0;
	value = TIM_GetCapture1(TIM3);	// 获取比较器值
	angle = value_to_angle(value);	// 将比较器转换成角度
	if(angle <= X_MIN_ANGLE)
		angle = X_MIN_ANGLE;
	else if(angle >= X_MAX_ANGLE)
		angle = X_MAX_ANGLE;
	return angle;
}


/*********************************************
*@brief		获取当前舵机 Y 方向的角度
*@param		None
*@retval	角度
**********************************************/
double Servo_Get_Y_Angle(void)
{
	uint16_t value = 0;
	double angle = 0;
	value = TIM_GetCapture2(TIM3);	// 获取比较器值
	angle = value_to_angle(value);  // 将比较器转换成角度
	if(angle <= Y_MIN_ANGLE)
		angle = Y_MIN_ANGLE;
	else if(angle >= Y_MAX_ANGLE)
		angle = Y_MAX_ANGLE;
	return angle;
}











