#include "stm32f1xx_hal.h"
#include <stdbool.h>

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
static void set_number_digits(uint16_t number);
static void display_digit(uint8_t posiiton, uint8_t digit);
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
    
    HAL_TIM_Base_Start_IT(&htim2);
    
    uint32_t time_elapsed = HAL_GetTick();
    const uint16_t interval_ms = 1000;

    set_number_digits(set_time);
    
    while (1)
    {
        uint32_t now = HAL_GetTick();

        if (now - time_elapsed >= interval_ms && is_running)
        {

            if (time_left > 0)
            {
                time_left--;
                set_number_digits(time_left);
                
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

uint8_t digit_pattern[] =
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

const uint16_t digit_pin[] = {
    DIG_1,
    DIG_2,
    DIG_3,
    DIG_4
};

const uint16_t mask = DIG_1 | DIG_2 | DIG_3 | DIG_4;

static void display_digit(uint8_t position, uint8_t digit)
{

    if(position > 3 || digit > 9){
        return;
    }
    DIG_PORT->ODR &= ~(mask);
    DISPLAY_PORT->ODR = digit_pattern[digit];
    DIG_PORT->ODR |= digit_pin[position];

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

static volatile uint8_t digits[4];

static void set_number_digits(uint16_t number){
    
    digits[0] = number/1000;
    digits[1] = (number/100) % 10;
    digits[2] = (number/10) % 10;
    digits[3] = number % 10;
    
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

static uint8_t current_digit = 0;
// The user-facing callback that runs right after the flag is cleared
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        display_digit(current_digit, digits[current_digit]);
        current_digit = (current_digit + 1) % 4;
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
                set_number_digits(set_time);
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
                set_number_digits(set_time);
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
                set_number_digits(set_time); 
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
    
    /* Configure segment pins */
    GPIO_InitStruct.Pin = SEG_A | SEG_B | SEG_C | SEG_D |
                            SEG_E | SEG_F | SEG_G | SEG_DP;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DISPLAY_PORT, &GPIO_InitStruct);
    
    /* Configure digit select pins */
    GPIO_InitStruct.Pin = DIG_1 | DIG_2 | DIG_3 | DIG_4;
    HAL_GPIO_Init(DIG_PORT, &GPIO_InitStruct);
    
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