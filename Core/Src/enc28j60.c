/*
 * enc28j60.c
 *
 *  Created on: Dec 11, 2022
 *      Author: mtgin
 */

#include "enc28j60.h"
#include <stdint.h>

#pragma region Functions_Headers
static uint8_t read_control_reg(uint8_t reg);
static uint16_t read_control_reg_pair(uint8_t reg);
static void read_buffer_mem(uint8_t *data, size_t data_size);

static void write_control_reg(uint8_t reg, uint8_t reg_data);
static void write_control_reg_pair(uint8_t reg, uint16_t reg_data);
static void write_buffer_mem(uint8_t *data, size_t data_size);

static uint16_t read_phy_reg(uint8_t reg);
static void write_phy_reg(uint8_t reg, uint16_t reg_data);

static void bit_field_set(uint8_t reg, uint8_t reg_data);
static void bit_field_clear(uint8_t reg, uint8_t reg_data);

static void system_reset(void);

static void write_command(ENC28J60_Command command, uint8_t arg_data);

static void set_cs(ENC28J60_CS_State state);
static void write_byte(uint8_t data);
static void write_bytes(uint8_t *data, size_t size);
static uint8_t read_byte(void);

static uint8_t get_reg_addr(uint8_t reg);
static ENC28J60_RegBank get_reg_bank(uint8_t reg);
static ENC28J60_RegType get_reg_type(uint8_t reg);
static void check_bank(uint8_t reg);
#pragma endregion

static ENC28J60_RegBank current_bank = BANK_0;

static uint8_t command_op_codes[COMMANDS_NUM] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x07};

static ENC28J60_RegType get_reg_type(uint8_t reg){
    // Applies a mask to a register, and shift to right to get a raw type value
    ENC28J60_RegType type = (ENC28J60_RegType) ((reg & ENC28J60_REG_TYPE_MASK) >> ENC28J60_REG_TYPE_OFFSET);

    return type;
}

static ENC28J60_RegBank get_reg_bank(uint8_t reg){
    ENC28J60_RegBank bank = (ENC28J60_RegBank) ((reg & ENC28J60_REG_BANK_MASK) >> ENC28J60_REG_BANK_OFFSET);
}

static uint8_t get_reg_addr(uint8_t reg){
    return (reg & ENC28J60_REG_ADDR_MASK);
}

static void set_cs(ENC28J60_CS_State state){
    HAL_GPIO_WritePin(ENC28J60_CHIP_SELECT_GPIO_Port, ENC28J60_CHIP_SELECT_Pin, (GPIO_PinState)state);
}

static void write_bytes(uint8_t *data, size_t size){
    HAL_SPI_Transmit(&hspi5, data, size, ENC28J60_SPI_TIMEOUT);
}

static void write_byte(uint8_t data){
    HAL_SPI_Transmit(&hspi5, &data, 1, ENC28J60_SPI_TIMEOUT);
}

static uint8_t read_byte(void){
    uint8_t tx_data = 0x00;
    uint8_t rx_data = 0x00;
    //FIXME
    HAL_SPI_TransmitReceive(&hspi5, &tx_data, &rx_data, 1, ENC28J60_SPI_TIMEOUT);
    return rx_data;
}

static void bit_field_set(uint8_t reg, uint8_t reg_data){
    uint8_t reg_addr = get_reg_addr(reg);

    check_bank(reg);

    set_cs(CS_LOW);
    write_command(BIT_FIELD_SET, reg_addr);
    write_byte(reg_data);
    set_cs(CS_HIGH);
}

static void bit_field_clear(uint8_t reg, uint8_t reg_data){
    uint8_t reg_addr = get_reg_addr(reg);

    check_bank(reg);

    set_cs(CS_LOW);
    write_command(BIT_FIELD_CLEAR, reg_addr);
    write_byte(reg_data);
    set_cs(CS_HIGH);
}

