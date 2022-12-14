/*
 * ethernet.c
 *
 *  Created on: Dec 12, 2022
 *      Author: mtgin
 */

#include "ethernet.h"
#include <string.h>

static void eth_response(ETH_Frame *frame, uint16_t len);

void eth_process(ENC28J60_Frame *frame){
    uint16_t response_size = 0;
    uint16_t request_size = receive_frame(frame);

    if(request_size <= 0){
        return;
    }

    ETH_Frame *eth_frame = (ETH_Frame*)frame->data;
    uint16_t ether_type = ntohs(eth_frame->ether_type);

    if(ether_type == ETH_FRAME_TYPE_ARP){
        uint16_t arp_frame_len = request_size - sizeof(ETH_Frame);
        response_size = arp_process((ARP_Frame*)eth_frame->data, arp_frame_len);
    }

    if(response_size > 0){
        eth_response(eth_frame, response_size);
    }
}

static void eth_response(ETH_Frame *frame, uint16_t len){
    memcpy(frame->dest_mac_address, frame->src_mac_address, MAC_ADDRESS_BYTES_NUM);
    memcpy(frame->src_mac_address, mac_address, MAC_ADDRESS_BYTES_NUM);

    transmit_frame((uint8_t*)frame, len + sizeof(ETH_Frame));
}