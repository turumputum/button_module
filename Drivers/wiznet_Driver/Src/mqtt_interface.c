#include "mqtt_interface.h"
#include "main.h"
#include "socket.h"


uint32_t MilliTimer;

void MilliTimer_Handler(void) 
{
  MilliTimer++;
}

int8_t expired(Timer* timer) 
{
  int32_t left = timer->end_time - MilliTimer;
  return (left < 0);
}

void countdown_ms(Timer* timer, uint32_t timeout)
{
  timer->end_time = MilliTimer + timeout;
}

void countdown(Timer* timer, uint32_t timeout)
{
  timer->end_time = MilliTimer + (timeout * 1000);
}

int32_t left_ms(Timer* timer)
{
  int32_t left = timer->end_time - MilliTimer;
  return (left < 0) ? 0 : left;
}

void InitTimer(Timer* timer)
{
  timer->end_time = 0;
}


void NewNetwork(Network* n)
{
  n->my_socket = 0;
  n->mqttread = w5500_read;
  n->mqttwrite = w5500_write;
  n->disconnect = w5500_disconnect;
}

int32_t w5500_read(Network* n, uint8_t* buffer, int32_t len, int32_t timeout_ms)
{
  if ((getSn_SR(n->my_socket) == SOCK_ESTABLISHED) && (getSn_RX_RSR(n->my_socket) > 0))
    return recv(n->my_socket, buffer, len);
  
  return 0;
}

int32_t w5500_write(Network* n, uint8_t* buffer, int32_t len, int32_t timeout_ms)
{
  if (getSn_SR(n->my_socket) == SOCK_ESTABLISHED)
    return send(n->my_socket, buffer, len);
  
  return 0;
}

void w5500_disconnect(Network* n)
{
  disconnect(n->my_socket);
}

int32_t ConnectNetwork(Network* n, uint8_t* ip, uint16_t port)
{
	int ret;

	close(n->my_socket);
	ret = socket(n->my_socket, Sn_MR_TCP, 1245,0);// SF_TCP_NODELAY | SF_IO_NONBLOCK
  printf("Open socket:  %d \r\n", ret);
  //disconnect(n->my_socket);
  //printf("Disconnect:  %d \r\n", ret);
  ret= connect(n->my_socket, ip, port);
  //uint8_t tip[4] = { 10, 20, 0, 116};
  //ret= connect(0, tip, 1883);
  printf("Connect to Network at: %d:%d:%d:%d  port: %d  result: %d \r\n", ip[0],ip[1], ip[2], ip[3], port, ret);
  return ret;
}


