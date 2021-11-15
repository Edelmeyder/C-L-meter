#include "rlc.h"
#include "lcd.h"
#include <stm32f103x6.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define PI 3.141592654F

float inductance, Pe, F;
uint16_t n_measure;

void RLC_print(float);

void RLC_init() {
	n_measure = 0;
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;	// enable PORT A clock
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; // enable timer 2 clock
	GPIOA->CRL = (GPIOA->CRL & 0xF3FFF8FF) | 0x03000800; // PA2 (TIM2 CH3): input pull-up/down - PA6 GP output
	GPIOA->ODR |= (1<<2); // Pull-up
	TIM2->CCMR2 = 0x0001;  // Pin TIM2_CH3 as input for channel 3 - prescaler 1 - filter 1
	TIM2->CCER = (1<<8); // CC3P = 0 (rising edge), CC3E = 1 enable
	TIM2->PSC = PRESCALER; // prescaller 7200 - 72MHz/7200 = 10kHz
	TIM2->ARR = 50000-1;
	LCD_goToXY(0, 1);
	LCD_sendString((char*) "L= ", 3); 
}

void RLC_measure() {
	//float Pe, F;
	uint32_t t0, t;
	TIM2->CNT = 0;
	TIM2->CR1 = 1; // start counting up
	GPIOA->BSRR = (1<<6); //raise PA6
	while((TIM2->SR &(1<<3)) == 0); // wait until the CC3IF flag sets
	t0 = TIM2->CCR3; // gets initial count
	while((TIM2->SR &(1<<3)) == 0); // wait until the CC3IF flag sets
	t = TIM2->CCR3; // gets final count
	Pe = (t - t0) / 72000000.0; // contador sobre frecuencia de contador
	F = 1 / Pe; // frecuencia = 1 / periodo
	inductance = 1 / ((2*PI*F) * (2*PI*F) * CAP); // L = 1/F^2*C
	TIM2->CR1 = 0; // stops counting
	GPIOA->BRR = (1<<6);
	n_measure++;
}

void RLC_print(float L) {
	char *str;
	str = (char*) malloc(13*sizeof(char));
	if (inductance > 0.001) {
		inductance *= 1000;
		sprintf(str, "%7.2f mH %d", inductance, n_measure);
	} else if (inductance > 0.000001) {
		inductance *= 1000000;
		sprintf(str, "%7.2f uH %d", inductance, n_measure);
	} else {
		inductance *= 1000000000;
		sprintf(str, "%7.2f nH %d", inductance, n_measure);
	}
	LCD_goToXY(3, 1);
	LCD_sendString(str, 13);
}
