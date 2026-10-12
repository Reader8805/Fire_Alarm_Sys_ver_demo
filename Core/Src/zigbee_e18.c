#include "zigbee_e18.h"
#include <string.h>
#include <stdio.h>

extern UART_HandleTypeDef huart3;
extern void LCD_Log(const char* msg);

volatile uint8_t rx_buffer[RX_BUF_SIZE];
volatile uint16_t rx_head = 0, rx_tail = 0;
uint8_t rx_byte;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART3) {
        rx_buffer[rx_head] = rx_byte;
        rx_head = (rx_head + 1) % RX_BUF_SIZE;
        HAL_UART_Receive_IT(&huart3, &rx_byte, 1);
    }
}

uint8_t E18_ReadByte(void) {
    if (rx_head == rx_tail) return 0;
    uint8_t data = rx_buffer[rx_tail];
    rx_tail = (rx_tail + 1) % RX_BUF_SIZE;
    return data;
}

uint16_t E18_Available(void) {
    return (RX_BUF_SIZE + rx_head - rx_tail) % RX_BUF_SIZE;
}

void E18_ClearBuffer(void) { rx_head = rx_tail = 0; }

void Log_Hex_Compact(const char* prefix, uint8_t* data, uint8_t len) {
    char buf[21] = {0};
    int pos = snprintf(buf, sizeof(buf), "%s", prefix);
    for(int i = 0; i < len; i++) {
        if (pos + 2 < sizeof(buf)) {
            pos += snprintf(buf + pos, sizeof(buf) - pos, "%02X", data[i]);
        } else break;
    }
    LCD_Log(buf);
}

uint8_t E18_ExecCmd_With_Log(uint8_t type, uint8_t code, uint8_t *data, uint8_t data_len, uint32_t timeout) {
    uint8_t frame[64];
    uint8_t len = 3 + data_len;

    frame[0] = 0x55; frame[1] = len; frame[2] = type; frame[3] = code;
    uint8_t cs = type ^ code;
    for (int i = 0; i < data_len; i++) {
        frame[4 + i] = data[i];
        cs ^= data[i];
    }
    frame[4 + data_len] = cs;

    Log_Hex_Compact("TX:", frame, 5 + data_len);
    E18_ClearBuffer();
    HAL_UART_Transmit(&huart3, frame, 5 + data_len, 100);

    uint32_t start = HAL_GetTick();
    while (HAL_GetTick() - start < timeout) {
        if (E18_Available() >= 5 && rx_buffer[rx_tail] == 0x55) {
            uint8_t r_len = rx_buffer[(rx_tail + 1) % RX_BUF_SIZE];
            if (E18_Available() >= r_len + 2) {
                uint8_t rx_frame[64];
                for(int i = 0; i < r_len + 2; i++) rx_frame[i] = E18_ReadByte();
                Log_Hex_Compact("RX:", rx_frame, r_len + 2);
                if (rx_frame[2] == type && rx_frame[3] == code) return rx_frame[4];
            }
        } else if (E18_Available() > 0 && rx_buffer[rx_tail] != 0x55) {
            E18_ReadByte();
        }
    }
    return 0xFF;
}

// Lấy Status nhanh để nút PB3 dùng
uint8_t E18_Get_NetStatus(void) {
    uint8_t cmd_status[5] = {0x55, 0x03, 0x00, 0x00, 0x00};
    E18_ClearBuffer();
    HAL_UART_Transmit(&huart3, cmd_status, 5, 100);
    uint32_t start = HAL_GetTick();
    while (HAL_GetTick() - start < 1000) {
        if (E18_Available() >= 5 && rx_buffer[rx_tail] == 0x55) {
            uint8_t r_len = rx_buffer[(rx_tail + 1) % RX_BUF_SIZE];
            if (E18_Available() >= r_len + 2) {
                uint8_t rx_frame[64];
                for(int i = 0; i < r_len + 2; i++) rx_frame[i] = E18_ReadByte();
                if (rx_frame[2] == 0x00 && rx_frame[3] == 0x00) return rx_frame[4]; // Trả về Net Status
            }
        } else if (E18_Available() > 0 && rx_buffer[rx_tail] != 0x55) E18_ReadByte();
    }
    return 0xFF;
}

