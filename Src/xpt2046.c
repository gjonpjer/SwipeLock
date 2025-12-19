///*
// * xpt2046.c
// *
// *  Created on: Jan 6, 2020
// *      Author: Kotetsu Yamamoto
// *      Copyright [Kotetsu Yamamoto]
//
//I refer https://github.com/PaulStoffregen/XPT2046_Touchscreen/blob/master/XPT2046_Touchscreen.cpp
//
//from Original source:
//
// * Copyright (c) 2015, Paul Stoffregen, paul@pjrc.com
// *
// * Permission is hereby granted, free of charge, to any person obtaining a copy
// * of this software and associated documentation files (the "Software"), to deal
// * in the Software without restriction, including without limitation the rights
// * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// * copies of the Software, and to permit persons to whom the Software is
// * furnished to do so, subject to the following conditions:
// *
// * The above copyright notice, development funding notice, and this permission
// * notice shall be included in all copies or substantial portions of the Software.
// *
// * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// * THE SOFTWARE.
// */
//
//#include "xpt2046.h"
//#include "main.h"
//
//extern SPI_HandleTypeDef hspi3;
//#define touchSPI hspi3
//
//static void XPT2046_SetCS(void)
//{
//	HAL_GPIO_WritePin(T_CS_GPIO_Port, T_CS_Pin, GPIO_PIN_SET);
//}
//
//static void XPT2046_ResetCS(void)
//{
//	HAL_GPIO_WritePin(T_CS_GPIO_Port, T_CS_Pin, GPIO_PIN_RESET);
//}
//
//void writeSPI (uint8_t *data, int size)
//{
//	HAL_SPI_Transmit(&touchSPI, data, size, 2000);
//}
//
//void readSPI (uint8_t *data, int size)
//{
//	HAL_SPI_Receive(&touchSPI, data, size, 2000);
//}
//
//void T_Delay (uint16_t ms)
//{
//	HAL_Delay (ms);
//}
//
///**********************************************************************************************************/
//
//#define T_IRQ XPT2046_ReadIRQ()
//#define T_CS_ON XPT2046_SetCS()
//#define T_CS_OFF XPT2046_ResetCS()
//
//#define READ_TIMES 	5
//#define LOST_VAL 	1
//#define ERR_RANGE 50
//#define Z_THRESHOLD     400
//#define Z_THRESHOLD_INT	75
//#define MSEC_THRESHOLD  3
//
//static uint8_t XPT2046_initilazed = 0;
//
//static void XPT2046_Write_Byte(uint8_t num)
//{
//	writeSPI(&num, 1);
//}
//
//static uint16_t XPT2046_Read_AD(uint8_t CMD)
//{
//	uint8_t num[2];
//	uint16_t ret;
//
//	T_CS_OFF;
//	XPT2046_Write_Byte(CMD);
//	T_Delay(5); // modified
//
//	readSPI(num, 2);
//	T_CS_ON;
//
//	ret = num[0] << 8 | num[1];
//	ret >>= 3;
//	ret &= (1<<12)-1;
//
//	return ret;
//}
//
//static int16_t besttwoavg( int16_t x , int16_t y , int16_t z ) {
//  int16_t da, db, dc;
//  int16_t reta = 0;
//  if ( x > y ) da = x - y; else da = y - x;
//  if ( x > z ) db = x - z; else db = z - x;
//  if ( z > y ) dc = z - y; else dc = y - z;
//
//  if ( da <= db && da <= dc ) reta = (x + y) >> 1;
//  else if ( db <= da && db <= dc ) reta = (x + z) >> 1;
//  else reta = (y + z) >> 1;   //    else if ( dc <= da && dc <= db ) reta = (x + y) >> 1;
//
//  return (reta);
//}
//
//void XPT2046_Init(void)
//{
//	XPT2046_initilazed = 1;
//}
//
//void XPT2046_Update(uint16_t *x, uint16_t *y)
//{
//	int16_t data[6];
//	static uint32_t ptime = 0;
//
//	if (XPT2046_initilazed == 0) {
//		return;
//	}
//
//	if (HAL_GetTick() - ptime < MSEC_THRESHOLD) {
//		return;
//	}
//
//	int16_t z1 = XPT2046_Read_AD(0xb1); // z1
//	int32_t z = z1 + 4095;
//	int16_t z2 = XPT2046_Read_AD(0xc1); // z2
//	z -= z2;
//	if (z >= Z_THRESHOLD) {
//		XPT2046_Read_AD(0x91);  // dummy 1st X measure
//		data[0] = XPT2046_Read_AD(0x91);
//		data[1] = XPT2046_Read_AD(0xd1);
//		data[2] = XPT2046_Read_AD(0x91);
//		data[3] = XPT2046_Read_AD(0xd1);
//	} else {
//		data[0] = data[1] = data[2] = data[3] = 0;
//	}
//	data[4] = XPT2046_Read_AD(0x91);
//	data[5] = XPT2046_Read_AD(0xd0);
//	ptime = HAL_GetTick();
//	if (z < 0) z = 0;
//	int16_t intx = besttwoavg( data[0], data[2], data[4] );
//	int16_t inty = besttwoavg( data[1], data[3], data[5] );
//	if (z >= Z_THRESHOLD) {
//		*x = intx;
//		*y = inty;
//	} else { // this else was added by me : Gjonpjer Kola
//		/* No press: explicitly report 0,0 so the caller
//		 * can treat this as "not pressed" */
//		*x = 0;
//		*y = 0;
//	}
//}
//
//uint8_t XPT2046_IsReasonable(uint16_t x, uint16_t y)
//{
//	if (x >= XPT_XMIN && x <= XPT_XMAX && y >= XPT_YMIN && y <= XPT_YMAX) {
//		return 1;
//	}
//	return 0;
//}



