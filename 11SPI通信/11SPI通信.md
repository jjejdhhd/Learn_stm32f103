# 11 SPI通信
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



## 11.1 SPI通信协议
<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/11-1%E4%BD%BF%E7%94%A8%E5%88%B0SPI%E7%9A%84%E5%99%A8%E4%BB%B6.png" width="80%">
</div><div align=center>
图11-1 使用到SPI协议的器件
</div>

SPI(Serial Peripheral Interface，串行外设接口)是由Motorola公司开发的一种通用数据总线，与IIC差不多，也是为了实现主控芯片和各种外挂芯片之间的数据交流。**SPI和IIC都是常用的接口协议**，只是根据其不同的特点，应用场景有所不同。
> - 四根通信线(SPI官方文档名称)：
> > 1. SCK：Serial Clock，串行时钟线。别称SCLK、CLK、CK。作用是提供时钟信号，数据位的输入和输出都是在时钟上升沿、下降沿进行的。
> > 2. MOSI：Master Output Slave Input，主机输出从机输入。别称DO(Data Output)。
> > 3. MISO：Master Input Slave Output，主机输入从机输出。别称DI(Data Input)。
> > 4. SS：Slave Select，从机选择。别称NSS(Not Slave Select)、CS(Chip Select)。<u>SPI协议可以为每个从机都开辟一条SS线，专门用于控制该从机的选择(低电平有效)</u>（SPI壕无人性）。
> - 同步(时钟线快点慢点无所谓)，全双工，**高位先行**。
> - 支持总线挂载多设备，仅支持“一主多从”，不支持“多主机”。

IIC和SPI各有优缺点。IIC通过各种软硬件设置，使用最少的硬件资源（2根通信线）实现了最多的功能（双向通信、应答位等），相当于一个“精打细算、思维灵活”的协议；但是为了实现这些功能，其硬件采用开漏输出+上拉电阻的模式以防止电源短路，导致其高电平驱动能力不足，同时也导致其上升沿时间很长，这限制了其最高通信速率（IIC标准模式100kHz/快速模式400kHz）。相对的，SPI协议并没有规定最大传输速度，而是取决于外挂芯片厂商的设计需求，比如W25Q64芯片手册说明其SPI速率最高可达80MHz（甚至比stm32主频还要高）！但是，SPI的设计简单粗暴（学习起来更加简单），功能也不如IIC多，并且会消耗4根通信引脚，所以SPI相当于出手阔绰的土豪（我不在乎占了几根通信线，我只在乎我的任务有没有最简单、最快速的完成）。
> 注：IIC通过改进电路的方式，设计出高速模式3.4MHz，但目前并不普及。一般仍认为最高速率400kHz。


下面来介绍**SPI的硬件规定**：
<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/11-2SPI%E5%BA%94%E7%94%A8%E7%94%B5%E8%B7%AF.png" width="50%">
</div><div align=center>
图11-2 典型的SPI应用电路图
</div>

> - SPI主机：主机一般是控制器，如stm32。
> - SPI从机：从机一般是存储器、显示屏、通信模块、传感器等。
> - 时钟线和数据线：所有SPI设备的SCK、MOSI、MISO分别连在一起，即**相同名称的管脚连接在一起**。
> - 片选线：主机另外引出多条SS控制线，分别接到各从机的SS引脚。注意主机在同一时间只能选择一个从机进行通信(低电平有效)，否则会造成数据冲突。
> 
> 注：**SPI输出引脚配置为推挽输出(驱动能力强)，输入引脚配置为浮空或上拉输入。对于从机来说，只有当其被选中时，MISO才配置为推挽输出，否则为高阻态。**

下面来介绍**SPI基本收发时序**：
**SPI通信的基础是交换字节**。也就是说，每次SPI通信的过程中，通过各自的MOSI、MISO线，主机和从机的寄存器会形成一个循环移位操作，每个比特的通信都是转圈的循环移位，8个时钟周期完整的交换一个字节。那么根据需求有选择的忽略交换过来的数据，就可以实现（以主机举例，从机同理）主机只发送、主机只接收、主从机交换数据这三类操作。
<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/11-3SPI%E4%BA%A4%E6%8D%A2%E5%AD%97%E8%8A%82.png" width="60%">
</div><div align=center>
图11-3 SPI基本收发电路——移位示意图
</div>

> 工作原理：
> - 波特率发生器上升沿：所有寄存器左移一位。
> - 波特率发生器下降沿：将采样输入的数据放到寄存器的最低位。
> - 重复8个时钟周期，便可以实现主机和从机的数据交换。
> 
> 注：实际上，何时移位、何时采样、时钟极性都是可以设置的，下面将介绍。
>
> 功能介绍：显然存在资源浪费现象。
> - 同时进行发送和接收：正常的交换字节。
> - 只想发送、不想接收：不看接收过来的数据。
> - 只想接收、不想发送：随便发一个数据，比如0x00/0xFF。

