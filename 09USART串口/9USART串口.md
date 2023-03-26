# 9 USART串口
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



## 9.1 串口通信协议
从本节开始，将逐一学习STM32的通信接口。首先介绍以下stm32都集成了什么通信外设。

为了控制或读取外挂模块，stm32需要与外挂模块进行通信，来扩展硬件系统。而这个“通信”的过程就需要遵守相应的“通信协议”，也就是通信双方需要按照协议规则进行数据收发。不同外挂模块的会采用不同的通信协议，如下表：

<div align=center>
表9-1 stm32片上集成的通信模块外设
</div><div align=center>
<style type="text/css">
.tg  {border-collapse:collapse;border-spacing:0;}
.tg td{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  overflow:hidden;padding:10px 5px;word-break:normal;}
.tg th{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  font-weight:normal;overflow:hidden;padding:10px 5px;word-break:normal;}
.tg .tg-9fp1{color:#000000;font-family:inherit;font-weight:bold;text-align:center;vertical-align:middle}
.tg .tg-0so2{font-family:inherit;text-align:center;vertical-align:middle}
.tg .tg-c4ze{color:#000000;font-weight:bold;text-align:center;vertical-align:middle}
.tg .tg-nrix{text-align:center;vertical-align:middle}
</style>
<table class="tg">
<thead>
  <tr>
    <th class="tg-9fp1">名称</th>
    <th class="tg-9fp1">引脚</th>
    <th class="tg-9fp1">双工</th>
    <th class="tg-9fp1">时钟</th>
    <th class="tg-c4ze">电平</th>
    <th class="tg-c4ze">设备</th>
  </tr>
</thead>
<tbody>
  <tr>
    <td class="tg-0so2"><span style="color:#000">USART</span></td>
    <td class="tg-0so2"><span style="color:#000">TX/TXD、RX</span>/RXD</td>
    <td class="tg-0so2"><span style="color:#000">全双工</span></td>
    <td class="tg-0so2"><span style="color:#000">异步</span></td>
    <td class="tg-nrix"><span style="color:#000">单端</span></td>
    <td class="tg-nrix"><span style="color:#000">点对点</span></td>
  </tr>
  <tr>
    <td class="tg-0so2"><span style="color:#000">I2C</span></td>
    <td class="tg-0so2"><span style="color:#000">SCL、SDA</span></td>
    <td class="tg-0so2"><span style="color:#000">半双工</span></td>
    <td class="tg-0so2"><span style="color:#000">同步</span></td>
    <td class="tg-nrix"><span style="color:#000">单端</span></td>
    <td class="tg-nrix"><span style="color:#000">多设备</span></td>
  </tr>
  <tr>
    <td class="tg-0so2"><span style="color:#000">SPI</span></td>
    <td class="tg-0so2"><span style="color:#000">SCLK、MOSI、MISO、CS</span></td>
    <td class="tg-0so2"><span style="color:#000">全双工</span></td>
    <td class="tg-0so2"><span style="color:#000">同步</span></td>
    <td class="tg-nrix"><span style="color:#000">单端</span></td>
    <td class="tg-nrix"><span style="color:#000">多设备</span></td>
  </tr>
  <tr>
    <td class="tg-0so2"><span style="color:#000">CAN</span></td>
    <td class="tg-0so2"><span style="color:#000">CAN_H、CAN_L</span></td>
    <td class="tg-0so2"><span style="color:#000">半双工</span></td>
    <td class="tg-0so2"><span style="color:#000">异步</span></td>
    <td class="tg-nrix"><span style="color:#000">差分</span></td>
    <td class="tg-nrix"><span style="color:#000">多设备</span></td>
  </tr>
  <tr>
    <td class="tg-0so2"><span style="color:#000">USB</span></td>
    <td class="tg-0so2"><span style="color:#000">DP/D+、DM/D-</span></td>
    <td class="tg-0so2"><span style="color:#000">半双工</span></td>
    <td class="tg-0so2"><span style="color:#000">异步</span></td>
    <td class="tg-nrix"><span style="color:#000">差分</span></td>
    <td class="tg-nrix"><span style="color:#000">点对点</span></td>
  </tr>
</tbody>
</table>
</div>

> 下面介绍一下引脚的全称：
> - USART：TX(Transmit Exchange)数据发送脚、RX(Receive Exchange)数据接收脚。
> - IIC：SCL(Serial Clock)时钟线、SDA(Serial Data)数据线。
> - SPI：MOSI(Master Output Slave Input)主机输出数据脚、MISO(Master Input Slave Output)主机输入数据脚、CS(Chip Select)片选
> - USB：DP(Data Postive)差分线正、DM(Data Minus)差分线负
>
> 注：上述协议中，单端电平都需要共地。
> 注：使用差分信号可以抑制共模噪声，可以极大的提高信号的抗干扰特性，所以一般差分信号的传输速度和传输距离都非常高。


本节将介绍串口。串口是一种应用十分广泛的通讯接口，串口成本低、容易使用、通信线路简单，可实现两个设备的互相通信。单片机的串口可以使单片机与单片机、单片机与电脑、单片机与各式各样的模块互相通信，极大地扩展了单片机的应用范围，增强了单片机系统的硬件实力。

一般单片机中都会有串口的通信外设。在单片机领域中，相比于IIC、SPI等协议，串口是一种非常简单的通信接口，只支持**点对点的通信**。**单片机与电脑通信**，是串口的一大优势，可以外接电脑屏幕，非常适合调试程序、打印信息……IIC或SPI协议一般都是芯片之间的通信，有片选引脚所以支持总线通信，而不会直接外接在电脑上。


<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/9-01%E4%B8%B2%E5%8F%A3%E6%A8%A1%E5%9D%97%E5%AE%9E%E4%BE%8B.png" width="60%">
</div><div align=center>
图9-1 串口模块实例
</div>

> 1. USB转串口模块：使用CH340芯片，可以将串口协议转换成USB协议。使用此模块可以实现单片机与电脑的通信。
> 2. 陀螺仪模块：左侧是串口引脚，右侧是IIC引脚。可以用于测量角速度、角速度等姿态参数。
> 3. 蓝牙串口模块：上面的芯片可以和手机互联，下面四个引脚是串口引脚。此芯片可以实现手机遥控单片机的功能。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/9-02%E4%B8%B2%E5%8F%A3%E5%BC%95%E8%84%9A%E6%8E%A5%E7%BA%BF%E5%9B%BE.png" width="40%">
</div><div align=center>
图9-2 串口引脚接线图
</div>

> 下面介绍一些串口引脚的注意事项：
> - TX与RX：简单双向串口通信有两根通信线（发送端TX和接收端RX），要**交叉连接**。不过，若仅单向的数据传输，可以只接一根通信线。
> - GND：一定要共地。由于TX和RX的高低电平都是相对于GND来说的，所以GND严格来说也算是通信线。
> - VCC：相同的电平才能通信，如果两设备都有单独的供电，VCC就可以不接在一起。但如果某个模块没有供电，就需要连接VCC，注意供电电压要按照模块要求来，必要时需要添加电压转换电路。

电平标准是数据1和数据0的表达方式，是传输线缆中人为规定的电压与数据的对应关系，**串口常用的电平标准**有如下三种：
> 1. TTL电平【单片机】：+3.3V或+5V表示1，0V表示0。一般低压小型设备，使用的都是TTL电平。传输范围几十米。
> 2. RS232电平：-3\~-15V表示1，+3\~+15V表示0。一般在大型机器上使用，由于环境比较恶劣，静电干扰比较大，所以通信电压都很大，并且允许波动的范围也很大。传输范围几十米。
> 3. RS485电平：两线压差+2\~+6V表示1，-2\~-6V表示0（差分信号）。抗干扰能力极强，通信距离可达上千米。
>
> 注：不同电平标准之间的转换只需要加电压转换芯片即可，并不需要修改相应的软件代码。



上面介绍了串口协议的硬件部分（如何表示1/0），下面来介绍串口协议的软件部分（如何使用1/0组成字节数据）。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/9-03%E4%B8%B2%E5%8F%A3%E7%9A%84%E5%8D%95%E5%AD%97%E8%8A%82%E5%8F%91%E9%80%81%E6%A0%BC%E5%BC%8F.png" width="90%">
</div><div align=center>
图9-3 串口的单字节发送格式
</div>

> 下面介绍串口的参数：
> - 波特率：串口通信的速率（bit/s），也就是通信双方所约定的通信速率（异步通信）。
> - 空闲状态：固定为高电平。
> - 起始位：固定为低电平，标志一个数据帧的开始。
> - 数据位：**低位先行**，数据帧的有效载荷，1为高电平，0为低电平。
> - 校验位（选填）：用于数据验证，根据数据位计算得来。
> - 停止位：固定为高电平，用于表示数据帧的间隔，同时也可以使得通信线回归到空闲状态。可以配置停止位是1位/2位。
> 

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/9-04%E4%B8%B2%E5%8F%A3%E6%97%B6%E5%BA%8F.png" width="80%">
</div><div align=center>
图9-4 串口时序
</div>

> 上图给出了几个例子，来展示串口通信的时序图：
> - 右侧最后两个图展示了不同长度停止位的现象。
> 
> 注：在stm32中，根据发送数据自动转换发送波形、或根据波形自动读取数据，都是由USART外设自动完成的，无需软件控制每一位的发送或读取。

## 9.2 stm32的片上外设-USART

**USART（Universal Synchronous/Asynchronous Receiver/Transmitter）通用同步/异步收发器** 是STM32内部集成的硬件外设，可根据数据寄存器的一个字节数据 <u>自动生成数据帧时序，从TX引脚发送出去</u>，也可 <u>自动接收RX引脚的数据帧时序</u>，拼接为一个字节数据，存放在数据寄存器里。USART中的“S”表示同步，只支持时钟输出，不支持时钟输入，是为了兼容别的协议或特殊用途而设计的，并不支持两个USART之间进行同步通信，所以这个功能几乎不会用到，**一般更常使用的是UART同步异步收发器**。下面是一些参数：
> - 自带波特率发生器，最高达4.5Mbits/s，常用9600/115200。
> - 可配置数据位长度（8/9）、停止位长度（0.5/1/1.5/2）。
> - 可选校验位：无校验（常用）/奇校验/偶校验。
> - 支持同步模式（一般不用）、硬件流控制（指示从设备准备好接收的信号，一般不用）、**DMA**、智能卡、IrDA（手机红外通信，但并不是红外遥控，目前很少见）、LIN（局域网的通信协议）。
> - STM32F103C8T6 USART资源：**USART1(APB2)、USART2(APB1)、USART3(APB1)**。
> 

<div align=center>
表9-2 串口引脚定义和引脚复用关系
</div><div align=center>
<style type="text/css">
.tg  {border-collapse:collapse;border-spacing:0;}
.tg td{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  overflow:hidden;padding:10px 5px;word-break:normal;}
.tg th{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  font-weight:normal;overflow:hidden;padding:10px 5px;word-break:normal;}
.tg .tg-cly1{text-align:left;vertical-align:middle}
.tg .tg-9fp1{color:#000000;font-family:inherit;font-weight:bold;text-align:center;vertical-align:middle}
.tg .tg-wa1i{font-weight:bold;text-align:center;vertical-align:middle}
.tg .tg-nrix{text-align:center;vertical-align:middle}
</style>
<table class="tg">
<thead>
  <tr>
    <th class="tg-9fp1">引脚</th>
    <th class="tg-wa1i">USART1</th>
    <th class="tg-wa1i">USART2</th>
    <th class="tg-wa1i">USART3</th>
  </tr>
</thead>
<tbody>
  <tr>
    <td class="tg-nrix">TX</td>
    <td class="tg-nrix"><span style="font-weight:bold">PA9</span>/PB6</td>
    <td class="tg-nrix">PA2</td>
    <td class="tg-nrix">PB10</td>
  </tr>
  <tr>
    <td class="tg-nrix">RX</td>
    <td class="tg-nrix"><span style="font-weight:bold">PA10</span>/PB7</td>
    <td class="tg-nrix">PA3</td>
    <td class="tg-nrix">PB11</td>
  </tr>
  <tr>
    <td class="tg-nrix">CK</td>
    <td class="tg-nrix">PA8</td>
    <td class="tg-nrix">PA4</td>
    <td class="tg-nrix">PB12</td>
  </tr>
  <tr>
    <td class="tg-nrix">CTS</td>
    <td class="tg-nrix">PA11</td>
    <td class="tg-nrix">PA0</td>
    <td class="tg-nrix">PB13</td>
  </tr>
  <tr>
    <td class="tg-nrix">RTS</td>
    <td class="tg-nrix">PA12</td>
    <td class="tg-nrix">PA1</td>
    <td class="tg-nrix">PB14</td>
  </tr>
  <tr>
    <td class="tg-cly1" colspan="4">注：斜杠后面的引脚表示重定义</td>
  </tr>
</tbody>
</table>
</div>

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/9-05USART%E6%A1%86%E5%9B%BE.png" width="70%">
</div><div align=center>
图9-5 USART框图
</div>

> - 发送数据的过程：某时刻给“TDR”写入数据0x55，此时硬件检测到写入数据，就会检查当前“发送移位寄存器”是否 有数据正在进行移位，若正在移位，就会等待移位完成；若没有移位，就会将TDR中的数据立刻移动到“发送移位寄存器”中，准备发送。然后，“发送移位寄存器”就会在下面的“发送器控制”的驱动下，向右一位一位的移位（低位先行），将数据输出到TX发送引脚。移位完成后，新的数据会再次自动的从TDR转移到“发送移位寄存器”中来。<u>有了TDR和“发送移位寄存器”的双重缓存，可以保证连续发送数据时，数据帧之间不会有空闲。</u>
> - 写入数据的时机：当数据从TDR移动到“发送移位寄存器”时，**标志位TXE**置位（TX Empty，发送寄存器空），此时检测到TXE置位就可以继续写入下一个数据，但注意此时上一个数据实际上还没发送出去。
> - 接收数据的过程：数据从RX引脚通向“接收移位寄存器”，在“接收器控制”的驱动下，一位一位的读取RX电平并放在最高位，整个过程不断右移（低位先行），最终就可以接收1个字节。当一个字节移位完成后，这一个字节的数据就会整体地一下子转移到“接收数据寄存器RDR”中。然后就准备继续读取下一帧数据。<u>同样，RDR和“接收移位寄存器”也组成了双重缓存结构，可以保证连续读取数据。</u>
> - 读取数据的时机：接收数据从“接收移位寄存器”转移到RDR的过程中，**标志位RXNE**置位（RX Not Empty，接收数据寄存器非空）。所以当检测到标志位RXNE置位时，就可以将数据读走。
>
> 下面来看看其他的硬件电路功能：
> - 发送器控制：控制“发送移位寄存器”工作。
> - 接收器控制：控制“接收移位寄存器”工作。
> - 硬件数据流控：也就是“硬件流控制”，简称“流控”。如果主设备连续发送，导致从设备内部无法及时处理接收数据，就会出现数据丢弃或覆盖的现象（注意不是UART没接收到，而是从设备没有及时将数据从RDR取走），此时“流控”就可以帮助从设备向主设备发信号，指明自己还没有准备好接收数据，主设备也就不会发送数据了。本教程不涉及。
> > - nRTS（Request To Send）：输出脚，请求发送。也就是告诉主设备，当前是否已经准备好接收。前方“n”表示低电平有效。
> > - nCTS（Clear To Send）：输入脚，清除发送。用于接收从设备的nRTS信号。前方“n”表示低电平有效。
> - SCLK：用于产生“同步功能”的时钟信号，配合“发送移位寄存器”输出，用于给从设备提供时钟。注意没有同步时钟输入，所以两个USART之间不能同步通信。
> - 唤醒单元：实现串口挂载多设备。前面提到串口一般是点对点通信，但是这个模块通过给串口分配地址“USART地址”，就可以决定是否唤醒USART工作，进而实现了总线通信。
> - **状态寄存器SR**：存储着串口通信的各种标志位，其中比较重要的有发送寄存器空TXE、接收数据寄存器非空RXNE。
> - USART中断控制：中断输出控制，配置中断是否可以通向NVIC。其中断申请位就是状态寄存器SR中的各种标志位。
> - 波特率发生器部分：其实就是分频器，APB时钟进行分频，得到发送和接收移位的时钟。
> > - f~PCLKx(x=1,2)~：时钟输入。由于UASRT1挂载在APB2上，所以 时钟输入f~PCLKx(x=1,2)~ 就是PCLK2的时钟，一般为72MHz。而UASRT2、USART3都挂载在APB1上，所以 时钟输入f~PCLKx(x=1,2)~ 就是PCLK1的时钟，一般为36MHz。
> > - /UASRTDIV：时钟分频系数。内部结构也就是虚线框中的“传统的波特率发生器”。
> > - /16：再进行16分频，得到最终的“发送器时钟”、“接收器时钟”。
>
> 
> 注：发送时添加开始位、停止位；接收时去除开始位、停止位，这些工作由内部硬件电路自动完成。
> 注：更多关于控制寄存器CR、状态寄存器SR的描述可以查阅参考手册“25.6 USART寄存器描述”。


<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/9-06USART%E5%9F%BA%E6%9C%AC%E7%BB%93%E6%9E%84.png" width="75%">
</div><div align=center>
图9-6 USART基本结构
</div>

> 上图给出USART最主要、最基本的结构：
> - 波特率发生器：用于产生约定的通信速率。时钟来源是PCLK2/PCLK1，经过波特率发生器分频后，产生的时钟通向发送控制器、接收控制器。
> - 发送控制器、接收控制器：用于控制发送移位、接收移位。
> - GPIO：发送端配置成复用推挽输出、接收端配置成上拉输入。
> - 标志位：TXE置位时写入数据、RXNE置位时接收数据。
> - 开关控制：用于开启整个USART外设。

下面来看几个细节的问题：
**细节1：数据帧**

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/9-07%E5%AD%97%E9%95%BF%E8%AE%BE%E7%BD%AE.png" width="60%">
</div><div align=center>
图9-7 字长的配置
</div>

> 上图给出了8位字长（无校验位）、9位字长（有校验位）的波形：
> - 时钟上升沿：可以看到每个数据中间都有一个时钟上升沿，所以接收端采样时刻就是时钟上升沿。时钟的极性、相位等都可以通过配置寄存器配置。
> - 空闲帧、断开帧：用于局域网协议。
> - 尽量选择**9位字长有校验**、**8位字长无校验**。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/9-08%E9%85%8D%E7%BD%AE%E5%81%9C%E6%AD%A2%E4%BD%8D.png" width="60%">
</div><div align=center>
图9-8 停止位的配置
</div>

> stm32串口外设的停止位长度可以配置成0.5/1/1.5/1位，共四种选择，区别就是停止位的时长不一样。
> - 一般就选择**停止位长度为1位**。


**细节2：USART输入数据采样规则**
串口的输出TX只需要保持相应时长的电平即可，电路简单；但是串口输入RX则需要判断电平持续时间，所以电路会更加复杂，所以下面来详细介绍stm32串口外设对于输入数据的采样。
<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/9-09%E5%AF%B9%E4%BA%8E%E8%B5%B7%E5%A7%8B%E4%BD%8D%E7%9A%84%E9%87%87%E6%A0%B7.png" width="60%">
</div><div align=center>
图9-9 对于起始位的采样
</div>

> 注意到**采样时钟是波特率的16倍**，即会对某一位采样16次。
> - 检测下降沿：若突然检测到下降沿，则开始进行检测。
> - 检测3、5、7位：在第3、5、7间隔采样，<u>**采样判断原则**为若三位全为0，则正常进入后续；若有2个0，则还是会接着检测，但**噪声标志位NE(Noise Error)置位**（告诉用户，我这儿接收的信号有噪声，你悠着点用:joy:）；若低于2个0，则认为之前检测到的下降沿为噪声，忽略已经捕获的数据，重新回到空闲状态开始捕捉下降沿。</u>
> - 检测8、9、10位：连续采样。采样判断原则与上述相同。



<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/9-10%E5%AF%B9%E4%BA%8E%E6%95%B0%E6%8D%AE%E4%BD%8D%E7%9A%84%E9%87%87%E6%A0%B7.png" width="60%">
</div><div align=center>
图9-10 对于数据位的采样
</div>

> 由于起始位采样已经对齐了数据时钟，所以数据采样就直接在8、9、10位采样。
> - **数据采样的判断原则**：三个数据中，0/1哪个数据多就判断为接收到哪个。但是如果三个数据有不一致的情况，噪声标志位NE(Noise Error)置位。

**细节3：计算分频系数DIV**
<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/9-11%E6%B3%A2%E7%89%B9%E6%AF%94%E7%8E%87%E5%AF%84%E5%AD%98%E5%99%A8%E7%A4%BA%E6%84%8F%E5%9B%BE.png" width="70%">
</div><div align=center>
图9-11 波特比率寄存器示意图
</div>

发送器和接收器的波特率由波特率寄存器BRR里的分频系数DIV确定：
$$
波特率 = \frac{f_{PCLK2/1}}{16 * DIV}
$$

例如：若输入时钟为$f_{PCLK2/1}= 72MHz$，希望配置波特率为9600，则分频系数为：$DIV=\frac{72M}{16*9600}=468.75$，转换成二进制为```111010100.11```，于是USART_BRR的值为```0001_1101_0100_1100```(高位补零、低位补零)。

比较方便的是，**上述过程都可以使用USART的外设库函数实现**，调用时只需输入波特率，库函数会自动计算出DIV，并按照格式配置好BRR寄存器。

> 注：十进制转二进制工具为[菜鸟工具](https://c.runoob.com/)的 “[在线进制转换器](https://c.runoob.com/front-end/58/)”。


**细节4：USB转串口模块**
<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/9-12%E5%8E%9F%E7%90%86%E5%9B%BE-USB%E8%BD%AC%E4%B8%B2%E5%8F%A3%E6%A8%A1%E5%9D%97.png" width="75%">
</div><div align=center>
图9-12 USB转串口模块-原理图
</div>

> 主要关注的是该模块的供电情况。
> - USB插座：直接插在电脑USB端口上，注意整个模块的供电来自于USB的VCC+5V。
> - CON6插针座：
> > - 引脚2、引脚3：用于连接到stm32上进行串口通信。
> > - 引脚5【CH340_VCC】：通过跳线帽可以选择 **接入+3.3V（stm32）** 或者+5V。CH340芯片的供电引脚，同时决定了TTL，所以也就是串口通信的TTL电平。神奇的是，即使不接跳线帽CH340也可以正常工作，TTL为3.3V，但是显然接上电路以后更加稳定。
> > - 通信和供电的选择：CON6插针座选择引脚4/6进行通信后，剩下的引脚可以用于给从设备供电，但是剩下的这个脚显然与TTL电平不匹配。此时需要注意 **优先保证供电电平的正确**，通信TTL电平不一致问题不大。当然，若从设备自己有电源，那么就不存在这个问题了。
> - TXD指示灯、RXD指示灯：若相应总线上有数据传输，那么指示灯就会闪烁。

**细节5：数据模式**
显然计算机在通信过程中只能传输二进制数据，那么如何发送文本呢？就需要制定一个规则，来约定字符和接收数据的映射关系，即字符编码表。不同的编码格式有不同的映射，具体可以在网上随便搜搜“字符编码格式”，如知乎文章“[字符常见的编码方式](https://zhuanlan.zhihu.com/p/402161647)”。
> HEX模式/十六进制模式/二进制模式：以原始数据的形式显示；
> 文本模式/字符模式：以原始数据编码后的形式显示。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/9-13ASCII%E7%A0%81%E8%A1%A8.png" width="90%">
</div><div align=center>
图9-13 ASCII码表
</div>










## 9.3 USART收发相关实验
### 9.3.1 实验1：串口发送
需求：在软件代码中定义要发送的信息，然后通过串口发送到电脑端，使用“串口助手”小工具查看。要求依次发送单字节数据、数组、字符串、数据的每一位。
注：串口助手可以切换“文本模式”/“HEX模式”。
注：数字和字符的对应关系可以参考ASCII码表。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/9-14%E6%8E%A5%E7%BA%BF%E5%9B%BE-%E4%B8%B2%E5%8F%A3%E5%8F%91%E9%80%81.png" width="70%">
</div><div align=center>
图9-14 串口发送-接线图
</div>

注：接线图也可以不接OLED显示屏。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/9-15%E4%BB%A3%E7%A0%81%E8%B0%83%E7%94%A8-%E4%B8%B2%E5%8F%A3%E5%8F%91%E9%80%81.png" width="25%">
</div><div align=center>
图9-15 串口发送-代码调用（非库函数）
</div>

下面是代码展示：
**- main.c**
```c
#include "stm32f10x.h"                  // Device header
#include "SerialPort.h"
    
int main(void){
    uint8_t send_byte = 0x42;
    uint8_t send_array[6] = {0x30,0x31,0x32,0x33,0x34,0x35};
    //串口初始化
    SerialPort_Init();
    //发送单个字节
    SerialPort_SendByte('A');//可以直接发送字符
    SerialPort_SendByte(send_byte);
    SerialPort_SendByte('\r');
    SerialPort_SendByte('\n');
    //发送数组
    SerialPort_SendArray(send_array,6);
    SerialPort_SendByte('\r');
    SerialPort_SendByte('\n');
    //发送字符串
    SerialPort_SendString("Hello World!\r\n");
    //发送数字的每一位
    SerialPort_SendNum(65535, 5);
    SerialPort_SendString("\r\n");
    while(1){
//        //循环发送数字
//        SerialPort_SendByte(send_byte);
//        OLED_ShowHexNum(1,9,send_byte,2);
//        send_byte++;
//        Delay_ms(1000);
    };
}

```

**- SerialPort.h**
```c
#ifndef __SERIALPORT_H
#define __SERIALPORT_H

void SerialPort_Init(void);
void SerialPort_SendByte(uint8_t send_byte);
void SerialPort_SendArray(uint8_t *send_array, uint16_t size_array);
void SerialPort_SendString(char *send_string);
void SerialPort_SendNum(uint32_t send_num, uint16_t send_len);

#endif

```

**- SerialPort.c**
```c
#include "stm32f10x.h"                  // Device header

//串口初始化-USART1
void SerialPort_Init(void){
    //1.开启RCC外设时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    //2.初始化GPIO-PA9
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    //3.初始化USART结构体
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART1, &USART_InitStructure);
    //4.配置中断，开启NVIC（接收数据使用）
    //5.开启外设
    USART_Cmd(USART1, ENABLE);
}

//串口发送1字节数据
void SerialPort_SendByte(uint8_t send_byte){
    //向发送数据寄存器TDR中写入数据
    USART_SendData(USART1, send_byte);
    //确认数据被转移到发送移位寄存器（等待标志位TXE置位）
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE)==RESET);
}

//发送一个数组
void SerialPort_SendArray(uint8_t *send_array, uint16_t size_array){
    uint8_t i=0;
    for(i=0;i<size_array;i++){
        SerialPort_SendByte(send_array[i]);
    }
}

//发送一个字符串
void SerialPort_SendString(char *send_string){
    uint8_t i=0;
    for(i=0; send_string[i]!='\0'; i++){
        SerialPort_SendByte(send_string[i]);
    }
}

//非外部调用函数-幂次函数
uint32_t SerialPort_Pow(uint32_t X, uint32_t Y){
    uint32_t result = 1;
    while(Y--){
        result *= X;
    }
    return result;
}

//发送数字的每一位-先发高位
void SerialPort_SendNum(uint32_t send_num, uint16_t send_len){
    uint16_t i;
    for(i=0;i<send_len;i++){
        SerialPort_SendByte((send_num/SerialPort_Pow(10,send_len-i-1))%10+'0');
    }
}

```

编程感想：
> 1. 关于接线。注意串口通信的两个设备是交叉连接的，所以“USB转串口模块”的TX应该接在stm32串口的RX（PA10）、RX应该接在stm32串口的TX（PA9）！
> 2. 关于```sizeof```。相信很多人也想到，在封装发送数组的函数时，为什么不直接在函数中使用```sizeof```函数来计算数组的大小呢？这样能少传递一个参数，更简洁。但实际上，这里面的水很深，这样操作是不能得到正确结果的，具体原理可以参考CSDN博文“[数组名不等于指针](https://blog.csdn.net/as480133937/article/details/123512497)”。最终结论就是，数组名在传参过程中，会退化成一个指针，此时所表示的大小不是数组的实际大小，而是这个指针的大小，所以要想在函数中使用到数组大小，最好还是多传递一个参数。

### 9.3.2 实验2：移植```printf```函数
需求：将C语言自带函数```printf```进行封装，默认成将需要打印的数据发送到串口，进而可以显示在电脑端串口助手上。

**接线图**、**函数调用（非库函数）** 与上一小节实验“串口发送”相同。本节本人目前也不懂原理，所以下面直接给出 **代码展示**：

**方法一：**
1. 首先点击“魔术棒”，在Target界面的“Code Generation”方框中勾选“USE MicroLIB”。MicroLIB是Keil为嵌入式平台优化的一个精简库，要使用```printf```函数就会用到这个MicroLIB精简库。
2. 对```printf```函数重定向，将```printf```函数打印的东西输出到串口。于是在 **SerialPort.c** 模块中添加下列代码：
```c
#include <stdio.h>

//对printf函数重定向-将fputc函数原型重定向到串口
//注：ptintf函数本质上就是循环调用fputc，将字符一个一个输出
int fputc(int ch, FILE *f){
    SerialPort_SendByte(ch);
    return ch;
}
```
在 **SerialPort.h** 模块中添加下列代码：
```c
#include <stdio.h>
```
3. 于是就可以在 **main.c** 中调用```printf```函数，将数据输出到串口了。
```c
//使用重定向的printf函数
    printf("%d\r\n",666);
```

但注意这个方法直接将```printf```函数重定向到了USART1，别的USART外设（USART2、USART3等）想用就没办法了。所以有如下的改进方法。

**方法二：**
若多个串口都想使用```printf```函数，那么就可以使用```sprintf```函数。```sprintf```函数可以将格式化字符输出到一个字符串里，然后再调用相应的“串口发送字符串”函数发送这个字符串，**整个过程不涉及重定向**，于是就实现了所有USART外设都可以打印信息到串口了。所以下面可以直接在 **main.c** 中定义：
```c
char String[100];//定义一个足够长的字符串数组
sprintf(String, "Num=%d\r\n", 666);//将格式化字符串存储在String中
SerialPort_SendString(String);//串口发送字符串
```

**方法三：**
方法二的```sprintf```函数很方便，但是直接在主函数中写比较麻烦，于是本方法就是来封装“方法二”。具体方法如下：
1. C语言可变参数。在串口模块```SerialPort.c```中添加下面代码：
```c
#include <stdarg.h>

//对sprintf函数进行封装
void SerialPort_Printf(char *format,...){
    char String[100];//定义输出的字符串
    va_list arg;//定义参数列表变量
    va_start(arg, format);//从format位置开始接收参数表，放在arg里面
    vsprintf(String, format, arg);//sprintf只接收直接写的参数，对于封装格式改用vsprintf
    va_end(arg);//释放参数表
    SerialPort_SendString(String);  
}
//注意上述函数要在头文件中声明，但头文件就不需要再添加<stdarg.h>了。
```
2. 直接在主函数中调用：
```c
SerialPort_Printf("Num=%d\r\n", 888);
```
**特殊说明：显示汉字**
使用上面几种```printf```函数时，有可能会出现乱码，要解决这个问题，关键是要保持Keil编译器（“扳手”图标Editor界面的Encoding下拉菜单）和“串口助手”的文本编码一致。可以选择以下两种情况：
> 1. 两者都选择UTF-8编码。但是直接在字符串写汉字，编译器有可能报错，解决方法是点击“魔术棒”-->C/C++-->最下面的Controls输入 ```--no-multibyte-chars``` 即可。
> 2. 有些串口助手软件可能不兼容UTF-8，所以两者都需要选择GB2312编码。
>
> 注：更改编译器文本编码格式后，需要将文件全部关闭，重新打开，否则编码方式不会改变。





### 9.3.3 实验3：串口发送+接收
需求：电脑端发送数据，stm32接收到数据后，将数据在OLED显示屏上显示出来、并且回传到电脑。
> 程序整体思路：
> 1. 查询。主函数不断查询RXNE标志位，但是会占用很多的CPU资源，所以不推荐。
> 2. 中断。推荐方法，下面的演示也是基于此方法。

本实验的 **接线图** 与前两小节的实验均相同，下面给出 **代码调用**：
<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/9-16%E4%BB%A3%E7%A0%81%E8%B0%83%E7%94%A8-%E4%B8%B2%E5%8F%A3%E5%8F%91%E9%80%81%2B%E6%8E%A5%E6%94%B6.png" width="25%">
</div><div align=center>
图9-16 串口发送+接收-代码调用（非库函数）
</div>

下面是 **代码展示**，仅给出在串口模块中增添的函数：
**- main.c**
```c
#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "SerialPort.h"

int main(void){    
    uint8_t Rx_byte = 0;//串口接收的单比特数据
    
    //设置中断分组
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    //OLED初始化
    OLED_Init();
    OLED_ShowString(1,1,"Rx_byte:");
    
    //串口初始化
    SerialPort_Init();

    while(1){
        if(SerialPort_GetRxFlag()==1){
            Rx_byte = SerialPort_GetRxData();
            OLED_ShowHexNum(1,9,Rx_byte,2);
            SerialPort_SendByte(Rx_byte);
        }
    };
}

```

**- SerialPort.c**
```c
uint8_t SerialPort_RxData = 0;
uint8_t SerialPort_RxFlag = 0;

//获取接收的状态
uint8_t SerialPort_GetRxFlag(void){
    if(SerialPort_RxFlag==1){
        SerialPort_RxFlag = 0;
        return 1;
    }else{
        return 0;
    }
}

//获取接收的数据
uint8_t SerialPort_GetRxData(void){
    return SerialPort_RxData;
}

//USART1_RXNE中断函数
void USART1_IRQHandler(void){
    if(USART_GetITStatus(USART1, USART_IT_RXNE)==SET){
        SerialPort_RxFlag = 1;
        SerialPort_RxData = USART_ReceiveData(USART1);
        //读操作可以自动清零标志位，但加上也没事
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}
//不要忘了将前两个函数在头文件中声明
```
















## 9.4 USART串口数据包
数据包的作用是将一个个单独的数据打包，方便进行多字节的数据通信。因为实际应用中，经常需要进行数据打包。比如陀螺仪传感器需要将数据发送到stm32，其中包括X轴、Y轴、Z轴三个字节，循环不断的发送；若采用一个一个进行发送的方式，接收方就有可能分不清对应的顺序，进而出现数据错位现象。此时，若能将同一批数据进行分割和打包，就可以**方便接收方识别**。

**1. 数据包格式的定义：**

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/9-17HEX%E6%95%B0%E6%8D%AE%E5%8C%85.png" width="60%">
</div><div align=center>
图9-17 HEX数据包示意图
</div>

若载荷数据与包头、包尾一样怎么办呢？有三种解决思路：
> 1. 限制载荷数据的范围。使其不会与包头、包尾重复。
> 2. 尽量使用固定长度的数据包。只要数据长度固定，那么就可以通过包头、包尾定位数据。
> 3. 增加包头包尾的数量，使其尽量呈现出载荷数据不会出现的状态。
> 
> 注：<u>包尾可以去除</u>。但是这样会使得载荷数据和包头重复的问题更加严重。

若想发送16位整型数据、32位整型数据、float、double、结构体等，只需使用 **```uint8_t```型指针** 指向这些数据，就可以进行发送(将各种数据转换成字节流)。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/9-18%E6%96%87%E6%9C%AC%E6%95%B0%E6%8D%AE%E5%8C%85%E7%A4%BA%E6%84%8F%E5%9B%BE.png" width="60%">
</div><div align=center>
图9-18 文本数据包示意图
</div>

文本数据包中，每个数据都经过了一层编码和译码。
由于包头包尾非常容易唯一确定，文本数据包基本不用担心载荷数据和包头包尾重复的问题。


优缺点比较：
> - HEX数据包：
> > - 优点：传输最直接，解析数据非常简单，比较适合一些模块发送最原始的数据。如使用串口通信的陀螺仪、温湿度传感器。
> > - 缺点：灵活性不足，载荷容易和包头包尾重复。
>
> - 文本数据包：
> > - 优点：数据直观易理解，非常灵活，比较适合一些输入指令进行人机交互的场合。如蓝牙模块常用的AT指令、CNC和3D打印机常用的G代码，都是文本数据包的格式。
> > - 缺点：解析效率低。


**2. 数据包格式的收发流程：**
数据包发送是非常简单的，直接发就完事儿了。但是接收数据包的过程比较复杂，这是就要考虑使用**状态机**。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/9-19%E7%8A%B6%E6%80%81%E6%9C%BA-%E6%8E%A5%E6%94%B6HEX%E6%95%B0%E6%8D%AE%E5%8C%85.png" width="70%">
</div><div align=center>
图9-19 接收HEX数据包-状态机
</div>

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/9-20%E7%8A%B6%E6%80%81%E6%9C%BA-%E6%8E%A5%E6%94%B6%E6%96%87%E6%9C%AC%E6%95%B0%E6%8D%AE%E5%8C%85.png" width="65%">
</div><div align=center>
图9-20 接收文本数据包-状态机
</div>





## 9.5 USART数据包相关实验
### 9.5.1 实验1：串口收发HEX数据包
需求：自定义数据包格式，使用串口完成指定格式的数据包收发，并将收发结果显示在OLED上。另外按键的功能是将发送的当前存储的发送数据全部加1再发送出去。
> - 数据包头：0xFF。
> - 载荷数据：固定数据段长度为4个字节。
> - 数据包尾：0xFE。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/9-21%E6%8E%A5%E7%BA%BF%E5%9B%BE-%E4%B8%B2%E5%8F%A3%E6%94%B6%E5%8F%91HEX%E6%95%B0%E6%8D%AE%E5%8C%85.png" width="70%">
</div><div align=center>
图9-21 串口收发HEX数据包-接线图
</div>

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/9-22%E4%BB%A3%E7%A0%81%E8%B0%83%E7%94%A8-%E4%B8%B2%E5%8F%A3%E6%94%B6%E5%8F%91HEX%E6%95%B0%E6%8D%AE%E5%8C%85.png" width="25%">
</div><div align=center>
图9-22 串口收发HEX数据包-代码调用（非库函数）
</div>

下面是代码展示：其中将串口的模块的数据接收部分全部删除，重写。
**- main.c**
```c
#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "SerialPort.h"
#include "Key.h"

int main(void){
    //存储串口接收的HEX数据包
    uint8_t *Rx_Packet = SerialPort_GetRxPacket();
    //存储串口发送的HEX数据包
    uint8_t Tx_Packet[4] = {0x01,0x02,0x03,0x04};
    
    //设置中断分组
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    //OLED初始化
    OLED_Init();
    OLED_ShowString(1,1,"Rx_Packet:");
    OLED_ShowString(3,1,"Tx_Packet:");
    //串口初始化
    SerialPort_Init();
    //按键初始化
    Key_Init();
        
    while(1){
        //显示接收到的数据
        if(SerialPort_GetRxFlag()==1){
            OLED_ShowHexNum(2,1,*Rx_Packet,2);
            OLED_ShowHexNum(2,4,*(Rx_Packet+1),2);
            OLED_ShowHexNum(2,7,*(Rx_Packet+2),2);
            OLED_ShowHexNum(2,10,*(Rx_Packet+3),2);
        }
        
        //检测按键，发送数据包到电脑
        if(Key_GetNum()==1){
            Tx_Packet[0]++;
            Tx_Packet[1]++;
            Tx_Packet[2]++;
            Tx_Packet[3]++;
            SerialPort_SendPacket(Tx_Packet);
            OLED_ShowHexNum(4,1,Tx_Packet[0],2);
            OLED_ShowHexNum(4,4,Tx_Packet[1],2);
            OLED_ShowHexNum(4,7,Tx_Packet[2],2);
            OLED_ShowHexNum(4,10,Tx_Packet[3],2);
        }
    };
}

```

**- SerialPort.c新增函数**
```c
uint8_t SerialPort_RxPacket[4];
uint8_t SerialPort_RxPacketFlag = 0;

//获取接收的状态
uint8_t SerialPort_GetRxFlag(void){
    if(SerialPort_RxPacketFlag==1){
        SerialPort_RxPacketFlag = 0;
        return 1;
    }else{
        return 0;
    }
}

//获取接收的HEX数据包
uint8_t* SerialPort_GetRxPacket(void){
    return SerialPort_RxPacket;
}

//发送HEX数据包
void SerialPort_SendPacket(uint8_t *send_array){
    SerialPort_SendByte(0xFF);
    SerialPort_SendArray(send_array, 4);
    SerialPort_SendByte(0xFE);
}

//USART1_RXNE中断函数
void USART1_IRQHandler(void){
    uint8_t rec_byte;
    static uint8_t rx_state;
    static uint8_t rx_index;
    if(USART_GetITStatus(USART1, USART_IT_RXNE)==SET){
        rec_byte = USART_ReceiveData(USART1);
        
        //利用状态机，接收HEX数据包
        if(rx_state==0){
            if(rec_byte==0xFF){
                rx_index = 0;
                rx_state = 1;
            }
        }else if(rx_state==1){
            SerialPort_RxPacket[rx_index] = rec_byte;
            rx_index++;
            if(rx_index>=4){
                rx_state = 2;
            }
        }else if(rx_state==2){
            if(rec_byte==0xFE){
                SerialPort_RxPacketFlag = 1;
                rx_state = 0;
            }
        }
        
        //读操作可以自动清零标志位，但加上也没事
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}
//除了中断函数，其余函数还要在头文件SerialPort.h中声明
```

编程感想：
> 1. 数据混叠。若电脑端连续发送数据包，而stm32处理不及时，会导致数据错位。但是一般像传感器模块等的数据都具有连续性，所以就算数据错位也没关系。
> 2. 发送数据不匹配。注意发送字节数据一定要写成```0x11```的形式，而不是直接写一个```11```进行发送。
> 3. 收发数据没反应。注意一定要在最开始声明的地方赋初值，否则有可能读不出数据。当然，还有一种可能是串口连接不稳定，可以重新拔插一下串口。
> 


### 9.5.2 实验2：串口收发文本数据包
需求：使用电脑端发送指定格式的文本数据包，来控制单片机点亮或熄灭LED灯，单片机完成指令后再将接收的状态回传到电脑。
> - 数据包头：@。
> - 数据包：有效指令为"@LED_ON\r\n"、"@LED_OFF\r\n"。（不定字长）
> - 数据包尾：\r\n。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/9-23%E6%8E%A5%E7%BA%BF%E5%9B%BE-%E4%B8%B2%E5%8F%A3%E6%94%B6%E5%8F%91%E6%96%87%E6%9C%AC%E6%95%B0%E6%8D%AE%E5%8C%85.png" width="60%">
</div><div align=center>
图9-23 串口收发文本数据包-接线图
</div>

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/9-24%E4%BB%A3%E7%A0%81%E8%B0%83%E7%94%A8-%E4%B8%B2%E5%8F%A3%E6%94%B6%E5%8F%91%E6%96%87%E6%9C%AC%E6%95%B0%E6%8D%AE%E5%8C%85.png" width="25%">
</div><div align=center>
图9-24 串口收发文本数据包-代码调用（非库函数）
</div>

下面是代码展示：
**- main.c**
```c
#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "SerialPort.h"
#include "LED.h"
#include <string.h>

int main(void){
    //存储串口接收的HEX数据包
    char *Rx_Packet = SerialPort_GetRxPacket();
    
    //设置中断分组
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    //OLED初始化
    OLED_Init();
    OLED_ShowString(1,1,"Rx_Packet:");
    OLED_ShowString(3,1,"Tx_Packet:");
    //串口初始化
    SerialPort_Init();
    //LED初始化
    LED_Init();
        
    while(1){
        //对接收到的文本进行判断
        if(SerialPort_GetRxFlag()==1){
            //OLED显示接收到的文本
            OLED_ShowString(2,1,"                ");
            OLED_ShowString(2,1,Rx_Packet);
            //根据接收的内容执行相应的动作
            if(strcmp(Rx_Packet, "LED_ON")==0){
                LED1_ON();
                OLED_ShowString(4,1,"                ");
                OLED_ShowString(4,1,"LED_ON_OK");
                SerialPort_SendString("LED_ON_OK\r\n");
            }else if(strcmp(Rx_Packet, "LED_OFF")==0){
                LED1_OFF();
                OLED_ShowString(4,1,"                ");
                OLED_ShowString(4,1,"LED_OFF_OK");
                SerialPort_SendString("LED_OFF_OK\r\n");
            }else{
                OLED_ShowString(4,1,"                ");
                OLED_ShowString(4,1,"ERROR_COMMAND");
                SerialPort_SendString("ERROR_COMMAND\r\n");
            }
        }
    };
}

```

**- SerialPort.c**新增函数（将上一节HEX数据包部分全部删除）
```c
char SerialPort_RxPacket[100];
uint8_t SerialPort_RxPacketFlag = 0;

//获取接收的状态
uint8_t SerialPort_GetRxFlag(void){
    if(SerialPort_RxPacketFlag==1){
        SerialPort_RxPacketFlag = 0;
        return 1;
    }else{
        return 0;
    }
}

//获取接收的HEX数据包
char* SerialPort_GetRxPacket(void){
    return SerialPort_RxPacket;
}

//USART1_RXNE中断函数
void USART1_IRQHandler(void){
    uint8_t rec_byte;
    static uint8_t rx_state;
    static uint8_t rx_index;
    if(USART_GetITStatus(USART1, USART_IT_RXNE)==SET){
        rec_byte = USART_ReceiveData(USART1);
        
        //利用状态机，接收HEX数据包
        if(rx_state==0){
            if(rec_byte== '@'){
                rx_index = 0;
                rx_state = 1;
            }
        }else if(rx_state==1){
            if(rec_byte != '\r'){
                SerialPort_RxPacket[rx_index] = rec_byte;
                rx_index++;
            }else{
                rx_state = 2;
            }
        }else if(rx_state==2){
            if(rec_byte == '\n'){
                SerialPort_RxPacket[rx_index] = '\0';//字符串结束标志符
                SerialPort_RxPacketFlag = 1;
                rx_state = 0;
            }
        }
        
        //读操作可以自动清零标志位，但加上也没事
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}
//除了中断函数，其他函数还要在头文件SerialPort.h中声明
```

**- LED.c**新增函数
```c
/**
  * @brief  LED1亮
  */
void LED1_ON(void){
    GPIO_ResetBits(GPIOA, GPIO_Pin_1);
}

/**
  * @brief  LED1灭
  */
void LED1_OFF(void){
    GPIO_SetBits(GPIOA, GPIO_Pin_1);
}
//注意还要在头文件LED.h中声明
```

编程感想：
> 1. 数据混叠。同样的问题，如果电脑端发送指令太快，就有可能导致stm32来不及处理，导致错误。一个解决方法是指令发慢一点；当然另一种方法是增加标志位，当LED的状态没有改变之前，不理会接收数据，这样会破坏当前程序的独立性，并且原理不复杂，我就先不写了。












## 9.6 软件使用：FlyMcu串口下载 & STLINK Utility

> - **FlyMcu**可以通过串口给stm32下载程序。绿色软件，无需安装。
> - **STLINK Utility**通过STLINK给stm32下载程序。需要安装。

以上两款软件类似于51单片机的程序烧录软件——STC-ISP，可以通过串口给51单片机下载程序。
**硬件方面**，接线图本章的第一个实验“串口发送”相同，这是因为该芯片的串口下载只适配了USART1，所以引脚都要连接在USART1引脚上。
<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/9-14%E6%8E%A5%E7%BA%BF%E5%9B%BE-%E4%B8%B2%E5%8F%A3%E5%8F%91%E9%80%81.png" width="70%">
</div><div align=center>
图9-25 串口下载程序-硬件接线示意图
</div>

下面依次介绍FlyMcu、STLINK Utility：

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/9-26FlyMcu%E7%95%8C%E9%9D%A2%E5%8A%9F%E8%83%BD%E4%BB%8B%E7%BB%8D.png" width="60%">
</div><div align=center>
图9-26 FlyMcu界面功能介绍
</div>

> FlyMcu只能下载HEX文件，**Keil生成HEX文件** 的方法：
> - 点击“魔术棒”-->Output选项卡-->勾选“Create HEX File”。编译过后，就可以在工程目录的“Object文件夹”下找到该工程的HEX文件——“工程名.hex”。
> 
> **使用FlyMcu下载程序**：
> 1. 配置下载串口：点击菜单栏“搜索串口”，稍等一会，点击紧随其后的“Port:xx”选项，选择对应的串口。接着点击紧随其后的“bps:xx”选择下载的波特率。
> 2. 选择程序文件。菜单栏下一行，点击“...”按钮，选择HEX文件。
> 3. 其他功能保持默认。
> 4. 调整启动位置为BootLoader程序。由于串口下载需要使得stm32的启动位置为BootLoader启动程序，而BootLoader启动程序固定放在“系统存储器”，所以BOOT引脚应调整为 [BOOT1,BOOT0]=[0,1]。注意调整BOOT后，一定要按一下最小系统板上的复位键，调整才会生效。
> <img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/1-1%E5%90%AF%E5%8A%A8%E9%85%8D%E7%BD%AE.png" width="80%">
> 5. 点击FlyMcu的“开始编程”。此时程序就会被下载到主闪存中了，但由于BOOT引脚没有变动，所以程序下载完复位后，还是会停留在BootLoader程序中。
> 6. 调整启动位置为主闪存。BOOT引脚应调整为 [BOOT1,BOOT0]=[0,0]，然后按下复位键，就可以看到程序正常运行了。
> > 注：关于串口下载的原理，我觉得前面将“存储器映像”时已经说得很清楚了，就不记录了，UP讲解的地方在“[P30 [9-6] FlyMcu串口下载&STLINK Utility](https://www.bilibili.com/video/BV1th411z7sn/?p=30) 中的4:56~8:11”。
> 
> **每次下载程序都要拔插两边跳线帽，太麻烦了，有什么更简单的方法吗？**
> 1. 方法一：设计外围的**STM32一键下载电路**。利用“USB转串口模块”上CH340的 RTS#、DTR# 两个流控输出引脚分别控制stm32的BOOT0引脚、复位引脚（但是不使用流控功能，而仅仅只是当作一个普通的GPIO口），加上FlyMcu的选项，便可以利用电路自动切换BOOT引脚的高低电平。
> 2. 方法二：软件设置自动跳转（一次性功能）。勾选FlyMcu的“编程后执行”，去除勾选“编程到FLASH时写选项字节”。然后 [BOOT1,BOOT0]=[0,1]，并按下复位键。然后点击“开始编程”，就可以看到程序正常执行。
> > 方法二评价：该方法原理是下载程序后，软件设置从主闪存（0x08000000）开始执行程序，但是按下复位键后，程序又会回到“系统存储器”执行BootLoader程序。所以可以不断的使用串口调试程序，最后调试成功后再将BOOT引脚切换回来即可。也就是，只需要最开始和最后切换跳线帽而已。
>
> **读FLASH：** 读取主闪存中的程序数据，以BIN文件格式存储（BIN文件不包含地址信息，HEX文件包含地址信息）。后续可以使用STLINK Utility下载，实现盗取他人软件劳动成果:joy:。
>
> **清除芯片：** 将主程序区域全部擦除（全部变成高电平）。
>
> **读器件信息：** 读取芯片的信息。但是FLASH容量、SRAM容量等信息可能会出错。
>
> **设定选项字节等：** 可以设置选项字节的各项参数。点击“STM32F1选项设置”，弹出以下界面：
> <img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/9-26%E8%AE%BE%E5%AE%9A%E9%80%89%E9%A1%B9%E5%AD%97%E8%8A%82%E7%AD%89.png" width="50%">
> > 1. 读保护字节：是否允许读出主闪存数据。注意如果“阻止读出”，那么就无法使用Keil下载程序了。另外，在清除读保护的同时，同时也会清空主闪存的数据。
> > 2. 硬件选项字节：有需求可以使用。
> > 3. 用户数据字节：有需求可以使用。那使用选项字节存储数据有什么好处呢？
> > > 1. 选项字节的数据相当于是“世外桃源”，无论如何下载程序，选项字节中的数据都可以不变，可以存储一些不随程序变化的参数；
> > > 2. 用上位机（如FlyMcu、STLINK Utility）可以很方便的修改。
> > 4. 写保护字节：可以对Flash的某几页单独写保护。比如在主程序的最后几页写了一些自定的数据，不希望在下载时被擦除，就可以将最后几页设置写保护锁起来。注意，开启“写保护”后，若需要对保护区写入程序就会出错！并且该软件不支持单独写入选项字节，只能在Flash下载时顺便写入选项字节，那也就是说，**如果将Flash前几页写保护了，就再也无法使用FlyMcu下载程序了！！**（使用STLINK Utility可以补救回来）
> > 注意配置好选项字节数据后，点击“采用这个设置”，并在FlyMcu主界面勾选“编程到FLASH时写选项字节”。



<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/9-27STLINKUtility%E7%95%8C%E9%9D%A2.png" width="65%">
</div><div align=center>
图9-27 STLINK Utility界面功能介绍
</div>

> STLINK Utility需要安装，安装完成后，在桌面上存在快捷方式。STLINK Utility可以下载多种文件，包括HEX文件、BIN文件等。下面演示 **下载程序的流程**：
> 1. 硬件接线。无需连接“USB转串口模块”，只需连接STLINK即可。BOOT引脚设置为 [BOOT1,BOOT0]=[0,0]，然后按下复位键。点击左起第三个按钮进行连接。
> 2. 左起第一个按钮：打开程序文件，支持HEX格式、BIN格式。（也可以直接跳到下一步）
> 3. 左起第六个按钮：下载程序。跳过上一步的，此时选择程序文件。然后点击“Start”下载程序。下载完成后可以发现程序正常运行。
> 
> **选项字节的配置**：菜单栏“Target”-->“Option Byte...”，下面依次介绍：
> > <img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/--27%E9%80%89%E9%A1%B9%E5%AD%97%E8%8A%82%E9%85%8D%E7%BD%AE.png" width="40%">
> > 
> > 1. 读保护。
> > 2. 硬件参数。灰色的选项就是本芯片不支持的选项。
> > 3. 用户参数。
> > 4. 写保护。
> >
> > 注：配置好之后点击“Apply”，就可以直接单独更改选项字节的参数。
> 
> **左起第二个按钮**：读芯片数据，格式可以选为HEX格式、BIN格式。
> **左起第四个按钮**：断开连接。
> **左起第五个按钮**：擦除芯片。
> **STLINK固件更新**：菜单栏“ST-LINK”-->Firmware update-->重新拔插STLINK-->点击“Device Connect”-->“Yes>>>>”。
