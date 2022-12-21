//
// Created by fedir on 21.12.22.
//

#ifndef ETHERNET_TEST_HTU_H
#define ETHERNET_TEST_HTU_H

#include "main.h"
#include "i2c.h"

#define HTU21_WRITE_ADDRESS         0x80
#define HTU21_READ_ADDRESS          0x81

#define HTU21_TEMPERATURE_HOLD      0xE3
#define HTU21_TEMPERATURE_NO_HOLD   0xF3

#define HTU21_HUMIDITY_HOLD         0xE5
#define HTU21_HUMIDITY_NO_HOLD      0xF5

#define HTU21_WRITE_USER_REGISTER   0xE6
#define HTU21_READ_USER_REGISTER    0xE7
#define HTU21_SOFT_RESET            0xFE

float htu_measure_temperature(void);

#endif //ETHERNET_TEST_HTU_H
