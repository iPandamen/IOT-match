#ifndef _thermistor_h_
#define _thermistor_h_

/*===========================================================================*/
#ifdef __cplusplus  /* C++֧�� */
	extern "C"{
#endif
		
#include "sys.h"
#include "adc.h"

/********************************************************
*�������ƣ�int8_t getTemperature(void)
*���ܣ��������ǰ���������⵽���¶�
*���룺��
*���������¶�ֵ���з��ţ���λ����
*����˵�����ȷ�Χ-20��~120��
**********************************************************/
int8_t getTemperature(void);


/*===========================================================================*/
#ifdef __cplusplus  /* C++֧�� */
	}
#endif
/*===========================================================================*/

#endif  /* endif thermistor.h */
