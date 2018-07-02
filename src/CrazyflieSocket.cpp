#include "CrazyflieSocket.h"

#include <sstream>
#include <stdexcept>

#include <string.h>
#include <errno.h>


CrazyflieSocket::CrazyflieSocket()
    : ITransport()
{
    memset((char *)&m_myaddr, 0, sizeof(m_myaddr));
    m_myaddr.sin_family = AF_INET;
    m_srcaddr.sin_family = AF_INET;

    // Let the OS pick the port and an interface address
    m_myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    m_myaddr.sin_port = htons(0);

    // try to setup udp socket for communcation
    if ((m_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        throw std::runtime_error("Can't create socket ! ");
    }
    // Try to bind the socket
    if (bind(m_fd, (struct sockaddr *)&m_myaddr, sizeof(m_myaddr)) < 0) {
        throw std::runtime_error("Bind failed !");
    }
    m_fds[0].fd = m_fd;
    m_fds[0].events = POLLIN;
    m_address_set = false;
}

CrazyflieSocket::~CrazyflieSocket()
{}

void CrazyflieSocket::setSocketLinkDest(uint16_t port, const char* address_src)
{
    if (strcmp(address_src, "INADDR_ANY") == 0){
        m_srcaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }else if (inet_addr(address_src) == INADDR_NONE){
        throw std::runtime_error("address is invalid !");
    }else {
        m_srcaddr.sin_addr.s_addr = inet_addr(address_src);
    }
    m_srcaddr.sin_port = htons(port);
    m_addrlen = sizeof(m_srcaddr);
    m_address_set = true;
}

void CrazyflieSocket::sendPacket(
    const uint8_t* data,
    uint32_t length,
    Ack& result)
{
    if (!m_address_set){
        throw std::runtime_error("No destination address has been set !");
    }

    int transferred;

    // Send data
    transferred = sendto(m_fd, data, length, 0, (struct sockaddr *)&m_srcaddr, m_addrlen);
    if (transferred <= 0) {
        throw std::runtime_error(strerror(errno));
    }

    if (length != transferred) {
        std::stringstream sstr;
        sstr << "Did transfer " << transferred << " but " << length << " was requested!";
        throw std::runtime_error(sstr.str());
    }

    // Read result
    int result_poll = ::poll(&m_fds[0], (sizeof(m_fds[0]) / sizeof(m_fds[0])), 1000);
    if (result_poll < 0){
        throw std::runtime_error(strerror(errno));
    }else if (result_poll == 0){
        result.ack = false;
        result.size = 0;
        return ;
    }

    if ( ! (m_fds[0].revents & POLLIN) ){
        result.ack = false;
        result.size = 0;
        return;
    }

    transferred = recvfrom(m_fd, (unsigned char *) &result.data[0], sizeof(result) - 2, 0, (struct sockaddr *)&m_srcaddr, &m_addrlen);
    if (transferred <= 0){
        result.ack = false;
        result.size = 0;
        return;
    }
    
    result.ack = true;
    result.size = transferred;
}

void CrazyflieSocket::sendPacketNoAck(
    const uint8_t* data,
    uint32_t length)
{
    if (!m_address_set){
        throw std::runtime_error("No destination address has been set !");
    }

    int transferred;

    // Send data
    transferred = sendto(m_fd, data, length, 0, (struct sockaddr *)&m_srcaddr, m_addrlen);
    if (transferred <= 0) {
        throw std::runtime_error(strerror(errno));
    }

    if (length != transferred) {
        std::stringstream sstr;
        sstr << "Did transfer " << transferred << " but " << length << " was requested!";
        throw std::runtime_error(sstr.str());
    }
}

void CrazyflieSocket::recvPacket(Ack& result)
{
    int result_poll = ::poll(&m_fds[0], (sizeof(m_fds[0]) / sizeof(m_fds[0])), 1000);
    if (result_poll < 0){
        throw std::runtime_error(strerror(errno));
    }else if (result_poll == 0){
        result.ack = false;
        result.size = 0;
        return ;
    }

    if ( ! (m_fds[0].revents & POLLIN) ){
        result.ack = false;
        result.size = 0;
        return;
    }

    int transferred = recvfrom(m_fd, (unsigned char *) &result.data[0], sizeof(result) - 2, 0, (struct sockaddr *)&m_srcaddr, &m_addrlen);
    if (transferred <= 0){
        result.ack = false;
        result.size = 0;
        return;
    }
    
    result.ack = true;
    result.size = transferred;
}