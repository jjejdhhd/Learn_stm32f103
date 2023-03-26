# 8 DMA直接存储器读取
 [toc]

注：笔记主要参考B站 [江科大自化协](https://space.bilibili.com/383400717) 教学视频“[STM32入门教程-2023持续更新中](https://www.bilibili.com/video/BV1th411z7sn/)”。
注：工程及代码文件放在了本人的[Github仓库](https://github.com/jjejdhhd/Learn_stm32f103/tree/main)。
***

## 8.1 DMA简介
**DMA（Direct Memory Access）直接存储器存取** 可以直接访问STM32内部的存储器，包括外设寄存器（一般指外设的数据寄存器DR，如ADC的数据寄存器、串口数据寄存器等）、运行内存SRAM（存储运行变量）、程序存储器FLASH（存储程序代码）等。DMA可以提供 <u>外设寄存器和存储器</u> 或者 <u>存储器和存储器之间</u> 的高速数据传输，无须CPU干预，节省了CPU的资源。翻译成人话就是，是一个数据转运小助手，主要用来协助CPU完成数据转运的工作。下面是stm32中DMA的一些配置：
> - stm32系列芯片共有12个**独立**可配置的通道：DMA1（7个通道），DMA2（5个通道）。
> - 每个通道都支持软件触发和**特定的**硬件触发。
> > - **软件触发**应用场景：数据源中的数据已确定。如将FLASH中的数据转运到SRAM中（存储器-->存储器），一次触发后会将数据以最快的速度全部转运完毕。
> > - **硬件触发**应用场景：数据源中的数据没有全部确定，需要在特定的时机转运数据。如将ADC数据转运到存储器（外设寄存器-->存储器），等对应ADC通道的数据转换完成后，才硬件触发一次DMA转运数据。
> - STM32F103C8T6型号的DMA资源：**DMA1（7个通道）**。

下面介绍“存储器映像”。计算机有五大组成部分：运算器、控制器、存储器、输入设备、输出设备。计算机的核心关键部分就是CPU和存储器，上述的运算器和控制器会合在一起组成CPU，而存储器主要关心**存储器的内容和地址**。下图给出了stm32中都有哪些存储器，以及这些存储器的地址都是什么，即 **“存储器映像”**，下表则对下图进行了一个简单的总结：

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/8-01%E5%AD%98%E5%82%A8%E5%99%A8%E6%98%A0%E5%83%8F.png" width="70%">
</div><div align=center>
图8-1 存储器映像——来自数据手册“4 存储器映像”
</div>

<div align=center>
表8-1 stm32存储器映像——总结自“STM32F103x8B数据手册”
</div><div align=center>
<style type="text/css">
.tg  {border-collapse:collapse;border-spacing:0;}
.tg td{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  overflow:hidden;padding:10px 5px;word-break:normal;}
.tg th{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  font-weight:normal;overflow:hidden;padding:10px 5px;word-break:normal;}
.tg .tg-9fp1{color:#000000;font-family:inherit;font-weight:bold;text-align:center;vertical-align:middle}
.tg .tg-0so2{font-family:inherit;text-align:center;vertical-align:middle}
</style>
<table class="tg">
<thead>
  <tr>
    <th class="tg-9fp1"><span style="font-weight:bold">类型</span></th>
    <th class="tg-9fp1"><span style="font-weight:bold">起始地址</span></th>
    <th class="tg-9fp1"><span style="font-weight:bold">存储器</span></th>
    <th class="tg-9fp1"><span style="font-weight:bold">用途</span></th>
  </tr>
</thead>
<tbody>
  <tr>
    <td class="tg-0so2" rowspan="3"><span style="color:#000">ROM</span></td>
    <td class="tg-0so2"><span style="color:#000">0x0800 0000</span></td>
    <td class="tg-0so2"><span style="color:#000">程序存储器Flash</span></td>
    <td class="tg-0so2"><span style="color:#000">存储C语言编译后的程序代码</span></td>
  </tr>
  <tr>
    <td class="tg-0so2"><span style="color:#000">0x1FFF F000</span></td>
    <td class="tg-0so2"><span style="color:#000">系统存储器</span></td>
    <td class="tg-0so2"><span style="color:#000">存储BootLoader，用于串口下载</span></td>
  </tr>
  <tr>
    <td class="tg-0so2"><span style="color:#000">0x1FFF F800</span></td>
    <td class="tg-0so2"><span style="color:#000">选项字节</span></td>
    <td class="tg-0so2"><span style="color:#000">存储一些独立于程序代码的配置参数</span></td>
  </tr>
  <tr>
    <td class="tg-0so2" rowspan="3"><span style="color:#000">RAM</span></td>
    <td class="tg-0so2"><span style="color:#000">0x2000 0000</span></td>
    <td class="tg-0so2"><span style="color:#000">运行内存SRAM</span></td>
    <td class="tg-0so2"><span style="color:#000">存储运行过程中的临时变量</span></td>
  </tr>
  <tr>
    <td class="tg-0so2"><span style="color:#000">0x4000 0000</span></td>
    <td class="tg-0so2"><span style="color:#000">外设寄存器</span></td>
    <td class="tg-0so2"><span style="color:#000">存储各个外设的配置参数</span></td>
  </tr>
  <tr>
    <td class="tg-0so2"><span style="color:#000">0xE000 0000</span></td>
    <td class="tg-0so2"><span style="color:#000">内核外设寄存器</span></td>
    <td class="tg-0so2"><span style="color:#000">存储内核各个外设的配置参数</span></td>
  </tr>
</tbody>
</table>
</div>

> - 程序存储器FLASH：下载程序的位置，程序一般也是从主闪存里开始运行。若某变量地址为0x0800_xxxx，那么它就是属于主闪存的数据。
> - 系统存储器：存储BootLoader程序（俗称“刷机”），芯片出厂时自动写入，一般不允许修改。
> - 选项字节：存储的主要是FLASH的读保护、写保护、看门狗等配置。下载程序可以不刷新选项字节的内容，从而保持相应配置不变。
> - 运行内存SRAM：在程序中定义变量、数组、结构体的地方，类似于电脑的内存条。
> - 外设寄存器：初始化各种外设的过程中，最终所读写的寄存器就属于这个区域。
> - 内核外设寄存器：就是NVIC和SysTick。由于不是同一个厂家设计，所以专门留出来<u>内核外设</u>的地址空间，和<u>其他外设</u>的地址空间不一样。
>
> 注：由于stm32是32位的系统，所以寻址空间最大可达4GB（每个地址都代表1Byte），而stm32的存储器硬件最多也就是KB级别的，所以实际上4GB的寻址空间使用率远远低于1%。
> 注：上表中前三者存储介质都是FLASH，但是一般说“FLASH”就是代指“主闪存FLASH”，而不是另外两块区域。
> 注：上表中后三者存储介质也是SRAM，但是一般将“SRAM”就是代指“运行内存”，“外设寄存器”就直接叫“寄存器”。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/8-02DMA%E6%A1%86%E5%9B%BE.png" width="60%">
</div><div align=center>
图8-2 DMA电路框图
</div>

> 将从以下加粗的四大部分介绍DMA的电路结构。
> - **总线矩阵：** 为了高效有条理的访问存储器，设置了总线矩阵。<u>左端是主动单元</u>，拥有存储器的访问权；<u>右端是被动单元</u>，它们的存储器只能被左端的主动单元读写。
> > - 总线矩阵内部的仲裁器：如果DMA和CPU都要访问同一个目标，那么DMA就会暂停CPU的访问，以防止冲突。但是总线仲裁器仍然会保证CPU得到一半的总线带宽，以确保CPU正常工作。
> - **主动单元：**
> 1. Cortex-M3核心（左上角）：包含了CPU和内核外设。<u>剩下的所有东西都可以看成是存储器</u>，比如Flash是主闪存、SRAM是运行内存、各种外设寄存器也都可以看成是一种SRAM存储器。
> 2. ICode总线：指令总线。加载程序指令。
> 3. DCode总线：数据总线，专门用来访问Flash。
> 4. 系统总线：是访问其他东西的。
> 5. DMA总线：用于访问各个存储器，包括DMA1总线（7个通道）、DMA2总线（5通道）、以太网外设的私有DMA总线。由于DMA要转运数据，所以**DMA也有访问的主动权**。
> 6. DMA1、DMA2：各个通道可以分别设置转运数据的源地址和目的地址，所以**各个通道可以独立的进行数据转运**。
> > > - 仲裁器：调度各个通道，防止产生冲突。虽然多个通道可以独立地转运数据，但是DMA总线只有一条，所以所有的通道都只能 **分时复用** 这一条DMA总线，若通道间产生冲突，就会由仲裁器根据通道的优先级决定使用顺序。
> > > - AHB从设备：用于配置DMA参数，也就是DMA自身的寄存器。DMA的外设配置寄存器直接连接在了被动单元侧的<u>AHB总线</u>上。所以DMA既是总线矩阵上的主动单元，可以读写各种寄存器；同时也是AHB总线上的被动单元。CPU配置DMA的线路：“系统”总线-->总线矩阵-->AHB总线-->DMA中的AHB从设备。
> - **被动单元：**
> 1. Flash：主闪存，只读存储器。若直接通过总线访问(无论是CPU还是DMA)，都只能读取数据而不能写入。若DMA的目的地址为FLASH区域，那么转运就会出错。要想对Flash写入，可以通过“Flash接口控制器”。
> 2. SRAM：运行内存，通过总线可以任意读写。
> 3. 各种外设寄存器（右侧两个方框）：需要对比参考手册中的描述，这些寄存器的类型可能为 只读/只写/读写。不过日常主要使用的数据寄存器，都是可以正常读写的。
> - **DMA请求：** 用于硬件触发DMA的数据转运。“请求”就是“触发”的意思，此线路右侧的触发源是各个外设，所以这个“DMA请求”就是 **DMA的硬件触发源**，如ADC转换完成、串口接收到数据等信号。
>


从上面DMA的电路介绍中，不难看出**寄存器是一种特殊的存储器**，寄存器的两大作用：
> 1. 存储数据。被CPU或DMA等读写，就像读写运行内存一样。
> 2. 控制电路。寄存器的每一位都接了一个导线，可以用于控制外设电路的状态，比如置引脚的高低电平、导通或断开开关、切换数据选择器，或者多位结合起来当做计数器、数据寄存器等。

所以寄存器是连接软件和硬件的桥梁，<u>软件读写寄存器，就相当于在控制硬件的执行</u>。既然外设相当于寄存器，寄存器又是存储器，那么<u>使用DMA转运数据，本质上就是从某个地址取数据，再放到另一个地址去。</u>


<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/8-03DMA%E5%9F%BA%E6%9C%AC%E7%BB%93%E6%9E%84.png" width="70%">
</div><div align=center>
图8-3 DMA基本结构
</div>

> 前面的“DMA电路框图”只是一个笼统的结构图，没有展现出对于DMA内部的执行细节。而上图“DMA基本结构”则可用于代码编写时的思路参考，以实现控制DMA工作。
> - 外设寄存器站点、存储器站点（Flash和SRAM）：数据转运的两大站点。“外设站点”的参数不一定是外设，“存储器站点”的参数也不一定是存储器。这两个站点的名字只是图一乐，真正表示的意思就是发送端和接收端的参数。可以看到图中由 **三类数据转运线路**。注意Falsh一般只读，所以不存在 “SRAM到Flash”或“Falsh到Flash”的线路。
> > 注：虽然名字“图一乐”，但在stm32手册中，“存储器”一般特指“Flash”和“SRAM”；“外设”一般特指“外设寄存器”。
> 
> 下面的7个参数都属于DMA的初始化结构体：
> 1. 方向：指明“外设寄存器站点”是发送端还是接收端。
> - 站点参数：
> > 2. 起始地址：配合“方向”的设置，两个起始地址指明了发送端地址、接收端地址。
> > 3. 数据宽度：指定一次转运的数据宽度，可选字节Byte(8位)、半字HalfWord(16位)、字Word(32位)。比如ADC转换的数据位宽是16位，就需要选择数据宽度为“半字”。
> > 4. 地址是否自增：一次转运完成后，下一次转运是否地址自增。如ADC扫描模式使用DMA进行数据转运，数据源是ADC_DR寄存器，显然不需要地址自增；而数据目的地是存储器，就需要地址自增，以防止数据覆盖。
> 5. 传输计数器：是一种自减计数器，指定总共需要转运几次。转运结束后，之前自增的地址也会恢复成起始地址，以方便新一轮的转换。**参考手册规定，写“传输计数器”时必须使用“开关控制”关闭DMA。**
> 6. 自动重装器：传输计数器自减到0后，是否重装到最初的值继续转运，也就决定了两种转运模式：单次模式（不重装）、循环模式（重装）。例如想转运一个数组，就是单次模式；如果想转运ADC扫描模式+连续转换，此时DMA就需要循环模式。
> 7. M2M(Memory to Memory)：DMA的触发源选择器，设置为1选择软件触发、设置为0选择硬件触发。
> > - DMA的软件触发(连续触发)：和之前不同，DMA的软件触发无需手动触发。选择软件触发(M2M=1)后，若使能DMA，那么DMA会自动以最快的速度连续触发，尽快完成本轮转换。**软件触发不能和循环模式同时使用**，防止DMA停不下来。<u>软件触发常用于存储器到存储器的转运。</u>
> > - DMA的硬件触发(单次触发、使用更多)：硬件触发源可以选择ADC、串口、定时器等，由相应的外设库函数使能对应的硬件触发源。一般都是<u>与外设有关的转运使用硬件触发</u>，这些转运都需要特定的时机，如ADC转换完成、串口收到数据、定时时间到等。
> 
> - 开关控制：即```DMA_Cmd```函数，用于使能DMA。
>
> 总结：DMA转运的必须条件：开关控制使能、传输计数器大于零、有触发源。

下面再看**两个细节和两个例子**：

**细节1：DMA请求**
首先进一步介绍“DMA基本结构”中的硬件触发源——“DMA请求”：
<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/8-04DMA1%E8%AF%B7%E6%B1%82%E6%98%A0%E5%83%8F.png" width="55%">
</div><div align=center>
图8-4 DMA1请求映像
</div>

> 上图是DMA1的请求映像，所以有7个通道，每个通道都有一个数据选择器，可以选择硬件触发/软件触发，
> - EN位：其实就是DMA的开关控制。
> - 软件触发需要M2M位置1，硬件触发则需要M2M位置0。
> - 硬件触发源：**每个通道的硬件触发源各有不同**。如ADC1触发必须选择通道1、TIM2更新时间触发必须选择通道2…… **通道的选择由相应的外设库函数决定**，如ADC库函数```ADC_DMACmd```用于开启通道1、TIM库函数```TIM_DMACmd```可以开启通道2(TIM2_UP)。理论上可以同时开启同一通道的多个触发源，但一般只开启一个。
> - 软件触发通道可以任意选择。
> - 



**细节2：数据宽度与对齐**
前面提到数据发送端和接收端都可以设置数据宽度，若数据宽度设置相同显然就是正常转运，那如果发送端和接收端设置不同的数据宽度，会发生什么情况呢？见下图：
<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/8-05%E6%95%B0%E6%8D%AE%E5%AE%BD%E5%BA%A6%E4%B8%8E%E5%AF%B9%E9%BD%90%E8%AF%B4%E6%98%8E.png" width="60%">
</div><div align=center>
图8-5 数据宽度与对齐说明
</div>

> 基本原则就是：
> - 源端宽度<目的宽度：在目的宽度的高位补零。
> - 源端宽度>目的宽度：按照目的宽度，只保留源端的低位，多余的高位全部舍弃。
> 
> 上面的过程类似于```uint8_t```、```uint16_t```、```uint32_t```之间的相互赋值，不够就补零，超了就舍弃高位。


**例1：DMA数据转运**
任务是将SRAM数组DataA转运到另一个SRAM数组DataB（存储器到存储器）。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/8-06%E6%95%B0%E6%8D%AE%E8%BD%AC%E8%BF%90%2BDMA.png" width="45%">
</div><div align=center>
图8-6 “数据转运+DMA”示意图
</div>

> 下面给出各参数的配置说明：
> - 两个站点的参数：外设地址->DataA数组首地址、存储器地址->DataB数组首地址；数据宽度都是8位；为保证数据的一一对应，“外设”和“存储器”都设置为地址自增。
> - “方向”参数：默认是“外设”-->“存储器”，当然也可以将方向反过来。
> - 传输计数器：数组大小为7，所以计数器为7。
> - 自动重装：不需要。
> - 触发源：软件触发。由于是“存储器->存储器”的触发，所以不需要等待转运时机。
> - 开关控制：最后调用```DMA_Cmd```开启DMA转运。
>
> 注：上述为“**复制转运**”，转运完成后DataA数据不会消失。


**例2：ADC扫描模式+DMA**
期望将外设ADC多通道（扫描模式）的数据，依次搬运到SRAM中的ADValue数组中（外设到存储器）。
注意下图左侧给出了ADC的扫描模式示意图，ADC每触发一次，7个通道依次进行ADC数据转换，每个通道转换完成时，都会将转换结果放到ADC_DR数据寄存器中，也就是**ADC的所有通道共用一个ADC数据寄存器**。所以每个通道转换完成后，都需要DMA立即转运一次，防止新来的通道数据将之前的通道数据覆盖掉。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/8-07ADC%E6%89%AB%E6%8F%8F%E6%A8%A1%E5%BC%8F%2BDMA.png" width="70%">
</div><div align=center>
图8-7 “ADC扫描模式+DMA”示意图
</div>

> 下面给出各参数的配置说明：
> - 起始地址：“外设站点”地址设置为ADC_DR寄存器的地址；“存储器站点”地址为ADValue(SRAM)的首地址。
> - 数据宽度：“外设站点”和“存储器站点”都设置为16位(HalfWord)。
> - 地址自增：“外设站点”不自增，“存储器站点”自增。
> - 方向：“外设”站点为发送端。
> - 传输计数器：按照ADC需要扫描的通道数来，即为7。
> - 计数器是否自动重装：ADC单次扫描，可以不自动重装；ADC连续扫描，DMA就使用自动重装，此时ADC启动下一轮转换，DMA同时也启动下一轮转运，可以实现同步工作。
> - 触发选择：选择硬件触发——ADC单通道转换完成。虽然前面说过，只有当所有通道都转换完成后，才会触发转换完成EOC标志，其余时间没有任何中断/标志，但实际上，**硬件中保留了单个通道针对DMA的请求**（虽然参考手册只字不提）。

一般来说，<u>DMA最常见的用途就是配合ADC的扫描模式</u>。因为ADC的扫描模式有数据覆盖的特征，或者说这个数据覆盖的问题是ADC固有的缺陷，这个缺陷使得ADC和DMA成为了最常见的伙伴。ADC对DMA的需求非常强烈，其他外设使用DMA可以提高效率，属于是锦上添花的操作，不使用DMA顶多只是损失一些性能；但是ADC的扫描模式如果不使用DMA，功能都会受到很大的限制。所以**ADC和DMA的结合最为常见**。

更多关于DMA的详细内容可以查阅参考手册“2 存储器和总线构架”、“10 DMA控制器(DMA)”。











## 8.2 实验：DMA数据转运-存储器到存储器
需求：使用DMA，进行存储器到存储器的数据转运。

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/8-08%E6%8E%A5%E7%BA%BF%E5%9B%BE-DMA%E6%95%B0%E6%8D%AE%E8%BD%AC%E8%BF%90.png" width="70%">
</div><div align=center>
图8-8 “DMA数据转运”-接线图
</div>

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/8-09%E4%BB%A3%E7%A0%81%E8%B0%83%E7%94%A8-DMA%E6%95%B0%E6%8D%AE%E8%BD%AC%E8%BF%90.png" width="25%">
</div><div align=center>
图8-9 “DMA数据转运”-代码调用（非库函数）
</div>

代码展示：
**- main.c**
```c
#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "DMA_User.h"
#include "Delay.h"

uint8_t DataA[] = {0x11,0x22,0x33,0x44};//源端数组
uint8_t DataB[] = {0x00,0x00,0x00,0x00};//目的端数组

    
int main(void){
    //OLED显示屏初始化
    OLED_Init();
    OLED_ShowString(1,1,"DataA:");
    OLED_ShowHexNum(1,7,(uint32_t)DataA,8);
    OLED_ShowHexNum(2,1,DataA[0],2);
    OLED_ShowHexNum(2,4,DataA[1],2);
    OLED_ShowHexNum(2,7,DataA[2],2);
    OLED_ShowHexNum(2,10,DataA[3],2);
    OLED_ShowString(3,1,"DataB:");
    OLED_ShowHexNum(3,7,(uint32_t)DataB,8);
    OLED_ShowHexNum(4,1,DataB[0],2);
    OLED_ShowHexNum(4,4,DataB[1],2);
    OLED_ShowHexNum(4,7,DataB[2],2);
    OLED_ShowHexNum(4,10,DataB[3],2);
    
//    //验证存储器映像-注意srm32中地址都是32位的
//    uint8_t aa = 0x66;//存储在运行内存SRAM中
//    const uint8_t bb = 0x55;//存储在Flash中
//    OLED_ShowHexNum(1,1,aa,2);
//    OLED_ShowHexNum(1,4,(uint32_t)&aa,8);//SRAM地址0x2000开头
//    OLED_ShowHexNum(2,1,bb,2);
//    OLED_ShowHexNum(2,4,(uint32_t)&bb,8);//Flash地址0x0800开头
//    OLED_ShowHexNum(3,1,(uint32_t)&ADC1->DR,8);//ADC外设寄存器地址
    
    //DMA 初始化
    DMA_User_Init((uint32_t)&DataA, (uint32_t)&DataB, 4);
    
    while(1){
        //改变数据并显示
        DataA[0]++;
        DataA[1]++;
        DataA[2]++;
        DataA[3]++;
        OLED_ShowHexNum(2,1,DataA[0],2);
        OLED_ShowHexNum(2,4,DataA[1],2);
        OLED_ShowHexNum(2,7,DataA[2],2);
        OLED_ShowHexNum(2,10,DataA[3],2);
        OLED_ShowHexNum(4,1,DataB[0],2);
        OLED_ShowHexNum(4,4,DataB[1],2);
        OLED_ShowHexNum(4,7,DataB[2],2);
        OLED_ShowHexNum(4,10,DataB[3],2);
        Delay_ms(1000);
        
        //转运数据并显示
        DMA_User_Transfer();
        OLED_ShowHexNum(2,1,DataA[0],2);
        OLED_ShowHexNum(2,4,DataA[1],2);
        OLED_ShowHexNum(2,7,DataA[2],2);
        OLED_ShowHexNum(2,10,DataA[3],2);
        OLED_ShowHexNum(4,1,DataB[0],2);
        OLED_ShowHexNum(4,4,DataB[1],2);
        OLED_ShowHexNum(4,7,DataB[2],2);
        OLED_ShowHexNum(4,10,DataB[3],2);
        Delay_ms(1000);
    };
}

```

**- DMA_User.h**
```c
#ifndef __DMA_USER_H
#define __DMA_USER_H

void DMA_User_Init(uint32_t AddrA, uint32_t AddrB, uint16_t BuffSize);
void DMA_User_Transfer(void);

#endif

```

**- DMA_User.c**
```c
#include "stm32f10x.h"                  // Device header

uint16_t DMA_User_BuffSize;//传输计数器

//DMA初始化-DMA1_Channel1-从AddrA到AddrB转运，不使能
void DMA_User_Init(uint32_t AddrA, uint32_t AddrB, uint16_t BuffSize){
    
    DMA_User_BuffSize = BuffSize;
    
    //1.开启RCC
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    //2.初始化DMA
    DMA_InitTypeDef DMA_InitStructure;
    DMA_StructInit(&DMA_InitStructure);
    DMA_InitStructure.DMA_PeripheralBaseAddr = AddrA;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Enable;
    DMA_InitStructure.DMA_MemoryBaseAddr     = AddrB;
    DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_BufferSize         = BuffSize;//传输计数器
    DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralSRC;//方向：外设作为源端
    DMA_InitStructure.DMA_M2M                = DMA_M2M_Enable;//软件触发
    DMA_InitStructure.DMA_Mode               = DMA_Mode_Normal;//是否使用自动重装
    DMA_InitStructure.DMA_Priority           = DMA_Priority_Medium;//DMA多通道才会用到
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);
    //3.开关控制使能
    DMA_Cmd(DMA1_Channel1, DISABLE);
    //4.开启硬件触发源（按需求选做）
}

//手动触发一次DMA的转运
void DMA_User_Transfer(void){
    //触发转运
    DMA_Cmd(DMA1_Channel1, DISABLE);
    DMA_SetCurrDataCounter(DMA1_Channel1,DMA_User_BuffSize);
    DMA_Cmd(DMA1_Channel1, ENABLE);
    //判断转运完成，并清除相应标志位
    while(DMA_GetFlagStatus(DMA1_FLAG_TC1)==RESET);
    DMA_ClearFlag(DMA1_FLAG_TC1);
    DMA_Cmd(DMA1_Channel1, DISABLE);
}

```

编程感想：
> 1. DMA不涉及外围电路，所以直接添加在System文件夹中，注意库函数中已经包含了```DMA.h```、```DMA.c```，所以不要重名。
> 2. DMA初始化参数。外设的三个参数和存储器的三个参数名称居然不一样！！！！！！淦。
> 3. 变量地址。对一个变量取地址之后，会存放在一个指针变量里，要想调用OLED显示屏函数，还要进行强制类型转换。若不加强制类型转换，就是指针跨级赋值，编译会报错。注意变量的地址都是由编译器决定的，所以并不固定。
> 3. ```const```变量。stm32f103c8t6中拥有64KB的Flash、20KB的SRAM，所以有一些不需要更改但又很大的数组，就可以定义为```const```常量（如字模库等），存储在Flash中，减小SRAM空间占用（防止变成“栈溢出工程师”，哈哈哈哈）。
> 4. 访问外设寄存器。可以用结构体方便的访问外设的寄存器，比如ADC的数据寄存器就是```ADC1->DR```。
> 5. 库函数中对于地址的定义。介绍一个比较巧妙的点。在代码中，右键```ADC1->DR```中的“ADC1”，跳转到地址定义。可以看到其定义了一个基地址，并将其强制转换成一个结构体指针```(ADC_TypeDef *)```。右键跳转进这个结构体，可以发现里面的变量定义顺序与参考手册“11.12.15 ADC寄存器地址映像”中的顺序完全一致，于是便**巧妙地利用结构体的顺序定义出“偏移量”**。当然上述这一套看起来可能比较麻烦，也可以采用下面这个方法：
```c
#define ADC1_DR (uint32_t *)0x4001244C //定义变量的寄存器地址
*ADC1_DR//然后就可以直接在函数中获取到ADC1的数据寄存器的值了
```












## 8.3 实验：DMA+AD多通道-外设到存储器
需求：用ADC的扫描模式完成多通道采集，然后使用DMA完成外设到存储器的数据转运。（与上一节“ADC多通道”现象一致）

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/8-10%E6%8E%A5%E7%BA%BF%E5%9B%BE-DMA%2BAD%E5%A4%9A%E9%80%9A%E9%81%93.png" width="70%">
</div><div align=center>
图8-10 “DMA+AD多通道”-接线图
</div>

<div align=center>
<img src="https://raw.githubusercontent.com/jjejdhhd/Git_img2023/main/STM32F103_JKD/8-11%E4%BB%A3%E7%A0%81%E8%B0%83%E7%94%A8-DMA%2BAD%E5%A4%9A%E9%80%9A%E9%81%93.png" width="25%">
</div><div align=center>
图8-11 “DMA+AD多通道”-代码调用（非库函数）
</div>

代码展示：
**- main.c**
```c
#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "ADC_User.h"
    
int main(void){
    uint16_t ADC_value[4] = {0,0,0,0};
    //OLED显示屏初始化
    OLED_Init();
    OLED_ShowString(1,1,"C1:+0.00V");
    OLED_ShowString(2,1,"C2:+0.00V");
    OLED_ShowString(3,1,"C3:+0.00V");
    OLED_ShowString(4,1,"C4:+0.00V");
    
    //ADC扫描模式初始化
    ADC_User_InitMuti((uint32_t)&ADC_value,4);
    ADC_User_Start();
    
    while(1){    
        OLED_ShowFloat(1,4,(float)ADC_value[0]*3.3/4095,1,2);
        OLED_ShowFloat(2,4,(float)ADC_value[1]*3.3/4095,1,2);
        OLED_ShowFloat(3,4,(float)ADC_value[2]*3.3/4095,1,2);
        OLED_ShowFloat(4,4,(float)ADC_value[3]*3.3/4095,1,2);
    };
}

```

**- ADC_User.c新增函数**
```c
//ADC多通道初始化-ADC1的通道0~3-PA0~PA3共四个通道
void ADC_User_InitMuti(uint32_t AddrB, uint16_t BuffSize){
    //1.开启外设时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);//6分频使得ADC时钟为12MHz
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    //2.配置GPIO
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AIN;//模拟输入
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    //3.配置ADC多路开关，选择通道进入规则组
    ADC_RegularChannelConfig(ADC1,ADC_Channel_0,1,ADC_SampleTime_1Cycles5);
    ADC_RegularChannelConfig(ADC1,ADC_Channel_1,2,ADC_SampleTime_1Cycles5);
    ADC_RegularChannelConfig(ADC1,ADC_Channel_2,3,ADC_SampleTime_1Cycles5);
    ADC_RegularChannelConfig(ADC1,ADC_Channel_3,4,ADC_SampleTime_1Cycles5);
    //4.配置ADC转换器
    ADC_InitTypeDef ADC_InitStructure;
    ADC_InitStructure.ADC_DataAlign          = ADC_DataAlign_Right;//数据右对齐
    ADC_InitStructure.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None;//不使用外部触发（软件触发）
    ADC_InitStructure.ADC_Mode               = ADC_Mode_Independent;//独立模式
    ADC_InitStructure.ADC_NbrOfChannel       = BuffSize;//通道总数（非扫描模式，此参数不起作用）
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;//连续转换
    ADC_InitStructure.ADC_ScanConvMode       = ENABLE;//扫描模式
    ADC_Init(ADC1, &ADC_InitStructure);
    //5.配置ADC开关控制
    ADC_Cmd(ADC1, ENABLE);
    //6.进行ADC校准
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1)==SET);
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1)==SET);
    //7.开启ADC1硬件触发源
    ADC_DMACmd(ADC1, ENABLE);
    //8.配置DMA
    DMA_InitTypeDef DMA_InitStructure;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryBaseAddr     = AddrB;//(uint32_t)ADC_Value;
    DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_BufferSize         = BuffSize;//传输计数器
    DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralSRC;//方向：外设作为源端
    DMA_InitStructure.DMA_M2M                = DMA_M2M_Disable;//硬件触发
    DMA_InitStructure.DMA_Mode               = DMA_Mode_Circular;//是否使用自动重装
    DMA_InitStructure.DMA_Priority           = DMA_Priority_Medium;//DMA多通道才会用到
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);
    //9.DMA开关控制使能
    DMA_Cmd(DMA1_Channel1, ENABLE);

}
//记得在头文件中声明
```


编程感想：
> 1. 硬件自动化。传统上CPU控制外设的思路是，由CPU控制所有外设的运行或停止。但通过上面的编程可以发现，当所有的外设初始化完成后，就不需要额外的软件资源，外设之间完全可以相互配合实现自动化工作，这是stm32的一大特色。这样不仅可以减轻CPU负担，还可以大大提升外设的性能。
> 2. 卡了很久的Bug，读取到的ADC转换数据都很小。传感器要将AO模拟输出接到GPIO上，而不是DO数字输出！
> 3. 卡了很久的Bug，读取到的数据都一样，且变化也相同。DMA的M2M参数选择为“硬件触发”。
> 4. 卡了很久的Bug，读取的ADC通道数和预期的错位（比如应该是通道1的电位器数据，却出现在了通道4上）。这是因为在DMA初始化之前就对ADC进行了软件触发。所以结论就是，不管ADC、DMA的初始化的顺序如何，最关键的一步 **“ADC软件触发”一定要等ADC、DMA初始化都完成后再进行**，也就是一定要放在最后！！
> 5. 关于连续触发。编程时有两种思路，一种是直接ADC+DMA初始化完成，给一个ADC的软件触发信号，就可以直接啥都不用管直接在```while```中显示；另一种是每次进行ADC转换之前，都要进行一次软件触发。值得注意的是，这两种思路和DMA的配置没有任何关系！毕竟ADC的通道转换完成信号是DMA的硬件触发源，所以DMA要设置成自动重装、硬件触发模式就OK。ADC也必须是扫描模式（毕竟多通道），但至于是否 ADC连续转换 就可以看个人的喜好了。
> 6. 关于代码架构。本来是想在ADC和DMA两个文件中，分别进行ADC和DMA的初始化。但其实想想，ADC的多通道模式一般就是搭配DMA，而DAM自己再进行一个专门的ADC初始化函数就显得多余，所以直接将DMA的初始化放在ADC多通道模式的初始化函数当中，那是一点问题都没有。而且还能使代码调用关系更加简洁明了。

