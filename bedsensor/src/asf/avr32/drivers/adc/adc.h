/*****************************************************************************
 *
 * \file
 *
 * \brief ADC header for AVR UC3 UC3.
 *
 * This file defines a useful set of functions for ADC on AVR UC3 devices.
 *
 * Copyright (c) 2009 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an Atmel
 *    AVR product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 *****************************************************************************/


#ifndef _ADC_H_
#define _ADC_H_

/**
 * \defgroup group_avr32_drivers_adc ANALOG - ADC - Analog to Digital Converter
 *
 * Analog to Digital Converter is able to capture analog signals and transform them
 * into digital format with 10-bit resolution.
 *
 * \{
 */


#include <avr32/io.h>
#include "compiler.h"


// if using 8 bits for ADC, define this flag in your compiler options
//! Max value for ADC resolution
#ifdef USE_ADC_8_BITS
#  define ADC_MAX_VALUE    0xFF
#else
#  define ADC_MAX_VALUE    0x3FF
#endif


/*! \brief Configure ADC. Mandatory to call.
 * If not called, ADC channels will have side effects
 *
 * \param *adc Base address of the ADC
 */
extern void adc_configure(volatile avr32_adc_t * adc);

/*! \brief Start analog to digital conversion
 * \param *adc Base address of the ADC
 */
extern void adc_start(volatile avr32_adc_t * adc);

/*! \brief Enable channel
 *
 * \param *adc Base address of the ADC
 * \param  channel   channel to enable (0 to 7)
 */
extern void adc_enable(volatile avr32_adc_t * adc, unsigned short channel);

/*! \brief Disable channel
 *
 * \param *adc Base address of the ADC
 * \param  channel   channel to disable (0 to 7)
 */
extern void adc_disable(volatile avr32_adc_t * adc, unsigned short channel);

/*! \brief Get channel 0 to 7 status
 *
 * \param *adc Base address of the ADC
 * \param  channel   channel to handle (0 to 7)
 * \return bool      true if channel is enabled
 *                   false if channel is disabled
 */
extern bool adc_get_status(volatile avr32_adc_t * adc, unsigned short channel);

/*! \brief Check channel conversion status
 *
 * \param *adc Base address of the ADC
 * \param  channel   channel to check (0 to 7)
 * \return bool      true if conversion not running
 *                   false if conversion running
 */
extern bool adc_check_eoc(volatile avr32_adc_t * adc, unsigned short channel);

/*! \brief Check channel conversion overrun error
 *
 * \param *adc Base address of the ADC
 * \param  channel   channel to check (0 to 7)
 * \return bool      FAIL if an error occured
 *                   PASS if no error occured
 */
extern bool adc_check_ovr(volatile avr32_adc_t * adc, unsigned short channel);

/*! \brief Get channel value
 *
 * \param *adc Base address of the ADC
 * \param  channel   channel to handle (0 to 7)
 * \return The value aquired (unsigned long)
 */
extern unsigned long adc_get_value(volatile avr32_adc_t * adc, unsigned short channel);

/*! \brief Wait for the next converted data and return its value
 *
 * \param *adc Base address of the ADC
 * \return The latest converted value (unsigned long)
 */
extern unsigned long adc_get_latest_value(volatile avr32_adc_t * adc);

/**
 * \}
 */

#endif  // _ADC_H_
