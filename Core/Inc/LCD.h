/*
 * LCD.h
 *
 *  Created on: Sep 23, 2026
 *      Author: DANG
 */

#ifndef INC_LCD_H_
#define INC_LCD_H_

#include "main.h"

#define LCD_PORT        GPIOE

// Chân điều khiển
#define LCD_PIN_RS      GPIO_PIN_8
#define LCD_PIN_RW      GPIO_PIN_9
#define LCD_PIN_E       GPIO_PIN_10
#define LCD_PIN_BLC     GPIO_PIN_11 // Chân điều khiển đèn nền (Backlight)
// Chân dữ liệu D0 - D7 tương ứng với PE0 - PE7
#define LCD_DATA_MASK   0x00FF      // Mask để lấy 8 bit thấp của Port E

// Bộ đệm hiển thị: 4 dòng, mỗi dòng 20 ký tự
extern char lcd_buffer[4][20];


void LCD_Init(void);                        // Khởi tạo LCD
void LCD_Backlight(uint8_t state);          // Bật/Tắt đèn nền (1: Bật, 0: Tắt)
void LCD_ClearBuffer(void);                 // Xóa sạch dữ liệu trong buffer
void LCD_UpdateScreen(void);                // Đẩy dữ liệu từ buffer ra màn hình thực tế

// Các hàm gửi lệnh cấp thấp (thường chỉ dùng nội bộ hoặc cần custom màn hình)
void LCD_SendCommand(uint8_t cmd);
void LCD_SendData(uint8_t data);
void LCD_SetCursor(uint8_t row, uint8_t col);
#endif /* INC_LCD_H_ */
