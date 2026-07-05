#include "stm32f1xx_hal.h"

#define SERVO_PIN GPIO_PIN_2
#define SERVO_PORT GPIOA

TIM_HandleTypeDef htim2;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_PWM_Init(void);
static uint16_t ServoPos(uint8_t POSITION);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM2_PWM_Init();
    
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
    uint32_t now = HAL_GetTick(), time_elapsed = 0;
    while (1)
    {
        for (uint8_t position = 0; position < 180; position += 5){
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, ServoPos(position));
            HAL_Delay(1000);
        }
        for (uint8_t position = 180; position > 0; position -= 5){
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, ServoPos(position));
            HAL_Delay(1000);
        }

        if (now - time_elapsed <= 500){

        }
    }
}

static uint16_t ServoPos(uint8_t POSITION)
{
    if (POSITION > 180) POSITION = 180;

    return ((uint32_t)POSITION * 2000 / 180) + 500;
}

static void MX_TIM2_PWM_Init(void){
    __HAL_RCC_TIM2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = SERVO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(SERVO_PORT, &GPIO_InitStruct);

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 72 - 1;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 20000 - 1;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_PWM_Init(&htim2);

    TIM_OC_InitTypeDef sConfigOC = {0};

    sConfigOC.OCMode = TIM_OCMODE_PWM1;           // Set standard PWM mode 1
    sConfigOC.Pulse = 500;                        // CCR Value: Sets the servo to starting position (0 degrees)
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;   // Pin goes HIGH starting from 0, turns LOW at CCR match

    HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3); // Map settings to Channel 1
}

static void MX_GPIO_Init(void){}

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
