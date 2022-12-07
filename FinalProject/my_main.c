#include "st_basic.h"

//Keypad variables
const char keypad[16] = { '1', '2', '3', 'A', '4', '5', '6', 'B', '7', '8', '9', 'C', '*', '0', '#', 'D' };
const char password[4] = { '4', '4', '4', '4' };
unsigned char isPushed[16] = { };

//Stepper variables
unsigned int stepperPin[4] = { 2, 3, 6, 7 };
const unsigned int stepperHalfState[8][4] = { { 1, 0, 0, 0 }, { 1, 1, 0, 0 }, { 0, 1, 0, 0 }, { 0, 1, 1, 0 }, { 0, 0, 1, 0 }, { 0, 0, 1, 1 }, { 0, 0, 0, 1 }, { 1, 0, 0, 1 } };

void PasswordInput(char character, unsigned char *motorMoving, unsigned int *sequence);
void Timer2_Init(void);
int isTooClose(unsigned char *flagCaptureMiss);																							
unsigned int InputCapture(unsigned char *flagCaptureMiss);
unsigned int DistanceCalc(unsigned int value);
void Step(int step, int *currentStep);

int main(void)
{
	int tmpLol = 0;
	unsigned int x = 0;
	unsigned int z = 0;
	unsigned int i = 0;
	unsigned int j = 0;
	unsigned char motorMoving = 0;
	int currentStep = 0;
	int isOpening = 1;// Unused because the motor can't turn on the oposite way
	unsigned int sequence = 0;
	unsigned char flagCaptureMiss = 0;

	ClockInit();
	USART2_Init();
	Timer2_Init();

	//Keypad GPIO initialization
	for (; x <= 3; x++) GPIO_Init(GPIOA, x, GPIO_INPUT_PULLDOWN);
	//Ultrasonic GPIO initialization
	GPIO_Init(GPIOC, 14, GPIO_OUTPUT);
	//Stepper GPIO initialization
	for (; z < 4; z++)
		GPIO_Init(GPIOB, stepperPin[z], GPIO_OUTPUT);

	while (1)
	{
		//When motor is moving, stop if something is too close or if delay expired
		if (motorMoving)
		{
			if (isTooClose(&flagCaptureMiss)) { motorMoving= 0; }//If something is to cloose, stop the motor rotation
			else Step(1, &currentStep);	
			if (tmpLol < 1000) tmpLol++;
			//stop the motor roation after the quarter of a total rotation
			if (tmpLol >= 999) {
				motorMoving = 0;
				currentStep = 0;
				isOpening = isOpening * -1;//variable used for the rotation sense change, not implemented but ready to be
			}
		}
		//When motor is stopped, check the keypad
		else
		{
			tmpLol = 0;
			//Scan all the rows
			for (i = 0; i < 4; i++)
			{
				//Clear the currently activated output during a short period
				//After discharging the capacitor, set the line to analog mode
				GPIOE->BSRR |= (1 << ((i - 1 + 4) % 4 + 12 + 16));
				Delay(1);
				GPIO_Init(GPIOE, (i - 1 + 4) % 4 + 12, GPIO_ANALOG);
				//Set the next row to output mode and set the output to high
				//Wait for the capacitor to be charged
				GPIO_Init(GPIOE, i + 12, GPIO_OUTPUT);
				GPIOE->BSRR |= (1 << (i + 12));
				Delay(1);
				//Scan all the columns
				for (j = 0; j < 4; j++)
				{
					//Check if the input is high
					if (GPIOA->IDR & (1 << j))
					{
						//If the button is pushed for the first time, get it as a true input
						//Mark the flag not to get the input continuously
						if (!isPushed[4 * i + j])
						{
							PasswordInput(keypad[4 * i + j], &motorMoving, &sequence);
							isPushed[4 * i + j] = 1;
						}
					}
					//If the button is not pushed, clear the flag to get the next button push
					else isPushed[4 * i + j] = 0;
				}
			}
		}
	}
}

