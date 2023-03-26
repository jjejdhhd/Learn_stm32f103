# 7 ADC模数转换器
 [toc]

注：笔记主要参考B站 [江科大自化协](https://space.bilibili.com/383400717) 教学视频“[STM32入门教程-2023持续更新中](https://www.bilibili.com/video/BV1th411z7sn/)”。
注：工程及代码文件放在了本人的[Github仓库](https://github.com/jjejdhhd/Learn_stm32f103/tree/main)。
***

## 7.1 模数转换器原理
**ADC（Analog-Digital Converter）模拟-数字转换器** 可以将引脚上连续变化的模拟电压转换为内存中存储的数字变量，建立<u>模拟电路到数字电路的桥梁</u>。当然，也存在<u>数字到模拟的桥梁</u>，如DAC、PWM波等，并且由于PWM只有完全导通和完全断开两种状态，**PWM电路简单且没有额外的功率损耗**，所以相比DAC，使用PWM来等效模拟量更适合直流电机调速等大功率应用场景（惯性系统）。而DAC主要应用于波形生成领域，如信号发生器、音频解码芯片等。c8t6的stm32只有ADC外设，没有DAC外设，下面是ADC参数的介绍：
> - 12位逐次逼近型ADC，1us转换时间（信号频率较高时需要注意）。
> - 输入电压范围：0\~3.3V，转换结果范围：0\~4095。
> - 18个输入通道，可测量16个外部（GPIO口）和2个内部信号源（内部温度传感器和内部参考电压）。
> > - 内部温度传感器：可以测量芯片温度，比如电脑CPU的温度显示。
> > - 内部参考电压：是一个1.2V左右的基准电压，不随外部供电电压变化。如果外部供电电压不是3.3V，那读取GPIO口的电压就不对，此时就可以通过读取这个基准电压来校准。
> - 规则组和注入组两个转换单元，是stm32的ADC增强功能。
> - 模拟看门狗自动监测输入电压范围，当AD值高于上阈值或低于下阈值时，就会申请中断，可减轻软件负担。
> - **STM32F103C8T6 ADC资源**：ADC1、ADC2，10个外部输入通道。

ADC的知识点比较琐碎，下面将依次介绍：
- 1. ADC电路结构：包括逐次逼近型ADC结构和stm32中的ADC结构。
- 2. 引脚复用关系：ADC输入17种输入通道的引脚定义。
- 3. 规则组的转换模式：ADC的4种配置方式。
- 4. 触发转换信号：启动ADC转换一次的信号。
- 5. 数据对齐：ADC转换之后的结果，采用左对齐/右对齐。
- 6. 转换时间：如何计算ADC的转换时间。
- 7. 校准：ADC的校准电路。
- 8. 外围电路设计：如何设计一个稳定的外部模拟输入源。
- 9. 总结：直接参考“ADC基本结构图”就可以照着写代码。


### 7.1.1 ADC电路结构
stm32中ADC采用了逐次逼近型ADC结构，下面以ADC0809芯片为例介绍这种结构：

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/7-01%E9%80%90%E6%AC%A1%E9%80%BC%E8%BF%91%E5%9E%8BADC.png" width="40%">
</div><div align=center>
图7-1 逐次逼近型ADC-ADC0809
</div>

> ADC0809是一个独立的8位逐次逼近型ADC，单片机内部没有集成ADC时需要外挂ADC芯片，ADC0809就是这么一款经典的ADC芯片。现在很多单片机内部已经集成了ADC外设，就不需要外挂芯片，可以直接测量电压。
> - IN7\~IN0：8路模拟输入。
> - ADDA、ADDB、ADDC、ALE：地址锁存，选择当前的模拟输入引脚。相当于38译码器。
> - CLOCK：时钟线。
> - START：开始AD转换。
> - EOC：转换结束标志位。
> - 内部DAC：加权电阻网络，用于产生和输入模拟信号进行比较的模拟信号。
> - OE：输出使能，控制三态门输出。
> - D7\~D0：输出的8位数字信号。
> - V~REF(+)~、V~REF(-)：参考电压。

下面是stm32中的单路ADC框图：
<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/7-2%E5%8D%95%E4%B8%AAADC%E6%A1%86%E5%9B%BE1.png" width="70%">
</div><div align=center>
图7-2 单个ADC框图
</div>

> - ADCx_IN0\~ADCx_IN15、温度传感器、V~REFINT~：ADC的16个输入通道。
> - 注入通道【使用不多】：最多一次性选4路通道，配合4个16位寄存器，就可以一次性转换4路模拟数据。
> - 规则通道【常用】：最多一次性选16路通道，但只有1个16位寄存器，存在新来的数据覆盖上一个数据的问题，此时要么尽快将数据取走，要是使用DMA帮助转运数据，进而可以实现一次性转换16路模拟数据。当然，一次就选一个通道，就是普通的ADC功能。
> - 触发转换电路：stm32中的ADC触发方式：
> > - 软件触发：在程序中手动调一句代码。
> > - 硬件触发：上图所示的触发源。主要来自于定时器TIMx，也可以外部中断引脚EXTI。正常思路是：定时器每隔1ms产生一次中断 --> 中断函数中开启触发转换信号 --> ADC完成一次转换。缺点是需要频繁进入中断，消耗软件资源。但是得益于上图的硬件电路设计，stm32可以直接使用定时器主模式触发ADC转换，硬件全自动无需申请中断，可以极大地减轻CPU负担。
> - V~DDA~、V~SSA~：ADC的供电引脚。
> - V~REF+~、V~REF-~：ADC的参考电压，决定了ADC的输入电压的范围。stm32内部已经和V~DDA~、V~SSA~连接在一起了。
> - ADCCLK：来自ADC的预分频器，这个ADC的预分频器则来自于“RCC时钟树”。具体可以查看时钟树的电路，默认情况就是对72MHz进行ADC预分频，由于ADCCLK最大18MHz，所以只能选择6分频/8分频。
> - DMA请求：触发DMA进行数据转运。下一章讲。
> - 注入通道数据寄存器、规则通道数据寄存器：用于存放转换结果。
> - 模拟看门狗：一旦高于上阈值或低于下阈值，就会申请模拟看门狗的中断，最终进入NVIC。
> - 转换结束EOC：规则通道转换完成，会在状态寄存器置标志位。
> - 注入转换结束JEOC：注入通道转换完成，会在状态寄存器置标志位。
> - NVIC：嵌套向量中断控制器，控制是否响应上面这三个中断。


### 7.1.2 引脚复用关系
<div align=center>
表7-1 ADC通道和引脚复用的关系
</div><div align=center>
<style type="text/css">
.tg  {border-collapse:collapse;border-spacing:0;}
.tg td{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  overflow:hidden;padding:10px 5px;word-break:normal;}
.tg th{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  font-weight:normal;overflow:hidden;padding:10px 5px;word-break:normal;}
.tg .tg-8dei{color:#000000;font-family:inherit;text-align:center;vertical-align:top}
.tg .tg-hddh{color:#000000;font-family:inherit;font-weight:bold;text-align:center;vertical-align:top}
</style>
<table class="tg">
<thead>
  <tr>
    <th class="tg-hddh"><span style="font-weight:bold">通道</span></th>
    <th class="tg-hddh"><span style="font-weight:bold">ADC1</span></th>
    <th class="tg-hddh"><span style="font-weight:bold">ADC2</span></th>
    <th class="tg-hddh"><span style="font-weight:bold">ADC3</span></th>
  </tr>
</thead>
<tbody>
  <tr>
    <td class="tg-8dei">通道0</td>
    <td class="tg-hddh">PA0</td>
    <td class="tg-hddh">PA0</td>
    <td class="tg-8dei">PA0</td>
  </tr>
  <tr>
    <td class="tg-8dei">通道1</td>
    <td class="tg-hddh">PA1</td>
    <td class="tg-hddh">PA1</td>
    <td class="tg-8dei">PA1</td>
  </tr>
  <tr>
    <td class="tg-8dei">通道2</td>
    <td class="tg-hddh">PA2</td>
    <td class="tg-hddh">PA2</td>
    <td class="tg-8dei">PA2</td>
  </tr>
  <tr>
    <td class="tg-8dei">通道3</td>
    <td class="tg-hddh">PA3</td>
    <td class="tg-hddh">PA3</td>
    <td class="tg-8dei">PA3</td>
  </tr>
  <tr>
    <td class="tg-8dei">通道4</td>
    <td class="tg-hddh">PA4</td>
    <td class="tg-hddh">PA4</td>
    <td class="tg-8dei">PF6</td>
  </tr>
  <tr>
    <td class="tg-8dei">通道5</td>
    <td class="tg-hddh">PA5</td>
    <td class="tg-hddh">PA5</td>
    <td class="tg-8dei">PF7</td>
  </tr>
  <tr>
    <td class="tg-8dei">通道6</td>
    <td class="tg-hddh">PA6</td>
    <td class="tg-hddh">PA6</td>
    <td class="tg-8dei">PF8</td>
  </tr>
  <tr>
    <td class="tg-8dei">通道7</td>
    <td class="tg-hddh">PA7</td>
    <td class="tg-hddh">PA7</td>
    <td class="tg-8dei">PF9</td>
  </tr>
  <tr>
    <td class="tg-8dei">通道8</td>
    <td class="tg-hddh">PB0</td>
    <td class="tg-hddh">PB0</td>
    <td class="tg-8dei">PF10</td>
  </tr>
  <tr>
    <td class="tg-8dei">通道9</td>
    <td class="tg-hddh">PB1</td>
    <td class="tg-hddh">PB1</td>
    <td class="tg-8dei"></td>
  </tr>
  <tr>
    <td class="tg-8dei">通道10</td>
    <td class="tg-8dei">PC0</td>
    <td class="tg-8dei">PC0</td>
    <td class="tg-8dei">PC0</td>
  </tr>
  <tr>
    <td class="tg-8dei">通道11</td>
    <td class="tg-8dei">PC1</td>
    <td class="tg-8dei">PC1</td>
    <td class="tg-8dei">PC1</td>
  </tr>
  <tr>
    <td class="tg-8dei">通道12</td>
    <td class="tg-8dei">PC2</td>
    <td class="tg-8dei">PC2</td>
    <td class="tg-8dei">PC2</td>
  </tr>
  <tr>
    <td class="tg-8dei">通道13</td>
    <td class="tg-8dei">PC3</td>
    <td class="tg-8dei">PC3</td>
    <td class="tg-8dei">PC3</td>
  </tr>
  <tr>
    <td class="tg-8dei">通道14</td>
    <td class="tg-8dei">PC4</td>
    <td class="tg-8dei">PC4</td>
    <td class="tg-8dei"></td>
  </tr>
  <tr>
    <td class="tg-8dei">通道15</td>
    <td class="tg-8dei">PC5</td>
    <td class="tg-8dei">PC5</td>
    <td class="tg-8dei"></td>
  </tr>
  <tr>
    <td class="tg-8dei">通道16</td>
    <td class="tg-hddh">温度传感器</td>
    <td class="tg-8dei"></td>
    <td class="tg-8dei"></td>
  </tr>
  <tr>
    <td class="tg-8dei">通道17</td>
    <td class="tg-hddh">内部参考电压</td>
    <td class="tg-8dei"></td>
    <td class="tg-8dei"></td>
  </tr>
</tbody>
</table>
</div>

上表给出了stm32系列芯片中所有的ADC通道，其中 **加粗的通道** 表示stm32f103c8t6所拥有的引脚（10个外部输入引脚+2路内部引脚），注意对于c8t6这个型号来说，ADC1和ADC2共用引脚，不仅可以单独使用，可以组成更加复杂的双ADC模式。双ADC模式通过配合可以组成同步模式、交叉模式（ADC1和ADC2交叉对同一个通道进行采样，以提高采样率）等。

### 7.1.3 规则组的转换模式
stm32的ADC最多同时支持16个通道，那么ADC每次扫描1个通道还是多个通道，便是选择 **非扫描模式/扫描模式**；而对于单个通道的ADC转换来说，触发一次ADC是只转换一次，还是自动的进行连续转换，便是选择 **单次转换/连续转换**。上面这两种选择进行组合，便产生了 **规则组的4种转换模式**：

**1. 单次转换、非扫描模式：**
触发一次仅转换一次；仅序列1有效，但可以任意指定需要转换的通道。此时ADC选择一组的方式退化成只能选择一个。读取数据时，需要等待EOC标志位置1，然后从数据寄存器读取结果。如要再进行转换，就需要再次触发转换。
<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/7-03-a%E5%8D%95%E6%AC%A1%E8%BD%AC%E6%8D%A2%E9%9D%9E%E6%89%AB%E6%8F%8F%E6%A8%A1%E5%BC%8F.png" width="50%">
</div><div align=center>
图7-3-a) 单次转换、非扫描模式示意图
</div>

**2. 连续转换、非扫描模式：**
相比于上一个模式，仅需要一次触发，ADC就会在一次转换完成后立刻进入下一次转换，实现不断地自动进行转换。此时就不需要读EOC看转换是否完成，直接想读数据的时候就读。
<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/7-03-b%E8%BF%9E%E7%BB%AD%E8%BD%AC%E6%8D%A2%E9%9D%9E%E6%89%AB%E6%8F%8F%E6%A8%A1%E5%BC%8F.png" width="50%">
</div><div align=center>
图7-3-b) 连续转换、非扫描模式示意图
</div>

**3. 单次转换、扫描模式：**
相比于第一种模式，可以一次性转换多个通道，不过还是触发一次、所有通道只转换一次。
<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/7-03-c%E5%8D%95%E6%AC%A1%E8%BD%AC%E6%8D%A2%E6%89%AB%E6%8F%8F%E6%A8%A1%E5%BC%8F.png" width="60%">
</div><div align=center>
图7-3-c) 单次转换、扫描模式示意图
</div>

**4. 连续转换、扫描模式：**
不仅可以一次性转换多个通道，还可以实现触发一次、自动不间断转换。
<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/7-03-d%E8%BF%9E%E7%BB%AD%E8%BD%AC%E6%8D%A2%E6%89%AB%E6%8F%8F%E6%A8%A1%E5%BC%8F.png" width="60%">
</div><div align=center>
图7-3-d) 连续转换、扫描模式示意图
</div>

