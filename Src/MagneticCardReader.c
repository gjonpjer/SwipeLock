/*
 * MagneticCardReader.c
 *
 *  Created on: Nov 24, 2025
 *      Author: Thomas
 */

#include "MagneticCardReader.h" /* MAGNETIC_CARD_READER_DATA_CAPACITY,
MAGNETIC_CARD_READER_DATA_CHARS_CAPACITY,
MAGNETIC_CARD_READER_DATA_READY_ELAPSED_MILLISECOND_THRESHOLD,
MAGNETIC_CARD_READER_DATA_SCAN_CODES_CAPACITY, MAGNETIC_CARD_READER_LEFT_SHIFT_PRESSED_TO_CHAR,
MAGNETIC_CARD_READER_LEFT_SHIFT_PRESSED_TO_CHAR_CAPACITY,
MAGNETIC_CARD_READER_LEFT_SHIFT_RELEASED_TO_CHAR,
MAGNETIC_CARD_READER_LEFT_SHIFT_RELEASED_TO_CHAR_CAPACITY, MagneticCardReader,
MagneticCardReaderDataCharsReadyCallback, initializeMagneticCardReader, readMagneticCardReaderData,
tryReadMagneticCardReaderData, tryWriteMagneticCardReaderData, writeMagneticCardReaderData */
#include "main.h" /* GPIO_PinState, GPIO_TypeDef, HAL_NVIC_SystemReset, HAL_TIM_ActiveChannel,
TIM_TypeDef */
#include "stdbool.h" // bool, false, true
#include "stdint.h" // int32_t, uint16_t, uint8_t
#include "stdio.h" // fprintf, stderr
#include "stdlib.h" // malloc

uint32_t temp_tick = 0;

char MAGNETIC_CARD_READER_LEFT_SHIFT_PRESSED_TO_CHAR[
MAGNETIC_CARD_READER_LEFT_SHIFT_PRESSED_TO_CHAR_CAPACITY] = {
	[0x15] = 'Q',
	[0x16] = '!',
	[0x1A] = 'Z',
	[0x1B] = 'S',
	[0x1C] = 'A',
	[0x1D] = 'W',
	[0x21] = 'C',
	[0x22] = 'X',
	[0x23] = 'D',
	[0x24] = 'E',
	[0x25] = '$',
	[0x26] = '#',
	[0x2A] = 'V',
	[0x2B] = 'F',
	[0x2C] = 'T',
	[0x2D] = 'R',
	[0x2E] = '%',
	[0x31] = 'N',
	[0x32] = 'B',
	[0x33] = 'H',
	[0x34] = 'G',
	[0x35] = 'Y',
	[0x36] = '^',
	[0x3A] = 'M',
	[0x3B] = 'J',
	[0x3C] = 'U',
	[0x3D] = '&',
	[0x3E] = '*',
	[0x41] = '<',
	[0x42] = 'K',
	[0x43] = 'I',
	[0x44] = 'O',
	[0x45] = ')',
	[0x46] = '(',
	[0x49] = '>',
	[0x4A] = '?',
	[0x4B] = 'L',
	[0x4C] = ':',
	[0x4D] = 'P',
	[0x52] = '"',
	[0x55] = '+',
}; // MAGNETIC_CARD_READER_LEFT_SHIFT_PRESSED_TO_CHAR

char MAGNETIC_CARD_READER_LEFT_SHIFT_RELEASED_TO_CHAR[
MAGNETIC_CARD_READER_LEFT_SHIFT_RELEASED_TO_CHAR_CAPACITY] = {
	[0x16] = '1',
	[0x1E] = '2',
	[0x25] = '4',
	[0x26] = '3',
	[0x29] = ' ',
	[0x2E] = '5',
	[0x36] = '6',
	[0x3D] = '7',
	[0x3E] = '8',
	[0x41] = ',',
	[0x45] = '0',
	[0x46] = '9',
	[0x49] = '.',
	[0x4A] = '/',
	[0x4C] = ';',
	[0x4E] = '-',
	[0x52] = '\'',
	[0x55] = '=',
	[0x5A] = '\n',
}; // MAGNETIC_CARD_READER_LEFT_SHIFT_RELEASED_TO_CHAR


