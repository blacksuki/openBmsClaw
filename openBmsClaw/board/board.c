#include "board.h"

#include "../config/sys_config.h"
#include "../drivers/adc/adc_probe.h"
#include "../drivers/i2c/i2c_probe.h"

#define RCC_BASE_ADDR 0x40021000u
#define GPIOA_BASE_ADDR 0x40010800u

#define RCC_APB2ENR_OFFSET 0x18u
#define GPIO_CRL_OFFSET 0x00u
#define GPIO_CRH_OFFSET 0x04u
#define GPIO_BSRR_OFFSET 0x10u
#define GPIO_BRR_OFFSET 0x14u
#define USART_SR_OFFSET 0x00u
#define USART_DR_OFFSET 0x04u
#define USART_BRR_OFFSET 0x08u
#define USART_CR1_OFFSET 0x0Cu

#define RCC_APB2ENR (*(volatile uint32_t *)(RCC_BASE_ADDR + RCC_APB2ENR_OFFSET))
#define GPIOA_CRL (*(volatile uint32_t *)(GPIOA_BASE_ADDR + GPIO_CRL_OFFSET))
#define GPIOA_CRH (*(volatile uint32_t *)(GPIOA_BASE_ADDR + GPIO_CRH_OFFSET))
#define GPIOA_BSRR (*(volatile uint32_t *)(GPIOA_BASE_ADDR + GPIO_BSRR_OFFSET))
#define GPIOA_BRR (*(volatile uint32_t *)(GPIOA_BASE_ADDR + GPIO_BRR_OFFSET))
#define USART1_BASE_ADDR 0x40013800u
#define USART1_SR (*(volatile uint32_t *)(USART1_BASE_ADDR + USART_SR_OFFSET))
#define USART1_DR (*(volatile uint32_t *)(USART1_BASE_ADDR + USART_DR_OFFSET))
#define USART1_BRR (*(volatile uint32_t *)(USART1_BASE_ADDR + USART_BRR_OFFSET))
#define USART1_CR1 (*(volatile uint32_t *)(USART1_BASE_ADDR + USART_CR1_OFFSET))

#define RCC_APB2ENR_AFIOEN (1u << 0)
#define RCC_APB2ENR_IOPAEN (1u << 2)
#define RCC_APB2ENR_USART1EN (1u << 14)

#define STATUS_LED_PIN_INDEX 1u
#define STATUS_LED_PIN_MASK (1u << STATUS_LED_PIN_INDEX)
#define STATUS_LED_CRL_SHIFT (STATUS_LED_PIN_INDEX * 4u)
#define STATUS_LED_CRL_MASK (0xFu << STATUS_LED_CRL_SHIFT)
#define STATUS_LED_OUTPUT_PP_2MHZ (0x2u << STATUS_LED_CRL_SHIFT)

#define UART_TX_PIN_INDEX 9u
#define UART_RX_PIN_INDEX 10u
#define UART_TX_CRH_SHIFT ((UART_TX_PIN_INDEX - 8u) * 4u)
#define UART_RX_CRH_SHIFT ((UART_RX_PIN_INDEX - 8u) * 4u)
#define UART_TX_CRH_MASK (0xFu << UART_TX_CRH_SHIFT)
#define UART_RX_CRH_MASK (0xFu << UART_RX_CRH_SHIFT)
#define UART_TX_AF_PP_50MHZ (0xBu << UART_TX_CRH_SHIFT)
#define UART_RX_INPUT_FLOATING (0x4u << UART_RX_CRH_SHIFT)

#define USART_SR_TXE (1u << 7)
#define USART_CR1_RE (1u << 2)
#define USART_CR1_TE (1u << 3)
#define USART_CR1_UE (1u << 13)

static void board_uart_init(void) {
  /*
   * This minimal project currently boots without an explicit SystemInit()
   * implementation in the repo, so use the reset-default APB2 clock
   * assumption (HSI 8 MHz) for a stable 115200 configuration.
   */
  USART1_BRR = 0x45u;
  USART1_CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

void board_init(void) {
  RCC_APB2ENR |= RCC_APB2ENR_AFIOEN | RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;
  GPIOA_CRL = (GPIOA_CRL & ~STATUS_LED_CRL_MASK) | STATUS_LED_OUTPUT_PP_2MHZ;
  GPIOA_CRH = (GPIOA_CRH & ~(UART_TX_CRH_MASK | UART_RX_CRH_MASK)) |
              UART_TX_AF_PP_50MHZ | UART_RX_INPUT_FLOATING;
  board_uart_init();
  board_status_led_set(false);
}

void board_status_led_set(bool on) {
  if (on) {
    GPIOA_BRR = STATUS_LED_PIN_MASK;
  } else {
    GPIOA_BSRR = STATUS_LED_PIN_MASK;
  }
}

void board_status_led_toggle(void) {
  static bool s_led_on = false;

  s_led_on = !s_led_on;
  board_status_led_set(s_led_on);
}

void board_busy_wait(volatile uint32_t cycles) {
  while (cycles-- > 0u) {
    __asm volatile("nop");
  }
}

void board_uart_write_string(const char *text) {
  if (text == 0) {
    return;
  }

  while (*text != '\0') {
    while ((USART1_SR & USART_SR_TXE) == 0u) {
    }

    USART1_DR = (uint32_t)(uint8_t)(*text);
    ++text;
  }
}

void board_adc_probe_init(void) { adc_probe_init(); }

uint16_t board_adc_probe_read_once(void) { return adc_probe_read_once(); }

uint16_t board_adc_probe_read_average(uint32_t sample_count) {
  return adc_probe_read_average(sample_count);
}

bool board_i2c_probe_init(void) { return i2c_probe_init(); }

int board_i2c_probe_scan(uint8_t *first_found_address) {
  return i2c_probe_scan(first_found_address);
}

const char *board_name(void) { return BOARD_NAME; }

bool board_has_status_led(void) { return BOARD_HAS_STATUS_LED; }

bool board_has_uart_log(void) { return BOARD_HAS_UART_LOG; }

bool board_has_i2c_bus(void) { return BOARD_HAS_I2C_BUS; }

bool board_has_adc_channel(void) { return BOARD_HAS_ADC_CHANNEL; }
