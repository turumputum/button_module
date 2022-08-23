/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "fatfs.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usb_otg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdio.h>
#include <string.h>
#include "stdlib.h"

#include "gestureConfig.h"

#include "w25qxx.h"

#include "ini.h"

#include "math.h"
#include "apds9960.h"

#include "PwmLed.h"

#include "usb_device.h"
#include "usbd_cdc_acm_if.h"
#include "usbd_hid_keyboard.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

#define ETH_BUFF_SIZE 255
#define MAX_BRIGHT 250
#define MIN_BRIGHT 50
#define FRONT_SIZE 20048

#define I2C_ADDRESS                                              0x5A
#define I2C_ID_ADDRESS                                           0x5D
#define I2C_TIMEOUT                                              10

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

int tempRight,tempLeft;


PwmLed_HandleTypeDef Led_1;

uint8_t *front;
uint8_t ledRefreshFlag = 0;
uint32_t prevTick = 0;

typedef struct {
	PwmLed_HandleTypeDef *pwmLed;
	uint32_t num_led;

	uint16_t frontLen;
	uint16_t diameter;
	uint16_t *ledsCoordinate;
	uint16_t effectBufLen;
	uint8_t *effectBuf;
	double ledAngleRadian;

	uint16_t tick;
	uint16_t tickLenght;

	uint8_t effect;

	uint8_t *ledBrightMass;
	uint32_t brightMassLen;

	uint8_t state;

	RgbColor RGB;
	HsvColor HSV;
} ledRing_HandleTypeDef;

#define LEFT_ARROW 0x50
#define RIGHT_ARROW 0x4F
#define UP_ARROW 0x52
#define DOWN_ARROW 0x51
#define ENTER_KEY 0x58
#define CDC_REQUEST 0x99

typedef struct {
	uint8_t MODIFIER;
	uint8_t RESERVED;
	uint8_t KEYCODE1;
	uint8_t KEYCODE2;
	uint8_t KEYCODE3;
	uint8_t KEYCODE4;
	uint8_t KEYCODE5;
	uint8_t KEYCODE6;
} keyboardHID;

