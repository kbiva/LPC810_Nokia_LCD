/*
 * nokia6100.c
 *
 *  Created on: 2013.09.11
 *      Author: Kestutis Bivainis
 */

#include <stdint.h>
#include "spi.h"
#include "nokia6100.h"
#include "font6x8.h"

volatile static int32_t j = 0;

void LCDInit(void) {

	LPC_GPIO_PORT ->DIR0 |= (1 << RESET_PIN);
	LPC_GPIO_PORT ->CLR0 = 1 << RESET_PIN;

	for (j = 0; j < 1000; j++) {};

	LPC_GPIO_PORT ->SET0 = 1 << RESET_PIN;
	for (j = 0; j < 10000; j++) {};

	// Sleep out (command 0x11)
	spi0Transfer(SLEEPOUT);

#ifdef _NOKIA6100
	// Inversion on (command 0x20)
	spi0Transfer(INVON);
#endif

	// Color Interface Pixel Format (command 0x3A)
	spi0Transfer(COLMOD);
#ifdef _8BITCOLOR
	spi0Transfer(COLOR_MODE_8BIT | 0x100);
	LCDSetup8BitColor();
#elif defined _12BITCOLOR
	spi0Transfer(COLOR_MODE_12BIT | 0x100);
#elif defined _16BITCOLOR
	spi0Transfer(COLOR_MODE_16BIT | 0x100);
#endif

	// Memory access controller (command 0x36)
	// 0xC8 = mirror x and y, reverse rgb    Nokia 6100
	// 0x80 = mirror y                       Nokia 6030
	// 0x80 = mirror y                       Nokia 3100
	spi0Transfer(MADCTL);
	spi0Transfer(MADCTL_DATA | 0x100);
	// Write contrast (command 0x25)
	spi0Transfer(SETCON);
	spi0Transfer(SETCON_DATA | 0x100);

	for (j = 0; j < 1000; j++) {};

	// Display On (command 0x29)
	spi0Transfer(DISPON);
}

void LCDClearScreen(int32_t color) {
	uint32_t i;

	// Row address set (command 0x2B)
	spi0Transfer(PASET);
	spi0Transfer(START_Y | 0x100);
	spi0Transfer(END_Y | 0x100);

	// Column address set (command 0x2A)
	spi0Transfer(CASET);
	spi0Transfer(START_X | 0x100);
	spi0Transfer(END_X | 0x100);

	spi0Transfer(RAMWR);

#ifdef _8BITCOLOR
	for (i = 0; i < ((MAX_X * MAX_Y)); i++) {
		spi0Transfer((color & 0xFF) | 0x100);
	}
#elif defined _12BITCOLOR
	for (i = 0; i < ((MAX_X * MAX_Y) / 2); i++)
	{
		spi0Transfer(((color >> 4)&0xFF)|0x100);
		spi0Transfer((((color & 0x0F) << 4) | ((color >> 8) & 0x0F))|0x100);
		spi0Transfer((color&0xFF)|0x100);
	}
#elif defined _16BITCOLOR
	for (i = 0; i < ((MAX_X * MAX_Y)); i++)
	{
		spi0Transfer((color>>8)|0x100);
		spi0Transfer((color&0xFF)|0x100);
	}
#endif

}

void LCDSetPixel(int32_t x, int32_t y, int32_t color) {
	// Row address set (command 0x2B)
	spi0Transfer(PASET);
	spi0Transfer(x | 0x100);
	spi0Transfer(x | 0x100);

	// Column address set (command 0x2A)
	spi0Transfer(CASET);
	spi0Transfer(y | 0x100);
	spi0Transfer(y | 0x100);

	// WRITE MEMORY
	spi0Transfer(RAMWR);

#ifdef _8BITCOLOR
	spi0Transfer((color & 0xFF) | 0x100);
#elif defined _12BITCOLOR
	spi0Transfer(((color >> 4)&0xFF)|0x100);
	spi0Transfer(((color & 0x0F) << 4)|0x100);
#elif defined _16BITCOLOR
	spi0Transfer(((color >> 8)&0xFF)|0x100);
	spi0Transfer((color&0xFF)|0x100);
#endif
}

void LCDSetLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t color) {
	int32_t dy = y1 - y0;
	int32_t dx = x1 - x0;
	int32_t stepx, stepy;
	if (dy < 0) {
		dy = -dy;
		stepy = -1;
	} else {
		stepy = 1;
	}
	if (dx < 0) {
		dx = -dx;
		stepx = -1;
	} else {
		stepx = 1;
	}
	dy <<= 1; // dy is now 2*dy
	dx <<= 1; // dx is now 2*dx
	LCDSetPixel(x0, y0, color);
	if (dx > dy) {
		int32_t fraction = dy - (dx >> 1); // same as 2*dy - dx
		while (x0 != x1) {
			if (fraction >= 0) {
				y0 += stepy;
				fraction -= dx; // same as fraction -= 2*dx
			}
			x0 += stepx;
			fraction += dy; // same as fraction -= 2*dy
			LCDSetPixel(x0, y0, color);
		}
	} else {
		int fraction = dx - (dy >> 1);
		while (y0 != y1) {
			if (fraction >= 0) {
				x0 += stepx;
				fraction -= dy;
			}
			y0 += stepy;
			fraction += dx;
			LCDSetPixel(x0, y0, color);
		}
	}
}

void LCDSetRect(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint8_t fill, int32_t color) {
	int32_t xmin, xmax, ymin, ymax;
	int32_t i;

	// check if the rectangle is to be filled
	if (fill == FILL) {
		// best way to create a filled rectangle is to define a drawing box
		// and loop two pixels at a time
		// calculate the min and max for x and y directions
		xmin = (x0 <= x1) ? x0 : x1;
		xmax = (x0 > x1) ? x0 : x1;
		ymin = (y0 <= y1) ? y0 : y1;
		ymax = (y0 > y1) ? y0 : y1;

		// specify the controller drawing box according to those limits
		// Row address set (command 0x2B)
		spi0Transfer(PASET);
		spi0Transfer(xmin | 0x100);
		spi0Transfer(xmax | 0x100);

		// Column address set (command 0x2A)
		spi0Transfer(CASET);
		spi0Transfer(ymin | 0x100);
		spi0Transfer(ymax | 0x100);

		// WRITE MEMORY
		spi0Transfer(RAMWR);

#ifdef _8BITCOLOR
		// loop on total number of pixels
		for (i = 0; i < ((xmax - xmin + 1) * (ymax - ymin + 1)); i++) {
			spi0Transfer((color & 0xFF) | 0x100);
		}
#elif defined _12BITCOLOR
		// loop on total number of pixels / 2
		for (i = 0; i < ((((xmax - xmin + 1) * (ymax - ymin + 1)) / 2) + 1); i++)
		{
			// use the color value to output three data bytes covering two pixels
			spi0Transfer(((color >> 4)&0xFF)|0x100);
			spi0Transfer(((color & 0x0F) << 4) | ((color >> 8) & 0x0F)|0x100);
			spi0Transfer((color&0xFF)|0x100);
		}
#elif defined _16BITCOLOR
		// loop on total number of pixels
		for (i = 0; i < ((xmax - xmin + 1) * (ymax - ymin + 1)); i++)
		{
			spi0Transfer(((color >> 8)&0xFF)|0x100);
			spi0Transfer((color&0xFF)|0x100);
		}
#endif
	} else {
		// best way to draw unfilled rectangle is to draw four lines
		LCDSetLine(x0, y0, x1, y0, color);
		LCDSetLine(x0, y1, x1, y1, color);
		LCDSetLine(x0, y0, x0, y1, color);
		LCDSetLine(x1, y0, x1, y1, color);
	}
}

void LCDSetCircle(int32_t x0, int32_t y0, int32_t radius, int32_t color) {
	int32_t f = 1 - radius;
	int32_t ddF_x = 0;
	int32_t ddF_y = -2 * radius;
	int32_t x = 0;
	int32_t y = radius;
	LCDSetPixel(x0, y0 + radius, color);
	LCDSetPixel(x0, y0 - radius, color);
	LCDSetPixel(x0 + radius, y0, color);
	LCDSetPixel(x0 - radius, y0, color);
	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x + 1;
		LCDSetPixel(x0 + x, y0 + y, color);
		LCDSetPixel(x0 - x, y0 + y, color);
		LCDSetPixel(x0 + x, y0 - y, color);
		LCDSetPixel(x0 - x, y0 - y, color);
		LCDSetPixel(x0 + y, y0 + x, color);
		LCDSetPixel(x0 - y, y0 + x, color);
		LCDSetPixel(x0 + y, y0 - x, color);
		LCDSetPixel(x0 - y, y0 - x, color);
	}
}

