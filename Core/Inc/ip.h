//
// Created by fedir on 15.12.22.
//

#ifndef ETHERNET_TEST_IP_H
#define ETHERNET_TEST_IP_H

#include "main.h"
#include "ethernet.h"
#include "icmp.h"
#include "udp.h"

#define IP_FRAME_PROTOCOL_ICMP              0x01
#define IP_FRAME_PROTOCOL_UDP               0x11

typedef struct ip_frame_mask{
    uint8_t var_header_len;
    uint8_t diff_len_services;
    uint16_t total_length;
    uint16_t iden;
    uint16_t frag_offset;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t header_checksum;
    uint8_t src_ip_addr[IP_ADDRESS_BYTES_NUM];
    uint8_t dest_ip_addr[IP_ADDRESS_BYTES_NUM];
    uint8_t data[];
} ip_frame_mask;

extern uint8_t server_ip_address[IP_ADDRESS_BYTES_NUM];

uint16_t ip_calculate_checksum(uint8_t *data, uint16_t length);
uint16_t ip_process(ip_frame_mask *ip_frame, uint16_t frame_length);
void ip_transmit(uint8_t *data, uint16_t data_length, uint8_t dst_addr[IP_ADDRESS_BYTES_NUM], uint8_t src_addr[IP_ADDRESS_BYTES_NUM], uint8_t protocol, uint8_t dest_mac_addr[MAC_ADDRESS_BYTES_NUM]);

#endif //ETHERNET_TEST_IP_H
