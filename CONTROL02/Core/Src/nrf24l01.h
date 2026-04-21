#ifndef NRF24L01_H
#define NRF24L01_H

#include "main.h"

// NRF24L01 寄存器地址定义
#define NRF_CONFIG      0x00
#define NRF_EN_AA       0x01
#define NRF_EN_RXADDR   0x02
#define NRF_SETUP_AW    0x03
#define NRF_SETUP_RETR  0x04
#define NRF_RF_CH       0x05
#define NRF_RF_SETUP    0x06
#define NRF_STATUS      0x07
#define NRF_OBSERVE_TX  0x08
#define NRF_CD          0x09
#define NRF_RX_ADDR_P0  0x0A
#define NRF_RX_ADDR_P1  0x0B
#define NRF_RX_ADDR_P2  0x0C
#define NRF_RX_ADDR_P3  0x0D
#define NRF_RX_ADDR_P4  0x0E
#define NRF_RX_ADDR_P5  0x0F
#define NRF_TX_ADDR     0x10
#define NRF_RX_PW_P0    0x11
#define NRF_RX_PW_P1    0x12
#define NRF_RX_PW_P2    0x13
#define NRF_RX_PW_P3    0x14
#define NRF_RX_PW_P4    0x15
#define NRF_RX_PW_P5    0x16
#define NRF_FIFO_STATUS 0x17
#define NRF_DYNPD       0x1C
#define NRF_FEATURE     0x1D

// 指令定义
#define NRF_CMD_R_REGISTER      0x00
#define NRF_CMD_W_REGISTER      0x20
#define NRF_CMD_R_RX_PAYLOAD    0x61
#define NRF_CMD_W_TX_PAYLOAD    0xA0
#define NRF_CMD_FLUSH_TX        0xE1
#define NRF_CMD_FLUSH_RX        0xE2
#define NRF_CMD_REUSE_TX_PL     0xE3
#define NRF_CMD_R_RX_PL_WID     0x60
#define NRF_CMD_W_ACK_PAYLOAD   0xA8
#define NRF_CMD_W_TX_PAYLOAD_NOACK 0xB0
#define NRF_CMD_NOP             0xFF

// STATUS寄存器标志位
#define NRF_STATUS_RX_DR    0x40
#define NRF_STATUS_TX_DS    0x20
#define NRF_STATUS_MAX_RT   0x10
#define NRF_STATUS_TX_FULL  0x01

// FIFO_STATUS寄存器标志位
#define NRF_FIFO_TX_REUSE   0x40
#define NRF_FIFO_TX_FULL    0x20
#define NRF_FIFO_TX_EMPTY   0x10
#define NRF_FIFO_RX_FULL    0x02
#define NRF_FIFO_RX_EMPTY   0x01

// 函数声明
void NRF24L01_Init(void);
uint8_t NRF24L01_Check(void);
void NRF24L01_TXMode(void);
void NRF24L01_RXMode(void);
uint8_t NRF24L01_SendPacket(uint8_t *data, uint8_t len);
uint8_t NRF24L01_ReceivePacket(uint8_t *data);
void NRF24L01_WriteReg(uint8_t reg, uint8_t value);
uint8_t NRF24L01_ReadReg(uint8_t reg);
void NRF24L01_WriteBuf(uint8_t reg, uint8_t *pBuf, uint8_t len);
void NRF24L01_ReadBuf(uint8_t reg, uint8_t *pBuf, uint8_t len);
void NRF24L01_CE_Low(void);
void NRF24L01_CE_High(void);
void NRF24L01_CSN_Low(void);
void NRF24L01_CSN_High(void);
uint8_t NRF24L01_SPI_SendByte(uint8_t byte);

#endif
