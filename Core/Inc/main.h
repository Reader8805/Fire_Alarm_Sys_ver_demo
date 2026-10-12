/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LCD_D2_Pin GPIO_PIN_2
#define LCD_D2_GPIO_Port GPIOE
#define LCD_D3_Pin GPIO_PIN_3
#define LCD_D3_GPIO_Port GPIOE
#define LCD_D4_Pin GPIO_PIN_4
#define LCD_D4_GPIO_Port GPIOE
#define LCD_D5_Pin GPIO_PIN_5
#define LCD_D5_GPIO_Port GPIOE
#define LCD_D6_Pin GPIO_PIN_6
#define LCD_D6_GPIO_Port GPIOE
#define PB3_Pin GPIO_PIN_1
#define PB3_GPIO_Port GPIOC
#define PB4_Pin GPIO_PIN_2
#define PB4_GPIO_Port GPIOC
#define PB5_Pin GPIO_PIN_3
#define PB5_GPIO_Port GPIOC
#define IC_74HC595_LATCH_Pin GPIO_PIN_4
#define IC_74HC595_LATCH_GPIO_Port GPIOA
#define IC_74HC595_SCK_Pin GPIO_PIN_5
#define IC_74HC595_SCK_GPIO_Port GPIOA
#define IC_74HC595_Data_Pin GPIO_PIN_7
#define IC_74HC595_Data_GPIO_Port GPIOA
#define PB6_Pin GPIO_PIN_4
#define PB6_GPIO_Port GPIOC
#define PB7_Pin GPIO_PIN_5
#define PB7_GPIO_Port GPIOC
#define LCD_D7_Pin GPIO_PIN_7
#define LCD_D7_GPIO_Port GPIOE
#define LCD_RS_Pin GPIO_PIN_8
#define LCD_RS_GPIO_Port GPIOE
#define LCD_RW_Pin GPIO_PIN_9
#define LCD_RW_GPIO_Port GPIOE
#define LCD_E_Pin GPIO_PIN_10
#define LCD_E_GPIO_Port GPIOE
#define LCD_BLC_Pin GPIO_PIN_11
#define LCD_BLC_GPIO_Port GPIOE
#define BUZZER_Pin GPIO_PIN_12
#define BUZZER_GPIO_Port GPIOE
#define ZB_TX_Pin GPIO_PIN_10
#define ZB_TX_GPIO_Port GPIOB
#define ZB_RX_Pin GPIO_PIN_11
#define ZB_RX_GPIO_Port GPIOB
#define ZB_NWK_Pin GPIO_PIN_12
#define ZB_NWK_GPIO_Port GPIOB
#define ZB_RST_Pin GPIO_PIN_15
#define ZB_RST_GPIO_Port GPIOB
#define IC_74HC595_CLEAR_Pin GPIO_PIN_14
#define IC_74HC595_CLEAR_GPIO_Port GPIOD
#define PB8_Pin GPIO_PIN_6
#define PB8_GPIO_Port GPIOC
#define PB9_Pin GPIO_PIN_7
#define PB9_GPIO_Port GPIOC
#define RS232_TX_Pin GPIO_PIN_9
#define RS232_TX_GPIO_Port GPIOA
#define RS232_RX_Pin GPIO_PIN_10
#define RS232_RX_GPIO_Port GPIOA
#define PB2_Pin GPIO_PIN_11
#define PB2_GPIO_Port GPIOA
#define AC_DETECT_Pin GPIO_PIN_12
#define AC_DETECT_GPIO_Port GPIOA
#define RS485_TX_Pin GPIO_PIN_5
#define RS485_TX_GPIO_Port GPIOD
#define RS485_RX_Pin GPIO_PIN_6
#define RS485_RX_GPIO_Port GPIOD
#define RELAY1_Pin GPIO_PIN_3
#define RELAY1_GPIO_Port GPIOB
#define RELAY2_Pin GPIO_PIN_4
#define RELAY2_GPIO_Port GPIOB
#define OUT1_Pin GPIO_PIN_5
#define OUT1_GPIO_Port GPIOB
#define OUT2_Pin GPIO_PIN_6
#define OUT2_GPIO_Port GPIOB
#define LCD_D0_Pin GPIO_PIN_0
#define LCD_D0_GPIO_Port GPIOE
#define LCD_D1_Pin GPIO_PIN_1
#define LCD_D1_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
