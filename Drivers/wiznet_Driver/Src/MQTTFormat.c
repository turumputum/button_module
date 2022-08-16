/*
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *******************************************************************************/

#include <string.h>
#include "../Inc/MQTTPacket.h"
#include "../Inc/StackTrace.h"

const char *MQTTPacket_names[] = { "RESERVED", "CONNECT", "CONNACK", "PUBLISH", "PUBACK", "PUBREC", "PUBREL", "PUBCOMP", "SUBSCRIBE", "SUBACK", "UNSUBSCRIBE",
		"UNSUBACK", "PINGREQ", "PINGRESP", "DISCONNECT" };

const char* MQTTPacket_getName(uint16_t packetid) {
	return MQTTPacket_names[packetid];
}

int32_t MQTTStringFormat_connect(char *strbuf, int32_t strbuflen, MQTTPacket_connectData *data) {
	int32_t strindex = 0;

	strindex = snprintf(strbuf, strbuflen, "CONNECT MQTT version %ld, client id %.*s, clean session %ld, keep alive %d", (int32_t) data->MQTTVersion,
			(int) data->clientID.lenstring.len, (char*) data->clientID.lenstring.data, (int32_t) data->cleansession, data->keepAliveInterval);
	if (data->willFlag)
		strindex += snprintf(&strbuf[strindex], strbuflen - strindex, ", will QoS %d, will retain %d, will topic %.*s, will message %.*s", data->will.qos,
				data->will.retained, (int) data->will.topicName.lenstring.len, (char*) data->will.topicName.lenstring.data,
				(int) data->will.message.lenstring.len, (char*) data->will.message.lenstring.data);
	if (data->username.lenstring.data && data->username.lenstring.len > 0)
		strindex += snprintf(&strbuf[strindex], strbuflen - strindex, ", user name %.*s", (int) data->username.lenstring.len,
				(char *) data->username.lenstring.data);
	if ((data->password.lenstring.data) && (data->password.lenstring.len > 0))
		strindex += snprintf(&strbuf[strindex], strbuflen - strindex, ", password %.*s", (int)data->password.lenstring.len, (char *)data->password.lenstring.data);
	return strindex;
}

int32_t MQTTStringFormat_connack(char *strbuf, int32_t strbuflen, uint8_t connack_rc, uint8_t sessionPresent) {
	int32_t strindex = snprintf(strbuf, strbuflen, "CONNACK session present %d, rc %d", sessionPresent, connack_rc);
	return strindex;
}

int32_t MQTTStringFormat_publish(char *strbuf, int32_t strbuflen, uint8_t dup, uint8_t qos, uint8_t retained, uint16_t packetid, MQTTString topicName,
		uint8_t *payload, int32_t payloadlen) {
	int32_t strindex = snprintf(strbuf, strbuflen, "PUBLISH dup %d, QoS %d, retained %d, packet id %d, topic %.*s, payload length %ld, payload %.*s", dup, qos,
			retained, packetid, (topicName.lenstring.len < 20) ? (int)topicName.lenstring.len : 20, (char *)topicName.lenstring.data, payloadlen,
			(payloadlen < 20) ? (int)payloadlen : 20, (char *)payload);
	return strindex;
}

int32_t MQTTStringFormat_ack(char *strbuf, int32_t strbuflen, uint8_t packettype, uint8_t dup, uint16_t packetid) {
	int32_t strindex = snprintf(strbuf, strbuflen, "%s, packet id %d", MQTTPacket_names[packettype], packetid);

	if (dup)
		strindex += snprintf(strbuf + strindex, strbuflen - strindex, ", dup %d", dup);

	return strindex;
}

int32_t MQTTStringFormat_subscribe(char *strbuf, int32_t strbuflen, uint8_t dup, uint16_t packetid, int32_t count, MQTTString topicFilters[],
		int32_t requestedQoSs[]) {
	return snprintf(strbuf, strbuflen, "SUBSCRIBE dup %d, packet id %d count %ld topic %.*s qos %d", dup, packetid, count, (int)topicFilters[0].lenstring.len,
			topicFilters[0].lenstring.data, (int)requestedQoSs[0]);
}

int32_t MQTTStringFormat_suback(char *strbuf, int32_t strbuflen, uint16_t packetid, int32_t count, int32_t *grantedQoSs) {
	return snprintf(strbuf, strbuflen, "SUBACK packet id %d count %ld granted qos %d", packetid, count, (int)grantedQoSs[0]);
}

int32_t MQTTStringFormat_unsubscribe(char *strbuf, int32_t strbuflen, uint8_t dup, uint16_t packetid, int32_t count, MQTTString topicFilters[]) {
	return snprintf(strbuf, strbuflen, "UNSUBSCRIBE dup %d, packet id %d count %ld topic %.*s", dup, packetid, count, (int)topicFilters[0].lenstring.len,
			(char *)topicFilters[0].lenstring.data);
}

