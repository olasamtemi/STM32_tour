#include "stm32f1xx_hal.h"

#define MOTOR_PWM GPIO_PIN_1
#define MOTOR_PORT GPIOA
#define BUTTON_PIN GPIO_PIN_4
#define BUTTON_PORT GPIOA
#define MOTOR_IN1 GPIO_PIN_7
#define MOTOR_IN2 GPIO_PIN_6
#define MOTOR_IN1_PORT GPIOA
#define MOTOR_IN2_PORT GPIOA
#define DEBOUNCE_MS 250

TIM_HandleTypeDef htim2;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_PWM_Init(void);

volatile uint32_t last_button_press = 0;
volatile uint8_t fan_speed_step = 0;

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM2_PWM_Init(); 
    
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);

    
    while (1)
    {
      HAL_GPIO_WritePin(MOTOR_IN1_PORT, MOTOR_IN1, 1);
      HAL_GPIO_WritePin(MOTOR_IN1_PORT, MOTOR_IN2, 0);

        switch (fan_speed_step)
        {
        case 1:
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 300);
            break;
            
        case 2:
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 700);
        break;
        
        case 3:
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 999);
        break;
        
        default:
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);
            break;
        }

       __WFI(); 
    }
}

static void MX_TIM2_PWM_Init(void){
    __HAL_RCC_TIM2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    // Configure MOTOR_PWM as output for the on-board LED
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = MOTOR_PWM;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(MOTOR_PORT, &GPIO_InitStruct);
    
    // 3. Configure the Timer Base Frequency
    // Target Frequency = 72,000,000 / (71 + 1) * (49 + 1) = 20,000 Hz (20 kHz)
    // A 1kHz frequency is fast enough that the human eye cannot see it blinking!
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 71;                    
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    // htim2.Init.Period = 50 -1; //For 20KHz frequency
    htim2.Init.Period = 1000 - 1;     //For 1kHz frequency  // Max count limit (ARR)
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
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = BUTTON_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    HAL_GPIO_Init(BUTTON_PORT, &GPIO_InitStruct);
    // No pullup resistor because I included an external 10K resistor
    
    GPIO_InitStruct.Pin = MOTOR_IN1;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(MOTOR_IN1_PORT, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = MOTOR_IN2;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(MOTOR_IN2_PORT, &GPIO_InitStruct);

    // EXTI4 handles pin 4 on any GPIO port
    HAL_NVIC_SetPriority(EXTI4_IRQn, 2, 0); 
    HAL_NVIC_EnableIRQ(EXTI4_IRQn); 
}

void EXTI4_IRQHandler(void){
  HAL_GPIO_EXTI_IRQHandler(BUTTON_PIN);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
  if (GPIO_Pin == BUTTON_PIN){
    uint32_t interrupt_time = HAL_GetTick();
    
    // Software Debounce: Ignore any edge transitions within 200ms of a valid press
    if (interrupt_time - last_button_press > DEBOUNCE_MS) {
      
      // Toggle the fan speed
        fan_speed_step++;
        if(fan_speed_step > 3)
            fan_speed_step = 0;
        
      last_button_press = interrupt_time;
    }
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
