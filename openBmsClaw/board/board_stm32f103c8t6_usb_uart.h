#ifndef OPEN_BMS_CLAW_BOARD_BOARD_STM32F103C8T6_USB_UART_H
#define OPEN_BMS_CLAW_BOARD_BOARD_STM32F103C8T6_USB_UART_H

#define BOARD_NAME "stm32f103c8t6_usb_uart_core"

/*
 * These macros describe the current verified baseline, not every capability
 * that the MCU could theoretically provide.
 */
#define BOARD_HAS_STATUS_LED 1
#define BOARD_HAS_UART_LOG 1
#define BOARD_HAS_I2C_BUS 1
#define BOARD_HAS_ADC_CHANNEL 1

#endif
