//
// Created by fedir on 21.12.22.
//

#include "logger.h"

static uint8_t eth_init_start_message[] = "Initializing Ethernet\n";
static uint8_t eth_init_finish_message[] = "Ethernet initialization finished\n";
static uint8_t arp_search_started_message[] = "ARP request with server target sent to network\n";
static uint8_t dhcp_client_process_message[] = "Caught DHCP client message";
static uint8_t dhcp_server_process_message[] = "Caught DHCP server message";
uint8_t enc28j60_receive_message[] = "There are some packets in buffer\n";

void log_eth_init_start(void){
    log_message(eth_init_start_message, sizeof(eth_init_start_message));
}

void log_eth_init_finish(void){
    log_message(eth_init_finish_message, sizeof(eth_init_finish_message));
}

void log_arp_search_started(void){
    log_message(arp_search_started_message, sizeof (arp_search_started_message));
}

void log_dhcp_client_request(void){
    log_message(dhcp_client_process_message, sizeof (dhcp_client_process_message));
}

void log_dhcp_server_request(void){
    log_message(dhcp_server_process_message, sizeof (dhcp_server_process_message));
}

void log_enc28j60_has_packages_in_buffer(void){
    log_message(enc28j60_receive_message, sizeof (enc28j60_receive_message));
}

void log_message(uint8_t *message, uint16_t size){
    HAL_UART_Transmit(&huart2, message, size, USART_LOG_TIMEOUT);
}