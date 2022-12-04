#include "st_basic.h"

void ADC1_Init(void);
void NumberToString(unsigned int number);

int main(void)
{
	ClockInit();
	USART2_Init();
	ADC1_Init();
	
	while (1)
	{
		ADC1->CR |= ADC_CR_ADSTART;
		while (ADC1->CR & ADC_CR_ADSTART);
		NumberToString(ADC1->DR);
		
		Delay(500);
	}
}

void ADC1_Init(void)
{
	RCC->AHB2ENR |= RCC_AHB2ENR_ADCEN;
	
	GPIO_Init(GPIOA, 5, GPIO_ANALOG);
	GPIOA->ASCR |= GPIO_ASCR_ASC5;
	
	ADC1->SMPR2 = (5 << ADC_SMPR2_SMP10_Pos);
	ADC1->SQR1 = (10 << ADC_SQR1_SQ1_Pos);
	ADC123_COMMON->CCR = ADC_CCR_PRESC_3 | ADC_CCR_PRESC_0
                     | ADC_CCR_CKMODE_1 | ADC_CCR_CKMODE_0;
	
	ADC1->CR = ADC_CR_ADVREGEN;
	Delay(1);
	ADC1->CR |= ADC_CR_ADCAL;
	while (ADC1->CR & ADC_CR_ADCAL);
	ADC1->CR |= ADC_CR_ADEN;
	while (!(ADC1->ISR & ADC_ISR_ADRDY));
}

void NumberToString(unsigned int number)
{
	char result[6];
	for (int i = 3, j = 1; i >= 0; i--, j *= 10)
		result[i] = (number / j) % 10 + '0';
	result[4] = '\n';
	result[5] = '\0';
	
	USART2_TX_String(result);
}
