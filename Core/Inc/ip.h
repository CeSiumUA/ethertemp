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

typedef struct IP_Frame{
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
} IP_Frame;

uint8_t ip_calculate_checksum(uint8_t *data, uint16_t length);
uint16_t ip_process(IP_Frame *ip_frame, uint16_t frame_length);
void ip_transmit(uint8_t *data, uint16_t data_length, uint8_t dst_addr[IP_ADDRESS_BYTES_NUM], uint8_t src_addr[IP_ADDRESS_BYTES_NUM], uint8_t protocol);

#endif //ETHERNET_TEST_IP_H
