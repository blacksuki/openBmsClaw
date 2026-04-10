#include "samples/temperature_alarm/temperature_alarm_sample_port.h"

#define RCC_BASE_ADDR 0x40021000u
#define GPIOA_BASE_ADDR 0x40010800u
#define GPIOB_BASE_ADDR 0x40010C00u
#define ADC1_BASE_ADDR 0x40012400u

#define RCC_CFGR_OFFSET 0x04u
#define RCC_APB2ENR_OFFSET 0x18u

#define GPIO_CRL_OFFSET 0x00u
#define GPIO_CRH_OFFSET 0x04u
#define GPIO_IDR_OFFSET 0x08u
#define GPIO_BSRR_OFFSET 0x10u
#define GPIO_BRR_OFFSET 0x14u

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
#define GPIOA_IDR (*(volatile uint32_t *)(GPIOA_BASE_ADDR + GPIO_IDR_OFFSET))
#define GPIOA_BSRR (*(volatile uint32_t *)(GPIOA_BASE_ADDR + GPIO_BSRR_OFFSET))
#define GPIOA_BRR (*(volatile uint32_t *)(GPIOA_BASE_ADDR + GPIO_BRR_OFFSET))
#define GPIOB_CRL (*(volatile uint32_t *)(GPIOB_BASE_ADDR + GPIO_CRL_OFFSET))
#define GPIOB_CRH (*(volatile uint32_t *)(GPIOB_BASE_ADDR + GPIO_CRH_OFFSET))
#define GPIOB_BSRR (*(volatile uint32_t *)(GPIOB_BASE_ADDR + GPIO_BSRR_OFFSET))
#define GPIOB_BRR (*(volatile uint32_t *)(GPIOB_BASE_ADDR + GPIO_BRR_OFFSET))

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
#define RCC_APB2ENR_IOPBEN (1u << 3)
#define RCC_APB2ENR_ADC1EN (1u << 9)

#define HEARTBEAT_LED_PIN_INDEX 2u
#define ALARM_LED_PIN_INDEX 3u
#define THERMISTOR_AO_PIN_INDEX 4u
#define THERMISTOR_DO_PIN_INDEX 5u
#define WARNING_AUX_LED_PIN_INDEX 1u
#define BUZZER_SIG_PIN_INDEX 9u

#define HEARTBEAT_LED_CRL_SHIFT (HEARTBEAT_LED_PIN_INDEX * 4u)
#define ALARM_LED_CRL_SHIFT (ALARM_LED_PIN_INDEX * 4u)
#define THERMISTOR_AO_CRL_SHIFT (THERMISTOR_AO_PIN_INDEX * 4u)
#define THERMISTOR_DO_CRL_SHIFT (THERMISTOR_DO_PIN_INDEX * 4u)
#define WARNING_AUX_LED_CRL_SHIFT (WARNING_AUX_LED_PIN_INDEX * 4u)
#define BUZZER_SIG_CRH_SHIFT ((BUZZER_SIG_PIN_INDEX - 8u) * 4u)

#define GPIO_CONFIG_MASK(shift) (0xFu << (shift))
#define GPIO_MODE_ANALOG_INPUT(shift) (0x0u << (shift))
#define GPIO_MODE_FLOATING_INPUT(shift) (0x4u << (shift))
#define GPIO_MODE_OUTPUT_PP_2MHZ(shift) (0x2u << (shift))

#define HEARTBEAT_LED_PIN_MASK (1u << HEARTBEAT_LED_PIN_INDEX)
#define ALARM_LED_PIN_MASK (1u << ALARM_LED_PIN_INDEX)
#define WARNING_AUX_LED_PIN_MASK (1u << WARNING_AUX_LED_PIN_INDEX)
#define BUZZER_SIG_PIN_MASK (1u << BUZZER_SIG_PIN_INDEX)
#define THERMISTOR_DO_PIN_MASK (1u << THERMISTOR_DO_PIN_INDEX)

#define ADC_CHANNEL_4 4u

#define ADC_CR2_ADON (1u << 0)
#define ADC_CR2_RSTCAL (1u << 3)
#define ADC_CR2_EXTSEL_SWSTART (0x7u << 17)
#define ADC_CR2_EXTTRIG (1u << 20)
#define ADC_CR2_SWSTART (1u << 22)
#define ADC_CR2_CAL (1u << 2)

#define ADC_SR_EOC (1u << 1)
#define ADC_SMPR2_CHANNEL_4_SHIFT (ADC_CHANNEL_4 * 3u)
#define ADC_SMPR2_SAMPLE_239_5_CYCLES (0x7u << ADC_SMPR2_CHANNEL_4_SHIFT)

static void temperature_alarm_sample_port_delay(volatile uint32_t cycles)
{
    while (cycles-- > 0u)
    {
        __asm volatile ("nop");
    }
}

