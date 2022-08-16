#include "gestureConfig.h"
//#include "hsv.h"
#include <string.h>
#include <stdio.h>
#include "stdlib.h"
#include "fatfs.h"

configuration config;

int configReader(void *user, const char *section, const char *name, const char *value) {
	configuration *pconfig = (configuration*) user;

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
	if (MATCH("led", "r")) {
		pconfig->RGB.r = atoi(value);
	} else if (MATCH("led", "g")) {
		pconfig->RGB.g = atoi(value);
	} else if (MATCH("led", "b")) {
		pconfig->RGB.b = atoi(value);
	} else if (MATCH("led", "brightMax")) {
		pconfig->brightMax = atoi(value);
	} else if (MATCH("led", "brightMin")) {
		pconfig->brightMin = atoi(value);
	} else if (MATCH("led", "testMode")) {
		pconfig->testMode = atoi(value);
	} else if (MATCH("gesture", "vertical_swipe_enable")) {
		pconfig->vertical_swipe_enable = atoi(value);
	} else if (MATCH("gesture", "horizontal_swipe_enable")) {
		pconfig->horizontal_swipe_enable = atoi(value);
	} else if (MATCH("system", "HID_keyboard_enable")) {
		pconfig->HID_keyboard_enable = atoi(value);
	} else {
		return 0; /* unknown section/name, error */
	}
	return 1;
}
//--------------------------------------------------------------------------------------------------
void set_default_config(void) {
	config.RGB.r = 255;
	config.RGB.g = 0;
	config.RGB.b = 0;
	config.brightMax = 200;
	config.brightMin = 50;

	config.testMode = 1;

	config.vertical_swipe_enable = 1;
	config.horizontal_swipe_enable = 1;

	config.HID_keyboard_enable = 1;
}
//--------------------------------------------------------------------------------------------------
void write_default_config(void) {
	f_unlink("config.ini"); // delelet error file

	FRESULT fr;
	FIL errFile;
	unsigned int bytesWritten;
	char buff[50];
	fr = f_open(&errFile, "config.ini", FA_CREATE_ALWAYS | FA_WRITE);
	if (fr != FR_OK) {
		printf("f_open() failed, res = %d\r\n", fr);
		return;
	}
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "[led]\n");
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "r = %d \n", config.RGB.r);
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "g = %d \n", config.RGB.g);
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "b = %d \n", config.RGB.b);
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "brightMax = %d \n", config.brightMax);
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "brightMin = %d \n", config.brightMin);
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "testMode = %d \n", config.testMode);
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "\n[gesture]\n");
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "vertical_swipe_enable = %d \n", config.vertical_swipe_enable);
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "horizontal_swipe_enable = %d \n", config.horizontal_swipe_enable);
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "\n[system]\n");
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "HID_keyboard_enable = %d \n", config.HID_keyboard_enable);
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);

	fr = f_close(&errFile);
	if (fr != FR_OK) {
		printf("f_close() failed, res = %d\r\n", fr);
		return;
	}

}

//--------------------------------------------------------------------------------------------------
void writeErrorTxt(char *buff) {

	FRESULT fr;
	FIL errFile;
	fr = f_open(&errFile, "error.txt", FA_CREATE_ALWAYS | FA_WRITE);
	if (fr != FR_OK) {
		printf("f_open() failed, res = %d\r\n", fr);
		return;
	}

	unsigned int bytesWritten;
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
	if (fr != FR_OK) {
		printf("f_write() failed, res = %d\r\n", fr);
		return;
	}

	fr = f_close(&errFile);
	if (fr != FR_OK) {
		printf("f_close() failed, res = %d\r\n", fr);
		return;
	}

}
