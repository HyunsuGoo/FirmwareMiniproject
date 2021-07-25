/*
 * edge_device.h
 *
 *  Created on: 2021. 5. 20.
 *      Author: Administrator
 */

#ifndef EDGE_DEVICE_H_
#define EDGE_DEVICE_H_

#define	STOP	0
#define	CW		1
#define	CCW		2

// GPIO
GPIO_InitTypeDef LED;
GPIO_InitTypeDef PIEZO;
GPIO_InitTypeDef SW;
GPIO_InitTypeDef MOTOR;
GPIO_InitTypeDef FND;
GPIO_InitTypeDef CLCD;
GPIO_InitTypeDef CDS, VR;
ADC_HandleTypeDef ADCHandler1, ADCHandler2;
ADC_ChannelConfTypeDef sConfig;
GPIO_InitTypeDef UART;
UART_HandleTypeDef UartHandle;
TIM_HandleTypeDef TimHandle10;
TIM_OC_InitTypeDef TIM_OCInit;

uint8_t FND_DATA_TBL[] = {0x3F,0X06,0X5B,0X4F,0X66,0X6D,0X7D,0X07,0X7F,
						  0X67,0X77,0X7C,0X39,0X5E,0X79,0X71,0X08,0X80};

uint32_t DoReMi[9] = {0, 523, 587, 659, 698, 783, 880, 987, 1046};
uint8_t i = 0;

//	ms_delay routine
void ms_delay_int_count (volatile unsigned long nTime)
{
	nTime = nTime * 14000;
	for(; nTime > 0; nTime--);
}

//	us_delay routine
void us_delay_int_count (volatile unsigned long nTime)
{
	nTime = nTime * 12;
	for(; nTime > 0; nTime--);
}

void Edge_PIEZO_Init(void)
{
	// PIEZO Init
	// Using PORTA2
	__HAL_RCC_GPIOA_CLK_ENABLE();
	PIEZO.Pin		= GPIO_PIN_2;
	PIEZO.Mode		= GPIO_MODE_OUTPUT_PP;
	PIEZO.Pull		= GPIO_NOPULL;					// 사용안함
	PIEZO.Speed		= GPIO_SPEED_HIGH;				// HIGH 설정
	HAL_GPIO_Init(GPIOA, &PIEZO);
}

void Edge_SW_EXTI_Init(void){
	__HAL_RCC_GPIOB_CLK_ENABLE();

	SW.Pin		= GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
	SW.Mode		= GPIO_MODE_IT_RISING;	// EXTI
	SW.Pull		= GPIO_NOPULL;
	SW.Speed	= GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOB, &SW);

	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void Edge_MOTOR1_Init(void)
{
	// MOTOR1
	// PORTB8~9, PORTA3
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	MOTOR.Pin	= GPIO_PIN_8 | GPIO_PIN_9;
	MOTOR.Mode	= GPIO_MODE_OUTPUT_PP;
	MOTOR.Pull	= GPIO_NOPULL;
	MOTOR.Speed	= GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOB, &MOTOR);

	MOTOR.Pin	= GPIO_PIN_3;
	HAL_GPIO_Init(GPIOA, &MOTOR);
}

void MOTOR1_MODE(uint8_t mode)
{
	if(mode == STOP){
		//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);		// MOTTOR1_EN
		//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);		// MOTTOR1_INT1
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);		// MOTTOR1_INT2
	}
	else if(mode == CW){
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);		// MOTTOR1_EN
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);		// MOTTOR1_INT1
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);	// MOTTOR1_INT2
	}
	else if(mode == CCW){
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);		// MOTTOR1_EN
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);	// MOTTOR1_INT1
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);		// MOTTOR1_INT2
	}
}

void CLCD_config(void)
{
	// LCD DATA4~7 -> PORTC 4~7
	// LCD RW, RS, E -> PORTB 0, 1, 2

	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	CLCD.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
	CLCD.Mode = GPIO_MODE_OUTPUT_PP;
	CLCD.Pull = GPIO_NOPULL;
	CLCD.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOB, &CLCD);

	CLCD.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
	HAL_GPIO_Init(GPIOC, &CLCD);
}

