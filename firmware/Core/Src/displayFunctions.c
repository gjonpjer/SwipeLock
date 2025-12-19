
/***********************
 * Stdlib includes
 ***********************/
#include <stdio.h>
#include <stdbool.h>
/***********************
 * General includes
 ***********************/
#include "stm32l4xx_hal.h"
#include "LockMechanism.h"
#include "Audio.h"
#include "MCard.h"
#include "SwipeLock.h"
#include "displayFunctions.h"
#include "lvgl.h"

/***********************
 * Defines
 ***********************/
#define MICHIGAN_BLUE 	0x00274C
#define MICHIGAN_MAIZE	0xFFCB05

#define MAX_MASTER_CARDS 8 // they won't even know lol

/***********************
 * Variables
 ***********************/

// Declare images of Matt, James, Carl
LV_IMAGE_DECLARE(james_img);
LV_IMAGE_DECLARE(matt_img);
LV_IMAGE_DECLARE(samples_headshot);

/* Variable to keep track of the occupants image if it is one of the 3 above */
static lv_obj_t* occ_face_img = NULL;

/*
 * MCard struct variable to keep track of current occupant
 */
MCard current_occupant;

/*
 * Boolean to stop startup from updating after user taps
 */
bool display_startup_done = false;

/*
 * public boolean to know when to an MCard to the admin cards
 */
bool in_admin_card_menu = false;

// December 8 2025
bool in_admin_main_menu = false;
bool in_admin_pin_menu 	= false;


/*
 * Display state
 */
locker_state_t current_display_state = LOCKER_STATE_STARTUP;

/*
 * Admin state
 */
static admin_state_t admin_state = ADMIN_UNCONFIGURED;

/*
 * Admin pin state
 */
static admin_pin_mode_t admin_pin_mode = ADMIN_PIN_MODE_SET_CHANGE;

/*
 * LVGL screens for each state, this will decrease latency
 * since when transitioning to a different screen you
 * won't need to start from fresh, you can just load the previous screen
 */
static lv_obj_t* vacant_scr 	= NULL;
static lv_obj_t* occupied_scr 	= NULL;
static lv_obj_t* abandoned_scr 	= NULL;

// December 8, 2025
/*
 * Pointer to the select time duration screen, occurs before occupied
 */
static lv_obj_t* time_duration_scr = NULL;

/*
 * Buttons for small, medium, long time
 */
static lv_obj_t* time_btn_s = NULL;
static lv_obj_t* time_btn_m = NULL;
static lv_obj_t* time_btn_l = NULL;


/*
 * Occupied-screen widgets that will be updated
 */
static lv_obj_t* occ_name 		= NULL;

/*
 * Occupied countdown state
 */
static lv_obj_t  	*occ_countdown_label = NULL;
static lv_timer_t 	*occ_countdown_timer = NULL;
static int32_t     	occ_seconds_left = 0;
static bool        	occ_countdown_active = false;

/*
 * Abandoned screen label
 */
static lv_obj_t* abandoned_header = NULL;

/*
 * Admin menu button
 */
static lv_obj_t* admin_btn = NULL;

/*
 * Admin screen and widgets
 */
static lv_obj_t* admin_scr 			= NULL;
static lv_obj_t* admin_pin_btn 		= NULL;
static lv_obj_t* admin_card_btn 	= NULL;
static lv_obj_t* admin_back_btn 	= NULL;
static lv_obj_t* admin_status_label = NULL;

/*
 * Admin control panel 4 digit code page
 */
static lv_obj_t* admin_pin_title		= NULL;
static lv_obj_t* admin_pin_scr			= NULL;
static lv_obj_t* admin_pin_dots[4]  	= {NULL};
static lv_obj_t* admin_pin_btnm 		= NULL;
static lv_obj_t* admin_pin_back_btn 	= NULL;

/* PIN state */
static uint8_t  admin_pin_digits[4] = {0};
static uint8_t  admin_pin_len       = 0;
static bool     admin_pin_set       = false;
static uint16_t admin_pin_code      = 0;    // 0000..9999 stored as integer

/*
 * Admin card variables
 */
static MCard master_cards[MAX_MASTER_CARDS];
static uint8_t master_card_sz = 0;

static lv_obj_t* admin_card_scr = NULL;
static lv_obj_t* admin_card_table = NULL;
static lv_obj_t* admin_card_back_btn = NULL;

/* Optional: track whether we're still on the intro screen */
static bool intro_active = true;

static uint8_t intro_state = 0;
static uint32_t intro_ts = 0;

static lv_obj_t *heading; // used in display_startup
static lv_obj_t *subheading; // used in display_startup

/*
 * Subtitle / status labels for vacant, occupied, and abandoned
 */
static lv_obj_t* vacant_msg_label    = NULL;
static lv_obj_t* occupied_msg_label  = NULL;
static lv_obj_t* abandoned_msg_label = NULL;

/* Transient status message timer */
static lv_timer_t* status_msg_timer = NULL;

/* Remember which screen we showed the message on */
static locker_state_t status_msg_state;

/* timer variable to protect the door latch (Solenoid) */
static lv_timer_t* lock_timer = NULL;

/********************************
 * Private function declarations
 ********************************/

static lv_obj_t* create_back_button(lv_obj_t* parent, lv_event_cb_t cb);

static void init_admin_btn(void);

static void startup_screen_event_cb(lv_event_t * e);
static void btn_event_cb(lv_event_t * e);

static void admin_btn_event_cb(lv_event_t * e);
static void admin_back_btn_event_cb(lv_event_t * e);
static void display_admin_menu(void);

static void init_time_duration_screen(void);

/*
 * Admin pin functions
 */
static void admin_pin_back_btn_event_cb(lv_event_t* e);
static void admin_pin_btn_event_cb(lv_event_t *e);
static void admin_pin_btnm_event_cb(lv_event_t *e);
static void admin_pin_update_dots(void);
static void admin_pin_clear_all(void);
static void admin_pin_append_digit(uint8_t d);
static void admin_pin_enter(void);
static void init_display_admin_pin(void);
static void display_admin_pin(void);

/*
 * Admin card functions
 */
static void admin_card_btn_event_cb(lv_event_t * e);
static void admin_card_back_btn_event_cb(lv_event_t * e);
static void init_display_admin_cards(void);
static void display_admin_cards(void);
static void master_cards_table_refresh(void);
static void master_cards_table_event_cb(lv_event_t * e);
static void master_cards_table_draw_event_cb(lv_event_t * e);

static void occupied_update_countdown_label(void);

static void return_to_locker_screen(void);

/*
 * status message function callback
 */
static void status_msg_timer_cb(lv_timer_t* t);

/*
 * Time duration function callback
 */
static void time_duration_btn_event_cb(lv_event_t * e);

/*
 * Timer functions for the door latch (solenoid)
 */
static void lock_timer_cb(lv_timer_t* t);
void trigger_timed_unlock(void);

/***********************
 * Callback functions
 ***********************/

/* Callback to latch the door when after the door stays open for 5-10 seconds */
static void lock_timer_cb(lv_timer_t* t)
{
	latch_door();
	lock_timer = NULL;
}

/* Callback to update the amount of time the occupant chose for the locker */
static void time_duration_btn_event_cb(lv_event_t * e)
{
    lv_obj_t *btn = lv_event_get_target_obj(e);

    // sound like other UI actions
    playSound();

    /* Map S/M/L -> seconds
       S ->  1:00  =  60
       M ->  2:30  = 150
       L ->  5:00  = 300
    */
    if (btn == time_btn_s) {
        occ_seconds_left = 60;
    } else if (btn == time_btn_m) {
        occ_seconds_left = 150;
    } else if (btn == time_btn_l) {
        occ_seconds_left = 300;
    } else {
        return;
    }
    /* transition to occupied screen for the current_occupant */
    trigger_timed_unlock(); // only unlocks once for occupied
    display_occupied(current_occupant.firstInitial, current_occupant.lastName);
}

/* Callback to update the status message label for vacant, occupied, abandoned */
static void status_msg_timer_cb(lv_timer_t* t)
{
	 LV_UNUSED(t);

	lv_obj_t *label = NULL;
	const char *default_txt = NULL;

	switch (status_msg_state) {
	case LOCKER_STATE_VACANT:
		label = vacant_msg_label;
		default_txt = "(Swipe MCard to occupy)";
		break;
	case LOCKER_STATE_OCCUPIED:
		label = occupied_msg_label;
		default_txt = "(Swipe MCard to vacate)";
		break;
	case LOCKER_STATE_ABANDONED:
		label = abandoned_msg_label;
		default_txt = "(Swipe MCard to occupy)";
		break;
	default:
		break;
	}

	if (label && default_txt) {
		lv_label_set_text(label, default_txt);
	}

	/* One-shot: destroy timer */
	if (status_msg_timer) {
		lv_timer_del(status_msg_timer);
		status_msg_timer = NULL;
	}
}

