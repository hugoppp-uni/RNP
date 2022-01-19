#pragma once

#include "lib/include/CRC.h"
#include "logger.hpp"
#include "flags.hpp"


#define HEADER_SIZE (sizeof(int) + sizeof(short) + sizeof(Flags))
#define MAX_DATAGRAM_SIZE 512
using PayloadArray = std::array<char, MAX_DATAGRAM_SIZE - HEADER_SIZE>;

#pragma pack(1)
class BftDatagram {
public:

    BftDatagram() : checksum(0), payload_size(0), flags(Flags::None) {};
    explicit BftDatagram(Flags flags);
    BftDatagram(Flags flags, std::vector<char> data);
    BftDatagram(Flags flags, const std::string &data);

    bool check_integrity();
    int send(int sockfd, const sockaddr_in &client_addr) const;

    static BftDatagram receive(int fd, sockaddr_in &client_addr);

    [[nodiscard]] std::string to_string() const;

    //getter
    [[nodiscard]] std::vector<char> get_payload() const { return {payload.begin(), payload.begin() + payload_size}; }
    [[nodiscard]] std::string get_payload_as_string() const { return {payload.begin(), payload.begin() + payload_size}; }
    [[nodiscard]] Flags get_flags() const { return flags; }
    [[nodiscard]] int get_payload_size() const { return payload_size; }
    [[nodiscard]] std::string checksum_as_string() const;
    [[nodiscard]] int size() const {
        return payload_size + (int) HEADER_SIZE;
    }
private:
    unsigned int checksum;
    unsigned short payload_size;
    Flags flags;
    PayloadArray payload = {0};

    [[nodiscard]] unsigned int calc_checksum();
};

static_assert(sizeof(BftDatagram) == MAX_DATAGRAM_SIZE);