**5. 间断模式：**
除了上面四种模式，ADC还有其他的配置模式，如间断模式：每个几次转换就停下来，等待触发……
更多模式细节可以查阅参考手册“11.3 ADC功能描述”。

### 7.1.4 触发转换信号

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/7-04ADC%E5%A4%96%E9%83%A8%E8%A7%A6%E5%8F%91.png" width="70%">
</div><div align=center>
图7-4 ADC外部触发
</div>

上一小节提到，要想ADC进行转换，还需要完成 **触发** 这个操作。触发信号可以是 软件触发、硬件触发。**软件触发**可以由ADC的库函数完成；**硬件触发**见上图。
### 7.1.5 数据对齐
<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/7-05%E6%95%B0%E6%8D%AE%E5%AF%B9%E9%BD%90%E7%A4%BA%E6%84%8F%E5%9B%BE.png" width="70%">
</div><div align=center>
图7-5 数据对齐示意图
</div>

因为ADC是12位的，而寄存器宽度为16位，所有便有了数据对齐方式的选择。
> 1. 右对齐【常用】：读出的值就是实际值。
> 2. 左对齐：有时候不需要太大的分辨率，便将12位ADC的转换数据左对齐，然后只取高8位。

### 7.1.6 转换时间
低速采样可以忽略转换频率，**高速采样必须考虑转换时间** 的损耗。AD转换的步骤主要为：采样，保持，量化，编码。“采样”时间越长，越可以消除一些毛刺信号的干扰；而“量化、编码”消耗的时间则比“采样、保持”更长。在STM32中，**ADC的总转换时间** 为：
$$T_{CONV} = 采样时间 + 12.5个ADC周期$$
- 采样时间：在配置ADC的多路选择开关时可选，是ADC采样周期的倍数，如1.5倍、7.5倍、13.5倍、……、239.5倍。
- ADC周期：就是从RCC分频过来的RCCCLK(最高14MHz)，总采样时间不会小于$1\mu s$。
> 例如：当ADCCLK=14MHz，采样时间为1.5个ADC周期，$T_{CONV} = 1.5 + 12.5 = 14个ADC周期 = 1μs$。