/* Callback to draw the master cards table. */
static void master_cards_table_draw_event_cb(lv_event_t* e)
{
	lv_draw_task_t * draw_task    = lv_event_get_draw_task(e);
	if (!draw_task) return;

	lv_draw_dsc_base_t * base_dsc = (lv_draw_dsc_base_t *)lv_draw_task_get_draw_dsc(draw_task);
	if (!base_dsc) return;

	/* Only touch table items (cells) */
	if (base_dsc->part != LV_PART_ITEMS) return;

	uint32_t row = base_dsc->id1;
	uint32_t col = base_dsc->id2;

	lv_draw_fill_dsc_t  * fill_dsc  = lv_draw_task_get_fill_dsc(draw_task);
	lv_draw_label_dsc_t * label_dsc = lv_draw_task_get_label_dsc(draw_task);
	lv_draw_border_dsc_t* border_dsc = lv_draw_task_get_border_dsc(draw_task);

	/* Header row styling (row 0) */
	if (row == 0) {
		if (label_dsc) {
			label_dsc->align = LV_TEXT_ALIGN_CENTER;
			label_dsc->color = lv_color_hex(MICHIGAN_BLUE);   /* text on maize */
		}
		if (fill_dsc) {
			fill_dsc->color = lv_color_hex(MICHIGAN_MAIZE);
			fill_dsc->opa   = LV_OPA_COVER;
		}
		if (border_dsc) {
			border_dsc->color = lv_color_hex(MICHIGAN_BLUE);
			border_dsc->opa   = LV_OPA_COVER;
		}
		return;
	}

	/* Body rows (row > 0): white / light-grey striping */
	if (fill_dsc) {
		if (row % 2 == 1) {
			/* odd rows: white */
			fill_dsc->color = lv_color_hex(0xFFFFFF);
		} else {
			/* even rows: light grey */
			fill_dsc->color = lv_color_hex(0xF0F0F0);
		}
		fill_dsc->opa = LV_OPA_COVER;
	}

	/* Default text color for name & header is Michigan blue.
	 * We only override for delete column. */
	if (label_dsc) {
		label_dsc->font = &lv_font_montserrat_18;   // <--- use 22 if needed
		label_dsc->color = lv_color_hex(MICHIGAN_BLUE);
	}

    /* --- Center names in column 0 --- */
    if (col == 0 && row > 0) {
        if (label_dsc) {
            label_dsc->align = LV_TEXT_ALIGN_CENTER;   // center horizontally
        }
    }

	/* Delete column (col = 1, non-header): red minus centered */
	if (col == 1) {
		if (label_dsc) {
			label_dsc->color = lv_color_hex(0xB00000);          /* red minus */
			label_dsc->align = LV_TEXT_ALIGN_CENTER;            /* center icon */
		}
	}
}

/* Click handler for table: clicking delete symbol removes that key */
static void master_cards_table_event_cb(lv_event_t * e)
{
    lv_obj_t * table = lv_event_get_target_obj(e);

    uint32_t row = 0, col = 0;
    lv_table_get_selected_cell(table, &row, &col);

    /* If nothing is selected, LVGL sets row/col to LV_TABLE_CELL_NONE (0xFFFF) */
	if (row == LV_TABLE_CELL_NONE || col == LV_TABLE_CELL_NONE)
		return;

    /* Ignore header row and name column */
    if (row == 0 || col != 1)
        return;

    uint16_t idx = (uint16_t)(row - 1); /* row 1 corresponds to master_keys[0] */

    if (idx >= master_card_sz)
        return;

    /* Remove this entry by shifting others up */
    for (uint16_t i = idx; i + 1 < master_card_sz; ++i) {
        master_cards[i] = master_cards[i + 1];
    }
    master_card_sz--;

    master_cards_table_refresh();
}

/* Callback function for the admin card button */
static void admin_card_btn_event_cb(lv_event_t* e)
{
	// december 8 2025
	in_admin_card_menu = true;
	in_admin_main_menu = false;
	in_admin_pin_menu  = false;

	playSound();

	display_admin_cards();
}

/* Callback for the admin card menu back button, sends user back to the admin main menu*/
static void admin_card_back_btn_event_cb(lv_event_t* e)
{
	in_admin_card_menu = false;
	playSound();
	display_admin_menu();
}

/* Callback function for the countdown timer on the occupied screen */
static void occupied_countdown_cb(lv_timer_t * t)
{
    LV_UNUSED(t);

    if (!occ_countdown_active) return;
    if (occ_seconds_left <= 0) return;

    occ_seconds_left--;

    // IMPORTANT: Prevent a hardware fault from the card reader being interrupted by the LVGL display
    if (!(swipeLock.magneticCardReader.isReadingData || swipeLock.magneticCardReader.isWritingData)) {
    	occupied_update_countdown_label();
    }

    if (occ_seconds_left == 0) {
        occ_countdown_active = false;

        lv_timer_pause(occ_countdown_timer); // pause the timer

        current_display_state = LOCKER_STATE_ABANDONED;

        lv_obj_t* active_scr = lv_scr_act();

        // unlock the door when time runs out
//        unlatch_door();
        trigger_timed_unlock();

        // TODO later: auto-vacate, change color, show message, etc.
        if (active_scr == occupied_scr) {
        	display_abandoned(current_occupant.firstInitial, current_occupant.lastName);
        }
    }
}

/* Callback for the admin pin keypad */
static void admin_pin_btnm_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) return;

    lv_obj_t * obj = lv_event_get_target_obj(e);
    int32_t id = lv_buttonmatrix_get_selected_button(obj);
    if (id < 0) return;

    const char * txt = lv_buttonmatrix_get_button_text(obj, id);
    if (!txt || txt[0] == '\0') return;

  	playSound();
    /* "-" = clear all */
    if (lv_strcmp(txt, "-") == 0) {
        admin_pin_clear_all();
    }
    /* Right arrow = enter */
    else if (lv_strcmp(txt, LV_SYMBOL_RIGHT) == 0) {
        admin_pin_enter();
    }
    /* Digits 0-9 */
    else if (txt[0] >= '0' && txt[0] <= '9' && txt[1] == '\0') {
        uint8_t d = (uint8_t)(txt[0] - '0');
        admin_pin_append_digit(d);
    }
}

/* Callback for the admin pin page back button */
static void admin_pin_back_btn_event_cb(lv_event_t* e)
{
	playSound();
	if (admin_pin_mode == ADMIN_PIN_MODE_UNLOCK) {
		/* User cancelled the unlock attempt, should return back to original screen */
//		return_to_locker_screen();

		// December 8 2025
		/* User cancelled unlock and is exiting admin UI */
		in_admin_main_menu = false;
		in_admin_card_menu = false;

		if (swipeLock_has_pending_occupy_swipe()) {
			swipeLock_apply_pending_occupy_swipe();
		} else {
			return_to_locker_screen();
		}
	} else {
		/* User is in the panel already, should return back to admin menu*/
		display_admin_menu();
	}
}

/* Callback for the admin pin setup screen */
static void admin_pin_btn_event_cb(lv_event_t* e)
{
	playSound();
	admin_pin_mode = ADMIN_PIN_MODE_SET_CHANGE;
	display_admin_pin();
}

/* Callback for admin button press, will open the admin menu */
static void admin_btn_event_cb(lv_event_t* e)
{
	playSound();
	/* Determine which screen to go to based on configuration */
	switch (admin_state) {
	case ADMIN_UNCONFIGURED: // no authentication
		/* Go to admin menu screen */
		display_admin_menu();
		break;

	case ADMIN_LOCKED_CONFIGURED: // panel configured, requires pin to enter
		admin_pin_mode = ADMIN_PIN_MODE_UNLOCK;
		display_admin_pin();
		break;

	case ADMIN_UNLOCKED_CONFIGURED: // shouldn't happen
	default:
		display_admin_menu();
		break;
	}
}

/* Callback function for admin back button, will return back to previous screen */
static void admin_back_btn_event_cb(lv_event_t* e)
{
	if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;

	/* If a pin has been set, lock the admin panel when leaving */
	if (admin_pin_set)
		admin_state = ADMIN_LOCKED_CONFIGURED;

	playSound();

	// December 8 2025
	/* We are leaving the admin UI */
	in_admin_main_menu = false;
	in_admin_pin_menu  = false;
	in_admin_card_menu = false;

	if (swipeLock_has_pending_occupy_swipe()) {
		/* A user swiped while admin was open: go straight to time selection */
		swipeLock_apply_pending_occupy_swipe();
	} else {
		/* Normal behavior: return to VACANT/OCCUPIED/ABANDONED screen */
		return_to_locker_screen();
	}

//	return_to_locker_screen();
}