uint8_t keyboardhid[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

enum {
	LED_RUN, LED_STOP
};

enum {
	FADE_UP, FADE_DOWN, RUN, SWIPE_UP, SWIPE_DOWN, SWIPE_LEFT, SWIPE_RIGHT, WAITING
};
extern USBD_HandleTypeDef hUsbDevice;
ledRing_HandleTypeDef ledRing;

uint16_t effectTick, effectLenght;

int gesture = DIR_NONE;
int prevGesture = DIR_NONE;
//uint32_t gestureTik;
uint32_t lastGesture = 0;
uint16_t waTick;
uint32_t HID_release_tick;
//uint8_t waFlag;

extern configuration config;

uint8_t usb_buff_tx[20]; // Строка которую хотим отправить по usb
uint8_t usb_buff_rx[20]; // Данные полученные по usb

uint8_t CDC_reciveFlag;

FRESULT res;
FATFS fs;
DIR dir; /* Directory object */
FILINFO fno; /* File information */
uint8_t bb[_MAX_SS];

uint8_t buf_1[8] = { 8, 7, 6, 5, 4, 3, 2, 1 };
uint8_t buf_2[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
int calcSin(int tick, int lenght) {
	float phase = tick * M_PI / lenght;
	int delta = config.brightMax - config.brightMin;
	//int value = sin(3.14 - phase) * delta / 2 + delta / 2;
	int value = sin(phase) * delta;
	value = value + config.brightMin;
	if (value > config.brightMax)
		value = config.brightMax;
	return value;
}

//--------------------------------------------------------------------------------------------------
/*
int _write(int file, char *ptr, int len) {
	// Implement your write code here, this is used by puts and printf for example 
	int i = 0;
	for (i = 0; i < len; i++)
		ITM_SendChar((*ptr++));
	return len;
}
*/

//--------------------------------------------------------------------------------------------------
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM1) {
		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
			HAL_TIM_PWM_Stop_DMA(htim, TIM_CHANNEL_1);
		}
	}
	if (htim->Instance == TIM3) {
		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
			HAL_TIM_PWM_Stop_DMA(htim, TIM_CHANNEL_1);
		}
	}
}
//--------------------------------------------------------------------------------------------------
void setMinBright(ledRing_HandleTypeDef *ledRing) {
	ledRing->HSV.v = config.brightMin;
	ledRing->RGB = HsvToRgb(ledRing->HSV);
	for (int i = 0; i < ledRing->num_led; i++) {
		PwmLed_setPixel_gammaCorrection(ledRing->pwmLed, ledRing->RGB.r, ledRing->RGB.g, ledRing->RGB.b, i);
	}
	PwmLed_light(ledRing->pwmLed);

}
//--------------------------------------------------------------------------------------------------
void setLedsBlack(PwmLed_HandleTypeDef *pwmLed) {

	for (int i = 0; i < pwmLed->LED_COUNT; i++) {
		PwmLed_setPixel_gammaCorrection(pwmLed, 0, 0, 0, i);
	}
	PwmLed_light(pwmLed);
}
//--------------------------------------------------------------------------------------------------
uint8_t setLedEffect(ledRing_HandleTypeDef *ledRing, uint8_t effect, uint16_t lenght) {
	if ((effect == FADE_UP) || (effect == FADE_DOWN)) {
		ledRing->state = LED_RUN;
		ledRing->tick = 0;
		ledRing->effect = effect;
		ledRing->tickLenght = lenght;
		ledRing->HSV = RgbToHsv(ledRing->RGB);
	}

	if ((effect == SWIPE_DOWN) || (effect == SWIPE_UP) || (effect == SWIPE_LEFT) || (effect == SWIPE_RIGHT)) {
		ledRing->state = LED_RUN;
		ledRing->effect = effect;
		ledRing->tick = 0;
		ledRing->tickLenght = lenght;
		ledRing->HSV = RgbToHsv(ledRing->RGB);
		ledRing->frontLen = lenght / 2;
		ledRing->diameter = lenght - ledRing->frontLen;
		ledRing->effectBufLen = lenght + ledRing->frontLen;

		free(ledRing->effectBuf);
		ledRing->effectBuf = (uint8_t*) malloc(ledRing->effectBufLen * sizeof(uint8_t));
		memset(ledRing->effectBuf, config.brightMin, ledRing->effectBufLen);

		//printf("Front:");
		for (int i = 0; i < ledRing->frontLen; i++) {
			ledRing->effectBuf[i] = calcSin(i, ledRing->frontLen);
			//printf("%d-", ledRing->effectBuf[i]);
		}
		//printf("\r\n");

		//calc first quarter
		uint16_t quarterNum = ledRing->num_led / 4;
		for (int t = 0; t < quarterNum; t++) {
			ledRing->ledsCoordinate[t] = ledRing->frontLen + (ledRing->diameter / 2 - ledRing->diameter / 2 * cos(ledRing->ledAngleRadian / 2 + ledRing->ledAngleRadian * t));

		}
		//mirror to second quarter
		for (int t = 0; t < quarterNum; t++) {
			ledRing->ledsCoordinate[quarterNum + t] = ledRing->frontLen + ledRing->diameter / 2 + (ledRing->frontLen + ledRing->diameter / 2 - ledRing->ledsCoordinate[quarterNum - t - 1]);

		}

		//printf("ledsCoordinate:");
		for (int t = 0; t < ledRing->num_led / 2; t++) {
			//printf("%d-", ledRing->ledsCoordinate[t]);
		}
		//printf("\r\n");

	}

	return 1;
}
//--------------------------------------------------------------------------------------------------
uint8_t processLedEffect(ledRing_HandleTypeDef *ledRing) {

	ledRing->tick++;
	if (ledRing->tick >= ledRing->tickLenght) {
		ledRing->state = LED_STOP;
		ledRing->effect = WAITING;
		setMinBright(ledRing);
		return LED_STOP;
	}

	if ((ledRing->effect == FADE_UP) || (ledRing->effect == FADE_DOWN)) {

		float progres = (float) ledRing->tick / ledRing->tickLenght;
		if (ledRing->effect == FADE_UP) {
			ledRing->HSV.v = config.brightMin + progres * (config.brightMax - config.brightMin);
		} else if (ledRing->effect == FADE_DOWN) {
			ledRing->HSV.v = config.brightMax - (progres * (config.brightMax - config.brightMin));
		}
		RgbColor tmpRGB = HsvToRgb(ledRing->HSV);
		for (int i = 0; i < ledRing->num_led; i++) {
			PwmLed_setPixel_gammaCorrection(ledRing->pwmLed, tmpRGB.r, tmpRGB.g, tmpRGB.b, i);
		}
		PwmLed_light(ledRing->pwmLed);
	}

	if ((ledRing->effect == SWIPE_DOWN) || (ledRing->effect == SWIPE_UP) || (ledRing->effect == SWIPE_LEFT) || (ledRing->effect == SWIPE_RIGHT)) {

		int tmp = ledRing->effectBuf[ledRing->effectBufLen - 1];
		for (int i = 0; i < ledRing->effectBufLen - 1; i++) {
			ledRing->effectBuf[ledRing->effectBufLen - i - 1] = ledRing->effectBuf[ledRing->effectBufLen - i - 2];
		}
		ledRing->effectBuf[0] = tmp;

		/*
		 printf("effectBuf:");
		 for (int i = 0; i < ledRing->effectBufLen; i++) {
		 printf("%d-", ledRing->effectBuf[i]);
		 }
		 printf("\r\n");
		 */

		for (int i = 0; i < ledRing->num_led / 2; i++) {
			ledRing->ledBrightMass[i] = ledRing->effectBuf[ledRing->ledsCoordinate[i]];
			ledRing->ledBrightMass[ledRing->num_led - 1 - i] = ledRing->effectBuf[ledRing->ledsCoordinate[i]];
		}

		int rotateNum;
		if (ledRing->effect == SWIPE_DOWN) {
			rotateNum = 0;
		} else if (ledRing->effect == SWIPE_LEFT) {
			rotateNum = ledRing->num_led / 4;
		} else if (ledRing->effect == SWIPE_UP) {
			rotateNum = ledRing->num_led / 2;
		} else if (ledRing->effect == SWIPE_RIGHT) {
			rotateNum = ledRing->num_led * 3 / 4;
		}

		for (int y = 0; y < rotateNum; y++) {
			uint8_t tmp = ledRing->ledBrightMass[ledRing->num_led - 1];
			for (int i = 1; i < ledRing->num_led + 1; i++) {
				ledRing->ledBrightMass[ledRing->num_led - i] = ledRing->ledBrightMass[ledRing->num_led - 1 - i];
			}
			ledRing->ledBrightMass[0] = tmp;
		}

		//printf("Tick:%d LedBrigtMass-", ledRing->tick);
		for (int i = 0; i < ledRing->num_led; i++) {
			ledRing->HSV.v = ledRing->ledBrightMass[i];
			RgbColor tmpRGB = HsvToRgb(ledRing->HSV);
			//printf("%d-", ledRing->ledBrightMass[i]);
			PwmLed_setPixel_gammaCorrection(ledRing->pwmLed, tmpRGB.r, tmpRGB.g, tmpRGB.b, i);
		}
		//printf("\r\n");

		PwmLed_light(ledRing->pwmLed);

	}

	return 1;
}
//--------------------------------------------------------------------------------------------------
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	
	if (htim->Instance == TIM4) {

		ledRefreshFlag = 1;

		waTick++;
		if (waTick > 60) {
			waTick = 0;
//			waFlag = 1;
		}

		if (((HAL_GetTick() - HID_release_tick) > 20) && (HID_release_tick != 0)) {
			HID_release_tick = 0;
			keyboardhid[2] = 0x00;
			//USBD_HID_SendReport(&hUsbDevice, keyboardhid, sizeof(keyboardhid));
			USBD_HID_Keybaord_SendReport(&hUsbDevice, keyboardhid, sizeof(keyboardhid));
		}

	}
	prevTick = HAL_GetTick();
}
//--------------------------------------------------------------------------------------------------
void CDC_report(uint8_t in) {
	if (in == LEFT_ARROW) {
		CDC_Transmit(0, (uint8_t*) ("left\n"), strlen("left\n"));
	} else if (in == RIGHT_ARROW) {
		CDC_Transmit(0, (uint8_t*) ("right\n"), strlen("right\n"));
	} else if (in == UP_ARROW) {
		CDC_Transmit(0, (uint8_t*) ("up\n"), strlen("up\n"));
	} else if (in == DOWN_ARROW) {
		CDC_Transmit(0, (uint8_t*) ("down\n"), strlen("down\n"));
	} else if (in == CDC_REQUEST) {
			CDC_Transmit(0, (uint8_t*) ("Request\n"), strlen("Request\n"));
		}
}
//--------------------------------------------------------------------------------------------------
void CDC_checkRecive() {
	if (CDC_reciveFlag) {
		printf("recive some: %s \n", usb_buff_rx);
		CDC_report(CDC_REQUEST);
	}
	CDC_reciveFlag = 0;
}
//--------------------------------------------------------------------------------------------------
void w25qSetBlockProtectt(uint8_t protect) {
	uint8_t temp[] = { 0x50, 0x00 };

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, temp, 1, 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

	temp[0] = 0x01;
	temp[1] = ((protect & 0x0F) << 2);

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, temp, 2, 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
}
//--------------------------------------------------------------------------------------------------
void HID_keyboard_report(uint8_t in) {
	if (config.HID_keyboard_enable == 1) {
		HID_release_tick = HAL_GetTick();

		if (in == RIGHT_ARROW) {
			keyboardhid[2] = RIGHT_ARROW;
		} else if (in == DOWN_ARROW) {
			keyboardhid[2] = DOWN_ARROW;
		} else if (in == LEFT_ARROW) {
			keyboardhid[2] = LEFT_ARROW;
		} else if (in == UP_ARROW) {
			keyboardhid[2] = UP_ARROW;
		}
		//USBD_HID_SendReport(&hUsbDevice, keyboardhid, sizeof(keyboardhid));
		USBD_HID_Keybaord_SendReport(&hUsbDevice, keyboardhid, sizeof(keyboardhid));
	}
}