void LCDPutChar(char c, int32_t x, int32_t y, int32_t fColor, int32_t bColor) {
	int32_t i, j;
	uint32_t nCols;
	uint32_t nRows;
	uint32_t nBytes;
	unsigned char PixelRow;
	unsigned char Mask;
	uint32_t Word0;
	uint32_t Word1;
	unsigned char *pFont;
	unsigned char *pChar;
	unsigned char *FontTable[] = { (unsigned char *) FONT6x8 };

	// get pointer to the beginning of the selected font table
	pFont = (unsigned char *) FontTable[0];
	// get the nColumns, nRows and nBytes
	nCols = *pFont;
	nRows = *(pFont + 1);
	nBytes = *(pFont + 2);

	// get pointer to the last byte of the desired character
	pChar = pFont + (nBytes * (c - 0x1F)) + nBytes - 1;

	// Row address set (command 0x2B)
	spi0Transfer(PASET);
	spi0Transfer(x | 0x100);
	spi0Transfer((x + nRows - 1) | 0x100);

	// Column address set (command 0x2A)
	spi0Transfer(CASET);
	spi0Transfer(y | 0x100);
	spi0Transfer((y + nCols - 1) | 0x100);

	// WRITE MEMORY
	spi0Transfer(RAMWR);

	// loop on each row, working backwards from the bottom to the top
	for (i = nRows - 1; i >= 0; i--) {
		// copy pixel row from font table and then decrement row
		PixelRow = *pChar--;

		// loop on each pixel in the row (left to right)
		// Note: we do two pixels each loop
		Mask = 0x80;
		for (j = 0; j < nCols; j += 2) {
			// if pixel bit set, use foreground color; else use the background color
			// now get the pixel color for two successive pixels
			if (PixelRow & Mask)
				Word0 = fColor;
			else
				Word0 = bColor;
			Mask >>= 1;

			if (PixelRow & Mask)
				Word1 = fColor;
			else
				Word1 = bColor;
			Mask >>= 1;

#ifdef _8BITCOLOR
			// use this information to output two data bytes
			spi0Transfer((Word0 & 0xFF) | 0x100);
			spi0Transfer((Word1 & 0xFF) | 0x100);
#elif defined _12BITCOLOR
			// use this information to output three data bytes
			spi0Transfer(((Word0 >> 4)&0xFF)|0x100);
			spi0Transfer(((Word0 & 0x0F) << 4)|((Word1 >> 8) & 0x0F)|0x100);
			spi0Transfer((Word1&0xFF)|0x100);
#elif defined _16BITCOLOR
			// use this information to output four data bytes
			spi0Transfer(((Word0 >> 8)&0xFF)|0x100);
			spi0Transfer((Word0&0xFF)|0x100);
			spi0Transfer(((Word1 >> 8)&0xFF)|0x100);
			spi0Transfer((Word1&0xFF)|0x100);
#endif
		}
	}

}

void LCDPutStr(char *pString, int32_t x, int32_t y, int32_t fColor, int32_t bColor) {
	// loop until null-terminator is seen
	while (*pString) {
		// draw the character
		LCDPutChar(*pString++, x, y, fColor, bColor);

		// advance the y position
		y = y + 6;

		// bail out if y exceeds 131
		if (y > END_Y)
			break;
	}
}

void LCDSetup8BitColor(void) {

	spi0Transfer(RGBSET);  // Define Color Table  (command 0x2D)
	// red
	spi0Transfer(0x00 | 0x100);
	spi0Transfer(0x02 | 0x100);
	spi0Transfer(0x04 | 0x100);
	spi0Transfer(0x06 | 0x100);
	spi0Transfer(0x09 | 0x100);
	spi0Transfer(0x0B | 0x100);
	spi0Transfer(0x0D | 0x100);
	spi0Transfer(0x0F | 0x100);
	// green
	spi0Transfer(0x00 | 0x100);
	spi0Transfer(0x02 | 0x100);
	spi0Transfer(0x04 | 0x100);
	spi0Transfer(0x06 | 0x100);
	spi0Transfer(0x09 | 0x100);
	spi0Transfer(0x0B | 0x100);
	spi0Transfer(0x0D | 0x100);
	spi0Transfer(0x0F | 0x100);
	// blue
	spi0Transfer(0x00 | 0x100);
	spi0Transfer(0x04 | 0x100);
	spi0Transfer(0x0B | 0x100);
	spi0Transfer(0x0F | 0x100);
}

