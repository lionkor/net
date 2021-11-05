#pragma once

#if defined(WIN32) || defined(__MINGW32__) || defined(_MSC_VER)
#define LK_NET_WINSOCK
#include <conio.h>
#include <winsock.h>
#else // __APPLE__, __linux__
#define LK_NET_POSIX
#include <arpa/inet.h>
#include <errno.h>
#include <sys/socket.h>
#include <termios.h>
#include <unistd.h>
#endif

#include <cerrno>
#include <cstring>
#include <string>

static inline std::string get_api_error() {
#ifdef LK_NET_WINSOCK
    // This will provide us with the error code and an error message, all in one.
    int err;
    char msgbuf[256];
    msgbuf[0] = '\0';

    err = WSAGetLastError();

    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        msgbuf,
        sizeof(msgbuf),
        nullptr);

    if (*msgbuf) {
        return std::to_string(WSAGetLastError()) + " - " + std::string(msgbuf);
    } else {
        return std::to_string(WSAGetLastError());
    }
#else // posix
    return std::strerror(errno);
#endif
}