char* MQTTFormat_toClientString(char *strbuf, int32_t strbuflen, uint8_t *buf, int32_t buflen) {
	int32_t index = 0;
	int32_t rem_length = 0;
	MQTTHeader header = { 0 };

	header.byte = buf[index++];
	index += MQTTPacket_decodeBuf(&buf[index], &rem_length);

	switch (header.bits.type) {
	case CONNACK: {
		uint8_t sessionPresent, connack_rc;
		if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, buf, buflen) == 1)
			MQTTStringFormat_connack(strbuf, strbuflen, connack_rc, sessionPresent);
	}
		break;
	case PUBLISH: {
		uint8_t dup, retained, *payload;
		uint16_t packetid;
		uint8_t qos;
		int32_t payloadlen;
		MQTTString topicName = MQTTString_initializer;

		if (MQTTDeserialize_publish(&dup, &qos, &retained, &packetid, &topicName, &payload, &payloadlen, buf, buflen) == 1)
			MQTTStringFormat_publish(strbuf, strbuflen, dup, qos, retained, packetid, topicName, payload, payloadlen);
	}
		break;
	case PUBACK:
	case PUBREC:
	case PUBREL:
	case PUBCOMP: {
		uint8_t packettype, dup;
		uint16_t packetid;
		if (MQTTDeserialize_ack(&packettype, &dup, &packetid, buf, buflen) == 1)
			MQTTStringFormat_ack(strbuf, strbuflen, packettype, dup, packetid);
	}
		break;
	case SUBACK: {
		uint16_t packetid;
		int32_t maxcount = 1, count = 0;
		int32_t grantedQoSs[1];
		if (MQTTDeserialize_suback(&packetid, maxcount, &count, grantedQoSs, buf, buflen) == 1)
			MQTTStringFormat_suback(strbuf, strbuflen, packetid, count, grantedQoSs);
	}
		break;
	case UNSUBACK: {
		uint16_t packetid;
		if (MQTTDeserialize_unsuback(&packetid, buf, buflen) == 1)
			MQTTStringFormat_ack(strbuf, strbuflen, UNSUBACK, 0, packetid);
	}
		break;
	case PINGREQ:
	case PINGRESP:
	case DISCONNECT:
		snprintf(strbuf, strbuflen, "%s", MQTTPacket_names[header.bits.type]);
		break;
	}

	return strbuf;
}

char* MQTTFormat_toServerString(char *strbuf, int32_t strbuflen, uint8_t *buf, int32_t buflen) {
	int32_t index = 0;
	int32_t rem_length = 0;
	MQTTHeader header = { 0 };

	header.byte = buf[index++];
	index += MQTTPacket_decodeBuf(&buf[index], &rem_length);

	switch (header.bits.type) {
	case CONNECT: {
		MQTTPacket_connectData data;
		int32_t rc;

		if ((rc = MQTTDeserialize_connect(&data, buf, buflen)) == 1)
			MQTTStringFormat_connect(strbuf, strbuflen, &data);
	}
		break;
	case PUBLISH: {
		uint8_t dup, retained, *payload;
		uint16_t packetid;
		uint8_t qos;
		int32_t payloadlen;
		MQTTString topicName = MQTTString_initializer;

		if (MQTTDeserialize_publish(&dup, &qos, &retained, &packetid, &topicName, &payload, &payloadlen, buf, buflen) == 1)
			MQTTStringFormat_publish(strbuf, strbuflen, dup, qos, retained, packetid, topicName, payload, payloadlen);
	}
		break;
	case PUBACK:
	case PUBREC:
	case PUBREL:
	case PUBCOMP: {
		uint8_t packettype, dup;
		uint16_t packetid;

		if (MQTTDeserialize_ack(&packettype, &dup, &packetid, buf, buflen) == 1)
			MQTTStringFormat_ack(strbuf, strbuflen, packettype, dup, packetid);
	}
		break;
	case SUBSCRIBE: {
		uint8_t dup;
		uint16_t packetid;
		int32_t maxcount = 1, count = 0;
		MQTTString topicFilters[1];
		int32_t requestedQoSs[1];

		if (MQTTDeserialize_subscribe(&dup, &packetid, maxcount, &count, topicFilters, requestedQoSs, buf, buflen) == 1)
			MQTTStringFormat_subscribe(strbuf, strbuflen, dup, packetid, count, topicFilters, requestedQoSs);
		;
	}
		break;
	case UNSUBSCRIBE: {
		uint8_t dup;
		uint16_t packetid;
		int32_t maxcount = 1, count = 0;
		MQTTString topicFilters[1];
		if (MQTTDeserialize_unsubscribe(&dup, &packetid, maxcount, &count, topicFilters, buf, buflen) == 1)
			MQTTStringFormat_unsubscribe(strbuf, strbuflen, dup, packetid, count, topicFilters);
	}
		break;
	case PINGREQ:
	case PINGRESP:
	case DISCONNECT:
		snprintf(strbuf, strbuflen, "%s", MQTTPacket_names[header.bits.type]);
		break;
	}

	strbuf[strbuflen] = '\0';
	return strbuf;
}