void PasswordInput(char character, unsigned char *motorMoving, unsigned int *sequence)
{
	//Check if the input character matches the password
	if (password[*sequence] == character)
	{
		//Check if the input character is the last character
		if (*sequence == 3)
		{
			//If so, start rotating the motor and reset the sequence index
			*motorMoving = 1;
			*sequence = 0;
		}
		//If the input character is not the last character, keep getting input
		else (*sequence)++;
	}
	//If the password is wrong, reset the sequence index
	else *sequence = 0;
}

void Timer2_Init(void)
{
	//Enable the clock to the timer
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
	//Set PA5 to the timer channel pin
	GPIO_Init(GPIOA, 5, GPIO_ALTERNATIVE);
	GPIOA->AFR[0] |= (1 << GPIO_AFRL_AFSEL5_Pos);
	//Set the digital filter to sample 8 times and make the channel as an input
	TIM2->CCMR1 |= (3 << TIM_CCMR1_IC1F_Pos) | TIM_CCMR1_CC1S_0;
	//Make the input capture unit sensitive to both rising and falling edges
	TIM2->CCER |= (TIM_CCER_CC1NP | TIM_CCER_CC1P | TIM_CCER_CC1E);
	//Enable the timer
	TIM2->CR1 |= TIM_CR1_CEN;
}

//check if there's any object or person to close to the door
int isTooClose(unsigned char *flagCaptureMiss)
{
	return ((InputCapture(flagCaptureMiss) < 10)/ (80 * 58) ? 1 : 0);
}

unsigned int InputCapture(unsigned char *flagCaptureMiss)
{
	//Check flagCaptureMiss
	switch (*flagCaptureMiss)
	{
		//1 when the capture is timed out and waiting for the rising edge
		case 1:
			//Check if the rising edge has occured
			//In that case, clear the input capture flag and set the state to 2
			if (TIM2->SR & TIM_SR_CC1IF)
			{
				TIM2->SR &= ~TIM_SR_CC1IF;
				*flagCaptureMiss = 2;
			}
			
			//After that, return the value as there is no obstacle
			return 500 * 80 * 58;
		//2 when the input is high and waiting for the falling edge
		case 2:
			//Check if the falling edge has occured
			//In that case, reset all the flag and get ready for the next input capture
			if (TIM2->SR & TIM_SR_CC1IF)
			{
				TIM2->SR &= ~TIM_SR_CC1IF;
				*flagCaptureMiss = 0;
			}
			//If not, just return the value as there is no obstacle
			else return 500 * 80 * 58;
			break;
		//0 when the capturing is ready
		//In that case, do nothing and proceed the input capture
		default: break;
	}
	
	//Send a pulse to the ultrasonic sensor to get a distance value
	GPIOC->BSRR = GPIO_BSRR_BS14;
	Delay(1);
	GPIOC->BSRR = GPIO_BSRR_BR14;
	//Reset the counter value and wait for the rising edge
	//If the rising edge does not appear until 20ms, abort the input capture
	//That is regarded as there is no obstacle
	TIM2->CNT = 0;
	while (!(TIM2->SR & TIM_SR_CC1IF)) if (TIM2->CNT >= 20 * 80 * 1000)
	{
		*flagCaptureMiss = 1;
		return 500 * 80 * 58;
	}
	//Ignore the rising edge by clearing the flag
	TIM2->SR &= ~TIM_SR_CC1IF;
	//Reset the counter value and wait for the falling edge
	TIM2->CNT = 0;
	while (!(TIM2->SR & TIM_SR_CC1IF));
	//Read and return the captured value
	return TIM2->CCR1;
}


void Step(int step, int *currentStep)
{
	int i = 0;
	//Inspect the direction by the sign of the step
	int direction = (step > 0) ? 1 : -1;

	//Repeat until the remaining step becomes 0
	while (step != 0)
	{
		//Increment or decrement 1 step according to the direction
		*currentStep = (*currentStep + direction + 8) % 8;
		//Set or clear the GPIO outputs according to the step sequence
		for (; i < 4; i++)
		{
			GPIOB->BSRR = (stepperHalfState[*currentStep][i] << stepperPin[i]);
			GPIOB->BSRR = (!stepperHalfState[*currentStep][i] << (stepperPin[i] + 16));
		}
		//Decrease the remaining step
		step -= direction;
		//Wait for the rotor to rotate
		Delay(2);
	}
}
