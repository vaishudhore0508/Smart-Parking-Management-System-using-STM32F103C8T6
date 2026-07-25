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
#include <stdio.h>
#include <string.h>


/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/**************** LCD ****************/

#define RS_PIN GPIO_PIN_0
#define RS_PORT GPIOA

#define EN_PIN GPIO_PIN_1
#define EN_PORT GPIOA

#define D4_PIN GPIO_PIN_2
#define D4_PORT GPIOA

#define D5_PIN GPIO_PIN_3
#define D5_PORT GPIOA

#define D6_PIN GPIO_PIN_4
#define D6_PORT GPIOA

#define D7_PIN GPIO_PIN_5
#define D7_PORT GPIOA

/**************** IR Sensors ****************/

#define ENTRY_IR_PIN GPIO_PIN_6
#define ENTRY_IR_PORT GPIOA

#define EXIT_IR_PIN GPIO_PIN_7
#define EXIT_IR_PORT GPIOA

/**************** Servo ****************/

#define SERVO_CHANNEL TIM_CHANNEL_1

/**************** LEDs ****************/

#define GREEN_LED_PIN GPIO_PIN_12
#define GREEN_LED_PORT GPIOB

#define RED_LED_PIN GPIO_PIN_13
#define RED_LED_PORT GPIOB

/**************** Parking ****************/

#define TOTAL_SLOTS 10


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

TIM_HandleTypeDef htim1;

/* USER CODE BEGIN PV */

uint8_t Car_Count=0;

char lcd_buffer[17];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
/* USER CODE BEGIN PFP */
/******** LCD ********/

void LCD_Enable(void);

void LCD_SendNibble(uint8_t nibble);

void LCD_Command(uint8_t cmd);

void LCD_Data(uint8_t data);

void LCD_Init(void);

void LCD_Clear(void);

void LCD_SetCursor(uint8_t row,uint8_t col);

void LCD_String(char *str);

/******** Parking ********/

void Display_Count(void);

void Check_Entry(void);

void Check_Exit(void);

/******** Servo ********/

void Servo_Open(void);