#include "xpt2046.h"
//#include <stdio.h>

// ====== Adjust these to match your wiring / CubeMX names ======

// Example: suppose in CubeMX you named them T_CS and T_IRQ:
#define XPT2046_CS_GPIO_Port   GPIOD
#define XPT2046_CS_Pin         GPIO_PIN_7

#define XPT2046_IRQ_GPIO_Port  GPIOE
#define XPT2046_IRQ_Pin        GPIO_PIN_4

// ===============================================================

// ILI9488 resolution
#define LCD_WIDTH   480
#define LCD_HEIGHT  320

// Raw ADC calibration values -update these after measuring NOTE: They've been updated
#define XPT2046_X_MIN  350
#define XPT2046_X_MAX  3900
#define XPT2046_Y_MIN  350
#define XPT2046_Y_MAX  3800

// XPT2046 command bytes (12-bit conversion, single, power on)
#define XPT2046_CMD_X  0x94  // 1001 0010: measure X
#define XPT2046_CMD_Y  0xD4  // 1101 0010: measure Y

// SPI handle (from CubeMX)
extern SPI_HandleTypeDef hspi3;

// ---------- Chip select helpers ----------

static inline void xpt2046_select(void)
{
    HAL_GPIO_WritePin(XPT2046_CS_GPIO_Port, XPT2046_CS_Pin, GPIO_PIN_RESET);
}

static inline void xpt2046_deselect(void)
{
    HAL_GPIO_WritePin(XPT2046_CS_GPIO_Port, XPT2046_CS_Pin, GPIO_PIN_SET);
}

// ---------- Low-level ADC read ----------
//
// Sends a control byte (cmd) and reads a 12-bit ADC value.
// Returns 0..4095 (12-bit result).
//
static uint16_t xpt2046_read_adc(uint8_t cmd)
{
    uint8_t tx = cmd;
    uint8_t rx[2] = {0, 0};

    xpt2046_select();

    HAL_SPI_Transmit(&hspi3, &tx, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi3, rx, 2, HAL_MAX_DELAY);

    xpt2046_deselect();

    uint16_t value = ((uint16_t)rx[0] << 8) | rx[1];
    value >>= 3; // keep top 12 bits

    return value; // 0..4095
}

// ---------- Mapping raw -> screen coords ----------

static void convertADC(uint16_t *x, uint16_t *y)
{
    uint32_t rx = *x;
    uint32_t ry = *y;

    // Clamp to calibrated range
    if (rx < XPT2046_X_MIN) rx = XPT2046_X_MIN;
    if (rx > XPT2046_X_MAX) rx = XPT2046_X_MAX;
    if (ry < XPT2046_Y_MIN) ry = XPT2046_Y_MIN;
    if (ry > XPT2046_Y_MAX) ry = XPT2046_Y_MAX;

    // Map to [0, width-1], [0, height-1]
    rx = (rx - XPT2046_X_MIN) * (LCD_WIDTH  - 1) / (XPT2046_X_MAX - XPT2046_X_MIN);
    ry = (ry - XPT2046_Y_MIN) * (LCD_HEIGHT - 1) / (XPT2046_Y_MAX - XPT2046_Y_MIN);

    // If orientation is flipped or rotated, adjust here.
    // Example: flip X:
    // rx = (LCD_WIDTH - 1) - rx;
    //
    // Example: 90-degree rotation:
    // uint32_t tmp = rx;
    // rx = ry;
    // ry = (LCD_HEIGHT - 1) - tmp;

    *x = (uint16_t)rx;
    *y = (uint16_t)ry;
}

// ---------- Public: read one touch sample (screen coords) ----------
//
// Returns true if pressed, false if not pressed.
//
bool xpt2046_read_touch(uint16_t *x, uint16_t *y)
{
    // PENIRQ (T_IRQ): active LOW when pressed
    if (HAL_GPIO_ReadPin(XPT2046_IRQ_GPIO_Port, XPT2046_IRQ_Pin) == GPIO_PIN_SET) {
        return false; // no touch
    }

    const uint8_t samples = 3; // NOTE: Changed from 5 to 3
    uint32_t sx = 0;
    uint32_t sy = 0;

    // Average a few samples to reduce noise
    for (uint8_t i = 0; i < samples; i++) {
        sx += xpt2046_read_adc(XPT2046_CMD_X);
        sy += xpt2046_read_adc(XPT2046_CMD_Y);
    }

    uint16_t raw_x = (uint16_t)(sx / samples);
    uint16_t raw_y = (uint16_t)(sy / samples);

    convertADC(&raw_x, &raw_y);

    *x = raw_x;
    *y = raw_y;

    return true;
}

// ---------- Debug helper: print raw ADC values ----------
//
// Call this in a loop while watching UART.
// Use it to find X_MIN/X_MAX/Y_MIN/Y_MAX.
//
void xpt2046_test_raw_touch(void)
{

    if (HAL_GPIO_ReadPin(XPT2046_IRQ_GPIO_Port, XPT2046_IRQ_Pin) == GPIO_PIN_SET) {
        return;
    }

    const uint8_t samples = 5;
    uint32_t sx = 0;
    uint32_t sy = 0;

    for (uint8_t i = 0; i < samples; i++) {
        sx += xpt2046_read_adc(XPT2046_CMD_X);
        sy += xpt2046_read_adc(XPT2046_CMD_Y);
    }

    uint16_t rx = (uint16_t)(sx / samples);
    uint16_t ry = (uint16_t)(sy / samples);

    int z = 0;
}