### 7.1.7 校准
ADC有一个内置自校准模式。校准可大幅减小因内部电容器组的变化而造成的准精度误差。校准期间，在每个电容器上都会计算出一个误差修正码(数字值)，这个码用于消除在随后的转换中每个电容器上产生的误差。
> - 建议在每次上电后执行一次校准。
> - 启动校准前， ADC必须处于关电状态超过至少两个ADC时钟周期。
> - 校准过程的代码是固定的，只需要在ADC初始化之后加几句代码即可。

### 7.1.8 外围电路设计
<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/7-06ADC%E5%A4%96%E5%9B%B4%E7%94%B5%E8%B7%AF%E8%AE%BE%E8%AE%A1%E5%9B%BE.png" width="60%">
</div><div align=center>
图7-6 ADC外围电路设计图
</div>

> 在设计ADC的模拟输入源时，为确保电路安全，可选择以下几种方案：
> 1. 电位器产生可调电压：注意阻值不要太小(最少为kΩ级)，以防烧毁电位器。
> 2. 传感器输出电压：如光敏电阻、热敏电阻、红外接收管、麦克风等，都可以等效为一个可变电阻。通过与一个固定电阻（应于传感器阻值相近）进行分压，从而输出可调电压，此电路图中输出电压与传感器阻值成正比。比如本节就直接用传感器模块的AO引脚。
> 3. 简易电压转换电路：经过分压后就可以采集0~5V、0~10V的输入电压值，但是若电压再高，建议使用专用的采集芯片，如隔离放大器等，做好高低电压的隔离进而保护电路安全。


