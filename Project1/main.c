#include "st_basic.h"

const char keypad[16] = { '1', '2', '3', 'A',
	                        '4', '5', '6', 'B',
	                        '7', '8', '9', 'C',
	                        '*', '0', '#', 'D' };
const char password[4] = { '4', '4', '4', '4' };

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