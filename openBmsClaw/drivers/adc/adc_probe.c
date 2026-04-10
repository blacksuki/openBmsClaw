#include "drivers/adc/adc_probe.h"

#define RCC_BASE_ADDR 0x40021000u
#define GPIOA_BASE_ADDR 0x40010800u
#define ADC1_BASE_ADDR 0x40012400u

#define RCC_CFGR_OFFSET 0x04u
#define RCC_APB2ENR_OFFSET 0x18u

#define GPIO_CRL_OFFSET 0x00u

#define ADC_SR_OFFSET 0x00u
#define ADC_CR2_OFFSET 0x08u
#define ADC_SMPR2_OFFSET 0x10u
#define ADC_SQR1_OFFSET 0x2Cu
#define ADC_SQR2_OFFSET 0x30u
#define ADC_SQR3_OFFSET 0x34u
#define ADC_DR_OFFSET 0x4Cu

#define RCC_CFGR (*(volatile uint32_t *)(RCC_BASE_ADDR + RCC_CFGR_OFFSET))
#define RCC_APB2ENR (*(volatile uint32_t *)(RCC_BASE_ADDR + RCC_APB2ENR_OFFSET))

#define GPIOA_CRL (*(volatile uint32_t *)(GPIOA_BASE_ADDR + GPIO_CRL_OFFSET))

#define ADC1_SR (*(volatile uint32_t *)(ADC1_BASE_ADDR + ADC_SR_OFFSET))
#define ADC1_CR2 (*(volatile uint32_t *)(ADC1_BASE_ADDR + ADC_CR2_OFFSET))
#define ADC1_SMPR2 (*(volatile uint32_t *)(ADC1_BASE_ADDR + ADC_SMPR2_OFFSET))
#define ADC1_SQR1 (*(volatile uint32_t *)(ADC1_BASE_ADDR + ADC_SQR1_OFFSET))
#define ADC1_SQR2 (*(volatile uint32_t *)(ADC1_BASE_ADDR + ADC_SQR2_OFFSET))
#define ADC1_SQR3 (*(volatile uint32_t *)(ADC1_BASE_ADDR + ADC_SQR3_OFFSET))
#define ADC1_DR (*(volatile uint32_t *)(ADC1_BASE_ADDR + ADC_DR_OFFSET))

#define RCC_CFGR_ADCPRE_MASK (0x3u << 14)
#define RCC_CFGR_ADCPRE_DIV8 (0x3u << 14)

#define RCC_APB2ENR_IOPAEN (1u << 2)
#define RCC_APB2ENR_ADC1EN (1u << 9)

#define ADC_INPUT_PIN_INDEX 6u
#define ADC_INPUT_CRL_SHIFT (ADC_INPUT_PIN_INDEX * 4u)
#define ADC_INPUT_CRL_MASK (0xFu << ADC_INPUT_CRL_SHIFT)
#define ADC_INPUT_ANALOG_MODE (0x0u << ADC_INPUT_CRL_SHIFT)

#define ADC_CHANNEL_6 6u

#define ADC_CR2_ADON (1u << 0)
#define ADC_CR2_CAL (1u << 2)
#define ADC_CR2_RSTCAL (1u << 3)
#define ADC_CR2_EXTSEL_SWSTART (0x7u << 17)
#define ADC_CR2_EXTTRIG (1u << 20)
#define ADC_CR2_SWSTART (1u << 22)

#define ADC_SR_EOC (1u << 1)

#define ADC_SMPR2_CHANNEL_6_SHIFT (ADC_CHANNEL_6 * 3u)
#define ADC_SMPR2_CHANNEL_6_MASK (0x7u << ADC_SMPR2_CHANNEL_6_SHIFT)
#define ADC_SMPR2_SAMPLE_239_5_CYCLES (0x7u << ADC_SMPR2_CHANNEL_6_SHIFT)

static void adc_probe_delay(volatile uint32_t cycles)
{
    while (cycles-- > 0u)
    {
        __asm volatile ("nop");
    }
}

void adc_probe_init(void)
{
    RCC_APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_ADC1EN;
    GPIOA_CRL = (GPIOA_CRL & ~ADC_INPUT_CRL_MASK) | ADC_INPUT_ANALOG_MODE;

    RCC_CFGR = (RCC_CFGR & ~RCC_CFGR_ADCPRE_MASK) | RCC_CFGR_ADCPRE_DIV8;

    ADC1_SQR1 = 0u;
    ADC1_SQR2 = 0u;
    ADC1_SQR3 = ADC_CHANNEL_6;
    ADC1_SMPR2 =
        (ADC1_SMPR2 & ~ADC_SMPR2_CHANNEL_6_MASK) |
        ADC_SMPR2_SAMPLE_239_5_CYCLES;

    ADC1_CR2 = ADC_CR2_ADON;
    adc_probe_delay(4000u);

    ADC1_CR2 |= ADC_CR2_RSTCAL;
    while ((ADC1_CR2 & ADC_CR2_RSTCAL) != 0u)
    {
    }

    ADC1_CR2 |= ADC_CR2_CAL;
    while ((ADC1_CR2 & ADC_CR2_CAL) != 0u)
    {
    }

    ADC1_CR2 = ADC_CR2_ADON | ADC_CR2_EXTSEL_SWSTART | ADC_CR2_EXTTRIG;
    (void)adc_probe_read_once();
}

uint16_t adc_probe_read_once(void)
{
    ADC1_SR = 0u;
    ADC1_SQR3 = ADC_CHANNEL_6;
    ADC1_CR2 |= ADC_CR2_SWSTART;

    while ((ADC1_SR & ADC_SR_EOC) == 0u)
    {
    }

    return (uint16_t)(ADC1_DR & 0xFFFFu);
}

uint16_t adc_probe_read_average(uint32_t sample_count)
{
    uint32_t total = 0u;
    uint32_t count = sample_count;

    if (count == 0u)
    {
        count = 1u;
    }

    for (uint32_t index = 0u; index < count; ++index)
    {
        total += adc_probe_read_once();
    }

    return (uint16_t)(total / count);
}
