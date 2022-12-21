/*
 * enc28j60.c
 *
 *  Created on: Dec 11, 2022
 *      Author: mtgin
 */

#include "enc28j60.h"
#include <stdint.h>

static enc28j60_reg_bank current_bank = BANK_0;

static uint16_t current_ptr = ENC28J60_RX_BUF_START;

static uint8_t command_op_codes[COMMANDS_NUM] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x07};

uint8_t mac_address[MAC_ADDRESS_BYTES_NUM] = {0x00, 0x17, 0x22, 0xED, 0xA5, 0x01};

uint8_t ip_address[IP_ADDRESS_BYTES_NUM] = {192, 168, 0, 133};

static uint8_t read_control_reg(uint8_t reg);
static uint16_t read_control_reg_pair(uint8_t reg);
static void read_buffer_mem(uint8_t *data, uint16_t data_size);

static void write_control_reg(uint8_t reg, uint8_t reg_data);
static void write_control_reg_pair(uint8_t reg, uint16_t reg_data);
static void write_buffer_mem(uint8_t *data, uint16_t data_size);

static uint16_t read_phy_reg(uint8_t reg);
static void write_phy_reg(uint8_t reg, uint16_t reg_data);

static void bit_field_set(uint8_t reg, uint8_t reg_data);
static void bit_field_clear(uint8_t reg, uint8_t reg_data);

static void system_reset(void);

static void write_command(enc28j60_command command, uint8_t arg_data);

static void set_cs(enc28j60_cs_state state);
static void write_byte(uint8_t data);
static void write_bytes(uint8_t *data, uint8_t size);
static uint8_t read_byte(void);

static uint8_t get_reg_addr(uint8_t reg);
static enc28j60_reg_bank get_reg_bank(uint8_t reg);
static enc28j60_reg_type get_reg_type(uint8_t reg);
static void check_bank(uint8_t reg);

static enc28j60_reg_type get_reg_type(uint8_t reg){
    // Applies a mask to a register, and shift to right to get a raw type value
    enc28j60_reg_type type = (enc28j60_reg_type) ((reg & ENC28J60_REG_TYPE_MASK) >> ENC28J60_REG_TYPE_OFFSET);

    return type;
}

static enc28j60_reg_bank get_reg_bank(uint8_t reg){
    enc28j60_reg_bank bank = (enc28j60_reg_bank) ((reg & ENC28J60_REG_BANK_MASK) >> ENC28J60_REG_BANK_OFFSET);
    return bank;
}

static uint8_t get_reg_addr(uint8_t reg){
    return (reg & ENC28J60_REG_ADDR_MASK);
}

static void set_cs(enc28j60_cs_state state){
    HAL_GPIO_WritePin(ENC28J60_CHIP_SELECT_GPIO_Port, ENC28J60_CHIP_SELECT_Pin, (GPIO_PinState)state);
}

static void write_bytes(uint8_t *data, uint8_t size){
    HAL_SPI_Transmit(&hspi1, data, size, ENC28J60_SPI_TIMEOUT);
}

static void write_byte(uint8_t data){
    HAL_SPI_Transmit(&hspi1, &data, 1, ENC28J60_SPI_TIMEOUT);
}

