/*
 * SwipeLock.c
 *
 *  Created on: Nov 23, 2025
 *      Author: Thomas
 */

#include "main.h" // HAL_GetTick
#include "displayFunctions.h"

//#include "LockMechanism.h"
#include "MagneticCardReader.h"
#include "MCard.h" // MCard, initializeMCard
#include "SwipeLock.h" /* SwipeLock, abandonSwipeLock, initializeSwipeLock,
isSwipeLockAbandonable, occupySwipeLock, vacateSwipeLock */

#include <string.h>
#include "stdbool.h" // bool, false, true
#include "stdint.h" // int32_t
#include "stdlib.h" // free

// december 8 2025
static bool pending_swipe_to_occupy = false;
static MCard pending_mcard;

bool swipeLock_has_pending_occupy_swipe(void)
{
    return pending_swipe_to_occupy;
}

void swipeLock_apply_pending_occupy_swipe(void)
{
    if (!pending_swipe_to_occupy) return;

    pending_swipe_to_occupy = false;

    /* Use this card as the new occupant and go to time-selection */
    current_occupant = pending_mcard;
    display_time_duration_screen();
}

SwipeLock swipeLock;
void abandonSwipeLock(SwipeLock *swipeLock) {
	swipeLock->state = AbandonedSwipeLockState;
} // abandonSwipeLock

void initializeSwipeLock(SwipeLock *swipeLock, GPIO_TypeDef *gpioX, HAL_TIM_ActiveChannel
halTimActiveChannel, TIM_TypeDef *timX, uint16_t gpioPin) {
	initializeMagneticCardReader(&swipeLock->magneticCardReader, gpioX, halTimActiveChannel, &
swipeLockMagneticCardReaderDataCharsReadyCallback, timX, gpioPin);
	swipeLock->state = VacantSwipeLockState;
} // initializeSwipeLock

bool isSwipeLockAbandonable(SwipeLock *swipeLock) {
	if ((int32_t) (HAL_GetTick() - swipeLock->becameOccupiedTick) >= OCCUPIED_SWIPE_LOCK_STATE_MILLISECOND_DURATION)
		return true;
	else
		return false;
} // isSwipeLockAbandonable

void occupySwipeLock(SwipeLock *swipeLock, MCard *newSwipeLockOccupant) {
//	swipeLock->becameOccupiedTick = HAL_GetTick();
//	swipeLock->occupant.firstInitial = newSwipeLockOccupant->firstInitial;
//	swipeLock->occupant.lastName = newSwipeLockOccupant->lastName;
//	newSwipeLockOccupant->lastName = NULL;
//	swipeLock->occupant.umId = newSwipeLockOccupant->umId;
//	swipeLock->state = OccupiedSwipeLockState;
} // occupySwipeLock

// Callback functio at the end of every swipe
void swipeLockMagneticCardReaderDataCharsReadyCallback(MagneticCardReader *magneticCardReader) {
	// Early return if swipe occurs on startup
	if (current_display_state == LOCKER_STATE_STARTUP) {
		return;
	}

	// Initialize MCard, handle the rest of the states
	MCard mCard;
	bool isMCardInitialized = initializeMCard(&mCard, magneticCardReader->dataChars);

//	// TODO handle bad mcards
//	if (isMCardInitialized) {
//
//		if (in_admin_card_menu) {
//			admin_add_master_card(&mCard);
//		} else {
//			switch (current_display_state) {
//			case LOCKER_STATE_ABANDONED:
//			case LOCKER_STATE_VACANT: {
//				// Locker is vacant, transition to the occupied locker
//
//				current_occupant = mCard;
//				display_time_duration_screen();
////				display_occupied(current_occupant.firstInitial, current_occupant.lastName);
//				break;
//			}
//			case LOCKER_STATE_OCCUPIED: {
//
//				// User or admin unlocking their locker, switch to vacant,
//				if (mCard.umId == current_occupant.umId || is_admin(&mCard)) {
//					display_vacant();
//				} else { // erroneous user
//					ui_show_status_msg("This is not your SwipeLock!", 3000);
//				}
//
//				break;
//			}
//			default:
//				break;
//			}
//		}
//	} else {
//		ui_show_status_msg("Please swipe a valid MCard.", 3000);
//	}
    if (!isMCardInitialized) {
        ui_show_status_msg("Please swipe a valid MCard.", 3000);
        return;
    }

    /* 1) Admin card menu: add manager card, do nothing else */
    if (in_admin_card_menu) {
        admin_add_master_card(&mCard);
        return;
    }

    /* 2) Admin control panel or PIN screen:
     *    - Don’t switch screens now.
     *    - If locker is vacant/abandoned, remember this swipe
     *      so we can start the timer-selection after admin presses Back.
     */
    if (in_admin_main_menu || in_admin_pin_menu) {
        if (current_display_state == LOCKER_STATE_VACANT ||
            current_display_state == LOCKER_STATE_ABANDONED) {
            pending_swipe_to_occupy = true;
            pending_mcard = mCard;
        }
        /* Ignore unlock-style swipes while admin is open for now */
        return;
    }

    /* 3) Normal behavior when not in any admin UI */
    switch (current_display_state) {
    case LOCKER_STATE_ABANDONED:
    case LOCKER_STATE_VACANT: {
        // Locker is vacant, transition to timer selection
        current_occupant = mCard;
        display_time_duration_screen();
        break;
    }
    case LOCKER_STATE_OCCUPIED: {
        // User or admin unlocking their locker
        if (mCard.umId == current_occupant.umId || is_admin(&mCard)) {
        	trigger_timed_unlock();
            display_vacant();
        } else {
            ui_show_status_msg("This is not your SwipeLock!", 3000);
        }
        break;
    }
    default:
        break;
    }
} // swipeLockMagneticCardReaderDataCharsReadyCallback

void tryAbandonSwipeLock(SwipeLock *swipeLock) {
	if (isSwipeLockAbandonable(swipeLock))
		abandonSwipeLock(swipeLock);
} // tryAbandonSwipeLock

void vacateSwipeLock(SwipeLock *swipeLock) {
//	free(swipeLock->occupant.lastName);
	swipeLock->state = VacantSwipeLockState;
} // vacateSwipeLock
