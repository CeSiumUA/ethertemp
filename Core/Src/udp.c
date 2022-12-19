//
// Created by fedir on 15.12.22.
//
#include "udp.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static void pong(udp_frame_mask *frame);
static udp_consumed_port *get_port(uint16_t port_number);
static udp_consumed_port *set_port(uint16_t port_number, udp_package_type package_type);

static udp_consumed_port *udp_reserved_ports = NULL;
static uint16_t ports_whitelist[] = {67, 68, 25512};

static udp_consumed_port *create_port(uint16_t port_number, udp_package_type package_type){
    udp_consumed_port *reserved_port = malloc(sizeof (udp_reserved_ports));
    reserved_port->package_type = package_type;
    reserved_port->port  = port_number;
    reserved_port->next = NULL;

    return reserved_port;
}

static udp_consumed_port *get_port(uint16_t port_number){
    udp_consumed_port *port = udp_reserved_ports;
    while(port != NULL && port -> port != port_number){
        port = port->next;
    }

    return port;
}

static udp_consumed_port *set_port(uint16_t port_number, udp_package_type package_type){

    udp_consumed_port *port = udp_reserved_ports;
    if(port == NULL){
        udp_reserved_ports = create_port(port_number, package_type);
        return udp_reserved_ports;
    }

    while (port->port != port_number){
        if(port->next == NULL){
            port -> next = create_port(port_number, package_type);
        }
        port = port->next;
    }

    port->package_type = package_type;

    return port;
}

static void pong(udp_frame_mask *frame){
    uint16_t dst_port = frame->src_port;
    frame->src_port = frame->dst_port;
    frame->dst_port = dst_port;
}

uint16_t udp_process(udp_frame_mask *udp_frame, uint8_t src_ip_address[IP_ADDRESS_BYTES_NUM], uint16_t frame_length){
    uint16_t rx_checksum = udp_frame->checksum;
    udp_frame -> checksum = 0;

    uint16_t udp_frame_length = ntohs(udp_frame->length);

    uint16_t package_dst_port = ntohs(udp_frame->dst_port);

    bool port_allowed = false;

    for(int i = 0; i < sizeof(ports_whitelist); i++){
        if(package_dst_port == ports_whitelist[i]){
            port_allowed = true;
        }
    }

    if(!port_allowed) return 0;

    uint16_t pseudo_header_buf_length = sizeof (udp_ipv4_pseudo_header) + udp_frame_length;
    uint8_t pseudo_header_buf[pseudo_header_buf_length];

    udp_ipv4_pseudo_header *pseudo_header = (udp_ipv4_pseudo_header *)pseudo_header_buf;
    memcpy(pseudo_header->data, (uint8_t *)udp_frame, udp_frame_length);
    pseudo_header->zeros = 0;
    pseudo_header->protocol = 0x11;
    pseudo_header->udp_length = udp_frame->length;
    memcpy(pseudo_header->src_ip_addr, src_ip_address, IP_ADDRESS_BYTES_NUM);
    memcpy(pseudo_header->dest_ip_addr, ip_address, IP_ADDRESS_BYTES_NUM);

    uint16_t calculated_checksum = ip_calculate_checksum(pseudo_header_buf, pseudo_header_buf_length);
    if(calculated_checksum != rx_checksum){
        return 0;
    }

    udp_consumed_port *consumed_port = get_port(package_dst_port);
    if(consumed_port == NULL){
        consumed_port = set_port(package_dst_port, (udp_package_type) package_dst_port);
    }

    switch (consumed_port->package_type) {
        case PING_PONG:
            pong(udp_frame);
            break;
        case DHCP_SERVER:
            dhcp_server_process((dhcp_frame_mask *)udp_frame->data, udp_frame->length - sizeof (udp_frame_mask));
        case DHCP_CLIENT:
            dhcp_client_process((dhcp_frame_mask *)udp_frame->data, udp_frame->length - sizeof (udp_frame_mask));
            break;
        case NONE:
        default:
            pong(udp_frame);
            break;
    }

    return frame_length;
}

void udp_transmit(uint8_t *data, uint16_t data_length, uint16_t dst_port, uint16_t src_port, uint8_t dst_address[IP_ADDRESS_BYTES_NUM], uint8_t src_address[IP_ADDRESS_BYTES_NUM], udp_package_type package_type, uint8_t dest_mac_address[MAC_ADDRESS_BYTES_NUM]){
    uint16_t overall_length = sizeof (udp_frame_mask) + data_length;
    data -= sizeof (udp_frame_mask);
    udp_frame_mask *frame = (udp_frame_mask *)data;

    frame->length = htons(overall_length);
    frame->src_port = htons(src_port);
    frame->dst_port = htons(dst_port);
    frame -> checksum = 0;

    udp_ipv4_pseudo_header *pseudo_header_ptr = (udp_ipv4_pseudo_header *)(data - sizeof (udp_ipv4_pseudo_header));
    memcpy(pseudo_header_ptr->src_ip_addr, src_address, IP_ADDRESS_BYTES_NUM);
    memcpy(pseudo_header_ptr->dest_ip_addr, dest_mac_address, IP_ADDRESS_BYTES_NUM);
    pseudo_header_ptr->zeros = 0;
    pseudo_header_ptr->protocol = 0x11;
    pseudo_header_ptr->udp_length = htons(overall_length);

    frame -> checksum = ip_calculate_checksum((uint8_t *)pseudo_header_ptr, sizeof (udp_ipv4_pseudo_header) + overall_length);

    set_port(src_port, package_type);

    ip_transmit(data, overall_length, dst_address, src_address, IP_FRAME_PROTOCOL_UDP, dest_mac_address);
}
