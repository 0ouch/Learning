#define ASIO_STANDALONE
#include <asio.hpp>
#include <iostream>
#include <string>

int main() {
    try {
        asio::io_context io;
        asio::ip::tcp::socket socket(io);
        socket.connect(asio::ip::tcp::endpoint(
            asio::ip::address::from_string("127.0.0.1"), 12345));
        std::cout << "Connected to server" << std::endl;

        std::cout << "Enter a number: ";
        std::string input;
        std::getline(std::cin, input);
        input += '\n';

        asio::write(socket, asio::buffer(input));

        asio::streambuf buf;
        asio::read_until(socket, buf, '\n');
        std::istream stream(&buf);
        std::string response;
        std::getline(stream, response);
        std::cout << "Server response: " << response << std::endl;

        std::cout << "Press Enter to exit" << std::endl;
        std::cin.get();
    }
    catch (std::exception& e) {
        std::cerr << "Client error: " << e.what() << std::endl;
    }
    return 0;
}