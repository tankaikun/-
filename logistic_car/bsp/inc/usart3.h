#ifndef __USART3_H
#define __USART3_H

#include <stm32f10x.h>

/* GPS接口配置 使用不同的串口时，要修改对应的接口 */
#define USART3_DR_Base            (0x40004800+0x04)		// 串口3的数据寄存器地址

#define GPS_DATA_ADDR             USART3_DR_Base        //GPS使用的串口的数据寄存器地址
#define GPS_RBUFF_SIZE            2000                   //串口接收缓冲区大小

/* 外设 */
#define GPS_USART                    USART3
#define GPS_DMA                      DMA1
#define GPS_DMA_CLK                  RCC_AHBPeriph_DMA1
#define GPS_DMA_CHANNEL              DMA1_Channel3
#define GPS_DMA_IRQn                 DMA1_Channel3_IRQn         //GPS中断源

/* 外设标志 */
#define GPS_DMA_FLAG_TC              DMA1_FLAG_TC3
#define GPS_DMA_FLAG_TE              DMA1_FLAG_TE3
#define GPS_DMA_FLAG_HT              DMA1_FLAG_HT3
#define GPS_DMA_FLAG_GL              DMA1_FLAG_GL3
#define GPS_DMA_IT_HT                DMA1_IT_HT3
#define GPS_DMA_IT_TC                DMA1_IT_TC3

// 函数声明
void gps_usart3_config(void);


//extern int flag_gps = 0;

#endif /*  */


