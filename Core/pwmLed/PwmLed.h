/*
 * PwmLed.h
 *
 *  Created on: Oct 20, 2021
 *      Author: Yac
 */

#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>


#define WS2815 2
#define SK6812_RGB 1
#define UCS1903 0

//--------------------------------------------------
#define BitIsSet(reg, bit) ((reg & (1<<bit)) != 0)
//--------------------------------------------------


typedef struct{
TIM_HandleTypeDef *htim;
uint32_t  CHANNEL;
uint8_t TYPE;
uint16_t BITS_PER_PIXEL;
uint16_t LOW_LENGHT;
uint16_t HIGH_LENGHT;
uint16_t DELAY_LEN;
uint16_t LED_COUNT;
uint16_t ARRAY_LEN;
uint16_t *BUF_DMA;

}PwmLed_HandleTypeDef;

void PwmLed_init(PwmLed_HandleTypeDef *pwmLed,TIM_HandleTypeDef *htim, uint32_t chennel, uint16_t LED_COUNT, uint8_t type);

void PwmLed_setPixel_gammaCorrection(PwmLed_HandleTypeDef *pwmLed, uint8_t Rpixel, uint8_t Gpixel, uint8_t Bpixel, uint16_t pos);
void PwmLed_light(PwmLed_HandleTypeDef *pwmLed);

void PwmLed_pixel_rgb_to_buf_dma(PwmLed_HandleTypeDef *pwmLed, uint8_t Rpixel, uint8_t Gpixel, uint8_t Bpixel, uint16_t posX);



