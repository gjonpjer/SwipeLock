/*
 * SwipeLock.h
 *
 *  Created on: Nov 23, 2025
 *      Author: Thomas
 */

#ifndef INC_SWIPE_LOCK_H_
#define INC_SWIPE_LOCK_H_
#define OCCUPIED_SWIPE_LOCK_STATE_MILLISECOND_DURATION 60000 // 60,000
#include "MCard.h" // MCard
#include "MagneticCardReader.h" // MagneticCardReader
#include "main.h" // GPIO_TypeDef, HAL_TIM_ActiveChannel, TIM_TypeDef
#include "stdbool.h" // bool
#include "stdint.h" // uint16_t, uint32_t

typedef enum SwipeLockState {
	AbandonedSwipeLockState,
	OccupiedSwipeLockState,
	StartupSwipeLockState,
	VacantSwipeLockState,
} SwipeLockState;

typedef struct SwipeLock {
	MagneticCardReader magneticCardReader;
	MCard occupant;
	uint32_t becameOccupiedTick;
	SwipeLockState state;
} SwipeLock;

bool swipeLock_has_pending_occupy_swipe(void);
void swipeLock_apply_pending_occupy_swipe(void);


bool isSwipeLockAbandonable(SwipeLock *swipeLock);
extern SwipeLock swipeLock;
void abandonSwipeLock(SwipeLock *swipeLock);
void initializeSwipeLock(SwipeLock *swipeLock, GPIO_TypeDef *gpioX, HAL_TIM_ActiveChannel
halTimActiveChannel, TIM_TypeDef *timX, uint16_t gpioPin);
void occupySwipeLock(SwipeLock *swipeLock, MCard *newSwipeLockOccupant);
void swipeLockMagneticCardReaderDataCharsReadyCallback(MagneticCardReader *magneticCardReader);
void tryAbandonSwipeLock(SwipeLock *swipeLock);
void vacateSwipeLock(SwipeLock *swipeLock);

#endif /* INC_SWIPE_LOCK_H_ */