### 7.1.9 总结
<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/7-03ADC%E5%9F%BA%E6%9C%AC%E7%BB%93%E6%9E%84.png" width="70%">
</div><div align=center>
图7-7 ADC基本结构
</div>

上图给出了ADC的基本结构，编程时照着写就行。




## 7.2 实验：ADC单通道
需求：测量旋转电位器的模拟电压。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/7-07%E6%8E%A5%E7%BA%BF%E5%9B%BE-ADC%E5%8D%95%E9%80%9A%E9%81%93.png" width="70%">
</div><div align=center>
图7-8 ADC单通道-接线图
</div>

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/7-08%E5%87%BD%E6%95%B0%E8%B0%83%E7%94%A8-ADC%E5%8D%95%E9%80%9A%E9%81%93.png" width="25%">
</div><div align=center>
图7-9 ADC单通道-函数调用（非库函数）
</div>

代码展示：
**- main.c**
```c
#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "ADC_User.h"

int main(void){
    
    //OLED显示屏初始化
    OLED_Init();
    OLED_ShowString(1,1,"Voltage-PA0:");
    OLED_ShowString(2,1,"+00.00 V");    
    
    //ADC初始化
    ADC_User_Init();
    ADC_User_Start();
    
    while(1){
        OLED_ShowFloat(2,1,(float)ADC_User_Get()*3.3/4095,2,2);
    };
}

```

