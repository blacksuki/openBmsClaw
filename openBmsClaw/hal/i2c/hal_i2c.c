#include "hal_i2c.h"
#include "../../config/board_config.h"

#define RCC_BASE_ADDR 0x40021000u
#define GPIOB_BASE_ADDR 0x40010C00u
#define I2C1_BASE_ADDR 0x40005400u

#define RCC_APB2ENR_OFFSET 0x18u
#define RCC_APB1ENR_OFFSET 0x1Cu

#define GPIO_CRL_OFFSET 0x00u
#define GPIO_IDR_OFFSET 0x08u
#define GPIO_BSRR_OFFSET 0x10u
#define GPIO_BRR_OFFSET 0x14u

#define I2C_CR1_OFFSET 0x00u
#define I2C_CR2_OFFSET 0x04u
#define I2C_OAR1_OFFSET 0x08u
#define I2C_DR_OFFSET 0x10u
#define I2C_SR1_OFFSET 0x14u
#define I2C_SR2_OFFSET 0x18u
#define I2C_CCR_OFFSET 0x1Cu
#define I2C_TRISE_OFFSET 0x20u

#define RCC_APB2ENR (*(volatile uint32_t *)(RCC_BASE_ADDR + RCC_APB2ENR_OFFSET))
#define RCC_APB1ENR (*(volatile uint32_t *)(RCC_BASE_ADDR + RCC_APB1ENR_OFFSET))

#define GPIOB_CRL (*(volatile uint32_t *)(GPIOB_BASE_ADDR + GPIO_CRL_OFFSET))
#define GPIOB_IDR (*(volatile uint32_t *)(GPIOB_BASE_ADDR + GPIO_IDR_OFFSET))
#define GPIOB_BSRR (*(volatile uint32_t *)(GPIOB_BASE_ADDR + GPIO_BSRR_OFFSET))
#define GPIOB_BRR (*(volatile uint32_t *)(GPIOB_BASE_ADDR + GPIO_BRR_OFFSET))

#define I2C1_CR1 (*(volatile uint32_t *)(I2C1_BASE_ADDR + I2C_CR1_OFFSET))
#define I2C1_CR2 (*(volatile uint32_t *)(I2C1_BASE_ADDR + I2C_CR2_OFFSET))
#define I2C1_OAR1 (*(volatile uint32_t *)(I2C1_BASE_ADDR + I2C_OAR1_OFFSET))
#define I2C1_DR (*(volatile uint32_t *)(I2C1_BASE_ADDR + I2C_DR_OFFSET))
#define I2C1_SR1 (*(volatile uint32_t *)(I2C1_BASE_ADDR + I2C_SR1_OFFSET))
#define I2C1_SR2 (*(volatile uint32_t *)(I2C1_BASE_ADDR + I2C_SR2_OFFSET))
#define I2C1_CCR (*(volatile uint32_t *)(I2C1_BASE_ADDR + I2C_CCR_OFFSET))
#define I2C1_TRISE (*(volatile uint32_t *)(I2C1_BASE_ADDR + I2C_TRISE_OFFSET))

#define RCC_APB2ENR_AFIOEN (1u << 0)
#define RCC_APB2ENR_IOPBEN (1u << 3)
#define RCC_APB1ENR_I2C1EN (1u << 21)

#define I2C_SCL_PIN_INDEX 6u
#define I2C_SDA_PIN_INDEX 7u
#define I2C_SCL_PIN_MASK (1u << I2C_SCL_PIN_INDEX)
#define I2C_SDA_PIN_MASK (1u << I2C_SDA_PIN_INDEX)
#define I2C_SCL_CRL_SHIFT (I2C_SCL_PIN_INDEX * 4u)
#define I2C_SDA_CRL_SHIFT (I2C_SDA_PIN_INDEX * 4u)
#define I2C_SCL_CRL_MASK (0xFu << I2C_SCL_CRL_SHIFT)
#define I2C_SDA_CRL_MASK (0xFu << I2C_SDA_CRL_SHIFT)

#define GPIO_MODE_AF_OD_50MHZ 0xFu
#define GPIO_MODE_OUT_OD_2MHZ 0x6u

#define I2C_CR1_PE (1u << 0)
#define I2C_CR1_START (1u << 8)
#define I2C_CR1_STOP (1u << 9)
#define I2C_CR1_ACK (1u << 10)
#define I2C_CR1_SWRST (1u << 15)

