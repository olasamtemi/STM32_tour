#include "stm32f1xx_hal.h"

// Hardware Handle
TIM_HandleTypeDef htim2;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_PWM_Init(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM2_PWM_Init(); // Sets up PA1 as a direct hardware PWM engine

    // Start the PWM generation on Timer 2, Channel 2 physically!
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);

    int brightness = 0;
    int fadeAmount = 15; // How fast the LED fades

    while (1)
    {
        // Dynamically update the CCR (Pulse) register to change brightness
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, brightness);

        // Change the brightness for the next loop
        brightness += fadeAmount;

        // Reverse the direction of the fading at the ends of the fade
        if (brightness <= 0 || brightness >= 999) {
            fadeAmount = -fadeAmount;
        }

        HAL_Delay(30); // Smooth pacing delay for the human eye
    }
}

static void MX_TIM2_PWM_Init(void)
{
    // 1. Enable Clocks for both Timer 2 and GPIOA
    __HAL_RCC_TIM2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // 2. Configure PA1 as Alternate Function Push-Pull
    // This detaches it from standard GPIO registers and chains it directly to TIM2
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP; // ALTERNATE FUNCTION!
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 3. Configure the Timer Base Frequency
    // Target Frequency = 72,000,000 / (71 + 1) * (999 + 1) = 1,000 Hz (1 kHz)
    // A 1kHz frequency is fast enough that the human eye cannot see it blinking!
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 71;            // Timer ticks at 1MHz        
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 999;              // Max count limit (ARR)
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_PWM_Init(&htim2);

    // 4. Configure the PWM Channel Parameters
    TIM_OC_InitTypeDef sConfigOC = {0};
    sConfigOC.OCMode = TIM_OCMODE_PWM1;           // Standard PWM mode 1
    sConfigOC.Pulse = 0;                          // Start with brightness at 0 (CCR initial state)
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    
    HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2); // Target Channel 2
}

static void MX_GPIO_Init(void)
{
    // Keeping my boilerplate layout standard
    __HAL_RCC_GPIOB_CLK_ENABLE();
}

void SysTick_Handler(void)
{
    HAL_IncTick(); // Kept active for the main loop fading delay
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}