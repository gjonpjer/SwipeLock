#include "Audio.h"
#include "main.h"
#include "stm32l4xx_hal.h"


void playSound(void)
{
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
	HAL_Delay(50);
	HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_3);
}

//void stopSound(void)
//{
//	HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_3);
//}
