//
// Created by fedir on 16.12.22.
//

#ifndef ETHERNET_TEST_DHCP_H
#define ETHERNET_TEST_DHCP_H

#include "udp.h"

#define DHCP_OP_REQUEST                             0x01
#define DHCP_OP_REPLY                               0x02

#define DHCP_HTYPE_ETH                              0x01
#define DHCP_HLEN_ETH                               0x06

#define DHCP_MAGIC_COOKIE_0                         0x63
#define DHCP_MAGIC_COOKIE_1                         0x82
#define DHCP_MAGIC_COOKIE_2                         0x53
#define DHCP_MAGIC_COOKIE_3                         0x63

#define DHCP_OPTION_PARAMETER_REQUEST_LIST          55
#define DHCP_OPTION_HOST_NAME                       12
#define DHCP_OPTION_DHCP_MSG_TYPE                   53
#define DHCP_OPTION_CLIENT_ID                       61
#define DHCP_OPTION_REQUESTED_IP                    50
#define DHCP_OPTION_SERVER_IP                       54
#define DHCP_OPTION_PARAMETER_REQUEST_SUBNET_MASK   1
#define DHCP_OPTION_PARAMETER_REQUEST_ROUTE_GATEWAY 3
#define DHCP_OPTION_PARAMETER_DNS                   6
#define DHCP_OPTION_PARAMETER_DOMAIN_NAME           15
#define DHCP_OPTION_END_MARK                        255

#define DHCP_UDP_SOURCE_PORT                        68
#define DHCP_UDP_DESTINATION_PORT                   67

typedef struct dhcp_frame_mask{
    uint8_t op;
    uint8_t htype;
    uint8_t hlen;
    uint8_t hops;
    uint32_t xid;
    uint16_t secs;
    uint16_t flags;
    uint8_t ciaddr[4];
    uint8_t yiaddr[4];
    uint8_t siaddr[4];
    uint8_t giaddr[4];
    uint8_t chaddr[16];
    uint8_t sname[64];
    uint8_t file[128];
    uint8_t options[];
} dhcp_frame_mask;

void dhcp_client_process(dhcp_frame_mask *frame, uint16_t frame_length);
void dhcp_server_process(dhcp_frame_mask *frame, uint16_t frame_length);
void dhcp_discover(void);

#endif //ETHERNET_TEST_DHCP_H
