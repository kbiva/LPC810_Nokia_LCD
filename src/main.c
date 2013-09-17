/*
 * main.c
 *
 *  Created on: 2013.09.11
 *      Author: Kestutis Bivainis
 */

#include "LPC8xx.h"
#include "gpio.h"
#include "spi.h"
#include "mrt.h"
#include "nokia6100.h"

int32_t TempColor[11] = { BLACK, WHITE, RED, GREEN, BLUE, CYAN,
                      MAGENTA, YELLOW, BROWN, ORANGE, PINK };

char *TempChar[11] = { "Black", "White", "Red", "Green", "Blue", "Cyan",
                       "Magenta", "Yellow", "Brown", "Orange", "Pink" };

void configurePins() {
	/* Enable SWM clock */
	//  LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 7);  // this is already done in SystemInit()
	/* Pin Assign 8 bit Configuration */
	/* U0_TXD */
	/* U0_RXD */
	LPC_SWM ->PINASSIGN0 = 0xffff0004UL;
	/* SPI0_SCK */
	LPC_SWM ->PINASSIGN3 = 0x03ffffffUL;
	/* SPI0_MOSI */
	/* SPI0_SSEL */
	//LPC_SWM ->PINASSIGN4 = 0xff05ff01UL;
	LPC_SWM->PINASSIGN4 = 0xffffff01UL;

	/* Pin Assign 1 bit Configuration */
	//LPC_SWM ->PINENABLE0 = 0xffffffffUL;
	/* RESET */
	LPC_SWM->PINENABLE0 = 0xffffffbfUL;

	/* Enable UART clock */
	//LPC_SYSCON ->SYSAHBCLKCTRL |= (1 << 18);

	/* Pin I/O Configuration */
	/* LPC_IOCON->PIO0_0 = 0x90; */
	/* LPC_IOCON->PIO0_1 = 0x90; */
	/* LPC_IOCON->PIO0_2 = 0x90; */
	/* LPC_IOCON->PIO0_3 = 0x90; */
	/* LPC_IOCON->PIO0_4 = 0x90; */
	LPC_IOCON ->PIO0_5 = 0x88;

}

uint32_t xor128(void) {
	static uint32_t x = 123456789;
	static uint32_t y = 362436069;
	static uint32_t z = 521288629;
	static uint32_t w = 88675123;
	uint32_t t;

	t = x ^ (x << 11);
	x = y;
	y = z;
	z = w;
	return w = w ^ (w >> 19) ^ (t ^ (t >> 8));
}

int32_t main(void) {

	int32_t k,a;
	int32_t p1x;
	int32_t p1y;
	int32_t p2x;
	int32_t p2y;
	int32_t pcol;

	/* Initialise the GPIO block */
	gpioInit();

	// SPI 0 init
	spi0Init(0, 0);

	/* Configure the multi-rate timer for 1ms ticks */
	mrtInit(__SYSTEM_CLOCK / 1000);

	/* Configure the switch matrix (setup pins for UART0 and GPIO) */
	configurePins();

	// LCD init
	LCDInit();

	LCDClearScreen(BLACK);
	LCDPutStr("LPC800 Mini-Kit:", MAX_X / 2 + 40, 1, WHITE, BLACK);
	LCDPutStr("32-bit ARM Cortex-M0+", MAX_X / 2 + 30, 1, WHITE, BLACK);
	LCDPutStr("LPC810 @30Mhz", MAX_X / 2 + 20, 1, WHITE, BLACK);
	LCDPutStr("4kB flash, 1kB SRAM", MAX_X / 2 + 10 , 1, WHITE, BLACK);
	LCDPutStr("Nokia 6100 LCD", MAX_X / 2 , 1, WHITE, BLACK);
	LCDPutStr("@8-bit color", MAX_X / 2 - 10, 1, WHITE, BLACK);
	LCDPutStr("9-bit SPI @30Mhz", MAX_X / 2 - 20, 1, WHITE, BLACK);

	for(k=0;k<200;k++) {
	  LCDPutStr("kbiva.wordpress.com", MAX_X / 2 - 40, 10, TempColor[xor128()%10+1], BLACK);
	  mrtDelay(50);
    }

    for (k = 0; k < 11; k++)
    {
      LCDClearScreen(TempColor[k]);
      LCDPutStr(TempChar[k], 5, 40, WHITE, BLACK);
      mrtDelay(1000);
    }

    for(a=0;a<4;a++) {

    	LCDClearScreen(BLACK);
   	 	switch(a) {
    		case 0:LCDPutStr("1000 lines", MAX_X / 2 - 10, 10, WHITE, BLACK);break;
    		case 1:LCDPutStr("1000 rectangles", MAX_X / 2 - 10, 10, WHITE, BLACK);break;
    		case 2:LCDPutStr("1000 circles", MAX_X / 2 - 10, 10, WHITE, BLACK);break;
    		case 3:LCDPutStr("1000 filled", MAX_X / 2 - 10, 10, WHITE, BLACK);
    		       LCDPutStr("rectangles", MAX_X / 2 - 20 , 10, WHITE, BLACK);break;
    	}
   	 	mrtDelay(1000);

    	for (k = 0; k < 1000; k++) {
    	 	p1x = xor128() % MAX_X;
    	 	p1y = xor128() % MAX_Y;
    	 	p2x = xor128() % MAX_X;
    	 	p2y = xor128() % MAX_Y;
#ifdef _8BITCOLOR
    	 	pcol = xor128() % 256;
#elif defined _12BITCOLOR
    	 	pcol=xor128() % 4096;
#elif defined _16BITCOLOR
    	 	pcol=xor128() % 65536;
#endif
    	 	switch(a){
    	 		case 0:LCDSetLine(p1x, p1y, p2x, p2y, pcol);break;
    	 		case 1:LCDSetRect(p1x, p1y, p2x, p2y, NOFILL, pcol);break;
    	 		case 2:LCDSetCircle(p1x, p1y, p2x/2, pcol);break;
    	 		case 3:LCDSetRect(p1x, p1y, p2x, p2y, FILL, pcol);break;
    	 	}
    	}
    }

	//LCDClearScreen(BLACK);

	spi0Transfer(DISPOFF);
	spi0Transfer(SLEEPIN);

	while (1) {
	}
}
