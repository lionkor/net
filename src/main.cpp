#include "Socket.h"
#include <iostream>

int main() {
    lk::net::TCPSocket socket;
    socket.bind(6699);
    socket.listen(10);
    auto client = socket.accept();
    std::string msg = "Hello, World!";
    (void)client.write(msg);
    std::string buf(64, ' ');
    (void)client.read(buf);
    std::cout << "client said: " << buf << "\n";
}
