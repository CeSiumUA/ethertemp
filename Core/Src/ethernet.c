/*
 * ethernet.c
 *
 *  Created on: Dec 12, 2022
 *      Author: mtgin
 */

#include "ethernet.h"
#include <string.h>
#include <stdlib.h>

static void eth_response(ETH_Frame *frame, uint16_t len);

void eth_process(ENC28J60_Frame *frame){
    uint16_t response_size = 0;
    uint16_t request_size = receive_frame(frame);

    if(request_size <= 0){
        return;
    }

    ETH_Frame *eth_frame = (ETH_Frame*)frame->data;
    uint16_t ether_type = ntohs(eth_frame->ether_type);

    uint16_t process_frame_len = request_size - sizeof(ETH_Frame);

    //FIXME use switch instead
    if(ether_type == ETH_FRAME_TYPE_ARP){
        response_size = arp_process((ARP_Frame*)eth_frame->data, process_frame_len);
    }
    else if(ether_type == ETH_FRAME_TYPE_IP){
        response_size = ip_process((IP_Frame*)eth_frame -> data, process_frame_len);
    }

    if(response_size > 0){
        eth_response(eth_frame, response_size);
    }
}

void eth_transmit(uint8_t *data, uint16_t length, uint16_t ether_type, uint8_t dest_mac[MAC_ADDRESS_BYTES_NUM]){
    ETH_Frame *eth_frame = malloc(sizeof(ETH_Frame) + length);
    eth_frame -> ether_type = htons(ether_type);
    memcpy(eth_frame -> dest_mac_address, dest_mac, MAC_ADDRESS_BYTES_NUM);
    memcpy(eth_frame -> src_mac_address, mac_address, MAC_ADDRESS_BYTES_NUM);
    memcpy(eth_frame -> data, data, length);

    transmit_frame((uint8_t*)eth_frame, length + sizeof(ETH_Frame));

    free(eth_frame);
}

static void eth_response(ETH_Frame *frame, uint16_t len){
    memcpy(frame->dest_mac_address, frame->src_mac_address, MAC_ADDRESS_BYTES_NUM);
    memcpy(frame->src_mac_address, mac_address, MAC_ADDRESS_BYTES_NUM);

    transmit_frame((uint8_t*)frame, len + sizeof(ETH_Frame));
}