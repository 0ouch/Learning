#define ASIO_STANDALONE
#include <asio.hpp>
#include <iostream>
#include <string>

int main() {
    asio::io_context io;
    asio::ip::tcp::socket socket(io);
    socket.connect(asio::ip::tcp::endpoint(asio::ip::address::from_string("127.0.0.1"), 1234));
    std::cout << "Connected to server" << std::endl;

    std::cout << "Enter command (example: timer 5): ";
    std::string msg;
    std::getline(std::cin, msg);
    msg += '\n';

    asio::write(socket, asio::buffer(msg));

    // Читаем первый ответ (Ready in )
    asio::streambuf buf1;
    asio::read_until(socket, buf1, '\n');
    std::istream stream1(&buf1);
    std::string response1;
    std::getline(stream1, response1);
    std::cout << "Server: " << response1 << std::endl;

    // Читаем второй ответ (Done!)
    asio::streambuf buf2;
    asio::read_until(socket, buf2, '\n');
    std::istream stream2(&buf2);
    std::string response2;
    std::getline(stream2, response2);
    std::cout << "Server: " << response2 << std::endl;

    std::cin.get();

    return 0;
}