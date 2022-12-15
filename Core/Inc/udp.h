//
// Created by fedir on 15.12.22.
//

#ifndef ETHERNET_TEST_UDP_H
#define ETHERNET_TEST_UDP_H

#include "main.h"
#include "ip.h"

typedef struct UDP_Frame{
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t length;
    uint16_t checksum;
    uint8_t data[];
} UDP_Frame;

uint16_t udp_process(UDP_Frame *udp_frame, uint16_t frame_length);
void udp_transmit(uint8_t *data, uint16_t data_length);

#endif //ETHERNET_TEST_UDP_H
