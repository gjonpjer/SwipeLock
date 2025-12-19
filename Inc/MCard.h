/*
 * MCard.h
 *
 *  Created on: Nov 23, 2025
 *      Author: Thomas
 */

#ifndef INC_M_CARD_H_
#define INC_M_CARD_H_
#include "stdbool.h" // bool
#include "stdint.h" // uint32_t

typedef struct MCard {
	char lastName[21];
	uint32_t umId;
	char firstInitial;
} MCard;
bool initializeMCard(MCard *mCard, char *magneticCardReaderDataChars);

#endif /* INC_M_CARD_H_ */
