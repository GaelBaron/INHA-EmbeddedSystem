#include "st_basic.h"

//Keypad variables
const char keypad[16] = { '1', '2', '3', 'A', '4', '5', '6', 'B', '7', '8', '9', 'C', '*', '0', '#', 'D' };
const char password[4] = { '4', '4', '4', '4' };
unsigned char isPushed[16] = { };
unsigned int sequence = 0;

//Timer variables
unsigned char flagCaptureMiss = 0;

//Stepper variables
const int stepperPin[4] = { 2, 3, 6, 7 };
const unsigned int stepperHalfState[8][4] = { { 1, 0, 0, 0 }, { 1, 1, 0, 0 }, { 0, 1, 0, 0 }, { 0, 1, 1, 0 }, { 0, 0, 1, 0 }, { 0, 0, 1, 1 }, { 0, 0, 0, 1 }, { 1, 0, 0, 1 } };
int currentStep = 0;
int isOpening = 1;
unsigned char isMotorMove = 0;

void PasswordInput(char character);
void Timer2_Init(void);
int isTooClose(void);																							
unsigned int InputCapture(void);
unsigned int DistanceCalc(unsigned int value);
void Step(int step);

int main(void)
{
	int tmpLol = 0;
	unsigned int x = 0;
	unsigned int z = 0;
	unsigned int i = 0;
	unsigned int j = 0;

	ClockInit();
	USART2_Init();
	Timer2_Init();
	for (; x <= 3; x++) GPIO_Init(GPIOA, x, GPIO_INPUT_PULLDOWN);
	GPIO_Init(GPIOC, 14, GPIO_OUTPUT);
	for (; z < 4; z++)
		GPIO_Init(GPIOB, stepperPin[z], GPIO_OUTPUT);

	while (1)
	{
		//When motor is moving, stop if something is too close or if delay expired
		if (isMotorMove)
		{
			if (isTooClose()) { isMotorMove = 0; }
			else Step(1);
			if (tmpLol < 1000) tmpLol++;
			if (tmpLol >= 999) {
				isMotorMove = 0;
				currentStep = 0;
				isOpening = isOpening * -1;
			}
		}
		//When motor is stopped, check the keypad
		else
		{
			tmpLol = 0;
			for (i = 0; i < 4; i++)
			{
				GPIOE->BSRR |= (1 << ((i - 1 + 4) % 4 + 12 + 16));
				Delay(1);
				GPIO_Init(GPIOE, (i - 1 + 4) % 4 + 12, GPIO_ANALOG);
				GPIO_Init(GPIOE, i + 12, GPIO_OUTPUT);
				GPIOE->BSRR |= (1 << (i + 12));
				Delay(1);
				for (j = 0; j < 4; j++)
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
}

void PasswordInput(char character)
{
	if (password[sequence] == character)
	{
		if (sequence == 3)
		{
			isMotorMove = 1;
			sequence = 0;
		}
		else sequence++;
	}
	else sequence = 0;
}

void Timer2_Init(void)
{
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
	GPIO_Init(GPIOA, 5, GPIO_ALTERNATIVE);
	GPIOA->AFR[0] |= (1 << GPIO_AFRL_AFSEL5_Pos);
	TIM2->CCMR1 |= (3 << TIM_CCMR1_IC1F_Pos) | TIM_CCMR1_CC1S_0;
	TIM2->CCER |= (TIM_CCER_CC1NP | TIM_CCER_CC1P | TIM_CCER_CC1E);
	TIM2->CR1 |= TIM_CR1_CEN;
}

int isTooClose(void)
{
	return ((InputCapture() < 10)/ (80 * 58) ? 1 : 0);
}

unsigned int InputCapture(void)
{
	switch (flagCaptureMiss)
	{
		case 1:
			if (TIM2->SR & TIM_SR_CC1IF)
			{
				TIM2->SR &= ~TIM_SR_CC1IF;
				flagCaptureMiss = 2;
			}
			
			return 500 * 80 * 58;
		case 2:
			if (TIM2->SR & TIM_SR_CC1IF)
			{
				TIM2->SR &= ~TIM_SR_CC1IF;
				flagCaptureMiss = 0;
			}
			else return 500 * 80 * 58;
			break;
		default: break;
	}
	
	GPIOC->BSRR = GPIO_BSRR_BS14;
	Delay(1);
	GPIOC->BSRR = GPIO_BSRR_BR14;
	TIM2->CNT = 0;
	while (!(TIM2->SR & TIM_SR_CC1IF)) if (TIM2->CNT >= 20 * 80 * 1000)
	{
		flagCaptureMiss = 1;
		return 500 * 80 * 58;
	}
	TIM2->SR &= ~TIM_SR_CC1IF;
	TIM2->CNT = 0;
	while (!(TIM2->SR & TIM_SR_CC1IF));
	return TIM2->CCR1;
}


void Step(int step)
{
	int i = 0;
	int direction = (step > 0) ? 1 : -1;

	while (step != 0)
	{
		currentStep = (currentStep + direction + 8) % 8;
		for (; i < 4; i++)
		{
			GPIOB->BSRR = (stepperHalfState[currentStep][i] << stepperPin[i]);
			GPIOB->BSRR = (!stepperHalfState[currentStep][i] << (stepperPin[i] + 16));
		}
		step -= direction;
		Delay(2);
	}
}
