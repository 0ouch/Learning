#define ASIO_STANDALONE
#include <asio.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <thread>

using asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket,
        asio::strand<asio::io_context::executor_type> strand,
        std::vector<std::string>& log)
        : socket_(std::move(socket))
        , strand_(std::move(strand))
        , log_(log) {
    }

    void start() {
        do_read();
    }

private:
    void do_read() {
        auto self = shared_from_this();
        // Очищаем буфер перед чтением (на случай остатков)
        buf_.consume(buf_.size());
        asio::async_read_until(socket_, buf_, '\n',
            asio::bind_executor(strand_,
                [this, self](asio::error_code ec, std::size_t) {
                    if (ec) return;
                    std::istream stream(&buf_);
                    std::string request;
                    std::getline(stream, request);
                    handle_request(request);
                }));
    }

    void handle_request(const std::string& request) {
        int n = std::stoi(request);

        uint64_t result = 1;
        for (int i = 2; i <= n; ++i)
            result *= i;

        std::string response = "Factorial: " + std::to_string(result) + "\n";

        // Запись в общий лог (мы внутри strand – потокобезопасно)
        log_.push_back(response);

        do_write(response);
    }

    void do_write(std::string msg) {
        auto self = shared_from_this();
        // Сохраняем строку в shared_ptr, чтобы она жила до завершения async_write
        auto msg_ptr = std::make_shared<std::string>(std::move(msg));
        asio::async_write(socket_,
            asio::buffer(*msg_ptr),
            asio::bind_executor(strand_,
                [this, self, msg_ptr](asio::error_code ec, std::size_t) {
                    if (ec) return;
                    do_read(); // ждём следующее сообщение
                }));
    }

    tcp::socket socket_;
    asio::strand<asio::io_context::executor_type> strand_;
    std::vector<std::string>& log_;
    asio::streambuf buf_;
};

class Server {
public:
    Server(asio::io_context& io, short port,
        asio::strand<asio::io_context::executor_type> strand,
        std::vector<std::string>& log)
        : acceptor_(io, tcp::endpoint(tcp::v4(), port))
        , strand_(std::move(strand))
        , log_(log) {
        do_accept();
    }

private:
    void do_accept() {
        acceptor_.async_accept(
            [this](asio::error_code ec, tcp::socket socket) {
                if (ec) return;
                std::make_shared<Session>(std::move(socket), strand_, log_)->start();
                do_accept();
            });
    }

    tcp::acceptor acceptor_;
    asio::strand<asio::io_context::executor_type> strand_;
    std::vector<std::string>& log_;
};

int main(int argc, char* argv[]) {
    try {
        int num_threads = 4;
        if (argc > 1)
            num_threads = std::stoi(argv[1]);

        asio::io_context io;
        std::vector<std::string> log;

        asio::strand<asio::io_context::executor_type> strand(io.get_executor());

        Server server(io, 12345, strand, log);
        auto work = asio::make_work_guard(io);

        std::cout << "Multithreaded server on port 12345, threads: " << num_threads << std::endl;

        std::vector<std::thread> threads;
        for (int i = 0; i < num_threads; ++i)
            threads.emplace_back([&io]() { io.run(); });

        for (auto& t : threads)
            t.join();
    }
    catch (std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
    }
    return 0;
}