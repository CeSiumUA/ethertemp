//
// Created by fedir on 16.12.22.
//
#include "dhcp.h"
#include <stdlib.h>
#include <string.h>
#include "usart.h"

static const char host_name[] = "STM32F411";

void dhcp_discover(void){
    uint8_t request_options[] = {
            // Magic cookie
            DHCP_MAGIC_COOKIE_0, DHCP_MAGIC_COOKIE_1, DHCP_MAGIC_COOKIE_2, DHCP_MAGIC_COOKIE_3,
            // Message type
            DHCP_OPTION_DHCP_MSG_TYPE, 1, 1,
            // Client identifier
            DHCP_OPTION_CLIENT_ID, 7, 1, mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5],
            //Host name
            DHCP_OPTION_HOST_NAME, 9, 'S', 'T', 'M', '3', '2', 'F', '4', '1', '1',
            //Requested IP address
            DHCP_OPTION_PARAMETER_REQUEST_LIST, 4, DHCP_OPTION_PARAMETER_REQUEST_SUBNET_MASK, DHCP_OPTION_PARAMETER_REQUEST_ROUTE_GATEWAY, DHCP_OPTION_PARAMETER_DOMAIN_NAME, DHCP_OPTION_PARAMETER_DNS,
            //End mark
            DHCP_OPTION_END_MARK
    };

    uint32_t xid = rand();

    uint16_t frame_size = sizeof (dhcp_frame) + sizeof(request_options);

    dhcp_frame *frame = malloc(frame_size);

    frame->op = DHCP_OP_REQUEST;
    frame->htype = DHCP_HTYPE_ETH;
    frame->hlen = DHCP_HLEN_ETH;
    frame->hops = 0;
    frame->xid = xid;
    frame->secs = 0;
    frame->flags = 0;

    uint8_t zero_values = sizeof (frame->ciaddr) + sizeof (frame->yiaddr) + sizeof (frame->siaddr) + sizeof (frame->giaddr);

    uint8_t *ciaddr_ptr = frame->ciaddr;

    for(uint8_t i = 0; i < zero_values; i++){
        *ciaddr_ptr++ = 0;
    }

    memcpy(frame->chaddr, mac_address, sizeof(mac_address));

    uint8_t *sname_ptr = frame->sname;
    for(uint8_t i = 0; i < (sizeof (frame->sname) + sizeof (frame->file)); i++){
        *sname_ptr++ = 0;
    }

    uint8_t destination_ip_address[] = {255, 255, 255, 255};
    uint8_t source_ip_address[] = {0, 0, 0, 0};

    uint8_t destination_mac_address[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    memcpy(frame->options, request_options, sizeof (request_options));

    const char message1[] = "Sending DHCP Discover message...\n";
    HAL_UART_Transmit(&huart2, message1, sizeof(message1), 100);

    udp_transmit((uint8_t*)frame,
                 frame_size,
                 DHCP_UDP_DESTINATION_PORT,
                 DHCP_UDP_SOURCE_PORT,
                 destination_ip_address,
                 source_ip_address,
                 DHCP,
                 destination_mac_address);

    free(frame);

    const char message2[] = "DHCP discover sent and resources freed\n";
    HAL_UART_Transmit(&huart2, message2, sizeof(message2), 100);
}