// KHỞI TẠO TỰ ĐỘNG & CHECK TRẠNG THÁI GATEWAY
void E18_Gateway_Init(void) {
    LCD_Log("--- SETUP GATEWAY ---");
    LCD_Log("Checking Status...");

    uint8_t cmd_status[5] = {0x55, 0x03, 0x00, 0x00, 0x00};
    E18_ClearBuffer();
    HAL_UART_Transmit(&huart3, cmd_status, 5, 100);

    uint32_t start = HAL_GetTick();
    uint8_t dev_type = 0xFF, net_status = 0xFF, current_channel = 0x00;
    uint16_t current_panid = 0x0000;
    uint8_t got_status = 0;

    while (HAL_GetTick() - start < 1500) {
        if (E18_Available() >= 5 && rx_buffer[rx_tail] == 0x55) {
            uint8_t r_len = rx_buffer[(rx_tail + 1) % RX_BUF_SIZE];
            if (E18_Available() >= r_len + 2) {
                uint8_t rx_frame[64];
                for(int i = 0; i < r_len + 2; i++) rx_frame[i] = E18_ReadByte();

                if (rx_frame[2] == 0x00 && rx_frame[3] == 0x00) {
                    net_status = rx_frame[4];
                    dev_type = rx_frame[5];
                    if (net_status == 0x00 && r_len >= 0x11) {
                        current_channel = rx_frame[14];
                        current_panid = rx_frame[15] | (rx_frame[16] << 8);
                    }
                    got_status = 1;
                    break;
                }
            }
        } else if (E18_Available() > 0 && rx_buffer[rx_tail] != 0x55) E18_ReadByte();
    }

    if (got_status) {
        // Đã là Coordinator (0x00), Có mạng (0x00), Kênh 11, PANID 0x2026 -> Chuẩn
        if (dev_type == 0x00 && net_status == 0x00 && current_channel == 0x0B && current_panid == 0x2026) {
            LCD_Log("=> Cau hinh GW OK!");

            LCD_Log("Mo mang tu dong 180s...");
            E18_ExecCmd_With_Log(0x00, 0x02, NULL, 0, 1000);
        } else {
            LCD_Log("=> Sai Config, Setup lai!");

            if (net_status == 0x00 || net_status == 0x02) {
                LCD_Log("Thoat mang cu...");
                uint8_t leave_payload[4] = {0x01, (uint8_t)(current_panid & 0xFF), (uint8_t)(current_panid >> 8), current_channel};
                E18_ExecCmd_With_Log(0x00, 0x04, leave_payload, 4, 1000);
                HAL_Delay(1000);
            }

            LCD_Log("Ghi cau hinh moi...");
            uint8_t p_type[1] = {0x00}; // Coordinator
            E18_ExecCmd_With_Log(0x00, 0x05, p_type, 1, 500);

            uint8_t p_panid[2] = {0x26, 0x20}; // PANID 0x2026
            E18_ExecCmd_With_Log(0x00, 0x08, p_panid, 2, 500);

            uint8_t p_chan[2] = {0x02, 0x0B}; // Kenh 11
            E18_ExecCmd_With_Log(0x00, 0x06, p_chan, 2, 500);

            uint8_t p_pwr[2] = {0x01, 0x14}; // Pwr Max
            E18_ExecCmd_With_Log(0x00, 0x0D, p_pwr, 2, 500);

            LCD_Log("Rebooting E18...");
            HAL_GPIO_WritePin(ZB_RST_GPIO_Port, ZB_RST_Pin, GPIO_PIN_RESET);
            HAL_Delay(100);
            HAL_GPIO_WritePin(ZB_RST_GPIO_Port, ZB_RST_Pin, GPIO_PIN_SET);
            HAL_Delay(3000);
            E18_ClearBuffer();

            LCD_Log("Mo mang moi (180s)...");
            E18_ExecCmd_With_Log(0x00, 0x02, NULL, 0, 1000);
        }
    } else {
        LCD_Log("=> Khong ket noi E18!");
    }
}

uint8_t E18_Broadcast_Data(uint8_t* data, uint8_t data_len) {
    uint8_t frame[128];
    uint8_t len = 15 + data_len;
    static uint8_t seq_num = 0;

    frame[0] = 0x55; frame[1] = len; frame[2] = 0x02; frame[3] = 0x0F;
    frame[4] = 0x00; frame[5] = 0xFF; frame[6] = 0xFF;
    frame[7] = 0xFF; frame[8] = seq_num++; frame[9] = 0x00;
    frame[10]= 0x08; frame[11]= 0xFC;
    frame[12]= 0x00; frame[13]= 0x00; // ManuCode 0x0000
    frame[14]= 0x00;
    frame[15]= 0x01; // CmdID = 0x01

    for (int i = 0; i < data_len; i++) frame[16 + i] = data[i];

    uint8_t cs = 0;
    for (int i = 2; i < 16 + data_len; i++) cs ^= frame[i];
    frame[16 + data_len] = cs;

    E18_ClearBuffer();
    HAL_UART_Transmit(&huart3, frame, 17 + data_len, 200);
    return 0x00;
}
