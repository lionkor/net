#include "lk/net/Socket.h"

#include <exception>
#include <iostream>
#include <stdexcept>

using namespace lk;
using namespace lk::net;

#ifdef LK_NET_WINSOCK
#endif // LK_NET_WINSOCK

// needed in order to initialize WSA
static void ensure_subsystems_initialized() {
#ifdef LK_NET_WINSOCK
    WSADATA data;
    if (WSAStartup(514, &data)) {
        throw std::runtime_error("WSAStartup: " + get_api_error());
    }
#endif // LK_NET_WINSOCK
}

Socket::Socket(detail::SockType type, detail::SockProtocol protocol) {
    ensure_subsystems_initialized();
    m_sock.fd = ::socket(AF_INET6, int(type), int(protocol));
    if (m_sock.fd < 0) {
        throw std::runtime_error("socket: " + get_api_error());
    }
#ifdef LK_NET_WINSOCK
    const char optval = 0;
    int ret = ::setsockopt(m_sock.fd, SOL_SOCKET, SO_DONTLINGER, &optval, sizeof(optval));
#else // POSIX
    int optval = true;
    int ret = ::setsockopt(m_sock.fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<void*>(&optval), sizeof(optval));
#endif
    if (ret < 0) {
        throw std::runtime_error("setsockopt reuse port: " + get_api_error());
    }
}

Socket::~Socket() {
    if (m_sock.fd > 0) {
#ifdef LK_NET_WINSOCK
        ::shutdown(m_sock.fd, SD_BOTH);
        ::closesocket(m_sock.fd);
#else
        ::shutdown(m_sock.fd, SHUT_RDWR);
        ::close(m_sock.fd);
#endif
    }
}

void Socket::bind(uint16_t port, const std::string& addr) {
    auto final_addr = detail::ensure_ipv6_format(addr);
    auto server = detail::create_sockaddr(final_addr, port);
    m_port = port;
    m_address = final_addr;
    if (::bind(m_sock.fd, reinterpret_cast<struct sockaddr*>(&server), sizeof(server)) < 0) {
        throw std::runtime_error("bind: " + get_api_error());
    }
}

void Socket::connect(const std::string& addr, uint16_t port) {
    auto final_addr = detail::ensure_ipv6_format(addr);
    auto client = detail::create_sockaddr(final_addr, port);
    m_port = port;
    m_address = final_addr;
    if (::connect(m_sock.fd, reinterpret_cast<struct sockaddr*>(&client), sizeof(client)) < 0) {
        throw std::runtime_error("connect: " + get_api_error());
    }
}

void Socket::listen(int backlog) {
    int ret = ::listen(m_sock.fd, backlog);
    if (ret < 0) {
        throw std::runtime_error("listen: " + get_api_error());
    }
}

Socket Socket::accept() {
    Socket sock;
    struct sockaddr_in6 addr;
    socklen_t len;
    sock.m_sock.fd = ::accept(m_sock.fd, reinterpret_cast<struct sockaddr*>(&addr), &len);
    if (sock.m_sock.fd < 0) {
        throw std::runtime_error("accept: " + get_api_error());
    }
    char buf[INET6_ADDRSTRLEN];
    const char* ret = inet_ntop(AF_INET6, reinterpret_cast<const void*>(&addr.sin6_addr), buf, len);
    if (ret) {
        sock.m_address = ret;
        sock.m_port = ntohs(addr.sin6_port);
    }
    return sock;
}

void Socket::set_read_timeout(size_t ms) {
#ifdef LK_NET_WINSOCK
    int ret = ::setsockopt(m_sock.fd, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&ms), sizeof(ms));
#else // POSIX
    struct timeval optval;
    optval.tv_sec = (int)(ms / 1000);
    optval.tv_usec = (ms % 1000) * 1000;
    int ret = ::setsockopt(m_sock.fd, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const void*>(&optval), sizeof(optval));
#endif
    if (ret < 0) {
        throw std::runtime_error("setsockopt recv timeout: " + get_api_error());
    }
}

void Socket::set_write_timeout(size_t ms) {
#ifdef LK_NET_WINSOCK
    int ret = ::setsockopt(m_sock.fd, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&ms), sizeof(ms));
#else // POSIX
    struct timeval optval;
    optval.tv_sec = (int)(ms / 1000);
    optval.tv_usec = (ms % 1000) * 1000;
    int ret = ::setsockopt(m_sock.fd, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<void*>(&optval), sizeof(optval));
#endif
    if (ret < 0) {
        throw std::runtime_error("setsockopt recv timeout: " + get_api_error());
    }
}

Socket::Socket() { }

TCPSocket::TCPSocket()
    : Socket(detail::SockType::STREAM, detail::SockProtocol::TCP) {
}

std::array<std::byte, sizeof(uint32_t)> TCPSocket::make_size_header(uint32_t n) {
    uint32_t ordered = htonl(uint32_t(n));
    std::array<std::byte, sizeof(uint32_t)> array;
    std::memcpy(array.data(), &ordered, sizeof(ordered));
    return array;
}

UDPSocket::UDPSocket()
    : Socket(detail::SockType::DGRAM, detail::SockProtocol::UDP) {
}

std::string detail::ensure_ipv6_format(const std::string& addr) {
    std::string final_addr;
    if (std::count(addr.begin(), addr.end(), '.') == 3
        && addr.substr(0, 2) != "::") {
        // assume it's a.b.c.d format (ipv4), in which case we explicitly map it
        final_addr = "::FFFF:" + addr;
    } else {
        final_addr = addr;
    }
    return final_addr;
}

struct sockaddr_in6 detail::create_sockaddr(const std::string& ipv6, uint16_t port) {
    struct sockaddr_in6 addr;
    addr.sin6_family = AF_INET6;
    addr.sin6_port = ::htons(port);
    int ret = inet_pton(AF_INET6, ipv6.c_str(), reinterpret_cast<void*>(&addr.sin6_addr));
    if (ret < 0) {
        throw std::runtime_error("inet_pton: " + get_api_error());
    } else if (ret == 0) {
        throw std::runtime_error("inet_pton: not a valid IPv6 network address");
    }
    return addr;
}
