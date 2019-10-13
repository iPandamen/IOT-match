# ReadME


### ADC
>温度计算
>
>>	T(°C)= [(V25 - Vsense) / Avg_Slope] + 25
>>	

>其中：
>
>> 	`Vsense` 是传感器输出的电压
>> 	`V25`=Vsense在25度时的数值（典型值为：`1.43V`)
>> 	`Avg_Slope` = 温度与Vsense 曲线的平均斜率（单位为  mv/°C 或 uv / °C)(典型值为`4.3mV/°C`)
>> 	

> 	一般计算我们采用公式：
> 
> >		T(°C)=[(1.43 - Vsense) / 0.0043 ]+ 25;
> >		

### LF卡

>ID1：
>>Page 1:	07 A8 50 26 3F 41 D5 84
>>Page 0:
>>>Blokc 0：00 28 01 17
>>>Block 1:	00 00 00 00
>>>Block 2:	BB BB BB 3B 	无法写入
>>>Block 3: 00 00 00 00 
>>>Block 4: 00 00 00 00
>>>Block 5: 00 00 00 00
>>>Block 6: 00 00 00 00
>>>Block 7:	77 77 77 77

>ID2： 读不出来
>>Page 1:	
>>Page 0:
>>
>>>Blokc 0：
>>>Block 1:	55 55 55 55
>>>Block 2:	55 55 55 55
>>>Block 3:  
>>>Block 4:
>>>Block 5:
>>>Block 6:
>>>Block 7:
>>>

>ID3：
>>Page 1:	07 A8 50 26 3F 41 69 53
>>Page 0:
>>
>>>Blokc 0：00 28 01 17
>>>Block 1:  00 00 00 00
>>>Block 2: 00 00 00 00 
>>>Block 3: 00 00 00 00
>>>Block 4: 00 00 00 00
>>>Block 5: 00 00 00 00
>>>Block 6: 00 00 00 00
>>>Block 7: 77 77 77 77
>>


### 指纹模块


