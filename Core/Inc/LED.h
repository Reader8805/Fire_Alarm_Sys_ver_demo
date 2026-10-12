/*
 * LED.h
 *
 *  Created on: Sep 23, 2026
 *      Author: DANG
 */

#ifndef INC_LED_H_
#define INC_LED_H_
#include "main.h"
/* ===================================================================
 * ĐỊNH NGHĨA CHÂN GIAO TIẾP 74HC595
 * =================================================================== */
// Chân truyền dữ liệu nối tiếp (Serial Data)
#define HC595_PORT_DATA   GPIOA
#define HC595_PIN_DATA    GPIO_PIN_7
// Chân tạo xung nhịp dịch bit (Shift Register Clock)
#define HC595_PORT_CLK    GPIOA
#define HC595_PIN_CLK     GPIO_PIN_5
// Chân chốt dữ liệu xuất ra ngoài (Storage Register Clock / Latch)
#define HC595_PORT_LATCH  GPIOA
#define HC595_PIN_LATCH   GPIO_PIN_4
// Chân xóa dữ liệu của 74HC595 (Master Reclear)
#define HC595_PORT_CLEAR  GPIOD
#define HC595_PIN_CLEAR   GPIO_PIN_14


/* ===================================================================
 * KHAI BÁO CÁC HÀM XỬ LÝ (API)
 * =================================================================== */
/**
 * @brief  Hàm dịch 32-bit dữ liệu ra chuỗi 4 IC 74HC595 mắc nối tiếp.
 * @param  data: Giá trị 32-bit (tương ứng 32 trạng thái LED) cần xuất.
 */
void HC595_ShiftOut(uint32_t data);

/**
 * @brief  Hàm chạy hiệu ứng quét lần lượt từng LED (sáng 0.5s rồi tắt).
 */
void Test_All_LEDs(void);

#endif /* INC_LED_H_ */