**- ADC_User.h**
```c
#ifndef __ADC_USER_H
#define __ADC_USER_H

void ADC_User_Init(void);
void ADC_User_Start(void);
uint16_t ADC_User_Get(void);

#endif

```

**- ADC_User.c**
```c
#include "stm32f10x.h"                  // Device header

//ADC初始化-规则组PA0
void ADC_User_Init(void){
    //1.开启外设时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);//6分频使得ADC时钟为12MHz
    //2.配置GPIO
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AIN;//模拟输入
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    //3.配置多路开关，选择通道进入规则组
    ADC_RegularChannelConfig(ADC1,ADC_Channel_0,1,ADC_SampleTime_1Cycles5);
    //4.配置ADC转换器
    ADC_InitTypeDef ADC_InitStructure;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;//连续转换
    ADC_InitStructure.ADC_DataAlign          = ADC_DataAlign_Right;//数据右对齐
    ADC_InitStructure.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None;//不使用外部触发（软件触发）
    ADC_InitStructure.ADC_Mode               = ADC_Mode_Independent;//独立模式
    ADC_InitStructure.ADC_NbrOfChannel       = 1;//只有1个通道（非扫描模式，参数不起作用）
    ADC_InitStructure.ADC_ScanConvMode       = DISABLE;//非扫描模式（因为是单通道）
    ADC_Init(ADC1, &ADC_InitStructure);
    //5.配置开关控制
    ADC_Cmd(ADC1, ENABLE);
    //6.进行ADC校准
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1)==SET);
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1)==SET);
}

//对ADC进行一次软件触发
void ADC_User_Start(void){
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

//获取ADC转换结果
uint16_t ADC_User_Get(void){
    //等待转换完成并读取
    while(ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC) == RESET);
    return ADC_GetConversionValue(ADC1);//硬件会自动清除EOC标志位
}

```


