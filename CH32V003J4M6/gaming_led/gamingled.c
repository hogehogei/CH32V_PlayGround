#include "ch32v003fun.h"
#include <stdint.h>

void Init_Timer2(void)
{
	RCC->APB1PCENR |= RCC_APB1Periph_TIM2;

	// Reset TIM2 to init all regs
	RCC->APB1PRSTR |= RCC_APB1Periph_TIM2;
	RCC->APB1PRSTR &= ~RCC_APB1Periph_TIM2;

	// SMCFGR: default clk input is CK_INT
	// set TIM2 clock prescaler divider 
	TIM2->PSC = (uint16_t)47;       // 48MHz / (47+1) = 1000KHz(Timer CK_CNT)
	// set PWM total cycle width
	TIM2->ATRLR = (uint16_t)1024;	// 1000KHz / 1024 = 1000Hz(1ms)
	
	// for channel 1, 3, 4, let CCxS stay 00 (output), set OCxM to 111 (PWM II)
	// enabling preload causes the new pulse width in compare capture register only to come into effect when UG bit in SWEVGR is set (= initiate update) (auto-clears)
	TIM2->CHCTLR1 |= TIM_OC1M_2 | TIM_OC1M_1 | TIM_OC1M_0 | TIM_OC1PE;
	TIM2->CHCTLR2 |= TIM_OC3M_2 | TIM_OC3M_1 | TIM_OC3M_0 | TIM_OC3PE | TIM_OC4M_2 | TIM_OC4M_1 | TIM_OC4M_0 | TIM_OC4PE;
	
	// CTLR1: default is up, events generated, edge align
	// enable auto-reload of preload
	TIM2->CTLR1 |= TIM_ARPE;

	// Enable CH1 output, positive pol
	TIM2->CCER |= TIM_CC1E | TIM_CC1P;
	// Enable CH3 output, positive pol
	TIM2->CCER |= TIM_CC3E | TIM_CC3P;
	// Enable CH4 output, positive pol
	TIM2->CCER |= TIM_CC4E | TIM_CC4P;

	// initialize counter
	TIM2->SWEVGR |= TIM_UG;

	// Enable TIM2
	TIM2->CTLR1 |= TIM_CEN;
}

void Timer2_SetPWMWidth(uint8_t ch, uint16_t width)
{
	switch(ch & 3)
	{
	case 0: 
		TIM2->CH1CVR = width; 
		break;
	case 1: 
		TIM2->CH2CVR = width; 
		break;
	case 2: 
		TIM2->CH3CVR = width; 
		break;
	case 3: 
		TIM2->CH4CVR = width; 
		break;
	}
	TIM2->SWEVGR |= TIM_UG; // load new value in compare capture register
}

int main()
{
	uint32_t max_duty = 1024;
	uint32_t count = 0;
	SystemInit();

	// Enable GPIOC, GPIOD
	RCC->APB2PCENR |= RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD;

	// Port8, Disable SWIO(using as GPIO)
	AFIO->PCFR1 |= AFIO_PCFR1_SWJ_CFG_DISABLE;
	// Timer2 port remap
	// (CH1/ETR/PC1, CH2/PC7, CH3/PD6, CH4/PD5)
	AFIO->PCFR1 |= AFIO_PCFR1_TIM2_REMAP_FULLREMAP;


	// GPIO C1 Push-Pull 
	GPIOC->CFGLR &= ~(0xF<<(4*1));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*1);
	// GPIO D5 Push-Pull
	GPIOD->CFGLR &= ~(0xf<<(4*5));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*5);
	// GPIO D6 Push-Pull
	GPIOD->CFGLR &= ~(0xf<<(4*6));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*6);
	
	Init_Timer2();

	while(1)
	{
		Timer2_SetPWMWidth( 0, (uint16_t)(count % max_duty) );
		Timer2_SetPWMWidth( 2, (uint16_t)((count + (max_duty/3)) % max_duty) );
		Timer2_SetPWMWidth( 3, (uint16_t)((count + ((max_duty/3)*2)) % max_duty) );
		count = (count + 1) % max_duty;

		Delay_Ms(5);
	}
}
