#include "stm32f1xx_hal.h"
#include "display.h"

#define ADC_PIN GPIO_PIN_0
#define ADC_PORT GPIOB

TIM_HandleTypeDef htim2;
ADC_HandleTypeDef hadc1;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_ADC_Init(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;

    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
    MX_GPIO_Init();
    MX_ADC_Init();
    HAL_ADCEx_Calibration_Start(&hadc1);
    MX_TIM2_Init();
    Display_Init();
    
    HAL_TIM_Base_Start_IT(&htim2);

    while (1)
    {
        HAL_ADC_Start(&hadc1);

        if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
        {
            uint32_t adc_value = HAL_ADC_GetValue(&hadc1);
            Display_SetNumber(adc_value);
        }

        HAL_ADC_Stop(&hadc1);
    }
}

static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Pin = ADC_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(ADC_PORT, &GPIO_InitStruct);

}

static void MX_ADC_Init(void){
    ADC_ChannelConfTypeDef channel_config = {0};

    __HAL_RCC_ADC1_CLK_ENABLE();
    
    hadc1.Instance = ADC1;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;

    HAL_ADC_Init(&hadc1);

    channel_config.Channel = ADC_CHANNEL_8;
    channel_config.Rank = ADC_REGULAR_RANK_1;
    channel_config.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;    
    HAL_ADC_ConfigChannel(&hadc1, &channel_config);
    
}


static void MX_TIM2_Init(void){
    __HAL_RCC_TIM2_CLK_ENABLE();

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 72 - 1;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 1000 - 1;
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
        Display_Refresh();
    }
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
