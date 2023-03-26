# 6 TIM定时器 
 [toc]

注：笔记主要参考B站 [江科大自化协](https://space.bilibili.com/383400717) 教学视频“[STM32入门教程-2023持续更新中](https://www.bilibili.com/video/BV1th411z7sn/)”。
注：工程及代码文件放在了本人的[Github仓库](https://github.com/jjejdhhd/Learn_stm32f103/tree/main)。
***

定时器是STM32中功能最强大、结构最复杂的一个外设。定时器将包括四部分8小节：

1. 第一部分主要讲定时器基本定时的功能，也就是指定一个时间，让定时器每个这段时间就产生一个中断，如实现时钟、秒表等。
2. 第二部分主要讲定时器输出比较的功能。输出比较模块最常见的用途就是产生PWM波，用于驱动电机、舵机等设备。
3. 第三部分主要讲定时器输入捕获的功能。将会学习使用输入捕获模块测量方波频率。
4. 第四部分主要讲定时器的编码器接口。使用编码器接口，可以方便的读取正交编码器的输出波形，广泛应用在编码电机测速中。

## 6.1 TIM定时中断原理
**TIM（Timer）定时器** <u>最基本的功能</u> 是对输入的时钟进行计数，并在计数值达到设定值时触发中断。若这个输入是一个可靠的基准时钟，那么对这个基准时钟技术就实现了计时的功能。
> 如STM32中主频一般是72MHz，那么计数值设定为72，就是每1us触发一次中断；计数值设定为72000，就是每1ms触发一次中断。

STM32定时器拥有由16位 **计数器、预分频器、自动重装寄存器** 组成的时基单元，在72MHz计数时钟下可以实现最大59.65s的定时。TIM不仅具备基本的定时中断功能，而且还包含内外时钟源选择、输入捕获、主从触发模式、输出比较、编码器接口等多种功能，并且根据复杂度和应用场景分为了高级定时器、通用定时器、基本定时器三种类型。


<div align=center>
表6-1 定时器类型
</div><div align=center>
<style type="text/css">
.tg  {border-collapse:collapse;border-spacing:0;}
.tg td{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  overflow:hidden;padding:10px 5px;word-break:normal;}
.tg th{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  font-weight:normal;overflow:hidden;padding:10px 5px;word-break:normal;}
.tg .tg-cly1{text-align:left;vertical-align:middle}
.tg .tg-c4ze{color:#000000;font-weight:bold;text-align:center;vertical-align:middle}
</style>
<table class="tg">
<thead>
  <tr>
    <th class="tg-c4ze"><span style="font-weight:bold">类型</span></th>
    <th class="tg-c4ze"><span style="font-weight:bold">编号</span></th>
    <th class="tg-c4ze"><span style="font-weight:bold">总线</span></th>
    <th class="tg-c4ze"><span style="font-weight:bold">功能</span></th>
  </tr>
</thead>
<tbody>
  <tr>
    <td class="tg-cly1"><span style="color:#000">高级定时器【最复杂】</span></td>
    <td class="tg-cly1"><span style="color:#000">TIM1、TIM8</span></td>
    <td class="tg-cly1"><span style="color:#000">APB2</span></td>
    <td class="tg-cly1"><span style="color:#000">拥有通用定时器全部功能，并额外具有重复计数器、死区生成、互补输出、刹车输入等功能</span></td>
  </tr>
  <tr>
    <td class="tg-cly1"><span style="color:#000">通用定时器【最常用】</span></td>
    <td class="tg-cly1"><span style="color:#000">TIM2、TIM3、TIM4、TIM5</span></td>
    <td class="tg-cly1"><span style="color:#000">APB1</span></td>
    <td class="tg-cly1"><span style="color:#000">拥有基本定时器全部功能，并额外具有内外时钟源选择、输入捕获、输出比较、编码器接口、主从触发模式等功能</span></td>
  </tr>
  <tr>
    <td class="tg-cly1"><span style="color:#000">基本定时器【最简单】</span></td>
    <td class="tg-cly1"><span style="color:#000">TIM6、TIM7</span></td>
    <td class="tg-cly1"><span style="color:#000">APB1</span></td>
    <td class="tg-cly1"><span style="color:#000">拥有定时中断、主模式触发DAC的功能</span></td>
  </tr>
</tbody>
</table>
</div>

> - 只有高级定时器连接的是性能最高的APB2总线，剩下的通用定时器和基本定时器都是APB1总线。
> - 高级定时器额外多出的功能主要是为了三相无刷电机的驱动设计的，本课程不会涉及到。
> - 不同型号芯片的定时器数量不同，stm32f103c8t6定时器资源：**TIM1、TIM2、TIM3、TIM4**。没有基本定时器。

下面依次介绍上面三种类型的定时器。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/5-1%E5%9F%BA%E6%9C%AC%E5%AE%9A%E6%97%B6%E5%99%A8%E6%A1%86%E5%9B%BE.png" width="65%">
</div><div align=center>
图6-1 基本定时器框图
</div>

> - 内部时钟（CK_CNT）：一般就是系统的主频72MHz，通向时基单元的输入。
> - 时基单元：16位预分频器 + 16位计数器 + 16位自动重装载寄存器。
> > 1. 预分频器：对输入的72MHz时钟进行预分频，寄存器内存储的值是实际的分频系数减一。写0就是不分频，写1就是2分频，写2就是3分频……
> > 2. 计数器：对预分频后的计数时钟进行计数，每遇到上升沿就加一。
> > 3. 自动重装载寄存器：存储计数的最大值，到达此值后触发中断并清零计数器。
> - 折线UI：向上的折线箭头表示该位置会产生中断信号——“更新中断”（由计数值等于自动重装值产生的中断），这个中断信号会通向NVIC。
> - 折线U：向下的折线箭头表示该位置会产生事件——“更新事件”，这个更新事件不会触发中断，但可以触发内部其他电路的工作。
> 
> 主模式触发DAC：
> - stm32的一大特色就是主从触发模式，可以让内部的硬件在不受程序的控制下自动运行，可以极大地减轻CPU的负担。
> - 驱动DAC正常思路及其问题：每隔一段时间就产生一个定时中断，手动更新DAC的值。但这样子会频繁的产生中断，会影响主程序的运行和其他中断的响应。
> - 解决方法：定时器设计了一个主模式，使用主模式可以将定时器的“更新事件”映射到“触发输出TRGO”，然后TRGO直接接到DAC的触发转换引脚上，于是定时器的更新就不需要中断来实现了。整个过程无需软件参与，实现了硬件的自动化。
>
> 注：除了主模式外，还有更多的硬件自动化设计。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/5-2%E9%80%9A%E7%94%A8%E5%AE%9A%E6%97%B6%E5%99%A8%E6%A1%86%E5%9B%BE.png" width="80%">
</div><div align=center>
图6-2 通用定时器框图
</div>

> - 时基单元：中间的PSC预分频器、自动重装载寄存器、CNT计数器。通用定时器和高级定时器新增两个功能——“向下计数模式”、“中央计数模式”。
> > - 向上计数模式【常用】：从0开始累加，到自动重装载值触发中断。
> > - 向下计数模式：从自动重装载值递减，到0触发中断。
> > - 中央对齐模式：从0开始累加，到自动重装载值触发中断，然后递减，到0再次触发中断。常用于电机控制的SVPWM算法中。
> - 内外时钟源选择和主从触发模式结构：上面的一大块。下面介绍各种各样的**内外时钟源**：
> > 1. 内部时钟CK_INT【常用】：通常为72MHz，基本定时器只能选择CK_INT，通用定时器和高级定时器则新增了下面的时钟源。
> > 2. 外部TIMx_ETR【常用】：引脚的位置可以参考引脚定义表，如stm32f103c8t6的PA0引脚复用了TIM2_CH1_ETR等。外部输入了方波时钟，然后通过极性选择、滤波等电路进行整形，然后兵分两路，一路ETRF进入触发控制器，紧跟着就可以选择成为时基单元的时钟（**外部时钟模式2**）；一路进入选择器等待成为TRGI。
> > - TRGI主要用作触发输入来使用，可以触发定时器的从模式，本小节仅用做外部时钟（**外部时钟模式1**），其他功能后续再讲。
> > 3. 外部ITR信号：包括ITR0~ITR4（引脚定义见参考手册“表78 TIMx内部触发连接”），来自其他定时器，实现了定时器的级联。这个ITR信号从上一级定时器的主模式TRGO引脚来（图片右上方）。
> > 4. 外部TI1F-ED：来自于输入捕获单元的TIMx_CH1引脚，后缀“ED”意为边沿，也就是说该路时钟的上升沿和下降沿均有效，也就是CH1引脚的边沿。
> > 5. 外部TI1FP1：来自CH1引脚时钟。
> > 6. 外部TI2FP2：来自CH2引脚时钟。
> > - 编码器接口：可以读取正交编码器的输出波形，后续会再介绍。
> > - TRGO引脚：**定时器的主模式输出**，可以将内部的一些事件映射到TRGO上，相比基本定时器这些事件的范围显然更广。
> > 
> > 注：最后三种外部时钟用于输入捕获和测频率，后续介绍。
> 
> - 输出比较电路：下面右侧的一大堆，总共有4个通道，可以用于输出PWM波形驱动电机。
> - 输入捕获电路：下面左侧的一大推，也是有4个通道，可以用于测量输入方波的频率。
> 
> 注：输入捕获电路和输出比较电路不能同时使用，所以共用中间的“捕获/比较寄存器”以及输入/输出的引脚。后续再介绍。
> 

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/5-3%E9%AB%98%E7%BA%A7%E5%AE%9A%E6%97%B6%E5%99%A8%E6%A1%86%E5%9B%BE1.png" width="80%">
</div><div align=center>
图6-3 高级定时器框图
</div>

> 相比于通用定时器，高级定时器主要增加了以下功能：对输出比较模块的升级
> - 重复次数计数器：可以实现每个几个计数周期，才发生一次更新事件和更新中断，相当于自带一级的定时器级联。
> - DTG（Dead Time Generate）：死区生成电路。将输出引脚由原来的一个变为两个互补的输出，可以输出一对互补的有死区的PWM波（防止出现短暂的直通现象），可以驱动三相无刷电机（如四轴飞行器、电动车后轮、电钻等）。
> - BRK刹车输入：为了给电机驱动提供安全保障，如果外部引脚BKIN（Break IN）产生了刹车信号或者内部时钟失效，这个电路就会自动切断电机的输出，防止意外的发生。


<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-4%E5%AE%9A%E6%97%B6%E4%B8%AD%E6%96%AD%E5%9F%BA%E6%9C%AC%E7%BB%93%E6%9E%84%E5%9B%BE.png" width="80%">
</div><div align=center>
图6-4 定时中断基本结构图
</div>

> 上图是UP主自己画的定时中断基本结构图，去掉了一些无关的东西并加了一些定时器框图中没有体现的模块，后续在配置TIM时可以直接参考本图。
> - 时基单元：中间的粉色部分。
> - 运行控制：控制寄存器的一些位，如启动停止、向上或向下计数等，操作这些寄存器就可以控制时基单元的运行了。
> - 内部时钟模式、外部时钟模式2、外部时钟模式1：外部时钟源选择。这个选择器的输出就是为时基单元提供时钟。
> - 编码器模式：编码器独有的模式，一般用不到。
> - 中断申请控制：由于定时器内部有很多地方要申请中断，“中断申请控制”就用来使能控制这些中断是否使能。比如，中断信号会先在状态寄存器里置一个中断标志位，这个标志位会通过中断输出控制，到NVIC申请中断。

**上面三个框图中，有阴影的部分都是包含缓冲寄存器（影子寄存器）的**，具体作用见下面时基单元运行时的一些细节：

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-5%E9%A2%84%E5%88%86%E9%A2%91%E5%99%A8%E6%97%B6%E5%BA%8F.png" width="60%">
</div><div align=center>
图6-5 预分频器时序-2分频
</div>

> - CK_PSC：预分频器的输入时钟，选择内部时钟源就是72MHz。
> - CNT_EN：计数器使能。高电平计数器正常运行，低电平计数器停止。
> - CK_CNT：计数器时钟，既是预分频器的时钟输出，也是计数器的时钟输入。
> - 计数器寄存器：对CK_CNT进行自增计数，到达自动重装载值清零。
> - 更新事件：计数器寄存器到达自动重装载值时产生一个脉冲。
>
> 下面三行时序体现了预分频计数器的一种缓冲机制：
>
> - 预分频控制寄存器：供用户读写使用，实时响应用户控制，但并不直接决定分频系数。
> - 预分频缓冲器：也称为影子寄存器，真正起分频作用的寄存器。只有在更新事件到达时，才从“预分频控制寄存器”更新预分频参数。以确保更新事件的稳定性。
> - 预分频计数器：按照预分频参数进行计数，以产生对应的CK_CNT脉冲。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-6%E8%AE%A1%E6%95%B0%E5%99%A8%E6%97%A0%E9%A2%84%E8%A3%85%E6%97%B6%E5%BA%8F.png" width="60%">
</div><div align=center>
图6-6 计数器无预装时序
</div>

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-7%E8%AE%A1%E6%95%B0%E5%99%A8%E6%9C%89%E9%A2%84%E8%A3%85%E6%97%B6%E5%BA%8F.png" width="60%">
</div><div align=center>
图6-7 计数器有预装时序
</div>

> 正常的计数器时序没啥好说的，就是根据CK_CNT计数，到达自动重装载值产生中断，所以只需看一下更新自动重装载值的过程：
> - 无预装时序（禁用缓冲寄存器）：有溢出问题。若将自动重装载值变小，且此时计数器寄存器已经超过这个新的重装载值，那么计数器寄存器就会一直计数到FFFF才清零。这可能会造成一些问题。
> - 有预装时序（启用缓冲寄存器）：比较稳定。只有更新事件来临时才更新自动重装载值。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-8%E6%97%B6%E9%92%9F%E6%A0%91.png" width="60%">
</div><div align=center>
图6-8 RCC时钟树
</div>

> 时钟是所有外设的基础，所以是需要最先配置的。ST公司写好了```SystemInit```函数来配置时钟树，下面具体介绍：
> - 左侧是**时钟产生电路**，右侧**时钟分配电路**，中间的SYSCLK就是72MHz的系统时钟。
> 
> - 时钟产生电路：
> > - 四个振荡源：
> > > 1. 内部的8MHz高速RC振荡器。
> > > 2. 外部的4-16MHz高速石英晶体振荡器：一般8MHz，相比于内部的RC高速振荡器更加稳定。
> > > 3. 外部的32.768kHz低速晶振：一般给RTC提供时钟。
> > > 4. 内部的40kHz低速RC振荡器：给看门狗提供时钟。
> > > 
> > > 上面的两个高速晶振用于给系统提供时钟，如AGB、APB1、APB2的时钟。
> > - SystemInit函数配置时钟的过程：首先启动内部8MHz时钟为系统时钟，然后配置外部8MHz时钟到PLLMUL模块进行9倍频到72MHz，等到这个72MHz时钟稳定后，再将其作为系统时钟。于是就实现了系统时钟从8MHz切换到72MHz。
> > - CSS：监测外部时钟的运行状态，一旦外部时钟失效，就会自动把外部时钟切换成内部时钟，保障系统时钟的运行，防止程序卡死造成事故。如果外部晶振出问题，那么就会导致程序的时钟变为8MHz，也就是比预期的时钟慢了9倍。
>
> - 时钟分配电路
> > - AHB总线：有预分频器，SysytemInit配置分频系数为1，于是AHB时钟输出就是72MHz。
> > - APB1总线：SysytemInit配置分频系数为2，于是APB1时钟输出就是36MHz。
> > - APB2总线：SysytemInit配置分频系数为1，于是APB2时钟输出就是72MHz。
> > - 外设时钟使能：就是库函数```RCC_APB2PeriphClockCmd```开启的地方，可以控制相应的外设时钟开启。
> > - 定时器的时钟：从图中可以看出，按照SystemInit的默认配置，所有的定时器时钟都是72MHz。











## 6.2 TIM定时中断相关实验
### 6.2.1 实验：定时器定时中断-内部时钟
需求：在OLED显示屏上显示数字，每秒自动加一。
<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-9%E6%8E%A5%E7%BA%BF%E5%9B%BE-%E5%AE%9A%E6%97%B6%E5%99%A8%E4%B8%AD%E6%96%AD-%E5%86%85%E9%83%A8%E6%97%B6%E9%92%9F.png" width="70%">
</div><div align=center>
图6-9 定时器定时中断-内部时钟-接线图
</div>

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-10%E4%BB%A3%E7%A0%81%E8%B0%83%E7%94%A8-%E5%AE%9A%E6%97%B6%E5%99%A8%E4%B8%AD%E6%96%AD-%E5%86%85%E9%83%A8%E6%97%B6%E9%92%9F.png" width="25%">
</div><div align=center>
图6-10 定时器定时中断-内部时钟-代码调用（非库函数）
</div>

下面是代码展示：
**- main.c**
```c
#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "Timer.h"

uint16_t TimerCount = 0;

int main(void){
    //配置中断的优先级分组，每个工程只能出现一次！！
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    //OLED显示屏初始化
    OLED_Init();
    OLED_ShowString(1,1,"TIM_Interrupt:");
    OLED_ShowNum(2,1,0,5);
    
    //定时器初始化
    Timer_Init();
    
    while(1){
        OLED_ShowNum(2,1,TimerCount,5);
    };
}

//TIM2定时中断后的中断函数
void TIM2_IRQHandler(void){
    if(TIM_GetITStatus(TIM2,TIM_IT_Update)==SET){
        TimerCount++;
        TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
    }
}

```

**- Timer.h**
```c
#ifndef __TIMER_H
#define __TIMER_H

void Timer_Init(void);

#endif

```

**- Timer.c**
```c
#include "stm32f10x.h"                  // Device header

// 定时器初始化-TIM2
void Timer_Init(void){
    //1.初始化RCC内部时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    //2.选择时基单元的时钟
    TIM_InternalClockConfig(TIM2);//默认使用内部时钟，也可以不写
    //3.配置时基单元
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;//外部时钟源的输入滤波器采样频率，内部时钟无所谓
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;//加计数
    TIM_TimeBaseInitStructure.TIM_Period = 10000-1;//ARR自动重装器的值10000
    TIM_TimeBaseInitStructure.TIM_Prescaler = 7200-1;//PSC预分频的值7200
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;//重复计数器的值（高级定时器才有）
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);
    TIM_ClearFlag(TIM2,TIM_FLAG_Update);//消除上一行TIM_TimeBaseInit立刻产生更新事件影响
    //4.配置中断输出控制
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    //5.配置NVIC
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2 ;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStructure);
    //6.配置运行控制
    TIM_Cmd(TIM2, ENABLE);    
}

/*
//TIM2定时中断后的中断函数
void TIM2_IRQHandler(void){
    if(TIM_GetITStatus(TIM2,TIM_IT_Update)==SET){
        ??    
        TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
    }
}
*/

```


编程感想：
> 1. 由于TIM中断与其他外部硬件没有关系，所以就直接放在了System文件夹。
> 2. 复位后计数器的值从1开始而不是从0开始，说明上电初始化后TIM2就立刻中断了一次。这是因为时基单元初始化函数```TIM_TimeBaseInit```，在函数的最后生成了一个更新事件，来保证可以立刻重新装载预分频器和重复计数器的值。要消除这个影响，就在```TIM_TimeBaseInit```后面加一句```TIM_ClearFlag```来清除相应的中断标志位。

### 6.2.2 实验：定时器定时中断-外部时钟
需求：对外部输入的方波（对射式红外传感器）进行计次，每出现9个方波就自动加一。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-11%E6%8E%A5%E7%BA%BF%E5%9B%BE-%E5%AE%9A%E6%97%B6%E5%99%A8%E4%B8%AD%E6%96%AD-%E5%A4%96%E9%83%A8%E6%97%B6%E9%92%9F.png" width="75%">
</div><div align=center>
图6-11 定时器定时中断-外部时钟-接线图
</div>

“定时器定时中断-外部时钟”与“定时器定时中断-内部时钟”的 **代码调用关系相同**。下面是代码展示：

**- main.c**
```c
#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "Timer.h"

uint16_t TimerCount = 0;

int main(void){
    //配置中断的优先级分组，每个工程只能出现一次！！
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    //OLED显示屏初始化
    OLED_Init();
    OLED_ShowString(1,1,"TimerCount:");
    OLED_ShowNum(2,1,0,5);
    OLED_ShowString(3,1,"CNT:");
    OLED_ShowNum(3,5,0,5);
    
    //定时器初始化
    Timer_Init();
    
    while(1){
        OLED_ShowNum(2,1,TimerCount,5);
        OLED_ShowNum(3,5,TIM_GetCounter(TIM2),5);
    };
}

//TIM2定时中断后的中断函数
void TIM2_IRQHandler(void){
    if(TIM_GetITStatus(TIM2,TIM_IT_Update)==SET){
        TimerCount++;
        TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
    }
}

```

**- Timer.h**
```c
#ifndef __TIMER_H
#define __TIMER_H

void Timer_Init(void);

#endif

```

**- Timer.c**
```c
#include "stm32f10x.h"                  // Device header

// 定时器初始化-TIM2
void Timer_Init(void){
    //1.初始化RCC内部时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    //2.配置GPIO-PA0
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//虽然器件手册推荐浮空输入，但上拉输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    //3.选择时基单元的时钟-ETR外部时钟模式2
    TIM_ETRClockMode2Config(TIM2, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_Inverted, 0x0F);
    //4.配置时基单元
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;//外部时钟源的输入滤波器采样频率，内部时钟无所谓
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;//加计数
    TIM_TimeBaseInitStructure.TIM_Period = 10-1;//ARR自动重装器的值
    TIM_TimeBaseInitStructure.TIM_Prescaler = 1-1;//PSC预分频的值
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;//重复计数器的值（高级定时器才有）
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);
    TIM_ClearFlag(TIM2,TIM_FLAG_Update);//消除上一行TIM_TimeBaseInit立刻产生更新事件影响
    //5.配置中断输出控制
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    //6.配置NVIC
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2 ;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStructure);
    //7.配置运行控制
    TIM_Cmd(TIM2, ENABLE);    
}

/*
//TIM2定时中断后的中断函数
void TIM2_IRQHandler(void){
    if(TIM_GetITStatus(TIM2,TIM_IT_Update)==SET){
        ??    
        TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
    }
}
*/

```

编程感想：
> 1. PA0配置成上拉输入：虽然手册里写TIM2的ETR时钟配置成浮空输入，但UP主说不喜欢浮空输入，因为这会导致输入电平跳个没完。只有当外部输入信号的功率很小，内部的上拉的电阻可能会影响到输入信号，此时采用浮空输入防止影响外部输入的电平。其他情况一律上拉输入。
> 2. 关于时钟源选择函数 ```TIM_ETRClockMode2Config```：第三个参数设置极性，其实就是规定在外部时钟的 上升沿/下降沿 计数；第四个参数滤波器，就是设置对于外部时钟的采样情况，具体的含义可以参考器件手册“14.4.3 从模式控制寄存器(TIMx_SMCR)”中的位11:8。











## 6.3 TIM输出比较原理
TIM的 **OC（Output Compare）输出比较** 主要用于输出PWM波形，PWM又是驱动电机的必要条件（智能车、机器人等），所以应用广泛。**输出比较功能** 可以通过比较 **CNT计数器** 与 **CCR捕获/比较寄存器** （见图6-2“通用定时器框图”）的大小，来对输出电平进行置1、置0或翻转的操作，用于输出一定频率和占空比的PWM波形。
> - 每个高级定时器和通用定时器都拥有4个输出比较通道。
> - 高级定时器的前3个通道额外拥有死区生成和互补输出的功能。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-12PWM%E7%A4%BA%E6%84%8F%E5%9B%BE.png" width="50%">
</div><div align=center>
图6-12 PWM原理示意图
</div>

简单介绍一下PWM，即**PWM（Pulse Width Modulation）脉冲宽度调制**。在 **具有惯性的系统** 中，可以通过对一系列脉冲的宽度进行调制，来等效地输出的模拟参量，常应用于电机控速等领域。PWM参数：
> - 频率 = 1 / T~S~，一般在 几kHz\~几十kHz。
> - 占空比 = T~ON~ / T~S~
> - 分辨率 = 占空比变化步距，也就是占空比变化的精细程度。一般1%足够使用。
> 
> 注：**定时中断的频率就是PWM波的频率，只不过占空比的变化范围由自动重装载值决定。**


有关“通用定时器框图”已经在6.1节“TIM定时中断原理”介绍过，下面来具体介绍其中的输出比较通道的电路结构。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-12%E9%80%9A%E7%94%A8%E5%AE%9A%E6%97%B6%E5%99%A8-%E8%BE%93%E5%87%BA%E6%AF%94%E8%BE%83%E7%94%B5%E8%B7%AF.png" width="60%">
</div><div align=center>
图6-13 捕获/比较通道的输出部分-通用定时器
</div>

> - 左侧输入：CNT和CCR比较的结果。
> - ETRF：定时器的小功能，一般不用，无需了解。
> - 输出模式控制器：CNT>=CCR1时，输出模式控制器收到信号并输出oc1ref。输出比较的8种模式如下：
> <img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-13%E8%BE%93%E5%87%BA%E6%AF%94%E8%BE%83%E7%9A%848%E7%A7%8D%E6%A8%A1%E5%BC%8F.png" width="70%">
> 见stm32参考手册“14.3.7强制输出模式”、“14.3.8输出比较模式”、“14.3.9PWM模式”三节。
> - 主模式控制器：可以将oc1ref映射到主模式的TRGO输出上去。
> - CC1P：极性选择。选择是否需要将oc1ref的高低电平翻转一下。
> - 输出使能电路：由CC1E选择要不要输出。
> - OC1：后续通过TIMx_CH1输出到GPIO引脚上。


<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-14%E9%AB%98%E7%BA%A7%E5%AE%9A%E6%97%B6%E5%99%A8-%E8%BE%93%E5%87%BA%E6%AF%94%E8%BE%83%E7%94%B5%E8%B7%AF.png" width="60%">
</div><div align=center>
图6-14 捕获/比较通道的输出部分-高级定时器
</div>

> 相比于“通用定时器”，“高级定时器”的输出比较电路增加了“死区生成器”和“互补输出电路”。
> - 死区生成器：消除上下两路可能同时导通的短暂状态，以防止电路发热、功率损耗。
> - 互补输出OC1N：与OC1反相，用于驱动三相无刷电机。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-15PWM%E5%9F%BA%E6%9C%AC%E7%BB%93%E6%9E%84.png" width="70%">
</div><div align=center>
图6-15 PWM基本结构-通用定时器
</div>

> 上图与“定时器中断”的区别在于最后输出的时候不需要“更新事件”的中断申请，而是走输出比较电路。
> 下面是一些参数计算：
> - PWM频率：	Freq = CK_PSC / (PSC + 1) / (ARR + 1)
> - PWM占空比：	Duty = CCR / (ARR + 1)
> - PWM分辨率：	Reso = 1 / (ARR + 1)












## 6.4 TIM输出比较相关实验
### 6.4.1 舵机简介
<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-16%E8%88%B5%E6%9C%BA%E5%AE%9E%E7%89%A9%E5%9B%BE.png" width="70%">
</div><div align=center>
图6-16 舵机实物图
</div>

舵机是一种根据输入PWM信号 **占空比来控制输出角度** 的装置。
> - 型号：SG90。
> - 三根输入线：棕色是电源负、红色是电源正、橙色是PWM信号线。注意这个颜色因型号不同可能不同，需要查看说明手册。
> - 输入PWM信号要求：周期为20ms(50Hz)，高电平宽度为0.5ms\~2.5ms（占空比2.5%\~12.5%），脉冲控制精度为2us(0.18°，占空比精度0.01%)。
> - 内部电板的基本思路：根据输入占空比得到期望角度，然后检测当前角度，若当前角度较小则顺时针转；反之则逆时针转，直到与期望角度相同。
> 
> 注：**这里实际上是将PWM当作一种通信协议，而不是模拟输出。** 
> 更多细节可以参考CSDN文章 “[SG90舵机的使用](https://blog.csdn.net/weixin_43148648/article/details/113447204)”。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-17%E8%88%B5%E6%9C%BA%E7%94%B5%E8%B7%AF%E5%9B%BE.png" width="40%">
</div><div align=center>
图6-17 舵机电路图
</div>

> - GND：stm32地。
> - 5V电源线：舵机属于大功率设备，驱动电源也期望是大功率的输出设备，驱动电源要注意和stm32开发板共地。对于本实验来说，可以使用STLINK的5V输出引脚进行供电，属于USB供电符合功率要求，另外要求不严格也可以不共地。
> - PWM：作为通信线无需大功率，可以连接到stm32的某个引脚，如PA0。

### 6.4.2 直流电机简介
<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-18%E5%AE%9E%E7%89%A9%E5%9B%BE-%E7%9B%B4%E6%B5%81%E7%94%B5%E6%9C%BA%E5%8F%8A%E9%A9%B1%E5%8A%A8%E8%8A%AF%E7%89%87.png" width="56%">
</div><div align=center>
图6-18 直流电机及驱动芯片（H桥电路）实物图
</div>

直流电机是一种将电能转换为机械能的装置。
> - 型号：130直流电机。
> - 直流电机两个引脚：当电极正接时，电机正转；当电极反接时，电机反转。

直流电机属于大功率器件，GPIO口无法直接驱动，需要配合 **电机驱动电路** 来操作，如TB6612、DRV8833、L9110、L298N等。另外还有一些用分离元件（如MOS）做的驱动电路，支持更大的驱动功率。本实验采用的 **TB6612** 是一款双路H桥型的直流电机驱动芯片，可以驱动两个直流电机并且控制其转速和方向。而有些芯片（如ULN2003）一路就只有一个开关管，所以只能控制电机在一个方向转，选择电机的时候注意区分。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-19%E7%94%B5%E8%B7%AF%E5%9B%BE-TB6612.png" width="65%">
</div><div align=center>
图6-19 TB6612电路图
</div>

> - VM：电机电源的正极，范围4.5V~10V。要接一个可以输出大电流的电源，且电压一般与电机的额定电压保持一致。
> - VCC：逻辑电平输入端，范围2.7V~5.5V。这个要与控制控制器的电源保持一致，所以采用stm32就是3.3V、采用51单片机就是5V。
> - 三个GND：都是电源负极，随便选一个接地即可。
> - STBY：Stand by，待机控制引脚。接地，芯片处于待机状态不工作；接逻辑电源VCC，芯片正常工作。
> - PWMA、AIN1、AIN2：接在单片机GPIO引脚上，用于控制电机，控制逻辑如上图。PWMA接PWM波，AIN1、AIN2可以任意接普通的GPIO口。
> - AO1、AO2：按照控制逻辑，从VM汲取电流来驱动电机。
> - PWMB、BIN1、BIN2与BO1、BO2：同上述，控制另一个电机的转动。
>
> 注：根据逻辑控制真值表，**这里的PWM就是用来等效成一个模拟量**。



### 6.4.3 实验：PWM驱动呼吸灯-引脚重映射
需求：实现0.5s逐渐亮、0.5s逐渐灭的呼吸灯，PA0高电平驱动。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-20%E6%8E%A5%E7%BA%BF%E5%9B%BE-PWM%E5%91%BC%E5%90%B8%E7%81%AF.png" width="65%">
</div><div align=center>
图6-20 PWM驱动呼吸灯-接线图
</div>

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-21%E4%BB%A3%E7%A0%81%E8%B0%83%E7%94%A8-PWM%E5%91%BC%E5%90%B8%E7%81%AF.png" width="25%">
</div><div align=center>
图6-21 PWM驱动呼吸灯-代码调用（非库函数）
</div>

代码展示：OLED和Delay相关代码见前面，本节略。
**- main.c**
```c
#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "PWM.h"
#include "Delay.h"

int main(void){
    uint16_t pwm_duty = 0;//PWM波的占空比
    uint8_t  pwm_flag = 1;//占空比变化控制信号，1升0降
    
    //OLED显示屏初始化
    OLED_Init();
    OLED_ShowString(1,1,"BreathLED:");
    OLED_ShowString(2,1,"Init");
    //PWM初始化
    PWM_Init();
    
    while(1){
        
        Delay_ms(5);//0.5s完成100个占空比变化
        //调整占空比
        if(pwm_flag==1){
            if(pwm_duty<100){
                pwm_duty++;
                OLED_ShowString(2,1,"Inhale");
            }else if(pwm_duty==100){
                pwm_flag = 0;
            }else{
                pwm_duty = 0;
                pwm_flag = 1;
            }
        }else if(pwm_flag == 0){
            if(pwm_duty >0 && pwm_duty<=100){
                pwm_duty--;
                OLED_ShowString(2,1,"Exhale");
            }else if(pwm_duty==0){
                pwm_flag = 1;
            }else{
                pwm_duty = 0;
                pwm_flag = 1;
            }
        }else{
            pwm_duty = 0;
            pwm_flag = 1;
        }
        //改变占空比
        PWM_SetDuty(pwm_duty);
    };
}

```

**- PWM.h**
```c
#ifndef __PWM_H
#define __PWM_H

void PWM_Init(void);
void PWM_SetDuty(uint16_t pwm_duty);

#endif

```

**- PWM.c**
```c
#include "stm32f10x.h"                  // Device header

//TIM输出比较模式-PWM初始化
void PWM_Init(void){
    //1.配置RCC
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
//    //隐：引脚重映射，将PA0映射到PA15
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);//引脚重映射会使用AFIO
//    GPIO_PinRemapConfig(GPIO_PartialRemap1_TIM2, ENABLE);//参考手册“8.3.7定时器复用功能重映射”
//    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//PA15、PB3、PB4变成普通IO口
    //2.选择时基单元时钟
    TIM_InternalClockConfig(TIM2);
    //3.配置时基单元-10kHz
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;//向上计数
    TIM_TimeBaseInitStructure.TIM_Period = 100-1;//自动重装载值
    TIM_TimeBaseInitStructure.TIM_Prescaler = 72-1;//预分频
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0x0000;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);
    //4.配置运行控制
    TIM_Cmd(TIM2, ENABLE);
    //5.配置输出捕获电路
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCStructInit(&TIM_OCInitStructure);//后续即使用到高级定时器初始化，也不会出错
    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;      //PWM模式1
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse       = 0x0000;                 //占空比
    TIM_OC1Init(TIM2, &TIM_OCInitStructure);
    //7.配置GPIO-PA0
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;//复用推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;      //GPIO_Pin_15-引脚重映射
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

//设置PWM波的占空比
//范围是0~100
void PWM_SetDuty(uint16_t pwm_duty){
    TIM_SetCompare1(TIM2, pwm_duty);
}

```


编程感想：
> 1. 在Hardware文件夹创建了```PWM.h```和```PWM.c```。
> 2. 输出比较电路的OC1输出怎么映射到GPIO引脚上呢？其实在引脚定义表上就已经固定好了，OC1输出固定是PA0。但是配置GPIO时注意配置成“复用推挽输出”模式，因为GPIO输出框图中显示，只有复用输出模式才能使信号来自片上外设。
> 3. 引脚重映射。进行引脚重映射时，需要不断对照参考手册及引脚定义表。若对 <u>调试端口</u> 进行引脚重映射，需要三步：开启AFIO时钟，进行引脚重映射，解除调试端口复用。若想对 <u>定时器和其他外设的复用引脚</u> 进行重映射，那就只需要前两步即可。

### 6.4.4 实验：PWM驱动舵机
需求：按下按键开关，舵机角度就变化一次，并在OLED显示屏上显示舵机当前的角度。
> 注1：按键开关PB1，舵机PWM接PA1（输出比较电路通道2）、电源接STLINK上的5V引脚、GND接面包板GND。
> 注2：输入PWM信号要求周期为20ms(50Hz)，高电平宽度为0.5ms\~2.5ms（占空比2.5%\~12.5%），脉冲控制精度为2us(0.18°，占空比精度0.01%)。
> 注3：舵机的三根输入线，棕色是电源负、红色是电源正、橙色是PWM信号线。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-22%E6%8E%A5%E7%BA%BF%E5%9B%BE-PWM%E9%A9%B1%E5%8A%A8%E8%88%B5%E6%9C%BA.png" width="65%">
</div><div align=center>
图6-22 PWM驱动舵机-接线图
</div>

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-23%E4%BB%A3%E7%A0%81%E8%B0%83%E7%94%A8-PWM%E9%A9%B1%E5%8A%A8%E8%88%B5%E6%9C%BA.png" width="25%">
</div><div align=center>
图6-23 PWM驱动舵机-代码调用（非库函数）
</div>

代码展示：首先是在UP主提供的```OLED.c```中添加一个显示单精度浮点数的函数，然后OLED其他代码和Key相关代码略，仅展示新增函数。
**- ```OLED_ShowFloat```函数**：（按照UP主的风格复制编写）
```c
/**
  * @brief  OLED显示单精度浮点数（十进制，带符号数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：-3.4E38~3.4E38
  * @param  Len1 要显示整数长度，范围：1~10
  * @param  Len2 要显示小数长度，范围：1~10
  * @retval 无
  */
void OLED_ShowFloat(uint8_t Line, uint8_t Column, float Number, uint8_t Len1, uint8_t Len2)
{
	uint8_t i;
	uint32_t Number1;
	if (Number >= 0)
	{
		OLED_ShowChar(Line, Column, '+');
		Number1 = Number;
	}
	else
	{
		OLED_ShowChar(Line, Column, '-');
		Number1 = -Number;
        Number = -Number;
	}
	for (i = 0; i < Len1; i++)							
	{
		OLED_ShowChar(Line, Column + i + 1, Number1 / OLED_Pow(10, Len1 - i - 1) % 10 + '0');
	}
    OLED_ShowChar(Line, Column + 1 + Len1, '.');
    Number1 = (uint32_t)((Number - (uint32_t)Number)*OLED_Pow(10,Len2));
    for (i = 0; i < Len2; i++)							
	{
		OLED_ShowChar(Line, Column + i + 2 + Len1, Number1 / OLED_Pow(10, Len2 - i - 1) % 10 + '0');
	}
}
//别忘了在OLED.h文件中声明
```

**- main.c**
```c
#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "Key.h"
#include "SteerEngine.h"

int main(void){
    float sg90_degree = 0; //舵机的角度，范围-90~90，精度为1
    
    //OLED显示屏初始化
    OLED_Init();
    OLED_ShowString(1,1,"SG90-angle:");
    OLED_ShowString(2,1,"-90.00 degree");
    //按键初始化
    Key_Init();
    //舵机初始化
    SteerEngine_Init();
    sg90_degree = -90;
    SteerEngine_SetDegree(sg90_degree);
    
    while(1){
        if(Key_GetNum()==1){
            //改变舵机的角度
            if(sg90_degree<90){
                sg90_degree += 8.8;
            }else{
                sg90_degree = -90;
            }
                SteerEngine_SetDegree(sg90_degree);
            //显示舵机当前角度
            OLED_ShowFloat(2,1,sg90_degree,2,2);
        }
    };
}

```

**- SteerEngine.h**
```c
#ifndef __STEERENGINE_H
#define __STEERENGINE_H

void SteerEngine_Init(void);
void SteerEngine_SetDegree(float SteerEngine_Degree);

#endif

```

**- SteerEngine.c**
```c
#include "stm32f10x.h"                  // Device header

//舵机驱动初始化-TIM2输出比较通道2-PA1
void SteerEngine_Init(void){
    //1.配置RCC
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    //2.选择时基单元时钟
    TIM_InternalClockConfig(TIM2);
    //3.配置时基单元-50Hz-总计分频720000*2
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;//向上计数
    TIM_TimeBaseInitStructure.TIM_Period = 10000-1;//自动重装载值-占空比精度0.01%
    TIM_TimeBaseInitStructure.TIM_Prescaler = 144-1;//预分频
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0x0000;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);
    //4.配置运行控制
    TIM_Cmd(TIM2, ENABLE);
    //5.配置输出捕获电路
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCStructInit(&TIM_OCInitStructure);//后续即使用到高级定时器初始化，也不会出错
    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;      //PWM模式1
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse       = 0x0000;                 //占空比
    TIM_OC2Init(TIM2, &TIM_OCInitStructure);
    //7.配置GPIO-PA1
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;//复用推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

//设置舵机的旋转角度
//-90~90度-->占空比250~1250
void SteerEngine_SetDegree(float SteerEngine_Degree){
    if(SteerEngine_Degree>=-90 && SteerEngine_Degree<=90){
        TIM_SetCompare2(TIM2, (SteerEngine_Degree+90)*1000/180+250);
    }else{
        //期待报错，但不知道怎么写代码
    }
}

```


编程感想：
> 1. 本实验用的舵机旋转范围固定为-90度~90度，无法实现360度旋转。这与舵机的内置电路板有关系，舵机电机本身是有360旋转的潜力的。
> 2. 舵机角度不对。程序下载后，拆下旋转片，按照当前舵机的设定角度重新安装旋转片即可，但是注意这个旋转片带齿，所以角度还是会有轻微的误差。不要旋转片安装好且上电的情况下，强行拨动旋转片，有可能会烧坏舵机的内置电路板！
> 3. 使用多个输出比较通道可以相位同步！由于四个通道共用一个时基单元，所以只能做到相位同步，只是 捕获/比较寄存器 的值不一样而已。


### 6.4.5 实验：PWM驱动直流电机
需求：按下按键开关，直流电机依次改变转速：+20、+40、+60、+80、+100、-100、-80、-60、-40、-20、0。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-24%E6%8E%A5%E7%BA%BF%E5%9B%BE-PWM%E9%A9%B1%E5%8A%A8%E7%9B%B4%E6%B5%81%E7%94%B5%E6%9C%BA.png" width="75%">
</div><div align=center>
图6-24 PWM驱动直流电机-接线图
</div>

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-25%E4%BB%A3%E7%A0%81%E8%B0%83%E7%94%A8-PWM%E9%A9%B1%E5%8A%A8%E7%9B%B4%E6%B5%81%E7%94%B5%E6%9C%BA.png" width="25%">
</div><div align=center>
图6-25 PWM驱动直流电机-代码调用（非库函数）
</div>

代码展示：OLED和Key的相关代码和上一小节“PWM驱动舵机”一样。下面仅展示新增代码。
**- main.c**
```c
#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "Key.h"
#include "DC_Motor.h"

int main(void){
    //直流电机的参数，正负代表转向，数值代表占空比(范围-100~100)
    int16_t DCmotor_para = 0;
    //OLED显示屏初始化
    OLED_Init();
    OLED_ShowString(1,1,"DC_Motor:");
    OLED_ShowString(2,1,"+000%");
    //按键初始化
    Key_Init();
    //直流电机初始化
    DC_Motor_Init();
    
    while(1){
        if(Key_GetNum()==1){
            if(DCmotor_para<100){
                DCmotor_para += 20;
            }else if(DCmotor_para>=100){
                DCmotor_para = -100;
            }
            DC_Motor_SetRotateSpeed(DCmotor_para);
        }
        OLED_ShowSignedNum(2,1,DCmotor_para,3);
    };
}

```

**- DC_Motor.h**
```c
#ifndef __DC_MOTOR_H
#define __DC_MOTOR_H

void DC_Motor_Init(void);
void DC_Motor_SetRotateSpeed(int16_t DC_Motor_para);

#endif

```

**- DC_Motor.c**
```c
#include "stm32f10x.h"                  // Device header

//直流电机初始化-PWM-TIM2输出比较3(PA2)；AIN1-A4；AIN2-A5
void DC_Motor_Init(void){
    //1.配置RCC
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    //2.选择时基单元时钟
    TIM_InternalClockConfig(TIM2);
    //3.配置时基单元-20kHz-总计分频3600
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;//向上计数
    TIM_TimeBaseInitStructure.TIM_Period = 100-1;//自动重装载值-占空比精度1%
    TIM_TimeBaseInitStructure.TIM_Prescaler = 36-1;//预分频
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0x0000;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);
    //4.配置运行控制
    TIM_Cmd(TIM2, ENABLE);
    //5.配置输出捕获电路
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCStructInit(&TIM_OCInitStructure);//后续即使用到高级定时器初始化，也不会出错
    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;      //PWM模式1
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse       = 0x0000;                 //占空比
    TIM_OC3Init(TIM2, &TIM_OCInitStructure);
    //7.配置GPIO-PA2
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;//复用推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    //8.配置AIN1-A4、AIN2-A5
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(GPIOA, GPIO_Pin_4 | GPIO_Pin_5);//直流电机初始化为停止状态
}

//调制直流电机转向及转速
//占空比范围-100~100，精度1
void DC_Motor_SetRotateSpeed(int16_t DC_Motor_para){    
    //参数正-顺时针；参数负-逆时针
    if(DC_Motor_para>=0){
        GPIO_SetBits(GPIOA, GPIO_Pin_4);
        GPIO_ResetBits(GPIOA, GPIO_Pin_5);
    }else{
        GPIO_ResetBits(GPIOA, GPIO_Pin_4);
        GPIO_SetBits(GPIOA, GPIO_Pin_5);
        DC_Motor_para = -DC_Motor_para;
    }
    //调节输出PWM波的占空比
    TIM_SetCompare3(TIM2, DC_Motor_para);
}

```


编程感想：
> 1. 关于直流电机的转向。安装好风扇叶后上电进行测试，发现黑色线接AO1红色线接AO2，可以保证直流电机正转时向前吹风，反转向后吸风。
> 2. 关于GPIO口的模式。查阅参考手册“8.1.11外设的GPIO配置”，发现TIM2的输出通道要配置成 **复用推挽输出** 模式，而AIN1、AIN2所对应的配置成普通的 **推挽输出** 即可。
> 3. 关于直流电机的声音。使用1kHz的PWM波驱动直流电机时，旋转速度较低时，用手捏住电机轴不让它转，直流电机可能会发出类似蜂鸣器的声音，这是因为1kHz在人耳能听到的频率范围（20Hz~20kHz）内。将频率改成20kHz以上就不会听到了。

### 6.4.6 扩展实验：旋转编码器控制舵机
需求：将旋转编码器顺时针转则舵机正转，逆时针转则舵机反转，分辨率为10°。
> 1. 接线图参考“5-2旋转编码器计次”、“6-4PWM驱动舵机”两个实验的接线。
> > - 旋转编码器：A口接PB0、B口接PB1。VCC和GND直连面包板。
> > - 舵机：舵机的三根输入线，棕色是电源负、红色是电源正、橙色是PWM信号线接TIM2输出比较通道2(PA1)。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-26%E4%BB%A3%E7%A0%81%E8%B0%83%E7%94%A8-%E6%97%8B%E8%BD%AC%E7%BC%96%E7%A0%81%E5%99%A8%E6%8E%A7%E5%88%B6%E8%88%B5%E6%9C%BA.png" width="25%">
</div><div align=center>
图6-26 旋转编码器控制舵机-代码调用（非库函数）
</div>

代码展示：SteerEngine模块和RotaryEncode模块与之前相同，不予赘述。
**- main.c**
```c
#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "RotaryEncoder.h"
#include "SteerEngine.h"

int main(void){
    int16_t RE_count=0, RE_change=0;//旋转编码器参数
    float sg90_degree = 0;//舵机角度
    //OLED显示屏初始化
    OLED_Init();
    OLED_ShowString(1,1,"SG90_degree:");
    OLED_ShowString(2,1,"+00 degree");
    //旋转编码器初始化
    RotaryEncoder_Init();
    //舵机初始化
    SteerEngine_Init();
    SteerEngine_SetDegree(0);
    
    while(1){
        //旋转编码器顺时针转-->舵机顺时针转（角度减）
        //旋转编码器逆时针转-->舵机逆时针转（角度加）
        if(RE_count != RotaryEncoder_GetCount()){
            RE_count = RotaryEncoder_GetCount();
            if(RotaryEncoder_GetChange() == 1){
                if(sg90_degree>-90 && sg90_degree<=90){
                    sg90_degree -= 18;
                }else if(sg90_degree != -90){
                    sg90_degree = 0;
                }
            }else if(RotaryEncoder_GetChange() == -1){
                if(sg90_degree>=-90 && sg90_degree<90){
                    sg90_degree += 18;
                }else if(sg90_degree != 90){
                    sg90_degree = 0;
                }
            }
            SteerEngine_SetDegree(sg90_degree);
            OLED_ShowSignedNum(2,1,sg90_degree,2);
        }
        
    };
}

```

### 6.4.7 扩展实验：旋转编码器控制直流电机
需求：将旋转编码器顺时针转则加速，逆时针转则减速，分辨率为20%，边界为 [-100, +100]。
> 1. 接线图参考“5-2旋转编码器计次”、“6-5PWM驱动直流电机”两个实验的接线。
> > - 旋转编码器：A口接PB0、B口接PB1。VCC和GND直连面包板。
> > - 直流电机：PWMA接PA2、AIN1接PA4、PA5。VM连STLINK的5V。AO1、AO2接直流电机。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-27%E4%BB%A3%E7%A0%81%E8%B0%83%E7%94%A8-%E6%97%8B%E8%BD%AC%E7%BC%96%E7%A0%81%E5%99%A8%E6%8E%A7%E5%88%B6%E7%9B%B4%E6%B5%81%E7%94%B5%E6%9C%BA.png" width="25%">
</div><div align=center>
图6-27 旋转编码器控制直流电机-代码调用（非库函数）
</div>


代码展示：DC_Motor模块和RotaryEncode模块与之前相同，不予赘述。
**- main.c**
```c
#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "RotaryEncoder.h"
#include "DC_Motor.h"

int main(void){
    //直流电机的参数，正负代表转向，数值代表占空比(范围-100~100)
    int16_t DCmotor_para = 0;
    //旋转编码器数值及转向
    int16_t RE_rotate;
    int16_t RE_num = 0;
    
    //OLED显示屏初始化
    OLED_Init();
    OLED_ShowString(1,1,"DC_Motor:");
    OLED_ShowString(2,1,"+000%");
    //旋转编码器初始化
    RotaryEncoder_Init();
    //直流电机初始化
    DC_Motor_Init();
    
    while(1){
        if(RE_num != RotaryEncoder_GetCount()){
            RE_num = RotaryEncoder_GetCount();
            RE_rotate = RotaryEncoder_GetChange();
//            OLED_ShowSignedNum(3,1,RE_rotate,1);
//            OLED_ShowSignedNum(4,1,RotaryEncoder_GetCount(),5);
            //改变转速
            if(RE_rotate == 1){//旋转编码器顺时针转，增大转速
                if(DCmotor_para>=-100 && DCmotor_para<100){
                    DCmotor_para += 20;
                }else if(DCmotor_para!=100){
                    DCmotor_para = 0;
                }
                //更新显示及转速
                OLED_ShowSignedNum(2,1,DCmotor_para,3);
                DC_Motor_SetRotateSpeed(DCmotor_para);
            }else if(RE_rotate == -1){//旋转编码器逆时针转，减小转速
                if(DCmotor_para>-100 && DCmotor_para<=100){
                    DCmotor_para -= 20;
                }else if(DCmotor_para!=-100){
                    DCmotor_para = 0;
                }
                //更新显示及转速
                OLED_ShowSignedNum(2,1,DCmotor_para,3);
                DC_Motor_SetRotateSpeed(DCmotor_para);
            }
            
        }
    };
}

```

编程感想：
> 1. 关于外设的调用。旋转编码器的原理是外部中断EXTI，随便指定两个GPIO端口即可。直流电机需要用到定时器的输出比较电路，需要占用一个固定的引脚输出PWM，另外再随便占用两个GPIO端口控制正反转。
> 2. 接上直流电机后，旋转编码器控制的不精准。这个问题以目前的知识水平，没有解决。故障现象具体大致可以参考B站视频“[旋转编码器控制直流电机正反转及转速](https://www.bilibili.com/video/BV1P44y137Rq/)”。













## 6.5 TIM输入捕获原理
本节的功能描述对应参考手册内容为“14.3.5 输入捕获模式”、“14.3.6 PWM输入模式”、“14.3.15 定时器同步”。

**IC（Input Capture）输入捕获**模式 下，当通道 **输入引脚** 出现指定电平跳变时，当前CNT的值将被锁存到CCR中，读取CCR的值就可以测量PWM波形的脉冲间隔（频率）、电平持续时间（占空比）等参数。
> - 每个高级定时器和通用定时器都拥有4个输入捕获通道，基本定时器没有输入捕获/输出比较的功能。
> - 可配置为 **PWMI模式**，同时测量频率和占空比。
> - 可配合 **主从触发模式**（UP主自己取的名字），实现硬件全自动测量。
>
> 注：输入捕获和输出比较共用CCR寄存器和引脚，所以对于同一个定时器不能同时使用。
> 注：PWMI模式和主从触发模式的硬件设计巧妙，可以极大的减轻软件的压力。

**6.5.1 频率测量原理**
stm32只能测量数字信号（高电平3.3V，低电平0V）的频率，对于正弦波则需要使用运放比较、电压隔离等模块进行预处理，转换成数字信号之后才能进行测量。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-28%E9%A2%91%E7%8E%87%E6%B5%8B%E9%87%8F%E5%8E%9F%E7%90%86.png" width="70%">
</div><div align=center>
图6-28 频率测量原理
</div>

> 1. 测频法：在闸门时间T内，对上升沿计次，得到N，则频率为 $f_x = N/T$。如测量1s内有多少个上升沿就是多少Hz。适合测量高频信号，数据更新慢。
> 2. 测周法：两个上升沿内，以标准频率 $f_c$ 计次，得到N ，则频率 $f_x = f_c / N$。本质上就是直接测量一个周期的时间，适合测量低频信号，数据更新速度取决于待测信号频率，更新速度相对更快。
> 3. 中界频率：测频法与测周法误差相等的频率点 $f_m = \sqrt{f_c / T}$。待测信号频率大于中介频率，更适合用测频法；反之则适合用测周法。
>
> 注：**测频法**可以参考之前的 **外部中断计次** 的代码；而**测周法**则需要用到TIM的 **输入捕获模式**。

**6.5.2 TIM的输入捕获电路**
<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-29%E9%80%9A%E7%94%A8%E5%AE%9A%E6%97%B6%E5%99%A8%E6%94%BE%E5%A4%A7%E5%9B%BE.png" width="80%">
</div><div align=center>
图6-29 输入捕获电路（通用定时器局部放大图）
</div>

> - TIMx_CH1 ~ TIMx_CH4：4个输入引脚，参考引脚定义表可以知道引脚复用在哪个位置。
> - 三输入异或门：数据选择器可以使其不起作用。起作用时主要是为三相无刷电机服务，此时三个输入接三相无刷电机的霍尔传感器（检测转子位置），然后这个定时器就可以作为换相电路的接口定时器，驱动换相电路工作。本节不涉及。
> - 输入滤波器：对输入信号进行滤波，避免一些高频的毛刺信号误触发。
> - 边沿检测器：与外部中断类似，可以选择高电平触发、低电平触发等。
> - TI1FP1、TI1FP2：是两套独立的信号，都经过各自的滤波器、极性选择，进而输出到后续电路。之所以设计成交叉电路，主要有两个好处：可以灵活切换后续捕获电路的输入，而不需要重新初始化；将一个引脚输入同时映射到两个捕获单元，是PWMI模式的经典结构。下面三路的信号同理。
> - TRC1：来源于时钟源的选择，也可以作为输入捕获信号的输入。
> - 预分频器：对信号进行分频，其输出的信号触发捕获电路进行工作：每来一个触发信号，CCR就会读取一次CNT的值，同时发生一个捕获事件ICxPS，这个事件会在状态寄存器置标志位，同时也可以产生中断（捕获中断）。
> > 例（测周法）：比如可以配置上升沿触发捕获，没来一次上升沿，CNT就将值转运到CCR，由于CNT由内部时钟源驱动，此时CNT的值实际上就是两次上升沿之间的时间间隔，于是就实现了测周法。
> > 注：要想在一次捕获后将CNT清零，可以使用 主从触发模式 配置硬件自动完成。

上面对于输入捕获电路的基本结构做了相应的介绍，下面来介绍更加细节的电路结构：
<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-30%E8%BE%93%E5%85%A5%E6%8D%95%E8%8E%B7%E9%80%9A%E9%81%93-%E7%94%B5%E8%B7%AF%E5%8D%95%E8%B7%AF.png" width="70%">
</div><div align=center>
图6-30 输入捕获通道电路（部分示意图）
</div>

> 上图给出了输入捕获通道的实际电路，但其实TI1FP2也有一路单独的滤波器和边沿检测器，这部分电路并没有显示在图中。
> - TI1：实际上就是CH1引脚。
> - F~DTS~：滤波器的采样时钟来源。
> - ICF[3:0]：控制滤波器参数，可以对消除高频毛刺——参考手册“14.4.7 捕获/比较模式寄存器(TIMx_CCMR1)”。这个滤波器的基本原理就是以采样频率对输入信号进行采样，当连续N个值都为高电平才输出高电平、连续N个值都为低电平才输出低电平，若N个值产生了高频抖动，那输出就不变化。采样频率越低、采样个数N越大，采样效果越好。
> <img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-30%E8%BE%93%E5%85%A5%E6%8D%95%E8%8E%B7%E7%94%B5%E8%B7%AF-%E6%BB%A4%E6%B3%A2%E5%99%A8%E5%8F%82%E6%95%B0.png" width="70%">
> - 边沿检测器：捕获TI1F的上升沿/下降沿。
> - CC1P位：属于TIMx_CCER寄存器，控制极性选择。
> - CC1S[1:0]：对输入的TI1FP1、TI2FP1、TRC进行选择。
> - ICPS[1:0]：控制分频器，可以选择不分频、2分频、4分频、8分频。
> - CC1E：控制输出使能或失能。进而就可以在输入TI1的边沿，将CNT转运到CCR。
> - 从模式控制器：实现硬件自动化操作的利器，可以来自于TI1F_ED、TI1FP1。后续可以配置相应的硬件，在TI1FP1的上升沿对CNT自动清零。下面介绍主从触发模式。

**6.5.3 主从触发模式**
<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-31%E4%B8%BB%E4%BB%8E%E8%A7%A6%E5%8F%91%E6%A8%A1%E5%BC%8F%E8%AF%B4%E6%98%8E.png" width="70%">
</div><div align=center>
图6-31 主从触发模式-配置概述
</div>

> 主从触发模式实际上是UP主自己取的名字，参考手册中只有主模式、从模式的介绍。
> 主从触发模式就是 **主模式、从模式、触发源选择** 三个功能的简称，因为对于单个定时器来说，既可以配置成主模式，输出触发源；也可以配置成从模式，受其他触发源的控制。
> - 主模式：将定时器内部的信号映射到TRGO引脚，用于触发别的外设。见“14.4.2 控制寄存器2(TIMx_CR2)”：
> <img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-31%E4%B8%BB%E6%A8%A1%E5%BC%8F%E8%A7%A6%E5%8F%91%E6%BA%90%E9%80%89%E6%8B%A9.png" width="70%">
> - 从模式：接收其他外设/自身外设的信号（TRGI），来控制自身定时器的运行。可以执行的操作见上图。参考手册见“14.4.3 从模式控制寄存器(TIMx_SMCR)”：
> <img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-31%E4%BB%8E%E6%A8%A1%E5%BC%8F%E6%93%8D%E4%BD%9C%E9%80%89%E6%8B%A9.png" width="70%">
> - 触发源选择：选择从模式的触发源，可以认为是从模式的一部分。可以选择的触发源见上图，注意触发源不包含TI3、TI4信号，所以进行 **从模式自动清零CNT，只能使用通道1和通道2**。参考手册见“14.4.3 从模式控制寄存器(TIMx_SMCR)”：
> <img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-31%E4%BB%8E%E6%A8%A1%E5%BC%8F%E8%A7%A6%E5%8F%91%E6%BA%90%E9%80%89%E6%8B%A9.png" width="70%">
> 
> 例：若想让TI1FP1信号自动触发CNT清零。那么触发源选择TIFP1，从模式选择```Reset```，即可实现硬件自动化。

不用担心，看似复杂，但实际操作过程中，也是在库函数中调用相应的函数即可快速的完成配置。

**6.5.4 输入捕获基本结构和PWMI基本结构**
<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-32%E8%BE%93%E5%85%A5%E6%8D%95%E8%8E%B7%E5%9F%BA%E6%9C%AC%E7%BB%93%E6%9E%84.png" width="70%">
</div><div align=center>
图6-32 输入捕获基本结构
</div>

> 本结构只使用了一个通道，所以只能测量频率。
> - 时基单元：与之前的相同，CNT就是就是测周法用于计时的东西。注意**测频率的标准频率是预分频之后的频率**。
> - TI1FP1：兵分两路，一路用于触发转运CNT到CCR，一路用于触发清零CNT。硬件执行时肯定是先转运再清零，或者是两者同时非阻塞进行。
> - 读取CCR值：需要测频率时就读取CCR值，不需要测频率时整个电路自动运行，也**不占用软件资源**。
> - 关于CNT：计数最大值是65535，所以待测频率不能太低。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-33PWMI%E5%9F%BA%E6%9C%AC%E7%BB%93%E6%9E%84.png" width="70%">
</div><div align=center>
图6-33 PWMI基本结构
</div>

> 使用两个通道同时捕获一个引脚，可以同时测量周期和占空比。
> - TI1FP1：上升沿触发，和之前相同。通道1的捕获寄存器CCR1表示整个周期的时间。
> - TI1FP2：配置为下降沿触发，通过交叉通道触发通道2的捕获单元。于是通道2的捕获寄存器CCR2就表示高电平周期。










## 6.6 TIM输入捕获相关实验
### 6.6.1 实验：输入捕获模式测频率
需求：PA0产生频率可调的PWM波，然后在PA6端口进行测量，并在OLED上显示相应的频率。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-34%E6%8E%A5%E7%BA%BF%E5%9B%BE-%E8%BE%93%E5%85%A5%E6%8D%95%E8%8E%B7%E6%B5%8B%E9%A2%91%E7%8E%87.png" width="70%">
</div><div align=center>
图6-34 输入捕获模式测频率-接线图
</div>

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-35%E4%BB%A3%E7%A0%81%E8%B0%83%E7%94%A8-%E8%BE%93%E5%85%A5%E6%8D%95%E8%8E%B7%E6%B5%8B%E9%A2%91%E7%8E%87.png" width="25%">
</div><div align=center>
图6-35 输入捕获模式测频率-代码调用（非库函数）
</div>

代码展示：OLED代码见第四章“4OLED调试工具”、PWM相关代码见“PWM驱动呼吸灯实验”。
**- main.c**
```c
#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "PWM.h"
#include "InputCapture.h"

int main(void){
    //OLED显示屏初始化
    OLED_Init();
    OLED_ShowString(1,1,"Frequency:");
    OLED_ShowString(2,1,"00000 Hz");
    
    //初始化PWM波-PA0输出
    PWM_Init();
    PWM_SetDuty(50);//占空比（单位：%）
    PWM_SetFreq(10000); //PWM频率（单位：Hz）
    
    //初始化输入捕获
    InputCapture_Init();
    
    while(1){
        OLED_ShowNum(2,1, InputCapture_GetFreq(),5);
    };
}

```

**- InputCapture.h**
```c
#ifndef __INPUTCAPTURE_H
#define __INPUTCAPTURE_H

void InputCapture_Init(void);
uint32_t InputCapture_GetFreq(void);

#endif

```

**- InputCapture.c**
```c
#include "stm32f10x.h"                  // Device header

//输入捕获初始化-TIM3输入捕获通道1-PA6
void InputCapture_Init(void){
    //1.开启外设时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    //2.GPIO配置
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    //3.时基单元配置
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = 0xffff;
    TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0x00;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);
    TIM_Cmd(TIM3, ENABLE);
    //4.输入捕获单元配置
    TIM_ICInitTypeDef TIM_ICInitStructure;
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;//输入捕获通道1
    TIM_ICInitStructure.TIM_ICFilter = 0x3;
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;//上升沿计数
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;//不选择交叉通道
    TIM_ICInit(TIM3, &TIM_ICInitStructure);
    //4.从模式配置
    TIM_SelectInputTrigger(TIM3, TIM_TS_TI1FP1);
    TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_Reset);
}

//根据输入捕获的值计算频率
//计算范围：16HZ~500kHz，较为精准的范围：16Hz~10kHz，高频分辨率约为15Hz
uint32_t InputCapture_GetFreq(void){
    return (uint32_t)1000000/(uint32_t)(TIM_GetCapture1(TIM3)+1);
}

```

编程感想：
> 1. 调节PWM波的频率。影响PWM频率的两个参数是自动重装载值ARR、预分频系数PSC，但是注意如果改变ARR会同步影响占空比，所以为了简单，就调节预分频系数来改变PSC即可。
> 2. 运行控制。注意配置时基单元的时候，一定不要完了时基单元的运行控制函数```TIM_Cmd```。
> 3. 计算频率。最后获取输入捕获值计算频率时，由于CNT会在上升沿清零计数器，所以单周期的最后一个小周期可能会由于上升沿的触发而少记一次，所以记得将输入捕获值+1后，再计算频率。这个是测周法固有的误差。


### 6.6.2 实验：PWMI模式测频率占空比
需求：PA0产生频率和占空比可调的PWM波，然后在PA6端口采用PWMI模式进行测量，并在OLED上显示相应的频率和占空比。

**接线图**、**代码调用**与上一小节完全相同，所不同的是在原来的```InputCapture.c```基础上，增加了两个函数。下面是**代码展示**：
**- main.c**
```c
#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "PWM.h"
#include "InputCapture.h"

int main(void){
    //OLED显示屏初始化
    OLED_Init();
    OLED_ShowString(1,1,"Freq:00000 Hz");
    OLED_ShowString(2,1,"Duty:000%");
    
    //初始化PWM波-PA0输出
    PWM_Init();
    PWM_SetDuty(79);//占空比（单位：%）
    PWM_SetFreq(10000); //PWM频率（单位：Hz）
    
    //初始化输入捕获
    InputCapture_PWMIInit();
    
    while(1){
        OLED_ShowNum(1,6, InputCapture_GetFreq(),5);
        OLED_ShowNum(2,6, InputCapture_GetDuty(),3);
//        OLED_ShowNum(3,1,TIM_GetCapture2(TIM3),5);
//        OLED_ShowNum(4,1,TIM_GetCapture1(TIM3),5);
    };
}

```

**- InputCapture.c**
```c
#include "stm32f10x.h"                  // Device header

//PWMI模式初始化-TIM3输入捕获通道1-PA6
void InputCapture_PWMIInit(void){
    //1.开启外设时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    //2.GPIO配置
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    //3.时基单元配置
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = 0xffff;
    TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0x00;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);
    TIM_Cmd(TIM3, ENABLE);
    //4.输入捕获单元配置
    //输入捕获通道1
    TIM_ICInitTypeDef TIM_ICInitStructure;
    TIM_ICInitStructure.TIM_Channel     = TIM_Channel_1;//输入捕获通道1
    TIM_ICInitStructure.TIM_ICFilter    = 0x3;
    TIM_ICInitStructure.TIM_ICPolarity  = TIM_ICPolarity_Rising;//上升沿计数
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;//不选择交叉通道
    TIM_PWMIConfig(TIM3, &TIM_ICInitStructure);//注意只是这里换了！！！使用该函数不需要再初始化通道2
    //4.从模式配置
    TIM_SelectInputTrigger(TIM3, TIM_TS_TI1FP1);
    TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_Reset);
}

//根据输入捕获的值计算占空比-PWMI模式初始化才能调用
//计算范围：1%~100%，占空比分辨率1%
uint16_t InputCapture_GetDuty(void){
//    float duty;
//    duty = ((float)(TIM_GetCapture2(TIM3)+1))/((float)(TIM_GetCapture1(TIM3)+1));
//    return (uint16_t)(duty*100);
    return (TIM_GetCapture2(TIM3)+1)*100/(TIM_GetCapture1(TIM3)+1);
}
//别忘了将这两个函数在InputCapture.h头文件中声明
```












## 6.7 TIM编码器接口原理
**Encoder Interface 编码器接口** 是定时器的时钟源之一，可接收增量（正交）编码器的信号，<u>根据编码器旋转产生的正交信号脉冲，自动控制CNT自增或自减</u>（不消耗软件资源），从而指示编码器的位置、旋转方向和旋转速度。当然上面这个功能也可以使用外部中断+软件代码来手动实现，但是stm32开辟出专用的硬件资源，可以减轻软件的消耗。
> - 每个高级定时器和通用定时器都拥有1个编码器接口。注意定时器配置成编码器接口模式后，就干不了别的活了。
> - 两个输入引脚借用了**输入捕获的通道1和通道2**。
> - 典型应用场景：一般应用在电机控制的项目上。使用PWM驱动电机，再使用定时器的编码器接口测量电机的速度，然后再用PID算法进行闭环控制。一般电机的转速比较高，会使用无接触式的霍尔传感器或光栅来输出包含转速信息的信号（如正交编码器信号）。

在要求不严格的场景下，正交信号的某一路信号便可以传递转速信息，但不能说明旋转方向；要指明旋转方向，可以再添加一路专门用于指示旋转方向的信号，但是这样就不是正交信号了。正交信号使用两路相位相差90°的信号，可以同时说明转速、转向，并且由于其特殊的正交性质，还额外增加了抗干扰的能力。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-36%E6%AD%A3%E4%BA%A4%E7%BC%96%E7%A0%81%E5%99%A8%E4%BF%A1%E5%8F%B7%E7%A4%BA%E6%84%8F%E5%9B%BE.png" width="60%">
</div><div align=center>
图6-36 正交编码器信号示意图
</div>

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-38%E7%BC%96%E7%A0%81%E5%99%A8%E6%8E%A5%E5%8F%A3%E5%9F%BA%E6%9C%AC%E6%8E%A5%E5%8F%A3.png" width="70%">
</div><div align=center>
图6-37 计数方向与编码器信号的关系
</div>

为了消除某些毛刺噪声，stm32处理正交信号的基本逻辑：
> 在两路的**所有的边沿**部分都进行判断，按照上表指示的逻辑对CNT进行增/减。下面是两个例子，演示了正交信号的抗噪声能力：
> - 两路均不反相：
> <img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-37%E5%9D%87%E4%B8%8D%E5%8F%8D%E7%9B%B8.png" width="70%">
> - TI1反相，TI2不反相：
> <img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-37TI1%E5%8F%8D%E7%9B%B8.png" width="70%">
>
> 上面这两种模式的应用：如果发现计次方向和预期方向相反，那么就调整一下极性即可。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-37%E8%AE%A1%E6%95%B0%E6%96%B9%E5%90%91%E4%B8%8E%E7%BC%96%E7%A0%81%E5%99%A8%E4%BF%A1%E5%8F%B7%E7%9A%84%E5%85%B3%E7%B3%BB.png" width="70%">
</div><div align=center>
图6-38 定时器“编码器接口”基本结构
</div>

> - 两个输入:编码器接口的输入固定为输入捕获通道1的TI1FP1（CH1引脚）和输入捕获通道2的TI2FP2（CH2引脚）。CH3和CH4与编码器接口无关。
> - 极性选择：极性选择实际上就是选择是否对输入信号进行反相。在输入捕获模式的作用效果是选择上升沿有效还是下降沿有效；在编码器接口模式的作用效果是影响判断的高低电平。
>
> 注：进一步详细介绍参考“14.3.12 编码器接口模式”。




## 6.8 实验：编码器接口测速
需求：使用定时器的编码器接口，对旋转编码器进行计次、测速。
> - 旋转编码器一圈共有20个分割点，单个分割点为18°。
> - 注：用编码器接口对旋转编码器进行测速，显然太奢侈了。但是目前手头上的器件，只有旋转编码器能输出两路正交的编码器信号，所以就对旋转编码器进行测速了。
> - 基本思路：两个定时器，一个定时中断，一个编码器接口。每次定时中断读取一次计数器CNT的值并清零，就可以不断地得到相应的速度。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-39%E6%8E%A5%E7%BA%BF%E5%9B%BE-%E7%BC%96%E7%A0%81%E5%99%A8%E6%8E%A5%E5%8F%A3.png" width="70%">
</div><div align=center>
图6-39 编码器接口测速-接线图
</div>

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/6-40%E4%BB%A3%E7%A0%81%E8%B0%83%E7%94%A8-%E7%BC%96%E7%A0%81%E5%99%A8%E6%8E%A5%E5%8F%A3%E6%B5%8B%E9%80%9F.png" width="25%">
</div><div align=center>
图6-40 编码器接口测速-代码调用（非库函数）
</div>

代码展示：定时器Timer相关代码参考“6.2.1 定时器定时中断-内部时钟”，只不过设定定时器的中短间隔为1s。
**- main.c**
```c
#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "EncoderInterface.h"
#include "Timer.h"

int16_t timer_cnt = 0;//定时器的计数器

int main(void){
    //定义旋转编码器的转速（单位：转/分）
    float RE_RPM = 0;
    
    //OLED显示屏初始化
    OLED_Init();
    OLED_ShowString(1,1,"RE_Speed:");
    OLED_ShowString(2,1,"+000.00 RPM");
    OLED_ShowString(3,1,"CNT:+00000");    
    
    //编码器接口初始化
    EncoderInterface_Init();
    
    //定时器初始化
    Timer_Init();
    
    while(1){
        RE_RPM = (float)timer_cnt/4/20*60;//定时器的闸门时间是1s
        OLED_ShowFloat(2,1,RE_RPM,3,2);
        OLED_ShowSignedNum(3,5,timer_cnt,5);
    };
}

//TIM2定时中断后的中断函数-获取计数器的值并清零
void TIM2_IRQHandler(void){
    if(TIM_GetITStatus(TIM2,TIM_IT_Update)==SET){
        timer_cnt = EncoderInterface_Get();
        TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
    }
}

```

**- EncoderInterface.h**
```c
#ifndef __ENCODERINTERFACE_H
#define __ENCODERINTERFACE_H

void EncoderInterface_Init(void);
uint16_t EncoderInterface_Get(void);

#endif

```

**- EncoderInterface.c**
```c
#include "stm32f10x.h"                  // Device header

//定时器的编码器接口初始化-TIM3-PA6、PA7
void EncoderInterface_Init(void){
    //1.开启外设时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    //2.配置GPIO
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    //3.配置时基单元-编码器接口托管时钟
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = 0xffff;
    TIM_TimeBaseInitStructure.TIM_Prescaler = 1 - 1;//默认不分频
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0x00;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);
    //4.配置输入捕获通道
    TIM_ICInitTypeDef TIM_ICInitStructure;
    TIM_ICStructInit(&TIM_ICInitStructure);//防止没定义的参数影响程序正常运行
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;//输入捕获通道1
    TIM_ICInitStructure.TIM_ICFilter = 0x3;
    TIM_ICInit(TIM3, &TIM_ICInitStructure);
    TIM_ICStructInit(&TIM_ICInitStructure);
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;//输入捕获通道2
    TIM_ICInitStructure.TIM_ICFilter = 0x3;
    TIM_ICInit(TIM3, &TIM_ICInitStructure);
    //5.选择编码器接口
    TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    //6.启动定时器
    TIM_Cmd(TIM3, ENABLE);
}

//读取计数器的值并清零
uint16_t EncoderInterface_Get(void){
    uint16_t temp = TIM_GetCounter(TIM3);
    TIM_SetCounter(TIM3, 0x0000);
    return temp;
}

```

编程感想：
> 1. GPIO输入模式。在配置成上拉输入、下拉输入的时候，记得要与外部引脚的默认电平保持一致，防止默认电平打架。不过默认高电平是一种行业习惯。若实在不确定外部引脚默认电平，就选择浮空输入，但缺点就是容易受到噪声的干扰。
> 2. 极性的配置。```TIM_EncoderInterfaceConfig```编码器配置中对于极性的配置和输入捕获通道对于极性的配置重复，所以若将编码器配置放在后面，那么在输入捕获通道配置时就无需特别指明极性选择的配置了。
> 3. 旋转编码器的段落感。转动一下，计数器会变动4次。这是因为在双边沿都会进行判断计数。
> 4. 初始化！！！把定时器的代码加进来后，库库一顿操作在中断函数里取CNT值、主函数计算转速，一运行啥都没有，反复看代码也没看出啥逻辑上的毛病，一筹莫展。忽然发现，原来是忘了加定时器的初始化函数:sweat_smile:。
