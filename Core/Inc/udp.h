//
// Created by fedir on 15.12.22.
//

#ifndef ETHERNET_TEST_UDP_H
#define ETHERNET_TEST_UDP_H

#include "main.h"
#include "ip.h"

#define UDP_MAX_HASH_TABLE_SIZE         65535

typedef enum udp_package_type{
    NONE,
    DHCP_INCOMING = 67,
    DHCP_OUTGOING = 68,
    PING_PONG
} udp_package_type;

typedef struct udp_consumed_port{
    uint16_t port;
    udp_package_type package_type;
    struct udp_consumed_port *next;
} udp_consumed_port;

typedef struct udp_frame_mask{
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t length;
    uint16_t checksum;
    uint8_t data[];
} udp_frame_mask;

uint16_t udp_process(udp_frame_mask *udp_frame, uint16_t frame_length);
void udp_transmit(uint8_t *data, uint16_t data_length, uint16_t dst_port, uint16_t src_port, uint8_t dst_address[IP_ADDRESS_BYTES_NUM], uint8_t src_address[IP_ADDRESS_BYTES_NUM], udp_package_type package_type, uint8_t dest_mac_address[MAC_ADDRESS_BYTES_NUM]);

#endif //ETHERNET_TEST_UDP_H
