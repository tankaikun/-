#ifndef __USART3_H
#define __USART3_H

#include <stm32f10x.h>

/* GPS�ӿ����� ʹ�ò�ͬ�Ĵ���ʱ��Ҫ�޸Ķ�Ӧ�Ľӿ� */
#define USART3_DR_Base            (0x40004800+0x04)		// ����3�����ݼĴ�����ַ

#define GPS_DATA_ADDR             USART3_DR_Base        //GPSʹ�õĴ��ڵ����ݼĴ�����ַ
#define GPS_RBUFF_SIZE            2000                   //���ڽ��ջ�������С

/* ���� */
#define GPS_USART                    USART3
#define GPS_DMA                      DMA1
#define GPS_DMA_CLK                  RCC_AHBPeriph_DMA1
#define GPS_DMA_CHANNEL              DMA1_Channel3
#define GPS_DMA_IRQn                 DMA1_Channel3_IRQn         //GPS�ж�Դ

/* �����־ */
#define GPS_DMA_FLAG_TC              DMA1_FLAG_TC3
#define GPS_DMA_FLAG_TE              DMA1_FLAG_TE3
#define GPS_DMA_FLAG_HT              DMA1_FLAG_HT3
#define GPS_DMA_FLAG_GL              DMA1_FLAG_GL3
#define GPS_DMA_IT_HT                DMA1_IT_HT3
#define GPS_DMA_IT_TC                DMA1_IT_TC3

// ��������
void gps_usart3_config(void);


//extern int flag_gps = 0;

#endif /*  */