static uint8_t read_byte(void){
    uint8_t tx_data = 0x00;
    uint8_t rx_data = 0x00;
    HAL_SPI_TransmitReceive(&hspi1, &tx_data, &rx_data, 1, ENC28J60_SPI_TIMEOUT);
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

void transmit_frame(uint8_t *data, uint16_t size){
    while((read_control_reg(ECON1) & ECON1_TXRTS_BIT) != 0){
        if((read_control_reg(EIR) & EIR_TXERIF_BIT) != 0){
            bit_field_set(ECON1, ECON1_TXRST_BIT);
            bit_field_clear(ECON1, ECON1_TXRST_BIT);
        }
    }

    write_control_reg_pair(EWRPTL, ENC28J60_TX_BUF_START);

    uint8_t control_bytes = 0x00;
    write_buffer_mem(&control_bytes, 1);
    write_buffer_mem(data, size);

    write_control_reg_pair(ETXSTL, ENC28J60_TX_BUF_START);
    write_control_reg_pair(ETXNDL, ENC28J60_TX_BUF_START + size);

    bit_field_set(ECON1, ECON1_TXRTS_BIT);
}

uint16_t receive_frame(enc28j60_frame_mask *frame){
    uint16_t data_size = 0;
    uint8_t packets_count = read_control_reg(EPKTCNT);

    if(packets_count == 0){
        return data_size;
    }

    uint8_t receive_message_log[] = "There are some packets in buffer\n";

    HAL_UART_Transmit(&huart2, receive_message_log, sizeof(receive_message_log), 100);

    write_control_reg_pair(ERDPTL, current_ptr);

    read_buffer_mem((uint8_t *)frame, ENC28J60_HEADER_SIZE);

    current_ptr = frame->nextPtr;

    if((frame->status & ENC28J60_FRAME_RX_OK_MASK) != 0){
        data_size = frame->length - ENC28J60_CRC_SIZE;

        if(data_size > ENC28J60_FRAME_DATA_MAX){
            data_size = ENC28J60_FRAME_DATA_MAX;
        }

        read_buffer_mem((uint8_t*)&(frame->data[0]), data_size);
        read_buffer_mem((uint8_t*)&(frame->checkSum), ENC28J60_CRC_SIZE);
    }

    uint16_t next_ptr = frame->nextPtr - 1;
    if(next_ptr > ENC28J60_RX_BUF_END){
        next_ptr = ENC28J60_RX_BUF_END;
    }

    write_control_reg_pair(ERXRDPTL, next_ptr);
    bit_field_set(ECON2, ECON2_PKTDEC_BIT);

    return data_size;
}

static void write_command(enc28j60_command command, uint8_t arg_data){
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
    enc28j60_reg_type reg_type = get_reg_type(reg);
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

static void read_buffer_mem(uint8_t *data, uint16_t data_size){
    set_cs(CS_LOW);

    write_command(READ_BUFFER_MEM, ENC28J60_BUF_COMMAND_ARG);
    for(int i = data_size; i > 0; i--){
        *data = read_byte();
        data++;
    }

    set_cs(CS_HIGH);
}

static void write_buffer_mem(uint8_t *data, uint16_t data_size){
    set_cs(CS_LOW);

    write_command(WRITE_BUFFER_MEM, ENC28J60_BUF_COMMAND_ARG);
    write_bytes(data, data_size);

    set_cs(CS_HIGH);
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

    enc28j60_reg_bank reg_bank = get_reg_bank(reg);

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

void initialize_enc28j60(void){
    HAL_GPIO_WritePin(ENC28J60_RESET_GPIO_Port, ENC28J60_RESET_Pin, GPIO_PIN_RESET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(ENC28J60_RESET_GPIO_Port, ENC28J60_RESET_Pin, GPIO_PIN_SET);
    HAL_Delay(50);

    system_reset();

    write_control_reg_pair(ERXSTL, ENC28J60_RX_BUF_START);
    write_control_reg_pair(ERXNDL, ENC28J60_RX_BUF_END);
    write_control_reg_pair(ERDPTL, ENC28J60_RX_BUF_START);

    bit_field_clear(ERXFCON, ERXFCON_UCEN_BIT | ERXFCON_ANDOR_BIT | ERXFCON_CRCEN_BIT | ERXFCON_PMEN_BIT | ERXFCON_MPEN_BIT | ERXFCON_HTEN_BIT | ERXFCON_MCEN_BIT | ERXFCON_BCEN_BIT);

    write_control_reg(MACON1, MACON1_MARXEN_BIT);
    write_control_reg(MACON4, MACON4_DEFER);

    write_control_reg_pair(MAIPGL, ENC28J60_NBB_PACKET_GAP);
    write_control_reg(MABBIPG, ENC28J60_BB_PACKET_GAP);

    write_control_reg(MACON3, MACON3_PADCFG0_BIT | MACON3_TXCRCEN_BIT | MACON3_FRMLNEN_BIT);

    write_control_reg_pair(MAMXFLL, ENC28J60_FRAME_DATA_MAX);

    write_control_reg(MAADR1, mac_address[0]);
    write_control_reg(MAADR2, mac_address[1]);
    write_control_reg(MAADR3, mac_address[2]);
    write_control_reg(MAADR4, mac_address[3]);
    write_control_reg(MAADR5, mac_address[4]);
    write_control_reg(MAADR6, mac_address[5]);

    write_phy_reg(PHCON2, PHCON2_HDLDIS_BIT);

    start_enc28j60_receiving();
}

void start_enc28j60_receiving(void){
    bit_field_set(ECON1, ECON1_RXEN_BIT);
}
