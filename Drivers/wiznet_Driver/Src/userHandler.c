/**
 * @file	userHandler.c
 * @brief	User Control Example
 * @version 1.0
 * @date	2014/07/15
 * @par Revision
 *			2014/07/15 - 1.0 Release
 * @author	
 * \n\n @par Copyright (C) 1998 - 2014 WIZnet. All rights reserved.
 */

#include "../Inc/userHandler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "board.h"
//#include "httpUtil.h"

#include "../Inc/wizchip_conf.h"

// Pre-defined Set CGI functions
int8_t set_diodir(uint8_t *uri);
int8_t set_diostate(uint8_t *uri);

extern char *NAME;
extern uint8_t udpSrvIP[4];
extern int udpSrvPort;
extern int val;

uint8_t predefined_get_cgi_processor(uint8_t *uri_name, uint8_t *buf,
		uint16_t *len) {
	uint8_t ret = 1;	// ret = 1 means 'uri_name' matched


	if (strcmp((const char*) uri_name, "todo.cgi") == 0) {

	} else if (strcmp((const char*) uri_name, "get_netinfo.cgi") == 0) {
		make_json_netinfo(buf, len);
	}

	return ret;
}

uint8_t predefined_set_cgi_processor(uint8_t *uri_name, uint8_t *uri,
		uint8_t *buf, uint16_t *len) {
	uint8_t ret = 1;	// ret = '1' means 'uri_name' matched
	//uint8_t val = 0;

	if (strcmp((const char*) uri_name, "todo.cgi") == 0) {
		// to do
		;//val = todo(uri);
		 //*len = sprintf((char *)buf, "%d", val);
	} else if (strcmp((const char*) uri_name, "/set_netinfo.cgi") == 0) {

		//parce input data

		wiz_NetInfo netinfo;

		char *reserve;
		char *str = (char*) uri;

		char *tmpAdrStr = strstr(str, "MAC");
		tmpAdrStr += 4;
		if (sscanf((char*) tmpAdrStr, "%x:%x:%x:%x:%x:%x", (unsigned int *)&netinfo.mac[0],
				(unsigned int *)&netinfo.mac[1], (unsigned int *)&netinfo.mac[2], (unsigned int *)&netinfo.mac[3],
				(unsigned int *)&netinfo.mac[4], (unsigned int *)&netinfo.mac[5]) != 6) {

			printf("error parse input MAC\r\n");
		}

		tmpAdrStr = strstr(str, "IP");
		tmpAdrStr += 3;
		if (sscanf(tmpAdrStr, "%d.%d.%d.%d", (int *)&netinfo.ip[0], (int *)&netinfo.ip[1],
				(int *)&netinfo.ip[2], (int *)&netinfo.ip[3]) != 4) {
			printf("error parse input IP\r\n");
		}

		tmpAdrStr = strstr(str, "SN");
		tmpAdrStr += 3;
		if (sscanf(tmpAdrStr, "%d.%d.%d.%d", (int *)&netinfo.sn[0], (int *)&netinfo.sn[1],
				(int *)&netinfo.sn[2], (int *)&netinfo.sn[3]) != 4) {
			printf("error parse input SN\r\n");
		}

		tmpAdrStr = strstr(str, "GW");
		tmpAdrStr += 3;
		if (sscanf(tmpAdrStr, "%d.%d.%d.%d", (int *)&netinfo.gw[0], (int *)&netinfo.gw[1],
				(int *)&netinfo.gw[2], (int *)&netinfo.gw[3]) != 4) {
			printf("error parse input GW\r\n");
		}

		tmpAdrStr = strstr(str, "DNS");
		tmpAdrStr += 4;
		if (sscanf(tmpAdrStr, "%d.%d.%d.%d", (int *)&netinfo.dns[0], (int *)&netinfo.dns[1],
				(int *)&netinfo.dns[2], (int *)&netinfo.dns[3]) != 4) {
			printf("error parse input DNS\r\n");
		}

		tmpAdrStr = strstr(str, "UDP_SRV_IP");
		tmpAdrStr += 11;
		if (sscanf(tmpAdrStr, "%d.%d.%d.%d", (int *)&udpSrvIP[0], (int *)&udpSrvIP[1],
				(int *)&udpSrvIP[2], (int *)&udpSrvIP[3]) != 4) {
			printf("error parse input UDP_SRV_IP\r\n");
		}

		tmpAdrStr = strstr(str, "UDP_SRV_PORT");
		tmpAdrStr += 13;
		if (sscanf(tmpAdrStr, "%d", &udpSrvPort) != 1) {
			printf("error parse input UDP_SRV_PORT\r\n");
		}

		tmpAdrStr = strstr(str, "DHCP");
		tmpAdrStr += 5;
		strtok_r((char*) tmpAdrStr, " ", &reserve);
		if (!strcmp(tmpAdrStr, "false")) {
			netinfo.dhcp = NETINFO_STATIC;
		} else {
			netinfo.dhcp = NETINFO_DHCP;
		}

		tmpAdrStr = strstr(reserve, "NAME");
		tmpAdrStr += 5;
		NAME = strtok_r((char*) tmpAdrStr, " ", &reserve);




		//________setNetInfo

		wizchip_setnetinfo(&netinfo);

		//setToFlash

		netInfo_saveToFlash(&netinfo);

		printf("============== Change NetInfo ==============\r\n");
		printNetInfo();

	} else {
		ret = 0;
	}

	return ret;
}