static void temperature_alarm_sample_port_adc_init(void)
{
    RCC_CFGR = (RCC_CFGR & ~RCC_CFGR_ADCPRE_MASK) | RCC_CFGR_ADCPRE_DIV8;

    ADC1_SQR1 = 0u;
    ADC1_SQR2 = 0u;
    ADC1_SQR3 = ADC_CHANNEL_4;
    ADC1_SMPR2 =
        (ADC1_SMPR2 & ~(0x7u << ADC_SMPR2_CHANNEL_4_SHIFT)) |
        ADC_SMPR2_SAMPLE_239_5_CYCLES;

    ADC1_CR2 = ADC_CR2_ADON;
    temperature_alarm_sample_port_delay(4000u);

    ADC1_CR2 |= ADC_CR2_RSTCAL;
    while ((ADC1_CR2 & ADC_CR2_RSTCAL) != 0u)
    {
    }

    ADC1_CR2 |= ADC_CR2_CAL;
    while ((ADC1_CR2 & ADC_CR2_CAL) != 0u)
    {
    }

    ADC1_CR2 = ADC_CR2_ADON | ADC_CR2_EXTSEL_SWSTART | ADC_CR2_EXTTRIG;
    (void)temperature_alarm_sample_port_read_adc_average(1u);
}

static uint16_t temperature_alarm_sample_port_read_adc_once(void)
{
    ADC1_SR = 0u;
    ADC1_SQR3 = ADC_CHANNEL_4;
    ADC1_CR2 |= ADC_CR2_SWSTART;

    while ((ADC1_SR & ADC_SR_EOC) == 0u)
    {
    }

    return (uint16_t)(ADC1_DR & 0xFFFFu);
}

void temperature_alarm_sample_port_init(void)
{
    RCC_APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_ADC1EN;

    GPIOA_CRL =
        (GPIOA_CRL &
         ~(GPIO_CONFIG_MASK(HEARTBEAT_LED_CRL_SHIFT) |
           GPIO_CONFIG_MASK(ALARM_LED_CRL_SHIFT) |
           GPIO_CONFIG_MASK(THERMISTOR_AO_CRL_SHIFT) |
           GPIO_CONFIG_MASK(THERMISTOR_DO_CRL_SHIFT))) |
        GPIO_MODE_OUTPUT_PP_2MHZ(HEARTBEAT_LED_CRL_SHIFT) |
        GPIO_MODE_OUTPUT_PP_2MHZ(ALARM_LED_CRL_SHIFT) |
        GPIO_MODE_ANALOG_INPUT(THERMISTOR_AO_CRL_SHIFT) |
        GPIO_MODE_FLOATING_INPUT(THERMISTOR_DO_CRL_SHIFT);

    GPIOB_CRL =
        (GPIOB_CRL & ~GPIO_CONFIG_MASK(WARNING_AUX_LED_CRL_SHIFT)) |
        GPIO_MODE_OUTPUT_PP_2MHZ(WARNING_AUX_LED_CRL_SHIFT);

    GPIOB_CRH =
        (GPIOB_CRH & ~GPIO_CONFIG_MASK(BUZZER_SIG_CRH_SHIFT)) |
        GPIO_MODE_OUTPUT_PP_2MHZ(BUZZER_SIG_CRH_SHIFT);

    GPIOA_BSRR = HEARTBEAT_LED_PIN_MASK | ALARM_LED_PIN_MASK;
    GPIOB_BRR = WARNING_AUX_LED_PIN_MASK;
    GPIOB_BRR = BUZZER_SIG_PIN_MASK;

    temperature_alarm_sample_port_adc_init();
}

uint16_t temperature_alarm_sample_port_read_adc_average(uint32_t sample_count)
{
    uint32_t total = 0u;
    uint32_t count = sample_count;

    if (count == 0u)
    {
        count = 1u;
    }

    for (uint32_t index = 0u; index < count; ++index)
    {
        total += temperature_alarm_sample_port_read_adc_once();
    }

    return (uint16_t)(total / count);
}

bool temperature_alarm_sample_port_read_digital_threshold(void)
{
    return (GPIOA_IDR & THERMISTOR_DO_PIN_MASK) != 0u;
}

void temperature_alarm_sample_port_set_buzzer(bool on)
{
    if (on)
    {
        GPIOB_BSRR = BUZZER_SIG_PIN_MASK;
    }
    else
    {
        GPIOB_BRR = BUZZER_SIG_PIN_MASK;
    }
}

void temperature_alarm_sample_port_set_heartbeat_led(bool on)
{
    if (on)
    {
        GPIOA_BRR = HEARTBEAT_LED_PIN_MASK;
    }
    else
    {
        GPIOA_BSRR = HEARTBEAT_LED_PIN_MASK;
    }
}

void temperature_alarm_sample_port_set_alarm_led(bool on)
{
    if (on)
    {
        GPIOA_BRR = ALARM_LED_PIN_MASK;
    }
    else
    {
        GPIOA_BSRR = ALARM_LED_PIN_MASK;
    }
}

void temperature_alarm_sample_port_set_warning_aux_led(bool on)
{
    if (on)
    {
        GPIOB_BSRR = WARNING_AUX_LED_PIN_MASK;
    }
    else
    {
        GPIOB_BRR = WARNING_AUX_LED_PIN_MASK;
    }
}
