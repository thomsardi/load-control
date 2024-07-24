#ifndef ETHERNET_DEF_H
#define ETHERNET_DEF_H

#include <stdint.h>

enum server_type : uint8_t {
    STATIC = 1,
    DHCP = 2
};
enum mode_type : uint8_t {
    AP = 1,
    STATION = 2,
    AP_STATION = 3
};

#endif