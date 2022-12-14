#include "my_st_basic.h"

unsigned int sysMillis = 0;

void ClockInit(void)
{
	FLASH->ACR |= FLASH_ACR_LATENCY_4WS;
	
	RCC->PLLCFGR = RCC_PLLCFGR_PLLREN | (20 << RCC_PLLCFGR_PLLN_Pos)
	             | RCC_PLLCFGR_PLLM_0 | RCC_PLLCFGR_PLLSRC_HSI;
	
	RCC->CR |= RCC_CR_PLLON | RCC_CR_HSION;
	
	while (!((FLASH->ACR & FLASH_ACR_LATENCY_4WS) && (RCC->CR & RCC_CR_PLLRDY)
		    && (RCC->CR & RCC_CR_HSIRDY)));
	
	RCC->CFGR = RCC_CFGR_SW_PLL;
	
	RCC->CR &= ~RCC_CR_MSION;
	
	SysTick->LOAD = SysTick->CALIB & SysTick_LOAD_RELOAD_Msk;
	SysTick->CTRL = SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}

void GPIO_Init(GPIO_TypeDef *port, unsigned int pin, GPIO_Mode mode)
{
	unsigned int modeIn32Bit = ((mode & 3) << (2 * pin));
	unsigned int pullUpDown = ((mode >> 2) << (2 * pin));
	
	RCC->AHB2ENR |= (1 << (((unsigned int)port - GPIOA_BASE) >> 10));
	
	port->MODER |= modeIn32Bit;
	port->MODER &= (modeIn32Bit | ~(3 << (2 * pin)));
	
	port->PUPDR |= pullUpDown;
	port->PUPDR &= (pullUpDown | ~(3 << (2 * pin)));
}

void USART2_Init(void)
{
	RCC->CCIPR |= RCC_CCIPR_USART2SEL_1;
	RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;
	
	GPIO_Init(GPIOD, 5, GPIO_ALTERNATIVE);
	GPIO_Init(GPIOD, 6, GPIO_ALTERNATIVE);
	GPIOD->AFR[0] |= (7 << GPIO_AFRL_AFSEL6_Pos) | (7 << GPIO_AFRL_AFSEL5_Pos);
	
	USART2->BRR = 139;
	USART2->CR3 |= USART_CR3_OVRDIS;
	USART2->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

void Delay(unsigned int duration)
{
	unsigned int prevMillis = sysMillis;
	while (sysMillis - prevMillis <= duration);
}

char USART2_RX(void)
{
	while (!(USART2->ISR & USART_ISR_RXNE));
	return USART2->RDR;
}

void USART2_TX(char character)
{
	while (!(USART2->ISR & USART_ISR_TXE));
	USART2->TDR = character;
}

void USART2_TX_String(const char *string)
{
	while (*string != '\0') USART2_TX(*string++);
}

void SysTick_Handler(void) { sysMillis++; }
