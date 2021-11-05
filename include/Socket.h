#pragma once

#include <concepts>
#include <functional>
#include <string>

#include "Compat.h"

namespace lk::net {

namespace detail {
    struct OSSocket final {
        int fd;
    };
    enum class SockType : int {
        STREAM = SOCK_STREAM,
        DGRAM = SOCK_DGRAM,
    };
    enum class SockProtocol : int {
        TCP = IPPROTO_TCP,
        UDP = IPPROTO_UDP,
    };

    std::string ensure_ipv6_format(const std::string& any_addr);
    struct ::sockaddr_in6 create_sockaddr(const std::string& ipv6, uint16_t port);

    template<typename T>
    concept DataSizeAccessible = requires(T a) {
        { a.data() } -> std::convertible_to<const typename T::value_type*>;
        { a.size() } -> std::convertible_to<size_t>;
    };

    template<typename T>
    concept DataSizeMutableAccessible = requires(T a) {
        { a.data() } -> std::convertible_to<typename T::value_type*>;
        { a.size() } -> std::convertible_to<size_t>;
    };
}

// dont use directly
class Socket {
public:
    Socket(detail::SockType type, detail::SockProtocol protocol);
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket(Socket&&) = default;
    Socket& operator=(Socket&&) = default;
    virtual ~Socket();

    void bind(uint16_t port, const std::string& addr = "::");
    void connect(const std::string& addr, uint16_t port);
    void listen(int backlog);
    [[nodiscard]] Socket accept();

    // only set if bind() or connect() was called on the socket, or if the socket was returned from accept()
    std::string address() const { return m_address; }
    // only set if bind() or connect() was called on the socket, or if the socket was returned from accept()
    uint16_t port() const { return m_port; }

    template<typename T>
    [[nodiscard]] int64_t write(const T* buf, size_t count);
    template<typename T>
    [[nodiscard]] int64_t read(T* buf, size_t count);
    template<detail::DataSizeAccessible ContainerT>
    [[nodiscard]] int64_t write(const ContainerT& container);
    template<detail::DataSizeMutableAccessible ContainerT>
    [[nodiscard]] int64_t read(ContainerT& container);

protected:
    Socket();
    detail::OSSocket m_sock {};

private:
    std::string m_address {};
    uint16_t m_port {};
};

template<detail::DataSizeMutableAccessible ContainerT>
int64_t Socket::read(ContainerT& container) {
    return read(container.data(), container.size() * sizeof(typename ContainerT::value_type));
}

template<typename T>
int64_t Socket::write(const T* buf, size_t count) {
#ifdef LK_NET_WINSOCK
    return int64_t(::send(m_sock.fd, reinterpret_cast<const char*>(buf), count * sizeof(T), 0));
#else // POSIX
    return int64_t(::send(m_sock.fd, reinterpret_cast<const void*>(buf), count * sizeof(T), 0));
#endif
}

template<typename T>
int64_t Socket::read(T* buf, size_t count) {
#ifdef LK_NET_WINSOCK
    return int64_t(::recv(m_sock.fd, reinterpret_cast<char*>(buf), count * sizeof(T), 0));
#else // POSIX
    return int64_t(::recv(m_sock.fd, reinterpret_cast<void*>(buf), count * sizeof(T), 0));
#endif
}

template<detail::DataSizeAccessible ContainerT>
int64_t Socket::write(const ContainerT& container) {
    return write(container.data(), container.size() * sizeof(typename ContainerT::value_type));
}

class TCPSocket : public Socket {
public:
    TCPSocket();
};

class UDPSocket : public Socket {
public:
    UDPSocket();
};

} // namespace lk::net
