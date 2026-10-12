#ifndef INC_ZIGBEE_E18_H_
#define INC_ZIGBEE_E18_H_

#include "main.h"
#include <stdint.h>

#define RX_BUF_SIZE 256

extern volatile uint8_t rx_buffer[RX_BUF_SIZE];
extern volatile uint16_t rx_head;
extern volatile uint16_t rx_tail;
extern uint8_t rx_byte;

// Các hàm API chính
void E18_Gateway_Init(void);
uint8_t E18_Broadcast_Data(uint8_t* data, uint8_t data_len);

// Các hàm xử lý Buffer
uint16_t E18_Available(void);
uint8_t E18_ReadByte(void);
void E18_ClearBuffer(void);

// Lệnh nội bộ
uint8_t E18_Get_NetStatus(void);
void Log_Hex_Compact(const char* prefix, uint8_t* data, uint8_t len);
uint8_t E18_ExecCmd_With_Log(uint8_t type, uint8_t code, uint8_t *data, uint8_t data_len, uint32_t timeout);

#endif /* INC_ZIGBEE_E18_H_ */
