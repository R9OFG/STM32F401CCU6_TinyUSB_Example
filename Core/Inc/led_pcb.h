/*
 * led_pcb.h
 *
 * Created on: Oct 27, 2025
 * Author: R9OFG.RU
 */

#ifndef INC_LED_PCB_H_
#define INC_LED_PCB_H_

#include "main.h"

#define LED_OFF()		(GPIOC->BSRR = GPIO_BSRR_BS13)	//Выключаем светик на плате PC13
#define LED_ON()		(GPIOC->BSRR = GPIO_BSRR_BR13)	//Включаем светик на плате PC13
#define LED_TOGGLE()	(GPIOC->ODR ^= LED_PCB_Pin)		//Меняем состояние светика на плате PC13

void Toggle_LED(void);

#endif /* INC_LED_PCB_H_ */
