//
// Created by fedir on 15.12.22.
//

#ifndef ETHERNET_TEST_UDP_H
#define ETHERNET_TEST_UDP_H

#include <stdbool.h>
#include "main.h"
#include "ip.h"
#include "dhcp.h"

#define UDP_MAX_HASH_TABLE_SIZE         65535

typedef enum udp_package_type{
    NONE,
    DHCP_SERVER = 67,
    DHCP_CLIENT = 68,
    PING_PONG = 25512,
    TEMP_SENSOR_OUT = 50002,
    TEMP_SENSOR_IN
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

typedef struct udp_ipv4_pseudo_header{
    uint8_t src_ip_addr[IP_ADDRESS_BYTES_NUM];
    uint8_t dest_ip_addr[IP_ADDRESS_BYTES_NUM];
    uint8_t zeros;
    uint8_t protocol;
    uint16_t udp_length;
    uint8_t data[];
} udp_ipv4_pseudo_header;

void udp_send_info_to_server(float temp_data, float rh_data);
uint16_t udp_process(udp_frame_mask *udp_frame, uint8_t src_ip_address[IP_ADDRESS_BYTES_NUM], uint16_t frame_length);
void udp_transmit(uint8_t *data, uint16_t data_length, uint16_t dst_port, uint16_t src_port, uint8_t dst_address[IP_ADDRESS_BYTES_NUM], uint8_t src_address[IP_ADDRESS_BYTES_NUM], udp_package_type package_type, uint8_t dest_mac_address[MAC_ADDRESS_BYTES_NUM]);

#endif //ETHERNET_TEST_UDP_H
