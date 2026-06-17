#include "stm32f1xx_hal.h"

void SystemClock_Config(void);
static void MX_GPIO_Init(void);

#define LED_PIN GPIO_PIN_2
#define LED_PORT GPIOB
#define BUTTON_PIN GPIO_PIN_4
#define BUTTON_PORT GPIOA

// Mark variables shared between ISR and main loop as volatile
volatile uint32_t blink_delay = 500;
volatile uint32_t last_button_press = 0;

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();

  uint32_t now;
  uint32_t next_blink = 500;

  while (1)
  {
      now = HAL_GetTick();

      if (now >= next_blink) {
          HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
          next_blink = now + blink_delay;
      }

      __WFI(); // Wakes up every 1ms via SysTick or on EXTI button press
  }
}

static void MX_GPIO_Init(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  
  // Configure PB2 as output (push-pull) for the LED
  GPIO_InitStruct.Pin = LED_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);
  
  // Configure PA4 as input with EXTI Rising Edge Trigger (for NC external pull-up configuration)
  GPIO_InitStruct.Pin = BUTTON_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING; 
  GPIO_InitStruct.Pull = GPIO_NOPULL; // External resistor handles the pull-up physics
  HAL_GPIO_Init(BUTTON_PORT, &GPIO_InitStruct);

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
    if (interrupt_time - last_button_press > 250) {
      
      // Toggle the speed state
      if (blink_delay == 500) {
        blink_delay = 50;
      } else {
        blink_delay = 500;
      }
      
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