void CLCD_write(uint8_t rs, uint8_t data)
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);		// rw 0
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, rs);					// rs
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);		// Enable reset
	us_delay_int_count(2);

	// HIGH bit
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, (data >> 4) & 0x01);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, (data >> 5) & 0x01);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, (data >> 6) & 0x01);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, (data >> 7) & 0x01);

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);		// Enable set
	us_delay_int_count(2);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);	// Enable set
	us_delay_int_count(2);

	// LOW bit
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, (data >> 0) & 0x01);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, (data >> 1) & 0x01);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, (data >> 2) & 0x01);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, (data >> 3) & 0x01);

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);		// Enable set
	us_delay_int_count(2);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);	// Enable set
	ms_delay_int_count(2);
}

void CLCD_init(void)
{
	CLCD_write(0, 0x33);		// control signal	4bit 설정 특수 명령
	CLCD_write(0, 0x32);		// control signal	4bit 설정 특수 명령
	CLCD_write(0, 0x28);		// control signal	set_function
	CLCD_write(0, 0x0F);		// control signal	set_display
	CLCD_write(0, 0x01);		// control signal	CLCD_clear
	CLCD_write(0, 0x06);		// control signal	set_entry_mode
	CLCD_write(0, 0x02);		// control signal	return_home

}

void CLCD_put_string(char* str)
{
	uint8_t i;

	if (!str)
	{
		return;
	}

	for (i = 0; str[i] != '\0' ; i++)
	{
		CLCD_write(1, str[i]);
	}
}

void VR_ADC2_Config(void)
{
	__HAL_RCC_ADC2_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	VR.Pin = GPIO_PIN_1;
	VR.Mode = GPIO_MODE_ANALOG;
	HAL_GPIO_Init(GPIOA, &VR);

	ADCHandler2.Instance = ADC2;
	ADCHandler2.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV8;
	ADCHandler2.Init.Resolution = ADC_RESOLUTION_12B;
	ADCHandler2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	ADCHandler2.Init.ScanConvMode = DISABLE;
	ADCHandler2.Init.ContinuousConvMode = ENABLE;		// ENABLE : 연속모드 DISABLE : 단일모드
	ADCHandler2.Init.NbrOfConversion = 1;
	ADCHandler2.Init.ExternalTrigConv = ADC_SOFTWARE_START;

	HAL_ADC_Init(&ADCHandler2);

	sConfig.Channel = ADC_CHANNEL_1;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
	HAL_ADC_ConfigChannel(&ADCHandler2, &sConfig);

	// Enable Interrupt & permission
	HAL_NVIC_SetPriority(ADC_IRQn, 10, 0);
	HAL_NVIC_EnableIRQ(ADC_IRQn);
}

void TIMER10_Config(void){
	__HAL_RCC_TIM10_CLK_ENABLE();

	// 기본 타이머 설정
	TimHandle10.Instance			= TIM10;			// TIM10
	TimHandle10.Init.Period			= ((168000000 / 10)/DoReMi[i]) - 1;
	TimHandle10.Init.Prescaler 		=  10 - 1;
	TimHandle10.Init.ClockDivision 	= TIM_CLOCKDIVISION_DIV1;
	TimHandle10.Init.CounterMode	= TIM_COUNTERMODE_UP;
	HAL_TIM_Base_Init(&TimHandle10);

	// OC setup
	TIM_OCInit.OCMode = TIM_OCMODE_TIMING;
	TIM_OCInit.Pulse = (((168000000 / 10)/DoReMi[i])/2) - 1;	// duty rate
	HAL_TIM_OC_ConfigChannel(&TimHandle10, &TIM_OCInit, TIM_CHANNEL_1);
	HAL_TIM_OC_Init(&TimHandle10);

	//
	HAL_NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 3, 0);
	HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
}

#endif /* EDGE_DEVICE_H_ */