编程感想：
> 1. 关于函数命名。注意stm32库函数已经有了ADC打头的库函数了，如```ADC_Init()```，所以命名的时候不要再使用ADC，而可以使用ADC_User。
> 2. GPIO配置成模拟输入AIN模式。AIN模式下，GPIO口无效，可以防止GPIO的输入输出对模拟电压造成干扰。AIN模式是ADC的专属模式。实际测试中，浮空输入、上拉输入、模拟输入的展示效果几乎没有区别（但是硬件原理完全不同）。
> 3. 函数提示设置：找到扳手图标—->Text Completion栏—->把Show Code Completion List For下面的框全部勾上。
> 4. 读取规则组数据后，无需软件清除EOC标志位。参考手册中说明，读取ADCC_DR就会自动清除EOC标志位。所以参考手册还是非常重要！！
> 5. 关于数据抖动。实测发现ADC转换后的结果会抖动，若想消除这种现象，可以有以下几种方法：
> > - 迟滞比较：设置两个阈值，低于下阈值执行操作，高于上阈值执行操作。
> > - 滤波：如均值滤波（LPF）。
> > - 裁剪分辨率：去除转换结果的最后抖动的几位。






## 7.3 实验：ADC多通道
需求：同时获取电位器、光敏电阻模块、热敏电阻模块、反射红外模块共四组数字量。


<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/7-09%E6%8E%A5%E7%BA%BF%E5%9B%BE-ADC%E5%A4%9A%E9%80%9A%E9%81%93.png" width="80%">
</div><div align=center>
图7-10 ADC多通道-接线图
</div>

