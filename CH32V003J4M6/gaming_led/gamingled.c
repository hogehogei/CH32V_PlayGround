#include "ch32v003fun.h"
#include <stdint.h>

int main()
{
	SystemInit();

	// Enable GPIOC
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD;

	// GPIO C1 Push-Pull
	GPIOC->CFGLR &= ~(0xF<<(4*1));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*1);

	// GPIO C2 Push-Pull
	GPIOC->CFGLR &= ~(0xf<<(4*2));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*2);

	// GPIO D6 Push-Pull
	GPIOD->CFGLR &= ~(0xf<<(4*6));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*6);


	while(1)
	{
		GPIOC->BSHR = 0x00000006;		// Set GPIO C1, C2
		GPIOD->BSHR = 0x00000040;		// Set GPIO D6
		Delay_Ms( 250 );
		GPIOC->BSHR = 0x00060000;		// Clear GPIO C1, C2
		GPIOD->BSHR = 0x00400000;		// Clear GPIO D6
		Delay_Ms( 250 );
	}
}
