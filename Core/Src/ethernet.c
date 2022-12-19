/*
 * ethernet.c
 *
 *  Created on: Dec 12, 2022
 *      Author: mtgin
 */

#include "ethernet.h"
#include <string.h>
#include <stdlib.h>

static void eth_response(eth_frame_mask *frame, uint16_t len);

void eth_process(enc28j60_frame_mask *frame){
    uint16_t response_size = 0;
    uint16_t request_size = receive_frame(frame);

    if(request_size <= 0){
        return;
    }

    eth_frame_mask *eth_frame = (eth_frame_mask*)frame->data;
    uint16_t ether_type = ntohs(eth_frame->ether_type);

    uint16_t process_frame_len = request_size - sizeof(eth_frame_mask);

    //FIXME use switch instead
    if(ether_type == ETH_FRAME_TYPE_ARP){
        response_size = arp_process((arp_frame_mask *)eth_frame->data, process_frame_len);
    }
    else if(ether_type == ETH_FRAME_TYPE_IP){
        response_size = ip_process((ip_frame_mask*)eth_frame -> data, process_frame_len);
    }

    if(response_size > 0){
        eth_response(eth_frame, response_size);
    }
}

void eth_transmit(uint8_t *data, uint16_t length, uint16_t ether_type, uint8_t dest_mac[MAC_ADDRESS_BYTES_NUM]){
    eth_frame_mask *eth_frame = malloc(sizeof(eth_frame_mask) + length);
    eth_frame -> ether_type = htons(ether_type);
    memcpy(eth_frame -> dest_mac_address, dest_mac, MAC_ADDRESS_BYTES_NUM);
    memcpy(eth_frame -> src_mac_address, mac_address, MAC_ADDRESS_BYTES_NUM);
    memcpy(eth_frame -> data, data, length);

    transmit_frame((uint8_t*)eth_frame, length + sizeof(eth_frame_mask));

    free(eth_frame);
}

static void eth_response(eth_frame_mask *frame, uint16_t len){
    memcpy(frame->dest_mac_address, frame->src_mac_address, MAC_ADDRESS_BYTES_NUM);
    memcpy(frame->src_mac_address, mac_address, MAC_ADDRESS_BYTES_NUM);

    transmit_frame((uint8_t*)frame, len + sizeof(eth_frame_mask));
}