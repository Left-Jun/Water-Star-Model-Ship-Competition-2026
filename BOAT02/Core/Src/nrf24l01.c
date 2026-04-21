#include "nrf24l01.h"
#include <string.h>

extern SPI_HandleTypeDef hspi1;  // 使用SPI1

// 地址配置（收发双方必须一致）
const uint8_t TX_ADDRESS[] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
const uint8_t RX_ADDRESS[] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};

// 引脚控制函数（直接使用CubeMX生成的宏）
void NRF24L01_CE_Low(void)   { HAL_GPIO_WritePin(nRF24_CE_GPIO_Port, nRF24_CE_Pin, GPIO_PIN_RESET); }
void NRF24L01_CE_High(void)  { HAL_GPIO_WritePin(nRF24_CE_GPIO_Port, nRF24_CE_Pin, GPIO_PIN_SET); }
void NRF24L01_CSN_Low(void)  { HAL_GPIO_WritePin(nRF24_CSN_GPIO_Port, nRF24_CSN_Pin, GPIO_PIN_RESET); }
void NRF24L01_CSN_High(void) { HAL_GPIO_WritePin(nRF24_CSN_GPIO_Port, nRF24_CSN_Pin, GPIO_PIN_SET); }

// SPI底层字节收发
uint8_t NRF24L01_SPI_SendByte(uint8_t byte) {
    uint8_t rxData;
    HAL_SPI_TransmitReceive(&hspi1, &byte, &rxData, 1, 100);
    return rxData;
}

// 写寄存器
void NRF24L01_WriteReg(uint8_t reg, uint8_t value) {
    NRF24L01_CSN_Low();
    NRF24L01_SPI_SendByte(NRF_CMD_W_REGISTER | reg);
    NRF24L01_SPI_SendByte(value);
    NRF24L01_CSN_High();
}

// 读寄存器
uint8_t NRF24L01_ReadReg(uint8_t reg) {
    uint8_t value;
    NRF24L01_CSN_Low();
    NRF24L01_SPI_SendByte(NRF_CMD_R_REGISTER | reg);
    value = NRF24L01_SPI_SendByte(NRF_CMD_NOP);
    NRF24L01_CSN_High();
    return value;
}

// 写多字节
void NRF24L01_WriteBuf(uint8_t reg, uint8_t *pBuf, uint8_t len) {
    NRF24L01_CSN_Low();
    NRF24L01_SPI_SendByte(NRF_CMD_W_REGISTER | reg);
    for (uint8_t i = 0; i < len; i++) {
        NRF24L01_SPI_SendByte(pBuf[i]);
    }
    NRF24L01_CSN_High();
}

// 读多字节
void NRF24L01_ReadBuf(uint8_t reg, uint8_t *pBuf, uint8_t len) {
    NRF24L01_CSN_Low();
    NRF24L01_SPI_SendByte(NRF_CMD_R_REGISTER | reg);
    for (uint8_t i = 0; i < len; i++) {
        pBuf[i] = NRF24L01_SPI_SendByte(NRF_CMD_NOP);
    }
    NRF24L01_CSN_High();
}

// 模块自检：读SETUP_AW寄存器看是否为默认值0x03（或写入值）
uint8_t NRF24L01_Check(void) {
    uint8_t aw = NRF24L01_ReadReg(NRF_SETUP_AW);
    if (aw == 0x03) return 1;   // 正常
    else return 0;
}

