//
// Created by fedir on 15.12.22.
//
#include "ip.h"
#include <string.h>
#include <stdlib.h>

uint16_t ip_calculate_checksum(uint8_t *data, uint16_t length){
    uint32_t res = 0;
    uint16_t *ptr = (uint16_t*)data;

    while(length > 1){
        res += *ptr;
        ptr++;
        length -= 2;
    }

    if(length > 0){
        res += *(uint8_t*)ptr;
    }

    while (res > 0xffff){
        res = (res >> 16) + (res & 0xffff);
    }

    return ~((uint16_t)res);
}

uint16_t ip_process(ip_frame_mask *ip_frame, uint16_t frame_length){
    uint16_t new_frame_length = 0;

    /*if(memcmp(ip_frame -> dest_ip_addr, ip_address, IP_ADDRESS_BYTES_NUM) != 0) {
        return new_frame_length;
    }*/

    uint16_t rx_checksum = ip_frame->header_checksum;

    ip_frame->header_checksum = 0;

    uint16_t calc_checksum = ip_calculate_checksum((uint8_t*)ip_frame, sizeof(ip_frame_mask));

    if(rx_checksum != calc_checksum){
        return new_frame_length;
    }

    uint16_t data_len = frame_length - sizeof(ip_frame_mask);
    uint16_t new_data_len = 0;

    if(ip_frame->protocol == IP_FRAME_PROTOCOL_ICMP){
        new_data_len = icmp_process((icmp_echo_frame_mask*)ip_frame -> data, data_len);
    }
    else if(ip_frame->protocol == IP_FRAME_PROTOCOL_UDP){
        new_data_len = udp_process((udp_frame_mask*)ip_frame -> data, data_len);
    }

    new_frame_length = new_data_len + sizeof (ip_frame_mask);
    ip_frame->total_length = htons(new_frame_length);

    ip_frame->iden = 0;
    ip_frame->frag_offset = 0;

    memcpy(ip_frame->dest_ip_addr, ip_frame->src_ip_addr, IP_ADDRESS_BYTES_NUM);
    memcpy(ip_frame->src_ip_addr, ip_address, IP_ADDRESS_BYTES_NUM);

    ip_frame->header_checksum = ip_calculate_checksum((uint8_t*)ip_frame, sizeof(ip_frame_mask));

    return new_frame_length;
}

void ip_transmit(uint8_t *data, uint16_t data_length, uint8_t dst_addr[IP_ADDRESS_BYTES_NUM], uint8_t src_addr[IP_ADDRESS_BYTES_NUM], uint8_t protocol, uint8_t dest_mac_addr[MAC_ADDRESS_BYTES_NUM]){
    data -= sizeof (ip_frame_mask);
    ip_frame_mask *transmit_frame = (ip_frame_mask *)data;

    memcpy(transmit_frame -> dest_ip_addr, dst_addr, IP_ADDRESS_BYTES_NUM);
    memcpy(transmit_frame -> src_ip_addr, src_addr, IP_ADDRESS_BYTES_NUM);

    uint32_t frame_length = data_length + sizeof(ip_frame_mask);

    transmit_frame -> var_header_len = (4 << 4) | 5;

    transmit_frame -> protocol = protocol;

    transmit_frame -> ttl = -1;

    transmit_frame -> total_length = htons(frame_length);

    transmit_frame -> iden = 0;
    transmit_frame -> frag_offset = 0;

    transmit_frame -> header_checksum = 0;
    transmit_frame -> header_checksum = ip_calculate_checksum((uint8_t*)transmit_frame, sizeof(ip_frame_mask));

    eth_transmit(data, frame_length, ETH_FRAME_TYPE_IP, dest_mac_addr);
}