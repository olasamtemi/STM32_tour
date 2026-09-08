#include "stm32f1xx_hal.h"
#include "display.h"

/* 7-segment display pins */
#define SEG_A   GPIO_PIN_0
#define SEG_B   GPIO_PIN_1
#define SEG_C   GPIO_PIN_2
#define SEG_D   GPIO_PIN_3
#define SEG_E   GPIO_PIN_4
#define SEG_F   GPIO_PIN_5
#define SEG_G   GPIO_PIN_6
#define SEG_DP  GPIO_PIN_7
#define DISPLAY_PORT GPIOA

/* Digit select pins */
#define DIG_1   GPIO_PIN_5
#define DIG_2   GPIO_PIN_6
#define DIG_3   GPIO_PIN_7
#define DIG_4   GPIO_PIN_8
#define DIG_PORT GPIOB

static const uint8_t digit_pattern[10] =
{
    ~0x3F, // 0
    ~0x06, // 1
    ~0x5B, // 2
    ~0x4F, // 3
    ~0x66, // 4
    ~0x6D, // 5
    ~0x7D, // 6
    ~0x07, // 7
    ~0x7F, // 8
    ~0x6F  // 9
};

static const uint16_t digit_pin[4] =
{
    DIG_1,
    DIG_2,
    DIG_3,
    DIG_4
};

static const uint16_t digit_mask =
    DIG_1 | DIG_2 | DIG_3 | DIG_4;

static volatile uint8_t digits[4];
static uint8_t current_digit = 0;


void Display_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* Segment pins */
    GPIO_InitStruct.Pin = SEG_A | SEG_B | SEG_C | SEG_D |
                          SEG_E | SEG_F | SEG_G | SEG_DP;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DISPLAY_PORT, &GPIO_InitStruct);

    /* Digit select pins */
    GPIO_InitStruct.Pin = DIG_1 | DIG_2 | DIG_3 | DIG_4;
    HAL_GPIO_Init(DIG_PORT, &GPIO_InitStruct);

    /* Start with display disabled */
    DIG_PORT->ODR &= ~digit_mask;
}


void Display_SetNumber(uint16_t number)
{
    digits[0] = number / 1000;
    digits[1] = (number / 100) % 10;
    digits[2] = (number / 10) % 10;
    digits[3] = number % 10;
}


void Display_Refresh(void)
{
    /* Turn all digits off before changing segments */
    DIG_PORT->ODR &= ~digit_mask;

    /* Put the desired segment pattern on the bus */
    DISPLAY_PORT->ODR = digit_pattern[digits[current_digit]];

    /* Enable the current digit */
    DIG_PORT->ODR |= digit_pin[current_digit];

    /* Move to next digit for next refresh */
    current_digit = (current_digit + 1) % 4;
}