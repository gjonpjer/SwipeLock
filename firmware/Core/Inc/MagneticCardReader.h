/*
 * MagneticCardReader.h
 *
 *  Created on: Nov 23, 2025
 *      Author: Thomas
 */

#ifndef INC_MAGNETIC_CARD_READER_H_
#define INC_MAGNETIC_CARD_READER_H_
#define MAGNETIC_CARD_READER_DATA_CAPACITY 65536
#define MAGNETIC_CARD_READER_DATA_CHARS_CAPACITY 5957
#define MAGNETIC_CARD_READER_DATA_READY_ELAPSED_MILLISECOND_THRESHOLD 10
#define MAGNETIC_CARD_READER_DATA_SCAN_CODES_CAPACITY 5957
#define MAGNETIC_CARD_READER_LEFT_SHIFT_PRESSED_TO_CHAR_CAPACITY 256
#define MAGNETIC_CARD_READER_LEFT_SHIFT_RELEASED_TO_CHAR_CAPACITY 256
#include "main.h" // GPIO_PinState, GPIO_TypeDef, HAL_TIM_ActiveChannel, TIM_TypeDef
#include "stdbool.h" // bool
#include "stdint.h" // uint16_t, uint32_t, uint8_t

extern char MAGNETIC_CARD_READER_LEFT_SHIFT_PRESSED_TO_CHAR[
MAGNETIC_CARD_READER_LEFT_SHIFT_PRESSED_TO_CHAR_CAPACITY];
extern char MAGNETIC_CARD_READER_LEFT_SHIFT_RELEASED_TO_CHAR[
MAGNETIC_CARD_READER_LEFT_SHIFT_RELEASED_TO_CHAR_CAPACITY];
typedef struct MagneticCardReader MagneticCardReader;
typedef void (*MagneticCardReaderDataCharsReadyCallback)(MagneticCardReader *magneticCardReader);
typedef struct MagneticCardReader {
	char *dataChars;
	GPIO_TypeDef *gpioX;
	MagneticCardReaderDataCharsReadyCallback dataCharsReadyCallback;
	TIM_TypeDef *timX;
	uint8_t *dataScanCodes;
	volatile GPIO_PinState *data;
	volatile uint32_t previousDebounceTick;
	volatile uint32_t previousWriteDataTick;
	HAL_TIM_ActiveChannel halTimActiveChannel;
	uint16_t dataCharsSize;
	uint16_t dataScanCodesSize;
	uint16_t gpioPin;
	volatile uint16_t dataSize;
	bool debounce;
	bool isReadingData;
	volatile bool isWritingData;
} MagneticCardReader;
void initializeMagneticCardReader(MagneticCardReader *magneticCardReader, GPIO_TypeDef *gpioX,
HAL_TIM_ActiveChannel halTimActiveChannel, MagneticCardReaderDataCharsReadyCallback
dataCharsReadyCallback, TIM_TypeDef *timX, uint16_t gpioPin);
void readMagneticCardReaderData(MagneticCardReader *magneticCardReader);
void tryReadMagneticCardReaderData(MagneticCardReader *magneticCardReader);
void tryWriteMagneticCardReaderData(MagneticCardReader *magneticCardReader, HAL_TIM_ActiveChannel
halTimActiveChannel, TIM_TypeDef *timX);
void writeMagneticCardReaderData(MagneticCardReader *magneticCardReader);

#endif /* INC_MAGNETIC_CARD_READER_H_ */
