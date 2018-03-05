#include "spi.h"



// 
/**
  * @name    SPI_NRF_RW
  * @brief   用于向NRF读/写一字节数据
  * @param   写入的数据
  *	@arg 	 dat 
  * @retval  读取得的数据
  */
uint8_t spi_rw_byte(uint8_t dat)
{  	
   /* 当 SPI发送缓冲器非空时等待 */
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
   /* 通过SPI2发送一字节数据 */
  SPI_I2S_SendData(SPI1, dat);		
   /* 当SPI接收缓冲器为空时等待 */
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
  /* Return the byte read from the SPI bus */
  return SPI_I2S_ReceiveData(SPI1);
}

/**
  * @brief   用于向NRF特定的寄存器写入数据 SPI_NRF_WriteReg
  * @param   
  * @arg reg:NRF的命令+寄存器地址
  * @arg dat:将要向寄存器写入的数据
  * @retval  NRF的status寄存器的状态
  */
uint8_t spi_write_register(uint8_t reg,uint8_t dat)
{
 	u8 status;
	SPI1_CE_LOW();
	/*置低CSN，使能SPI传输*/
	SPI1_CSN_LOW();	
	
	/*发送命令及寄存器号 */
	status = spi_rw_byte(reg);
	/*向寄存器写入数据*/
	spi_rw_byte(dat);
	
	/*CSN拉高，完成*/	   
	SPI1_CSN_HIGH();	

	return(status);
}

/**SPI_NRF_ReadReg
  * @brief   用于从NRF特定的寄存器读出数据
  * @param   
  *		@arg reg:NRF的命令+寄存器地址
  * @retval  寄存器中的数据
  */
uint8_t spi_read_register(uint8_t reg)
{
 	uint8_t reg_val;

	SPI1_CE_LOW();
	/*置低CSN，使能SPI传输*/
 	SPI1_CSN_LOW();
			
	/*发送寄存器号*/
	spi_rw_byte(reg); 
	/*读取寄存器的值 */
	reg_val = spi_rw_byte(NOP);      
	
  /*CSN拉高，完成*/
	SPI1_CSN_HIGH();		

	return reg_val;
}	

/**
  * @brief   用于向NRF的寄存器中写入一串数据
  * @param   
  *		@arg reg : NRF的命令+寄存器地址
  *		@arg pBuf：用于存储将被读出的寄存器数据的数组，外部定义
  * 	@arg bytes: pBuf的数据长度
  * @retval  NRF的status寄存器的状态
  */
uint8_t spi_read_buffer(uint8_t reg,uint8_t *pBuf,uint8_t bytes)
{
 	uint8_t status, byte_cnt;

	SPI1_CE_LOW();
	/*置低CSN，使能SPI传输*/
	SPI1_CSN_LOW();
		
	/*发送寄存器号*/		
	status = spi_rw_byte(reg); 

 	/*读取缓冲区数据*/
	 for(byte_cnt=0;byte_cnt<bytes;byte_cnt++)		  
	   pBuf[byte_cnt] = spi_rw_byte(NOP); //从NRF24L01读取数据  

	 /*CSN拉高，完成*/
	SPI1_CSN_HIGH();	
		
 	return status;		//返回寄存器状态值
}

/**
  * @brief   用于向NRF的寄存器中写入一串数据
  * @param   
  *		@arg reg : NRF的命令+寄存器地址
  *		@arg pBuf：存储了将要写入写寄存器数据的数组，外部定义
  * 	@arg bytes: pBuf的数据长度
  * @retval  NRF的status寄存器的状态
  */
uint8_t spi_write_buffer(uint8_t reg ,uint8_t *pBuf,uint8_t bytes)
{
	uint8_t status,byte_cnt;
	SPI1_CE_LOW();
	/*置低CSN，使能SPI传输*/
	SPI1_CSN_LOW();			

	/*发送寄存器号*/	
	status = spi_rw_byte(reg); 

	/*向缓冲区写入数据*/
	for(byte_cnt=0;byte_cnt<bytes;byte_cnt++)
	spi_rw_byte(*pBuf++);	//写数据到缓冲区 	 
			 
	/*CSN拉高，完成*/
	SPI1_CSN_HIGH();			

	return (status);	//返回NRF24L01的状态 		
}


