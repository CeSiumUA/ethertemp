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

#define ETH_FRAME_TYPE_ARP                                      0x0806
#define ETH_FRAME_TYPE_IP                                       0x0800

typedef struct ETH_Frame{
    uint8_t dest_mac_address[MAC_ADDRESS_BYTES_NUM];
    uint8_t src_mac_address[MAC_ADDRESS_BYTES_NUM];
    uint16_t ether_type;
    uint8_t data[];
} ETH_Frame;

void eth_process(ENC28J60_Frame *frame);

#endif /* INC_ETHERNET_H_ */
