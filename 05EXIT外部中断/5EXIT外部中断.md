# 5 EXIT外部中断

 [toc]

注：笔记主要参考B站 [江科大自化协](https://space.bilibili.com/383400717) 教学视频“[STM32入门教程-2023持续更新中](https://www.bilibili.com/video/BV1th411z7sn/)”。
注：工程及代码文件放在了本人的[Github仓库](https://github.com/jjejdhhd/Learn_stm32f103/tree/main)。
***

## 5.1 STM32中断系统
<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/5-1%E4%B8%AD%E6%96%AD%E7%A4%BA%E6%84%8F%E5%9B%BE.png" width="60%">
</div><div align=center>
图5-1 中断及中断嵌套示意图
</div>

**中断** 是指在主程序运行过程中，出现了特定的中断触发条件（中断源），使得CPU暂停当前正在运行的程序，转而去处理中断程序，处理完成后又返回原来被暂停的位置继续运行。使用中断系统，可以极大程度地提高程序的效率，就像是给自己定闹钟，可以不用担心错过时间而可以安心睡觉。在这个过程中，有如下概念：
> - 中断优先级：当有多个中断源同时申请中断时，CPU会根据中断源的轻重缓急进行裁决，优先响应更加紧急的中断源。
> - 中断嵌套：当一个中断程序正在运行时，又有新的更高优先级的中断源申请中断，CPU再次暂停当前中断程序，转而去处理新的中断程序，处理完成后依次进行返回。


stm32的F1系列总共有68个可屏蔽中断通道（中断源），包含EXTI、TIM、ADC、USART、SPI、I2C、RTC等多个外设。所有的中断使用 **嵌套向量中断控制器NVIC** 统一管理中断，每个中断通道都拥有16个可编程的优先等级，可对优先级进行分组，进一步设置抢占优先级和响应优先级。具体到某一个型号的芯片可能不会有这么多中断，具体需要查看的芯片手册。下面是手册中的中断向量表节选：

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/5-2%E4%B8%AD%E6%96%AD%E5%90%91%E9%87%8F%E8%A1%A8%E7%A4%BA%E6%84%8F%E5%9B%BE.png" width="60%">
</div><div align=center>
图5-2 中断向量表示意图
</div>

> - 地址（最后一列）：存储中断地址，这个地址列表也称为 **中断向量表**。因为程序中的中断函数地址由编译器来分配，所以中断函数地址不固定。但是由于硬件的限制，中断跳转只能跳转到固定的地址执行程序。所以为了让硬件能跳转到一个地址不固定的中断函数里，就需要在内存中定义一个固定的地址列表。当中断发生后，首先跳转到这个固定的地址列表，编译器会在这个固定的位置加上一条跳转到中断函数的代码，于是中断跳转就可以跳转到任意位置了。**C语言编程无需关注中断向量表，汇编语言需要。**

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/5-3NVIC%E5%9F%BA%E6%9C%AC%E7%BB%93%E6%9E%84.png" width="60%">
</div><div align=center>
图5-3 NVIC的基本结构
</div>

上图给出了 **嵌套向量中断控制器NVIC** 的基本结构示意图。在stm32中，NVIC用于统一管理中断和分配中断优先级，属于内核外设，是CPU的小助手，可以让CPU专注于运算。从上图可以看出：
> - NVIC有很多输入口，每个都代表一个中断线路，如EXIT、TIM、ADC等。
> - 每个中断线路上的斜杠n表示n条线，因为一个外设可能会同时占用多个中断通道。
> - NVIC只有一个输出口，通过中断优先级确定中断执行的顺序。

**NVIC的中断优先级** 由优先级寄存器的4位二进制（十进制0~15）决定，这4位可以进行切分，分为 高n位 的抢占优先级和 低(4-n)位 的响应优先级。<u>抢占优先级高</u>的可以 中断嵌套，<u>响应优先级高</u>的可以 优先排队，抢占优先级和响应优先级均相同的按 **中断号** 排队。这个中断号就是指中断向量表的第二列“优先级”。
> 用医院的叫号系统来举例子。假设医生正在给某个病人看病，外面还有很多病人排队：
> 1. 新来的病人 抢占优先级高 就相当于直接进屋打断医生，给自己看病。
> 2. 新来的病人 响应优先级高 就相当于不打扰医生，但直接插队，排在队伍的第一个。

<div align=center>
表5-1 NVIC优先级的分组方式
</div><div align=center>
<style type="text/css">
.tg  {border-collapse:collapse;border-spacing:0;}
.tg td{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  overflow:hidden;padding:10px 5px;word-break:normal;}
.tg th{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  font-weight:normal;overflow:hidden;padding:10px 5px;word-break:normal;}
.tg .tg-baqh{text-align:center;vertical-align:top}
.tg .tg-1pye{color:#000000;font-weight:bold;text-align:center;vertical-align:top}
.tg .tg-0lax{text-align:left;vertical-align:top}
</style>
<table class="tg">
<thead>
  <tr>
    <th class="tg-1pye"><span style="font-weight:bold">分组方式</span></th>
    <th class="tg-1pye"><span style="font-weight:bold">抢占优先级</span></th>
    <th class="tg-1pye"><span style="font-weight:bold">响应优先级</span></th>
  </tr>
</thead>
<tbody>
  <tr>
    <td class="tg-baqh"><span style="color:#000">分组0（n=0）</span></td>
    <td class="tg-0lax"><span style="color:#000">0位，取值为0</span></td>
    <td class="tg-0lax"><span style="color:#000">4位，取值为0~15</span></td>
  </tr>
  <tr>
    <td class="tg-baqh"><span style="color:#000">分组1（n=1）</span></td>
    <td class="tg-0lax"><span style="color:#000">1位，取值为0~1</span></td>
    <td class="tg-0lax"><span style="color:#000">3位，取值为0~7</span></td>
  </tr>
  <tr>
    <td class="tg-baqh"><span style="color:#000">分组2（n=2）</span></td>
    <td class="tg-0lax"><span style="color:#000">2位，取值为0~3</span></td>
    <td class="tg-0lax"><span style="color:#000">2位，取值为0~3</span></td>
  </tr>
  <tr>
    <td class="tg-baqh"><span style="color:#000">分组3（n=3）</span></td>
    <td class="tg-0lax"><span style="color:#000">3位，取值为0~7</span></td>
    <td class="tg-0lax"><span style="color:#000">1位，取值为0~1</span></td>
  </tr>
  <tr>
    <td class="tg-baqh"><span style="color:#000">分组4（n=4）</span></td>
    <td class="tg-0lax"><span style="color:#000">4位，取值为0~15</span></td>
    <td class="tg-0lax"><span style="color:#000">0位，取值为0</span></td>
  </tr>
</tbody>
</table>
</div>

> 注：NVIC是内核外设，更多关于NVIC的介绍参考“STM32F10xxx Cortex-M3编程手册”。
> - NVIC中断分组的配置寄存器，在SCB_AIRCR中，PRIGROUP这三位就是用于配置中断分组的。

## 5.2 STM32外部中断EXTI

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/5-4%E5%A4%96%E9%83%A8%E4%B8%AD%E6%96%AD%E5%90%91%E9%87%8F%E8%A1%A8.png" width="70%">
</div><div align=center>
图5-4 外部中断向量表
</div>

中断系统是管理和执行中断的逻辑结构，外部中断是众多能产生中断的外设之一，而EXTI就是其中之一，上图给出了外部中断的中断向量表。**EXTI（Extern Interrupt）外部中断** 可以监测指定GPIO口的电平信号，当其指定的GPIO口产生电平变化时，EXTI将立即向NVIC发出中断申请，经过NVIC裁决后即可中断CPU主程序，使CPU执行EXTI对应的中断程序。
> - 支持的触发方式：上升沿/下降沿/双边沿/软件触发。
> - 支持的GPIO口：所有GPIO口，但**相同的Pin不能同时触发中断**。
> - 通道数：16个GPIO_Pin，外加PVD输出、RTC闹钟、USB唤醒、以太网唤醒。
> > 注：后面这四个功能是为了实现一些特殊的功能，比如想实现某个时间让stm32退出停止模式，由于外部中断可以在低功耗模式的停止模式下唤醒stm32，就可以在GPIO口上连接一个RTC时钟作为外部中断。
> - 触发响应方式：中断响应/事件响应。
> > 注：中断响应就是正常的中断流程，申请中断让CPU执行中断函数；事件响应就是外部中断发生时，不把外部中断信号给CPU，而是选择触发一个事件，将这个信号通向其他外设，来触发其他外设的操作，可以实现外设之间的联合工作。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/5-4EXIT%E5%9F%BA%E6%9C%AC%E7%BB%93%E6%9E%84.png" width="65%">
</div><div align=center>
图5-5 EXTI的基本结构
</div>

> - 最左侧：GPIO口的外设，每个外设都有16个引脚。
> - AFIO中断引脚选择：本质上就是数据选择器，从前面16*n个引脚中选择16根端口号不重复的引脚出来，连接到后面的EXTI通道中。在STM32中，AFIO主要完成两个任务：复用功能引脚重映射、中断引脚选择。下面是中断引脚选择的AFIO示意图：
> <img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/5-4AFIO%E4%B8%AD%E6%96%AD%E5%BC%95%E8%84%9A%E9%80%89%E6%8B%A9%E7%BB%93%E6%9E%84.png" width="55%">
> - PVD、RTC、USB、ETH：四个特殊功能的外设。
> - EXTI边沿检测及控制：20个输入通道、两类输出。一类输出到NVIC中，并且将这20路输出的9~5、15~10路外部中断合并在一起以节省通道；另一类输出到其他外设，直接就是20路输出。
>
> 注：**上面这个EXTI的基本结构也是编写代码时的主要参考图！**

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/5-5EXIT%E6%A1%86%E5%9B%BE.png" width="70%">
</div><div align=center>
图5-6 EXTI框图（方向为从右向左）-stm32F10系列参考手册
</div>

上图给出了参考手册中的EXTI框图。基本逻辑与“EXTI的基本结构”中所述相同，另外还有一些细节：
> - 边沿检测电路+软件中断事件寄存器：这个几个进行或门输出，便可以实现“上升沿/下降沿/双边沿/软件触发”这四种触发方式。
> - 请求挂起寄存器：相当于一个中断标志位，通过读取该寄存器可以判断是哪个通道触发的中断。
> - 中断屏蔽寄存器/事件屏蔽寄存器：相当于开关，只有置1，中断信号才能继续向左走。
> - 脉冲发生器：产生一个电平脉冲，用于触发其他外设的动作。

最后一个问题，到底什么样的设备需要用到外部中断呢？
> 答：对于stm32来说，若想获取一个由外部驱动的很快的突发信号，就需要外部中断。
> - 如旋转编码器，平常不会有什么变化，但是一旦拧动时，会产生一段时间变化很快的突发信号，就需要stm32能在短时间内快速读取并处理掉这个数据。
> - 再如红外遥控接收头，平常也不会有什么变化，但是一旦接收到信号时，这个信号也是转瞬即逝的。
> - 但是**不推荐按键使用外部中断**。因为外部中断不能很好的处理按键抖动和松手检测的问题，所以要求不高时，还是建议在主函数内部循环读取。

## 5.2 旋转编码器介绍
对射式红外传感器就是一种通用传感器模块，已经在第三节“GPIO通用输入输出口”中介绍过，不再赘述。本实验只介绍旋转编码器。

**旋转编码器** 是一种用来测量位置、速度或旋转方向的装置，当其旋转轴旋转时，其输出端可以输出与旋转速度和方向对应的方波信号，读取方波信号的频率和相位信息即可得知旋转轴的速度和方向。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/5-7%E6%97%8B%E8%BD%AC%E7%BC%96%E7%A0%81%E5%99%A8%E5%AE%9E%E7%89%A9%E5%9B%BE.png" width="70%">
</div><div align=center>
图5-7 旋转编码器实物图
</div>

如上图，旋转编码器主要有三种类型：光栅式 / <u>机械触点式</u> / 霍尔传感器式。下面是这三种形式的介绍：
> 1. 光栅式（老款鼠标）：配合对射式红外传感器使用，在旋转过程中光栅编码盘会不断地 阻挡/透过 红外射线，于是模块便会输出高低电平交替的方波，方波的频率便代表了旋转速度。缺点是只有一路输出，无法判断转动方向。
> 2. 机械触点式：内部使用机械触点检测通断，A口和B口输出的方波正交，具体看下面的介绍。当然，也有机械触点式编码器可以一个引脚输出速度信息、一个引脚输出旋转方向信息。
> 3. 霍尔传感器式：直接附在电机后面的编码器，中间是一个圆形磁铁，旋转时两侧的霍尔传感器便可输出正交的方波信号。
> 4. 独立的编码器元件：输入轴转动时，输出便有波形。
>
> 注：**触点式不适合高速旋转的场景**，常用于音量调节。非接触形式的电机可以用于电机测速。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/5-8%E6%97%8B%E8%BD%AC%E7%BC%96%E7%A0%81%E5%99%A8%E5%AE%9E%E7%89%A9%E6%8B%86%E8%A7%A3.png" width="70%">
</div><div align=center>
图5-8 机械触点式旋转编码器-实物拆解
</div>

> - 图片右侧是旋转编码器的旋钮，可以看到下面是一圈可以导电的金属片。
> - 中间有一个大的按键开关结构，也可以检测通断，但是该旋转编码器模块没有使用到该功能。
> - 左右两组金属触点。内部实际的连线如红线标注，C口接地，于是旋钮在旋转过程中就可以使A口、B口输出高低交替的方波。方波频率表示旋转速度。
> - A口、B口配合旋钮，可以产生相位相差90°的方波，称为正交信号。顺时针旋转A口相位超前，逆时针旋转B口相位超前。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/5-9%E6%97%8B%E8%BD%AC%E7%BC%96%E7%A0%81%E5%99%A8%E7%A1%AC%E4%BB%B6%E7%94%B5%E8%B7%AF.png" width="70%">
</div><div align=center>
图5-9 机械触点式旋转编码器-硬件电路
</div>

> - R1、R2：上拉电阻。
> - R3、R4：输出限流电阻，防止引脚电流过大。
> - C1、C2：滤波电容，滤除高频不稳定纹波。
>  
> 注：C口已经默认接地，只需关心A口、B口的高低变化及相位差即可。

## 5.3 实验：对射式红外传感器计次
需求：利用stm32的外部中断，对 对射式红外传感器 产生的下降沿进行计次。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/5-10%E6%8E%A5%E7%BA%BF%E5%9B%BE-%E5%AF%B9%E5%B0%84%E5%BC%8F%E7%BA%A2%E5%A4%96%E4%BC%A0%E6%84%9F%E5%99%A8%E8%AE%A1%E6%AC%A1.png" width="70%">
</div><div align=center>
图5-10 对射式红外传感器计次-接线图
</div>

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/5-11%E4%BB%A3%E7%A0%81%E8%B0%83%E7%94%A8-%E7%BA%A2%E5%A4%96%E4%BC%A0%E6%84%9F%E5%99%A8%E8%AE%A1%E6%AC%A1.png" width="25%">
</div><div align=center>
图5-11 对射式红外传感器计次-代码调用
</div>

代码展示：```OLED.h```、```OLED.c```、```OLED_Font.h```代码见第四节“OLED调试工具”，本节省略。
**- main.c**
```c
#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "CountSensor.h"

int main(void){
    //OLED初始化
    OLED_Init();
    OLED_ShowString(1,1,"Neg-edge:");
    OLED_ShowNum(2,1,0,5); 
    
    CountSensor_Init();
    
    while(1){
        OLED_ShowNum(2,1,CountSensor_Get(),5);  
    };
}

```

**- CountSensor.h**
```c
#ifndef __COUNTERSENSOR_H
#define __COUNTERSENSOR_H

void CountSensor_Init(void);
uint16_t CountSensor_Get(void);

#endif

```

**- CountSensor.c**
```c
#include "stm32f10x.h"                  // Device header

uint16_t CountSensor_Count = 0;//中断触发次数

/**
  * @brief  对射式红外传感器起初始化-PB14
  */
void CountSensor_Init(void){
    //EXIT初始化
    //1. 开启GPIO、AFIO的外设时钟（EXTI和NVIC的时钟是一直打开的）
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
    //2. 配置GPIO-PB14上拉输入
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;//上拉输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    //3. 配置AFIO（库函数在GPIO中）
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource14);//数据选择器
    //4. 配置NVIC
    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Line = EXTI_Line14;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//下降沿触发
    EXTI_Init(&EXTI_InitStructure);
    //5. 配置NVIC（库函数在misc.h文件中）
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//配置中断的优先级分组，每个工程只能出现一次！！
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  输出中断触发的次数
  * @retvl  无符号16位整型，范围0~65535
  */
uint16_t CountSensor_Get(void){
    return CountSensor_Count;
}

//中断函数的名字从启动文件“stratup_stm32f10x_md”中来
//中断函数都是无参无返回值的
void EXTI15_10_IRQHandler(void){
    //中断标志位判断
    if(EXTI_GetITStatus(EXTI_Line14)==SET){
        CountSensor_Count++;
        EXTI_ClearITPendingBit(EXTI_Line14);//清除中断标志位
    }
}

```

编程感想：
> 1. 下降沿触发（移除遮挡触发）。传感器无遮挡时，DO输出低电平；传感器有遮挡时，DO输出高电平。所以放入遮挡意味着上升沿，移除遮挡相当于下降沿。**采用上升沿触发计数可能不准确**，下降沿触发计数准确。
> 2. 中断函数的名字从启动文件“stratup_stm32f10x_md”中来，并且中断函数都是无参无返回值的。

## 5.4 实验：旋转编码器计次
需求：利用stm32的外部中断，对旋转编码器的转动进行计次，顺时针加、逆时针减，并显示在OLED显示屏上。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/5-12%E6%8E%A5%E7%BA%BF%E5%9B%BE-%E6%97%8B%E8%BD%AC%E7%BC%96%E7%A0%81%E5%99%A8%E8%AE%A1%E6%AC%A1.png" width="75%">
</div><div align=center>
图5-12 旋转编码器计次-接线图
</div>

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/5-13%E4%BB%A3%E7%A0%81%E8%B0%83%E7%94%A8-%E6%97%8B%E8%BD%AC%E7%BC%96%E7%A0%81%E5%99%A8%E8%AE%A1%E6%AC%A1.png" width="25%">
</div><div align=center>
图5-13 旋转编码器计次-代码调用（除库函数以外）
</div>

代码展示：```OLED.h```、```OLED.c```、```OLED_Font.h```代码见第四节“OLED调试工具”，本节省略。
**- main.c**
```c
#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "RotaryEncoder.h"

int main(void){
    //配置中断的优先级分组，每个工程只能出现一次！！
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    //OLED显示屏初始化
    OLED_Init();
    OLED_ShowString(1,1,"RE_Count:");
    OLED_ShowSignedNum(2,1,0,5);
    
    //传感器初始化
    RotaryEncoder_Init();
    
    while(1){
        OLED_ShowSignedNum(2,1,RotaryEncoder_GetCount(),5);
        if(RotaryEncoder_GetChange()==1)      {OLED_ShowString(3,1,"Clockwise.     ");}
        else if(RotaryEncoder_GetChange()==-1){OLED_ShowString(3,1,"anti-Clockwise.");}
    };
}

```

**- RotaryEncoder.h**
```c
#ifndef __ROTARYENCODER_H
#define __ROTARYENCODER_H

void RotaryEncoder_Init(void);
int16_t RotaryEncoder_GetCount(void);
int16_t RotaryEncoder_GetChange(void);

#endif

```

**- RotaryEncoder.c**
```c
#include "stm32f10x.h"                  // Device header

//旋转编码器计次
int16_t RotaryEncoder_Count_cur = 0;
int16_t RotaryEncoder_Count_pre = 0;

/**
  * @brief  旋转编码器（Rotary Encoder）初始化-A口PB0、B口PB1
  */
void RotaryEncoder_Init(void){
    //开启外设时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    
    //配置GPIOB-PB0、PB1上拉输入
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    //配置AFIO（库函数在GPIO中）
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource0);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource1);
    
    //配置EXTI
    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Line = EXTI_Line0 | EXTI_Line1;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//下降沿触发
    EXTI_Init(&EXTI_InitStructure);
    
    //配置NVIC（库函数在misc.h文件中）
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStructure);
    NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_Init(&NVIC_InitStructure);    
}

/**
  * @brief  获取从程序复位开始的计次
  * @retvl  int16_t变量，范围-32768~32767
  */
int16_t RotaryEncoder_GetCount(void){
    return RotaryEncoder_Count_cur;
}

/**
  * @brief  获取状态变化值
  * @retvl  int16_t变量，-1表示逆时针转、0初始化状态、1表示顺时针转
  */
int16_t RotaryEncoder_GetChange(void){
    return (RotaryEncoder_Count_cur - RotaryEncoder_Count_pre);
}

/**
  * @brief  A口下降沿中断函数
  */
void EXTI0_IRQHandler(void){
    if(EXTI_GetITStatus(EXTI_Line0)==SET){
        if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1)==0){
            RotaryEncoder_Count_pre = RotaryEncoder_Count_cur;
            RotaryEncoder_Count_cur--;//B口超前减计数
        }
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

/**
  * @brief  B口下降沿中断函数
  */
void EXTI1_IRQHandler(void){
    if(EXTI_GetITStatus(EXTI_Line1)==SET){
        if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0)==0){
            RotaryEncoder_Count_pre = RotaryEncoder_Count_cur;
            RotaryEncoder_Count_cur++;//A口超前加计数
        }
        EXTI_ClearITPendingBit(EXTI_Line1);
    }
}

```

编程感想：
> 1. 管理Hardware文件夹。本次实验继承的是“OLED显示屏”实验的代码，而非“对射式红外传感器计次”。猜测是因为驱动文件的命名不规范，本人的实验文件均按照模块的英文名来命名。
> 2. 注意每个模块在使用的时候都要进行初始化。
> 3. 注意进入中断函数的时候要检查中断标志位，退出的时候清零中断标志位。
> 4. 注意主函数和中断函数不要操控同一个硬件，避免不必要的硬件冲突。中断函数一般执行简短快速的代码，如操作中断标志位等。

