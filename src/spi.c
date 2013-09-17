/**************************************************************************/
/*!
 @file     spi.c
 @author   K. Townsend

 @section LICENSE

 Software License Agreement (BSD License)

 Copyright (c) 2013, K. Townsend (microBuilder.eu)
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 1. Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 3. Neither the name of the copyright holders nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**************************************************************************/
#include "spi.h"

/* Configure SPI as Master in Mode 0 (CPHA and CPOL = 0) */
void spi0Init(uint32_t div, uint32_t delay) {

	/* Enable SPI clock */
	LPC_SYSCON ->SYSAHBCLKCTRL |= (1 << 11);

	/* Bring SPI out of reset */
	LPC_SYSCON ->PRESETCTRL &= ~(0x1 << 0);
	LPC_SYSCON ->PRESETCTRL |= (0x1 << 0);

	/* Set clock speed and mode */
	LPC_SPI0 ->DIV = div;
	LPC_SPI0 ->DLY = delay;
	LPC_SPI0 ->CFG = (SPI_CFG_MASTER & ~SPI_CFG_ENABLE);
	LPC_SPI0 ->CFG |= SPI_CFG_ENABLE;
}

/* Send/receive data via the SPI bus (assumes 9bit data) */
void spi0Transfer(uint16_t data) {
	while ((LPC_SPI0 ->STAT & SPI_STAT_TXRDY)== 0 );
	// 9bits,end of transfer,transmit slave select,receive ignore
	LPC_SPI0 ->TXDATCTL = SPI_TXDATCTL_FSIZE(9-1) | SPI_TXDATCTL_EOT
			| SPI_TXDATCTL_TXSSEL_N | SPI_TXDATCTL_RXIGNORE | data;
}
