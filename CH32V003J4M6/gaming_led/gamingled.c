#include "ch32v003fun.h"
#include <stdint.h>

#define   MAX_DUTY ((uint16_t)1024)
#define   RED_BRIGHT_ALPHA      ((uint32_t)MAX_DUTY)
#define   GREEN_BRIGHT_ALPHA    ((uint32_t)MAX_DUTY * 50 / 100)
#define   BLUE_BRIGHT_ALPHA     ((uint32_t)MAX_DUTY * 50 / 100)

typedef struct 
{
    float r, g, b;
} Color;

void Init_Timer2(void)
{
    RCC->APB1PCENR |= RCC_APB1Periph_TIM2;

    // Reset TIM2 to init all regs
    RCC->APB1PRSTR |= RCC_APB1Periph_TIM2;
    RCC->APB1PRSTR &= ~RCC_APB1Periph_TIM2;

    // SMCFGR: default clk input is CK_INT
    // set TIM2 clock prescaler divider 
    TIM2->PSC = (uint16_t)479;      // 48MHz / (479+1) = 100KHz(Timer CK_CNT)
    // set PWM total cycle width
    TIM2->ATRLR = MAX_DUTY;	        // 100KHz / 1024 = 100Hz(10ms)
    
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
}

float floorf( float x )
{
    if( x > 0.f ){
        return (long) x;
    }
    
    return x < (long) x ? (long)(x - 1.0) : (long) x;
}

float roundf( float x )
{
  return x > 0.0 ? (long)(x + 0.5) : (long)(x - 0.5);
}

float absf( float x )
{
    return x < 0.f ? -x : x;
}

 // HSV(HSB)色空間からRGB色空間へ変換する 
 //  h(hue)       : 色相/色合い   0-360度の値
 //  s(saturation): 彩度/鮮やかさ 0-100%の値
 //  v(Value)     : 明度/明るさ   0-100%の値 
 //  ※v は b(Brightness)と同様 
 Color hsv2rgb(float h, float s, float v)
 {
#define  EPSILON ((float)0.00001)
    Color rgb = { 0.f, 0.f, 0.f };
    
    if ( absf(h - 360.f) < EPSILON ){
        h = 0.f;
    }
    
    s = s / 100.f;   
    v = v / 100.f;   
    
    if ( s < EPSILON ){
        rgb.r = v * 255.f;
        rgb.g = v * 255.f;
        rgb.b = v * 255.f;
        return rgb;
    } 
    
    int dh = floorf(h / 60.f);
    float p = v * (1.f - s);
    float q = v * (1.f - s * (h / 60.f - dh));
    float t = v * (1.f - s * (1.f - (h / 60.f - dh)));
    
    switch (dh){
        case 0 : rgb.r = v; rgb.g = t; rgb.b = p;  break;
        case 1 : rgb.r = q; rgb.g = v; rgb.b = p;  break;
        case 2 : rgb.r = p; rgb.g = v; rgb.b = t;  break;
        case 3 : rgb.r = p; rgb.g = q; rgb.b = v;  break;
        case 4 : rgb.r = t; rgb.g = p; rgb.b = v;  break;
        case 5 : rgb.r = v; rgb.g = p; rgb.b = q;  break;
    }   
    
    rgb.r =  roundf(rgb.r * 255);
    rgb.g =  roundf(rgb.g * 255);
    rgb.b =  roundf(rgb.b * 255);

    return rgb; 
 } 

int main()
{
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

    // Red: CH3, Green: CH1, Blue: CH4
    Color rgb;
    while(1)
    {
        rgb = hsv2rgb( count, 80, 80 );
        Timer2_SetPWMWidth( 0, rgb.g * GREEN_BRIGHT_ALPHA / 256 );
        Timer2_SetPWMWidth( 2, rgb.r * RED_BRIGHT_ALPHA / 256 );
        Timer2_SetPWMWidth( 3, rgb.b * BLUE_BRIGHT_ALPHA / 256 );

        count = (count + 1) % 360;

        Delay_Ms(5);
    }
}
