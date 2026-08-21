#include "stm32f1xx_hal.h"
#include <stdbool.h>
#include "display.h"

/* Button Pins */
#define INCR_BUTTON GPIO_PIN_1
#define DECR_BUTTON GPIO_PIN_0
#define START_PAUSE_BUTTON GPIO_PIN_9
#define BUTTON_PORT GPIOB

/* Buzzer Pin */
#define BUZZER GPIO_PIN_12
#define BUZZ_PORT GPIOB
#define DEBOUNCE_MS 200

TIM_HandleTypeDef htim2;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void time_up();

volatile uint16_t set_time;
volatile uint16_t time_left;
volatile static bool is_running = false;

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM2_Init();
    Display_Init();

    
    HAL_TIM_Base_Start_IT(&htim2);
    
    uint32_t time_elapsed = HAL_GetTick();
    const uint16_t interval_ms = 1000;

    Display_SetNumber(set_time);
    
    while (1)
    {
        uint32_t now = HAL_GetTick();

        if (now - time_elapsed >= interval_ms && is_running)
        {

            if (time_left > 0)
            {
                time_left--;
                Display_SetNumber(time_left);
                
                if (time_left == 0)
                {
                    time_up();
                }
                time_elapsed = now;
            }
            else 
            {
                is_running = false;
            }
        }

        __WFI(); // Wait For Interrupt
    }
}

static void time_up()
{
    uint32_t timer_elapse_time = HAL_GetTick(), buzzing;
    uint16_t buzz_duration_ms = 3000;

    is_running = false;                    
    for (buzzing = HAL_GetTick(); timer_elapse_time - buzzing <= buzz_duration_ms; )
    {
        HAL_GPIO_WritePin(BUZZ_PORT, BUZZER, 1);
        timer_elapse_time = HAL_GetTick();
    }

    HAL_GPIO_WritePin(BUZZ_PORT, BUZZER, GPIO_PIN_RESET);


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


void EXTI0_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(DECR_BUTTON);
}

void EXTI1_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(INCR_BUTTON);
}

void EXTI9_5_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(START_PAUSE_BUTTON);
}

volatile uint32_t last_incr_press = 0;
volatile uint32_t last_decr_press = 0;
volatile uint32_t last_start_press = 0;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    uint32_t interrupt_time = HAL_GetTick();

    if (GPIO_Pin == INCR_BUTTON)
    {
        if (interrupt_time - last_incr_press > DEBOUNCE_MS)
        {
            if (!is_running){
                set_time++;
                Display_SetNumber(set_time);
                time_left = set_time + 1;
                last_incr_press = interrupt_time;
            }
        }
    }
    else if (GPIO_Pin == DECR_BUTTON)
    {
        if (interrupt_time - last_decr_press > DEBOUNCE_MS)
        {
            if (set_time != 0 && !is_running){
                set_time--;
                Display_SetNumber(set_time);
                time_left = set_time + 1;
            }
            last_decr_press = interrupt_time;
        }
    }
    else if (GPIO_Pin == START_PAUSE_BUTTON)
    {
        if (interrupt_time - last_start_press > DEBOUNCE_MS)
        {
            if (time_left == 0){
                // RESET - ISH only when timer is elapsed (don't have room for extra push-button)
                Display_SetNumber(set_time); 
                time_left = set_time + 1;
                return;
            }
            if (!is_running)              
                is_running = true;
            else
                is_running = false;
            last_start_press = interrupt_time;
        }
    }
}

static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Pin = INCR_BUTTON | DECR_BUTTON | START_PAUSE_BUTTON;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(EXTI0_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);

    HAL_NVIC_SetPriority(EXTI1_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);

    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

    GPIO_InitStruct.Pin = BUZZER;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(BUZZ_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(BUZZ_PORT, BUZZER, GPIO_PIN_RESET);
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

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 |
                                  RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}