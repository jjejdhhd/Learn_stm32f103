#include "stm32f10x.h"                  // Device header

int main(void){
  //配置RCC寄存器，使能GPIOC的时钟。GPIO都属于APB2外设
  RCC->APB2ENR = 0x00000010;
  //配置寄存器PC13：通用推挽输出模式、输出模式50MHz
  GPIOC->CRH = 0x00300000;
  //输出数据寄存器PC13：
//  GPIOC->ODR = 0x00002000;//LED灭
  GPIOC->ODR = 0x00000000;//LED亮
  while(1){}
}
