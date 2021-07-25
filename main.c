#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

#include "stm32f4xx_hal.h"
#include "stm32f4xx_it.h"
#include "edge_device.h"
#include <stdio.h>

char* DoReMi_Eng[9] = {"Mute", " Do ", " Re ", " Mi ", " Pa ", "Sol ", " Ra ", " si ", "Do# "};
char tlcd_str[100];


int main(int argc, char* argv[])
{
	Edge_PIEZO_Init();				// PIEZO : PA2 Init
	Edge_SW_EXTI_Init();			// SW0~3 Init
	Edge_MOTOR1_Init();				// Motor1 Init

	// Setup CLCD
	CLCD_config();
	CLCD_init();
	CLCD_put_string("test");

	TIMER10_Config();
	HAL_TIM_Base_Start_IT(&TimHandle10);
	HAL_TIM_OC_Start_IT(&TimHandle10, TIM_CHANNEL_1);

	while (1)
	{

	}
}

// IRQHandler(ISR) 함수
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_pin)
{
	if (GPIO_pin == GPIO_PIN_12) {
		if (i == 8) {
			i = 0;
		}
		else {
			i++;
		}
		TimHandle10.Init.Period = ((168000000 / 10) / DoReMi[i]) - 1;
		HAL_TIM_Base_Init(&TimHandle10);

		TIM_OCInit.Pulse = (((16800000 / 10) / DoReMi[i])/2) - 1;
		HAL_TIM_OC_ConfigChannel(&TimHandle10, &TIM_OCInit, TIM_CHANNEL_1);
		HAL_TIM_OC_Init(&TimHandle10);

		sprintf(tlcd_str, "Scale : %s", DoReMi_Eng[i]);
		CLCD_write(0, 0xC0);
		CLCD_put_string(tlcd_str);
	}
	else if (GPIO_pin == GPIO_PIN_13) {
		if (i == 0) {
			i = 8;
		}
		else {
			i--;
		}
		TimHandle10.Init.Period = ((168000000 / 10) / DoReMi[i]) - 1;
		HAL_TIM_Base_Init(&TimHandle10);

		TIM_OCInit.Pulse = (((168000000 / 10) / DoReMi[i])/2) - 1;
		HAL_TIM_OC_ConfigChannel(&TimHandle10, &TIM_OCInit, TIM_CHANNEL_1);
		HAL_TIM_OC_Init(&TimHandle10);

		sprintf(tlcd_str, "Scale : %s", DoReMi_Eng[i]);
		CLCD_write(0, 0xC0);
		CLCD_put_string(tlcd_str);
	}
	else if (GPIO_pin == GPIO_PIN_14) {
		MOTOR1_MODE(CW);
	}
	else if (GPIO_pin == GPIO_PIN_15) {
		MOTOR1_MODE(CCW);
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)		// 주기 완료 함수
{
	if (htim->Instance == TIM10) {
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, 0);
	}
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)	// OC INT Callback
{
	if (htim->Instance == TIM10) {
		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, 1);
	}
}

#pragma GCC diagnostic pop
