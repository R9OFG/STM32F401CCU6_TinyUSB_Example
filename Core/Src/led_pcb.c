/*
 * led_pcb.c
 *
 * Created on: Oct 27, 2025
 * Author: R9OFG.RU
 *
 * MCU: STM32H723ZGT6
 * LED: Configure pin PC13 as GPIO_Output, GPIO level - High, GPIO Mode - Output, No pull-up and no pull-down, User Label - LED_PCB
 * Add: in main.c or other module - #include "led_pcb.h" //Led pcb
 */

#include "led_pcb.h"

//Функция смены состояния светика с неблокирующей задержкой
void Toggle_LED(void)
{
	//Не блокирующая задержка в 50мс
	static uint32_t last_toggle_time = 0;
	if (HAL_GetTick() - last_toggle_time >= 100)
	{
		//Меняем состояние светика на PG7
		GPIOC->ODR ^= GPIO_ODR_OD13;
		last_toggle_time = HAL_GetTick();
	}
}
