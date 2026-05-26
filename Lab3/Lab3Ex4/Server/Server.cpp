#define ASIO_STANDALONE
#include <asio.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <memory>   //shared_ptr
#include <thread>

using asio::ip::tcp;    //как namespace но только tcp

class Session : public std::enable_shared_from_this<Session> {  //класс работы с клиентом содержит:
public:
    Session(tcp::socket socket, //сокет
        asio::strand<asio::io_context::executor_type> strand,   //странд для защиты от гонки данных
        std::vector<std::string>& log)  //общий лог (ссылка)
        : socket_(std::move(socket))
        , strand_(std::move(strand))
        , log_(log) {
    }

    void start() {  //запуск начала цикла чтения и записи
        do_read();
    }

private:
    void do_read() {    //асинхронное чтение запроса от клиента
        auto self = shared_from_this();     //ссылка на себя чтобы объект не удалялся во время асинхронного выполнения
        asio::async_read_until(socket_, buf_, '\n',     //читаем строку асинхронно
            asio::bind_executor(strand_,        //привязываем колбэк к стренд, чтобы колбэки работали по очереди
                [this, self](asio::error_code ec, std::size_t) {    //лямбда-колбэк
                    if (ec) return;     //если ошибка - выходим
                    std::istream stream(&buf_); 
                    std::string request;
                    std::getline(stream, request);
                    handle_request(request);        //передаем запрос на обработку
                }));
    }

    void handle_request(const std::string& request) {   //строка в число
        int n = std::stoi(request);

        uint64_t result = 1;        //факториал считаем
        for (int i = 2; i <= n; ++i)
            result *= i;

        std::string response = "Factorial: " + std::to_string(result) + "\n";

        //запись в общий лог (мы внутри strand – потокобезопасно)
        log_.push_back(response);

        do_write(response);
    }

    void do_write(std::string msg) {        //ответ клиенту
        auto self = shared_from_this();     //чтобы объект жил
        auto msg_ptr = std::make_shared<std::string>(std::move(msg));   //сохраняем строку в shared_ptr, чтобы она жила до завершения async_write
        asio::async_write(socket_,
            asio::buffer(*msg_ptr),
            asio::bind_executor(strand_,
                [this, self, msg_ptr](asio::error_code ec, std::size_t) {
                    if (ec) return;
                    do_read(); // ждём следующее сообщение
                }));
    }

    tcp::socket socket_;        //конкретный сокет
    asio::strand<asio::io_context::executor_type> strand_;      //сериализатор колбэков
    std::vector<std::string>& log_;     //ссылка на общий журнал
    asio::streambuf buf_;   //буфер для четния данных
};

class Server {      //принимает подключения, создает Session
public:
    Server(asio::io_context& io, short port,
        asio::strand<asio::io_context::executor_type> strand,
        std::vector<std::string>& log)
        : acceptor_(io, tcp::endpoint(tcp::v4(), port))
        , strand_(std::move(strand))
        , log_(log) {
        do_accept();    //включаем первое подключение
    }

private:
    void do_accept() {  //асинхронно ждем подключения
        acceptor_.async_accept(
            [this](asio::error_code ec, tcp::socket socket) {
                if (ec) return;
                std::make_shared<Session>(std::move(socket), strand_, log_)->start();   //Session живет пока подключен клиент
                do_accept();    //ждем следующего
            });
    }

    tcp::acceptor acceptor_;    //понятно
    asio::strand<asio::io_context::executor_type> strand_;
    std::vector<std::string>& log_;
};

int main(int argc, char* argv[]) {
    try {
        int num_threads = 4;    //кол-во потоков по умолчанию
        if (argc > 1)
            num_threads = std::stoi(argv[1]);   //задаем руками 

        asio::io_context io;
        std::vector<std::string> log;

        asio::strand<asio::io_context::executor_type> strand(io.get_executor());

        Server server(io, 12345, strand, log);  //создаем сервер
        auto work = asio::make_work_guard(io);

        std::cout << "Multithreaded server on port 12345, threads: " << num_threads << std::endl;

        std::vector<std::thread> threads;   //пул потоков
        for (int i = 0; i < num_threads; ++i)
            threads.emplace_back([&io]() { io.run(); });

        for (auto& t : threads) //завершение
            t.join();
    }
    catch (std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
    }
    return 0;
}