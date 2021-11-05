#include "Socket.h"
#include <iostream>

int main() {
    lk::net::TCPSocket socket;
    socket.bind(6699);
    socket.listen(10);
    auto client = socket.accept();
    std::string msg = "Hello, World!";
    auto n = client.write(msg);
    if (size_t(n) != msg.size()) {
        // handle
    }
    std::string buf(64, ' ');
    n = client.read(buf);
    buf[n] = '\0';
    std::cout << "client said: " << buf << "\n";
}
