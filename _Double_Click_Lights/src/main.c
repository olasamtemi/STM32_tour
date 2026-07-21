#include "stm32f1xx_hal.h"

#define LED_PIN GPIO_PIN_2
#define LED_PORT GPIOB
#define BUTTON_PIN GPIO_PIN_4
#define BUTTON_PORT GPIOA

void SystemClock_Config(void);
static void MX_GPIO_Init(void);


int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    
    while (1)
    {
        __WFI(); //Wait for interrupt
    }
}

static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // Configure LED_PIN as output (push-pull) for the on-board LED
    GPIO_InitStruct.Pin = LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = BUTTON_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(BUTTON_PORT, &GPIO_InitStruct);
    
    HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI4_IRQn);
    
}

void EXTI4_IRQHandler(){
    HAL_GPIO_EXTI_IRQHandler(BUTTON_PIN);
}

#define DEBOUNCE_TIME_MS 50
#define DOUBLE_CLICK_WINDOW 300U

volatile uint32_t last_press_time = 0;
volatile uint8_t first_click_seen = 0;


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == BUTTON_PIN)
    {
        uint32_t interrupt_time = HAL_GetTick();
        volatile uint32_t time_difference = interrupt_time - last_press_time;

        if (time_difference < DEBOUNCE_TIME_MS){
            return;
        }

        if (first_click_seen && time_difference <= DOUBLE_CLICK_WINDOW){
            HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
            first_click_seen = 0;
        } else {
            first_click_seen = 1;
        }

        last_press_time = interrupt_time;

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
