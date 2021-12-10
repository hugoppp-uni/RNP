#include <iostream>
#include <thread>
#include "logger.hpp"

#include "options.hpp"
#include "connection_listener.hpp"

void handle_incoming_requests(const ConnectionListener &listener);

/*
 * For now, you can test this with netcat:
 * 1. http_server /home -p 8080
 * 2. echo -n "number 1" | netcat -N localhost 8080 & echo -n "number 2" | netcat -N localhost 8080
 */
int main(int argc, char **argv) {

    Options opt{argc, argv};

    Logger::set_logfile(opt.logfile.has_value() ? opt.logfile.value() : "");
    Logger::set_log_to_console(!opt.logfile.has_value());

    if (!opt.port.has_value())
        opt.port = 8080;

    ConnectionListener listener = ConnectionListener{opt.port.value()};
    std::thread listener_thread{[&listener] {

        bool running = true;
        while (running) {
            try {
                handle_incoming_requests(listener);
            } catch (ListenerClosedException &) {
                running = false;
            }
        }

    }};

    int c{0};
    while (c != 'q') {
        c = getchar();
    }

    listener.close();
    listener_thread.join();
    return 0;
}

void handle_incoming_requests(const ConnectionListener &listener) {
    std::unique_ptr<Connection> cnn = listener.accept_next_connection();
    if (cnn) {
        std::thread{[cnn = std::move(cnn)] {
            Logger::info("Client connected from '" + cnn->get_address()->str() + "', receiving data");
#ifndef NDEBUG
            Logger::data(cnn->receive_string());
#endif
            std::this_thread::sleep_for(std::chrono::seconds(4));
        }}.detach(); // <- just detach for now (aka do not terminate thread when it goes out of scope)
    } else {
        Logger::error("Error while accepting client connection");
    }
}
