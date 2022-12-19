/*
 * ethernet.h
 *
 *  Created on: Dec 12, 2022
 *      Author: mtgin
 */

#ifndef INC_ETHERNET_H_
#define INC_ETHERNET_H_

#include "enc28j60.h"
#include "arp.h"
#include "ip.h"

#define ETH_FRAME_TYPE_ARP                                      0x0806
#define ETH_FRAME_TYPE_IP                                       0x0800

typedef struct eth_frame_mask{
    uint8_t dest_mac_address[MAC_ADDRESS_BYTES_NUM];
    uint8_t src_mac_address[MAC_ADDRESS_BYTES_NUM];
    uint16_t ether_type;
    uint8_t data[];
} eth_frame_mask;

void eth_process(enc28j60_frame_mask *frame);
void eth_transmit(uint8_t *data, uint16_t length, uint16_t ether_type, uint8_t dest_mac[MAC_ADDRESS_BYTES_NUM]);

#endif /* INC_ETHERNET_H_ */
