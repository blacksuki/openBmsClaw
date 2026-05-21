#include "hal_exti.h"

#define RCC_BASE_ADDR 0x40021000u
#define GPIOA_BASE_ADDR 0x40010800u
#define GPIOB_BASE_ADDR 0x40010C00u
#define GPIOC_BASE_ADDR 0x40011000u
#define AFIO_BASE_ADDR 0x40010000u
#define EXTI_BASE_ADDR 0x40010400u
#define NVIC_BASE_ADDR 0xE000E100u

#define RCC_APB2ENR_OFFSET 0x18u

#define GPIO_CRL_OFFSET 0x00u
#define GPIO_CRH_OFFSET 0x04u
#define GPIO_ODR_OFFSET 0x0Cu

#define AFIO_EXTICR1_OFFSET 0x08u
#define AFIO_EXTICR2_OFFSET 0x0Cu
#define AFIO_EXTICR3_OFFSET 0x10u
#define AFIO_EXTICR4_OFFSET 0x14u

#define EXTI_IMR_OFFSET 0x00u
#define EXTI_RTSR_OFFSET 0x08u
#define EXTI_FTSR_OFFSET 0x0Cu
#define EXTI_SWIER_OFFSET 0x10u
#define EXTI_PR_OFFSET 0x14u

#define NVIC_ISER0_OFFSET 0x00u
#define NVIC_IPR2_OFFSET 0x308u /* EXTI4 (IRQ 10) priority register offset */

#define RCC_APB2ENR (*(volatile uint32_t *)(RCC_BASE_ADDR + RCC_APB2ENR_OFFSET))
#define AFIO_EXTICR1 (*(volatile uint32_t *)(AFIO_BASE_ADDR + AFIO_EXTICR1_OFFSET))
#define AFIO_EXTICR2 (*(volatile uint32_t *)(AFIO_BASE_ADDR + AFIO_EXTICR2_OFFSET))
#define AFIO_EXTICR3 (*(volatile uint32_t *)(AFIO_BASE_ADDR + AFIO_EXTICR3_OFFSET))
#define AFIO_EXTICR4 (*(volatile uint32_t *)(AFIO_BASE_ADDR + AFIO_EXTICR4_OFFSET))

#define EXTI_IMR (*(volatile uint32_t *)(EXTI_BASE_ADDR + EXTI_IMR_OFFSET))
#define EXTI_RTSR (*(volatile uint32_t *)(EXTI_BASE_ADDR + EXTI_RTSR_OFFSET))
#define EXTI_FTSR (*(volatile uint32_t *)(EXTI_BASE_ADDR + EXTI_FTSR_OFFSET))
#define EXTI_SWIER (*(volatile uint32_t *)(EXTI_BASE_ADDR + EXTI_SWIER_OFFSET))
#define EXTI_PR (*(volatile uint32_t *)(EXTI_BASE_ADDR + EXTI_PR_OFFSET))

#define NVIC_ISER0 (*(volatile uint32_t *)(NVIC_BASE_ADDR + NVIC_ISER0_OFFSET))

#define RCC_APB2ENR_AFIOEN (1u << 0)
#define RCC_APB2ENR_IOPAEN (1u << 2)
#define RCC_APB2ENR_IOPBEN (1u << 3)
#define RCC_APB2ENR_IOPCEN (1u << 4)

/* 外部中断非阻塞回调静态指针数组 (0 - 15 线对应) */
static hal_exti_callback_t s_exti_callbacks[16] = {0};