下面介绍**SPI交换单个字节的时序**：
> - 起始条件和终止条件：起始条件是SS从高电平切换到低电平，终止条件是SS从低电平切换到高电平。
> <img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/11-3%E8%B5%B7%E5%A7%8B%E7%BB%88%E6%AD%A2%E6%9D%A1%E4%BB%B6.png" width="50%">
> - 交换一个字节：两个配置位分别为**CPOL**(Clock Polarity, 时钟极性)规定空闲状态的时钟高低电平、**CPHA**(Clock Phase, 时钟相位)规定数据移入(数据采样)、移出的时机。
> > 1. **【模式0】[CPOL,CPHA] = [0,0]**，SCK低电平为空闲状态；SCK第一个边沿(上升沿)移入数据，第二个边沿(下降沿)移出数据。
> <img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/11-3%E9%85%8D%E7%BD%AE00.png" width="70%">
> > 2. 【模式1】[CPOL,CPHA] = [0,1]，SCK低电平为空闲状态；SCK第一个边沿移出数据，第二个边沿移入数据。
> <img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/11-3%E9%85%8D%E7%BD%AE01.png" width="70%">
> > 3. 【模式2】[CPOL,CPHA] = [1,0]，SCK高电平为空闲状态；SCK第一个边沿移入数据，第二个边沿移出数据。
> <img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/11-3%E9%85%8D%E7%BD%AE10.png" width="70%">
> > 4. 【模式3】[CPOL,CPHA] = [1,1]，SCK高电平为空闲状态；SCK第一个边沿移出数据，第二个边沿移入数据。
> <img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/11-3%E9%85%8D%E7%BD%AE11.png" width="70%">

下面介绍几个**SPI的通信实例**：
上面仅介绍了最基本的交换字节的时序。实际上，SPI要想与从机完成真正的通信，也有更高维度的数据帧结构：**指令码+读写数据**。每个SPI从机芯片都规定了指令集，指令集中不同的指令码对应不同的功能。下一小节将详细介绍W25Q64的指令集，本节仅看三个演示（默认【模式0】）：

1. 发送指令：向SS指定的设备，发送指令（0x06）。
<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/11-4SPI%E5%8F%91%E9%80%81%E6%8C%87%E4%BB%A4.png" width="55%">
</div><div align=center>
图11-4 时序图——SPI发送指令
</div>

由于上图是软件模拟SPI时序，所以MOSI的数据变化（那个上升沿）没有紧贴SCK下降沿，但是在硬件模拟SPI中是紧贴的。

2. 指定地址写：向SS指定的设备，发送写指令（0x02），随后在指定地址（Address[23:0]）下，写入指定数据（Data）。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/11-5SPI%E6%8C%87%E5%AE%9A%E5%9C%B0%E5%9D%80%E5%86%99.png" width="99%">
</div><div align=center>
图11-5 时序图——SPI指定地址写
</div>

$$
主机：发指令0x02+发Address[23:16]+发Address[15:8]+发Address[7:0]+发Data
$$

3. 指定地址读：向SS指定的设备，发送读指令（0x03），随后在指定地址（Address[23:0]）下，读取从机数据（Data）。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/11-6SPI%E6%8C%87%E5%AE%9A%E5%9C%B0%E5%9D%80%E8%AF%BB.png" width="99%">
</div><div align=center>
图11-6 时序图——SPI指定地址读
</div>

$$
主机：发指令0x03+发Address[23:16]+发Address[15:8]+发Address[7:0]+收Data
$$

首先可以观察到，由于从机SPI协议由硬件控制，所以从机发送过来的数据，其数据变化边沿都是紧贴着时钟下降沿完成的。并且，如果最后接收完一个字节后时钟仍为低电平，那么从机会继续将下一个地址的数据发送过来，就实现了“连续地址读”。

