#include "bell_and_output.h"

void Test_Buzzer_And_Outputs(void) {
    // 1. Khởi tạo trạng thái tắt an toàn cho 4 đầu ra trước khi test
    HAL_GPIO_WritePin(REL1_PORT, REL1_PIN, GPIO_PIN_RESET); // Tắt Relay 1 (Mức 0)
    HAL_GPIO_WritePin(REL2_PORT, REL2_PIN, GPIO_PIN_RESET); // Tắt Relay 2 (Mức 0)
    HAL_GPIO_WritePin(OUT1_PORT, OUT1_PIN, GPIO_PIN_SET);   // Tắt OUT1 (Mức 1 do Active Low)
    HAL_GPIO_WritePin(OUT2_PORT, OUT2_PIN, GPIO_PIN_SET);   // Tắt OUT2 (Mức 1 do Active Low)
    HAL_Delay(1000);

    // 2. Test Còi (Kêu Bíp Bíp Bíp 3 lần)
    for(int i = 0; i < 3; i++) {
        HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);   // Bật còi
        HAL_Delay(150);
        HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET); // Tắt còi
        HAL_Delay(150);
    }
    HAL_Delay(1000); // Nghỉ 1 giây trước khi test Relay

    // 3. Test lần lượt 4 đầu ra
    // Bật Relay 1
    HAL_GPIO_WritePin(REL1_PORT, REL1_PIN, GPIO_PIN_SET);
    HAL_Delay(1000);
    HAL_GPIO_WritePin(REL1_PORT, REL1_PIN, GPIO_PIN_RESET);
    HAL_Delay(500);

    // Bật Relay 2
    HAL_GPIO_WritePin(REL2_PORT, REL2_PIN, GPIO_PIN_SET);
    HAL_Delay(1000);
    HAL_GPIO_WritePin(REL2_PORT, REL2_PIN, GPIO_PIN_RESET);
    HAL_Delay(500);

    // Bật OUT 1 (Kéo xuống mức 0 để thông dòng qua Opto)
    HAL_GPIO_WritePin(OUT1_PORT, OUT1_PIN, GPIO_PIN_RESET);
    HAL_Delay(1000);
    HAL_GPIO_WritePin(OUT1_PORT, OUT1_PIN, GPIO_PIN_SET);
    HAL_Delay(500);

    // Bật OUT 2
    HAL_GPIO_WritePin(OUT2_PORT, OUT2_PIN, GPIO_PIN_RESET);
    HAL_Delay(1000);
    HAL_GPIO_WritePin(OUT2_PORT, OUT2_PIN, GPIO_PIN_SET);
}
