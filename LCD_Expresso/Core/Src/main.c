/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/


/* USER CODE BEGIN PV */


typedef enum
{
    SC_WELCOME,
    SC_INTRO,
    SC_SECRET,
    SC_PLAY_Q,
    SC_PLAY_BIG,
    SC_PLAY_SMALL,
    SC_FRIEND_Q,
    SC_FRIEND_A,
    SC_FRIEND_B,
    SC_FEEL_Q,
    SC_FEEL_A,
    SC_FEEL_B,
    SC_PROV_Q,
    SC_PROV_A,
    SC_PROV_B,
    SC_LOVE_Q,
    SC_LOVE_A,
    SC_LOVE_B,
    SC_GROW_Q,
    SC_GROW_A,
    SC_GROW_B,
    SC_LETTER_1,
    SC_LETTER_2,
    SC_LETTER_3,
    SC_ENDING,
    SC_COUNT
} AppScreen;

typedef struct
{
    const char *line1;
    const char *line2;
    AppScreen on_start;
    AppScreen on_big;
    AppScreen on_small;
} StoryScene;

static AppScreen current_screen = SC_WELCOME;
static int16_t rendered_screen = -1;


/* USER CODE END PV */


/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);

/* USER CODE BEGIN PFP */

static void App_Init(void);
static void App_Update(void);

/* USER CODE END PFP */


/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


/* ================= LCD 1602 DRIVER =================
   Interface : 4-bit
   RS        : Register_Select_Pin
   E         : Enable_Pin
   D4-D7     : D4_Pin to D7_Pin
   =================================================== */

static void LCD_PulseEnable(void)
{
    HAL_GPIO_WritePin(GPIOB, Enable_Pin, GPIO_PIN_SET);
    HAL_Delay(1);

    HAL_GPIO_WritePin(GPIOB, Enable_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);
}

