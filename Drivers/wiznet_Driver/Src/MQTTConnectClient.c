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

/**
  * Determines the length of the MQTT connect packet that would be produced using the supplied connect options.
  * @param options the options to be used to build the connect packet
  * @return the length of buffer needed to contain the serialized version of the packet
  */
int32_t MQTTSerialize_connectLength(MQTTPacket_connectData* options)
{
  int32_t len = 0;

  FUNC_ENTRY;

  if (options->MQTTVersion == 3)
    len = 12; /* variable depending on MQTT or MQIsdp */
  else if (options->MQTTVersion == 4)
    len = 10;

  len += MQTTstrlen(options->clientID) + 2;
  if (options->willFlag)
    len += MQTTstrlen(options->will.topicName) + 2 + MQTTstrlen(options->will.message) + 2;
  if (options->username.cstring || options->username.lenstring.data)
    len += MQTTstrlen(options->username)+2;
  if (options->password.cstring || options->password.lenstring.data)
    len += MQTTstrlen(options->password)+2;

  FUNC_EXIT_RC(len);
  return len;
}


/**
  * Serializes the connect options into the buffer.
  * @param buf the buffer into which the packet will be serialized
  * @param len the length in bytes of the supplied buffer
  * @param options the options to be used to build the connect packet
  * @return serialized length, or error if 0
  */
int32_t MQTTSerialize_connect(uint8_t* buf, int32_t buflen, MQTTPacket_connectData* options)
{
  uint8_t *ptr = buf;
  MQTTHeader header = {0};
  MQTTConnectFlags flags = {0};
  int32_t len = 0;
  int32_t rc = -1;

  FUNC_ENTRY;
  if (MQTTPacket_len(len = MQTTSerialize_connectLength(options)) > buflen)
  {
    rc = MQTTPACKET_BUFFER_TOO_SHORT;
    goto exit;
  }

  header.byte = 0;
  header.bits.type = CONNECT;
  writeChar(&ptr, header.byte); /* write header */

  ptr += MQTTPacket_encode(ptr, len); /* write remaining length */

  if (options->MQTTVersion == 4)
  {
    writeCString(&ptr, "MQTT");
    writeChar(&ptr, (char) 4);
  }
  else
  {
    writeCString(&ptr, "MQIsdp");
    writeChar(&ptr, (char) 3);
  }

  flags.all = 0;
  flags.bits.cleansession = options->cleansession;
  flags.bits.will = (options->willFlag) ? 1 : 0;

  if (flags.bits.will)
  {
    flags.bits.willQoS = options->will.qos;
    flags.bits.willRetain = options->will.retained;
  }

  if (options->username.cstring || options->username.lenstring.data)
    flags.bits.username = 1;
  if (options->password.cstring || options->password.lenstring.data)
    flags.bits.password = 1;

  writeChar(&ptr, flags.all);
  writeInt(&ptr, options->keepAliveInterval);
  writeMQTTString(&ptr, options->clientID);

  if (options->willFlag)
  {
    writeMQTTString(&ptr, options->will.topicName);
    writeMQTTString(&ptr, options->will.message);
  }

  if (flags.bits.username)
    writeMQTTString(&ptr, options->username);
  if (flags.bits.password)
    writeMQTTString(&ptr, options->password);

  rc = ptr - buf;

  exit: FUNC_EXIT_RC(rc);
  return rc;
}


/**
  * Deserializes the supplied (wire) buffer into connack data - return code
  * @param sessionPresent the session present flag returned (only for MQTT 3.1.1)
  * @param connack_rc returned integer value of the connack return code
  * @param buf the raw buffer data, of the correct length determined by the remaining length field
  * @param len the length in bytes of the data in the supplied buffer
  * @return error code.  1 is success, 0 is failure
  */
int32_t MQTTDeserialize_connack(uint8_t* sessionPresent, uint8_t* connack_rc, uint8_t* buf, int32_t buflen)
{
  MQTTHeader header = {0};
  uint8_t* curdata = buf;
  uint8_t* enddata = NULL;
  int32_t rc = 0;
  int32_t mylen;
  MQTTConnackFlags flags = {0};

  FUNC_ENTRY;
  header.byte = readChar(&curdata);
  if (header.bits.type != CONNACK)
    goto exit;

  curdata += (rc = MQTTPacket_decodeBuf(curdata, &mylen)); /* read remaining length */
  enddata = curdata + mylen;
  if (enddata - curdata < 2)
    goto exit;

  flags.all = readChar(&curdata);
  *sessionPresent = flags.bits.sessionpresent;
  *connack_rc = readChar(&curdata);

  rc = 1;
exit:
  FUNC_EXIT_RC(rc);
  return rc;
}


/**
  * Serializes a 0-length packet into the supplied buffer, ready for writing to a socket
  * @param buf the buffer into which the packet will be serialized
  * @param buflen the length in bytes of the supplied buffer, to avoid overruns
  * @param packettype the message type
  * @return serialized length, or error if 0
  */
int32_t MQTTSerialize_zero(uint8_t* buf, int32_t buflen, uint8_t packettype)
{
  MQTTHeader header = {0};
  int32_t rc = -1;
  uint8_t *ptr = buf;

  FUNC_ENTRY;
  if (buflen < 2)
  {
    rc = MQTTPACKET_BUFFER_TOO_SHORT;
    goto exit;
  }
  header.byte = 0;
  header.bits.type = packettype;
  writeChar(&ptr, header.byte); /* write header */

  ptr += MQTTPacket_encode(ptr, 0); /* write remaining length */
  rc = ptr - buf;
exit:
  FUNC_EXIT_RC(rc);
  return rc;
}


/**
  * Serializes a disconnect packet into the supplied buffer, ready for writing to a socket
  * @param buf the buffer into which the packet will be serialized
  * @param buflen the length in bytes of the supplied buffer, to avoid overruns
  * @return serialized length, or error if 0
  */
int32_t MQTTSerialize_disconnect(uint8_t* buf, int32_t buflen)
{
  return MQTTSerialize_zero(buf, buflen, DISCONNECT);
}


/**
  * Serializes a disconnect packet into the supplied buffer, ready for writing to a socket
  * @param buf the buffer into which the packet will be serialized
  * @param buflen the length in bytes of the supplied buffer, to avoid overruns
  * @return serialized length, or error if 0
  */
int32_t MQTTSerialize_pingreq(uint8_t* buf, int32_t buflen)
{
  return MQTTSerialize_zero(buf, buflen, PINGREQ);
}
