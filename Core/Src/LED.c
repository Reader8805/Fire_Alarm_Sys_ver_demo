#include "LED.h"

/* ===================================================================
 * CẤU HÌNH CÁC CHÂN LED NỐI TRỰC TIẾP VỚI MCU
 * =================================================================== */
// Kiểu dữ liệu cấu trúc lưu trữ thông tin Port và Pin của một LED
typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
} MCU_LED_t;

// Mảng chứa danh sách các LED nối trực tiếp vào MCU
static MCU_LED_t mcu_leds[] = {
    {GPIOD, GPIO_PIN_0},
    {GPIOD, GPIO_PIN_1},
	{GPIOD, GPIO_PIN_2},
	{GPIOD, GPIO_PIN_3},
	{GPIOD, GPIO_PIN_7},
	{GPIOD, GPIO_PIN_8},
	{GPIOD, GPIO_PIN_9},
	{GPIOD, GPIO_PIN_10},
	{GPIOD, GPIO_PIN_11},
	{GPIOD, GPIO_PIN_12},
	{GPIOD, GPIO_PIN_13},

};
// Macro tự động tính toán tổng số lượng LED đang có trong mảng mcu_leds
#define NUM_MCU_LEDS (sizeof(mcu_leds) / sizeof(mcu_leds[0]))

/* ===================================================================
 * THỰC THI CÁC HÀM
 * =================================================================== */

void HC595_ShiftOut(uint32_t data) {
    // Vòng lặp dịch 32 bit dữ liệu (đẩy vào 4 IC 74HC595, mỗi IC 8 bit).
    // Trình tự dịch: Đẩy từ bit cao nhất (MSB - bit 31) xuống bit thấp nhất (LSB - bit 0).
    for (int i = 31; i >= 0; i--) {

        // Bước 1: Kiểm tra xem bit thứ 'i' của biến 'data' đang là 0 hay 1
        if ((data >> i) & 0x01) {
            // Nếu là 1: Xuất mức điện áp CAO (3.3V) ra chân DATA
            HAL_GPIO_WritePin(HC595_PORT_DATA, HC595_PIN_DATA, GPIO_PIN_SET);
        } else {
            // Nếu là 0: Xuất mức điện áp THẤP (0V) ra chân DATA
            HAL_GPIO_WritePin(HC595_PORT_DATA, HC595_PIN_DATA, GPIO_PIN_RESET);
        }

        // Bước 2: Tạo 1 xung nhịp (Clock) để IC 74HC595 chộp lấy giá trị từ chân DATA vào thanh ghi.
        // Xung hợp lệ là xung có sườn dương (thay đổi từ mức THẤP lên CAO).
        HAL_GPIO_WritePin(HC595_PORT_CLK, HC595_PIN_CLK, GPIO_PIN_RESET); // Kéo xuống THẤP
        HAL_GPIO_WritePin(HC595_PORT_CLK, HC595_PIN_CLK, GPIO_PIN_SET);   // Kéo lên CAO
    }

    // Bước 3: Sau khi dịch đủ 32 bit vào thanh ghi nội bộ, tạo 1 xung chốt (Latch).
    // Xung này đẩy đồng loạt 32 bit đó ra các chân Q0-Q7 của 4 IC để thắp sáng LED.
    HAL_GPIO_WritePin(HC595_PORT_LATCH, HC595_PIN_LATCH, GPIO_PIN_RESET); // Kéo xuống THẤP
    HAL_GPIO_WritePin(HC595_PORT_LATCH, HC595_PIN_LATCH, GPIO_PIN_SET);   // Kéo lên CAO
}

void Test_All_LEDs(void) {
    // Cho phép 74HC595 hoạt động (Active Low)
    HAL_GPIO_WritePin(HC595_PORT_CLEAR, HC595_PIN_CLEAR, GPIO_PIN_SET);

    /* --- BƯỚC CHUẨN BỊ: TẮT TOÀN BỘ LED TRƯỚC KHI QUÉT --- */
    // Tắt các LED trên MCU (Mức CAO = Tắt)
    for (int i = 0; i < NUM_MCU_LEDS; i++) {
        HAL_GPIO_WritePin(mcu_leds[i].port, mcu_leds[i].pin, GPIO_PIN_SET);
    }
    // Tắt các LED trên 74HC595 (Mức CAO = Tắt -> Xuất 32 bit 1 = 0xFFFFFFFF)
    HC595_ShiftOut(0xFFFFFFFF);
    HAL_Delay(500); // Dừng nửa giây trước khi bắt đầu bài test


    /* --- GIAI ĐOẠN 1: QUÉT CÁC LED NỐI TRỰC TIẾP VỚI MCU --- */
    for (int i = 0; i < NUM_MCU_LEDS; i++) {
        // Bật sáng LED thứ 'i' (Mức THẤP = Sáng)
        HAL_GPIO_WritePin(mcu_leds[i].port, mcu_leds[i].pin, GPIO_PIN_RESET);

        HAL_Delay(500);

        // Tắt LED thứ 'i' (Mức CAO = Tắt)
        HAL_GPIO_WritePin(mcu_leds[i].port, mcu_leds[i].pin, GPIO_PIN_SET);
    }

    /* --- GIAI ĐOẠN 2: QUÉT 32 LED NỐI QUA 4 IC 74HC595 --- */
    for (int i = 0; i < 32; i++) {
        // Dùng toán tử NOT (~) để đảo bit.
        // Ví dụ i=0: ~(0000...0001) = 1111...1110 (Chỉ có bit 0 bằng 0 để LED 0 sáng)
        uint32_t led_data = ~(1U << i);

        HC595_ShiftOut(led_data);
        HAL_Delay(500);
    }

    // Kết thúc bài test: Tắt tất cả các LED trên 74HC595 (Xuất toàn bit 1)
    HC595_ShiftOut(0xFFFFFFFF);
}
