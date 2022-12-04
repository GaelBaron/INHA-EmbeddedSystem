#include "st_basic.h"

static const char keypad[16] = { '1', '2', '3', 'A',
	                        '4', '5', '6', 'B',
	                        '7', '8', '9', 'C',
	                        '*', '0', '#', 'D' };
static const char password[4] = { '9', '9', '9', '9' };

static unsigned char isPushed[16] = { };
static unsigned int sequence = 0;

void PasswordInput(char character);

int main(void)
{
	unsigned int x = 0;
	unsigned int i = 0;
	unsigned int j = 0;

	ClockInit();
	for (; x <= 3; x++) GPIO_Init(GPIOA, x, GPIO_INPUT_PULLDOWN);
	GPIO_Init(GPIOB, 2, GPIO_OUTPUT);
	GPIO_Init(GPIOE, 8, GPIO_OUTPUT);
	
	GPIOB->BSRR = GPIO_BSRR_BS2;
	
	while (1)
	{
		for (; i < 4; i++)
		{
			GPIOE->BSRR |= (1 << ((i - 1 + 4) % 4 + 12 + 16));
			Delay(1);
			GPIO_Init(GPIOE, (i - 1 + 4) % 4 + 12, GPIO_ANALOG);
			
			GPIO_Init(GPIOE, i + 12, GPIO_OUTPUT);
			GPIOE->BSRR |= (1 << (i + 12));
			Delay(1);
			
			for (; j < 4; j++)
			{
				if (GPIOA->IDR & (1 << j))
				{
					if (!isPushed[4 * i + j])
					{
						PasswordInput(keypad[4 * i + j]);
						isPushed[4 * i + j] = 1;
					}
				}
				
				else isPushed[4 * i + j] = 0;
			}
		}
	}
}

void PasswordInput(char character)
{
	if (password[sequence] == character)
	{
		if (sequence == 3)
		{
			GPIOB->BSRR = GPIO_BSRR_BR2;
			GPIOE->BSRR = GPIO_BSRR_BS8;
			Delay(3000);
			GPIOB->BSRR = GPIO_BSRR_BS2;
			GPIOE->BSRR = GPIO_BSRR_BR8;
			
			sequence = 0;
		}
		else sequence++;
	}
	else sequence = 0;
}







/*
#include "st_basic.h"
#include <stdio.h>

const char keypad[16] = { '1', '2', '3', 'A',
	                        '4', '5', '6', 'B',
	                        '7', '8', '9', 'C',
	                        '*', '0', '#', 'D' };
const char password[4] = { '1', '1', '1', '1' };

unsigned char isPushed[16] = { };
unsigned int sequence = 0;

void PasswordInput(char character);

int main(void)
{
	ClockInit();
	for (int i = 0; i <= 3; i++) GPIO_Init(GPIOA, i, GPIO_INPUT_PULLDOWN);
	GPIO_Init(GPIOB, 2, GPIO_OUTPUT);
	GPIO_Init(GPIOE, 8, GPIO_OUTPUT);
	
	GPIOB->BSRR = GPIO_BSRR_BS2;
	
	while (1)
	{
		for (int i = 0; i < 4; i++)
		{
			GPIOE->BSRR |= (1 << ((i - 1 + 4) % 4 + 12 + 16));
			Delay(1);
			GPIO_Init(GPIOE, (i - 1 + 4) % 4 + 12, GPIO_ANALOG);
			
			GPIO_Init(GPIOE, i + 12, GPIO_OUTPUT);
			GPIOE->BSRR |= (1 << (i + 12));
			Delay(1);
			
			for (int j = 0; j < 4; j++)
			{
				if (GPIOA->IDR & (1 << j))
				{
					if (!isPushed[4 * i + j])
					{
						printf("%c\n", keypad[4 * i + j]);
						PasswordInput(keypad[4 * i + j]);
						isPushed[4 * i + j] = 1;
					}
				}
				
				else isPushed[4 * i + j] = 0;
			}
		}
	}
}

void PasswordInput(char character)
{
	if (password[sequence] == character)
	{
		if (sequence == 3)
		{
			GPIOB->BSRR = GPIO_BSRR_BR2;
			GPIOE->BSRR = GPIO_BSRR_BS8;
			Delay(3000);
			GPIOB->BSRR = GPIO_BSRR_BS2;
			GPIOE->BSRR = GPIO_BSRR_BR8;
			
			sequence = 0;
		}
		else sequence++;
	}
	else sequence = 0;
}
*/





/*#define HALF_STEP
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

*/








/*#include <stm32l4xx.h>


void ClockInit(void);

int main(void)
{
	int i = 0;
	int j = 0;
	ClockInit();
	
	RCC->AHB2ENR = RCC_AHB2ENR_GPIOBEN | RCC_AHB2ENR_GPIOEEN;
	GPIOB->MODER &= ~GPIO_MODER_MODE2_1;
	GPIOE->MODER &= ~GPIO_MODER_MODE8_1;
	
	while (1)
	{
		GPIOB->BSRR = GPIO_BSRR_BS2;
		GPIOE->BSRR = GPIO_BSRR_BS8;
		
		for (; i < 1000000; i++);
		
		GPIOB->BSRR = GPIO_BSRR_BR2;
		GPIOE->BSRR = GPIO_BSRR_BR8;
		for (; j < 1000000; j++);
	}
}

void ClockInit(void)
{
	FLASH->ACR |= FLASH_ACR_LATENCY_4WS;

	RCC->PLLCFGR = RCC_PLLCFGR_PLLREN | (20 << RCC_PLLCFGR_PLLN_Pos)
							 | RCC_PLLCFGR_PLLM_0 | RCC_PLLCFGR_PLLSRC_HSI;
	
	RCC->CR |= RCC_CR_PLLON | RCC_CR_HSION;
	
	while (!((FLASH->ACR & FLASH_ACR_LATENCY_4WS)
				&& (RCC->CR & RCC_CR_PLLRDY)
				&& (RCC->CR & RCC_CR_HSIRDY)));

	RCC->CFGR = RCC_CFGR_SW_PLL;
	
	RCC->CR &= ~RCC_CR_MSION;
}
*/
