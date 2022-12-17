//
// Created by fedir on 15.12.22.
//
#include "udp.h"
#include <stdlib.h>
#include <string.h>

//OPTIMIZE
// consider using better approach, need to perform some tests
static udp_package_type udp_ports_table[UDP_MAX_HASH_TABLE_SIZE];

uint16_t calculate_checksum(UDP_Frame *frame){
    //TODO going to implement it in the future
    return frame->checksum;
}

uint16_t udp_process(UDP_Frame *udp_frame, uint16_t frame_length){
    uint16_t rx_checksum = udp_frame->checksum;
    udp_frame -> checksum = 0;

    return 0;
}

void udp_transmit(uint8_t *data, uint16_t data_length, uint16_t dst_port, uint16_t src_port, uint8_t dst_address[IP_ADDRESS_BYTES_NUM], uint8_t src_address[IP_ADDRESS_BYTES_NUM], udp_package_type package_type){
    uint16_t overall_length = sizeof (UDP_Frame) + data_length;
    UDP_Frame *frame = malloc(overall_length);

    frame->length = htons(overall_length);
    frame->src_port = htons(src_port);
    frame->dst_port = htons(dst_port);
    frame -> checksum = 0;
    frame -> checksum = calculate_checksum(frame);

    memcpy(frame->data, data, data_length);

    udp_ports_table[src_port] = package_type;

    ip_transmit((uint8_t*)frame, overall_length, dst_address, src_address, IP_FRAME_PROTOCOL_UDP);

    free(frame);
}