#define I2C_SR1_SB (1u << 0)
#define I2C_SR1_ADDR (1u << 1)
#define I2C_SR1_BTF (1u << 2)
#define I2C_SR1_RXNE (1u << 6)
#define I2C_SR1_TXE (1u << 7)
#define I2C_SR1_BERR (1u << 8)
#define I2C_SR1_AF (1u << 10)

#define I2C_SR2_BUSY (1u << 1)

#define I2C_APB1_CLOCK_MHZ 8u
#define I2C_STANDARD_MODE_CCR 40u
#define I2C_STANDARD_MODE_TRISE 9u
#define I2C_OWN_ADDRESS ((0x33u << 1) | (1u << 14))

/* ============================================================================
 * 内部辅助时延与等待函数
 * ============================================================================
 */

static void hal_i2c_delay_us(uint32_t us) {
  /* 近似估算延时，在 8MHz HSI 下 1us 约执行 2-3 条汇编循环 */
  uint32_t count = us * 2u;
  while (count-- > 0u) {
    __asm volatile("nop");
  }
}

static bool hal_i2c_wait_mask(volatile uint32_t *reg, uint32_t mask, bool set,
                              uint32_t timeout_ms) {
  /* 8MHz 时钟下 1ms 约对应 8000 次循环自减 */
  uint32_t cycles = timeout_ms * 8000u;
  if (cycles == 0u) {
    cycles = 8000u;
  }

  while (cycles-- > 0u) {
    if (set) {
      if ((*reg & mask) == mask) {
        return true;
      }
    } else {
      if ((*reg & mask) == 0u) {
        return true;
      }
    }

    /* 检查是否有总线出错或无应答发生 */
    if ((I2C1_SR1 & (I2C_SR1_BERR | I2C_SR1_AF)) != 0u) {
      return false;
    }
  }

  return false;
}

/* ============================================================================
 * 接口实现
 * ============================================================================
 */

bool hal_i2c_init(void) {
  /* 1. 开启时钟 */
  RCC_APB2ENR |= RCC_APB2ENR_AFIOEN | RCC_APB2ENR_IOPBEN;
  RCC_APB1ENR |= RCC_APB1ENR_I2C1EN;

  /* 2. 配置 PB6 (SCL) 与 PB7 (SDA) 为复用开漏输出 50MHz */
  GPIOB_CRL = (GPIOB_CRL & ~(I2C_SCL_CRL_MASK | I2C_SDA_CRL_MASK)) |
              (GPIO_MODE_AF_OD_50MHZ << I2C_SCL_CRL_SHIFT) |
              (GPIO_MODE_AF_OD_50MHZ << I2C_SDA_CRL_SHIFT);

  /* 3. 硬件控制器软复位 */
  I2C1_CR1 = I2C_CR1_SWRST;
  hal_i2c_delay_us(20);
  I2C1_CR1 = 0u;

  /* 4. 设置时钟参数 (标准 100kHz) */
  I2C1_CR2 = I2C_APB1_CLOCK_MHZ;
  I2C1_OAR1 = I2C_OWN_ADDRESS;
  I2C1_CCR = I2C_STANDARD_MODE_CCR;
  I2C1_TRISE = I2C_STANDARD_MODE_TRISE;

  /* 5. 使能外设与应答机制 */
  I2C1_CR1 = I2C_CR1_PE | I2C_CR1_ACK;

  /* 验证总线是否正常 (不被占线) */
  return hal_i2c_wait_mask(&I2C1_SR2, I2C_SR2_BUSY, false, 5u);
}