/* Callback for startup, occurs when the user taps the startup screen */
static void startup_screen_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code != LV_EVENT_PRESSED) return;

    // stop the startup animation logic
    intro_active = false;
    display_startup_done = true;

    // Update the UI state
    current_display_state = LOCKER_STATE_VACANT;

    // transition to the vacant screen
    trigger_timed_unlock();
    display_vacant();
}

// Callback for the button example (NOT USED IN PROJECT)
static void btn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target_obj(e);

    if(code == LV_EVENT_CLICKED) {
        static uint8_t cnt = 0;
        cnt++;

        /*Get the first child of the button which is the label and change its text*/
        lv_obj_t * label = lv_obj_get_child(btn, 0);
        lv_label_set_text_fmt(label, "Button: %d", cnt);
    }
}

/***********************
 * Private helper functions
 ***********************/
//
//static void image_of_james(void)
//{
//	LV_IMAGE_DECLARE(james_img);
//	lv_obj_t* img;
//
//	img = lv_image_create(lv_screen_active());
//	lv_image_set_src(img, &james_img);
//	lv_obj_align(img, LV_ALIGN_BOTTOM_LEFT, 0, 0);
//}
//
//static void image_of_matt(void)
//{
//	LV_IMAGE_DECLARE(matt_img);
//	lv_obj_t* img;
//
//	img = lv_image_create(lv_screen_active());
//	lv_image_set_src(img, &matt_img);
//	lv_obj_align(img, LV_ALIGN_BOTTOM_LEFT, 0, 0);
//}
//
///*
// * Literally an image of professor Sample
// * Use it when Sample abandoned or occupied locker
// */
//
//static void image_of_sample(void)
//{
//    LV_IMAGE_DECLARE(samples_headshot);
//    lv_obj_t * img;
//
//    img = lv_image_create(lv_screen_active());
//    lv_image_set_src(img, &samples_headshot);
////    lv_obj_align(img, LV_ALIGN_DEFAULT, 20, 0);
//    lv_obj_align(img, LV_ALIGN_BOTTOM_LEFT, 0, 0);
//}

/*
 * Function to open the door for X amount of time (5 seconds) and then locks the door after
 * Meant to protect the solenoid from overheating
 */
void trigger_timed_unlock(void)
{
	unlatch_door(); // Turn solenoid ON (Open/Unlock)

	// If a timer is already running, reset it (extend the open time)
	if (lock_timer != NULL) {
		lv_timer_reset(lock_timer);
	} else {
		// Create a new timer for 5000ms (5 seconds)
		// Adjust 5000 to 10000 if you want 10 seconds
		lock_timer = lv_timer_create(lock_timer_cb, 5000, NULL);
		lv_timer_set_repeat_count(lock_timer, 1); // Run only once
		lv_timer_set_auto_delete(lock_timer, true); // Delete after running
	}
}

/*
 * Helper function to create the time selection buttons
 * Will create a button with the timer value on it
 */
