/*
 * arp.c
 *
 *  Created on: Dec 12, 2022
 *      Author: mtgin
 */

#include "arp.h"
#include <string.h>

uint16_t arp_process(arp_frame_mask *frame, uint16_t frame_len){
    uint16_t new_frame_len = 0;

    if(memcmp(frame->dest_ip_addr, ip_address, IP_ADDRESS_BYTES_NUM) == 0){
        if(frame->op_code == (ntohs(ARP_OP_CODE_REQUEST))){
            memcpy(frame->dest_mac_addr, frame->src_mac_addr, MAC_ADDRESS_BYTES_NUM);
            memcpy(frame->src_mac_addr, mac_address, MAC_ADDRESS_BYTES_NUM);

            memcpy(frame->dest_ip_addr, frame->src_ip_addr, IP_ADDRESS_BYTES_NUM);
            memcpy(frame->src_ip_addr, ip_address, IP_ADDRESS_BYTES_NUM);

            frame->op_code = htons(ARP_OP_CODE_RESPONSE);
            new_frame_len = frame_len;
        }
    }

    return new_frame_len;
}