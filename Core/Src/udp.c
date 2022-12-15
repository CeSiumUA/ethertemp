//
// Created by fedir on 15.12.22.
//
#include "udp.h"
#include <stdlib.h>

uint16_t udp_process(UDP_Frame *udp_frame, uint16_t frame_length){

}

void udp_transmit(uint8_t *data, uint16_t data_length){
    UDP_Frame *frame = malloc(sizeof (UDP_Frame) + data_length);

    frame->length = sizeof (UDP_Frame) + data_length;
}