static lv_obj_t* create_time_button(lv_obj_t* parent, const char* time_text)
{
	lv_obj_t *btn = lv_btn_create(parent);
	lv_obj_set_size(btn, 110, 90);
	lv_obj_add_event_cb(btn, time_duration_btn_event_cb, LV_EVENT_CLICKED, NULL);

	/* Maize background */
	lv_obj_set_style_bg_color(btn, lv_color_hex(MICHIGAN_MAIZE), 0);
	lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
	lv_obj_set_style_radius(btn, 10, 0);

	/* Vertical layout inside */
	lv_obj_set_flex_flow(btn, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(btn,
						  LV_FLEX_ALIGN_CENTER,
						  LV_FLEX_ALIGN_CENTER,
						  LV_FLEX_ALIGN_CENTER);

	/* Time label only */
	lv_obj_t *lbl_time = lv_label_create(btn);
	lv_label_set_text(lbl_time, time_text);
	lv_obj_set_style_text_color(lbl_time, lv_color_white(), 0);
	lv_obj_set_style_text_font(lbl_time, &lv_font_montserrat_28, 0);
	    // larger font since it's the only text

	return btn;
}

static void init_time_duration_screen(void)
{
	if (time_duration_scr != NULL) {
	        return; // already created
	}

	time_duration_scr = lv_obj_create(NULL);

	lv_obj_set_style_bg_color(time_duration_scr, lv_color_hex(MICHIGAN_BLUE), 0);
	lv_obj_set_style_bg_opa(time_duration_scr, LV_OPA_COVER, 0);

	/* Title label: "Select Time Duration" */
	lv_obj_t *title = lv_label_create(time_duration_scr);
	lv_label_set_text(title, "Select Time Duration");
	lv_obj_set_style_text_color(title, lv_color_white(), 0);
	lv_obj_set_style_text_font(title, &lv_font_montserrat_22, 0);
	lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

	/* Container for the three buttons, centered horizontally */
	lv_obj_t *btn_row = lv_obj_create(time_duration_scr);
	lv_obj_set_size(btn_row, LV_PCT(100), LV_SIZE_CONTENT);
	lv_obj_align(btn_row, LV_ALIGN_CENTER, 0, 0);
	lv_obj_clear_flag(btn_row, LV_OBJ_FLAG_SCROLLABLE);

	/* Flex layout: row with evenly spaced children */
	lv_obj_set_flex_flow(btn_row, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(btn_row,
						  LV_FLEX_ALIGN_SPACE_EVENLY,  /* main axis: space them out */
						  LV_FLEX_ALIGN_CENTER,        /* cross axis: center */
						  LV_FLEX_ALIGN_CENTER);       /* track cross axis: center */

	time_btn_s = create_time_button(btn_row, "01:00");
	time_btn_m = create_time_button(btn_row, "02:30");
	time_btn_l = create_time_button(btn_row, "05:00");
}

static void update_instructor_img(char f_i, const char* l_n)
{
	if (!occ_face_img) {
		return;
	}

	const lv_image_dsc_t *src = NULL;

	if (f_i == 'J' && strcmp(l_n, "Carl") == 0) {
		src = &james_img;
	} else if (f_i == 'A' && strcmp(l_n, "Sample") == 0) {
		src = &samples_headshot;
	} else if (f_i == 'M' && strcmp(l_n, "Smith") == 0) {
		src = &matt_img;
	}

	 if (src) {
	        lv_image_set_src(occ_face_img, src);
	        lv_obj_clear_flag(occ_face_img, LV_OBJ_FLAG_HIDDEN);
	} else {
	        // Not one of the special three – hide the image
	        lv_obj_add_flag(occ_face_img, LV_OBJ_FLAG_HIDDEN);
	}
}

/*
 * Helper function for creating the back button for different screens.
 * Reduces code duplication
 */
static lv_obj_t* create_back_button(lv_obj_t *parent, lv_event_cb_t cb)
{
    /* Create button */
    lv_obj_t *btn = lv_button_create(parent);

    /* Size */
    lv_obj_set_size(btn, 60, 30);

    /* Maize background */
    lv_obj_set_style_bg_color(btn, lv_color_hex(MICHIGAN_MAIZE), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, LV_PART_MAIN);

    /* Border for consistency (TODO:optional) */
    lv_obj_set_style_border_width(btn, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(btn, lv_color_hex(MICHIGAN_BLUE), LV_PART_MAIN);

    /* Position (default top-left, can be re-aligned after creation) */
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 10, 15);

    /* Add arrow label */
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_obj_center(label);

    /* Add callback */
    if (cb)
        lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);

    return btn;
}

static void return_to_locker_screen(void)
{
	in_admin_card_menu = false;
	in_admin_pin_menu  = false;
	in_admin_main_menu = false;

	switch (current_display_state) {
	case LOCKER_STATE_VACANT:
		display_vacant();
		break;
	case LOCKER_STATE_OCCUPIED:
		// if display_occupied() resets countdown and you don’t want that,
		// you can make a display_occupied_return() that just reloads the screen.
		lv_screen_load(occupied_scr);
		lv_obj_set_parent(admin_btn, occupied_scr);
		lv_obj_align(admin_btn, LV_ALIGN_TOP_LEFT, 10, 10);
		break;

	case LOCKER_STATE_ABANDONED:
		display_abandoned(current_occupant.firstInitial, current_occupant.lastName);
		break;

	default:
		display_vacant();
		break;
	}
}

static void admin_pin_clear_all(void)
{
	admin_pin_len = 0;
	admin_pin_update_dots();
}

static void admin_pin_update_dots(void)
{
    for (int i = 0; i < 4; i++) {
        if (i < admin_pin_len) {
            /* Filled dot: black center */
            lv_obj_set_style_bg_opa(admin_pin_dots[i], LV_OPA_COVER, 0);
            lv_obj_set_style_bg_color(admin_pin_dots[i], lv_color_white(), 0);
        } else {
            /* Hollow dot */
            lv_obj_set_style_bg_opa(admin_pin_dots[i], LV_OPA_TRANSP, 0);
        }
    }
}

static void admin_pin_append_digit(uint8_t d)
{
    if (admin_pin_len >= 4) {
        return;  // ignore extra digits
    }

    admin_pin_digits[admin_pin_len] = d;
    admin_pin_len++;
    admin_pin_update_dots();
}


static void admin_pin_enter(void)
{
    if (admin_pin_len < 4) {
        /* Incomplete PIN: do nothing for now. */
        return;
    }

//    /* Save PIN as integer 0000..9999 */
//    admin_pin_code =
    uint16_t entered =
        admin_pin_digits[0] * 1000 +
        admin_pin_digits[1] * 100 +
        admin_pin_digits[2] * 10 +
        admin_pin_digits[3];

    // We're setting or changing the pin, not trying to enter the panel
    if (admin_pin_mode == ADMIN_PIN_MODE_SET_CHANGE) {
    	admin_pin_code = entered;
		admin_pin_set  = true;

		/* Inside admin panel and authenticated */
		admin_state = ADMIN_UNLOCKED_CONFIGURED;

		/* Go back to the admin control panel screen. */
		display_admin_menu();
    } else {
    	// Correct pin was entered, allow admin into the admin menu
    	if (entered == admin_pin_code) {
    		admin_state = ADMIN_UNLOCKED_CONFIGURED;
    		display_admin_menu();
    	} else {
    		// wrong pin entered, kick them out
    		admin_pin_clear_all();
    		return_to_locker_screen();
    	}
    }
}


/* Refill the table contents from the master_cards array */
static void master_cards_table_refresh(void)
{
	if (!admin_card_table) return; // safe exit, shouldn't happen

	/* Clear all rows first */
	uint16_t total_rows = MAX_MASTER_CARDS + 1; /* header + max keys */
	for (uint16_t r = 0; r < total_rows; ++r) {
		lv_table_set_cell_value(admin_card_table, r, 0, "");
		lv_table_set_cell_value(admin_card_table, r, 1, "");
	}

	/* Header */
	lv_table_set_cell_value(admin_card_table, 0, 0, "Manager MCard");
	lv_table_set_cell_value(admin_card_table, 0, 1, "Del");

	/* Fill rows with current master keys */
	for (uint16_t i = 0; i < master_card_sz; ++i) {
		char name_buf[32];
		lv_snprintf(name_buf, sizeof(name_buf), "%c. %s",
					master_cards[i].firstInitial,
					master_cards[i].lastName);

		uint16_t row = (uint16_t)(i + 1);
		lv_table_set_cell_value(admin_card_table, row, 0, name_buf);
		lv_table_set_cell_value(admin_card_table, row, 1, LV_SYMBOL_MINUS);
	}
}

/*
 * Initialize the admin cards display
 */
static void init_display_admin_cards(void)
{
	if (admin_card_scr != NULL)
		return;

	admin_card_scr = lv_obj_create(NULL);
	lv_obj_set_style_bg_color(admin_card_scr, lv_color_hex(MICHIGAN_BLUE), 0);
	lv_obj_set_style_bg_opa(admin_card_scr, LV_OPA_COVER, 0);

	/* Title */
	lv_obj_t * title = lv_label_create(admin_card_scr);
	lv_label_set_text(title, "Swipe To Add");
	lv_obj_set_style_text_color(title, lv_color_white(), 0);
	lv_obj_set_style_text_font(title, &lv_font_montserrat_22, 0);
	lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

	/* Back button */
	admin_card_back_btn = create_back_button(admin_card_scr, admin_card_back_btn_event_cb);

	/* Table */
	admin_card_table = lv_table_create(admin_card_scr);

	/* 2 columns: Name + delete */
	lv_table_set_col_cnt(admin_card_table, 2);
	lv_table_set_row_cnt(admin_card_table, MAX_MASTER_CARDS + 1); /* header + rows */

	/* Custom column widths */
	lv_table_set_col_width(admin_card_table, 0, 230);   // main label
	lv_table_set_col_width(admin_card_table, 1, 50);    // delete symbol

	/* Size/position */
	lv_obj_set_size(admin_card_table, 320, 240);
	lv_obj_align(admin_card_table, LV_ALIGN_BOTTOM_MID, 20, -10);

	/* Main background */
	lv_obj_set_style_bg_color(admin_card_table, lv_color_hex(MICHIGAN_BLUE), LV_PART_MAIN);
	lv_obj_set_style_bg_opa(admin_card_table, LV_OPA_COVER, LV_PART_MAIN);

	/* Per-cell base style: we’ll override colors per-row in draw callback */
	lv_obj_set_style_bg_opa(admin_card_table, LV_OPA_COVER, LV_PART_ITEMS);

	/* Text color in cells: Michigan blue by default */
	lv_obj_set_style_text_color(admin_card_table,
	                            lv_color_hex(MICHIGAN_BLUE),
	                            LV_PART_ITEMS);

	/* Taller rows via vertical padding (about 1.5x vs before) */
	lv_obj_set_style_pad_top(admin_card_table,    8, LV_PART_ITEMS);
	lv_obj_set_style_pad_bottom(admin_card_table, 8, LV_PART_ITEMS);
	lv_obj_set_style_pad_left(admin_card_table,   6, LV_PART_ITEMS);
	lv_obj_set_style_pad_right(admin_card_table,  4, LV_PART_ITEMS);

	/* Border around the whole table */
	lv_obj_set_style_border_width(admin_card_table, 2, LV_PART_MAIN);
	lv_obj_set_style_border_color(admin_card_table,
	                              lv_color_hex(MICHIGAN_BLUE),
	                              LV_PART_MAIN);

	/* Allow clicking rows (we’ll logically ignore clicks on the name column) */
	lv_obj_add_flag(admin_card_table, LV_OBJ_FLAG_CLICKABLE);

	/* Draw callback for header + striped rows + red delete symbols */
	lv_obj_add_event_cb(admin_card_table,
	                    master_cards_table_draw_event_cb,
	                    LV_EVENT_DRAW_TASK_ADDED,
	                    NULL);
	lv_obj_add_flag(admin_card_table, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);

	/* Click callback to handle deletes */
	lv_obj_add_event_cb(admin_card_table,
	                    master_cards_table_event_cb,
	                    LV_EVENT_VALUE_CHANGED,
	                    NULL);

	/* Populate initial contents (likely empty) */
	master_cards_table_refresh();
}

static void display_admin_cards(void)
{
	if (admin_card_scr == NULL)
		init_display_admin_cards();

	    master_cards_table_refresh();
	    lv_screen_load(admin_card_scr);
}

static void occupied_update_countdown_label(void)
{
    if (!occ_countdown_label) return;

    int m = occ_seconds_left / 60;
    int s = occ_seconds_left % 60;

    char buf[6];  // "MM:SS" + '\0'
    lv_snprintf(buf, sizeof(buf), "%02d:%02d", m, s);

    lv_label_set_text(occ_countdown_label, buf);
}

/***********************
 * Display functions
 ***********************/

/*
 * Public function to show the time duration option window when a user occupies a locker
 */
void display_time_duration_screen(void)
{
	if (time_duration_scr == NULL) {
		init_time_duration_screen();
	}

	lv_screen_load(time_duration_scr);
}


/*
 * Public function to update the UI of the current screen
 */
void ui_show_status_msg(const char* msg, uint32_t duration_ms)
{
	lv_obj_t *label = NULL;

	switch (current_display_state) {
	case LOCKER_STATE_VACANT:
		label = vacant_msg_label;
		break;
	case LOCKER_STATE_OCCUPIED:
		label = occupied_msg_label;
		break;
	case LOCKER_STATE_ABANDONED:
		label = abandoned_msg_label;
		break;
	default:
		return; /* startup / null etc: nothing to show */
	}

	if (!label) return;

	/* Set the message text */
	lv_label_set_text(label, msg);

	/* Kill any previous timer so messages do not fight each other */
	if (status_msg_timer) {
		lv_timer_del(status_msg_timer);
		status_msg_timer = NULL;
	}

	/* Remember which screen this message is for */
	status_msg_state = current_display_state;

	/* Start a one-shot timer to restore the default text */
	status_msg_timer = lv_timer_create(status_msg_timer_cb, duration_ms, NULL);
}

/*
 * public API: Return true if the card is admin
 */
bool is_admin(const MCard* card)
{
	/* Check if card is already present */
	for (uint8_t i = 0; i < master_card_sz; ++i) {
		if (master_cards[i].umId == card->umId) {
			return true;
		}
	}

	return false;
}

/*
 * public API: Add a master card entry from an MCard scan
 */
void admin_add_master_card(const MCard* card)
{
	if (!card)
		return;

	// If the user is already admin
	if (is_admin(card)) {
		return;
	}

	// TODO: Check if it's larger than max
	if (master_card_sz >= MAX_MASTER_CARDS) {
		return;
	}

	master_cards[master_card_sz++] = *card;

	/* If screen is active, update instantly */
	if (lv_scr_act() == admin_card_scr) {
		master_cards_table_refresh();
	}
}

static void init_display_admin_pin(void)
{
	if (admin_pin_scr != NULL) return;

	admin_pin_scr = lv_obj_create(NULL);
	lv_obj_set_style_bg_color(admin_pin_scr, lv_color_hex(MICHIGAN_BLUE), 0);
	lv_obj_set_style_bg_opa(admin_pin_scr, LV_OPA_COVER, 0);

	/* Title */
	admin_pin_title = lv_label_create(admin_pin_scr);
	lv_label_set_text(admin_pin_title, "Set admin PIN");
	lv_obj_set_style_text_color(admin_pin_title, lv_color_white(), 0);
	lv_obj_set_style_text_font(admin_pin_title, &lv_font_montserrat_22, 0);
	lv_obj_align(admin_pin_title, LV_ALIGN_TOP_MID, 0, 20);

	/* Back button */
	admin_pin_back_btn = create_back_button(admin_pin_scr, admin_pin_back_btn_event_cb);

	/* 4 hollow dots for PIN */
	int dot_spacing = 26;
	int dot_size    = 16;

	for (int i = 0; i < 4; i++) {
		admin_pin_dots[i] = lv_obj_create(admin_pin_scr);
		lv_obj_set_size(admin_pin_dots[i], dot_size, dot_size);

		lv_obj_set_style_radius(admin_pin_dots[i], dot_size / 2, 0);
		lv_obj_set_style_bg_opa(admin_pin_dots[i], LV_OPA_TRANSP, 0);           // hollow
		lv_obj_set_style_border_width(admin_pin_dots[i], 3, 0);
		lv_obj_set_style_border_color(admin_pin_dots[i], lv_color_white(), 0);

		lv_obj_align(admin_pin_dots[i], LV_ALIGN_TOP_MID,
					 (i - 1.5f) * dot_spacing, 65);
	}

	/* Button matrix for keypad */
	static const char * pin_btnm_map[] = {
		"1", "2", "3", "\n",
		"4", "5", "6", "\n",
		"7", "8", "9", "\n",
		"-", "0", LV_SYMBOL_RIGHT, ""
	};

	admin_pin_btnm = lv_buttonmatrix_create(admin_pin_scr);
	lv_buttonmatrix_set_map(admin_pin_btnm, pin_btnm_map);

	/* Size + position (adjust if needed for your screen) */
	lv_obj_set_size(admin_pin_btnm, 275, 215);
	lv_obj_align(admin_pin_btnm, LV_ALIGN_BOTTOM_MID, 0, -10);

	/* Make the matrix background transparent (remove white card) */
	lv_obj_set_style_bg_opa(admin_pin_btnm,
							LV_OPA_TRANSP,
							LV_PART_MAIN);

	/* Maize keys, blue border */
	lv_obj_set_style_bg_color(admin_pin_btnm,
							  lv_color_hex(MICHIGAN_MAIZE),
							  LV_PART_ITEMS);

	lv_obj_set_style_bg_opa(admin_pin_btnm,
							LV_OPA_COVER,
							LV_PART_ITEMS);

	lv_obj_set_style_border_width(admin_pin_btnm,
								  2,
								  LV_PART_ITEMS);

	lv_obj_set_style_border_color(admin_pin_btnm,
								  lv_color_hex(MICHIGAN_BLUE),
								  LV_PART_ITEMS);

	/* All keypad text (digits and symbols) white */
	lv_obj_set_style_text_color(admin_pin_btnm,
								lv_color_white(),
								LV_PART_ITEMS);

	/* Treat "-" as clear-all, and the right-arrow as Enter */
	lv_obj_add_event_cb(admin_pin_btnm,
						admin_pin_btnm_event_cb,
						LV_EVENT_VALUE_CHANGED,
						NULL);

	/* Keep focus off the matrix; we’re not using a textarea */
	lv_obj_remove_flag(admin_pin_btnm, LV_OBJ_FLAG_CLICK_FOCUSABLE);
}

static void display_admin_pin(void)
{
	if (admin_pin_scr == NULL) {
		init_display_admin_pin();
	}

	 // Update title whether you're setting or changing pin
	if (admin_pin_mode == ADMIN_PIN_MODE_UNLOCK) {
		lv_label_set_text(admin_pin_title, "Enter admin PIN");
	} else {
		 if (admin_state == ADMIN_UNCONFIGURED) {
			lv_label_set_text(admin_pin_title, "Set admin PIN");
		} else {
			lv_label_set_text(admin_pin_title, "Change admin PIN");
		}
	}

	// Reset input when screen is opened
	admin_pin_clear_all();


	// december 8 2025
	in_admin_pin_menu  = true;
	in_admin_main_menu = false;
	in_admin_card_menu = false;


	lv_screen_load(admin_pin_scr);
}

/*
 * function that initializes the admin button
 *
 * The admin button is located on the top left corner of
 * the screen for vacant, occupied, and abandoned
 *
 * When pressed, it opens up the admin menu
 * In the admin menu there will be two options,
 * which will be described in their respective menu
 *
 * IMPORTANT: The 3 screens that use this function must
 * init the admin btn before using.
 */
static void init_admin_btn(void)
{
	if (admin_btn != NULL)
	        return;

	    /* Default style */
	    static lv_style_t style;
	    static lv_style_t style_pr;
	    static bool styles_inited = false;

	    if (!styles_inited) {
	        styles_inited = true;

	        lv_style_init(&style);
	        lv_style_init(&style_pr);

	        /* Base button look */
	        lv_style_set_radius(&style, 8);
	        lv_style_set_bg_opa(&style, LV_OPA_100);
	        lv_style_set_bg_color(&style, lv_color_hex(MICHIGAN_MAIZE));
	        lv_style_set_bg_grad_color(&style, lv_color_hex(MICHIGAN_MAIZE));
	        lv_style_set_bg_grad_dir(&style, LV_GRAD_DIR_VER);

	        /* Border (thin dark blue/black) */
	        lv_style_set_border_opa(&style, LV_OPA_80);
	        lv_style_set_border_width(&style, 3);
	        lv_style_set_border_color(&style, lv_color_hex(0x00274C));  // Michigan blue

	        /* Outline for a subtle halo */
	        lv_style_set_outline_opa(&style, LV_OPA_80);
	        lv_style_set_outline_width(&style, 2);
	        lv_style_set_outline_color(&style, lv_color_hex(0x00274C));

	        /* Shadow */
	        lv_style_set_shadow_width(&style, 10);
	        lv_style_set_shadow_color(&style, lv_color_hex(0x000000));
	        lv_style_set_shadow_offset_x(&style, 3);
	        lv_style_set_shadow_offset_y(&style, 4);

	        /* Slightly smaller padding if you want it even tighter */
	        lv_style_set_pad_all(&style, 6);   // was 8

	        /* Pressed style: slight push + darker maize + expanding outline */
	        lv_style_set_translate_y(&style_pr, 2);
	        lv_style_set_shadow_offset_y(&style_pr, 2);
	        lv_style_set_bg_color(&style_pr, lv_color_hex(0xD6A404));
	        lv_style_set_bg_grad_color(&style_pr, lv_color_hex(0xB98C03));

	        lv_style_set_outline_width(&style_pr, 24);
	        lv_style_set_outline_opa(&style_pr, LV_OPA_0);

	        static lv_style_transition_dsc_t trans;
	        static lv_style_prop_t props[] = {
	            LV_STYLE_OUTLINE_WIDTH,
	            LV_STYLE_OUTLINE_OPA,
	            0
	        };

	        lv_style_transition_dsc_init(&trans, props, lv_anim_path_linear, 300, 0, NULL);
	        lv_style_set_transition(&style_pr, &trans);
	    }

	    admin_btn = lv_button_create(lv_screen_active());
	    lv_obj_remove_style_all(admin_btn);
	    lv_obj_add_style(admin_btn, &style, 0);
	    lv_obj_add_style(admin_btn, &style_pr, LV_STATE_PRESSED);

	    /* Let LVGL size the button to its content (roughly ~half 100x70) */
	    lv_obj_set_size(admin_btn, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

	    /* Add symbol label */
	    lv_obj_t * label = lv_label_create(admin_btn);
	    lv_label_set_text(label, LV_SYMBOL_SETTINGS);
	    lv_obj_center(label);

	    /* Symbol will be white */
	    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
	    lv_obj_set_style_text_font(label, &lv_font_montserrat_34, 0); // default font supports symbols

	    /* event callback for when admin button is pressed */
	    // TODO: event clicked or event pressed?
	    lv_obj_add_event_cb(admin_btn, admin_btn_event_cb, LV_EVENT_CLICKED, NULL);
}

/*
 * Display admin menu:
 *
 * Admin menu where there are two buttons in the center.
 *
 * The button on the left is to configure an admin pin, when the button
 * is pressed, it takes the user to a keypad to insert a 4 digit pin
 * This 4 digit pin can be used to open lockers through admin override
 *
 *
 * 	An admin card is a master key, you can unlock a locker without
 * 	needing to put in a pin
 *
 * 	On startup, there is no admin so the state of the locker is
 * 	ADMIN_UNCONFIGURED, anyone can enter the admin panel
 *
 * 	After the pin is set, then the state becomes ADMIN_UNLOCKED_CONFIGURED
 *
 * 	when the user leaves the admin menu, it becomes ADMIN_LOCKED_CONFIGURED
 *
 * 	when the user presses the admin button and the admin state is ADMIN_LOCKED_CONFIGURED
 * 	then the button first sends the user to a 4 digit code they need to insert
 * 	there will be a back button as well
 *
 * 	if the code they put is wrong, then user gets kicked out of window and goes back to
 * 	last screen
 */
static void init_display_admin_menu(void)
{
	if (admin_scr != NULL)
		return;

	admin_scr = lv_obj_create(NULL);

	lv_obj_set_style_bg_color(admin_scr, lv_color_hex(MICHIGAN_BLUE), 0);
	lv_obj_set_style_bg_opa(admin_scr, LV_OPA_COVER, 0);

	/* Title: admin control panel  */
	lv_obj_t * title = lv_label_create(admin_scr);
	lv_label_set_text(title, "Admin Control Panel");
	lv_obj_set_style_text_color(title, lv_color_white(), 0);
	lv_obj_set_style_text_font(title, &lv_font_montserrat_22, 0);
	lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

	/* Simple style for K / P buttons */
	static lv_style_t btn_style;
	static bool btn_style_inited = false;
	if (!btn_style_inited) {
		btn_style_inited = true;
		lv_style_init(&btn_style);
		lv_style_set_radius(&btn_style, 6);
		lv_style_set_bg_opa(&btn_style, LV_OPA_100);
		lv_style_set_bg_color(&btn_style, lv_color_hex(MICHIGAN_MAIZE));
		lv_style_set_border_width(&btn_style, 3);
		lv_style_set_border_color(&btn_style, lv_color_hex(0x00274C));
		lv_style_set_pad_all(&btn_style, 24);
	}

	/* K button (center-left) */
	admin_pin_btn = lv_button_create(admin_scr);
	lv_obj_add_style(admin_pin_btn, &btn_style, 0);

	/* Make rectangular: wider, less tall */
	lv_obj_set_size(admin_pin_btn, 128, 64);     // Wider + shorter
	lv_obj_align(admin_pin_btn, LV_ALIGN_CENTER, -80, 0);
	lv_obj_t * label_k = lv_label_create(admin_pin_btn);
	lv_label_set_text(label_k, LV_SYMBOL_KEYBOARD);
	lv_obj_set_style_text_font(label_k, &lv_font_montserrat_40, 0);
	lv_obj_center(label_k);

	// call back for pin button
	lv_obj_add_event_cb(admin_pin_btn, admin_pin_btn_event_cb, LV_EVENT_CLICKED, NULL);

	/* P button (center-right) */
	admin_card_btn = lv_button_create(admin_scr);
	lv_obj_add_style(admin_card_btn, &btn_style, 0);
	lv_obj_set_size(admin_card_btn, 128, 64); /* Same rectangular shape */
	lv_obj_align(admin_card_btn, LV_ALIGN_CENTER, 80, 0);
	lv_obj_t * label_p = lv_label_create(admin_card_btn);
	lv_label_set_text(label_p, LV_SYMBOL_DRIVE);
	lv_obj_set_style_text_font(label_p, &lv_font_montserrat_40, 0);
	lv_obj_center(label_p);

	// call back for the card button
	lv_obj_add_event_cb(admin_card_btn, admin_card_btn_event_cb, LV_EVENT_CLICKED, NULL);

	/* Back button in top-left */
	admin_back_btn = create_back_button(admin_scr, admin_back_btn_event_cb);

	/* Text that appears when the admin panel is unconfigured */
	/* Status text under the buttons */
	if (admin_status_label == NULL)
	{
		admin_status_label = lv_label_create(admin_scr);
		lv_label_set_text(admin_status_label, "Admin unconfigured");
		lv_obj_set_style_text_color(admin_status_label, lv_color_white(), 0);
		lv_obj_set_style_text_font(admin_status_label, &lv_font_montserrat_18, 0);
		/* Position: underneath the two buttons */
		lv_obj_align(admin_status_label, LV_ALIGN_CENTER, 0, 55);
	}

}

static void display_admin_menu(void)
{
	if (admin_scr == NULL)
		init_display_admin_menu();

	lv_screen_load(admin_scr);

	// December 8 2025
	in_admin_main_menu = true;
	in_admin_pin_menu  = false;
	in_admin_card_menu = false;

	/* Show or hide the status text depending on admin_state */
	if (admin_state == ADMIN_UNCONFIGURED) {
		lv_obj_clear_flag(admin_status_label, LV_OBJ_FLAG_HIDDEN);
	} else {
		lv_obj_add_flag(admin_status_label, LV_OBJ_FLAG_HIDDEN);
	}
}

// Initializes the home page of SwipeLock
void display_startup(void)
{
	const char* name = "SwipeLock";
	const char* info = "Tap anywhere to enter";

	lv_obj_t *scr = lv_screen_active();

    lv_obj_set_style_bg_color(scr,
                              lv_color_hex(MICHIGAN_BLUE),
                              LV_PART_MAIN);

    // make the whole screen clickable so "tap anywhere" works
	lv_obj_add_flag(scr, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_event_cb(scr, startup_screen_event_cb, LV_EVENT_PRESSED, NULL);


	// Create the header of the startup
    heading = lv_label_create(scr);
    lv_label_set_text(heading, name);
    lv_obj_set_style_text_color(heading, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(heading, &lv_font_montserrat_48, LV_PART_MAIN);
    lv_obj_align(heading, LV_ALIGN_CENTER, 0, -40);
    lv_obj_set_style_opa(heading, LV_OPA_TRANSP, LV_PART_MAIN);

    // Create the subheader of the startup
    subheading = lv_label_create(scr);
    lv_label_set_text(subheading, info);
    lv_obj_set_style_text_color(subheading, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(subheading, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_align(subheading, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_opa(subheading, LV_OPA_TRANSP, LV_PART_MAIN);

    intro_state = 0;
    intro_ts = lv_tick_get();
    intro_active = true;
}

void update_startup(void)
{
	 if (!intro_active) {
	        return;  // we already switched screens; do nothing
	}

    uint32_t now = lv_tick_get();

    switch (intro_state) {
    case 0:
        lv_obj_fade_in(heading, 500, 0);
        intro_ts = now;
        intro_state = 1;
        break;

    case 1:
        if (lv_tick_elaps(intro_ts) >= 500) {
            lv_obj_fade_in(subheading, 500, 0);
            intro_ts = now;
            intro_state = 2;
        }
        break;

    case 2:
        if (lv_tick_elaps(intro_ts) >= 2500) { // 500 in + 2000 hold
            lv_obj_fade_out(heading, 500, 0);
            lv_obj_fade_out(subheading, 500, 0);
            intro_ts = now;
            intro_state = 3;
        }
        break;

    case 3:
        if (lv_tick_elaps(intro_ts) >= 500) {
            // loop back, or transition to your next screen
            lv_obj_set_style_opa(heading, LV_OPA_TRANSP, LV_PART_MAIN);
            lv_obj_set_style_opa(subheading, LV_OPA_TRANSP, LV_PART_MAIN);
            intro_state = 0;
        }
        break;
    }
}


void gui_wait_ms(uint32_t ms)
{
    uint32_t start = HAL_GetTick();
    while (HAL_GetTick() - start < ms) {
        lv_timer_handler();     // let LVGL process animations, redraw, etc.
        HAL_Delay(2);           // small sleep to avoid busy-looping
    }
}

/*
 * private helper function for display_vacant
 * Creates the vacant display only once
 * Allows for
 */
static void init_display_vacant(void)
{
	if (vacant_scr != NULL)
		return;

	vacant_scr = lv_obj_create(NULL);
//	lv_obj_clean(scr);   // clear previous screen

	// UMich blue background
	lv_obj_set_style_bg_color(vacant_scr, lv_color_hex(MICHIGAN_BLUE), 0);
	lv_obj_set_style_bg_opa(vacant_scr, LV_OPA_COVER, 0);

	/* Main title: "Vacant" */
	lv_obj_t * label_main = lv_label_create(vacant_scr);
	lv_label_set_text(label_main, "Vacant");
	lv_obj_set_style_text_color(label_main, lv_color_white(), 0);
	lv_obj_set_style_text_font(label_main, &lv_font_montserrat_48, 0);
	lv_obj_align(label_main, LV_ALIGN_CENTER, 0, -6);

	/* Subtitle: "Swipe to occupy" */
	vacant_msg_label = lv_label_create(vacant_scr);
	lv_label_set_text(vacant_msg_label, "(Swipe MCard to occupy)");
	lv_obj_set_style_text_color(vacant_msg_label, lv_color_white(), 0);
	lv_obj_set_style_text_font(vacant_msg_label, &lv_font_montserrat_18, 0);
	lv_obj_align(vacant_msg_label, LV_ALIGN_CENTER, 0, 40);
}

void display_vacant(void)
{
	if (vacant_scr == NULL)
		init_display_vacant();

	// first initialize admin button here
	if (admin_btn == NULL) {
		init_admin_btn();
	}

	lv_obj_set_parent(admin_btn, vacant_scr);
	lv_obj_align(admin_btn, LV_ALIGN_TOP_LEFT, 10, 10);

	/* Unlock the door */
//	unlatch_door();
//	trigger_timed_unlock();

	/* Disable the countdown timer if it is still on */
	if (occ_countdown_active == true) {
		occ_countdown_active = false;
		lv_timer_pause(occ_countdown_timer); // pause the timer
	}

	current_display_state = LOCKER_STATE_VACANT;
	lv_screen_load(vacant_scr);
}

static void init_display_occupied(void)
{
	if (occupied_scr != NULL)
		return;


	// initialize the screen
	occupied_scr = lv_obj_create(NULL);
	lv_obj_set_style_bg_color(occupied_scr, lv_color_hex(MICHIGAN_BLUE), 0);
	lv_obj_set_style_bg_opa(occupied_scr, LV_OPA_COVER, 0);

	/* Line 1: "Occupied by" */
	lv_obj_t* label1 = lv_label_create(occupied_scr);
	lv_label_set_text(label1, "Occupied by");
	lv_obj_set_style_text_color(label1, lv_color_white(), 0);
	lv_obj_set_style_text_font(label1, &lv_font_montserrat_28, 0);
	lv_obj_align(label1, LV_ALIGN_CENTER, 0, -50);

	/* Line 2: "<F> <LastName>" – text will be set later */
	occ_name = lv_label_create(occupied_scr);
	lv_label_set_text(occ_name, "");  // placeholder
	lv_obj_set_style_text_color(occ_name, lv_color_white(), LV_PART_MAIN);
	lv_obj_set_style_text_font(occ_name, &lv_font_montserrat_40, LV_PART_MAIN);
	lv_obj_align(occ_name, LV_ALIGN_CENTER, 0, 0);

	/* Line 3: "(Swipe M-Card to vacate locker)" */
	occupied_msg_label = lv_label_create(occupied_scr);
	lv_label_set_text(occupied_msg_label, "(Swipe MCard to vacate)");
	lv_obj_set_style_text_color(occupied_msg_label, lv_color_white(), 0);
	lv_obj_set_style_text_font(occupied_msg_label, &lv_font_montserrat_18, 0);
	lv_obj_align(occupied_msg_label, LV_ALIGN_CENTER, 0, 40);


	/* Countdown label in top-right: "01:00" initially */
	occ_countdown_label = lv_label_create(occupied_scr);
	lv_obj_set_style_text_color(occ_countdown_label, lv_color_white(), 0);
	lv_obj_set_style_text_font(occ_countdown_label, &lv_font_montserrat_34, 0);
	lv_obj_align(occ_countdown_label, LV_ALIGN_TOP_RIGHT, -10, 20);
	lv_label_set_text(occ_countdown_label, "01:00");  // default appearance


	/* 12/5/2025:
	 * 	Create the image object
	 */
	occ_face_img = lv_image_create(occupied_scr);
    lv_obj_align(occ_face_img, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_add_flag(occ_face_img, LV_OBJ_FLAG_HIDDEN);  // hidden until needed

	/* Create the LVGL timer once, but keep it paused until we need it */
	if (occ_countdown_timer == NULL) {
		occ_countdown_timer = lv_timer_create(occupied_countdown_cb, 1000, NULL);
		lv_timer_pause(occ_countdown_timer);
	}
	/* Create the vacate button */
//	init_vacate_button();

}

void display_occupied(char f_i, const char* l_n)
{
	 if (occupied_scr == NULL)
		init_display_occupied();

	 /* Format "<first initial> <last name>" */
	 char buf[64];
	 snprintf(buf, sizeof(buf), "%c. %s", f_i, l_n);

	 /* Update the name label */
	 lv_label_set_text(occ_name, buf);

	 // admin button
	 if (admin_btn == NULL)
		 init_admin_btn();

	 lv_obj_set_parent(admin_btn, occupied_scr);
	 lv_obj_align(admin_btn, LV_ALIGN_TOP_LEFT, 10, 10);

	/* Reset and start countdown: 1 minute demo */
	 if (occ_seconds_left <= 0) {
		 occ_seconds_left     = 150;     // 01:00 ; Gjonpjer (12/7/2025) -> Changed to 2:30
	 }
	occ_countdown_active = true;
	occupied_update_countdown_label();  // show 01:00

	if (occ_countdown_timer) {
		lv_timer_resume(occ_countdown_timer);
	}

	/* If the user is the instructor, show the instructor's face */
	 update_instructor_img(f_i, l_n);

	 /* Lock the door */
//	 latch_door();
//	 trigger_timed_unlock();

	 /* Update the state and load the screen*/
	 current_display_state = LOCKER_STATE_OCCUPIED;
	 lv_screen_load(occupied_scr);
}

/*
 * Similar to occupy, if not the same in display. There's just no clock on the top right
 * nor keypad button on the bottom left
 */
static void init_display_abandoned(void)
{
	if (abandoned_scr != NULL)
		return;

	abandoned_scr = lv_obj_create(NULL);
	lv_obj_set_style_bg_color(abandoned_scr, lv_color_hex(MICHIGAN_BLUE), 0);
	lv_obj_set_style_bg_opa(abandoned_scr, LV_OPA_COVER, 0);

	lv_obj_t* label = lv_label_create(abandoned_scr);
	lv_label_set_text(label, "Abandoned by");
	lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
	lv_obj_set_style_text_font(label, &lv_font_montserrat_28, LV_PART_MAIN);
	lv_obj_align(label, LV_ALIGN_CENTER, 0, -50);

	abandoned_header = lv_label_create(abandoned_scr);
	lv_label_set_text(abandoned_header, "");
	lv_obj_set_style_text_color(abandoned_header, lv_color_white(), LV_PART_MAIN);
	lv_obj_set_style_text_font(abandoned_header, &lv_font_montserrat_40, LV_PART_MAIN);
	lv_obj_align(abandoned_header, LV_ALIGN_CENTER, 0, 0);

	abandoned_msg_label = lv_label_create(abandoned_scr);
	lv_label_set_text(abandoned_msg_label, "(Swipe MCard to occupy)");
	lv_obj_set_style_text_color(abandoned_msg_label, lv_color_white(), LV_PART_MAIN);
	lv_obj_set_style_text_font(abandoned_msg_label, &lv_font_montserrat_18, LV_PART_MAIN);
	lv_obj_align(abandoned_msg_label, LV_ALIGN_CENTER, 0, 40);
}

void display_abandoned(char f_i, const char* l_n)
{
	if (abandoned_scr == NULL)
		init_display_abandoned();

	/* Format "<first initial>. <last name>" */
	char buf[64];
	snprintf(buf, sizeof(buf), "%c. %s", f_i, l_n);

	// Update the abandoned_header
	lv_label_set_text(abandoned_header, buf);

	// admin btn
	if (admin_btn == NULL) {
		init_admin_btn();
	}

	lv_obj_set_parent(admin_btn, abandoned_scr);
	lv_obj_align(admin_btn, LV_ALIGN_TOP_LEFT, 10, 10);

	/* Unlock the door*/
//	unlatch_door();
//	trigger_timed_unlock();

	current_display_state = LOCKER_STATE_ABANDONED;
	lv_screen_load(abandoned_scr);
}

/***************************
 * Test Example Functions
 ***************************/

/*
 * Create a button with a label and react on click event.
 */
void buttonExample(void)
{
    lv_obj_t * btn = lv_button_create(lv_screen_active());     /*Add a button the current screen*/
    lv_obj_set_pos(btn, 10, 10);                            /*Set its position*/
    lv_obj_set_size(btn, 120, 50);                          /*Set its size*/
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);           /*Assign a callback to the button*/

    lv_obj_t * label = lv_label_create(btn);          /*Add a label to the button*/
    lv_label_set_text(label, "Button");                     /*Set the labels text*/
    lv_obj_center(label);
}


void fade_text(const char *txt, const lv_font_t* font)
{
    /* Bright blue background */
    lv_obj_set_style_bg_color(lv_screen_active(),
                              lv_color_hex(0x4093D6),
                              LV_PART_MAIN);

    /* Create label */
    lv_obj_t * label = lv_label_create(lv_screen_active());

    lv_label_set_text(label, txt);

    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, font, LV_PART_MAIN);

    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    /* Start fully transparent */
    lv_obj_set_style_opa(label, LV_OPA_TRANSP, LV_PART_MAIN);

    /* Fade in */
    lv_obj_fade_in(label, 500, 0);   // 500 ms fade-in
    gui_wait_ms(500);                // wait for fade-in to finish

    /* Stay fully visible for allocated time */
    gui_wait_ms(2000);

    /* Fade out (LVGL will auto-delete the object at the end) */
    lv_obj_fade_out(label, 500, 0);  // 500 ms fade-out
    gui_wait_ms(500);                // wait for fade-out to finish
}


void printHelloWorld(void)
{
    /*Change the background color*/
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(MICHIGAN_BLUE), LV_PART_MAIN);

    /*Create label*/
    lv_obj_t * label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Hello world");
    lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

void printGoodByeWorld(void)
{
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x003a57), LV_PART_MAIN);

    lv_obj_t * label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Goodbye world");
    lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}



static void textarea_event_handler(lv_event_t * e)
{
    lv_obj_t * ta = lv_event_get_target_obj(e);
    LV_UNUSED(ta);
    LV_LOG_USER("Enter was pressed. The current text is: %s", lv_textarea_get_text(ta));
}

static void btnm_event_handler(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target_obj(e);
    lv_obj_t * ta = (lv_obj_t *)lv_event_get_user_data(e);
    const char * txt = lv_buttonmatrix_get_button_text(obj, lv_buttonmatrix_get_selected_button(obj));

    if(lv_strcmp(txt, LV_SYMBOL_BACKSPACE) == 0) lv_textarea_delete_char(ta);
    else if(lv_strcmp(txt, LV_SYMBOL_NEW_LINE) == 0) lv_obj_send_event(ta, LV_EVENT_READY, NULL);
    else lv_textarea_add_text(ta, txt);

}

void textarea(void)
{
    lv_obj_t * ta = lv_textarea_create(lv_screen_active());
    lv_textarea_set_one_line(ta, true);
    lv_obj_align(ta, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_add_event_cb(ta, textarea_event_handler, LV_EVENT_READY, ta);
    lv_obj_add_state(ta, LV_STATE_FOCUSED); /*To be sure the cursor is visible*/

    static const char * btnm_map[] = {"1", "2", "3", "\n",
                                      "4", "5", "6", "\n",
                                      "7", "8", "9", "\n",
                                      LV_SYMBOL_BACKSPACE, "0", LV_SYMBOL_NEW_LINE, ""
                                     };

    lv_obj_t * btnm = lv_buttonmatrix_create(lv_screen_active());
    lv_obj_set_size(btnm, 350, 230);
    lv_obj_align(btnm, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_event_cb(btnm, btnm_event_handler, LV_EVENT_VALUE_CHANGED, ta);
    lv_obj_remove_flag(btnm, LV_OBJ_FLAG_CLICK_FOCUSABLE); /*To keep the text area focused on button clicks*/
    lv_buttonmatrix_set_map(btnm, btnm_map);
}


/*
 * Private helper function for the occupied screen
 *
 * Creates the button that allows the user to begin the process
 * of vacating their locker
 *
 */
//static void init_vacate_button(void)
//{
//    if (vacate_button != NULL)
//        return;
//
//    /*Init the style for the default state*/
//    static lv_style_t style;
//    lv_style_init(&style);
//
//    lv_style_set_radius(&style, 6);
//
//    lv_style_set_bg_opa(&style, LV_OPA_100);
//    lv_style_set_bg_color(&style, lv_color_hex(MICHIGAN_MAIZE));
//    lv_style_set_bg_grad_color(&style, lv_color_hex(MICHIGAN_MAIZE));
//    lv_style_set_bg_grad_dir(&style, LV_GRAD_DIR_VER);
//
//    lv_style_set_border_opa(&style, LV_OPA_40);
//    lv_style_set_border_width(&style, 4);
//    lv_style_set_border_color(&style, lv_color_black());
//
//    lv_style_set_shadow_width(&style, 8);
//    lv_style_set_shadow_color(&style, lv_color_hex(0x000000));
//    lv_style_set_shadow_offset_y(&style, 6);
//
//    lv_style_set_outline_opa(&style, LV_OPA_TRANSP);
//    lv_style_set_text_color(&style, lv_color_hex(0x00274C)); // Michigan blue text
//    lv_style_set_text_font(&style, &lv_font_montserrat_18);     // ⬅ slightly bigger font
//    lv_style_set_pad_all(&style, 20); // Change this value for size
//
//    /*Init pressed style*/
//    static lv_style_t style_pr;
//    lv_style_init(&style_pr);
//
//    /* Big outline that will animate outwards/fade */
//	lv_style_set_outline_width(&style_pr, 30);
//	lv_style_set_outline_opa(&style_pr, LV_OPA_TRANSP);
//
//    lv_style_set_translate_y(&style_pr, 4);
//    lv_style_set_shadow_offset_y(&style_pr, 2);
//
//    // darkened maize (custom tones)
//    lv_style_set_bg_color(&style_pr, lv_color_hex(0xD6A404));
//    lv_style_set_bg_grad_color(&style_pr, lv_color_hex(0xB98C03));
//
//    static lv_style_transition_dsc_t trans;
//    static lv_style_prop_t props[] = {LV_STYLE_OUTLINE_WIDTH, LV_STYLE_OUTLINE_OPA, 0};
//    lv_style_transition_dsc_init(&trans, props, lv_anim_path_linear, 300, 0, NULL);
//    lv_style_set_transition(&style_pr, &trans);
//
//    /* Create button */
//    vacate_button = lv_button_create(occupied_scr);
//    lv_obj_remove_style_all(vacate_button);
//    lv_obj_add_style(vacate_button, &style, 0);
//    lv_obj_add_style(vacate_button, &style_pr, LV_STATE_PRESSED);
//    lv_obj_set_size(vacate_button, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
//    lv_obj_align(vacate_button, LV_ALIGN_BOTTOM_MID, 0, -20);
//
//    /* Add button text */
//    lv_obj_t * label = lv_label_create(vacate_button);
//    lv_label_set_text(label, "Vacate Locker");
//    lv_obj_center(label);
//
//    /* TODO: add event callback that will open a message */
//}



//    /* Button matrix for keypad */
//    static const char * pin_btnm_map[] = {
//        "1", "2", "3", "\n",
//        "4", "5", "6", "\n",
//        "7", "8", "9", "\n",
//        "-", "0", LV_SYMBOL_RIGHT, ""
//    };
//
//    admin_pin_btnm = lv_buttonmatrix_create(admin_pin_scr);
//   lv_buttonmatrix_set_map(admin_pin_btnm, pin_btnm_map);
//
//
//	/* Size + position (adjust if needed for your screen) */
//	lv_obj_set_size(admin_pin_btnm, 275, 215);
//	lv_obj_align(admin_pin_btnm, LV_ALIGN_BOTTOM_MID, 0, -10);
//
//	/* Maize keys, blue border, white text */
//	lv_obj_set_style_bg_color(admin_pin_btnm, lv_color_hex(MICHIGAN_MAIZE), LV_PART_ITEMS);
//	lv_obj_set_style_bg_opa(admin_pin_btnm, LV_OPA_COVER, LV_PART_ITEMS);
//	lv_obj_set_style_border_width(admin_pin_btnm, 2, LV_PART_ITEMS);
//	lv_obj_set_style_border_color(admin_pin_btnm, lv_color_hex(MICHIGAN_BLUE), LV_PART_ITEMS);
//	lv_obj_set_style_text_color(admin_pin_btnm, lv_color_hex(0x00274C), LV_PART_ITEMS);
//
//	/* treat "-" as clear-all, and the right-arrow as Enter */
//	lv_obj_add_event_cb(admin_pin_btnm,
//				   admin_pin_btnm_event_cb,
//				   LV_EVENT_VALUE_CHANGED,
//				   NULL);
//
//	/* Keep focus off the matrix; we’re not using a textarea */
//	lv_obj_remove_flag(admin_pin_btnm, LV_OBJ_FLAG_CLICK_FOCUSABLE);
