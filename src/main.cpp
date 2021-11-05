#include "Socket.h"
#include <iostream>

int main() {
    lk::net::TCPSocket socket;
    socket.bind(6699);
    socket.listen(10);
    std::cout << "listening on [" << socket.address() << "]:" << socket.port() << std::endl;
    for (;;) {
        auto client = socket.accept();
        std::cout << "client connected: [" << client.address() << "]:" << client.port() << std::endl;
        client.set_read_timeout(5000);
        for (;;) {
            try {
                std::vector<char> buf;
                buf.resize(1024, '\0');
                auto rn = client.read(buf);
                if (rn < 0) {
                    std::cout << "client disconnected: " << lk::get_api_error() << std::endl;
                    break;
                }
                auto wn = client.write(buf.data(), rn);
                if (rn != wn) {
                    std::cout << "failed to write all data" << std::endl;
                    break;
                }
            } catch (const std::runtime_error& e) {
                std::cout << e.what() << std::endl;
                break;
            }
        }
    }
}
