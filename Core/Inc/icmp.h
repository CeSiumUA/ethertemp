//
// Created by fedir on 15.12.22.
//

#ifndef ETHERNET_TEST_ICMP_H
#define ETHERNET_TEST_ICMP_H

#include "main.h"
#include "ip.h"

#define ICMP_FRAME_TYPE_ECHO_REQUEST                            0x08
#define ICMP_FRAME_TYPE_ECHO_REPLY                              0x00

typedef struct icmp_echo_frame_mask{
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t id;
    uint16_t seq_num;
    uint8_t data[];
} icmp_echo_frame_mask;

uint16_t icmp_process(icmp_echo_frame_mask *icmp_frame, uint16_t frame_length);

#endif //ETHERNET_TEST_ICMP_H
