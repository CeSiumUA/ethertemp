//
// Created by fedir on 15.12.22.
//
#include "udp.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

//OPTIMIZE

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

uint16_t calculate_checksum(udp_frame_mask *frame){
    //TODO going to implement it in the future
    return frame->checksum;
}

static void pong(udp_frame_mask *frame){
    uint16_t dst_port = frame->src_port;
    frame->src_port = frame->dst_port;
    frame->dst_port = dst_port;
}

uint16_t udp_process(udp_frame_mask *udp_frame, uint16_t frame_length){
    uint16_t rx_checksum = udp_frame->checksum;
    udp_frame -> checksum = 0;

    uint16_t package_dst_port = ntohs(udp_frame->dst_port);

    bool port_allowed = false;

    for(int i = 0; i < sizeof(ports_whitelist); i++){
        if(package_dst_port == ports_whitelist[i]){
            port_allowed = true;
        }
    }

    if(!port_allowed) return 0;

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
    frame -> checksum = calculate_checksum(frame);

    set_port(src_port, package_type);

    ip_transmit(data, overall_length, dst_address, src_address, IP_FRAME_PROTOCOL_UDP, dest_mac_address);
}