void initializeMagneticCardReader(MagneticCardReader *magneticCardReader, GPIO_TypeDef *gpioX,
HAL_TIM_ActiveChannel halTimActiveChannel, MagneticCardReaderDataCharsReadyCallback
dataCharsReadyCallback, TIM_TypeDef *timX, uint16_t gpioPin) {
	magneticCardReader->data = malloc(MAGNETIC_CARD_READER_DATA_CAPACITY * sizeof(GPIO_PinState));
	if (!magneticCardReader->data) {
//		fprintf(stderr, "magneticCardReader->data == NULL, calling HAL_NVIC_SystemReset...\n");
	    HAL_NVIC_SystemReset();
	} // if
	magneticCardReader->dataChars = malloc(MAGNETIC_CARD_READER_DATA_CHARS_CAPACITY * sizeof(char));
	if (!magneticCardReader->dataChars) {
//		fprintf(stderr, "magneticCardReader->dataChars == NULL, calling HAL_NVIC_SystemReset...\n");
	    HAL_NVIC_SystemReset();
	} // if
	magneticCardReader->dataCharsReadyCallback = dataCharsReadyCallback;
	magneticCardReader->dataCharsSize = 0;
	magneticCardReader->dataScanCodes = malloc(MAGNETIC_CARD_READER_DATA_SCAN_CODES_CAPACITY *
sizeof(uint8_t));
	if (!magneticCardReader->dataScanCodes) {
//		fprintf(stderr, "magneticCardReader->dataScanCodes == NULL, calling HAL_NVIC_SystemReset...n");
	    HAL_NVIC_SystemReset();
	} // if
	magneticCardReader->dataScanCodesSize = 0;
	magneticCardReader->dataSize = 0;
	magneticCardReader->debounce = false;
	magneticCardReader->gpioPin = gpioPin;
	magneticCardReader->gpioX = gpioX;
	magneticCardReader->halTimActiveChannel = halTimActiveChannel;
	magneticCardReader->isReadingData = false;
	magneticCardReader->isWritingData = false;
	magneticCardReader->timX = timX;
} // initializeMagneticCardReader

void readMagneticCardReaderData(MagneticCardReader *magneticCardReader) {
	bool isLeftShiftPressed = false;
	bool isPreviousScanCodeBreak = false;

	magneticCardReader->isReadingData = true;
	for (uint16_t dataIndex = 0; dataIndex < magneticCardReader->dataSize; ++dataIndex) {
		uint8_t dataFrameIndex = dataIndex % 11;

		if (!dataFrameIndex) {
			uint16_t dataScanCodesIndex = dataIndex / 11;

			magneticCardReader->dataScanCodes[dataScanCodesIndex] = 0;
			++magneticCardReader->dataScanCodesSize;
		} else if (dataFrameIndex >= 1 && dataFrameIndex <= 8) {
			uint16_t dataScanCodesIndex = dataIndex / 11;

			magneticCardReader->dataScanCodes[dataScanCodesIndex] >>= 1;
			if (magneticCardReader->data[dataIndex])
				magneticCardReader->dataScanCodes[dataScanCodesIndex] |= 0x80;
		} // else if
	} // for
	for (uint16_t dataScanCodesIndex = 0; dataScanCodesIndex < magneticCardReader->dataScanCodesSize
; ++dataScanCodesIndex) {
		if (magneticCardReader->dataScanCodes[dataScanCodesIndex] == 0xF0) {
			isPreviousScanCodeBreak = true;
		} else {
			if (magneticCardReader->dataScanCodes[dataScanCodesIndex] == 0x12)
				isLeftShiftPressed = !isPreviousScanCodeBreak;
			else if (!isPreviousScanCodeBreak) {
				magneticCardReader->dataChars[magneticCardReader->dataCharsSize] =
isLeftShiftPressed ? MAGNETIC_CARD_READER_LEFT_SHIFT_PRESSED_TO_CHAR[magneticCardReader->
dataScanCodes[dataScanCodesIndex]] : MAGNETIC_CARD_READER_LEFT_SHIFT_RELEASED_TO_CHAR[
magneticCardReader->dataScanCodes[dataScanCodesIndex]];
				++magneticCardReader->dataCharsSize;
			} // else if
			if (isPreviousScanCodeBreak)
				isPreviousScanCodeBreak = false;
		} // else
	} // for
	magneticCardReader->dataCharsReadyCallback(magneticCardReader);
	magneticCardReader->dataCharsSize = 0;
	magneticCardReader->dataScanCodesSize = 0;
	magneticCardReader->dataSize = 0;
	magneticCardReader->debounce = true;
	magneticCardReader->isReadingData = false;
	magneticCardReader->isWritingData = false;
	magneticCardReader->previousDebounceTick = HAL_GetTick();
} // readMagneticCardReaderData

void tryReadMagneticCardReaderData(MagneticCardReader *magneticCardReader) {
	if (magneticCardReader->isWritingData && (int32_t) (HAL_GetTick() - magneticCardReader->
previousWriteDataTick) >= MAGNETIC_CARD_READER_DATA_READY_ELAPSED_MILLISECOND_THRESHOLD)
		readMagneticCardReaderData(magneticCardReader);
} // tryReadMagneticCardReaderData

void tryWriteMagneticCardReaderData(MagneticCardReader *magneticCardReader, HAL_TIM_ActiveChannel
halTimActiveChannel, TIM_TypeDef *timX) {
	if (halTimActiveChannel == magneticCardReader->halTimActiveChannel && timX == magneticCardReader
->timX && !magneticCardReader->isReadingData && !magneticCardReader->debounce)
		writeMagneticCardReaderData(magneticCardReader);
} // tryWriteMagneticCardReaderData

void writeMagneticCardReaderData(MagneticCardReader *magneticCardReader) {
	if (!magneticCardReader->isWritingData) {
		magneticCardReader->isWritingData = true;
	}
	magneticCardReader->data[magneticCardReader->dataSize] = HAL_GPIO_ReadPin(magneticCardReader->
gpioX, magneticCardReader->gpioPin);
	++magneticCardReader->dataSize;
	magneticCardReader->previousWriteDataTick = HAL_GetTick();
} // writeMagneticCardReaderData
