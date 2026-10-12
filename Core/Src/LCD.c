#include "LCD.h"
#include <stdbool.h>

char lcd_buffer[4][20];


static void LCD_WriteDataBus(uint8_t data) {
    // Sử dụng thanh ghi BSRR để cập nhật PE0-PE7 an toàn và nhanh nhất.
    // Nửa trên (<< 16): Ghi số 1 vào để xóa (Reset) sạch 8 chân PE0-PE7.
    // Nửa dưới: Đẩy giá trị data vào để cài đặt (Set) các chân cần thiết.
    LCD_PORT->BSRR = (0x00FF << 16) | data;
}

void LCD_SendCommand(uint8_t cmd) {
    HAL_GPIO_WritePin(LCD_PORT, LCD_PIN_RS, GPIO_PIN_RESET); // RS = 0
    HAL_GPIO_WritePin(LCD_PORT, LCD_PIN_RW, GPIO_PIN_RESET); // RW = 0
    HAL_GPIO_WritePin(LCD_PORT, LCD_PIN_E, GPIO_PIN_RESET);  // E đang thấp

    LCD_WriteDataBus(cmd);

    // Tạo xung chốt dữ liệu (Enable Pulse)
    HAL_GPIO_WritePin(LCD_PORT, LCD_PIN_E, GPIO_PIN_SET);
    HAL_Delay(1); // Cần delay để LCD kịp chộp dữ liệu
    HAL_GPIO_WritePin(LCD_PORT, LCD_PIN_E, GPIO_PIN_RESET);
    HAL_Delay(1);
}

void LCD_SendData(uint8_t data) {
    HAL_GPIO_WritePin(LCD_PORT, LCD_PIN_RS, GPIO_PIN_SET);   // RS = 1
    HAL_GPIO_WritePin(LCD_PORT, LCD_PIN_RW, GPIO_PIN_RESET); // RW = 0
    HAL_GPIO_WritePin(LCD_PORT, LCD_PIN_E, GPIO_PIN_RESET);  // E đang thấp

    LCD_WriteDataBus(data);

    HAL_GPIO_WritePin(LCD_PORT, LCD_PIN_E, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(LCD_PORT, LCD_PIN_E, GPIO_PIN_RESET);
    HAL_Delay(1);
}

void LCD_SetCursor(uint8_t row, uint8_t col) {
    uint8_t address = 0;
    switch (row) {
        case 0: address = 0x00; break;
        case 1: address = 0x40; break;
        case 2: address = 0x14; break;
        case 3: address = 0x54; break;
    }
    address += col;
    LCD_SendCommand(0x80 | address);
}

/* ===================================================================
 * HÀM DÀNH CHO NGƯỜI DÙNG
 * =================================================================== */

void LCD_Backlight(uint8_t state) {
    if (state) {
        HAL_GPIO_WritePin(LCD_PORT, LCD_PIN_BLC, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(LCD_PORT, LCD_PIN_BLC, GPIO_PIN_RESET);
    }
}

void LCD_Init(void) {
    HAL_Delay(50); // Chờ điện áp ổn định

    HAL_GPIO_WritePin(LCD_PORT, LCD_PIN_E | LCD_PIN_RW | LCD_PIN_RS, GPIO_PIN_RESET);

    // BẮT BUỘC: Gửi 3 lần mã 0x30 để reset cứng LCD về chế độ 8-bit
    LCD_SendCommand(0x30);
    HAL_Delay(5);
    LCD_SendCommand(0x30);
    HAL_Delay(1);
    LCD_SendCommand(0x30);
    HAL_Delay(1);

    // Cấu hình thông số chính thức
    LCD_SendCommand(0x38); // 8-bit mode, 2 lines (hiểu là đa dòng), 5x8 font
    LCD_SendCommand(0x0C); // Bật màn hình, tắt con trỏ
    LCD_SendCommand(0x01); // Xóa màn hình
    HAL_Delay(2);
    LCD_SendCommand(0x06); // Tự động dịch con trỏ sang phải

    LCD_ClearBuffer();
    LCD_Backlight(1);
}

void LCD_ClearBuffer(void) {
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 20; col++) {
            lcd_buffer[row][col] = ' ';
        }
    }
}

void LCD_UpdateScreen(void) {
    for (int row = 0; row < 4; row++) {
        LCD_SetCursor(row, 0);
        bool end_of_string = false;

        for (int col = 0; col < 20; col++) {
            // Khi gặp ký tự kết thúc chuỗi \0, đánh dấu và điền dấu cách cho phần còn lại của dòng
            if (lcd_buffer[row][col] == '\0') {
                end_of_string = true;
            }

            if (end_of_string) {
                LCD_SendData(' ');
            } else {
                LCD_SendData((uint8_t)lcd_buffer[row][col]);
            }
        }
    }
}
