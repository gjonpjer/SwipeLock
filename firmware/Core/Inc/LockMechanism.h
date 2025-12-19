/*
 * Lock Mechanism file
 */


#ifndef LOCK_MECHANISM_H
#define LOCK_MECHANISM_H

/*
 * To latch, set the GPIO pin to 0
 */
void latch_door();

/*
 * To unlatch, set the GPIO pin to 1
 */
void unlatch_door();

#endif
