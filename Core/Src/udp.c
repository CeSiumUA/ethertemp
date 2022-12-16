//
// Created by fedir on 15.12.22.
//
#include "udp.h"
#include <stdlib.h>
#include <string.h>

uint16_t udp_process(UDP_Frame *udp_frame, uint16_t frame_length){

}

void udp_transmit(uint8_t *data, uint16_t data_length, uint16_t dst_port, uint16_t src_port, uint8_t dst_address[IP_ADDRESS_BYTES_NUM], uint8_t src_address[IP_ADDRESS_BYTES_NUM]){
    uint16_t overall_length = sizeof (UDP_Frame) + data_length;
    UDP_Frame *frame = malloc(overall_length);

    frame->length = htons(overall_length);
    frame->src_port = htons(src_port);
    frame->dst_port = htons(dst_port);
    frame -> checksum = 0;

    memcpy(frame->data, data, data_length);

    ip_transmit((uint8_t*)frame, overall_length, dst_address, src_address, IP_FRAME_PROTOCOL_UDP);

    free(frame);
}