//
// Created by fedir on 21.12.22.
//
#include "htu.h"

float htu_measure_temperature(void){
    uint8_t data = HTU21_TEMPERATURE_HOLD;
    HAL_I2C_Master_Transmit(&hi2c2, HTU21_WRITE_ADDRESS, &data, 1, 1000);
    uint8_t response_buffer[3];
    HAL_I2C_Master_Receive(&hi2c2, HTU21_READ_ADDRESS, response_buffer, sizeof (response_buffer), 1000);
    uint16_t s_temp = (response_buffer[0] << 8) + response_buffer[1];
    float temperature = ((float)s_temp/(2 << 15)) * 175.72 - 46.85;
    return temperature;
}