void testModeColor(int dir) {
	if (config.testMode) {
		if (dir > 0) {
			ledRing.HSV.h += 20;
		} else if (dir < 0) {
			ledRing.HSV.h -= 20;
		}

	}
}

void testModeBright(int dir) {
	if (config.testMode) {
		if (dir > 0) {
			config.brightMax += 20;
			if (config.brightMax > 255)
				config.brightMax = 255;
			config.brightMin += 20;
			if (config.brightMin > 150)
				config.brightMin = 150;
		} else if(dir<0){
			config.brightMax -= 20;
			if (config.brightMax <100)
				config.brightMax = 100;
			config.brightMin -= 20;
			if (config.brightMin <10)
				config.brightMin = 10;
		}
	}
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_DMA_Init();
  MX_TIM1_Init();
  MX_SPI1_Init();
  MX_TIM2_Init();
  MX_I2C1_Init();
  MX_TIM4_Init();
//  MX_USB_OTG_FS_PCD_Init();
//  MX_FATFS_Init();
	MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	//--------------------------------------------------------------------------------------------------
	// printf("Hello\r\n");
	// W25qxx_Init();
	// //W25qxx_EraseChip();
	// //res = f_mkfs("", FM_FAT, 4096, bb, _MAX_SS);

	// res = f_mount(&fs, "", 1);
	// if (res == FR_OK) {
	// 	printf("mount FS OK\r\n");
	// } else if (res == 13) {
	// 	res = f_mkfs("", FM_FAT, 4096, bb, _MAX_SS);
	// 	printf("Formst disk:  %d \r\n", res);
	// } else {
	// 	printf("mount Fail:  %d \r\n", res);
	// }

	set_default_config();

	// res = f_opendir(&dir, "/");
	// if (res == FR_OK) {
	// 	res = f_findfirst(&dir, &fno, fno.fname, "*.ini");
	// 	if (res == FR_OK && fno.fname[0]) {
	// 		printf("found file  %s \r\n", fno.fname);
	// 	} else {
	// 		write_default_config();
	// 	}
	// }

	// res = ini_parse_fatfs("config.ini", configReader, &config);
	// if (res < 0) {
	// 	printf("Can't load 'config.ini', set default\n");
	// 	char error_string[100];
	// 	sprintf(error_string, "Fail read Config.ini, chek line %d", res);
	// 	writeErrorTxt(error_string);
	// }

	// res = f_mount(NULL, "", 1);
	// if (res != FR_OK) {
	// 	printf("unMount FS fail: %d\r\n", res);
	// }
	//--------------------------------------------------------------------------------------------------

	//MX_USB_DEVICE_Init();

	//--------------------------------------------------------------------------------------------------

	if (apds9960init()) {
		printf("APDS-9960 initialization complete\r\n");
	} else {
		printf("Something went wrong during APDS-9960 init!\r\n");
		printf("Error!!! Re-Start Board\r\n");
		HAL_Delay(1000);
		//NVIC_SystemReset();
	}
	// Start running the APDS-9960 gesture sensor engine
	if (enableGestureSensor(1)) {
		printf("Gesture sensor is now running\r\n");
	} else {
		printf("Something went wrong during gesture sensor init!\r\n");
		printf("Error!!! Re-Start Board\r\n");
		HAL_Delay(1000);
		//NVIC_SystemReset();
	}
	//--------------------------------------------------------------------------------------------------
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	PwmLed_init(&Led_1, &htim1, TIM_CHANNEL_1, 16, WS2815);
	ledRing.pwmLed = &Led_1;
	ledRing.num_led = 16;
	ledRing.ledBrightMass = (uint8_t*) malloc(ledRing.num_led * sizeof(uint8_t));
	ledRing.ledAngleRadian = 2 * M_PI / ledRing.num_led;
	ledRing.ledsCoordinate = (uint16_t*) malloc(ledRing.num_led / 2 * sizeof(uint16_t));
	ledRing.state = LED_STOP;
	ledRing.RGB.r = config.RGB.r;
	ledRing.RGB.g = config.RGB.g;
	ledRing.RGB.b = config.RGB.b;
	ledRing.HSV = RgbToHsv(ledRing.RGB);

	setLedEffect(&ledRing, SWIPE_DOWN, 90);

	HAL_TIM_Base_Start_IT(&htim4);
	int effectLen = 90;

	uint32_t lastShow = 0;
	
	//--------------------------------------------------------------------------------------------------
	while (1) 
	{
		u_int32_t now = HAL_GetTick();

		// overflow or 30 ms 
		if ( (now < lastGesture) || ((now - lastGesture) >= 30))
		{
			prevGesture = gesture;
			gesture = apds9960ReadSensor(); /* Read Gesture */
			if (gesture != 0) {
				printf("%ld: gesture is^ %d \r\n",HAL_GetTick(), gesture); /* Report to Serial Port(Debug Port) */
			}

			lastGesture = now;
		}

		if (gesture != DIR_NONE)
		{
			switch (gesture)
			{
				case DIR_DOWN:
					if (config.vertical_swipe_enable == 1) 
					{
						testModeBright(-1);
						setLedEffect(&ledRing, SWIPE_DOWN, effectLen);
//						CDC_report(DOWN_ARROW);
//						HID_keyboard_report(DOWN_ARROW);
					}
					break;

				case DIR_UP:
					if (config.vertical_swipe_enable == 1) 
					{
						testModeBright(1);
						setLedEffect(&ledRing, SWIPE_UP, effectLen);
//						CDC_report(UP_ARROW);
//						HID_keyboard_report(UP_ARROW);
					}
					break;

				case DIR_RIGHT:
					if (config.horizontal_swipe_enable == 1) 
					{
						setLedEffect(&ledRing, SWIPE_RIGHT, effectLen);
//						CDC_report(RIGHT_ARROW);
//						HID_keyboard_report(RIGHT_ARROW);
						testModeColor(1);
					}
					break;
				
				case DIR_LEFT:
					if (config.horizontal_swipe_enable == 1) 
					{
						setLedEffect(&ledRing, SWIPE_LEFT, effectLen);
//						CDC_report(LEFT_ARROW);
//						HID_keyboard_report(LEFT_ARROW);
						testModeColor(-1);
					}
					break;
				
				default:
					break;
			}

			gesture = DIR_NONE;
		}

// 		if ((now - lastShow) >= 1000)
// 		{
// extern unsigned long i2cErrors;

// 			printf("i2cErrors = %d\n", i2cErrors);

// 			lastShow = now;
// 		}


		if (ledRefreshFlag == 1) {
			ledRefreshFlag = 0;
			processLedEffect(&ledRing);
		}

//		CDC_checkRecive();
		HAL_Delay(1);
	}
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
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
	while (1) {
	}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
