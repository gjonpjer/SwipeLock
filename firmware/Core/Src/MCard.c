/*
 * MCard.c
 *
 *  Created on: Nov 24, 2025
 *      Author: Thomas
 */

#include "MCard.h" // initializeMCard, MCard
#include "ctype.h" // tolower
#include "main.h" // HAL_NVIC_SystemReset
#include "stdbool.h" // bool, false, true
#include "stdint.h" // uint8_t
#include "stdio.h" // fprintf, stderr
#include "stdlib.h" // malloc
#include "string.h" // memcpy, strchr

bool initializeMCard(MCard *mCard, char *magneticCardReaderDataChars) {
	char *magneticCardReaderDataCharsLastNameBegin;
	char *magneticCardReaderDataCharsLastNameEnd;
	uint8_t magneticCardReaderDataCharsLastNameLength;

	if (strncmp(magneticCardReaderDataChars + 2, "600847", 6))
		return false;
	magneticCardReaderDataCharsLastNameBegin = strchr(magneticCardReaderDataChars, '^');

	if (magneticCardReaderDataCharsLastNameBegin == NULL) {
		return false;
	}

	++magneticCardReaderDataCharsLastNameBegin;
	mCard->umId = 0;
	for (char *magneticCardReaderDataCharsUmIdChar = magneticCardReaderDataCharsLastNameBegin - 11;
magneticCardReaderDataCharsUmIdChar != magneticCardReaderDataCharsLastNameBegin - 3; ++
magneticCardReaderDataCharsUmIdChar)
		mCard->umId = mCard->umId * 10 + (*magneticCardReaderDataCharsUmIdChar - '0');

	magneticCardReaderDataCharsLastNameEnd = strchr(magneticCardReaderDataCharsLastNameBegin, '/');

	if (magneticCardReaderDataCharsLastNameEnd == NULL) {
		return false;
	}

	mCard->firstInitial = *(magneticCardReaderDataCharsLastNameEnd + 1);
	magneticCardReaderDataCharsLastNameLength = magneticCardReaderDataCharsLastNameEnd -
magneticCardReaderDataCharsLastNameBegin;
//	mCard->lastName = malloc((magneticCardReaderDataCharsLastNameLength + 1) * sizeof(char));
//	if (!mCard->lastName) {
//		fprintf(stderr, "mCard->lastName == NULL, calling HAL_NVIC_SystemReset...\n");
//		HAL_NVIC_SystemReset();
//	} // if
	mCard->lastName[magneticCardReaderDataCharsLastNameLength] = '\0';
	memcpy(mCard->lastName, magneticCardReaderDataCharsLastNameBegin,
magneticCardReaderDataCharsLastNameLength);
	for (uint8_t mCardLastNameIndex = 1; mCardLastNameIndex <
magneticCardReaderDataCharsLastNameLength; ++mCardLastNameIndex)
		mCard->lastName[mCardLastNameIndex] = tolower(mCard->lastName[mCardLastNameIndex]);

	return true;
} // initializeMCard
