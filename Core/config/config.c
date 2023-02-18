#include "../config/config.h"

#include <string.h>
#include <stdio.h>
#include "stdlib.h"
#include "fatfs.h"

configuration config;

int configReader(void *user, const char *section, const char *name, const char *value) {
	configuration *pconfig = (configuration*) user;

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
	for (int i = 0; i < 8; i++) {
		char tmp_name[20];
		memset(tmp_name, 0, sizeof(tmp_name));
		sprintf(tmp_name, "button_%d_mode", i+1);
		if (MATCH("buttons", tmp_name)) {
			pconfig->buttons_mode[i] = atoi(value);
		}

		memset(tmp_name, 0, sizeof(tmp_name));
		sprintf(tmp_name, "button_%d_inverse", i+1);
		if (MATCH("buttons", tmp_name)) {
			pconfig->buttons_inverse[i] = atoi(value);
		}

		memset(tmp_name, 0, sizeof(tmp_name));
		sprintf(tmp_name, "led_%d_inverse", i+1);
		if (MATCH("buttons", tmp_name)) {
			pconfig->leds_inverse[i] = atoi(value);
		}

		memset(tmp_name, 0, sizeof(tmp_name));
		sprintf(tmp_name, "led_%d_control", i+1);
		if (MATCH("buttons", tmp_name)) {
			pconfig->leds_control[i] = atoi(value);
		}

	}

	if (MATCH("system", "name")) {
		pconfig->name = strdup(value);
	} else if (MATCH("HID", "enable_HID")) {
		pconfig->enable_HID = atoi(value);
	} else if (MATCH("HID", "HID_keylayout")) {
		pconfig->HID_keylayout = atoi(value);
	} else if (MATCH("net", "enable_NET")) {
		pconfig->enable_NET = atoi(value);
	} else if (MATCH("net", "enable_DHCP")) {
		pconfig->enable_DHCP = atoi(value);
	} else if (MATCH("net", "IP")) {
		pconfig->IP = strdup(value);
	} else if (MATCH("net", "netMask")) {
		pconfig->netMask = strdup(value);
	} else if (MATCH("net", "gateWay")) {
		pconfig->gateWay = strdup(value);
	} else if (MATCH("udp", "enable_UDP")) {
		pconfig->enable_UDP = atoi(value);
	} else if (MATCH("udp", "UDP_IP_server")) {
		pconfig->UDP_IP_server = strdup(value);
	} else if (MATCH("udp", "UDP_port_server")) {
		pconfig->UDP_port_server = atoi(value);
	} else if (MATCH("udp", "UDP_port_my")) {
		pconfig->UDP_port_my = atoi(value);
	} else if (MATCH("osc", "enable_OSC")) {
		pconfig->enable_OSC = atoi(value);
	} else if (MATCH("osc", "OSC_IP_server")) {
		pconfig->OSC_IP_server = strdup(value);
	} else if (MATCH("osc", "OSC_port_server")) {
		pconfig->OSC_port_server = atoi(value);
	} else if (MATCH("osc", "OSC_port_my")) {
		pconfig->OSC_port_my = atoi(value);
	} else if (MATCH("mqtt", "enable_MQTT")) {
		pconfig->enable_MQTT = atoi(value);
	} else if (MATCH("mqtt", "IP_mqtt_broker")) {
		pconfig->IP_mqtt_broker = strdup(value);
	} else if (MATCH("mqtt", "port_mqtt_broker")) {
		pconfig->port_mqtt_broker = atoi(value);
	} else if (MATCH("mqtt", "login_mqtt_broker")) {
		pconfig->login_mqtt_broker = strdup(value);
	} else if (MATCH("mqtt", "pass_mqtt_broker")) {
		pconfig->pass_mqtt_broker = strdup(value);
	} else {
		return 0; /* unknown section/name, error */
	}
	return 1;
}
//--------------------------------------------------------------------------------------------------
void set_default_config(void) {

	config.name = "button_module_1";

	config.enable_HID = 0;
	config.HID_keylayout = 0;

	for (int i = 0; i < 8; i++) {
		config.buttons_mode[i] = 0;
		config.buttons_inverse[i] = 0;
		config.leds_inverse[i] = 0;
		config.leds_control[i] = 0;

	}
	config.enable_NET = 0;
	config.enable_DHCP = 0;
	config.IP = "";
	config.netMask = "";
	config.gateWay = "";

	config.enable_UDP = 0;
	config.UDP_IP_server = "";
	config.UDP_port_server = 0;
	config.UDP_port_my = 0;

	config.enable_OSC = 0;
	config.OSC_IP_server = "";
	config.OSC_port_server = 0;
	config.OSC_port_my = 0;

	config.enable_MQTT = 0;
	config.IP_mqtt_broker = "";
	config.port_mqtt_broker = 0;
	config.login_mqtt_broker = "";
	config.pass_mqtt_broker = "";

}
//--------------------------------------------------------------------------------------------------
void write_default_config(void) {
	f_unlink("config.ini"); // delelet error file

	FRESULT fr;
	FIL errFile;
	unsigned int bytesWritten;
	char buff[100];
	fr = f_open(&errFile, "config.ini", FA_CREATE_ALWAYS | FA_WRITE);
	if (fr != FR_OK) {
		printf("f_open() failed, res = %d\r\n", fr);
		return;
	}

	memset(buff, 0, sizeof(buff));
	sprintf(buff, "[system]\n");
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "name = %s \n", config.name);
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
	memset(buff, 0, sizeof(buff));

	sprintf(buff, "\n[HID]\n");
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "enable_HID = %d \n", config.enable_HID);
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "HID_keylayout = %d ;// 0-number key, 1-arrow keys, enter, backspace, space, shift\n", config.HID_keylayout);
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);

	sprintf(buff, "\n[buttons]\n\n");
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
	memset(buff, 0, sizeof(buff));

	for (int i = 0; i < 8; i++) {
		memset(buff, 0, sizeof(buff));
		sprintf(buff, "button_%d_mode = %d    ;// 0-push button, 1-selector group 1, 2-selector group 2, 3-trigger button\n", (i + 1),  config.buttons_mode[i]);
		fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
		memset(buff, 0, sizeof(buff));
		sprintf(buff, "button_%d_inverse = %d ;// 0-low 1-high \n", (i + 1), config.buttons_inverse[i]);
		fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
		memset(buff, 0, sizeof(buff));
		sprintf(buff, "led_%d_control = %d    ;// 0-external 1-local, 2-always on, 3-always off\n", (i + 1), config.leds_control[i]);
		fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
		memset(buff, 0, sizeof(buff));
		sprintf(buff, "led_%d_inverse = %d    ;// 0-low 1-high\n\n", (i + 1), config.leds_inverse[i]);
		fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);

	}

	memset(buff, 0, sizeof(buff));
	sprintf(buff, "\n[net]\n");
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "enable_NET = %d \n", config.enable_NET);
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "enable_DHCP = %d \n", config.enable_DHCP);
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "IP = %s \n", config.IP);
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "netMask = %s \n", config.netMask);
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "gateWay = %s \n", config.gateWay);
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);

	memset(buff, 0, sizeof(buff));
	sprintf(buff, "\n[udp]\n");
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "enable_UDP = %d \n", config.enable_UDP);
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "UDP_IP_server = %s \n", config.UDP_IP_server);
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "UDP_port_server = %d \n", config.UDP_port_server);
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "UDP_port_my = %d \n", config.UDP_port_my);
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);

	memset(buff, 0, sizeof(buff));
	sprintf(buff, "\n[osc]\n");
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "enable_OSC = %d \n", config.enable_OSC);
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "OSC_IP_server = %s \n", config.OSC_IP_server);
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "OSC_port_server = %d \n", config.OSC_port_server);
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "OSC_port_my = %d \n", config.OSC_port_my);
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);

	memset(buff, 0, sizeof(buff));
	sprintf(buff, "\n[mqtt]\n");
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
	memset(buff, 0, sizeof(buff));
		sprintf(buff, "enable_MQTT = %d \n", config.enable_MQTT);
		fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "IP_mqtt_broker = %s \n", config.IP_mqtt_broker);
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "port_mqtt_broker = %d \n", config.port_mqtt_broker);
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "login_mqtt_broker = %s \n", config.login_mqtt_broker);
	fr = f_write(&errFile, buff, strlen(buff), &bytesWritten);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "pass_mqtt_broker = %s \n", config.pass_mqtt_broker);
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
