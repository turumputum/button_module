/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
#define FADE_UP 1
#define FADE_DOWN 2
#define RUN 3
#define SWIPE_UP 4
#define SWIPE_DOWN 5
#define SWIPE_LEFT 6
#define SWIPE_RIGHT 7
#define WAITING 8
#define NOT_RECOGNIZED 9


/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define LED_8_Pin GPIO_PIN_14
#define LED_8_GPIO_Port GPIOC
#define BT_8_Pin GPIO_PIN_15
#define BT_8_GPIO_Port GPIOC
#define CS_Pin GPIO_PIN_4
#define CS_GPIO_Port GPIOA
#define W_SCK_Pin GPIO_PIN_5
#define W_SCK_GPIO_Port GPIOA
#define W_MISO_Pin GPIO_PIN_6
#define W_MISO_GPIO_Port GPIOA
#define W_MOSI_Pin GPIO_PIN_7
#define W_MOSI_GPIO_Port GPIOA
#define W_CS_Pin GPIO_PIN_0
#define W_CS_GPIO_Port GPIOB
#define W_RST_Pin GPIO_PIN_1
#define W_RST_GPIO_Port GPIOB
#define Int_Pin GPIO_PIN_10
#define Int_GPIO_Port GPIOB
#define Int_EXTI_IRQn EXTI15_10_IRQn
#define BT_1_Pin GPIO_PIN_12
#define BT_1_GPIO_Port GPIOB
#define LED_1_Pin GPIO_PIN_13
#define LED_1_GPIO_Port GPIOB
#define BT_2_Pin GPIO_PIN_14
#define BT_2_GPIO_Port GPIOB
#define LED_2_Pin GPIO_PIN_15
#define LED_2_GPIO_Port GPIOB
#define BT_3_Pin GPIO_PIN_8
#define BT_3_GPIO_Port GPIOA
#define LED_3_Pin GPIO_PIN_9
#define LED_3_GPIO_Port GPIOA
#define BT_4_Pin GPIO_PIN_10
#define BT_4_GPIO_Port GPIOA
#define LED_4_Pin GPIO_PIN_15
#define LED_4_GPIO_Port GPIOA
#define BT_5_Pin GPIO_PIN_4
#define BT_5_GPIO_Port GPIOB
#define LED_5_Pin GPIO_PIN_5
#define LED_5_GPIO_Port GPIOB
#define BT_6_Pin GPIO_PIN_6
#define BT_6_GPIO_Port GPIOB
#define LED_6_Pin GPIO_PIN_7
#define LED_6_GPIO_Port GPIOB
#define BT_7_Pin GPIO_PIN_8
#define BT_7_GPIO_Port GPIOB
#define LED_7_Pin GPIO_PIN_9
#define LED_7_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
