#include "stm32f1xx_hal.h" 

UART_HandleTypeDef huart1;

void UART1_Init(void) {
    HAL_Init();
    
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // Configure TX Pin (PA9) and RX Pin (PA10)
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP; // Alternate Function Push-Pull for UART
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Configure UART Settings
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    HAL_UART_Init(&huart1);
}

int main(void) {
    HAL_Init();
    UART1_Init();

    char msg[] = "Hello from STM32!\r\n";

    while (1) {
        // Send data over UART1 (Timeout = 100ms)
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, sizeof(msg)-1, 100);
        HAL_Delay(1000); // Wait 1 second
    }
}