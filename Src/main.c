#include <stdint.h>
#include "stm32f103xb.h"

#define PWM_PERIOD 0xFF

static void system_init(void);
static void clock_init(void);
static void gpio_init(void);
static void timer2_init(void);
static void delay(volatile uint32_t delay);

int main(void) {
	system_init();
    clock_init();
    gpio_init();
    timer2_init();

    uint32_t duty_cycle = 0;
	for(;;) {
        if(++duty_cycle >= PWM_PERIOD) {
            duty_cycle = 0;
        }

        TIM2->CCR1 = duty_cycle;
        delay((volatile uint32_t)100U);
    }
}

static void system_init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

    AFIO->MAPR &= ~AFIO_MAPR_SWJ_CFG;
    AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_1; // swjcfg 010 --> jtag disabled sw-dp enabled
}

static void clock_init(void) {
    RCC->CR |= RCC_CR_HSION;

    uint32_t timeout = 0xFFFF;
    while(((RCC->CR & RCC_CR_HSIRDY) == 0) && --timeout);

    if((RCC->CR & RCC_CR_HSIRDY) != 0) {
        RCC->CFGR &= ~RCC_CFGR_HPRE; // ahb pre /1

        RCC->CFGR &= ~RCC_CFGR_PPRE1; // apb1 pre /1

        RCC->CFGR &= ~RCC_CFGR_PPRE2; // apb2 pre /1

        RCC->CFGR &= ~RCC_CFGR_ADCPRE; // adc pre /2

        RCC->CFGR &= ~RCC_CFGR_SW; // hsi selected as system clock
        while((RCC->CFGR & RCC_CFGR_SWS) != 0x0);
    }
}

static void gpio_init() {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; // enable port a clock

    GPIOA->CRL |= GPIO_CRL_MODE0_0 | GPIO_CRL_MODE0_1; // output mode, max speed 50 MHz
    GPIOA->CRL &= ~GPIO_CRL_CNF0_0;
    GPIOA->CRL |= GPIO_CRL_CNF0_1; // alternate function output Push-pull
}

static void timer2_init(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; // enable timer 2 clock

    TIM2->PSC = 0; // prescaler = 0
    TIM2->ARR = PWM_PERIOD; // period = 255

    TIM2->CCMR1 &= ~TIM_CCMR1_CC1S; // cc1 channel output
    TIM2->CCMR1 &= ~TIM_CCMR1_OC1CE; // oc1ce 0
    TIM2->CCMR1 |= TIM_CCMR1_OC1PE; // oc1pe 1
    TIM2->CCMR1 &= ~TIM_CCMR1_OC1M;
    TIM2->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2; // pwm mode 1
    TIM2->CCMR1 &= ~TIM_CCMR1_OC1CE; // oc1ce 0

    TIM2->CCR1 = 0x0; // duty cycle 0%

    TIM2->CCER &= ~TIM_CCER_CC1P; // polarity high
    TIM2->CCER |= TIM_CCER_CC1E; // cc1 output enabled

    TIM2->EGR |= TIM_EGR_UG; // re-initialize the counter and generates an update of the registers

    TIM2->CR1 &= ~TIM_CR1_UDIS; // uev enabled
    TIM2->CR1 &= ~TIM_CR1_URS; // uev on overflow/underflow or UG
    TIM2->CR1 &= ~TIM_CR1_OPM; // one pulse disabled
    TIM2->CR1 &= ~TIM_CR1_CMS; // edge-aligned mode
    TIM2->CR1 &= ~TIM_CR1_ARPE; // not buffered
    TIM2->CR1 &= ~TIM_CR1_CKD; // no clk div

    TIM2->CR1 |= TIM_CR1_CEN; // counter enabled
}

static void delay(volatile uint32_t delay) {
    for(uint32_t i = 0; i < delay; i++) {
        for(uint32_t j = 0; j < delay; j++);
    }
}
