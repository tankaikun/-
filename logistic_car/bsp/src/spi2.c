#include "spi2.h"

void spi_gpio_config(void)
{
	/**
	* MOSI    out
	* MISO    in 
	* SCK     out
	* CSN     out
	* IRQ     in 
	* CE      out
	*/	
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOB,ENABLE);
	
	//初始化MISO 上拉输入
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	
	GPIO_InitStructure.GPIO_Pin = CC1101_SPI_MISO_PIN;
	GPIO_Init(CC1101_SPI_MISO_PORT,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = CC1101_SPI_IRQ_PIN;
	GPIO_Init(CC1101_SPI_IRQ_PORT, &GPIO_InitStructure);
	
	//SCK MOSI NSS配置为推挽输出
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	
	GPIO_InitStructure.GPIO_Pin = CC1101_SPI_MISO_PIN;
	GPIO_Init(CC1101_SPI_MOSI_PORT,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = CC1101_SPI_SCK_PIN;
	GPIO_Init(CC1101_SPI_SCK_PORT,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = CC1101_SPI_CE_PIN;
	GPIO_Init(CC1101_SPI_CE_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = CC1101_SPI_CSN_PIN;
	GPIO_Init(CC1101_SPI_CSN_PORT, &GPIO_InitStructure);
}


/**
  * @brief :SPI收发一个字节
  * @param :
  *			@TxByte: 发送的数据字节
  * @note  :非阻塞式，一旦等待超时，函数会自动退出
  * @retval:接收到的字节
  */
uint8_t drv_spi_read_write_byte( uint8_t TxByte )
{
	uint8_t i=0, revdata=0;
	
	for(i=0; i<8; i++)
	{
		// send 1 byte data
		SPI2_SCK_LOW();
		if(0x08 & TxByte)
		{
			SPI2_MOSI_HIGH();	
		}
		else 
		{
			SPI2_MOSI_LOW();
		}
		TxByte <<= 1;
		
		SPI2_SCK_HIGH();
		__nop();
		__nop();
		
		// receive 1 byte data
		revdata <<= 1;         // 
		if(SPI2_READ_MISO())
		{
			revdata |= 0x01;
		}
		__nop();
		__nop();
	}
	
	return revdata;
}

/**
  * @brief :SPI收发字符串
  * @param :
  *			@ReadBuffer: 接收数据缓冲区地址
  *			@WriteBuffer:发送字节缓冲区地址
  *			@Length:字节长度
  * @note  :非堵塞式，一旦等待超时，函数会自动退出
  * @retval:无
  */
void drv_spi_read_write_string( uint8_t* ReadBuffer, uint8_t* WriteBuffer, uint16_t Length )
{
	SPI2_CSN_LOW();
	
	while(Length--)
	{
		*ReadBuffer = drv_spi_read_write_byte(*WriteBuffer);
		ReadBuffer++;
		WriteBuffer++;
	}
	
	SPI2_CSN_HIGH();
}



