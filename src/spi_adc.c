/*
 * spi_adc.c
 *
 * Created: 7/22/2012 9:20:16 PM
 *  Author: Administrator
 */

#include "spi_adc.h"
#include <conf_board.h>
#include <asf.h>

uint16_t adc_sample(uint16_t chan)
{
	uint16_t spi_resp;
	uint16_t fake_data = 0,data1 = 0;
	spi_selectChip(ADC_SPI,3);
	spi_resp = spi_write(ADC_SPI,chan);
	spi_resp = spi_read(ADC_SPI,&fake_data);
	spi_unselectChip(ADC_SPI,3);
	
	spi_resp = spi_selectChip(ADC_SPI,ADC_CS);
	spi_resp = spi_write(ADC_SPI,0xFFFF);
	spi_resp = spi_read(ADC_SPI,&data1);
	spi_unselectChip(ADC_SPI,ADC_CS);
	
	return (0x3FFF & data1);
}