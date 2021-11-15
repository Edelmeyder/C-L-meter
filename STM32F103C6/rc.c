#include "rc.h"
#include "lcd.h"
#include <stm32f103x6.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

volatile uint32_t v = 0, cycles = 0, n_medida = 0;
volatile float capacitance, tau;

void delay_uss(uint16_t);
void RC_print(void);

void RC_init() {
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;	// enable PORT A clock
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;	// enable ADC1 clock
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; // enable timer 2 clock
	RCC->CFGR &= 0xFFFF3FFF;
	GPIOA->CRL = (GPIOA->CRL & 0xFF3FFF0F) | 0x00300000; // PA1 analog input, PA5 GP output
	GPIOA->BRR = (1<<5);
	ADC1->CR2 = 1; // ADC1 on
	ADC1->SMPR2 = (0<<3); // SMPR2->SMP1 = 001 => Sample time Ch1 7.5 adc cycles
	ADC1->SQR3 = 1; // Channel 1
	TIM2->ARR = 50000-1;
	LCD_goToXY(0, 0);
	LCD_sendString((uint8_t*) "C= ", 3); 
}

void RC_measure() {
	v = 0;
	TIM2->CNT = 0; // Reset counter
	//TIM2->CR1 = 1; // Start counting
	ADC1->CR2 |= 1; // Start conversion
	while((ADC1->SR) & (1<<1) == 0); // wait for EOC
	delay_uss(1);
	GPIOA->BSRR = (1<<5); // Raise PA5
	delay_uss(1);
	TIM2->CR1 = 1; // Start counting
	while(v < 2607) { // wait to reach 62.3% of 3V3
		ADC1->CR2 |= 1; // Start conversion
		while((ADC1->SR) & (1<<1) == 0); // wait for EOC
		v = ADC1->DR;
	}
	cycles = TIM2->CNT; 
	TIM2->CR1 = 0; // Stop timer counter
	//tau = cycles / 10000.0;
	tau = cycles / 72000000.0;
	capacitance = tau / RES;
	GPIOA->BRR = (1<<5);
	n_medida++;
}

void RC_Print(void) {
	char* str;
	str = (char*) malloc(13*sizeof(char));
	if (capacitance > 0.001) {
		capacitance *= 1000;
		sprintf(str, "%7.2f mF %d", capacitance, n_medida);
	} else if (capacitance > 0.000001) {
		capacitance *= 1000000;
		sprintf(str, "%7.2f uF %d", capacitance, n_medida);
	} else {
		capacitance *= 1000000000;
		sprintf(str, "%7.2f nF %d", capacitance, n_medida);
	}
	LCD_goToXY(3, 0);
	LCD_sendString((uint8_t*)str, 16);
}

void delay_uss(uint16_t t) {
	uint16_t l, i;
	for(i = 0; i < t; i++)
		for(l = 0; l < 12; l++){}
}
