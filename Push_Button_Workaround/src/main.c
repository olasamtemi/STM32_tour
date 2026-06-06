#include "stm32f1xx_hal.h" 

int main(void)
{
    HAL_Init();
    
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_PinState Read_Button;

    while(1){      
        Read_Button = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
        
        if (Read_Button == GPIO_PIN_SET)
        {
            HAL_Delay(100);
            
            Read_Button = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
            if (Read_Button == GPIO_PIN_SET)
            {
                HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_2);
            }
            // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);
        }
        // else{
        //     HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);
        // }
    }

    return 0;
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}