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

void Step(int step);

int main(void)
{
	ClockInit();
	GPIO_Init(GPIOA, 1, GPIO_INPUT_PULLDOWN);
	GPIO_Init(GPIOA, 2, GPIO_INPUT_PULLDOWN);
	for (int i = 0; i < 4; i++)
		GPIO_Init(GPIOB, stepperPin[i], GPIO_OUTPUT);
	
	while (1)
	{
		if (GPIOA->IDR & GPIO_IDR_ID1) Step(-200);
		else if (GPIOA->IDR & GPIO_IDR_ID2) Step(200);
	}
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