void Servo_Close(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/************************************************
                LCD Enable
************************************************/

void LCD_Enable(void)
{
    HAL_GPIO_WritePin(EN_PORT,EN_PIN,GPIO_PIN_SET);

    HAL_Delay(1);

    HAL_GPIO_WritePin(EN_PORT,EN_PIN,GPIO_PIN_RESET);

    HAL_Delay(1);
}

/************************************************
                Send Nibble
************************************************/

void LCD_SendNibble(uint8_t nibble)
{
    HAL_GPIO_WritePin(D4_PORT,D4_PIN,(nibble>>0)&1);

    HAL_GPIO_WritePin(D5_PORT,D5_PIN,(nibble>>1)&1);

    HAL_GPIO_WritePin(D6_PORT,D6_PIN,(nibble>>2)&1);

    HAL_GPIO_WritePin(D7_PORT,D7_PIN,(nibble>>3)&1);

    LCD_Enable();
}

/************************************************
                LCD Command
************************************************/

void LCD_Command(uint8_t cmd)
{
    HAL_GPIO_WritePin(RS_PORT,RS_PIN,GPIO_PIN_RESET);

    LCD_SendNibble(cmd>>4);

    LCD_SendNibble(cmd&0x0F);

    HAL_Delay(2);
}

/************************************************
                LCD Data
************************************************/

void LCD_Data(uint8_t data)
{
    HAL_GPIO_WritePin(RS_PORT,RS_PIN,GPIO_PIN_SET);

    LCD_SendNibble(data>>4);

    LCD_SendNibble(data&0x0F);

    HAL_Delay(2);
}
/************************************************
                LCD Initialize
************************************************/
void LCD_Init(void)
{
    HAL_Delay(50);

    HAL_GPIO_WritePin(RS_PORT, RS_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(EN_PORT, EN_PIN, GPIO_PIN_RESET);

    /* LCD Reset Sequence */
    LCD_SendNibble(0x03);
    HAL_Delay(5);

    LCD_SendNibble(0x03);
    HAL_Delay(5);

    LCD_SendNibble(0x03);
    HAL_Delay(2);

    LCD_SendNibble(0x02);
    HAL_Delay(2);

    LCD_Command(0x28);      // 4-bit mode, 2 lines
    LCD_Command(0x0C);      // Display ON
    LCD_Command(0x06);      // Entry mode
    LCD_Command(0x01);      // Clear display

    HAL_Delay(5);
}

/************************************************
                LCD Clear
************************************************/
void LCD_Clear(void)
{
    LCD_Command(0x01);
    HAL_Delay(2);
}

/************************************************
                LCD Set Cursor
************************************************/
void LCD_SetCursor(uint8_t row,uint8_t col)
{
    if(row==0)
        LCD_Command(0x80+col);
    else
        LCD_Command(0xC0+col);
}

/************************************************
                LCD String
************************************************/
void LCD_String(char *str)
{
    while(*str)
    {
        LCD_Data(*str++);
    }
}

/************************************************
                Display Parking Count
************************************************/
void Display_Count(void)
{
    LCD_Clear();

    LCD_SetCursor(0,0);
    LCD_String("SMART PARKING");

    LCD_SetCursor(1,0);

    sprintf(lcd_buffer,"%d/%d Cars",
            Car_Count,
            TOTAL_SLOTS);

    LCD_String(lcd_buffer);
}

/************************************************
                Servo Open (90°)
************************************************/
void Servo_Open(void)
{
    __HAL_TIM_SET_COMPARE(&htim1, SERVO_CHANNEL, 150);
}

/************************************************
                Servo Close (0°)
************************************************/
void Servo_Close(void)
{
    __HAL_TIM_SET_COMPARE(&htim1, SERVO_CHANNEL, 50);
}
/************************************************
                ENTRY SENSOR
************************************************/
void Check_Entry(void)
{
    if(HAL_GPIO_ReadPin(ENTRY_IR_PORT, ENTRY_IR_PIN) == GPIO_PIN_RESET)
    {
        HAL_Delay(50);      // Debounce

        if(HAL_GPIO_ReadPin(ENTRY_IR_PORT, ENTRY_IR_PIN) == GPIO_PIN_RESET)
        {
            if(Car_Count < TOTAL_SLOTS)
            {
                Car_Count++;

                /* Green LED ON */
                HAL_GPIO_WritePin(GREEN_LED_PORT,
                                  GREEN_LED_PIN,
                                  GPIO_PIN_SET);

                /* Red LED OFF */
                HAL_GPIO_WritePin(RED_LED_PORT,
                                  RED_LED_PIN,
                                  GPIO_PIN_RESET);

                /* Open Gate */
                Servo_Open();

                Display_Count();

                HAL_Delay(3000);

                /* Close Gate */
                Servo_Close();

                HAL_GPIO_WritePin(GREEN_LED_PORT,
                                  GREEN_LED_PIN,
                                  GPIO_PIN_RESET);
            }
            else
            {
                LCD_Clear();

                LCD_SetCursor(0,0);
                LCD_String("PARKING FULL");

                LCD_SetCursor(1,0);
                LCD_String("NO SPACE");

                HAL_GPIO_WritePin(RED_LED_PORT,
                                  RED_LED_PIN,
                                  GPIO_PIN_SET);

                HAL_Delay(2000);

                HAL_GPIO_WritePin(RED_LED_PORT,
                                  RED_LED_PIN,
                                  GPIO_PIN_RESET);

                Display_Count();
            }

            /* Wait until car leaves sensor */
            while(HAL_GPIO_ReadPin(ENTRY_IR_PORT,
                                   ENTRY_IR_PIN) == GPIO_PIN_RESET);

            HAL_Delay(200);
        }
    }
}

/************************************************
                EXIT SENSOR
************************************************/
void Check_Exit(void)
{
    if(HAL_GPIO_ReadPin(EXIT_IR_PORT, EXIT_IR_PIN) == GPIO_PIN_RESET)
    {
        HAL_Delay(50);

        if(HAL_GPIO_ReadPin(EXIT_IR_PORT, EXIT_IR_PIN) == GPIO_PIN_RESET)
        {
            if(Car_Count > 0)
            {
                Car_Count--;

                HAL_GPIO_WritePin(GREEN_LED_PORT,
                                  GREEN_LED_PIN,
                                  GPIO_PIN_SET);

                HAL_GPIO_WritePin(RED_LED_PORT,
                                  RED_LED_PIN,
                                  GPIO_PIN_RESET);

                Servo_Open();

                Display_Count();

                HAL_Delay(3000);

                Servo_Close();

                HAL_GPIO_WritePin(GREEN_LED_PORT,
                                  GREEN_LED_PIN,
                                  GPIO_PIN_RESET);
            }

            while(HAL_GPIO_ReadPin(EXIT_IR_PORT,
                                   EXIT_IR_PIN) == GPIO_PIN_RESET);

            HAL_Delay(200);
        }
    }
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
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);

  /* Gate Closed Initially */
  Servo_Close();

  /* LEDs OFF */
  HAL_GPIO_WritePin(GREEN_LED_PORT, GREEN_LED_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(RED_LED_PORT, RED_LED_PIN, GPIO_PIN_RESET);

  /* LCD Initialize */
  LCD_Init();

  Display_Count();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
	  Check_Entry();

	      Check_Exit();
    /* USER CODE BEGIN 3 */
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 71;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA0 PA1 PA2 PA3
                           PA4 PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA6 PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB12 PB13 PB14 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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