bool hal_exti_init(hal_exti_port_t port, uint8_t pin_index,
                   hal_exti_trigger_t trigger, hal_exti_callback_t callback) {
  volatile uint32_t *gpio_base = 0;
  uint32_t rcc_iop_en = 0;

  if (pin_index > 15) {
    return false;
  }

  /* 1. 确定 GPIO 基地址及使能时钟 */
  switch (port) {
    case HAL_EXTI_PORT_A:
      gpio_base = (volatile uint32_t *)GPIOA_BASE_ADDR;
      rcc_iop_en = RCC_APB2ENR_IOPAEN;
      break;
    case HAL_EXTI_PORT_B:
      gpio_base = (volatile uint32_t *)GPIOB_BASE_ADDR;
      rcc_iop_en = RCC_APB2ENR_IOPBEN;
      break;
    case HAL_EXTI_PORT_C:
      gpio_base = (volatile uint32_t *)GPIOC_BASE_ADDR;
      rcc_iop_en = RCC_APB2ENR_IOPCEN;
      break;
    default:
      return false;
  }

  /* 开启时钟：GPIO 与 AFIO */
  RCC_APB2ENR |= rcc_iop_en | RCC_APB2ENR_AFIOEN;

  /* 2. 配置引脚为上拉输入模式 (Input with Pull-Up) */
  volatile uint32_t *gpio_cr = gpio_base + (pin_index < 8 ? (GPIO_CRL_OFFSET / 4) : (GPIO_CRH_OFFSET / 4));
  uint8_t bit_shift = (pin_index % 8) * 4;
  
  /* 清除当前配置，设为上拉/下拉输入模式 (CNF: 0x2, MODE: 0x0 -> Value: 0x8) */
  *gpio_cr = (*gpio_cr & ~(0xFu << bit_shift)) | (0x8u << bit_shift);

  /* 设置 ODR 对应位为 1 启用上拉 (SoC INT脚为开漏低电平触发，故强制上拉) */
  volatile uint32_t *gpio_odr = gpio_base + (GPIO_ODR_OFFSET / 4);
  *gpio_odr |= (1u << pin_index);

  /* 3. 配置 AFIO 映射 */
  uint8_t exticr_index = pin_index / 4;
  uint8_t exticr_shift = (pin_index % 4) * 4;
  volatile uint32_t *exticr = 0;

  if (exticr_index == 0) {
    exticr = &AFIO_EXTICR1;
  } else if (exticr_index == 1) {
    exticr = &AFIO_EXTICR2;
  } else if (exticr_index == 2) {
    exticr = &AFIO_EXTICR3;
  } else {
    exticr = &AFIO_EXTICR4;
  }

  /* 清除映射并设置新映射 (PORT_A = 0, PORT_B = 1, PORT_C = 2) */
  *exticr = (*exticr & ~(0xFu << exticr_shift)) | (((uint32_t)port & 0xF) << exticr_shift);

  /* 4. 配置 EXTI 触发边沿 */
  uint32_t pin_mask = (1u << pin_index);
  
  if (trigger == HAL_EXTI_TRIGGER_RISING) {
    EXTI_RTSR |= pin_mask;
    EXTI_FTSR &= ~pin_mask;
  } else if (trigger == HAL_EXTI_TRIGGER_FALLING) {
    EXTI_RTSR &= ~pin_mask;
    EXTI_FTSR |= pin_mask;
  } else {
    EXTI_RTSR |= pin_mask;
    EXTI_FTSR |= pin_mask;
  }

  /* 注册 C 语言非阻塞回调 */
  s_exti_callbacks[pin_index] = callback;

  /* 5. 使能 EXTI 中断线及 NVIC 中断使能 */
  hal_exti_enable(pin_index);

  /* 对于 PA4 (EXTI4)，其 IRQ 编号为 10，开启 NVIC ISER0 对应位 */
  if (pin_index == 4) {
    /* 设置 EXTI4 优先级为较高 (在 NVIC_IPR2 中配置，此处我们保持默认并使能即可) */
    NVIC_ISER0 |= (1u << 10);
  }

  return true;
}

void hal_exti_enable(uint8_t pin_index) {
  if (pin_index <= 15) {
    EXTI_IMR |= (1u << pin_index);
  }
}

void hal_exti_disable(uint8_t pin_index) {
  if (pin_index <= 15) {
    EXTI_IMR &= ~(1u << pin_index);
  }
}

void hal_exti_software_trigger(uint8_t pin_index) {
  if (pin_index <= 15) {
    EXTI_SWIER |= (1u << pin_index);
  }
}

/* ============================================================================
 * EXTI4 硬件中断服务子程序 (ISR)
 * ============================================================================
 */
void EXTI4_IRQHandler(void) {
  /* 检查中断挂起标志是否由于 EXTI4 触发 */
  if ((EXTI_PR & (1u << 4)) != 0u) {
    /* 向 Pending 寄存器写 1 清除标志，防止发生重复进入中断死循环 */
    EXTI_PR = (1u << 4);

    /* 调度注册的高速回调函数 */
    if (s_exti_callbacks[4] != 0) {
      s_exti_callbacks[4]();
    }
  }
}