static uint16_t read_phy_reg(uint8_t reg){
    uint16_t data = 0;
    uint8_t reg_addr = get_reg_addr(reg);

    write_control_reg(MIREGADR, reg_addr);
    bit_field_set(MICMD, MICMD_MIIRD_BIT);

    while((read_control_reg(MISTAT) & MISTAT_BUSY_BIT) != 0){}

    bit_field_clear(MICMD, MICMD_MIIRD_BIT);
    data = read_control_reg_pair(MIRDL);

    return data;
}

static void write_phy_reg(uint8_t reg, uint16_t reg_data){
    uint8_t reg_addr = get_reg_addr(reg);

    write_control_reg(MIREGADR, reg_addr);
    write_control_reg_pair(MIWRL, reg_data);

    while((read_control_reg(MISTAT) & MISTAT_BUSY_BIT) != 0){}
}

static void write_command(ENC28J60_Command command, uint8_t arg_data){
    uint8_t data = 0;
    data = (command_op_codes[command] << ENC28J60_OP_CODE_OFFSET) | arg_data;
    write_byte(data);
}

static uint16_t read_control_reg_pair(uint8_t reg){
    uint16_t data = 0;
    data = (uint16_t)read_control_reg(reg) | ((uint16_t) read_control_reg(reg + 1) << 8);
    return data;
}

static void write_control_reg_pair(uint8_t reg, uint16_t reg_data){
    write_control_reg(reg, (uint8_t)reg_data);
    write_control_reg(reg + 1, (uint8_t)(reg_data >> 8));
}

static uint8_t read_control_reg(uint8_t reg){
    uint8_t data = 0;
    ENC28J60_RegType reg_type = get_reg_type(reg);
    uint8_t reg_addr = get_reg_addr(reg);

    check_bank(reg);

    set_cs(CS_LOW);
    write_command(READ_CONTROL_REG, reg_addr);

    if(reg_type == MAC_MII_REG){
        read_byte();
    }

    data = read_byte();

    set_cs(CS_HIGH);

    return data;
}

static void write_control_reg(uint8_t reg, uint8_t reg_data){
    uint8_t reg_addr = get_reg_addr(reg);

    check_bank(reg);

    set_cs(CS_LOW);

    write_command(WRITE_CONTROL_REG, reg_addr);
    write_byte(reg_data);

    set_cs(CS_HIGH);
}

static void read_buffer_mem(uint8_t *data, size_t data_size){
    set_cs(CS_LOW);

    write_command(READ_BUFFER_MEM, ENC28J60_BUF_COMMAND_ARG);
    for(int i = data_size; i > 0; i--){
        *data = read_byte();
        data++;
    }

    set_cs(CS_HIGH);
}

static void write_buffer_mem(uint8_t *data, size_t data_size){
    set_cs(CS_LOW);

    write_command(WRITE_BUFFER_MEM, ENC28J60_BUF_COMMAND_ARG);
    write_bytes(data, data_size);
}

static void system_reset(void){
    set_cs(CS_LOW);
    write_command(SYSTEM_RESET, ENC28J60_RESET_COMMAND_ARG);
    set_cs(CS_HIGH);

    current_bank = BANK_0;
    HAL_Delay(1);
}

static void check_bank(uint8_t reg){
    uint8_t addr = get_reg_addr(reg);

    if(addr >= ENC28J60_COMMON_REGS_ADDR){
        return;
    }

    ENC28J60_RegBank reg_bank = get_reg_bank(reg);

    if(current_bank == reg_bank){
        return;
    }

    uint8_t econ1_addr = get_reg_addr(ECON1);

    //Clear bank bits
    set_cs(CS_LOW);
    write_command(BIT_FIELD_CLEAR, econ1_addr);
    write_byte(ECON1_BSEL1_BIT | ECON1_BSEL0_BIT);
    set_cs(CS_HIGH);

    //Set bank bits
    set_cs(CS_LOW);
    write_command(BIT_FIELD_SET, econ1_addr);
    write_byte(reg_bank);
    set_cs(CS_HIGH);

    current_bank = reg_bank;
}