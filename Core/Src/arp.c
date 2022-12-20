/*
 * arp.c
 *
 *  Created on: Dec 12, 2022
 *      Author: mtgin
 */

#include "arp.h"
#include <string.h>
#include <stdlib.h>

#define ARP_SEARCH_BUFFER_MAX_LENGTH        256

static arp_table_entry *create_entry(uint8_t entry_ip_address[IP_ADDRESS_BYTES_NUM], uint8_t entry_mac_address[MAC_ADDRESS_BYTES_NUM]);
static arp_table_entry *set_entry(uint8_t entry_ip_address[IP_ADDRESS_BYTES_NUM], uint8_t entry_mac_address[MAC_ADDRESS_BYTES_NUM]);

static arp_table_entry *arp_table = NULL;

static arp_table_entry *create_entry(uint8_t entry_ip_address[IP_ADDRESS_BYTES_NUM], uint8_t entry_mac_address[MAC_ADDRESS_BYTES_NUM]){
    arp_table_entry *entry = malloc(sizeof (arp_table_entry));
    memcpy(entry->ip_addr, entry_ip_address, IP_ADDRESS_BYTES_NUM);
    memcpy(entry->mac_addr, entry_mac_address, MAC_ADDRESS_BYTES_NUM);

    entry->next = NULL;

    return entry;
}

arp_table_entry *get_entry(uint8_t entry_ip_address[IP_ADDRESS_BYTES_NUM]){
    arp_table_entry *entry = arp_table;

    while(entry != NULL && memcmp(entry->ip_addr, entry_ip_address, IP_ADDRESS_BYTES_NUM)){
        entry = entry->next;
    }

    return entry;
}

static arp_table_entry *set_entry(uint8_t entry_ip_address[IP_ADDRESS_BYTES_NUM], uint8_t entry_mac_address[MAC_ADDRESS_BYTES_NUM]){
    arp_table_entry *entry = arp_table;
    if(entry == NULL){
        arp_table = create_entry(entry_ip_address, entry_mac_address);
        return arp_table;
    }

    while (!memcmp(entry->ip_addr, entry_ip_address, IP_ADDRESS_BYTES_NUM)){
        if(entry->next == NULL){
            entry -> next = create_entry(entry_ip_address, entry_mac_address);
            return entry;
        }
        entry = entry->next;
    }

    memcpy(entry->mac_addr, entry_mac_address, MAC_ADDRESS_BYTES_NUM);

    return entry;
}

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
        else if(frame->op_code == (ntohs(ARP_OP_CODE_RESPONSE))){
            set_entry(frame->src_ip_addr, frame->src_mac_addr);
        }
    }

    return new_frame_len;
}

void arp_search(uint8_t dest_ip_address[IP_ADDRESS_BYTES_NUM]){
    uint8_t arp_search_buffer[ARP_SEARCH_BUFFER_MAX_LENGTH];

    uint8_t *arp_buf_start = arp_search_buffer + ARP_SEARCH_BUFFER_MAX_LENGTH - sizeof (arp_frame_mask);

    arp_frame_mask *arp_frame = (arp_frame_mask *)arp_buf_start;

    memcpy(arp_frame->src_ip_addr, ip_address, IP_ADDRESS_BYTES_NUM);
    memcpy(arp_frame->dest_ip_addr, dest_ip_address, IP_ADDRESS_BYTES_NUM);

    arp_frame->h_type = htons(1);

    arp_frame->p_type = htons(ETH_FRAME_TYPE_IP);

    arp_frame->h_len = 6;
    arp_frame->p_len = 4;

    arp_frame->op_code = htons(ARP_OP_CODE_REQUEST);

    uint8_t dest_mac_address[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    memcpy(arp_frame->dest_mac_addr, dest_mac_address, MAC_ADDRESS_BYTES_NUM);
    memcpy(arp_frame->src_mac_addr, mac_address, MAC_ADDRESS_BYTES_NUM);

    eth_transmit(arp_buf_start, sizeof (arp_frame_mask), ETH_FRAME_TYPE_ARP, dest_mac_address);
}