void hal_i2c_bus_recovery(void) {
#if CONFIG_I2C_BUS_RECOVERY_ENABLE
  /* 1. 将 SCL (PB6) 和 SDA (PB7) 临时转为通用开漏输出模式 */
  GPIOB_CRL = (GPIOB_CRL & ~(I2C_SCL_CRL_MASK | I2C_SDA_CRL_MASK)) |
              (GPIO_MODE_OUT_OD_2MHZ << I2C_SCL_CRL_SHIFT) |
              (GPIO_MODE_OUT_OD_2MHZ << I2C_SDA_CRL_SHIFT);

  /* 2. 发送最多 9 个时钟脉冲，强制解除从机锁死态 */
  for (int i = 0; i < 9; ++i) {
    GPIOB_BRR = I2C_SCL_PIN_MASK; /* SCL 低 */
    hal_i2c_delay_us(10);

    GPIOB_BSRR = I2C_SCL_PIN_MASK; /* SCL 高 */
    hal_i2c_delay_us(10);

    /* 若采样 SDA 为高电平，说明从机已释放总线，可提前中断 */
    if ((GPIOB_IDR & I2C_SDA_PIN_MASK) != 0u) {
      break;
    }
  }

  /* 3. 产生模拟 STOP 信号：SCL 高电平时，SDA 由低变高 */
  GPIOB_BRR = I2C_SDA_PIN_MASK; /* SDA 低 */
  hal_i2c_delay_us(10);
  GPIOB_BSRR = I2C_SCL_PIN_MASK; /* SCL 高 */
  hal_i2c_delay_us(10);
  GPIOB_BSRR = I2C_SDA_PIN_MASK; /* SDA 高 */
  hal_i2c_delay_us(20);

  /* 4. 恢复为 I2C 硬件复用开漏输出 */
  GPIOB_CRL = (GPIOB_CRL & ~(I2C_SCL_CRL_MASK | I2C_SDA_CRL_MASK)) |
              (GPIO_MODE_AF_OD_50MHZ << I2C_SCL_CRL_SHIFT) |
              (GPIO_MODE_AF_OD_50MHZ << I2C_SDA_CRL_SHIFT);

  /* 5. 软复位并重新初始化控制器 */
  hal_i2c_init();
#endif
}

hal_i2c_status_t hal_i2c_write(uint8_t dev_addr, uint8_t reg_addr,
                               const uint8_t *data, uint16_t len,
                               uint32_t timeout_ms) {
  volatile uint32_t temp_reg;

  if ((data == 0) && (len > 0)) {
    return HAL_I2C_ERR_BUS_ERROR;
  }

  /* 1. 检查总线是否忙。若锁死则执行自愈 */
  if ((I2C1_SR2 & I2C_SR2_BUSY) != 0u) {
    hal_i2c_bus_recovery();
    if ((I2C1_SR2 & I2C_SR2_BUSY) != 0u) {
      return HAL_I2C_ERR_BUSY;
    }
  }

  /* 2. 发送 START 信号并等候 */
  I2C1_CR1 |= I2C_CR1_START;
  if (!hal_i2c_wait_mask(&I2C1_SR1, I2C_SR1_SB, true, timeout_ms)) {
    I2C1_CR1 |= I2C_CR1_STOP;
    return HAL_I2C_ERR_TIMEOUT;
  }

  /* 3. 发送从机地址 + 写方向位 (0) */
  I2C1_DR = (uint32_t)(dev_addr << 1u);
  if (!hal_i2c_wait_mask(&I2C1_SR1, I2C_SR1_ADDR, true, timeout_ms)) {
    I2C1_CR1 |= I2C_CR1_STOP;
    if ((I2C1_SR1 & I2C_SR1_AF) != 0u) {
      I2C1_SR1 &= ~I2C_SR1_AF;
      return HAL_I2C_ERR_NACK;
    }
    return HAL_I2C_ERR_TIMEOUT;
  }

  /* 4. 清除 ADDR 标志位 (读取 SR1 后读取 SR2) */
  temp_reg = I2C1_SR1;
  temp_reg = I2C1_SR2;
  (void)temp_reg;

  /* 5. 发送目标寄存器地址 */
  I2C1_DR = (uint32_t)reg_addr;
  if (!hal_i2c_wait_mask(&I2C1_SR1, I2C_SR1_TXE, true, timeout_ms)) {
    I2C1_CR1 |= I2C_CR1_STOP;
    return HAL_I2C_ERR_TIMEOUT;
  }

  /* 6. 依次发送数据内容 */
  for (uint16_t i = 0; i < len; ++i) {
    I2C1_DR = (uint32_t)data[i];
    if (!hal_i2c_wait_mask(&I2C1_SR1, I2C_SR1_TXE, true, timeout_ms)) {
      I2C1_CR1 |= I2C_CR1_STOP;
      if ((I2C1_SR1 & I2C_SR1_AF) != 0u) {
        I2C1_SR1 &= ~I2C_SR1_AF;
        return HAL_I2C_ERR_NACK;
      }
      return HAL_I2C_ERR_TIMEOUT;
    }
  }

  /* 7. 等待最后一字节传输完成 (BTF) */
  if (!hal_i2c_wait_mask(&I2C1_SR1, I2C_SR1_BTF, true, timeout_ms)) {
    I2C1_CR1 |= I2C_CR1_STOP;
    return HAL_I2C_ERR_TIMEOUT;
  }

  /* 8. 产生 STOP 信号结束传输 */
  I2C1_CR1 |= I2C_CR1_STOP;
  return HAL_I2C_OK;
}

