#include "led.h"

 /**
  * @file   LED_GPIO_Config
  * @brief  LED灯引脚配置
  * @param  无
  * @retval 无
  */
void LED_GPIO_Config(void)
{	
    //定义一个GPIO_InitTypeDef 类型的结构体
    GPIO_InitTypeDef  GPIO_InitStructure;	
    RCC_APB2PeriphClockCmd(LED1_GPIO_RCC,ENABLE);//使能GPIO的外设时钟
   
    GPIO_InitStructure.GPIO_Pin =LED1_GPIO_PIN;//选择要用的GPIO引脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //设置引脚模式为推免输出模式						 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//设置引脚速度为50MHZ         
    GPIO_Init(LED1_GPIO_PORT, &GPIO_InitStructure);//调用库函数，初始化GPIO
}

 /**
  * @file   LEDXToggle
  * @brief  LED亮灭翻转
  * @param  ledx:1--PC13;2--PB0;1--PB1
  * @retval 无
  */
void LEDXToggle(void)
{
    LED1_GPIO_PORT->ODR^=LED1_GPIO_PIN;
}