static void LCD_WriteNibble(uint8_t nibble)
{
    HAL_GPIO_WritePin(GPIOB, D4_Pin,
        (nibble & 0x01U) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(GPIOB, D5_Pin,
        (nibble & 0x02U) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(GPIOB, D6_Pin,
        (nibble & 0x04U) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(GPIOB, D7_Pin,
        (nibble & 0x08U) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    LCD_PulseEnable();
}

static void LCD_WriteByte(uint8_t value, uint8_t is_data)
{
    HAL_GPIO_WritePin(GPIOB, Register_Select_Pin,
        is_data ? GPIO_PIN_SET : GPIO_PIN_RESET);

    LCD_WriteNibble((value >> 4U) & 0x0FU);
    LCD_WriteNibble(value & 0x0FU);
}

static void LCD_Command(uint8_t command)
{
    LCD_WriteByte(command, 0U);

    if (command == 0x01U || command == 0x02U)
    {
        HAL_Delay(2);
    }
}

static void LCD_Data(uint8_t data)
{
    LCD_WriteByte(data, 1U);
}

static void LCD_Init(void)
{
    HAL_Delay(50);

    HAL_GPIO_WritePin(GPIOB, Enable_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, Register_Select_Pin, GPIO_PIN_RESET);

    /* HD44780 initialisation sequence */
    LCD_WriteNibble(0x03U);
    HAL_Delay(5);

    LCD_WriteNibble(0x03U);
    HAL_Delay(2);

    LCD_WriteNibble(0x03U);
    HAL_Delay(2);

    LCD_WriteNibble(0x02U);

    /* 4-bit mode, 2 lines, 5x8 font */
    LCD_Command(0x28U);

    /* Display ON, cursor OFF, blink OFF */
    LCD_Command(0x0CU);

    /* Cursor increments, display does not shift */
    LCD_Command(0x06U);

    /* Clear display */
    LCD_Command(0x01U);
}

static void LCD_SetCursor(uint8_t column, uint8_t row)
{
    uint8_t address = (row == 0U) ? 0x00U : 0x40U;

    if (column > 15U)
        column = 15U;

    LCD_Command(0x80U | (address + column));
}

static void LCD_Print(const char *text)
{
    while (*text != '\0')
    {
        LCD_Data((uint8_t)*text++);
    }
}

static void LCD_PrintLine(uint8_t row, const char *text)
{
    uint8_t count = 0U;

    LCD_SetCursor(0U, row);

    while (*text != '\0' && count < 16U)
    {
        LCD_Data((uint8_t)*text++);
        count++;
    }

    while (count < 16U)
    {
        LCD_Data(' ');
        count++;
    }
}

static void LCD_ShowSlide(const char *title,
                          const char *subtitle,
                          uint32_t duration_ms)
{
    LCD_PrintLine(0U, title);
    LCD_PrintLine(1U, subtitle);

    HAL_Delay(duration_ms);
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();

/* USER CODE BEGIN 2 */

LCD_Init();
App_Init();

/* USER CODE END 2 */


  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    App_Update();
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, Enable_Pin|Register_Select_Pin|D4_Pin|D5_Pin
                          |D6_Pin|D7_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : START_Pin BIG_BUTTON_Pin SMALL_BUTTON_Pin */
  GPIO_InitStruct.Pin = START_Pin|BIG_BUTTON_Pin|SMALL_BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : Enable_Pin Register_Select_Pin D4_Pin D5_Pin
                           D6_Pin D7_Pin */
  GPIO_InitStruct.Pin = Enable_Pin|Register_Select_Pin|D4_Pin|D5_Pin
                          |D6_Pin|D7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}


/* USER CODE BEGIN 4 */

typedef struct
{
    GPIO_PinState raw_state;
    GPIO_PinState stable_state;
    uint32_t last_change;
} ButtonState;

static ButtonState start_btn;
static ButtonState big_btn;
static ButtonState small_btn;

#define BUTTON_DEBOUNCE_MS 25U

static void Button_InitState(ButtonState *button)
{
    button->raw_state = GPIO_PIN_SET;
    button->stable_state = GPIO_PIN_SET;
    button->last_change = HAL_GetTick();
}

/* Returns 1 once for each debounced press. */
static uint8_t Button_Pressed(GPIO_TypeDef *port,
                             uint16_t pin,
                             ButtonState *button)
{
    GPIO_PinState reading = HAL_GPIO_ReadPin(port, pin);
    uint32_t now = HAL_GetTick();

    if (reading != button->raw_state)
    {
        button->raw_state = reading;
        button->last_change = now;
    }

    if ((now - button->last_change) >= BUTTON_DEBOUNCE_MS)
    {
        if (button->stable_state != button->raw_state)
        {
            button->stable_state = button->raw_state;

            if (button->stable_state == GPIO_PIN_RESET)
            {
                return 1U;
            }
        }
    }

    return 0U;
}

static void App_Init(void)
{
    Button_InitState(&start_btn);
    Button_InitState(&big_btn);
    Button_InitState(&small_btn);

    current_screen = SC_WELCOME;
    rendered_screen = -1;
}


/* Story scenes: each button can lead somewhere different. */

#define STAY SC_COUNT

static const StoryScene story[SC_COUNT] =
{
    [SC_WELCOME] = {
        "FOR MY AJIKE <3", "START OUR STORY",
        SC_INTRO, STAY, STAY
    },

    [SC_INTRO] = {
        "5 MONTHS, BABE", "BIG=SECRET S=GO",
        SC_PLAY_Q, SC_SECRET, SC_PLAY_Q
    },

    [SC_SECRET] = {
        "YOU FOUND IT!", "I LOVE YOU. YES!",
        SC_PLAY_Q, SC_PLAY_Q, SC_PLAY_Q
    },

    [SC_PLAY_Q] = {
        "WHO IS THE BABY?", "BIG=ME S=YOU",
        SC_FRIEND_Q, SC_PLAY_BIG, SC_PLAY_SMALL
    },

    [SC_PLAY_BIG] = {
        "CORRECT. OBV.!", "YOU KNOWWWWW",
        SC_FRIEND_Q, SC_SECRET, SC_FRIEND_Q
    },

    [SC_PLAY_SMALL] = {
        "NICE TRY, BABE", "I'LL ALLOW IT :)",
        SC_FRIEND_Q, SC_FRIEND_Q, SC_SECRET
    },

    [SC_FRIEND_Q] = {
        "WHERE IT STARTED", "BIG=WHERE S=WHEN",
        SC_FEEL_Q, SC_FRIEND_A, SC_FRIEND_B
    },

    [SC_FRIEND_A] = {
        "KINGDOM SUMMIT", "WHERE IT BEGAN",
        SC_FEEL_Q, SC_FRIEND_B, SC_FEEL_Q
    },

    [SC_FRIEND_B] = {
        "NOT A BIG BANG", "JUST A SPRINKLE",
        SC_FEEL_Q, SC_FRIEND_A, SC_FEEL_Q
    },

    [SC_FEEL_Q] = {
        "THE 7 MONTHS B4?", "BIG=WHY S=WHEN",
        SC_PROV_Q, SC_FEEL_A, SC_FEEL_B
    },

    [SC_FEEL_A] = {
        "WANTED TO KNOW", "YOU FIRST, AJIKE",
        SC_PROV_Q, SC_FEEL_B, SC_PROV_Q
    },

    [SC_FEEL_B] = {
        "ALONG CAME YMR..", "I GOT HELPLESS",
        SC_PROV_Q, SC_FEEL_A, SC_PROV_Q
    },

    [SC_PROV_Q] = {
        "WHO GETS CREDIT?", "BIG=GOD S=GOD",
        SC_LOVE_Q, SC_PROV_A, SC_PROV_B
    },

    [SC_PROV_A] = {
        "GOD, OBVIOUSLY", "& HE'S NOT DONE",
        SC_LOVE_Q, SC_PROV_B, SC_LOVE_Q
    },

    [SC_PROV_B] = {
        "SUMMIT TO NOW,", "IT WAS ALL HIM",
        SC_LOVE_Q, SC_PROV_A, SC_LOVE_Q
    },

    [SC_LOVE_Q] = {
        "WHAT I LOVE...", "BIG=EYES S=HANDS",
        SC_GROW_Q, SC_LOVE_A, SC_LOVE_B
    },

    [SC_LOVE_A] = {
        "CRYSTAL CLEAR", "JUST RIGHT SIZE",
        SC_GROW_Q, SC_LOVE_B, SC_GROW_Q
    },

    [SC_LOVE_B] = {
        "THOSE FINGERS...", "STILL OBSESSED",
        SC_GROW_Q, SC_LOVE_A, SC_GROW_Q
    },

    [SC_GROW_Q] = {
        "NOW, THE BRAIN", "BIG=HSBS S=LOBE",
        SC_LETTER_1, SC_GROW_A, SC_GROW_B
    },

    [SC_GROW_A] = {
        "HOLY SPIRIT", "BACKED SCHOLAR",
        SC_LETTER_1, SC_GROW_B, SC_LETTER_1
    },

    [SC_GROW_B] = {
        "BIG FRONTAL LOBE", "RUNS THE GLOBE",
        SC_LETTER_1, SC_GROW_A, SC_LETTER_1
    },

    [SC_LETTER_1] = {
        "*WINKS*", "YOU EYE-ROLL :)",
        SC_LETTER_2, SC_LETTER_2, SC_LETTER_2
    },

    [SC_LETTER_2] = {
        "STILL HELPLESS", "AND LOVING IT",
        SC_LETTER_3, SC_LETTER_3, SC_LETTER_3
    },

    [SC_LETTER_3] = {
        "SO, BABY GIRL...", "ONE LAST THING:",
        SC_ENDING, SC_ENDING, SC_ENDING
    },

    [SC_ENDING] = {
        "LOVE YOU MI NENA", "NOW COME HUG ME",
        SC_WELCOME, SC_WELCOME, SC_WELCOME
    }
};


static void App_Update(void)
{
    uint8_t start_pressed = Button_Pressed(
        START_GPIO_Port, START_Pin, &start_btn);

    uint8_t big_pressed = Button_Pressed(
        BIG_BUTTON_GPIO_Port, BIG_BUTTON_Pin, &big_btn);

    uint8_t small_pressed = Button_Pressed(
        SMALL_BUTTON_GPIO_Port, SMALL_BUTTON_Pin, &small_btn);

    /* Draw each scene once, on entry. */
    if (rendered_screen != (int16_t)current_screen)
    {
        LCD_PrintLine(0U, story[current_screen].line1);
        LCD_PrintLine(1U, story[current_screen].line2);

        rendered_screen = (int16_t)current_screen;
    }

    /* One press produces at most one scene transition. */
    if (start_pressed)
    {
        if (story[current_screen].on_start != STAY)
            current_screen = story[current_screen].on_start;
    }
    else if (big_pressed)
    {
        if (story[current_screen].on_big != STAY)
            current_screen = story[current_screen].on_big;
    }
    else if (small_pressed)
    {
        if (story[current_screen].on_small != STAY)
            current_screen = story[current_screen].on_small;
    }
}

/* End story application */

/* USER CODE END 4 */


/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