## 11.2 W25Q64简介
W25Qxx系列是一种低成本、小型化、使用简单的非易失性存储器，常应用于数据存储、字库存储、固件程序存储(电脑BIOS固件)等场景。也就是如果程序需要存储大量的数据，就可以考虑外挂这款芯片。
> - 存储介质：[Nor Flash](https://baike.baidu.com/item/NOR%20Flash)(闪存)。还有Flash一种是[Nand Flash](https://baike.baidu.com/item/Nand%20flash)。
> - 时钟频率：80MHz / 160MHz (Dual SPI) / 320MHz (Quad SPI)。两重SPI是将MOSI和MISO同时用于收发；四重SPI是再加上写保护WP、数据保持HOLD进行收发数据，共四根线同时收发数据。
> - 存储容量（24位地址）：
> > W25Q40： 4Mbit / 512KByte
> > W25Q80： 8Mbit / 1MByte
> > W25Q16： 16Mbit / 2MByte
> > W25Q32： 32Mbit / 4MByte
> > **W25Q64： 64Mbit / 8MByte**(本节所使用)
> > W25Q128：128Mbit / 16MByte
> > W25Q256：256Mbit / 32MByte

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/11-7W25Q64%E5%8E%9F%E7%90%86%E5%9B%BE.png" width="60%">
</div><div align=center>
图11-7 W25Q64实物图及原理图
</div>

<div align=center>
表11-1 W25Q64引脚说明
</div><div align=center>
<style type="text/css">
.tg  {border-collapse:collapse;border-spacing:0;}
.tg td{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  overflow:hidden;padding:10px 5px;word-break:normal;}
.tg th{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  font-weight:normal;overflow:hidden;padding:10px 5px;word-break:normal;}
.tg .tg-baqh{text-align:center;vertical-align:top}
.tg .tg-5ncq{color:#333333;font-weight:bold;text-align:center;vertical-align:top}
</style>
<table class="tg">
<thead>
  <tr>
    <th class="tg-5ncq"><span style="font-weight:bold">引脚</span></th>
    <th class="tg-5ncq"><span style="font-weight:bold">功能</span></th>
  </tr>
</thead>
<tbody>
  <tr>
    <td class="tg-baqh"><span style="color:#000">VCC、GND</span></td>
    <td class="tg-baqh"><span style="color:#000">电源（2.7~3.6V）</span></td>
  </tr>
  <tr>
    <td class="tg-baqh"><span style="color:#000">CS（SS）</span></td>
    <td class="tg-baqh"><span style="color:#000">SPI片选</span></td>
  </tr>
  <tr>
    <td class="tg-baqh"><span style="color:#000">CLK（SCK）</span></td>
    <td class="tg-baqh"><span style="color:#000">SPI时钟</span></td>
  </tr>
  <tr>
    <td class="tg-baqh"><span style="color:#000">DI（MOSI）</span></td>
    <td class="tg-baqh"><span style="color:#000">SPI主机输出从机输入</span></td>
  </tr>
  <tr>
    <td class="tg-baqh"><span style="color:#000">DO（MISO）</span></td>
    <td class="tg-baqh"><span style="color:#000">SPI主机输入从机输出</span></td>
  </tr>
  <tr>
    <td class="tg-baqh"><span style="color:#000">WP</span></td>
    <td class="tg-baqh"><span style="color:#000">写保护</span></td>
  </tr>
  <tr>
    <td class="tg-baqh"><span style="color:#000">HOLD</span></td>
    <td class="tg-baqh"><span style="color:#000">数据保持，用于SPI总线进入中断</span></td>
  </tr>
</tbody>
</table>
</div>

上面原理图中可以看出，WP和HOLD两根线都接到了VCC正极，那就表示这两个功能暂时没有用到。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/11-8W25Q64%E7%94%B5%E8%B7%AF%E6%A1%86%E5%9B%BE.png" width="60%">
</div><div align=center>
图11-8 W25Q64电路框图
</div>

> - 芯片层级结构：**8MB存储空间-->128个64KB块-->16个4KB扇区-->16个256B页**。
> - SPI控制逻辑：芯片内部进行地址锁存、数据读写等操作，都可以由控制逻辑自动完成，**外部芯片主要关注与控制逻辑进行数据交互即可**。
> - 状态寄存器：非常重要，指明芯片是否处于忙状态、是否写使能、是否写保护等。
> - 写控制逻辑：配合WP引脚实现硬件写保护。
> - 高电压生成器：配合Flash进行编程，用于击穿内部晶体管，以实现掉电不丢失数据的特性。
> - 页地址锁存/寄存器：锁存3字节地址的高两个字节。通过写保护和行解码，来选择要操作哪一页。
> - 字节地址锁存/寄存器：锁存3字节地址的最低一个字节。通过列解码和256字节页缓存，来进行指定特定地址的读写操作。由于配有计数器，所以可以很容易实现从指定地址开始，连续读写多个字节。
> - 256字节页缓存区：是一个256字节的RAM存储器，通过这个RAM缓冲区以实现数据读写。写入数据时，为了跟上SPI通信速度，会先将数据放到RAM缓存区里，时序结束后，芯片才会将数据复制到Flash中，所以 **单次连续写入数据禁止超过256个字节**，并且写时序后芯片会进入忙状态（状态寄存器）。读取数据时由于只需要看Flash电路状态，所以没有限制。

与RAM支持直接读写、覆盖读写不同，为了兼顾掉电不丢失与存储容量大、成本低的特点，Flash会在操作的便捷性上做出一些妥协和让步。于是下面是 **Flash操作注意事项**：
> 写入操作时：
> - 写入操作前必须先进行写使能，一个写使能只能保证后面一条写操作的执行。这样设置是防止误操作。
> - 每个数据位只能由1改写为0，不能由0改写为1。所以**写入数据前必须先擦除**(发送擦除指令)，擦除后所有数据位变为1。
> - 擦除必须按照 **最小擦除单元(扇区)** 进行，可以选择的擦除单元有整个芯片、块、扇区。要想不丢失数据，就需要先将所有的数据读取出来，擦除后再统一写入；或者直接为单个字节数据占用一个扇区，就不需要先读取了。
> - **最多连续写入一页的数据(256字节)**，超过页尾位置的数据，会回到页首覆盖写入。也就是注意地址不要跨越页尾。
> - 写入操作结束后，芯片进入忙状态，不响应新的读写操作。可以使用“读状态寄存器指令”，BUSY为0时芯片空闲。
> -  写入操作总结：写使能-->(备份数据)-->擦除-->等待BUSY位为0-->写入数据-->等待BUSY位为0-->其他操作。
> 
> 读取操作时：
> - 直接调用读取时序，无需使能，无需额外操作，没有页的限制，读取操作结束后不会进入忙状态，但不能在忙状态时读取。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/11-9W25Q64%E6%8C%87%E4%BB%A4%E9%9B%86.png" width="70%">
</div><div align=center>
图11-9 W25Q64指令集（部分）
</div>

> - 更多指令集详细信息可以查看W25Q64芯片手册的“11.2.2 Instruction Set Table 1”。

## 11.3 实验：软件SPI读写W25Q64
需求：用stm32四个引脚控制高低电平，与W25Q64进行通信。在OLED上显示：
> - 第一行：显示MID(Manufacturer)和DID(Device)。MID是厂商ID(0xEF)，DID是设备ID(0x4017)。
> - 第二行：显示写入的四个数据。
> - 第三行：显示读出的四个数据。


<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/11-10%E6%8E%A5%E7%BA%BF%E5%9B%BE-%E8%BD%AF%E4%BB%B6SPI.png" width="70%">
</div><div align=center>
图11-10 软件SPI读写W25Q64-接线图
</div>

实际上使用软件模拟SPI，是可以任意选择端口的。但是为了后续硬件SPI实验不用再拆线，所以这里选择和stm32上SPI外设引脚。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/11-11%E4%BB%A3%E7%A0%81%E8%B0%83%E7%94%A8-%E8%BD%AF%E4%BB%B6SPI.png" width="25%">
</div><div align=center>
图11-11 软件SPI读写W25Q64-代码调用（非库函数）
</div>

下面是代码展示：
**- main.c**
```c
#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "W25Q64.h"

uint8_t ArrayWrite[4] = {0x11,0x22,0x33,0x44};
uint8_t ArrayRead[4];

int main(void){
    OLED_Init();   //OLED初始化
    W25Q64_Init(); //W25Q64初始化
    
    //初始化OLED显示
    OLED_ShowString(1,1,"MID:FF DID:FFFF ");
    OLED_ShowString(2,1,"W:FF FF FF FF");
    OLED_ShowString(3,1,"R:FF FF FF FF");
    
    //读取W25Q64的ID号
    W25Q64_ID W25Q64_ID_Structure;
    W25Q64_ReadID(&W25Q64_ID_Structure);
    OLED_ShowHexNum(1,5,W25Q64_ID_Structure.MID,2);
    OLED_ShowHexNum(1,12,W25Q64_ID_Structure.DID,4);
    
    //写入数据
    W25Q64_EraseSector(0x00000000);               //写擦除
    W25Q64_PageProgram(0x00000000, ArrayWrite, 4);//写数据
    //读取数据
    W25Q64_ReadByte(0x00000000, ArrayRead, 4);
    
    //将读写数据显示到OLED上
    uint16_t i;
    for(i=0;i<4;i++){
        OLED_ShowHexNum(2,3+3*i,ArrayWrite[i],2);
        OLED_ShowHexNum(3,3+3*i,ArrayRead[i],2);
    }
    while(1){
    };
}

```

**- SPI_User.h**
```c
#ifndef __SPI_USER_H
#define __SPI_USER_H

//SPI初始化
void SPI_User_Init(void);
//SPI起始信号
void SPI_User_Start(void);
//SPI终止信号
void SPI_User_Stop(void);
//SPI交换一个字节(模式0)
uint8_t SPI_User_SwapByte(uint8_t SendByte);

#endif

```

**- SPI_User.c**
```c
#include "stm32f10x.h"                  // Device header

//SPI-SS引脚写操作
void SPI_User_W_SS(uint8_t BitValue){
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, (BitAction)BitValue);
}
//SPI-SCK引脚写操作
void SPI_User_W_SCK(uint8_t BitValue){
    GPIO_WriteBit(GPIOA, GPIO_Pin_5, (BitAction)BitValue);
}
//SPI-MOSI引脚写操作
void SPI_User_W_MOSI(uint8_t BitValue){
    GPIO_WriteBit(GPIOA, GPIO_Pin_7, (BitAction)BitValue);
}
//SPI-MISO引脚读操作
uint8_t SPI_User_R_MISO(void){
    return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6);
}

//SPI初始化
void SPI_User_Init(void){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    //初始化CLK、SS、MOSI-推挽输出
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    //初始化MISO-上拉输入
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    SPI_User_W_SS(1); //SS默认高电平
    SPI_User_W_SCK(0);//模式0：SCK默认低电平
}
    
//SPI起始信号
void SPI_User_Start(void){
    SPI_User_W_SS(0);
}

//SPI终止信号
void SPI_User_Stop(void){
    SPI_User_W_SS(1);
}

//SPI交换一个字节(模式0)
uint8_t SPI_User_SwapByte(uint8_t SendByte){
    uint8_t i;
    //实现方法一：使用掩码依次提取数据的每一位
    uint8_t RecByte = 0x00;
    for(i=0;i<8;i++){
        SPI_User_W_MOSI((0x80>>i) & SendByte);
        SPI_User_W_SCK(1);//SCK上升沿
        if(SPI_User_R_MISO()==1){
            RecByte = (0x80>>i) | RecByte;
        }
        SPI_User_W_SCK(0);//SCK下降沿
    }
//    //实现方法二：使用循环移位模型
//    for(i=0;i<8;i++){
//        SPI_User_W_MOSI(0x80 & SendByte);
//        SendByte <<= 1;
//        SPI_User_W_SCK(1);//SCK上升沿
//        if(SPI_User_R_MISO()){
//            SendByte |= 0x01;
//        }
//        SPI_User_W_SCK(0);//SCK下降沿
//    }
    return RecByte;
}

```

**- W25Q64.h**
```c
#ifndef __W25Q64_H
#define __W25Q64_H

//W25Q64的ID结构体
typedef struct{
    uint8_t MID;
    uint16_t DID;
}W25Q64_ID;

//W25Q64的指令码
#define W25Q64_WRITE_ENABLE                      0x06
#define W25Q64_WRITE_DISABLE                     0x04
#define W25Q64_READ_STATUS_REGISTER_1            0x05
#define W25Q64_READ_STATUS_REGISTER_2            0x35
#define W25Q64_WRITE_STATUS_REGISTER             0x01
#define W25Q64_PAGE_PROGRAM                      0x02
#define W25Q64_QUAD_PAGE_PROGRAM                 0x32
#define W25Q64_BLOCK_ERASE_64KB                  0xD8
#define W25Q64_BLOCK_ERASE_32KB                  0x52
#define W25Q64_SECTOR_ERASE_4KB                  0x20
#define W25Q64_CHIP_ERASE                        0xC7
#define W25Q64_ERASE_SUSPEND                     0x75
#define W25Q64_ERASE_RESUME                      0x7A
#define W25Q64_POWER_DOWN                        0xB9
#define W25Q64_HIGH_PERFORMANCE_MODE             0xA3
#define W25Q64_CONTINUOUS_READ_MODE_RESET        0xFF
#define W25Q64_RELEASE_ POWER_DOWN_HPM_DEVICE_ID 0xAB
#define W25Q64_MANUFACTURER_DEVICE_ID            0x90
#define W25Q64_READ_UNIQUE_ID                    0x4B
#define W25Q64_JEDEC_ID                          0x9F
#define W25Q64_READ_DATA                         0x03
#define W25Q64_FAST_EAD                          0x0B
#define W25Q64_FAST_READ_DUAL_OUTPUT             0x3B
#define W25Q64_FAST_READ_DUAL_IO                 0xBB
#define W25Q64_FAST_READ_QUAD_OUTPUT             0x6B
#define W25Q64_FAST_READ_QUAD_IO                 0xEB
#define W25Q64_OCTAL_WORD_READ_QUAD_IO           0xE3

#define W25Q64_DUMMY_BYTE                        0xFF


//W25Q64初始化
void W25Q64_Init(void);
//读取W25Q64的ID
void W25Q64_ReadID(W25Q64_ID* ID_struct);
//页编程
void W25Q64_PageProgram(uint32_t start_addr, uint8_t *wByteArray, uint16_t count);
//写擦除-扇区
void W25Q64_EraseSector(uint32_t erase_addr);
//读取数据-读数据可以跨页
void W25Q64_ReadByte(uint32_t start_addr, uint8_t *rByteArray, uint32_t count);

#endif

```

**- SPI_User.c**
```c
#include "stm32f10x.h"                  // Device header
#include "SPI_User.h"
#include "W25Q64.h"

//W25Q64初始化
void W25Q64_Init(void){
    SPI_User_Init();
}

//读取W25Q64的ID
void W25Q64_ReadID(W25Q64_ID* ID_struct){
    SPI_User_Start();
    SPI_User_SwapByte(W25Q64_JEDEC_ID);
    ID_struct->MID = SPI_User_SwapByte(W25Q64_DUMMY_BYTE);
    ID_struct->DID = SPI_User_SwapByte(W25Q64_DUMMY_BYTE);
    ID_struct->DID <<= 8;
    ID_struct->DID |= SPI_User_SwapByte(W25Q64_DUMMY_BYTE);    
    SPI_User_Stop();
}

//发送写使能指令
void W25Q64_WriteEnable(void){
    SPI_User_Start();
    SPI_User_SwapByte(W25Q64_WRITE_ENABLE);
    SPI_User_Stop();
}

//等待W25Q64恢复成空闲状态
void W25Q64_WaitBusy(void){
    SPI_User_Start();
    SPI_User_SwapByte(W25Q64_READ_STATUS_REGISTER_1);//读状态寄存器1
    while((SPI_User_SwapByte(W25Q64_DUMMY_BYTE)&0x01) == 0x01);//Busy位为0就一直等待
    SPI_User_Stop();
}

//页编程
void W25Q64_PageProgram(uint32_t start_addr, uint8_t *wByteArray, uint16_t count){
    uint16_t i;
    W25Q64_WaitBusy();                          //等待忙状态置0
    W25Q64_WriteEnable();                       //写使能
    
    SPI_User_Start();
    SPI_User_SwapByte(W25Q64_PAGE_PROGRAM);     //写指令
    SPI_User_SwapByte(start_addr>>16);          //写地址[23:16]
    SPI_User_SwapByte(start_addr>>8);           //写地址[15:8]
    SPI_User_SwapByte(start_addr);              //写地址[7:0]
    for(i=0;i<count;i++){
        SPI_User_SwapByte(*(wByteArray+i));     //写数据
    }    
    SPI_User_Stop();
}

//写擦除-扇区
void W25Q64_EraseSector(uint32_t erase_addr){
    W25Q64_WaitBusy();                          //等待忙状态置0
    W25Q64_WriteEnable();                       //写使能
    
    SPI_User_Start();
    SPI_User_SwapByte(W25Q64_SECTOR_ERASE_4KB); //擦除指令
    SPI_User_SwapByte(erase_addr>>16);          //擦除地址[23:16]
    SPI_User_SwapByte(erase_addr>>8);           //擦除地址[15:8]
    SPI_User_SwapByte(erase_addr);              //擦除地址[7:0]
    SPI_User_Stop();
}

//读取数据-读数据可以跨页
void W25Q64_ReadByte(uint32_t start_addr, uint8_t *rByteArray, uint32_t count){
    uint32_t i;
    W25Q64_WaitBusy();                       //等待忙状态置0
    SPI_User_Start();
    SPI_User_SwapByte(W25Q64_READ_DATA);     //读指令
    SPI_User_SwapByte(start_addr>>16);       //读地址[23:16]
    SPI_User_SwapByte(start_addr>>8);        //读地址[15:8]
    SPI_User_SwapByte(start_addr);           //读地址[7:0]
    for(i=0;i<count;i++){
        *(rByteArray+i) = SPI_User_SwapByte(W25Q64_DUMMY_BYTE);//读数据
    }
    SPI_User_Stop();
}

```

编程感想：
> 1. 初始化数组的时候，一定要记得加```0x```，否则默认就是十进制数。
> 2. GPIO引脚选中的时候，格式为```GPIO_Pin_1```，而不是```GPIO_PinSource1```。










## 11.4 SPI通信外设
STM32内部集成了硬件SPI收发电路，可以由硬件自动执行时钟生成、数据收发等功能，不仅SPI性能更高、同时也减轻CPU的负担。下面是stm32中SPI的性能参数<u>(粗体表示使用中的默认配置)</u>：
> - 可配置**8位**/16位数据帧、**高位先行**/低位先行
> - 时钟频率： f~PCLK~/(2,4,8,16,32,64,128,256)，也就是外设时钟分频得来。APB2中，f~PCLK~=72MHz；APB1中，f~PCLK~=36MHz。
> - 支持多主机模型、**主机操作**、从机操作。
> - 可精简为半双工/单工通信，一般不用。
> - 支持DMA
> - 兼容[I2S协议](https://baike.baidu.com/item/I2S)(一种数字音频信号传输的专用协议)。
> - STM32F103C8T6 硬件SPI资源：SPI1(APB2)、SPI2(APB1)。

<div align=center>
表11-2 SPI引脚复用表
</div><div align=center>
<style type="text/css">
.tg  {border-collapse:collapse;border-spacing:0;}
.tg td{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  overflow:hidden;padding:10px 5px;word-break:normal;}
.tg th{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  font-weight:normal;overflow:hidden;padding:10px 5px;word-break:normal;}
.tg .tg-zeca{color:#333333;font-weight:bold;text-align:center;vertical-align:middle}
.tg .tg-wa1i{font-weight:bold;text-align:center;vertical-align:middle}
.tg .tg-nrix{text-align:center;vertical-align:middle}
</style>
<table class="tg">
<thead>
  <tr>
    <th class="tg-zeca">引脚</th>
    <th class="tg-zeca">SPI1</th>
    <th class="tg-wa1i">SPI2</th>
  </tr>
</thead>
<tbody>
  <tr>
    <td class="tg-nrix">NSS</td>
    <td class="tg-nrix">PA4/PA15</td>
    <td class="tg-nrix">PB12</td>
  </tr>
  <tr>
    <td class="tg-nrix">SCK</td>
    <td class="tg-nrix">PA5/PA3</td>
    <td class="tg-nrix">PB13</td>
  </tr>
  <tr>
    <td class="tg-nrix">MISO</td>
    <td class="tg-nrix">PA6/PA4</td>
    <td class="tg-nrix">PB14</td>
  </tr>
  <tr>
    <td class="tg-nrix">MOSI</td>
    <td class="tg-nrix">PA7/PA5</td>
    <td class="tg-nrix">PB15</td>
  </tr>
</tbody>
</table>

注：斜杠后引脚定义表示引脚重映射
</div>

下面介绍stm32中SPI外设的电路框图：

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/11-12SPI%E5%A4%96%E8%AE%BE%E6%A1%86%E5%9B%BE.png" width="70%">
</div><div align=center>
图11-12 SPI外设电路框图
</div>

> 寄存器配合介绍：
> - 移位寄存器：右侧的数据一位一位地从MOSI输出，MOSI的数据一位一位地移到左侧数据位。
> - LSBFIRST控制位：用于控制移位寄存器是低位先行(1)还是高位先行(0)。
> - MISO和MOSI的交叉：用于切换主从模式。不交叉时为主机模式，交叉时为从机模式。
> - 接收缓冲区、发送缓冲区：实际上分别就是接收数据寄存器RDR、发送数据缓冲区TDR。TDR和RDR占用同一个地址，统一叫作DR。移位寄存器空时，TXE标志位置1，TDR移入数据，下一个数据移入到TDR；移位寄存器接收完毕（同时也标志着移出完成），RXNE标志位置1，数据转运到RDR，此时需要尽快读出RDR，以防止被下一个数据覆盖。
> > 细节：SPI为全双工同步通信，所以为一个移位寄存器、两个缓冲区；IIC为单工通信，所以只需要一个移位寄存器、一个缓冲区；USRT为全双工异步通信，所以需要两个移位寄存器、两个缓冲区，且这两套分别独立。
>
> 控制逻辑介绍：
> - 波特率发生器：本质是一个分频器，用于产生SCK时钟。输入时钟就是外设时钟f~PCLK~=72MHz/36MHz。每产生一个时钟，就移入/移出一个比特。SPI_CR1中的[BR2,BR1,BR0]用于产生分频系数。
> <img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/11-12%E6%B3%A2%E7%89%B9%E7%8E%87%E6%8E%A7%E5%88%B6.png" width="60%">
> - SPI_CR1：SPI控制寄存器1，下面简单介绍一下。详细可以参考中文数据手册“23.5 SPI和I2S寄存器描述”一节。
> > - SPE(SPI Enable)：SPI使能，就是SPI_Cmd函数配置的位。
> > - BR(Baud Rate)：配置波特率，也就是SCK时钟频率。
> > - MSTR(Master)：配置主机模式(1)、从机(0)模式。
> > - CPOL、CPHA：用于选择SPI的四种模式。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/11-13SPI%E7%AE%80%E5%8C%96%E5%9B%BE.png" width="70%">
</div><div align=center>
图11-13 SPI外设简化框图-SPI主机模式
</div>

> - 波特率发生器：用于产生SCK时钟。
> - 数据控制器：根据配置，控制SPI外设电路的运行。
> - 字节交换过程：交换完毕，移位寄存器空，则TXE位置1、RXNE位置1，TDR会自动转运数据到移位寄存器，RDR数据等待用户读取。
> - 开关控制【代码】：SPI外设使能。
> - GPIO【代码】：用于各引脚的初始化。
> - 从机使能引脚SS【代码】：并不存在于SPI硬件外设中，实际使用随便指定一个GPIO口(例如PA4)即可。在一主多从模式下，GPIO模拟SS是最佳选择。

上面介绍了stm32中SPI外设的基本原理，在实际书写代码的过程中，使用一个结构体便可以直接配置 波特率发生器 和 字节交换的默认模式，这是SPI外设内部便会自动工作，**用户额外需要关心的只是何时读写DR**。下面介绍读写时序的流程，分别是性能更高、使用复杂的“主模式全双工连续传输”，以及性能较低、常用且简单易学的“非连续传输”：

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/11-14-%E4%B8%BB%E6%A8%A1%E5%BC%8F%E8%BF%9E%E7%BB%AD%E4%BC%A0%E8%BE%93-%E6%97%B6%E5%BA%8F%E5%9B%BE.png" width="70%">
</div><div align=center>
图11-14 主模式全双工连续传输-时序图
</div>

> 本模式可以实现数据的连续传输。
> - 连续写入数据：只要TXE置1，就立马进中断写入数据（会同时清除TXE位）；当写入到最后一个数据时，等待BSY位清除，发送流程完毕。
> - 连续读出数据：只要RXNE位置1，就立马进中断读出数据（会同时清除RXNE位）。若不及时读出，现有数据就会被新的数据覆盖。
>
> 评价：连续数据流传输对于软件的配合要求较高，需要在每个标志位产生后及时读写数据，整个发送和接收的流程是交错的，但是传输效率是最高的。对传输效率有严格要求才会用到此模式，否则一般采用下面更为简单的“非连续传输”。


<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/11-12%E9%9D%9E%E8%BF%9E%E7%BB%AD%E4%BC%A0%E8%BE%93-%E6%97%B6%E5%BA%8F%E5%9B%BE.png" width="70%">
</div><div align=center>
图11-15 非连续传输-时序图
</div>

> 本模式对于程序设计非常友好。
> - 字节交换流程：最开始等待TXE位置1，发送一个数据（会自动清除TXE）；等待RXNE置1，读取数据。再进行下一次的字节交换。
>
> 评价：非连续传输会损失数据传输效率，数据传输速率越快，损失越明显。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/11-6SPI%E6%8C%87%E5%AE%9A%E5%9C%B0%E5%9D%80%E8%AF%BB.png" width="99%">
</div><div align=center>
a) 软件SPI
</div><div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/11-16%E8%BD%AF%E7%A1%AC%E4%BB%B6%E6%B3%A2%E5%BD%A2%E5%AF%B9%E6%AF%94.png" width="99%">
</div><div align=center>
b) 硬件SPI
</div><div align=center>
图11-16 软/硬件波形图对比-SPI指定地址写
</div>

从上面SPI软/硬件波形的对比来看，硬件SPI的一大特点就是数据变化紧贴SCK边沿，而不是像软件SPI那样，因为代码语句的执行会有一定的延迟（即使SCK边沿变化和输出数据变化的代码是挨在一起的）。






## 11.5 实验：硬件SPI读写W25Q64
需求：与软件SPI读写W25Q64相同，读出W25Q64的ID，并将ID和读写数据都显示在OLED上。
> 注：根据引脚定义表，选择SPI1的PA5、PA6、PA7进行通信，另外选择PA4作为片选引脚SS，于是引脚选择也就和软件SPI相同。

**接线图**、**代码调用** 与上一个实验——“软件SPI读写W25Q64”一致，只是将```SPI_User```中的引脚变化使用硬件来实现了。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/10-17%E4%BB%A3%E7%A0%81%E8%B0%83%E7%94%A8-%E7%A1%AC%E4%BB%B6%E8%AF%BB%E5%86%99W25Q64.png" width="25%">
</div><div align=center>
图10-17 硬件读写W25Q64-代码调用（非库函数）
</div>

所以下面**代码展示**：```main.c```、```W25Q64.h```、```W25Q64.c```、```SPI_User.h```与源文件相同，只是```SPI_User.c```中只是将涉及到SPI引脚变化的操作都替换成库函数了，具体的变化过程参考图11-15“非连续传输”时序图及其说明。

**- SPI_User.c**
```c
#include "stm32f10x.h"                  // Device header

//SPI-SS引脚写操作
void SPI_User_W_SS(uint8_t BitValue){
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, (BitAction)BitValue);
}

//SPI初始化
void SPI_User_Init(void){
    //1.开启外设时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//GPIO时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE); //SPI1时钟
    //2.初始化端口
    //初始化SS-推挽输出
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    //初始化CLK、MOSI-外设复用推挽输出
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    //初始化MISO-上拉输入
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    //3.配置SPI
    SPI_InitTypeDef SPI_InitStructure;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2; //APB2的2分频-36MHz
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;                       //第一个边沿采样，第二个边沿输出
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;                         //时钟空闲时低电平
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;                  //数据位宽8bit
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; //SPI双线全双工
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;                 //高位先行
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;                      //主机模式
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;                          //软件自定义片选信号
    SPI_InitStructure.SPI_CRCPolynomial = 0x0007;                      //CRC用不到，所以默认值7
    SPI_Init(SPI1, &SPI_InitStructure);
    //4.SPI使能
    SPI_Cmd(SPI1, ENABLE);
    
    SPI_User_W_SS(1);//默认不选中从机
}
    
//SPI起始信号
void SPI_User_Start(void){
    SPI_User_W_SS(0);
}

//SPI终止信号
void SPI_User_Stop(void){
    SPI_User_W_SS(1);
}

//SPI交换一个字节(模式0)
uint8_t SPI_User_SwapByte(uint8_t SendByte){
    while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE)!=SET); //等待TXE置1
    SPI_I2S_SendData(SPI1,SendByte);                          //发送数据到TDR
    while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE)!=SET);//等待RNXE置1
    return SPI_I2S_ReceiveData(SPI1);                         //从RDR接收数据
}

```

编程感想：
> 1. Keil小技巧：没有代码自动补全时，就按```Ctrl+Alt+Space```，但记得先把系统中切换输入的```Ctrl+Space```快捷键取消。
> 2. 关于是否清除标志位。虽然在11-14、11-15的SPI时序图讲解中，手册上写明了TXE和RXNE“由硬件置位并由软件清除”，但是这并不代表需要一条专门的语句来清除标志位，比如SPI中就是读写数据的过程中就自动清除了，所以具体还需要查看数据手册的描述。
