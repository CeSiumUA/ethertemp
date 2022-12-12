/*
 * arp.h
 *
 *  Created on: Dec 12, 2022
 *      Author: mtgin
 */

#ifndef INC_ARP_H_
#define INC_ARP_H_

#include "main.h"

#define ARP_OP_CODE_REQUEST                                     0x0001
#define ARP_OP_CODE_RESPONSE                                    0x0002

typedef struct ARP_Frame{
    uint16_t h_type;
    uint16_t p_type;
    uint8_t h_len;
    uint8_t p_len;
    uint16_t op_code;
    uint8_t src_mac_addr[MAC_ADDRESS_BYTES_NUM];
    uint8_t src_ip_addr[IP_ADDRESS_BYTES_NUM];
    uint8_t dest_mac_addr[MAC_ADDRESS_BYTES_NUM];
    uint8_t dest_ip_addr[IP_ADDRESS_BYTES_NUM];
} ARP_Frame;

uint16_t arp_process(ARP_Frame *frame, uint16_t frame_len);

#endif /* INC_ARP_H_ */
