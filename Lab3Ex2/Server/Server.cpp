#define ASIO_STANDALONE
#include <asio.hpp>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <thread>

int main() {
    asio::io_context io;

    // Work guard не даёт io_context остановиться
    auto work = asio::make_work_guard(io);

    // Запускаем цикл обработки событий в фоновом потоке
    std::thread io_thread([&io]() {
        io.run();
        });

    asio::ip::tcp::acceptor acceptor(io, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 1234));
    std::cout << "Server listening on port 1234" << std::endl;

    while (true) {
        asio::ip::tcp::socket socket(io);
        acceptor.accept(socket);
        std::cout << "Client connected" << std::endl;

        asio::streambuf buf;
        asio::read_until(socket, buf, '\n');
        std::istream stream(&buf);
        std::string received;
        std::getline(stream, received);
        std::cout << "Received: " << received << std::endl;

        // Отправляем вычисление в очередь
        asio::post(io, [socket = std::move(socket), data = std::move(received)]() mutable {
            std::istringstream iss(data);
            std::vector<int> numbers;
            int num;
            while (iss >> num)
                numbers.push_back(num);

            int max_val = *std::max_element(numbers.begin(), numbers.end());

            std::string response = "Maximum: " + std::to_string(max_val) + "\n";
            asio::write(socket, asio::buffer(response));
            std::cout << "Response sent: " << response;
            });
    }

    io_thread.join();
    return 0;
}