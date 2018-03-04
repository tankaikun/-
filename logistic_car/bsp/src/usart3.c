
#include "usart3.h"
#include  "ucos_ii.h"
#include "bsp.h"
#include "stdio.h"

int flag_gps = 0;

/* DMA���ջ���  */
uint8_t gps_rbuff[GPS_RBUFF_SIZE];


static void usart3_init(int bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//ʹ��USART1��GPIOAʱ��
 	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);	

	//USART3_TX   GPIOB.10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
	GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIOA.9 
	//USART3_RX	  GPIOB.11
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//PA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIOA.10  
	
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);							//����ָ���Ĳ�����ʼ��VIC�Ĵ���

	USART_InitStructure.USART_BaudRate = bound;						//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;		//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;			//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;				//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

	USART_Init(USART3, &USART_InitStructure); 				
	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);					// ���ڿ����ж�	
	USART_ITConfig(USART3,USART_IT_TC,DISABLE); 
	USART_ITConfig(USART3,USART_IT_RXNE,DISABLE); 
	USART_Cmd(USART3, ENABLE);                    			
}

static void gps_dma_config(void)
{
	//������DMA����    
	DMA_InitTypeDef DMA_InitStructure;
    
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);			//����DMAʱ��  
    
    DMA_DeInit(DMA1_Channel3);									//DMA1ͨ��3����  
    
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART3->DR);			//�����ַ 
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)gps_rbuff;					//�ڴ��ַ  
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;							//dma���䷽����  
    DMA_InitStructure.DMA_BufferSize = GPS_RBUFF_SIZE;							//����DMA�ڴ���ʱ�������ĳ���  
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;			//����DMA���������ģʽ��һ������    
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;						//����DMA���ڴ����ģʽ 
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;		//���������ֳ� 
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;				//�ڴ������ֳ�  
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;								//����DMA�Ĵ���ģʽ  
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;						//����DMA�����ȼ���  
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;								//����DMA��2��memory�еı����������  
    DMA_Init(DMA1_Channel3,&DMA_InitStructure);  
  
    DMA_Cmd(DMA1_Channel3,ENABLE);  //ʹ��ͨ��3  
	USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);
}

void gps_usart3_config(void)
{
  usart3_init(38400);
  gps_dma_config();  
}


// USART3�жϷ�����
void USART3_IRQHandler(void)          
{
	uint8_t res;
	OSIntEnter();  
		res = res;
	LED1Toggle;
	if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET) 
	{
		res = USART3->SR;		
		res = USART3->DR;		// ����ж��жϱ�־λ

		DMA_Cmd(DMA1_Channel3, DISABLE);
		flag_gps = 1;           // �������

		//���ô������ݳ���  
    DMA_SetCurrDataCounter(DMA1_Channel3,GPS_RBUFF_SIZE); 
		DMA_Cmd(DMA1_Channel3, ENABLE);
     } 
	OSIntExit();  											 
} 




