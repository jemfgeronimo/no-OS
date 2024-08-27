/***************************************************************************//**
 *   @file   parameters.h
 *   @brief  Definitions specific to xilinx platform used by ad463x_fmcz
 *           project.
 *   @author Antoniu Miclaus (antoniu.miclaus@analog.com)
 *   @author Axel Haslam (ahaslam@baylibre.com)
********************************************************************************
 * Copyright 2024(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/
#ifndef __PARAMETERS_H__
#define __PARAMETERS_H__

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdio.h>
#include <xparameters.h>
#include <xil_cache.h>
#include <xilinx_uart.h>
#include "xilinx_gpio.h"
#include "axi_pwm_extra.h"
#include "spi_engine.h"


/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/
#ifdef _XPARAMETERS_PS_H_
#define UART_DEVICE_ID                  XPAR_XUARTPS_0_DEVICE_ID
#define INTC_DEVICE_ID                  XPAR_SCUGIC_SINGLE_DEVICE_ID

#ifdef XPS_BOARD_ZCU102
#define UART_IRQ_ID                     XPAR_XUARTPS_0_INTR
#else
#define UART_IRQ_ID                     XPAR_XUARTPS_1_INTR
#endif

#else // _XPARAMETERS_PS_H_
#define UART_DEVICE_ID                  XPAR_AXI_UART_DEVICE_ID
#define INTC_DEVICE_ID                  XPAR_INTC_SINGLE_DEVICE_ID
#define UART_IRQ_ID                     XPAR_AXI_INTC_AXI_UART_INTERRUPT_INTR
#endif // _XPARAMETERS_PS_H_

#define UART_BAUDRATE                   115200
#define UART_EXTRA                      &uart_extra_ip
#define UART_OPS                        &xil_uart_ops

#define DMA_BASEADDR                    XPAR_AXI_AD738X_DMA_BASEADDR
#define SPI_ENGINE_BASEADDR             XPAR_SPI_AD738X_ADC_SPI_AD738X_ADC_AXI_REGMAP_BASEADDR
#define RX_CLKGEN_BASEADDR              XPAR_SPI_CLKGEN_BASEADDR
#define AXI_PWMGEN_BASEADDR             XPAR_SPI_TRIGGER_GEN_BASEADDR

#define AD738X_SPI_CS                   0

#define SPI_DEVICE_ID                   0
#define SPI_OPS                         &spi_eng_platform_ops
#define SPI_EXTRA                       &spi_eng_init_param
#define SPI_CS                          0
#define SPI_BAUDRATE                    10000000

#define NO_OS_PWM_ID                    0
#define PWM_OPS                         &axi_pwm_ops
#define PWM_PERIOD_NS                   500
#define PWM_DUTY_NS                     10

#define SAMPLES_PER_CHANNEL		1000
#define BYTES_PER_SAMPLE		2
#define MAX_SIZE_BASE_ADDR		(SAMPLES_PER_CHANNEL * 2 * BYTES_PER_SAMPLE)

#define DCACHE_INVALIDATE		Xil_DCacheInvalidateRange

extern struct xil_uart_init_param uart_extra_ip;
extern struct spi_engine_init_param spi_eng_init_param;


#endif /* __PARAMETERS_H__ */