**代码调用**和上一小节的实验相同；由于使用软件实现ADC多通道，所以ADC_User部分代码仅增添了两个函数，下面的**代码展示**仅给出修改过的部分：
**- main.c**
```c
#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "ADC_User.h"

int main(void){
    int i = 0;
    //OLED显示屏初始化
    OLED_Init();
    OLED_ShowString(1,1,"C0:00.00 V");
    OLED_ShowString(2,1,"C1:00.00 V");
    OLED_ShowString(3,1,"C2:00.00 V");
    OLED_ShowString(4,1,"C3:00.00 V");
    
    //ADC初始化
    ADC_User_InitMuti();
    
    while(1){
        for (i=0;i<4;i++){
            ADC_User_MutiSel(i);
            ADC_User_Start();
            OLED_ShowFloat(i+1,4,(float)ADC_User_Get()*3.3/4095,2,2);
        }
    };
}

```

**- ADC_User.c**
```c
//ADC多通道初始化-ADC1的通道0~3-PA0~PA3共四个通道
void ADC_User_InitMuti(void){
    //1.开启外设时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);//6分频使得ADC时钟为12MHz
    //2.配置GPIO
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AIN;//模拟输入
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    //3.配置多路开关，选择通道进入规则组
    ADC_RegularChannelConfig(ADC1,ADC_Channel_0,1,ADC_SampleTime_1Cycles5);
    //4.配置ADC转换器
    ADC_InitTypeDef ADC_InitStructure;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;//单次转换
    ADC_InitStructure.ADC_DataAlign          = ADC_DataAlign_Right;//数据右对齐
    ADC_InitStructure.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None;//不使用外部触发（软件触发）
    ADC_InitStructure.ADC_Mode               = ADC_Mode_Independent;//独立模式
    ADC_InitStructure.ADC_NbrOfChannel       = 1;//只有1个通道（非扫描模式，参数不起作用）
    ADC_InitStructure.ADC_ScanConvMode       = DISABLE;//非扫描模式（因为是单通道）
    ADC_Init(ADC1, &ADC_InitStructure);
    //5.配置开关控制
    ADC_Cmd(ADC1, ENABLE);
    //6.进行ADC校准
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1)==SET);
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1)==SET);
}

//使用ADC的多路开关，选择哪个通道
//通道范围0~3
void ADC_User_MutiSel(uint16_t channelx){
    switch(channelx){
        case 0: ADC_RegularChannelConfig(ADC1,ADC_Channel_0,1,ADC_SampleTime_1Cycles5); break;
        case 1: ADC_RegularChannelConfig(ADC1,ADC_Channel_1,1,ADC_SampleTime_1Cycles5); break;
        case 2: ADC_RegularChannelConfig(ADC1,ADC_Channel_2,1,ADC_SampleTime_1Cycles5); break;
        case 3: ADC_RegularChannelConfig(ADC1,ADC_Channel_3,1,ADC_SampleTime_1Cycles5); break;
        default: ADC_RegularChannelConfig(ADC1,ADC_Channel_0,1,ADC_SampleTime_1Cycles5); break;
    }
}
//注意别忘了在ADC_User.h头文件中声明
```


编程感想：
> 1. 如何实现多通道ADC。若使用扫描模式实现多通道ADC，需要考虑数据覆盖的问题。下面是几种实现ADC多通道的思路：
> > - 扫描模式+DMA转运数据：DMA是转运多通道数据的**最优解**，但下节才学DMA，本节用不了。
> > - 扫描模式+手动转运数据：存在两个问题，一个是ADC在最后一个通道转换完成后才会产生EOC标志位，此时，数据寄存器早就被覆盖成最后一个通道的数据了，所以无法确定某个通道的转运时刻；ADC转换速度非常快，对于手动转运数据的要求非常高。解决思路就是使用间断模式，可以使ADC每转换一个通道就暂停一次，等待下一次触发才进行下一个通道的转换。于是便可以：触发-->Delay一段足够长的时间-->手动转运完数据-->触发……不难发现，**效率极低**。
> > - 非扫描模式+“时分复用”**【本节思路】**：还是采用“单次转换、非扫描模式”的单路ADC，但是可以不断第更换通道-->触发ADC-->读取数据，以软件完成扫描模式，进而实现多路ADC“单次转换、扫描模式”的功能。
