//
// Created by fedir on 16.12.22.
//
#include "dhcp.h"
#include <stdlib.h>
#include <string.h>
#include "usart.h"

static const char host_name[] = "STM32F411";

static void add_to_buffer(uint8_t *buf, uint16_t *counter, uint8_t value);
static void fill_request_buffer(uint8_t *buf, uint16_t *counter);

void dhcp_client_process(dhcp_frame_mask *frame, uint16_t frame_length){
    const char message[] = "Caught DHCP client message";
    HAL_UART_Transmit(&huart2, message, sizeof (message), 100);
}

void dhcp_server_process(dhcp_frame_mask *frame, uint16_t frame_length){
    const char message[] = "Caught DHCP server message";
    HAL_UART_Transmit(&huart2, message, sizeof (message), 100);
}

static void add_to_buffer(uint8_t *buf, uint16_t *counter, uint8_t value){
    *buf++ = value;
    *counter++;
}

static void fill_request_buffer(uint8_t *buf, uint16_t *counter){
    //Magic cookie
    add_to_buffer(buf, counter, DHCP_MAGIC_COOKIE_0);
    add_to_buffer(buf, counter, DHCP_MAGIC_COOKIE_1);
    add_to_buffer(buf, counter, DHCP_MAGIC_COOKIE_2);
    add_to_buffer(buf, counter, DHCP_MAGIC_COOKIE_3);

    //Message type
    add_to_buffer(buf, counter, DHCP_OPTION_DHCP_MSG_TYPE);
    add_to_buffer(buf, counter, 1);
    add_to_buffer(buf, counter, 1);

    //Client identifier
    add_to_buffer(buf, counter, DHCP_OPTION_CLIENT_ID);
    add_to_buffer(buf, counter, sizeof (mac_address) + 1);
    add_to_buffer(buf, counter, 0x01);

    for(int i = 0; i < sizeof (mac_address); i++){
        add_to_buffer(buf, counter, mac_address[i]);
    }

    //Requested IP
    add_to_buffer(buf, counter, DHCP_OPTION_REQUESTED_IP);
    add_to_buffer(buf, counter, sizeof (ip_address));

    for(int i = 0; i < sizeof (ip_address); i++){
        add_to_buffer(buf, counter, ip_address[i]);
    }

    //Host name
    add_to_buffer(buf, counter, DHCP_OPTION_HOST_NAME);
    add_to_buffer(buf, counter, sizeof (host_name));

    for(int i = 0; i < sizeof (host_name); i++){
        add_to_buffer(buf, counter, host_name[i]);
    }

    //Parameter request list
    add_to_buffer(buf, counter, DHCP_OPTION_PARAMETER_REQUEST_LIST);
    add_to_buffer(buf, counter, 4);
    add_to_buffer(buf, counter, DHCP_OPTION_PARAMETER_REQUEST_SUBNET_MASK);
    add_to_buffer(buf, counter, DHCP_OPTION_PARAMETER_REQUEST_ROUTE_GATEWAY);
    add_to_buffer(buf, counter, DHCP_OPTION_PARAMETER_DOMAIN_NAME);
    add_to_buffer(buf, counter, DHCP_OPTION_PARAMETER_DNS);

    //End mark
    add_to_buffer(buf, counter, DHCP_OPTION_END_MARK);
}

void dhcp_discover(void){

    uint8_t buff[ENC28J60_FRAME_DATA_MAX] = {0};
    uint8_t request_options_buffer[100] = {0};
    uint16_t request_options_buffer_counter = 0;

    fill_request_buffer(request_options_buffer, &request_options_buffer_counter);

    uint32_t xid = rand();

    uint16_t frame_size = sizeof (dhcp_frame_mask) + request_options_buffer_counter;

    uint8_t *dhcp_buf_start = buff + (ENC28J60_FRAME_DATA_MAX - frame_size);

    dhcp_frame_mask *frame = (dhcp_frame_mask *)dhcp_buf_start;

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

    memcpy(frame->options, request_options_buffer, request_options_buffer_counter);

    udp_transmit(dhcp_buf_start,
                 frame_size,
                 DHCP_UDP_DESTINATION_PORT,
                 DHCP_UDP_SOURCE_PORT,
                 destination_ip_address,
                 source_ip_address,
                 DHCP_CLIENT,
                 destination_mac_address);
}