#define ASIO_STANDALONE
#include <asio.hpp>
#include <iostream>
#include <string>

int main() {
    asio::io_context io;
    asio::ip::tcp::socket socket(io);
    socket.connect(asio::ip::tcp::endpoint(asio::ip::address::from_string("127.0.0.1"), 1234));
    std::cout << "connected to server\n";

    std::cout << "enter nums with space in between: ";
    std::string msg;
    std::getline(std::cin, msg);
    msg += '\n';

    asio::write(socket, asio::buffer(msg));

    asio::streambuf buf;
    asio::read_until(socket, buf, '\n');
    std::istream stream(&buf);
    std::string response;
    std::getline(stream, response);
    std::cout << "server response: " << response << std::endl;
    std::cin.get();
}