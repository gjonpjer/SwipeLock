/*
 * Lock mechanism C file
 */


#include "LockMechanism.h"
#include "stm32l4xx_hal.h"
#include "main.h"

void latch_door()
{
	HAL_GPIO_WritePin(SOLENOID_LATCH_GPIO_Port, SOLENOID_LATCH_Pin, GPIO_PIN_RESET);
}


void unlatch_door()
{
	HAL_GPIO_WritePin(SOLENOID_LATCH_GPIO_Port, SOLENOID_LATCH_Pin, GPIO_PIN_SET);
}
