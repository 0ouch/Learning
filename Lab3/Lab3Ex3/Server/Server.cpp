#define ASIO_STANDALONE
#include <asio.hpp>
#include <iostream>
#include <string>
#include <sstream>
#include <thread>

int main() {
    asio::io_context io;
    auto work = asio::make_work_guard(io);

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

        // Передаём обработку в очередь io_context
        asio::post(io, [socket = std::move(socket), data = std::move(received)]() mutable {
            // Проверяем, начинается ли строка с "timer "
            if (data.rfind("timer ", 0) == 0) {
                // Извлекаем число секунд
                std::string num_str = data.substr(6); // после timer
                int sec = std::stoi(num_str);

                // Немедленно отправляем подтверждение
                std::string ack = "Ready in " + std::to_string(sec) + " sec\n";
                asio::write(socket, asio::buffer(ack));
                std::cout << "Sent: " << ack;

                // Создаём таймер на sec секунд
                auto timer = std::make_shared<asio::steady_timer>(
                    socket.get_executor(), asio::chrono::seconds(sec));

                // Запускаем асинхронное ожидание
                timer->async_wait(
                    [socket = std::move(socket), timer](const asio::error_code&) mutable {
                        std::string done = "Done!\n";
                        asio::write(socket, asio::buffer(done));
                        std::cout << "Sent: " << done;
                    });
            }
            else {
                // Неизвестная команда
                std::string err = "Unknown command\n";
                asio::write(socket, asio::buffer(err));
            }
            });
    }

    io_thread.join();
    return 0;
}