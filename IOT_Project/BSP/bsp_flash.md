[TOC]


# stm32f10 flash 库文件

不同型号的 STM32，其 FLASH 容量也有所不同，最小的只有 16K 字节，最大的则达到了1024K 字节。战舰 STM32 开发板选择的 STM32F103ZET6 的 FLASH 容量为 512K 字节，属于大容量产品（另外还有中容量和小容量产品），block_size 属于 2K 字节。

------
库文件API：
## 锁定解锁
>* 1.void FLASH_Unlock(void)；

>* 1.void FLASH_Lock(void)；

## 写入

>* FLASH_Status FLASH_ProgramWord(uint32_t Address, uint32_t Data);  32字写入函数 
------
>*FLASH_Status FLASH_ProgramHalfWord(uint32_t Address, uint16_t Data); 32位半字写入， 写完后地址+2
------
>*FLASH_Status FLASH_ProgramOptionByteData(uint32_t Address, uint8_t Data);

## 读写
>* data=*(vu16*)addr;

## 擦除

>* FLASH_Status FLASH_ErasePage(uint32_t Page_Address);

------

>* FLASH_Status FLASH_EraseAllPages(void);

------


>* FLASH_Status FLASH_EraseOptionBytes(void);




# bsp_flash功能架构
## FLASH写入
>*  int32_t bsp_flash_write      (uint32_t addr,uint8_t* pbuf,int32_t len)
     输入参数：  add 写入地址  len 长度   需要写入的数组

------

```c
int32_t bsp_flash_write      (uint32_t addr,uint8_t* pbuf,int32_t len)
{
    uint32_t DATA_32 = 0;
    uint32_t i = 0;
    FLASH_Unlock();                                                           //FALSH 解锁
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR); //清除标志位
//    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRTPERR); 

    while (i < len)
    {
        if(len>(i+4)) { memcpy(&DATA_32,&pbuf[i],sizeof(uint32_t))  ; }
        else          { memcpy(&DATA_32,&pbuf[i],(len-i) ); }
        
        if (FLASH_ProgramWord(addr+i, DATA_32) == FLASH_COMPLETE)
        {
          i+=4;
        }
        else
        { 
            while (1);
        }
    }

    /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
    FLASH_Lock(); 
    return len;
}
```

## FLASH读取
>*  int32_t bsp_flash_read(uint32_t addr,uint8_t* p_dest,int32_t len)
     输入参数：  add 读取的flash地址   len长度   需要读出数据的缓存区
     采用memcpy作为读取的方式
------

  

```c
int32_t bsp_flash_read		 (uint32_t addr,uint8_t* p_dest,int32_t len)
{
    
    memcpy(p_dest, (void *)addr, len);
    return len;
}
```

## FLASH擦除
>*  int32_t int32_t bsp_flash_erase		 (uint32_t address,uint32_t len)
     输入参数：  address 需要擦除的flash首地址   len长度   需要擦除的flash首地址
------

```c
int32_t bsp_flash_erase		 (uint32_t address,uint32_t len)
{
    uint32_t EraseCounter = 0;
    uint32_t NbrOfPage = 0;
    
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR); 
    
    
    NbrOfPage = (len) / FLASH_PAGE_SIZE;
    if(len&(FLASH_PAGE_SIZE-1)){NbrOfPage++;}/*如果字节数大于当前块数*/
    
    for(EraseCounter = 0; (EraseCounter < NbrOfPage) && (FLASHStatus == FLASH_COMPLETE); EraseCounter++)
    {
        if (FLASH_ErasePage(address + (FLASH_PAGE_SIZE * EraseCounter))!= FLASH_COMPLETE)
        {
         /* Error occurred while sector erase. User can add here some code to deal with this error  */
       //     DBG_E("bsp_flash_erase error : address:0x%x ",address + (FLASH_PAGE_SIZE * EraseCounter));
            while (1);
        }
    }
    FLASH_Lock(); 
    return 0;
}
```


# 软件架构
软件架构说明
>*  1. 



#### 使用说明

**功能一**：默认工作状态，此状态下可开门。
		**开门方式**：1.输入密码（默认000000），按“确定”键开门。
				  2.刷卡可开门。

**功能二**：使用按键“项目二”，进入功能二，此状态下可添加数字密码。
		**添加过程**：先输入旧密码（默认000000），按“确定”键，再输入新密码，按“确定”键保存，按“取消”键返回默认状态。
注：密码最多存5个，存满后新加入的密码会覆盖最旧的密码。

**功能三**：使用按键“项目三”，进入功能三，此状态下可添加低频ID卡。
		**添加卡片**：先输入密码，按“确定”键，将卡片放到读卡器上。按“确定”键添加卡片，按“取消”键返回默认状态。
		**删除卡片**：先输入密码，按“确定”键，将卡片放到读卡器上。按“下”键删除卡片，按“取消”键返回默认状态。
注：密码最多存5个，存满后新加入的密码会覆盖最旧密码。

**功能四**：使用按键“项目四，进入功能四，此状态下可修改三轴加速计模块的参数。
		**阈值修改**：先输入密码，按“确定”键，按数字键输入新的警报阈值（建议范围40~100，默认50，数值越小，越灵敏），按“确定”键保存参数，按“取消”键返回默认状态。
		**校准过程**：先输入密码，按“确定”键。按“下”键校准三轴加速计模块，按“取消”键返回默认状态。

**功能五**：**使用按键**“项目五，进入功能五，此状态下可清除警报。
		**清除警报**：先输入密码，按“确定”键可清除警报。

**备注**：
任意功能下红外 和 三轴加速计模块 处于工作状态，会触发警报。
上电时会自动校准三轴加速计模块，之后倾斜一定的角度（20度左右）会触发警报。
按键“项目一”和按键“取消”功能相同。
门已开启的状态下不会触发任何警报。


#### 参与贡献

1. Fork 本项目
2. 新建 Feat_xxx 分支
3. 提交代码
4. 新建 Pull Request


#### 码云特技

1. 使用 Readme\_XXX.md 来支持不同的语言，例如 Readme\_en.md, Readme\_zh.md

#### markdown特技效果

1. >*
2. ------
3. 