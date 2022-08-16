/*
 * PwmLed.cpp
 *
 *  Created on: Oct 20, 2021
 *      Author: Yac
 */

#include <PwmLed.h>
#include <stdio.h>

const uint8_t gamma8[256] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 3,
		3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18,
		19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25, 25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36, 37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
		50, 51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68, 69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89, 90, 92, 93, 95, 96, 98, 99, 101, 102,
		104, 105, 107, 109, 110, 112, 114, 115, 117, 119, 120, 122, 124, 126, 127, 129, 131, 133, 135, 137, 138, 140, 142, 144, 146, 148, 150, 152, 154, 156, 158, 160, 162, 164,
		167, 169, 171, 173, 175, 177, 180, 182, 184, 186, 189, 191, 193, 196, 198, 200, 203, 205, 208, 210, 213, 215, 218, 220, 223, 225, 228, 231, 233, 236, 239, 241, 244, 247,
		249, 252, 255 };
//------------------------------------------------------------------

extern TIM_HandleTypeDef htim1;

void PwmLed_init(PwmLed_HandleTypeDef *pwmLed, TIM_HandleTypeDef *htim, uint32_t chennel, uint16_t led_count, uint8_t type) {
	pwmLed->htim = htim;
	pwmLed->CHANNEL = chennel;
	pwmLed->LED_COUNT = led_count;
	pwmLed->TYPE = type;
	if (pwmLed->TYPE == SK6812_RGB) {
		pwmLed->DELAY_LEN = 65;
		pwmLed->BITS_PER_PIXEL = 24;
		pwmLed->HIGH_LENGHT = (uint16_t)(pwmLed->htim->Init.Period)*0.5;
		pwmLed->LOW_LENGHT = (uint16_t)(pwmLed->htim->Init.Period)*0.25;
	} else if (pwmLed->TYPE == WS2815) {
		pwmLed->DELAY_LEN = 280;
		pwmLed->BITS_PER_PIXEL = 24;
		pwmLed->HIGH_LENGHT = (uint16_t)(pwmLed->htim->Init.Period)*0.5;
		pwmLed->LOW_LENGHT = (uint16_t)(pwmLed->htim->Init.Period)*0.25;

	}

	pwmLed->ARRAY_LEN = pwmLed->DELAY_LEN + pwmLed->LED_COUNT * 24+50;
	pwmLed->BUF_DMA = (uint16_t*) malloc(pwmLed->ARRAY_LEN * sizeof(uint16_t));
	memset(pwmLed->BUF_DMA, 0, pwmLed->ARRAY_LEN * sizeof(uint16_t));

	for (int i = pwmLed->DELAY_LEN; i < pwmLed->ARRAY_LEN; i++) {
		pwmLed->BUF_DMA[i] = pwmLed->LOW_LENGHT;

	}
}

void PwmLed_pixel_rgb_to_buf_dma(PwmLed_HandleTypeDef *pwmLed, uint8_t Rpixel, uint8_t Gpixel, uint8_t Bpixel, uint16_t posX) {
	volatile uint16_t i;
	volatile uint16_t pot;

	//printf("Set pixel %d: ", posX);

	for (i = 0; i < 8; i++) {
		if (BitIsSet(Rpixel,(7-i)) == 1) {
			pot = pwmLed->DELAY_LEN + posX * pwmLed->BITS_PER_PIXEL + i + 8;
			pwmLed->BUF_DMA[pot] = pwmLed->HIGH_LENGHT;
			//printf("%d ", pwmLed->BUF_DMA[pot]);
		} else {
			pot = pwmLed->DELAY_LEN + posX * pwmLed->BITS_PER_PIXEL + i + 8;
			pwmLed->BUF_DMA[pot] = pwmLed->LOW_LENGHT;
			//printf("%d ", pwmLed->BUF_DMA[pot]);
		}
		if (BitIsSet(Gpixel,(7-i)) == 1) {
			pot = pwmLed->DELAY_LEN + posX * pwmLed->BITS_PER_PIXEL + i + 0;
			pwmLed->BUF_DMA[pot] = pwmLed->HIGH_LENGHT;
			//printf("%d ", pwmLed->BUF_DMA[pot]);
		} else {
			pot = pwmLed->DELAY_LEN + posX * pwmLed->BITS_PER_PIXEL + i + 0;
			pwmLed->BUF_DMA[pot] = pwmLed->LOW_LENGHT;
			//printf("%d ", pwmLed->BUF_DMA[pot]);
		}
		if (BitIsSet(Bpixel,(7-i)) == 1) {
			pot = pwmLed->DELAY_LEN + posX * pwmLed->BITS_PER_PIXEL + i + 16;
			pwmLed->BUF_DMA[pot] = pwmLed->HIGH_LENGHT;
			//printf("%d ", pwmLed->BUF_DMA[pot]);
		} else {
			pot = pwmLed->DELAY_LEN + posX * pwmLed->BITS_PER_PIXEL + i + 16;
			pwmLed->BUF_DMA[pot] = pwmLed->LOW_LENGHT;
			//printf("%d ", pwmLed->BUF_DMA[pot]);
		}
	}

	if(pwmLed->BUF_DMA==0){
				  HAL_Delay(10);
			  }
	//printf("\n");
}

void PwmLed_setPixel_gammaCorrection(PwmLed_HandleTypeDef *pwmLed, uint8_t Rpixel, uint8_t Gpixel, uint8_t Bpixel, uint16_t pos) {
	PwmLed_pixel_rgb_to_buf_dma(pwmLed, gamma8[Rpixel], gamma8[Gpixel], gamma8[Bpixel], pos);
}

void PwmLed_light(PwmLed_HandleTypeDef *pwmLed) {

	if (HAL_TIM_PWM_Start_DMA(pwmLed->htim, pwmLed->CHANNEL, (uint32_t*) pwmLed->BUF_DMA, pwmLed->ARRAY_LEN) != HAL_OK) {
		//if(HAL_TIM_PWM_Start_DMA(&htim1,TIM_CHANNEL_1,(uint32_t*)pwmLed->BUF_DMA,pwmLed->ARRAY_LEN+10)!=HAL_OK){
		printf("HAL_TIM_PWM_Start_DMA FAILED \n");
	}

}
