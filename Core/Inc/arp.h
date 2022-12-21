/*
 * arp.h
 *
 *  Created on: Dec 12, 2022
 *      Author: mtgin
 */

#ifndef INC_ARP_H_
#define INC_ARP_H_

#include "main.h"
#include "ethernet.h"

#define ARP_OP_CODE_REQUEST                                     0x0001
#define ARP_OP_CODE_RESPONSE                                    0x0002

typedef struct arp_frame_mask{
    uint16_t h_type;
    uint16_t p_type;
    uint8_t h_len;
    uint8_t p_len;
    uint16_t op_code;
    uint8_t src_mac_addr[MAC_ADDRESS_BYTES_NUM];
    uint8_t src_ip_addr[IP_ADDRESS_BYTES_NUM];
    uint8_t dest_mac_addr[MAC_ADDRESS_BYTES_NUM];
    uint8_t dest_ip_addr[IP_ADDRESS_BYTES_NUM];
} arp_frame_mask;

typedef struct arp_table_entry{
    uint8_t ip_addr[IP_ADDRESS_BYTES_NUM];
    uint8_t mac_addr[MAC_ADDRESS_BYTES_NUM];
    struct arp_table_entry *next;
} arp_table_entry;

uint16_t arp_process(arp_frame_mask *frame, uint16_t frame_len);
arp_table_entry *get_entry(uint8_t entry_ip_address[IP_ADDRESS_BYTES_NUM]);
void arp_search(uint8_t dest_ip_address[IP_ADDRESS_BYTES_NUM]);
void arp_search_server(void);

#endif /* INC_ARP_H_ */
