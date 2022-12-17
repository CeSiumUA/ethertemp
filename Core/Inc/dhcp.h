//
// Created by fedir on 16.12.22.
//

#ifndef ETHERNET_TEST_DHCP_H
#define ETHERNET_TEST_DHCP_H

#include "udp.h"

#define DHCP_OP_REQUEST         0x01
#define DHCP_OP_REPLY           0x02

#define DHCP_HTYPE_ETH          0x01
#define DHCP_HLEN_ETH           0x06

#define DHCP_MAGIC_COOKIE       0x63825363

#define DHCP_OPTION_PARAMETER_REQUEST_LIST  55
#define DHCP_OPTION_HOST_NAME               12
#define DHCP_OPTION_DHCP_MSG_TYPE           53
#define DHCP_OPTION_CLIENT_ID               61
#define DHCP_OPTION_REQUESTED_IP            50
#define DHCP_OPTION_SERVER_IP               54

void dhcp_discover(void);

typedef struct dhcp_frame{
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
} dhcp_frame;

#endif //ETHERNET_TEST_DHCP_H
