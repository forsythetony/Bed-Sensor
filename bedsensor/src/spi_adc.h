/*
 * spi_adc.h
 *
 * Created: 7/22/2012 9:21:04 PM
 *  Author: Administrator
 */ 


#ifndef SPI_ADC_H_
#define SPI_ADC_H_

#include "compiler.h"
#include <spi.h>


uint16_t adc_sample(uint16_t chan);

#endif /* SPI_ADC_H_ */