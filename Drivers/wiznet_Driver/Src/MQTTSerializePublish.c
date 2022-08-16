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
 *    Ian Craggs - fix for https://bugs.eclipse.org/bugs/show_bug.cgi?id=453144
 *******************************************************************************/

#include <string.h>
#include "../Inc/MQTTPacket.h"
#include "../Inc/StackTrace.h"


/**
  * Determines the length of the MQTT publish packet that would be produced using the supplied parameters
  * @param qos the MQTT QoS of the publish (packetid is omitted for QoS 0)
  * @param topicName the topic name to be used in the publish  
  * @param payloadlen the length of the payload to be sent
  * @return the length of buffer needed to contain the serialized version of the packet
  */
int32_t MQTTSerialize_publishLength(uint8_t qos, MQTTString topicName, int32_t payloadlen)
{
  int32_t len = 2 + MQTTstrlen(topicName) + payloadlen;
  
  if (qos > 0)
    len += 2; /* packetid */
	
  return len;
}


/**
  * Serializes the supplied publish data into the supplied buffer, ready for sending
  * @param buf the buffer into which the packet will be serialized
  * @param buflen the length in bytes of the supplied buffer
  * @param dup integer - the MQTT dup flag
  * @param qos integer - the MQTT QoS value
  * @param retained integer - the MQTT retained flag
  * @param packetid integer - the MQTT packet identifier
  * @param topicName MQTTString - the MQTT topic in the publish
  * @param payload byte buffer - the MQTT publish payload
  * @param payloadlen integer - the length of the MQTT payload
  * @return the length of the serialized data.  <= 0 indicates error
  */
int32_t MQTTSerialize_publish(uint8_t* buf, int32_t buflen, uint8_t dup, uint8_t qos, uint8_t retained, uint16_t packetid,
		MQTTString topicName, uint8_t* payload, int32_t payloadlen)
{
  uint8_t *ptr = buf;
  MQTTHeader header = {0};
  int32_t rem_len = 0;
  int32_t rc = 0;

  FUNC_ENTRY;

  if (MQTTPacket_len(rem_len = MQTTSerialize_publishLength(qos, topicName, payloadlen)) > buflen)
  {
    rc = MQTTPACKET_BUFFER_TOO_SHORT;
    goto exit;
  }

  header.bits.type = PUBLISH;
  header.bits.dup = dup;
  header.bits.qos = qos;
  header.bits.retain = retained;
  writeChar(&ptr, header.byte); /* write header */

  ptr += MQTTPacket_encode(ptr, rem_len); /* write remaining length */;

  writeMQTTString(&ptr, topicName);

  if (qos > 0)
    writeInt(&ptr, packetid);

  memcpy(ptr, payload, payloadlen);
  ptr += payloadlen;

  rc = ptr - buf;
exit:
  FUNC_EXIT_RC(rc);
  return rc;
}



/**
  * Serializes the ack packet into the supplied buffer.
  * @param buf the buffer into which the packet will be serialized
  * @param buflen the length in bytes of the supplied buffer
  * @param type the MQTT packet type
  * @param dup the MQTT dup flag
  * @param packetid the MQTT packet identifier
  * @return serialized length, or error if 0
  */
int32_t MQTTSerialize_ack(uint8_t* buf, int32_t buflen, uint8_t packettype, uint8_t dup, uint16_t packetid)
{
  MQTTHeader header = {0};
  int32_t rc = 0;
  uint8_t *ptr = buf;

  FUNC_ENTRY;

  if (buflen < 4)
  {
    rc = MQTTPACKET_BUFFER_TOO_SHORT;
    goto exit;
  }

  header.bits.type = packettype;
  header.bits.dup = dup;
  header.bits.qos = (packettype == PUBREL) ? 1 : 0;
  writeChar(&ptr, header.byte); /* write header */

  ptr += MQTTPacket_encode(ptr, 2); /* write remaining length */
  writeInt(&ptr, packetid);
  rc = ptr - buf;
exit:
  FUNC_EXIT_RC(rc);
  return rc;
}


/**
  * Serializes a puback packet into the supplied buffer.
  * @param buf the buffer into which the packet will be serialized
  * @param buflen the length in bytes of the supplied buffer
  * @param packetid integer - the MQTT packet identifier
  * @return serialized length, or error if 0
  */
int32_t MQTTSerialize_puback(uint8_t* buf, int32_t buflen, uint16_t packetid)
{
  return MQTTSerialize_ack(buf, buflen, PUBACK, 0, packetid);
}


/**
  * Serializes a pubrel packet into the supplied buffer.
  * @param buf the buffer into which the packet will be serialized
  * @param buflen the length in bytes of the supplied buffer
  * @param dup integer - the MQTT dup flag
  * @param packetid integer - the MQTT packet identifier
  * @return serialized length, or error if 0
  */
int32_t MQTTSerialize_pubrel(uint8_t* buf, int32_t buflen, uint8_t dup, uint16_t packetid)
{
  return MQTTSerialize_ack(buf, buflen, PUBREL, dup, packetid);
}


/**
  * Serializes a pubrel packet into the supplied buffer.
  * @param buf the buffer into which the packet will be serialized
  * @param buflen the length in bytes of the supplied buffer
  * @param packetid integer - the MQTT packet identifier
  * @return serialized length, or error if 0
  */
int32_t MQTTSerialize_pubcomp(uint8_t* buf, int32_t buflen, uint16_t packetid)
{
  return MQTTSerialize_ack(buf, buflen, PUBCOMP, 0, packetid);
}


