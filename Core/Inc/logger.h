//
// Created by fedir on 21.12.22.
//

#ifndef ETHERNET_TEST_LOGGER_H
#define ETHERNET_TEST_LOGGER_H

#include "usart.h"
#include "main.h"

#define USART_LOG_TIMEOUT   100

void log_eth_init_start(void);
void log_eth_init_finish(void);
void log_message(uint8_t *message, uint16_t size);
void log_arp_search_started(void);
void log_dhcp_client_request(void);
void log_dhcp_server_request(void);
void log_enc28j60_has_packages_in_buffer(void);

#endif //ETHERNET_TEST_LOGGER_H
