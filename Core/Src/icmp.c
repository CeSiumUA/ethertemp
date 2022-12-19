//
// Created by fedir on 15.12.22.
//

#include "icmp.h"

uint16_t icmp_process(icmp_echo_frame_mask *icmp_frame, uint16_t frame_length){
    uint16_t new_frame_length = 0;

    uint16_t rx_checksum = icmp_frame -> checksum;
    icmp_frame->checksum = 0;
    uint16_t calc_checksum = ip_calculate_checksum((uint8_t*)icmp_frame, frame_length);

    if(rx_checksum != calc_checksum) return new_frame_length;

    if(icmp_frame -> type == ICMP_FRAME_TYPE_ECHO_REQUEST){
        icmp_frame -> type = ICMP_FRAME_TYPE_ECHO_REPLY;
        icmp_frame -> checksum = ip_calculate_checksum((uint8_t*)icmp_frame, frame_length);
        new_frame_length = frame_length;
    }

    return new_frame_length;
}