// 初始化NRF24L01（收发双方通用，已修正载荷宽度匹配）
void NRF24L01_Init(void) {
    NRF24L01_CE_Low();
    HAL_Delay(10);

    // 基础配置
    NRF24L01_WriteReg(NRF_SETUP_AW, 0x03);      // 5字节地址
    NRF24L01_WriteReg(NRF_SETUP_RETR, 0x1A);    // 自动重发：500us延时，最多10次
    NRF24L01_WriteReg(NRF_RF_CH, 0x02);         // 频道2 (2402MHz)
    NRF24L01_WriteReg(NRF_RF_SETUP, 0x06);      // 1Mbps，0dBm

    // 使能通道0自动应答和接收地址
    NRF24L01_WriteReg(NRF_EN_AA, 0x01);
    NRF24L01_WriteReg(NRF_EN_RXADDR, 0x01);

    // ★ 关键修正：载荷宽度改为1字节，匹配遥控器发送长度
    NRF24L01_WriteReg(NRF_RX_PW_P0, 3);   // 匹配遥控器发送的3字节数据
    // 写入收发地址（双方必须相同）
    NRF24L01_WriteBuf(NRF_RX_ADDR_P0, (uint8_t*)RX_ADDRESS, 5);
    NRF24L01_WriteBuf(NRF_TX_ADDR, (uint8_t*)TX_ADDRESS, 5);

    // 清空FIFO并清除中断标志
    NRF24L01_WriteReg(NRF_STATUS, 0x70);
    NRF24L01_CSN_Low();
    NRF24L01_SPI_SendByte(NRF_CMD_FLUSH_TX);
    NRF24L01_CSN_High();
    NRF24L01_CSN_Low();
    NRF24L01_SPI_SendByte(NRF_CMD_FLUSH_RX);
    NRF24L01_CSN_High();

    // 上电，进入待机模式（先写TX模式但不使能CE）
    NRF24L01_WriteReg(NRF_CONFIG, 0x0E);
    HAL_Delay(2);
    // 切换到RX模式并上电
    NRF24L01_WriteReg(NRF_CONFIG, 0x0F);
    HAL_Delay(2);
    NRF24L01_CE_High();    // 使能接收
    HAL_Delay(1);
}

// 设置为发送模式
void NRF24L01_TXMode(void) {
    NRF24L01_CE_Low();
    // 配置为TX模式，上电，CRC 2字节
    NRF24L01_WriteReg(NRF_CONFIG, 0x0E);
    NRF24L01_CE_High();
    HAL_Delay(1);
}

// 设置为接收模式
void NRF24L01_RXMode(void) {
    NRF24L01_CE_Low();
    // 配置为RX模式，上电，CRC 2字节
    NRF24L01_WriteReg(NRF_CONFIG, 0x0F);
    NRF24L01_CE_High();
    HAL_Delay(1);
}

// 发送数据包（阻塞式，带超时）
uint8_t NRF24L01_SendPacket(uint8_t *data, uint8_t len) {
    uint8_t status;
    uint32_t timeout = 100000;

    NRF24L01_CE_Low();

    // 清空TX FIFO
    NRF24L01_CSN_Low();
    NRF24L01_SPI_SendByte(NRF_CMD_FLUSH_TX);
    NRF24L01_CSN_High();

    // 写有效载荷
    NRF24L01_CSN_Low();
    NRF24L01_SPI_SendByte(NRF_CMD_W_TX_PAYLOAD);
    for (uint8_t i = 0; i < len; i++) {
        NRF24L01_SPI_SendByte(data[i]);
    }
    NRF24L01_CSN_High();

    // 启动发送（拉高CE至少10us）
    NRF24L01_CE_High();
    HAL_Delay(1);
    NRF24L01_CE_Low();

    // 等待发送完成或达到最大重发次数
    while (timeout--) {
        status = NRF24L01_ReadReg(NRF_STATUS);
        if (status & (NRF_STATUS_TX_DS | NRF_STATUS_MAX_RT)) {
            break;
        }
    }

    // 清除中断标志
    NRF24L01_WriteReg(NRF_STATUS, NRF_STATUS_TX_DS | NRF_STATUS_MAX_RT);

    if (status & NRF_STATUS_TX_DS) {
        return 1; // 发送成功
    } else {
        return 0; // 发送失败（超时或达到最大重发）
    }
}

// 接收数据包（非阻塞，有数据时返回长度，否则返回0）
uint8_t NRF24L01_ReceivePacket(uint8_t *data) {
    uint8_t status = NRF24L01_ReadReg(NRF_STATUS);
    uint8_t len = 0;

    // 检查是否有数据
    if (status & NRF_STATUS_RX_DR) {
        // 读取载荷宽度（可选）
        NRF24L01_CSN_Low();
        NRF24L01_SPI_SendByte(NRF_CMD_R_RX_PL_WID);
        len = NRF24L01_SPI_SendByte(NRF_CMD_NOP);
        NRF24L01_CSN_High();

        // 限制长度防止溢出
        if (len > 32) len = 32;

        // 读取载荷
        NRF24L01_CSN_Low();
        NRF24L01_SPI_SendByte(NRF_CMD_R_RX_PAYLOAD);
        for (uint8_t i = 0; i < len; i++) {
            data[i] = NRF24L01_SPI_SendByte(NRF_CMD_NOP);
        }
        NRF24L01_CSN_High();

        // 清除 RX_DR 标志（必须）
        NRF24L01_WriteReg(NRF_STATUS, NRF_STATUS_RX_DR);

        return len;
    }
    return 0;
}

