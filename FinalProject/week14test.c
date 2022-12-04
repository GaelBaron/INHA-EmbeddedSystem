#include "st_basic14.h"

const char keypad[16] = { '1', '2', '3', 'A',
	                        '4', '5', '6', 'B',
	                        '7', '8', '9', 'C',
	                        '*', '0', '#', 'D' };

int main(void)
{
	ClockInit();
	USART2_Init();
	
	for (int i = 0; i <= 3; i++) GPIO_Init(GPIOA, i, GPIO_INPUT_PULLDOWN);
	for (int i = 12; i <= 15; i++)
	{
		GPIO_Init(GPIOE, i, GPIO_INPUT);
		GPIOE->BSRR |= (1 << i);
	}
	
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
				if (GPIOA->IDR & (1 << j))
				{
					USART2_TX(keypad[4 * i + j]);
					USART2_TX('\n');
				}
		}
		
		USART2_TX_String("Keypad Scanning Complete!\n\n");
		Delay(200);
	}
}
