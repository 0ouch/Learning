#define ASIO_STANDALONE
#include <asio.hpp>
#include <iostream>
#include <string>
#include <cctype>

int main() {
    asio::io_context io;
    asio::ip::tcp::acceptor acceptor(io, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 1234));
    std::cout << "server started in port 1234, awaiting connection..." << std::endl;

    asio::ip::tcp::socket socket(io);
    acceptor.accept(socket);
    std::cout << "client connected!" << std::endl;

    asio::streambuf buf;
    asio::read_until(socket, buf, '\n');

    std::istream stream(&buf);
    std::string received;
    std::getline(stream, received);

    auto len = received.size();
    for (char& c : received)
        c = std::toupper(c); // писать только латиницей!

    std::string response = std::to_string(len) + ": " + received + "\n";
    asio::write(socket, asio::buffer(response));
    std::cout << "answer sent: " << response;
}