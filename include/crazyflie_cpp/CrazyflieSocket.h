#pragma once

#include <stdint.h>
#include <vector>
#include <string>

#include "ITransport.h"

#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>

class CrazyflieSocket
  : public ITransport
{
public:
  CrazyflieSocket();

  ~CrazyflieSocket();

  void setSocketLinkDest(
    uint16_t port, 
    const char* address_src);

  virtual void sendPacket(
    const uint8_t* data,
    uint32_t length,
    ITransport::Ack& result);

  virtual void sendPacketNoAck(
    const uint8_t* data,
    uint32_t length);

private:
  struct sockaddr_in m_myaddr;
  struct sockaddr_in m_srcaddr;
  socklen_t m_addrlen;
  int m_fd;
  struct pollfd m_fds[1];

  // Check if destination address has been set
  bool m_address_set;

};
