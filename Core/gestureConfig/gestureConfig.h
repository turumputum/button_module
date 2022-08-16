#include "hsv.h"
#include <stdio.h>

typedef struct {
	int brightMax;
	int brightMin;
	RgbColor RGB;
	uint8_t testMode;
	uint8_t HID_keyboard_enable;
	uint8_t vertical_swipe_enable;
	uint8_t horizontal_swipe_enable;

} configuration;

int configReader(void *user, const char *section, const char *name, const char *value);
void set_default_config(void);
void write_default_config(void);
void writeErrorTxt(char *buff);
