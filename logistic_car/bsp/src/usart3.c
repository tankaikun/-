
#include "usart3.h"
#include  "ucos_ii.h"
#include "bsp.h"
#include "stdio.h"

int flag_gps = 0;

/* DMA接收缓冲  */
uint8_t gps_rbuff[GPS_RBUFF_SIZE];


static void usart3_init(int bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//使能USART1，GPIOA时钟
 	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);	

	//USART3_TX   GPIOB.10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOA.9 
	//USART3_RX	  GPIOB.11
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//PA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOA.10  
	
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);							//根据指定的参数初始化VIC寄存器

	USART_InitStructure.USART_BaudRate = bound;						//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;		//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;			//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;				//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

	USART_Init(USART3, &USART_InitStructure); 				
	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);					// 串口空闲中断	
	USART_ITConfig(USART3,USART_IT_TC,DISABLE); 
	USART_ITConfig(USART3,USART_IT_RXNE,DISABLE); 
	USART_Cmd(USART3, ENABLE);                    			
}

static void gps_dma_config(void)
{
	//串口收DMA配置    
	DMA_InitTypeDef DMA_InitStructure;
    
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);			//启动DMA时钟  
    
    DMA_DeInit(DMA1_Channel3);									//DMA1通道3配置  
    
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART3->DR);			//外设地址 
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)gps_rbuff;					//内存地址  
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;							//dma传输方向单向  
    DMA_InitStructure.DMA_BufferSize = GPS_RBUFF_SIZE;							//设置DMA在传输时缓冲区的长度  
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;			//设置DMA的外设递增模式，一个外设    
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;						//设置DMA的内存递增模式 
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;		//外设数据字长 
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;				//内存数据字长  
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;								//设置DMA的传输模式  
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;						//设置DMA的优先级别  
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;								//设置DMA的2个memory中的变量互相访问  
    DMA_Init(DMA1_Channel3,&DMA_InitStructure);  
  
    DMA_Cmd(DMA1_Channel3,ENABLE);  //使能通道3  
	USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);
}

void gps_usart3_config(void)
{
  usart3_init(38400);
  gps_dma_config();  
}


// USART3中断服务函数
void USART3_IRQHandler(void)          
{
	uint8_t res;
	OSIntEnter();  
		res = res;
	LED1Toggle;
	if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET) 
	{
		res = USART3->SR;		
		res = USART3->DR;		// 清除中断中断标志位

		DMA_Cmd(DMA1_Channel3, DISABLE);
		flag_gps = 1;           // 接受完成

		//设置传输数据长度  
    DMA_SetCurrDataCounter(DMA1_Channel3,GPS_RBUFF_SIZE); 
		DMA_Cmd(DMA1_Channel3, ENABLE);
     } 
	OSIntExit();  											 
} 




