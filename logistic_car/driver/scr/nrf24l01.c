#include "nrf24l01.h"
#include "spi.h"
#include "usart.h"
#include  "..\..\uCOS-II\Source\ucos_ii.h"

uint8_t RX_BUF[RX_PLOAD_WIDTH];		//接收数据缓存
uint8_t TX_BUF[TX_PLOAD_WIDTH];		//发射数据缓存
uint8_t TX_ADDRESS[TX_ADR_WIDTH] = {0x34,0x43,0x10,0x10,0x11};  // 定义一个静态发送地址
uint8_t RX_ADDRESS[RX_ADR_WIDTH] = {0x34,0x43,0x10,0x10,0x11};

static void Delay(__IO u32 nCount)
{
  for(; nCount != 0; nCount--);
} 

void NRF_SPI_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(NRF_SPI_CE_CLK|NRF_SPI_IRQ_CLK|NRF_SPI_CSN_CLK\
												|NRF_SPI1_GPIO_CLK,ENABLE);
	/* MOSI MISO SCK */
	GPIO_InitStructure.GPIO_Pin = NRF_SPI_MISO_PIN|NRF_SPI_MOSI_PIN|NRF_SPI_SCK_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(NRF_SPI1_GPIO_PORT,&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = NRF_SPI_CE_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(NRF_SPI_CE_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = NRF_SPI_CSN_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(NRF_SPI_CSN_PORT, &GPIO_InitStructure);
	/*配置SPI_NRF_SPI的IRQ引脚*/
	GPIO_InitStructure.GPIO_Pin = NRF_SPI_IRQ_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU ; 
	GPIO_Init(NRF_SPI_IRQ_PORT, &GPIO_InitStructure); 
  
}


static void NRF_SPI_Config(void)
{
	SPI_InitTypeDef SPI_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; 
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;									
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; 
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;                
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;											 
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;												   
	SPI_Init(SPI1,&SPI_InitStructure);
	
	/* Enable SPI1  */
  SPI_Cmd(SPI1, ENABLE);
}


void SPI_NRF_Init(void)
{
	NRF_SPI_GPIO_Config();
	SPI1_CSN_HIGH();       
	NRF_SPI_Config();
}


void NRF_RX_Mode(void)

{
	SPI1_CE_LOW();	
	spi_write_buffer(NRF_WRITE_REG+RX_ADDR_P0,RX_ADDRESS,RX_ADR_WIDTH);
	spi_write_register(NRF_WRITE_REG+EN_AA,0x01);       // 失能通道0自动应答
	spi_write_register(NRF_WRITE_REG+EN_RXADDR,0x01);   // 
	spi_write_register(NRF_WRITE_REG+RF_CH,CHANAL);     
	spi_write_register(NRF_WRITE_REG+RX_PW_P0,RX_PLOAD_WIDTH);
	spi_write_register(NRF_WRITE_REG+RF_SETUP,0x0f);           // 
	spi_write_register(NRF_WRITE_REG+CONFIG, 0x0f);  
	
	SPI1_CE_HIGH();
	Delay(0xffff);
} 


void NRF_TX_Mode(void)
{  
	SPI1_CE_LOW();		
	spi_write_buffer(NRF_WRITE_REG+TX_ADDR,TX_ADDRESS,TX_ADR_WIDTH);    
	spi_write_buffer(NRF_WRITE_REG+RX_ADDR_P0,RX_ADDRESS,RX_ADR_WIDTH); 
	spi_write_register(NRF_WRITE_REG+EN_AA,0x01);     
	spi_write_register(NRF_WRITE_REG+EN_RXADDR,0x01); 
	spi_write_register(NRF_WRITE_REG+SETUP_RETR,0x1a);
	spi_write_register(NRF_WRITE_REG+RF_CH,CHANAL);      
	spi_write_register(NRF_WRITE_REG+RF_SETUP,0x0f);    //  00
	spi_write_register(NRF_WRITE_REG+CONFIG,0x0e);   
	
	SPI1_CE_HIGH();
	Delay(0xffff);
}


uint8_t NRF_Check(void)
{
	uint8_t buf[5]={0xC2,0xC2,0xC2,0xC2,0xC2};
	uint8_t buf1[5];
	uint8_t i; 
	 
	/*写入5个字节的地址.  */  
	spi_write_buffer(NRF_WRITE_REG+TX_ADDR,buf,5);

	/*读出写入的地址 */
	spi_read_buffer(TX_ADDR,buf1,5); 
	 
	/*比较*/               
	for(i=0;i<5;i++)
	{
		if(buf1[i]!=0xC2)
		break;
	} 
	       
	if(i==5)
	{
		printf("\r\n wireless link to mcu success \r\n ");
		return SUCCESS ;        
	}
	else
	{
		printf("\r\n wireless link fail \r\n ");
		return ERROR ;        
	}
}


uint8_t NRF_Tx_Dat(uint8_t *txbuf)
{
	uint8_t state;  

	SPI1_CE_LOW();
					
	spi_write_buffer(WR_TX_PLOAD,txbuf,TX_PLOAD_WIDTH);

	SPI1_CE_HIGH();
                
	while(SPI1_Read_IRQ()!=0); 	
                          
	state = spi_read_register(STATUS);
           
	spi_write_register(NRF_WRITE_REG+STATUS,state); 	

	spi_write_register(FLUSH_TX,NOP);    

	/*判断中断类型*/    
	if(state&MAX_RT)                     //达到最大重发次数
	{
		printf("\r\n send fail\r\n");
		return MAX_RT; 
	}
	else if(state&TX_DS)                 //发送完成
	{
		printf("\r\n send success\r\n");
		return TX_DS;
	}
	else				
	{	
		printf("\r\n unknown error\r\n");
		return ERROR;                    //其他原因发送失败
	}
} 

uint8_t NRF_Rx_Dat(uint8_t *rxbuf)
{
	uint8_t state; 
	SPI1_CE_HIGH();	    

	while(SPI1_Read_IRQ()!=0)
	{
		OSTimeDlyHMSM(0, 0, 0, 10);
	}

	SPI1_CE_LOW();  	
					
	state=spi_read_register(STATUS);
	 
	spi_write_register(NRF_WRITE_REG+STATUS,state);

	if(state&RX_DR)                                      
	{
		spi_read_buffer(RD_RX_PLOAD, rxbuf, RX_PLOAD_WIDTH); 
		spi_write_register(FLUSH_RX, NOP);                 
		return RX_DR; 
	}
	else    
		return ERROR;
	
	                                       
}