void printNetInfo(void) {

	wiz_NetInfo netinfo;
	ctlnetwork(CN_GET_NETINFO, (void*) &netinfo);

	printf(
			"NetinfoCallback({\"mac\":\"%.2X:%.2X:%.2X:%.2X:%.2X:%.2X\",\
														\"ip\":\"%d.%d.%d.%d\",\
														\"gw\":\"%d.%d.%d.%d\",\
														\"sn\":\"%d.%d.%d.%d\",\
														\"dns\":\"%d.%d.%d.%d\",\
														\"dhcp\":\"%d\",\
														\"name\":\"%s\"\
														});",
			netinfo.mac[0], netinfo.mac[1], netinfo.mac[2], netinfo.mac[3],
			netinfo.mac[4], netinfo.mac[5], netinfo.ip[0], netinfo.ip[1],
			netinfo.ip[2], netinfo.ip[3], netinfo.gw[0], netinfo.gw[1],
			netinfo.gw[2], netinfo.gw[3], netinfo.sn[0], netinfo.sn[1],
			netinfo.sn[2], netinfo.sn[3], netinfo.dns[0], netinfo.dns[1],
			netinfo.dns[2], netinfo.dns[3], netinfo.dhcp, NAME);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Pre-defined Get CGI functions
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void make_json_netinfo(uint8_t *buf, uint16_t *len) {
	wiz_NetInfo netinfo;
	ctlnetwork(CN_GET_NETINFO, (void*) &netinfo);

	// DHCP: 1 - Static, 2 - DHCP
	*len =
			sprintf((char*) buf,
					"NetinfoCallback({\"mac\":\"%.2X:%.2X:%.2X:%.2X:%.2X:%.2X\",\
											\"ip\":\"%d.%d.%d.%d\",\
											\"gw\":\"%d.%d.%d.%d\",\
											\"sn\":\"%d.%d.%d.%d\",\
											\"dns\":\"%d.%d.%d.%d\",\
											\"dhcp\":\"%d\",\
											\"name\":\"%s\",\
											\"udpSrvIp\":\"%d.%d.%d.%d\",\
											\"udpSrvPort\":\"%d\",\
											});",
					netinfo.mac[0], netinfo.mac[1], netinfo.mac[2],
					netinfo.mac[3], netinfo.mac[4], netinfo.mac[5],
					netinfo.ip[0], netinfo.ip[1], netinfo.ip[2], netinfo.ip[3],
					netinfo.gw[0], netinfo.gw[1], netinfo.gw[2], netinfo.gw[3],
					netinfo.sn[0], netinfo.sn[1], netinfo.sn[2], netinfo.sn[3],
					netinfo.dns[0], netinfo.dns[1], netinfo.dns[2],
					netinfo.dns[3], netinfo.dhcp, NAME,udpSrvIP[0], udpSrvIP[1], udpSrvIP[2], udpSrvIP[3], udpSrvPort);
}

// Write to FLASH
void netInfo_saveToFlash(wiz_NetInfo *netinfo) {
	uint32_t configAddr = ADDR_FLASH_SECTOR_4;

	HAL_FLASH_Unlock();

	__HAL_FLASH_CLEAR_FLAG(
			FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR);

	FLASH_Erase_Sector(FLASH_SECTOR_4, VOLTAGE_RANGE_3);

	for (int i = 0; i < 6; i++) {
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, configAddr, netinfo->mac[i]);
		configAddr++;
	}
	for (int i = 0; i < 4; i++) {
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, configAddr, netinfo->ip[i]);
		configAddr++;
	}
	for (int i = 0; i < 4; i++) {
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, configAddr, netinfo->sn[i]);
		configAddr++;
	}
	for (int i = 0; i < 4; i++) {
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, configAddr, netinfo->gw[i]);
		configAddr++;
	}
	for (int i = 0; i < 4; i++) {
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, configAddr, netinfo->dns[i]);
		configAddr++;
	}

	for (int i = 0; i < 4; i++) {
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, configAddr, udpSrvIP[i]);
		configAddr++;
	}

	uint8_t tmp = (udpSrvPort >> 8) & 0xff;
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, configAddr, tmp);
	configAddr++;

	tmp =  udpSrvPort & 0xff;
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, configAddr, tmp);
	configAddr++;

	HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, configAddr, netinfo->dhcp);
		configAddr++;

		int size = strlen(NAME);
		for (int i = 0; i < size; i++) {
			HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, configAddr, NAME[i]);
			configAddr++;
		}

		HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, configAddr, '\n');


	HAL_FLASH_Lock();

	printf("data writen to flash, go to reset!!!!\n");
	NVIC_SystemReset();
}

