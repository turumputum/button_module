#include "hsv.h"
#include <stdio.h>

#define PUSH_BUTTON 0
#define TRIGGER_BUTTON 3
#define SELECTOR_GROUP_1 1
#define SELECTOR_GROUP_2 2

#define NORMAL 0
#define INVERTED 1

#define EXTERNAL 0
#define LOCAL 1
#define ALWAYS_ON 2
#define ALWAYS_OFF 3


typedef struct {

	char *name;

	uint8_t enable_HID;
	uint8_t HID_keylayout;

	uint8_t buttons_mode[8];

	uint8_t buttons_inverse[8];

	uint8_t leds_inverse[8];

	uint8_t leds_control[8];


	uint8_t enable_NET;
	uint8_t enable_DHCP;
	char *IP;
	char *netMask;
	char *gateWay;

	uint8_t enable_UDP;
	char *UDP_IP_server;
	uint16_t UDP_port_server;
	uint16_t UDP_port_my;

	uint8_t enable_OSC;
	char *OSC_IP_server;
	uint16_t OSC_port_server;
	uint16_t OSC_port_my;

	uint8_t enable_MQTT;
	char *IP_mqtt_broker;
	uint16_t port_mqtt_broker;
	char *login_mqtt_broker;
	char *pass_mqtt_broker;

} configuration;

int configReader(void *user, const char *section, const char *name, const char *value);
void set_default_config(void);
void write_default_config(void);
void writeErrorTxt(char *buff);
