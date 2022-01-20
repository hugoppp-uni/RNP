#include "options.hpp"
#include "bft_datagram.hpp"
#include "file_writer.hpp"

//c
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h> /* for close() for socket */
#include "logger.hpp"
#include "flags.hpp"
#include <csignal>

#include <iomanip>  // Includes ::std::hex
#include <filesystem>

int sock_fd;
std::unique_ptr<FileWriter> fileWriter;

void handle_valid_datagram(BftDatagram &datagram, const std::string &dir);

void signalHandler(int signum) {
    Logger::debug("Interrupt signal " + std::to_string(signum) + " received.");
    // TODO: Cleanup stuff ...
    close(sock_fd);
    exit(signum);
}


int main(int argc, char **args) {
    signal(SIGINT, signalHandler); // Cleanup when pressing CTRL+C

    Options options{argc, args};
    if (options.debug) {
        Logger::info("BFT Server started in Debug mode");
    } else {
        Logger::info("BFT Server started in Production mode");
    }
    Logger::info("Listening on port " + std::to_string(options.port));
    Logger::info("Incoming files will be saved into folder: " + options.directory);

    struct sockaddr_in server_addr{
        .sin_family = AF_INET,
        .sin_port = htons(options.port),
        .sin_addr =  {htonl(INADDR_ANY)}
    };

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (bind(sock_fd, (struct sockaddr *) &server_addr, sizeof server_addr) == -1) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    sockaddr_in client_addr{0};
    while (true) {
        BftDatagram received_datagram = BftDatagram::receive(sock_fd, client_addr);
        Flags response_flags;

        if (received_datagram.check_integrity()) {
            handle_valid_datagram(received_datagram, options.directory);
            response_flags = Flags::ACK;
        } else {
            response_flags = Flags::ERR;
        }

        auto response = BftDatagram(response_flags);
        int bytes_sent = response.send(sock_fd, client_addr);
        if (bytes_sent <= 0)
            Logger::error("error while sending response: " + std::string(strerror(errno)));
    }
}

void handle_valid_datagram(BftDatagram &datagram, const std::string &dir) {
    if ((datagram.get_flags() & Flags::SYN) == Flags::SYN) {
        std::string filename = datagram.get_payload_as_string();
        Logger::info("Receiving file '" + filename + "'");
        std::string file_path = (std::filesystem::path(dir) / filename).string();
        fileWriter = std::make_unique<FileWriter>(file_path);
    } else if ((datagram.get_flags() & Flags::ABR) == Flags::ABR) {
        Logger::warn("Got ABR, deleting '" + fileWriter->file_path + "'");
        fileWriter->abort();
    } else if ((datagram.get_flags() & Flags::FIN) == Flags::FIN) {
        Logger::warn("Upload of '" + fileWriter->file_path + "'complete");
        fileWriter = nullptr;
    } else if (datagram.get_flags() == Flags::None) {
        const std::vector<char> &payload = datagram.get_payload();
        fileWriter->writeBytes(payload);
        Logger::info(
            "Wrote " + std::to_string(payload.size()) + "/" + std::to_string(fileWriter->get_bytes_written()) +
            " bytes to '" + fileWriter->file_path + "'");
    }
}