hal_i2c_status_t hal_i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data,
                              uint16_t len, uint32_t timeout_ms) {
  volatile uint32_t temp_reg;

  if ((data == 0) || (len == 0)) {
    return HAL_I2C_ERR_BUS_ERROR;
  }

  /* 1. 检查总线忙碌状态 */
  if ((I2C1_SR2 & I2C_SR2_BUSY) != 0u) {
    hal_i2c_bus_recovery();
    if ((I2C1_SR2 & I2C_SR2_BUSY) != 0u) {
      return HAL_I2C_ERR_BUSY;
    }
  }

  /* 2. 发送 START 信号 */
  I2C1_CR1 |= I2C_CR1_START;
  if (!hal_i2c_wait_mask(&I2C1_SR1, I2C_SR1_SB, true, timeout_ms)) {
    I2C1_CR1 |= I2C_CR1_STOP;
    return HAL_I2C_ERR_TIMEOUT;
  }

  /* 3. 发送从机地址 + 写方向位 (0) */
  I2C1_DR = (uint32_t)(dev_addr << 1u);
  if (!hal_i2c_wait_mask(&I2C1_SR1, I2C_SR1_ADDR, true, timeout_ms)) {
    I2C1_CR1 |= I2C_CR1_STOP;
    if ((I2C1_SR1 & I2C_SR1_AF) != 0u) {
      I2C1_SR1 &= ~I2C_SR1_AF;
      return HAL_I2C_ERR_NACK;
    }
    return HAL_I2C_ERR_TIMEOUT;
  }

  /* 清除 ADDR */
  temp_reg = I2C1_SR1;
  temp_reg = I2C1_SR2;

  /* 4. 发送需读取的内部寄存器首地址 */
  I2C1_DR = (uint32_t)reg_addr;
  if (!hal_i2c_wait_mask(&I2C1_SR1, I2C_SR1_TXE, true, timeout_ms)) {
    I2C1_CR1 |= I2C_CR1_STOP;
    return HAL_I2C_ERR_TIMEOUT;
  }

  /* 5. 发送 Repeated START (重起始) 信号转入读取方向 */
  I2C1_CR1 |= I2C_CR1_START;
  if (!hal_i2c_wait_mask(&I2C1_SR1, I2C_SR1_SB, true, timeout_ms)) {
    I2C1_CR1 |= I2C_CR1_STOP;
    return HAL_I2C_ERR_TIMEOUT;
  }

  /* 6. 发送从机地址 + 读方向位 (1) */
  I2C1_DR = (uint32_t)((dev_addr << 1u) | 1u);
  if (!hal_i2c_wait_mask(&I2C1_SR1, I2C_SR1_ADDR, true, timeout_ms)) {
    I2C1_CR1 |= I2C_CR1_STOP;
    return HAL_I2C_ERR_TIMEOUT;
  }

  /* 7. 根据传输长度区分读取模式 */
  if (len == 1u) {
    /* 单字节读取：在清除 ADDR 前关闭 ACK，并在清除后立即置位 STOP */
    I2C1_CR1 &= ~I2C_CR1_ACK;

    temp_reg = I2C1_SR1;
    temp_reg = I2C1_SR2;
    (void)temp_reg;

    I2C1_CR1 |= I2C_CR1_STOP;

    if (!hal_i2c_wait_mask(&I2C1_SR1, I2C_SR1_RXNE, true, timeout_ms)) {
      I2C1_CR1 |= I2C_CR1_ACK;
      return HAL_I2C_ERR_TIMEOUT;
    }
    data[0] = (uint8_t)I2C1_DR;
  } else {
    /* 多字节读取 */
    I2C1_CR1 |= I2C_CR1_ACK;

    temp_reg = I2C1_SR1;
    temp_reg = I2C1_SR2;
    (void)temp_reg;

    for (uint16_t i = 0; i < len; ++i) {
      if (i == (len - 1u)) {
        /* 最后一字节：关闭 ACK 并发出 STOP 信号 */
        I2C1_CR1 &= ~I2C_CR1_ACK;
        I2C1_CR1 |= I2C_CR1_STOP;
      }

      if (!hal_i2c_wait_mask(&I2C1_SR1, I2C_SR1_RXNE, true, timeout_ms)) {
        I2C1_CR1 |= I2C_CR1_ACK;
        return HAL_I2C_ERR_TIMEOUT;
      }
      data[i] = (uint8_t)I2C1_DR;
    }
  }

  /* 重新使能 ACK 以备将来使用 */
  I2C1_CR1 |= I2C_CR1_ACK;
  return HAL_I2C_OK;
}
