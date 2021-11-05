# net
[![CMake Linux Build](https://github.com/lionkor/net/actions/workflows/cmake-linux.yml/badge.svg)](https://github.com/lionkor/net/actions/workflows/cmake-linux.yml)
[![CMake Windows Build](https://github.com/lionkor/net/actions/workflows/cmake-windows.yml/badge.svg)](https://github.com/lionkor/net/actions/workflows/cmake-windows.yml)

A light and simple cross-platform socket wrapper in C++20.

Here's a minimal TCP server example, which says "Hello, World!", then prints the answer, and exits.
It also demonstrates binding to a specific IPv6 address (:: in this case).

```cpp
#include "Socket.h"
#include <iostream>

int main() {
    lk::net::TCPSocket socket;
    socket.bind(2233, "::");            // bind to localhost:2233
    socket.listen(10);                  // listen with a backlog of 10
    auto client = socket.accept();      // blocking accept
    std::string msg = "Hello, World!"; 
    auto n = client.write(msg);         // write takes any container
    if (size_t(n) != msg.size()) {
        // handle
    }
    std::string buf(64, ' ');
    n = client.read(buf);
    buf[n] = '\0';
    std::cout << "client said: " << buf << "\n";
}   // socket is closed via RAII
```
