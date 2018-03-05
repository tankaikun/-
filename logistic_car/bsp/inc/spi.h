#ifndef __SPI_H
#define __SPI_H

#include "stm32f10x.h"
#include "nrf24l01.h"


#define SPI1_CSN_HIGH()     GPIO_SetBits(NRF_SPI_CSN_PORT, NRF_SPI_CSN_PIN)
#define SPI1_CSN_LOW()      GPIO_ResetBits(NRF_SPI_CSN_PORT, NRF_SPI_CSN_PIN)		        //csn置低
#define SPI1_CE_HIGH()	    GPIO_SetBits(NRF_SPI_CE_PORT,NRF_SPI_CE_PIN)
#define SPI1_CE_LOW()	    	GPIO_ResetBits(NRF_SPI_CE_PORT,NRF_SPI_CE_PIN)			      	//CE置低
#define SPI1_Read_IRQ()			GPIO_ReadInputDataBit (NRF_SPI_IRQ_PORT, NRF_SPI_IRQ_PIN)   //中断引脚



uint8_t spi_rw_byte(uint8_t dat);
uint8_t spi_write_register(uint8_t reg,uint8_t dat);
uint8_t spi_read_register(uint8_t reg);
uint8_t spi_read_buffer(uint8_t reg,uint8_t *pBuf,uint8_t bytes);
uint8_t spi_write_buffer(uint8_t reg ,uint8_t *pBuf,uint8_t bytes);


#endif /* __SPI_H */
