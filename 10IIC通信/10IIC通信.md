# 10 I2C通信
 [toc]

注：笔记主要参考B站 [江科大自化协](https://space.bilibili.com/383400717) 教学视频“[STM32入门教程-2023持续更新中](https://www.bilibili.com/video/BV1th411z7sn/)”。
注：工程及代码文件放在了本人的[Github仓库](https://github.com/jjejdhhd/Learn_stm32f103/tree/main)。

 <!--
 下面是图片格式
<div align=center>
<img src="" width="70%">
</div><div align=center>
图x-x ??
</div>






下面是实验模块的模式
<div align=center>
<img src="" width="70%">
</div><div align=center>
图x-x ？？-接线图
</div>

<div align=center>
<img src="" width="70%">
</div><div align=center>
图x-x ？？-代码调用（非库函数）
</div>

下面是代码展示：
**- main.c**
```c

```

**- ??.h**
```c

```

**- ??.c**
```c

```

编程感想：
> 1. 
-->


***



## 10.1 I2C通信
<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/10-01IIC%E6%A8%A1%E5%9D%97.png" width="80%">
</div><div align=center>
图10-1 采用I2C协议的部分模块实物图
</div>

**I2C总线（Inter IC BUS）** 是由Philips公司开发的一种通用数据总线，应用广泛，下面是一些指标参数：
> - 两根通信线：SCL（Serial Clock，串行时钟线）、SDA（Serial Data，串行数据线）。
> - **同步**，半双工。
> - 主从机通信过程中，会带数据应答。
> - 支持总线挂载多设备（**一主多从**、多主多从）。“多主多从”模式更复杂，下面没有特殊说明，默认都是“一主多从”模式。
> - **高位先行**。
>
> 那同步时序和异步时序的优缺点各是什么呢？
> - 同步时序：优点是对时间要求不严格，可以软件手动翻转电平实现通信，所以可以大幅降低单片机对外围硬件电路的依赖，在一些低端单片机没有硬件资源的情况下，很容易使用软件来模拟时序；缺点就是多一个时钟线。
> - 异步时序：优点是少一根时钟线，节省资源；缺点是对时间要求严格，对硬件电路的依赖严重。
>
> 注：至于为什么学完串口，又要学习I2C协议，新手建议听听UP原视频“大公司要给我一千万”的例子——[P31的3：00~14:53](https://www.bilibili.com/video/BV1th411z7sn/?p=31)，引人入胜。


作为一个通信协议，就必须在硬件和软件上都做出规定：
> - 硬件上规定电路如何连接、端口的输入输出模式等
> - 软件上规定通信时序、字节如何传输、高位/低位先行等

下面首先来看I2C的硬件规定：

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/10-02%E4%B8%80%E4%B8%BB%E5%A4%9A%E4%BB%8E%E7%94%B5%E8%B7%AF%E5%9B%BE.png" width="80%">
</div><div align=center>
图10-2 I2C硬件电路示意图——一主多从
</div>

> 硬件电路规定所有I2C设备的SCL连在一起，SDA连在一起。设备的SCL和SDA均要配置成 **开漏输出模式**。SCL和SDA各添加一个 **上拉电阻**，阻值一般为4.7KΩ左右。
> 总结：为防止总线没协调好，导致电源短路，采用 <u>外置弱上拉电阻+开漏输出</u> 的电路结构。
> - CPU：主机，对总线控制权力大。任何时候，都是主机对SCL完全控制。空闲状态下，主机可以主动发起对SDA的控制。
> - 被控IC~x~：从机，对总线控制权力小。任何时刻，从机都只能被动读取SCL线。只有在从机发送数据、或者从机发送应答位时，从机才短暂的具有对SDA的控制权。
> - 从机设备地址：为了保证通信正常，每个从机设备都具有一个唯一的地址。在I2C协议标准中，从机设备地址分为7位地址、10位地址，其中7位较为简单且应用更广泛。不同的I2C模块出厂时，厂商都会为其分配唯一的7位地址，具体可以在芯片手册中查询，如MPU6050的地址是110_1000，AT24C02的地址是101_0000等，其中器件地址的最后几位是可以在电路中改变的。总线上都是不同模块一般不会有地址冲突，若总线上有相同的模块就需要外界电路来相应的器件地址。
> - 通信引脚结构：输入线都很正常。输出线则采用开漏输出，引脚下拉是“强下拉”，引脚上拉则处于“浮空状态”。于是所有设备都只能输出低电平，为了避免高电平造成的引脚浮空，就需要在总线外面设置上拉电阻（弱上拉）。采用这个模式的好处：
> > 1. 完全杜绝了电源短路的情况，保证了电路安全。
> > 2. 避免了引脚模式的频繁切换。开漏+弱上拉的模式，同时兼具了输入和输出的功能。要想输出就直接操纵总线；要想输入就直接输出高电平，然后读取总线数据，无需专门将GPIO切换成输入模式。
> > 3. 此模式可以实现“线与”功能。只有总线上所有设备都输出高电平，总线才是高电平；否则只要有一个设备输出低电平，总线就是低电平。I2C可以利用这个特征，执行多主机模式下的时钟同步（所以SCL也采用此模式）和总线仲裁。

下面介绍I2C通信的软件规定——时序结构（一主多从）：
> 下面的这些操作的主语都是“主机”。
> - **起始条件**：SCL高电平期间，SDA从高电平切换到低电平。
> - **终止条件**：SCL高电平期间，SDA从低电平切换到高电平。
> <img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/10-02%E6%97%B6%E5%BA%8F-%E8%B5%B7%E5%A7%8B%E5%92%8C%E7%BB%88%E6%AD%A2.png" width="40%">
> - **发送一个字节**：SCL低电平期间，主机将数据位依次放到SDA线上（**高位先行**），然后释放SCL，从机将在SCL高电平期间读取数据位，所以SCL高电平期间SDA不允许有数据变化，依次循环上述过程8次，即可发送一个字节。
> <img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/10-02%E6%97%B6%E5%BA%8F-%E5%8F%91%E9%80%81%E4%B8%80%E4%B8%AA%E5%AD%97%E8%8A%82.png" width="60%">
> - **接收一个字节**：主机在接收之前释放SDA，只控制SCL变化。SCL低电平期间，从机将数据位依次放到SDA线上（高位先行，且一般贴着SCL下降沿变化）；SCL高电平期间，主机读取数据位，所以SCL高电平期间SDA不允许有数据变化，依次循环上述过程8次，即可接收一个字节。
> <img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/10-02%E6%97%B6%E5%BA%8F-%E6%8E%A5%E6%94%B6%E4%B8%80%E4%B8%AA%E5%AD%97%E8%8A%82.png" width="60%">
> - **发送应答**：主机在接收完一个字节之后，在下一个时钟发送一位数据，数据0表示应答，数据1表示非应答。若从机没有收到主机的应答，就会完全释放SDA的控制权，回到待机模式。
> - **接收应答**：主机在发送完一个字节之后，在下一个时钟接收一位数据，判断从机是否应答，数据0表示应答，数据1表示非应答（主机在接收之前，需要释放SDA）。
> <img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/10-02%E6%97%B6%E5%BA%8F-%E5%8F%91%E9%80%81%E5%92%8C%E6%8E%A5%E6%94%B6%E5%BA%94%E7%AD%94.png" width="50%">
>
> 于是下面就可以拼凑出完整的数据帧：
> - **指定地址写**：对于指定设备（Slave Address），在指定地址（Reg Address）下，写入指定数据（Data）。
> <img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/10-02%E6%8C%87%E5%AE%9A%E5%9C%B0%E5%9D%80%E5%86%99.png" width="80%">
> - **当前地址读**：对于指定设备（Slave Address），在 **当前地址指针** （上电默认0x00）指示的地址下，读取从机数据（Data）。
> <img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/10-02%E6%97%B6%E5%BA%8F-%E5%BD%93%E5%89%8D%E5%9C%B0%E5%9D%80%E8%AF%BB.png" width="60%">
> - **指定地址读**（“哑写”更换地址）：对于指定设备（Slave Address），在指定地址（Reg Address）下，读取从机数据（Data）。
> <img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/10-02%E6%97%B6%E5%BA%8F-%E6%8C%87%E5%AE%9A%E5%9C%B0%E5%9D%80%E8%AF%BB.png" width="99%">
>
> 注：额外还有“连续写”、“连续读”就不多做介绍了。















## 10.2 MPU6050简介
<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/10-03MPU6050%E7%A4%BA%E6%84%8F%E5%9B%BE.png" width="70%">
</div><div align=center>
图10-3 MPU6050示意图
</div>

MPU6050是一个**6轴姿态传感器**，可以测量芯片自身X、Y、Z轴的加速度、角速度参数，通过数据融合，可进一步得到姿态角（欧拉角），常应用于平衡车、飞行器等需要检测自身姿态的场景。以飞机为例，**欧拉角**就是飞机机身相对于初始3个轴的夹角——俯仰（Pitch）、滚转（Roll）、偏航（Yaw）。但是单一的加速度计、陀螺仪、磁力计都不能获得精确且稳定的欧拉角，只有综合多种传感器进行数据融合、取长补短，才能获得精确且稳定的欧拉角。**常见的数据融合算法，一般有互补滤波、卡尔曼滤波等（惯性导航领域——姿态解算）。** 下面给出MPU6050芯片的一些参数：
> - 3轴加速度计（**Accel**erometer）：测量X、Y、Z轴的**加速度**。本质上就是弹簧测力计：$F=ma$。测量的值是<u>合加速度</u>，由于系统运动时有运动加速度的影响，所以只有在静态时才表示系统的姿态。也就是说，**加速度计静态稳定，而动态不稳定。**
> - 3轴陀螺仪传感器（**Gyro**scope）：测量X、Y、Z轴的**角速度**。若想得到角度，只需要对角速度进行积分即可。陀螺仪的缺点是，物体静止时，会由于噪声导致角速度无法完全归零，于是就会导致积分得到的角度产生缓慢的漂移，积分时间越长漂移越明显，但当物体运动时就可以忽略漂移的大小。也就是说，**陀螺仪动态稳定，而静态不稳定。**
> - 3轴磁场传感器（选修）：测量X、Y、Z轴的磁场强度，某些芯片会有此功能。
> - 气压传感器（选修）：测量气压大小，一般气压值反映高度信息（海拔高度），某些芯片会有此功能。
> 
> 注：于是姿态解算的大题思路就是：将“加速度计”、“陀螺仪”进行互补滤波，融合得到静态和动态都稳定的姿态角。
> 注：上图所示的机械结构只是便于理解，实际MPU6050芯片中不存在图示的机械结构。
> 
> - **16位ADC**采集传感器的模拟信号，量化范围：-32768~32767（有符号数）。
> - 加速度计满量程选择：±2、±4、±8、±16（g，1g=9.8m/s^2^）。
> - 陀螺仪满量程选择： ±250、±500、±1000、±2000（°/sec）。
> - 可配置的数字低通滤波器，使输出数据更加平缓。
> - 可配置的时钟源、可配置的采样分频。时钟源经过分频后，为内部的ADC提供转换时钟
> - I2C从机地址（手册可以查看到）：MPU6050的设备号固定为0x68（某些批次可能为0x98），所以可以用于验证I2C读出协议是否正常。对于出厂地址为0x68的芯片来说，调整AD0引脚电压就可以改变其从机地址：**1101000（AD0=0）、1101001（AD0=1）**。


<div align=center>
表10-1 MPU6050模块-引脚描述
</div><div align=center>
<style type="text/css">
.tg  {border-collapse:collapse;border-spacing:0;}
.tg td{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  overflow:hidden;padding:10px 5px;word-break:normal;}
.tg th{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  font-weight:normal;overflow:hidden;padding:10px 5px;word-break:normal;}
.tg .tg-baqh{text-align:center;vertical-align:top}
.tg .tg-1pye{color:#000000;font-weight:bold;text-align:center;vertical-align:top}
.tg .tg-hddh{color:#000000;font-family:inherit;font-weight:bold;text-align:center;vertical-align:top}
</style>
<table class="tg">
<thead>
  <tr>
    <th class="tg-hddh"><span style="font-weight:bold">引脚</span></th>
    <th class="tg-1pye"><span style="font-weight:bold">功能</span></th>
  </tr>
</thead>
<tbody>
  <tr>
    <td class="tg-baqh"><span style="color:#000">VCC、GND</span></td>
    <td class="tg-baqh"><span style="color:#000">电源</span></td>
  </tr>
  <tr>
    <td class="tg-baqh"><span style="color:#000">SCL、SDA</span></td>
    <td class="tg-baqh"><span style="color:#000">I2C通信引脚</span></td>
  </tr>
  <tr>
    <td class="tg-baqh"><span style="color:#000">XCL、XDA</span></td>
    <td class="tg-baqh"><span style="color:#000">主机I2C通信引脚</span></td>
  </tr>
  <tr>
    <td class="tg-baqh"><span style="color:#000">AD0</span></td>
    <td class="tg-baqh"><span style="color:#000">从机地址最低位</span></td>
  </tr>
  <tr>
    <td class="tg-baqh"><span style="color:#000">INT</span></td>
    <td class="tg-baqh"><span style="color:#000">中断信号输出</span></td>
  </tr>
</tbody>
</table>
</div>

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/10-04MPU6050%E7%A1%AC%E4%BB%B6%E7%94%B5%E8%B7%AF%E5%9B%BE.png" width="70%">
</div><div align=center>
图10-4 MPU6050模块-硬件电路
</div>

> - SCL、SDA：模块已经内置了上拉电阻，所以可以将端口直接用线连在GPIO口上。
> - XCL、XDA：使I2C可以作为主机，扩展主机功能，使其可以外接磁力计或气压计。因为只根据加速度计和陀螺仪融合出来的姿态角，无法准确计算出绕Z轴的角度（偏航角），因为其漂移无法通过加速度计进行修正，此时就需要外接指南针（磁力计）来提供长时间的稳定偏航角进行参考。另外要进行定高飞行，也需要外接一个气压计来感知海拔高度。然后MPU6050读取到这些扩展模块的数据，就可以通过其内部的DMP单元进行数据融合和姿态解算。当然也可以将磁力计或气压计直接挂载到I2C总线上，由stm32进行解算。
> - AD0引脚：扩展芯片的从机地址。模块电路中将AD0下拉接地，所以不接其他外部信号，从机地址固定为1101000。当然也可以外部强上拉成高电平。
> - INT中断输出引脚：可以配置芯片内部的一些事件，来触发中断引脚的输出，如数据准备好、I2C主机错误、自由落体检测、运动检测、零运动检测等。
> LDO低压差线性稳压器：MPU6050供电范围是2.375\~3.46V，为了扩大模块的供电范围，模块设计者添加了LDO电路。其输入电压范围为3.3\~5V，经过稳压器，可以输出稳定的3.3V，给模块上的芯片MPU6050供电。当然，如果已经有稳定的3.3V电源，就不再需要这部分电路了。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/10-05MPU6050%E8%8A%AF%E7%89%87%E6%A1%86%E5%9B%BE.png" width="75%">
</div><div align=center>
图10-5 MPU6050-芯片框图
</div>

> - 时钟系统：可以选择外部时钟电路。但一般选择内部时钟，所以模块中没有涉及这两个引脚。
> - 传感器部分：包括三个加速度计、三个陀螺仪、一个温度传感器。这些传感器本质上都相当于可变电阻，分压后的模拟电压经过16位ADC转换后，可以将数据输出到数据寄存器存起来。芯片内部的转换+转运都是全自动完成的，并且不存在数据覆盖的问题。只需要配置转换速率即可。
> - 自测单元：用于验证芯片是否正常。启动自测单元后，芯片内部会模拟一个外力，这个外力会导致传感器数据比平常大。于是进行自测的步骤就是：先使能自测读取数据，再失能自测读取数据，两者相减就可以得到“自测响应”。芯片手册中有这个“自测响应”的范围，若不在范围内，那就说明该传感器有问题不宜使用。
> - 电荷泵：电荷泵是一种升压电路。CPOUT引脚需要外接一个电容。通过不断地快速给外接电容并联充电、串联放电，就可以实现升压；再接上一个电压滤波电路，便可以输出一个稳定的升压。注意到这个升压电路是供给陀螺仪传感器使用的。
> - 寄存器和接口部分：
> > - 中断状态寄存器：可以控制内部哪些事件到中断引脚的输出。
> > - FIFO：先入先出寄存器，可以对数据流进行缓存。本节暂时不用。
> > - 配置寄存器：可以对内部的各个电路进行配置。
> > - 传感器数据寄存器：存储各个传感器的数据。
> > - 工厂校准：意思就是内部的传感器都进行了校准。无需了解。
> > - 数字运动处理器：简称DMP。是芯片内部自带的姿态解算的硬件算法，配合官方的DMP库，就可以进行姿态解算。本节暂时不涉及。
> > - FSYNC：帧同步。暂时不涉及。
> > - 从机I2C/SPI接口：从机的I2C、SPI接口，用于和STM32通信。
> > - 主机I2C接口：用于和MPU6050扩展的设备进行通信。
> > - 旁路选择器：前面说，MPU6050是stm32的小弟之一，而MPU6050自己也可以接很多小弟。开关拨到上面，小弟合并，导致stm32是所有设备的大哥；开关拨到下面，stm32只是MPU6050的大哥，而MPU6050又是扩展设备的大哥，这两管理系统相互独立、互不干扰。本节不会用到此功能。
> - 供电部分：按照手册要求供电即可。














## 10.3 实验：软件读写MPU6050
需求：通过I2C通信协议，对MPU6050内部的寄存器进行读写。写入到配置寄存器，就可以对这个外挂模块进行配置。读出数据寄存器，就可以获取外挂模块的数据。最终将读出的数据显示在OLED屏幕上。要求显示：设备的ID号、加速度传感器的三个输出数据、陀螺仪传感器的三个输出数据（角速度）。
> 注：串口是异步通信，时序严格，所以一般不用软件模拟；I2C等同步通信，对时序要求不严格，可以很方便的进行软件模拟。
> 注：软件模拟时，SCL和SDA都是普通的GPIO口，所以可以随便接。
> 注：SCL和SDA都应该有一个上拉电阻，在模块的电路中已经设计了这个上拉电阻，所以可以跳线直连GPIO口。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/10-06%E6%8E%A5%E7%BA%BF%E5%9B%BE-%E8%BD%AF%E4%BB%B6%E8%AF%BB%E5%86%99MPU6050.png" width="75%">
</div><div align=center>
图10-6 软件读写MPU6050-接线图
</div>

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/10-07%E4%BB%A3%E7%A0%81%E8%B0%83%E7%94%A8-%E8%BD%AF%E4%BB%B6%E8%AF%BB%E5%86%99MPU6050.png" width="25%">
</div><div align=center>
图10-7 软件读写MPU6050-代码调用（非库函数）
</div>


下面是代码展示：Delay、OLED相关代码省略。
**- main.c**
```c
#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "MPU6050.h"

int main(void){
    MPU6050_DataStruct SensorData;//存放6轴传感器数据
        
    OLED_Init();   //OLED初始化 
    MPU6050_Init();//MPU6050初始化
    
    //显示一些基本信息
    OLED_ShowString(1,1,"ID:");
    OLED_ShowHexNum(1,4,MPU6050_GetID(),2);
//    OLED_ShowString(1,1,"Acce:  | Gyro:");
    OLED_ShowString(2,1,"       |");
    OLED_ShowString(3,1,"       |");
    OLED_ShowString(4,1,"       |");
    //不断获取传感器值并显示
    while(1){
        MPU6050_GetData(&SensorData);
        //原始数值
//        OLED_ShowSignedNum(2,1,SensorData.Acce_X,5);
//        OLED_ShowSignedNum(3,1,SensorData.Acce_Y,5);
//        OLED_ShowSignedNum(4,1,SensorData.Acce_Z,5);
//        OLED_ShowSignedNum(2,9,SensorData.Gyro_X,5);
//        OLED_ShowSignedNum(3,9,SensorData.Gyro_Y,5);
//        OLED_ShowSignedNum(4,9,SensorData.Gyro_Z,5);
        //转换成小数的数值
        OLED_ShowFloat(2,1,(float)SensorData.Acce_X/32768*20,3,1);
        OLED_ShowFloat(3,1,(float)SensorData.Acce_Y/32768*20,3,1);
        OLED_ShowFloat(4,1,(float)SensorData.Acce_Z/32768*20,3,1);
        OLED_ShowFloat(2,9,(float)SensorData.Gyro_X/32768*500,3,1);
        OLED_ShowFloat(3,9,(float)SensorData.Gyro_Y/32768*500,3,1);
        OLED_ShowFloat(4,9,(float)SensorData.Gyro_Z/32768*500,3,1);
    };
}

```

**- I2C_User.h**
```c
#ifndef __I2C_USER_H
#define __I2C_USER_H

void  I2C_User_Init(void);
void I2C_User_Start(void);
void I2C_User_Stop(void);
void I2C_User_SendByte(uint8_t send_byte);
uint8_t I2C_User_RecvByte(void);
void I2C_User_SendAck(uint8_t AckBit);
uint8_t I2C_User_RecvAck(void);

#endif

```

**- I2C_User.c**
```c
//本文件 定义I2C的6个基本的时序单元，供其他模块调用
#include "stm32f10x.h"                  // Device header
#include "Delay.h"

//引脚操作的封装和改名，方便移植——移植时仅需修改本部分即可
//////////////////////////////////////////////////////////////////////////////
//定义I2C通信的两个引脚-PB10为SCL、PB11为SDA
#define I2C_User_SCL_Port GPIOB
#define I2C_User_SDA_Port GPIOB
#define I2C_User_SCL GPIO_Pin_10
#define I2C_User_SDA GPIO_Pin_11
//#define I2C_User_SCL_High GPIO_SetBits  (I2C_User_SCL_Port, I2C_User_SCL)
//#define I2C_User_SCL_Low  GPIO_ResetBits(I2C_User_SCL_Port, I2C_User_SCL)
//#define I2C_User_SDA_High GPIO_SetBits  (I2C_User_SDA_Port, I2C_User_SDA)
//#define I2C_User_SDA_Low  GPIO_ResetBits(I2C_User_SDA_Port, I2C_User_SDA)
//写SCL的操作
void I2C_User_W_SCL(uint8_t BitValue){
    GPIO_WriteBit(I2C_User_SCL_Port,I2C_User_SCL,(BitAction)BitValue);
    Delay_us(3);// 延迟一位（I2C最大通信速率400kHz-2.5us）
}

//写SDA的操作
void I2C_User_W_SDA(uint8_t BitValue){
    GPIO_WriteBit(I2C_User_SDA_Port,I2C_User_SDA,(BitAction)BitValue);
    Delay_us(3);// 延迟一位（I2C最大通信速率400kHz-2.5us）
}

//读SDA的操作
uint8_t I2C_User_R_SDA(void){
    uint8_t BitValue=0;
    BitValue = GPIO_ReadInputDataBit(I2C_User_SDA_Port,I2C_User_SDA);
    Delay_us(3);// 延迟一位（I2C最大通信速率400kHz-2.5us）
    return BitValue;
}

//初始化两个GPIO口
void  I2C_User_Init(void){
    // 开启APB2-GPIOB的外设时钟RCC
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    // 初始化PA的输出端口：定义结构体及参数
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = I2C_User_SCL | I2C_User_SDA;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;//开漏输出
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    // 默认输出为高电平-释放总线
    GPIO_SetBits(GPIOB, I2C_User_SCL | I2C_User_SDA);
}
//////////////////////////////////////////////////////////////////////////////

// 下面是I2C的六个基本时序-移植时无需修改
// 注：除了终止条件，SCL以高电平结束；其他时序都以SCL低电平结束
//////////////////////////////////////////////////////////////////////////////
//1. 发送起始位
void I2C_User_Start(void){
    // 调整SCL、SDA输出均为高电平-保险起见，先将SDA拉高
    I2C_User_W_SDA(1);
    I2C_User_W_SCL(1);
    // SDA输出下降沿
    I2C_User_W_SDA(0);
    // 拉低SCL
    I2C_User_W_SCL(0);
}

//2. 发送终止位
void I2C_User_Stop(void){
    // 调整SCL、SDA输出均为低电平
    I2C_User_W_SCL(0);
    I2C_User_W_SDA(0);
    // 拉高SCL
    I2C_User_W_SCL(1);
    // 拉高SDA
    I2C_User_W_SDA(1);  
}
    
//3. 发送一个字节
void I2C_User_SendByte(uint8_t send_byte){
    uint8_t i=0;
    for(i=0;i<8;i++){
        //改变SDA
        I2C_User_W_SDA((0x80>>i) & send_byte);
        //拉高SCL
        I2C_User_W_SCL(1);
        //拉低SCL
        I2C_User_W_SCL(0);
    }
}

//4. 接收一个字节
uint8_t I2C_User_RecvByte(void){
    uint8_t recv_byte=0x00;
    uint8_t i=0;
    //主机释放总线
    I2C_User_W_SDA(1);
    //接收数据
    for(i=0;i<8;i++){
        // 拉高SCL
        I2C_User_W_SCL(1);
        // 读取数据
        if(I2C_User_R_SDA()==1){
            recv_byte |= (0x80>>i);
        }
        // 拉低SCL
        I2C_User_W_SCL(0);
    }
    return recv_byte;
}

//5. 发送应答位
void I2C_User_SendAck(uint8_t AckBit){
    // 将应答位放在SDA上
    I2C_User_W_SDA(AckBit);
    // 拉高SCL
    I2C_User_W_SCL(1);
    // 拉低SCL
    I2C_User_W_SCL(0);
}

//6. 接收应答位
uint8_t I2C_User_RecvAck(void){
    uint8_t AckBit=0;
    //主机释放总线
    I2C_User_W_SDA(1);
    // 拉高SCL
    I2C_User_W_SCL(1);
    // 读取应答信号
    AckBit = I2C_User_R_SDA();
    // 拉低SCL
    I2C_User_W_SCL(0);
    return AckBit;
}
//////////////////////////////////////////////////////////////////////////////

```

**- MPU6050.h**
```c
#ifndef __MPU6050_H
#define __MPU6050_H

//定义MPU6050的6轴数据结构体
typedef struct{
    int16_t Gyro_X;
    int16_t Gyro_Y;
    int16_t Gyro_Z;
    int16_t Acce_X;
    int16_t Acce_Y;
    int16_t Acce_Z;
    int16_t Temp;
}MPU6050_DataStruct;

//外部可调用函数
void MPU6050_WriteReg(uint8_t RegAddr, uint8_t wData);
uint8_t MPU6050_ReadReg(uint8_t RegAddr);
void MPU6050_Init(void);
uint8_t MPU6050_GetID(void);
void MPU6050_GetData(MPU6050_DataStruct* MPU6050_Data);

#endif

```

**- MPU6050.c**
```c
#include "stm32f10x.h"                  // Device header
#include "I2C_User.h"
#include "MPU6050.h"

//定义从机地址、寄存器地址
#define MPU6050_ADDRESS           0xD0
#define MPU6050_REG_SMPLRT_DIV    0x19
#define MPU6050_REG_CONFIG        0x1A
#define MPU6050_REG_GYRO_CONFIG   0x1B
#define MPU6050_REG_ACCEL_CONFIG  0x1C

#define MPU6050_REG_ACCEL_XOUT_H  0x3B
#define MPU6050_REG_ACCEL_XOUT_L  0x3C
#define MPU6050_REG_ACCEL_YOUT_H  0x3D
#define MPU6050_REG_ACCEL_YOUT_L  0x3E
#define MPU6050_REG_ACCEL_ZOUT_H  0x3F
#define MPU6050_REG_ACCEL_ZOUT_L  0x40
#define MPU6050_REG_TEMP_OUT_H    0x41
#define MPU6050_REG_TEMP_OUT_L    0x42
#define MPU6050_REG_GYRO_XOUT_H   0x43
#define MPU6050_REG_GYRO_XOUT_L   0x44
#define MPU6050_REG_GYRO_YOUT_H   0x45
#define MPU6050_REG_GYRO_YOUT_L   0x46
#define MPU6050_REG_GYRO_ZOUT_H   0x47
#define MPU6050_REG_GYRO_ZOUT_L   0x48

#define MPU6050_REG_PWR_MGMT_1    0x6B
#define MPU6050_REG_PWR_MGMT_2    0x6C
#define MPU6050_REG_WHO_AM_I      0x75

//MPU6050指定地址写寄存器
void MPU6050_WriteReg(uint8_t RegAddr, uint8_t wData){
    I2C_User_Start();
    I2C_User_SendByte(MPU6050_ADDRESS);
    I2C_User_RecvAck();//暂时不对应答位进行处理
    I2C_User_SendByte(RegAddr);
    I2C_User_RecvAck();
    I2C_User_SendByte(wData);
    I2C_User_RecvAck();
    I2C_User_Stop();
}

//MPU6050指定地址读寄存器
uint8_t MPU6050_ReadReg(uint8_t RegAddr){
    uint8_t rData=0x00;
    
    I2C_User_Start();
    I2C_User_SendByte(MPU6050_ADDRESS);
    I2C_User_RecvAck();//暂时不对应答位进行处理
    I2C_User_SendByte(RegAddr);
    I2C_User_RecvAck();
    
    I2C_User_Start();
    I2C_User_SendByte(MPU6050_ADDRESS | 0x01);
    I2C_User_RecvAck();//暂时不对应答位进行处理
    rData = I2C_User_RecvByte();
    I2C_User_SendAck(1);
    I2C_User_Stop();
    
    return rData;
}

//MPU6050初始化
void MPU6050_Init(void){
    //I2C初始化
    I2C_User_Init();
    //不复位、解除睡眠、不开启循环模式、温度传感器失能、选择陀螺仪x轴时钟
    MPU6050_WriteReg(MPU6050_REG_PWR_MGMT_1,0x01);
    //没有开启循环模式
    MPU6050_WriteReg(MPU6050_REG_PWR_MGMT_2,0x00);
    //采样率10分频
    MPU6050_WriteReg(MPU6050_REG_SMPLRT_DIV,0x09);
    //不使用外部同步、DLPF设置等级6
    MPU6050_WriteReg(MPU6050_REG_CONFIG,0x06);
    //陀螺仪：自测失能、满量程±500°/s-000_01_000
    MPU6050_WriteReg(MPU6050_REG_GYRO_CONFIG,0x08);
    //加速度计：自测失能、满量程±2g、失能运动检测-000_00_000
    MPU6050_WriteReg(MPU6050_REG_ACCEL_CONFIG,0x00);
}

//获取MPU6050的ID号
uint8_t MPU6050_GetID(void){
    return MPU6050_ReadReg(MPU6050_REG_WHO_AM_I);
}

//获取MPU6050的传感器数据
void MPU6050_GetData(MPU6050_DataStruct* MPU6050_Data){
    uint16_t sensor_byte_L, sensor_byte_H;
    //获取加速度计数据
    sensor_byte_H = MPU6050_ReadReg(MPU6050_REG_ACCEL_XOUT_H);
    sensor_byte_L = MPU6050_ReadReg(MPU6050_REG_ACCEL_XOUT_L);
    MPU6050_Data->Acce_X = (int16_t)((sensor_byte_H<<8) | sensor_byte_L);
    sensor_byte_H = MPU6050_ReadReg(MPU6050_REG_ACCEL_YOUT_H);
    sensor_byte_L = MPU6050_ReadReg(MPU6050_REG_ACCEL_YOUT_L);
    MPU6050_Data->Acce_Y = (int16_t)((sensor_byte_H<<8) | sensor_byte_L);
    sensor_byte_H = MPU6050_ReadReg(MPU6050_REG_ACCEL_ZOUT_H);
    sensor_byte_L = MPU6050_ReadReg(MPU6050_REG_ACCEL_ZOUT_L);
    MPU6050_Data->Acce_Z = (int16_t)((sensor_byte_H<<8) | sensor_byte_L);
    //获取陀螺仪数据
    sensor_byte_H = MPU6050_ReadReg(MPU6050_REG_GYRO_XOUT_H);
    sensor_byte_L = MPU6050_ReadReg(MPU6050_REG_GYRO_XOUT_L);
    MPU6050_Data->Gyro_X = (int16_t)((sensor_byte_H<<8) | sensor_byte_L);
    sensor_byte_H = MPU6050_ReadReg(MPU6050_REG_GYRO_YOUT_H);
    sensor_byte_L = MPU6050_ReadReg(MPU6050_REG_GYRO_YOUT_L);
    MPU6050_Data->Gyro_Y = (int16_t)((sensor_byte_H<<8) | sensor_byte_L);
    sensor_byte_H = MPU6050_ReadReg(MPU6050_REG_GYRO_ZOUT_H);
    sensor_byte_L = MPU6050_ReadReg(MPU6050_REG_GYRO_ZOUT_L);
    MPU6050_Data->Gyro_Z = (int16_t)((sensor_byte_H<<8) | sensor_byte_L);
    //获取温度传感器数据
    sensor_byte_H = MPU6050_ReadReg(MPU6050_REG_TEMP_OUT_H);
    sensor_byte_L = MPU6050_ReadReg(MPU6050_REG_TEMP_OUT_L);
    MPU6050_Data->Temp = (int16_t)((sensor_byte_H<<8) | sensor_byte_L);
}

```

编程感想：
> 1. 扫描总线上所有设备的地址。进行“发送从机地址+0、接收应答、发送停止位”的操作，就可以通过应答位来判断当前地址的从机是否挂在总线上。若遍历所有的从机地址，那边可以实现扫描总线设备的操作。不过要注意遍历时地址只是前7位，第8位的R/W保持为0（写操作）不变，否则一旦交出总线控制权，从机可能会影响后续的操作。
> 2. 更改MPU6050模块地址。飞线接AD0引脚，即可改变从机地址的最低位（默认为0）。
> 3. C语言函数体返回多个参数。由于C语言函数体只能返回一个参数，所以要想返回多个参数，有以下三种方法：
> > 1. 定义外部数组变量。
> > 2. 返回数组指针。
> > 3. 定义结构体。这个看起来最简洁，所以本文定义结构体来存储数据。















## 10.4 I2C外设简介
总的来说，由于串口是异步时序，对时序要求极其严格，所以几乎不会软件模拟串口，<u>基本上一边倒的采用硬件实现串口通信</u>。而对于I2C通信来说，尽管用硬件电路可以减轻CPU负担，但是硬件I2C总线数量有限、引脚固定等，也有很多限制；而由于I2C是同步时序，软件模拟I2C足够简单且灵活，所以<u>还是有许多场合都使用软件I2C进行通信</u>。若只是简单应用I2C读写数据，使用软件模拟更加灵活；若对时序要求高，可以使用硬件I2C。本节介绍硬件实现I2C的原理。

STM32内部集成了硬件I2C收发电路，可以由**硬件自动执行**时钟生成、起始终止条件生成、应答位收发、数据收发等功能，**减轻CPU的负担**，还兼具可以实现完整的多主机通信、时序波形规整、通信速率快等优点，功能非常强大。具体参数如下：
> - 支持多主机模型（固定多主机/可变多主机）。stm32是按照“可变多主机”的模式设置硬件电路的，也就是说，需要自己声明自己是主机。
> - 支持7位/10位地址模式。10位地址发送两字节寻址，高5位固定为11110（这种开头不会在7位地址中出现），低10位作为地址（第一个字节的最低位还是R/W）。
> - 支持不同的通讯速度，标准速度(高达100 kHz)，快速(高达400 kHz)。只要不超过最大频率，多少都可以。
> - 支持DMA。多字节传输时可提高传输效率，如指定地址 读/写 多字节。若只有几个字节也没必要配置DMA。
> - 兼容SMBus（System Management Bus）协议。SMBus是基于I2C总线改进而来的，主要用于电源管理系统中。本节用不到。
> - STM32F103C8T6 硬件I2C资源：**I2C1、I2C2**。

<div align=center>
表10-1 I2C引脚复用表
</div>
<div align=center>
<style type="text/css">
.tg  {border-collapse:collapse;border-spacing:0;}
.tg td{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  overflow:hidden;padding:10px 5px;word-break:normal;}
.tg th{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  font-weight:normal;overflow:hidden;padding:10px 5px;word-break:normal;}
.tg .tg-wa1i{font-weight:bold;text-align:center;vertical-align:middle}
.tg .tg-fbj2{border-color:inherit;color:#000000;font-family:inherit;font-weight:bold;text-align:center;vertical-align:middle}
.tg .tg-nrix{text-align:center;vertical-align:middle}
</style>
<table class="tg">
<thead>
  <tr>
    <th class="tg-fbj2">引脚</th>
    <th class="tg-wa1i">I2C1</th>
    <th class="tg-wa1i">I2C2</th>
  </tr>
</thead>
<tbody>
  <tr>
    <td class="tg-nrix">SCL</td>
    <td class="tg-nrix">PB6/PB8</td>
    <td class="tg-nrix">PB10</td>
  </tr>
  <tr>
    <td class="tg-nrix">SDA</td>
    <td class="tg-nrix">PB7/PB9</td>
    <td class="tg-nrix">PB11</td>
  </tr>
  <tr>
    <td class="tg-nrix">SMBAI</td>
    <td class="tg-nrix">PB5</td>
    <td class="tg-nrix">PB12</td>
  </tr>
</tbody>
</table>

注：斜杠后引脚定义表示引脚重映射
</div>

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/10-08I2C%E5%8A%9F%E8%83%BD%E6%A1%86%E5%9B%BE.png" width="60%">
</div><div align=center>
图10-8 I2C功能框图
</div>

> SDA部分：
> - 发送数据过程：首先将数据写入“数据寄存器DR”，当没有移位时，DR中的数据就被转运到“数据移位寄存器”中，并同时置状态寄存器的TXE位为1（发送寄存器空）。此时就可以继续向DR中写入数据。
> - 接收数据流程：将数据一位一位的写入“数据移位寄存器”，当一个字节的数据收齐后，将会将数据整体从“数据移位寄存器”转运到“数据寄存器DR”，并同时置标志位RXNE（接收寄存器非空）。此时就可以将数据从DR中读出。
>
> - 比较器和地址寄存器（用不到）：从机模式使用。由于stm32的I2C是基于可变多主机模型设计的，不进行通信时默认为“从机”，于是“自身地址寄存器”就用于存放从机地址，可以由用户指定。“双地址寄存器”存储的也是从机地址，于是stm32就可以同时响应两个从机地址。
> - 数据校验模块（用不到）：当发送一个多字节的数据帧时，“帧错误校验计算”可以硬件自动执行CRC校验计算，得到一个字节的校验位附加在数据帧的最后。同样的，当接收的校验字节和CRC计算结果不匹配时，就会置校验错误标志位。
> 
> 评价：整个数据收发流程类似于“串口通信”，只不过串口是全双工通信，收发电路独立；I2C是半双工通信，收发共用一个电路。
>
> SCL部分：
> - 时钟控制：控制SCL线，具体细节无需了解。
> - 时钟控制寄存器CCR：控制“时钟控制”电路执行相应的功能。
> - 控制逻辑电路：写入“控制寄存器”，可以对整个电路进行控制；读取“状态寄存器”，可以得知整个电路的工作状态。
> - 中断：内部某些事情比较紧急时，可以申请中断。
> - DMA请求与响应：在进行很多字节的收发时，可以配合DMA来提高效率。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/10-09I2C%E5%9F%BA%E6%9C%AC%E7%BB%93%E6%9E%84%E5%9B%BE.png" width="70%">
</div><div align=center>
图10-9 I2C基本结构图
</div>

> 初始化I2C外设时注意四部分：
> 1. RCC时钟：现实需要初始化GPIO、I2C两个外设的时钟。
> 2. GPIO外设：都要配置成复用开漏输出模式。“复用”是交给片上外设来控制，“开漏输出”是I2C协议要求的端口配置。
> 3. I2C外设：老套路了，先定义结构体，再调用一个```I2C_Init```即可。
> 4. 开关控制：使能I2C外设。
>
> 注：上图简化成“一主多从模型”，所以SCL只有时钟输出。“多主机模型”时，SCL也会有输入。

I2C外设初始化完成以后，还要考虑引脚的时序如何变化，才能使得硬件I2C真正的能收发数据。**下面介绍硬件I2C的操作流程，编写I2C的读写时序需要参考下面由st公司给出的时序图**。
与软件I2C中CPU可以实时控制引脚变化不同，硬件I2C使用片上外设I2C来控制引脚变化，CPU不具有引脚的直接控制权，所以只能靠读取标志位来判断当前时序进行到哪一步了。参考手册中给出了“从机发送”、“从机接收”、“主机发送”、“主机接收”。由于本节是“一主多从”模式，所以只介绍“主机发送”、“主机接收”：
<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/10-10I2C%E4%B8%BB%E6%9C%BA%E5%8F%91%E9%80%81.png" width="70%">
</div><div align=center>
图10-10 “主机发送”时序
</div>

> 首先需要说明两点：
> 1. “EVx”事件：刚才提到CPU靠读取标志位来获取当前的时序状态，但有的状态会产生多个标志位，所以EVx事件就是组合了多个标志位的一个大标志位。
> 2. 关于应答位：I2C库函数发送数据自带接收应答、接收数据自带发送应答，所以用户想要发送应答就需要在发送数据时同时配置是否发送应答（有专门的库函数控制应答使能），而想要接收应答就直接读取相应的EVx事件（有专门的库函数）即可。
> 
> - 起始位、终止位：有相应的库函数可以直接调用。
> - EV5：起始位发送完毕，可以写入从机地址。
> - 地址：检测到EV5后，就将数据放到DR上（SB自动清零），发送完成后硬件自动检测应答，无应答则置应答失败的标志位，此时可以用中断来提醒。
> - EV6：从机地址发送完毕，可以写入新的数据到DR寄存器中。
> - EV8_1：只是表明了存在这么一个过渡状态，实际上用不到这个事件。
> - EV8：连续发送数据时，一旦检测到EV8事件，就可以再次写入数据。
> - EV8_2：发送最后一个字节数据之后，就等这个事件发生，就可以产生停止位了。


<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/10-11I2C%E4%B8%BB%E6%9C%BA%E6%8E%A5%E6%94%B6%E6%97%B6%E5%BA%8F.png" width="70%">
</div><div align=center>
图10-11 “主机接收”时序
</div>

> 注意没有给出“指定地址读”的时序，需要用户自行组合，也就是先用“主机发送”时序来一段“哑写”，然后再接着进行上图所示的“主机接收”时序。
>
> - “哑写”时序：按照“主机发送”时序，发送完寄存器地址后，直接等待EV8_2事件发生，然后直接产生“起始位”，接上下面的时序。
> - 起始位、终止位：有相应的库函数可以直接调用。
> - EV5：起始位发送完毕，可以写入从机地址。
> - EV6：注意检测到这个事件后，要配置是否发送应答，因为这决定了是否是连续接收数据。若不发送应答，还要直接产生停止条件（硬件I2C不会立即产生）；若发送应答，则需等待EV7。
> - EV7事件：表示当前可以读取数据。若上述没有发送应答，那么到这里时序结束。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/10-12%E8%BD%AF%E4%BB%B6%E7%A1%AC%E4%BB%B6I2C%E5%AF%B9%E6%AF%94.png" width="90%">
</div><div align=center>
图10-12 软件/硬件I2C波形对比
</div>

> 上图给出了使用 软件I2C/硬件I2C 的波形的不同：
> 1. 引脚电平变化：两者相同，对应的数据也相同。
> 2. 时钟线的规整程度：硬件I2C更加规整，每个时钟的周期、占空比都非常一致；软件I2C由于是在操作引脚之后，才添加了软件延迟（有可能多，有可能少），所以软件时序的时钟周期、占空比可能不规整（但是同步时序，不规整也无所谓）。
> 3. SDA数据变化时机：协议规定SCL低电平写、SCL高电平读，但是一般要求保证尽早的原则，所以理想状态下，可以直接认为是SCL下降沿写、SCL上升沿读。软件I2C在下降沿后，因为操作端口之后有一些延时，所以等了一会才进行写入操作；而硬件I2C中，数据写入都是紧贴下降沿、数据读出都是紧贴上升沿。这一点，在图中红色箭头处体现的非常明显。
> 4. 关于SCL占空比：ST公司规定时钟速率<=100kHz为标准通信，100kHz<=通信速率<=400kHz为高速通信。标准通信时，硬件I2C占空比固定为1:1；高速通信时，占空比可以选择SCL 低电平:高电平 = 16:9/2:1。之所以SCL低电平时间更长，是因为SCL、SDA引脚为都开漏输出，上拉为弱上拉，上升沿平缓时间长；下拉为强下拉，下降沿陡峭时间短。为了保证SCL低电平时，SDA有足够的时间改变数据，所以设置SCL低电平占空比更大。具体的高速波形，可以参考UP主讲解：“[P35[10-5]硬件I2C读写MPU6050](https://www.bilibili.com/video/BV1th411z7sn/?p=35)，20:10~25:16”。
> 



## 10.5 实验：硬件I2C读写MPU6050
需求：与软件I2C读写MOU6050相同，读出MPU6050数据并显示在OLED上。
> 注：根据引脚定义表，选择I2C2的PB10、PB11进行通信，于是引脚选择也就和软件I2C相同。

**接线图**与上一个实验——“软件I2C读写MOU6050”一致，但最底层的代码是直接调用的I2C库函数，所以**代码调用**稍有不同：

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/10-13%E4%BB%A3%E7%A0%81%E8%B0%83%E7%94%A8-%E7%A1%AC%E4%BB%B6I2C.png" width="25%">
</div><div align=center>
图10-13 软件读写MPU6050-代码调用（非库函数）
</div>

下面是**代码展示**：```main.c```、```MPU6050.h```与源文件相同，```MPU6050.c```中只是将涉及到I2C引脚变化的操作都替换成库函数了，具体的变化过程参考图10-10“主机发送”时序、图10-11“主机接收”时序。
**- MPU6050.c**
```c
#include "stm32f10x.h"                  // Device header
#include "MPU6050.h"

//定义从机地址、寄存器地址
#define MPU6050_ADDRESS           0xD0
#define MPU6050_REG_SMPLRT_DIV    0x19
#define MPU6050_REG_CONFIG        0x1A
#define MPU6050_REG_GYRO_CONFIG   0x1B
#define MPU6050_REG_ACCEL_CONFIG  0x1C

#define MPU6050_REG_ACCEL_XOUT_H  0x3B
#define MPU6050_REG_ACCEL_XOUT_L  0x3C
#define MPU6050_REG_ACCEL_YOUT_H  0x3D
#define MPU6050_REG_ACCEL_YOUT_L  0x3E
#define MPU6050_REG_ACCEL_ZOUT_H  0x3F
#define MPU6050_REG_ACCEL_ZOUT_L  0x40
#define MPU6050_REG_TEMP_OUT_H    0x41
#define MPU6050_REG_TEMP_OUT_L    0x42
#define MPU6050_REG_GYRO_XOUT_H   0x43
#define MPU6050_REG_GYRO_XOUT_L   0x44
#define MPU6050_REG_GYRO_YOUT_H   0x45
#define MPU6050_REG_GYRO_YOUT_L   0x46
#define MPU6050_REG_GYRO_ZOUT_H   0x47
#define MPU6050_REG_GYRO_ZOUT_L   0x48

#define MPU6050_REG_PWR_MGMT_1    0x6B
#define MPU6050_REG_PWR_MGMT_2    0x6C
#define MPU6050_REG_WHO_AM_I      0x75

//将I2C_CheckEvent封装成具有超时退出机制的WaitEvent函数
void MPU6050_WaitEvent(I2C_TypeDef* I2Cx, uint32_t I2C_EVENT){
    uint32_t Timeout;
    Timeout = 100000;
    while(I2C_CheckEvent(I2Cx, I2C_EVENT)==ERROR){
        Timeout--;
        if(Timeout==0){
            //超时退出操作，如打印错误日志、进行系统复位、紧急停机等
            break;
        }
    }
}

//MPU6050指定地址写寄存器
void MPU6050_WriteReg(uint8_t RegAddr, uint8_t wData){
    //1.发送起始位
    I2C_GenerateSTART(I2C2, ENABLE);
    MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT);//等待EV5事件发生
    //2.发送从机地址
    I2C_Send7bitAddress(I2C2, MPU6050_ADDRESS, I2C_Direction_Transmitter);
    //不需要接收应答，库函数发送数据自带接收应答、接收数据自带发送应答
    MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);//等待EV6事件发生
    //3.发送寄存器地址
    I2C_SendData(I2C2, RegAddr);//这一步无需等到EV8_1事件发生
    MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTING);//等待EV8事件发生
    //4.发送数据
    I2C_SendData(I2C2, wData);
    MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED);//等待EV8_2事件发生
    //5.发送停止位
    I2C_GenerateSTOP(I2C2, ENABLE);
    
}

//MPU6050指定地址读寄存器
uint8_t MPU6050_ReadReg(uint8_t RegAddr){
    uint8_t rData=0x00;
    
    //一、先进行一段“哑写”，指定要读取的地址
    //1.发送起始位
    I2C_GenerateSTART(I2C2, ENABLE);
    MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT);//等待EV5事件发生
    //2.发送从机地址
    I2C_Send7bitAddress(I2C2, MPU6050_ADDRESS, I2C_Direction_Transmitter);
    MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);//等待EV6事件发生
    //3.发送寄存器地址
    I2C_SendData(I2C2, RegAddr);//这一步无需等到EV8_1事件发生
    MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED);//等待EV8_2事件发生    
    
    //二、再按照主机接收（当前地址读）的流程来
    //4.再次发送起始位
    I2C_GenerateSTART(I2C2, ENABLE);
    MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT);//等待EV5事件发生
    //5.发送从机地址
    I2C_Send7bitAddress(I2C2, MPU6050_ADDRESS, I2C_Direction_Receiver);
    MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED);//等待EV6事件发生
    //6.按照手册，需要首先设置ACK=0、并且产生停止条件
    I2C_AcknowledgeConfig(I2C2, DISABLE);    
    I2C_GenerateSTOP(I2C2, ENABLE);
    MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_BYTE_RECEIVED);//等待EV7事件发生
    //7.接收数据
    rData = I2C_ReceiveData(I2C2);
    I2C_AcknowledgeConfig(I2C2, ENABLE);//将ACK改回默认的使能配置，方便指定地址收多个字节
    
    return rData;
}

//MPU6050初始化
void MPU6050_Init(void){
    //配置I2C外设
    // 1.开启RCC时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    // 2.GPIO口初始化位复用开漏输出
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;//复用开漏输出
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    // 3.使用结构体对整个I2C进行配置
    I2C_InitTypeDef I2C_InitStructure;
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;//默认给应答，后续可以单独修改
    I2C_InitStructure.I2C_ClockSpeed = 200000;//200kHz
    //时钟占空比只有在频率大于100kHz（快速模式）才起作用，标准模式固定为1:1
    //因为弱上拉导致上升沿时间比时钟下降沿更长，为了在快速模式下也准确传输数据，就需要给数据变化的区间（SCL低电平）更长的时间
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;//选择SCL低电平:高电平=2:1
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;//stm32作为从机，有几位地址
    I2C_InitStructure.I2C_OwnAddress1 = 0x00;//stm32作为从机，从机地址是多少
    I2C_Init(I2C2, &I2C_InitStructure);
    // 4.使能I2C
    I2C_Cmd(I2C2, ENABLE);
    
    //不复位、解除睡眠、不开启循环模式、温度传感器失能、选择陀螺仪x轴时钟
    MPU6050_WriteReg(MPU6050_REG_PWR_MGMT_1,0x01);
    //没有开启循环模式
    MPU6050_WriteReg(MPU6050_REG_PWR_MGMT_2,0x00);
    //采样率10分频
    MPU6050_WriteReg(MPU6050_REG_SMPLRT_DIV,0x09);
    //不使用外部同步、DLPF设置等级6
    MPU6050_WriteReg(MPU6050_REG_CONFIG,0x06);
    //陀螺仪：自测失能、满量程±500°/s-000_01_000
    MPU6050_WriteReg(MPU6050_REG_GYRO_CONFIG,0x08);
    //加速度计：自测失能、满量程±2g、失能运动检测-000_00_000
    MPU6050_WriteReg(MPU6050_REG_ACCEL_CONFIG,0x00);
}

//获取MPU6050的ID号
uint8_t MPU6050_GetID(void){
    return MPU6050_ReadReg(MPU6050_REG_WHO_AM_I);
}

//获取MPU6050的传感器数据
void MPU6050_GetData(MPU6050_DataStruct* MPU6050_Data){
    uint16_t sensor_byte_L, sensor_byte_H;
    //获取加速度计数据
    sensor_byte_H = MPU6050_ReadReg(MPU6050_REG_ACCEL_XOUT_H);
    sensor_byte_L = MPU6050_ReadReg(MPU6050_REG_ACCEL_XOUT_L);
    MPU6050_Data->Acce_X = (int16_t)((sensor_byte_H<<8) | sensor_byte_L);
    sensor_byte_H = MPU6050_ReadReg(MPU6050_REG_ACCEL_YOUT_H);
    sensor_byte_L = MPU6050_ReadReg(MPU6050_REG_ACCEL_YOUT_L);
    MPU6050_Data->Acce_Y = (int16_t)((sensor_byte_H<<8) | sensor_byte_L);
    sensor_byte_H = MPU6050_ReadReg(MPU6050_REG_ACCEL_ZOUT_H);
    sensor_byte_L = MPU6050_ReadReg(MPU6050_REG_ACCEL_ZOUT_L);
    MPU6050_Data->Acce_Z = (int16_t)((sensor_byte_H<<8) | sensor_byte_L);
    //获取陀螺仪数据
    sensor_byte_H = MPU6050_ReadReg(MPU6050_REG_GYRO_XOUT_H);
    sensor_byte_L = MPU6050_ReadReg(MPU6050_REG_GYRO_XOUT_L);
    MPU6050_Data->Gyro_X = (int16_t)((sensor_byte_H<<8) | sensor_byte_L);
    sensor_byte_H = MPU6050_ReadReg(MPU6050_REG_GYRO_YOUT_H);
    sensor_byte_L = MPU6050_ReadReg(MPU6050_REG_GYRO_YOUT_L);
    MPU6050_Data->Gyro_Y = (int16_t)((sensor_byte_H<<8) | sensor_byte_L);
    sensor_byte_H = MPU6050_ReadReg(MPU6050_REG_GYRO_ZOUT_H);
    sensor_byte_L = MPU6050_ReadReg(MPU6050_REG_GYRO_ZOUT_L);
    MPU6050_Data->Gyro_Z = (int16_t)((sensor_byte_H<<8) | sensor_byte_L);
    //获取温度传感器数据
    sensor_byte_H = MPU6050_ReadReg(MPU6050_REG_TEMP_OUT_H);
    sensor_byte_L = MPU6050_ReadReg(MPU6050_REG_TEMP_OUT_L);
    MPU6050_Data->Temp = (int16_t)((sensor_byte_H<<8) | sensor_byte_L);
}

```

编程感想：
> 1. 关于应答位。I2C库函数发送数据自带接收应答、接收数据自带发送应答，所以用户想要发送应答就需要提前配置，想要接收应答也就是读取相应的标志位。
> 2. 关于起始条件```I2C_GenerateSTART```。这个起始条件会等待前一个字节发送完毕后才产生，并不会直接截断传送。所以发送数据后，即使等待EV8事件发生，就发送起始条件，也没问题。但是保险起见，还是等待EV8_2事件，再发送起始条件。
> 3. 超时退出机制。程序中有大量的等待事件的while循环，万一有某个事件没有产生，程序就会卡死，非常危险，所以要设置超时退出机制。也不用很高大上，就是一个简单的计数退出就行。
> 4. 奇怪的bug。前天晚上写好了代码，结果下载以后一看，陀螺仪、加速度计的XYZ分别都是相同的值，但是不知道bug在哪，再加上还有别的事的就先走了，打算今早再看，结果代码没变，一下载显示正常！啊这...一观测就消失，量子bug是吧:joy:。





