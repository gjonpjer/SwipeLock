/*
 * displayFunctions.h
 *
 *  Created on: Nov 24, 2025
 *      Author: gjonpjer
 */

#ifndef INC_DISPLAYFUNCTIONS_H_
#define INC_DISPLAYFUNCTIONS_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "MCard.h"
#include "lvgl.h"

typedef enum {
	LOCKER_STATE_NULL,
	LOCKER_STATE_STARTUP,
	LOCKER_STATE_VACANT,
	LOCKER_STATE_OCCUPIED,
	LOCKER_STATE_ABANDONED
} locker_state_t;

typedef enum {
	ADMIN_UNCONFIGURED,
	ADMIN_UNLOCKED_CONFIGURED,
	ADMIN_LOCKED_CONFIGURED
} admin_state_t;

typedef enum {
	ADMIN_PIN_MODE_SET_CHANGE,
	ADMIN_PIN_MODE_UNLOCK
} admin_pin_mode_t;

extern MCard current_occupant;
extern locker_state_t current_display_state;
extern bool display_startup_done;
extern bool in_admin_card_menu;

extern bool in_admin_pin_menu;
extern bool in_admin_main_menu;

// ======= Functions for the final product ======= //

void trigger_timed_unlock(void);

/*
 * Initializes the startup screen of the display
 *
 * Displays "SwipeLock, tap anywhere to enter"
 * the display fades in and out
 *
 * When pressed, the display transitions to the vacant screen
 *
 * The startup display will never show again after the user interacts
 * with the screen
 */
void display_startup(void);

/*
 * Updates the display by fading in and out certain parts of the text
 * When the user interacts with the screen, or a swipe occurs, then
 * the display will transition either to vacant or occupied
 */
void update_startup(void);

/*
 * Helper function for the update_startup_screen func
 */
void gui_wait_ms(uint32_t ms);

/*
 * Displays to the user that the locker is now vacant, and is ready
 * to be occupied
 */
void display_vacant(void);

/*
 * Displays on screen user title
 * first initial, last name
 *
 * When occupied, a widget/button will be in the bottom center
 * that says "Vacate locker"
 *
 * The widget enables the user to vacate their locker if they wish to
 * do so before their allocated time is up
 *
 * Top right will have a countdown timer.
 * The locker will allot the amount of time allowed for the user on this locker
 *
 * TIME ALLOWED: 01:00 (1 minute)
 *
 * When the clock hits 00:00, then the locker's state will become abandoned
 */
void display_occupied(char f_i, const char* l_n);

/*
 * Displays on screen which user abandoned the locker
 * Underneath the name of previous occupant, it will say
 * "Swipe M-Card to occupy"
 */
void display_abandoned(char f_i, const char* l_n);


/*
 * Adds a master card to the system.
 *
 * Must check to see if admin is in the card manager window before addming the MCard to the
 * list of admin cards
 */
void admin_add_master_card(const MCard* card);


/*
 * Returns true if the MCard is actually an admin user
 */
bool is_admin(const MCard* card);


/*
 * When a user swipes an erroneous or invalid MCard, then a message
 * will show up on the display with the following message
 *
 * "Please swipe a valid MCard!"
 */
void print_bad_card(void);

/*
 * When a locker is already occupied
 */
void print_locker_in_use(void);

/*
 * Updates the small label on the vacant, occupied, and abandoned screen for a brief amount of time.
 */
void ui_show_status_msg(const char* msg, uint32_t duration_ms);

/*
 * When a user swipes to occupy a locker, they will first be prompted to ask
 * what
 */
void display_time_duration_screen(void);

// ======== Test functions to ensure the display works ======= //

void start_touch_color_test(void);

void printHelloWorld();
void printGoodByeWorld();

void buttonExample(void);

void fade_text(const char *txt, const lv_font_t* font);

#endif /* INC_DISPLAYFUNCTIONS_H_ */
