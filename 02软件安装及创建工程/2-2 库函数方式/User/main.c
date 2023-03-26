#include "stm32f10x.h"                  // Device header

int main(void){
  //1.开启GPIOC的外设时钟
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
  //2.配置寄存器PC13的端口模式
  // 2.1首先配置GPIO结构体
  GPIO_InitTypeDef GPIO_InitStructure;//给结构体起名字
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP; //寄存器模式为通用推挽输出
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13;      //寄存器引脚为13
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //输出速度为50MHz
  // 2.2然后才能调用函数配置寄存器
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  //3.配置数据寄存器PC13的输出
  GPIO_SetBits(GPIOC, GPIO_Pin_13);   //将PC13设置为高电平，LED灭
//  GPIO_ResetBits(GPIOC, GPIO_Pin_13); //将PC13设置为低电平，LED亮
  while(1){}
}
