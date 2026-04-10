#include "drivers/i2c/i2c_probe.h"

#define RCC_BASE_ADDR 0x40021000u
#define GPIOB_BASE_ADDR 0x40010C00u
#define I2C1_BASE_ADDR 0x40005400u

#define RCC_APB2ENR_OFFSET 0x18u
#define RCC_APB1ENR_OFFSET 0x1Cu
#define GPIO_CRL_OFFSET 0x00u

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
#define I2C_SCL_CRL_SHIFT (I2C_SCL_PIN_INDEX * 4u)
#define I2C_SDA_CRL_SHIFT (I2C_SDA_PIN_INDEX * 4u)
#define I2C_SCL_CRL_MASK (0xFu << I2C_SCL_CRL_SHIFT)
#define I2C_SDA_CRL_MASK (0xFu << I2C_SDA_CRL_SHIFT)
#define GPIO_AF_OD_50MHZ(shift) (0xFu << (shift))

#define I2C_CR1_PE (1u << 0)
#define I2C_CR1_START (1u << 8)
#define I2C_CR1_STOP (1u << 9)
#define I2C_CR1_SWRST (1u << 15)

#define I2C_SR1_SB (1u << 0)
#define I2C_SR1_ADDR (1u << 1)
#define I2C_SR1_AF (1u << 10)

#define I2C_SR2_BUSY (1u << 1)

#define I2C_TIMEOUT 100000u
#define I2C_APB1_CLOCK_MHZ 8u
#define I2C_STANDARD_MODE_CCR 40u
#define I2C_STANDARD_MODE_TRISE 9u
#define I2C_OWN_ADDRESS ((0x33u << 1) | (1u << 14))

static bool i2c_probe_wait_for_mask(volatile uint32_t *reg, uint32_t mask, bool set)
{
    uint32_t timeout = I2C_TIMEOUT;

    while (timeout-- > 0u)
    {
        if (set)
        {
            if ((*reg & mask) == mask)
            {
                return true;
            }
        }
        else if ((*reg & mask) == 0u)
        {
            return true;
        }
    }

    return false;
}

static void i2c_probe_generate_stop(void)
{
    I2C1_CR1 |= I2C_CR1_STOP;
}

static void i2c_probe_clear_ack_failure(void)
{
    I2C1_SR1 &= ~I2C_SR1_AF;
}

static bool i2c_probe_address(uint8_t address)
{
    volatile uint32_t sr1;
    volatile uint32_t sr2;
    uint32_t timeout = I2C_TIMEOUT;

    if (!i2c_probe_wait_for_mask(&I2C1_SR2, I2C_SR2_BUSY, false))
    {
        return false;
    }

    I2C1_CR1 |= I2C_CR1_START;
    if (!i2c_probe_wait_for_mask(&I2C1_SR1, I2C_SR1_SB, true))
    {
        i2c_probe_generate_stop();
        return false;
    }

    I2C1_DR = ((uint32_t)address << 1u);

    while (timeout-- > 0u)
    {
        sr1 = I2C1_SR1;

        if ((sr1 & I2C_SR1_ADDR) != 0u)
        {
            sr2 = I2C1_SR2;
            (void)sr2;
            i2c_probe_generate_stop();
            return true;
        }

        if ((sr1 & I2C_SR1_AF) != 0u)
        {
            i2c_probe_generate_stop();
            i2c_probe_clear_ack_failure();
            return false;
        }
    }

    i2c_probe_generate_stop();
    i2c_probe_clear_ack_failure();
    return false;
}

bool i2c_probe_init(void)
{
    RCC_APB2ENR |= RCC_APB2ENR_AFIOEN | RCC_APB2ENR_IOPBEN;
    RCC_APB1ENR |= RCC_APB1ENR_I2C1EN;

    GPIOB_CRL =
        (GPIOB_CRL & ~(I2C_SCL_CRL_MASK | I2C_SDA_CRL_MASK)) |
        GPIO_AF_OD_50MHZ(I2C_SCL_CRL_SHIFT) |
        GPIO_AF_OD_50MHZ(I2C_SDA_CRL_SHIFT);

    I2C1_CR1 = I2C_CR1_SWRST;
    I2C1_CR1 = 0u;

    I2C1_CR2 = I2C_APB1_CLOCK_MHZ;
    I2C1_OAR1 = I2C_OWN_ADDRESS;
    I2C1_CCR = I2C_STANDARD_MODE_CCR;
    I2C1_TRISE = I2C_STANDARD_MODE_TRISE;
    I2C1_CR1 = I2C_CR1_PE;

    return i2c_probe_wait_for_mask(&I2C1_SR2, I2C_SR2_BUSY, false);
}

int i2c_probe_scan(uint8_t *first_found_address)
{
    int found_count = 0;

    if (first_found_address != 0)
    {
        *first_found_address = 0u;
    }

    for (uint8_t address = 0x08u; address <= 0x77u; ++address)
    {
        if (!i2c_probe_address(address))
        {
            continue;
        }

        if ((found_count == 0) && (first_found_address != 0))
        {
            *first_found_address = address;
        }

        ++found_count;
    }

    return found_count;
}
