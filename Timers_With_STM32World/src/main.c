#include "stm32f1xx_hal.h"

#define LED_PIN GPIO_PIN_2
#define LED_PORT GPIOB

TIM_HandleTypeDef htim2;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM2_Init(); // Initialize our Timer 2 settings

    // Pull the trigger! Start counting with interrupts enabled
    HAL_TIM_Base_Start_IT(&htim2);

    while (1)
    {
        // Intentionally empty! LED will toggle seamlessly in the background
    }
}

static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Configure LED_PIN as output (push-pull) for the on-board LED
    GPIO_InitStruct.Pin = LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);

}

void SysTick_Handler(void)
{
    HAL_IncTick();
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
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

static void MX_TIM2_Init(void)
{
    // 1. Enable the power/clock signal to the TIM2 peripheral
    __HAL_RCC_TIM2_CLK_ENABLE();

    // 2. Map out our 1-second overflow math settings
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 7199;                  // Divide 72MHz by 7200 -> Ticks at 10kHz
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;  // Count upwards from 0
    htim2.Init.Period = 4999;                     // Overflow at 5,000 ticks -> 1/2 second interval
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    
    HAL_TIM_Base_Init(&htim2);

    // 3. Open the NVIC core gateway so the timer can interrupt the CPU
    HAL_NVIC_SetPriority(TIM2_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

// The physical CPU core jumps here when the timer hardware overflows
void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim2); // Clears the interrupt flag safely behind the scenes
}

// The user-facing callback that runs right after the flag is cleared
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        // Toggle the LED on the board!
        HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
    } 
}