void netInfo_readFromFlash(wiz_NetInfo *netinfo) {

	printf("Read netInfo from flash:\n");

	uint8_t *configPtr = (uint8_t*) ADDR_FLASH_SECTOR_4;

	for (int i = 0; i < 6; i++) {
		netinfo->mac[i] = *configPtr++;
	}

	for (int i = 0; i < 4; i++) {
		netinfo->ip[i] = *configPtr++;
	}


	if ((netinfo->ip[0] == 0) || netinfo->ip[0] == 255) {
		printf("chuhloma, load default settings:\n");
		netInfo_reset(netinfo);


	} else {
		for (int i = 0; i < 4; i++) {
			netinfo->sn[i] = *configPtr++;
		}


		for (int i = 0; i < 4; i++) {
			netinfo->gw[i] = *configPtr++;
		}


		for (int i = 0; i < 4; i++) {
			netinfo->dns[i] = *configPtr++;
		}



		for (int i = 0; i < 4; i++) {
			udpSrvIP[i] = *configPtr++;
		}

		udpSrvPort = *configPtr++;

		udpSrvPort = (udpSrvPort <<8);

		udpSrvPort = udpSrvPort|*configPtr++;

		netinfo->dhcp = *configPtr++;

		int t = 0;
		while (*configPtr != '\n') {
			NAME[t] = (char)*configPtr++;
		//	sprintf (NAME, "%c", *configPtr++);
			t++;
			if(t>50){break;}
		}


	}

}

void netInfo_print(wiz_NetInfo *netinfo) {

	printf("__________NET_INFO___________\n");
	printf("mac=%.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", netinfo->mac[0],
			netinfo->mac[1], netinfo->mac[2], netinfo->mac[3], netinfo->mac[4],
			netinfo->mac[5]);

	printf("ip=%d:%d:%d:%d\n", netinfo->ip[0], netinfo->ip[1], netinfo->ip[2],
				netinfo->ip[3]);

	printf("sn=%d:%d:%d:%d\n", netinfo->sn[0], netinfo->sn[1],
					netinfo->sn[2], netinfo->sn[3]);

	printf("gw=%d:%d:%d:%d\n", netinfo->gw[0], netinfo->gw[1],
					netinfo->gw[2], netinfo->gw[3]);

	printf("dns=%d:%d:%d:%d\n", netinfo->dns[0], netinfo->dns[1],
					netinfo->dns[2], netinfo->dns[3]);

	printf("dhcp=%d\n", netinfo->dhcp);
	printf("__________NET_INFO___________\n");

}

void netInfo_reset(wiz_NetInfo *netinfo) {
	netinfo->mac[0] = 0x00;
	netinfo->mac[1] = 0x08;
	netinfo->mac[2] = 0xdc;
	netinfo->mac[3] = 0xcb;
	netinfo->mac[4] = 0xcc;
	netinfo->mac[5] = 0xef;

	netinfo->ip[0] = 192;
	netinfo->ip[1] = 168;
	netinfo->ip[2] = 88;
	netinfo->ip[3] = 10;

	netinfo->sn[0] = 255;
	netinfo->sn[1] = 255;
	netinfo->sn[2] = 255;
	netinfo->sn[3] = 0;

	netinfo->gw[0] = 192;
	netinfo->gw[1] = 168;
	netinfo->gw[2] = 88;
	netinfo->gw[3] = 1;

	netinfo->dns[0] = 0;
	netinfo->dns[1] = 0;
	netinfo->dns[2] = 0;
	netinfo->dns[3] = 0;

	udpSrvIP[0]=255;
	udpSrvIP[1]=255;
	udpSrvIP[2]=255;
	udpSrvIP[3]=255;

	udpSrvPort = 8001;

	netinfo->dhcp = NETINFO_STATIC;
	NAME = "name";


	ctlnetwork(CN_SET_NETINFO, (void*) &netinfo);
	netInfo_saveToFlash(netinfo);
}
