#define HALF_STEP
#include "st_basic.h"

const int stepperPin[4] = { 2, 3, 6, 7 };
const unsigned int stepperFullState[4][4] = { { 1, 1, 0, 0 },
                                              { 0, 1, 1, 0 },
																							{ 0, 0, 1, 1 },
																							{ 1, 0, 0, 1 } };
const unsigned int stepperHalfState[8][4] = { { 1, 0, 0, 0 },
                                              { 1, 1, 0, 0 },
																			    		{ 0, 1, 0, 0 },
																			    		{ 0, 1, 1, 0 },
																			    		{ 0, 0, 1, 0 },
																				    	{ 0, 0, 1, 1 },
																				    	{ 0, 0, 0, 1 },
																				    	{ 1, 0, 0, 1 } };
int currentStep = 0;

void ADC1_Init(void);
void Step(int step);

int main(void)
{
	ClockInit();
	ADC1_Init();
	for (int i = 0; i < 4; i++)
		GPIO_Init(GPIOB, stepperPin[i], GPIO_OUTPUT);
	
	while (1)
	{
		ADC1->CR |= ADC_CR_ADSTART;
		while (ADC1->CR & ADC_CR_ADSTART);
		unsigned int ADC_Value = ADC1->DR;
		
		unsigned int speed = (ADC_Value > 2047) ? ADC_Value - 2047 : 2047 - ADC_Value;
		if (speed > 50)
		{
			Step((ADC_Value > 2047) ? 1 : -1);
			Delay(2047 / speed);
		}
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

void Step(int step)
{
	int direction = (step > 0) ? 1 : -1;
	while (step != 0)
	{
		#ifdef FULL_STEP
		currentStep = (currentStep + direction + 4) % 4;
		#elif defined HALF_STEP
		currentStep = (currentStep + direction + 8) % 8;
		#endif
		
		for (int i = 0; i < 4; i++)
		{
			#ifdef FULL_STEP
			GPIOB->BSRR = (stepperFullState[currentStep][i] << stepperPin[i]);
			GPIOB->BSRR = (!stepperFullState[currentStep][i] << (stepperPin[i] + 16));
			#elif defined HALF_STEP
			GPIOB->BSRR = (stepperHalfState[currentStep][i] << stepperPin[i]);
			GPIOB->BSRR = (!stepperHalfState[currentStep][i] << (stepperPin[i] + 16));
			#endif
		}
		
		step -= direction;
		Delay(2);
	}
}
