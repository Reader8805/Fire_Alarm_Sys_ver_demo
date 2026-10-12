/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "LED.h"
#include <stdio.h>
#include <string.h>
#include "LCD.h"
#include "bell_and_output.h"
#include "zigbee_e18.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MAX_NODES 30      // Giới hạn số lượng node
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */

uint8_t pb3_last_state = 1;
uint8_t pb4_last_state = 1;
uint8_t pb5_last_state = 1;

uint8_t joined_nodes = 0; // Biến đếm số thiết bị tham gia mạng
uint8_t joined_mac_list[MAX_NODES][8]; // Mảng lưu trữ 8 byte MAC của từng node
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */
// Hàm đẩy một dòng chữ lên LCD và tự động cuộn 4 dòng
void LCD_Log(const char* msg) {
    for (int i = 0; i < 3; i++) {
        strncpy(lcd_buffer[i], lcd_buffer[i+1], 20);
    }
    snprintf(lcd_buffer[3], 20, "%-19s", msg);
    LCD_UpdateScreen();
    HAL_Delay(120);
}
void E18_Test(void) {
	// ==============================================================
	      // NÚT PB5: MỞ MẠNG (Cho phép Join trong 180s)
	      // ==============================================================
	      uint8_t pb5_state = HAL_GPIO_ReadPin(PB5_GPIO_Port, PB5_Pin);
	      if (pb5_state == GPIO_PIN_RESET && pb5_last_state == GPIO_PIN_SET) {
	          HAL_Delay(50); // Chống dội phím
	          if (HAL_GPIO_ReadPin(PB5_GPIO_Port, PB5_Pin) == GPIO_PIN_RESET) {
	              LCD_Log("Dang mo mang 180s...");
	              E18_ExecCmd_With_Log(0x00, 0x02, NULL, 0, 1000);
	          }
	      }
	      pb5_last_state = pb5_state;

	      // ==============================================================
	      // NÚT PB3: Nếu đã join mạng thì sẽ thoát mạng, chưa join mạng thì factory rst (Kéo ZB_NWK_Pin xuống LOW 6 giây)
	      // ==============================================================
	      uint8_t pb3_state = HAL_GPIO_ReadPin(PB3_GPIO_Port, PB3_Pin);
	      if (pb3_state == GPIO_PIN_RESET && pb3_last_state == GPIO_PIN_SET) {
	          HAL_Delay(50);
	          if (HAL_GPIO_ReadPin(PB3_GPIO_Port, PB3_Pin) == GPIO_PIN_RESET) {
	              LCD_Log("!! FACTORY RESET !!");
	              LCD_Log("Dang xoa Flash (6s)..");
	              HAL_GPIO_WritePin(ZB_NWK_GPIO_Port, ZB_NWK_Pin, GPIO_PIN_RESET);
	              HAL_Delay(6500); // 6.5s cho an toàn
	              HAL_GPIO_WritePin(ZB_NWK_GPIO_Port, ZB_NWK_Pin, GPIO_PIN_SET);

	              LCD_Log("Rebooting...");
	              HAL_GPIO_WritePin(ZB_RST_GPIO_Port, ZB_RST_Pin, GPIO_PIN_RESET);
	              HAL_Delay(100);
	              HAL_GPIO_WritePin(ZB_RST_GPIO_Port, ZB_RST_Pin, GPIO_PIN_SET);
	              LCD_Log("Reset Hoan Tat!");
	              joined_nodes = 0;
	          }
	      }
	      pb3_last_state = pb3_state;

	      // ==============================================================
	      // NÚT PB4: GỬI BROADCAST (4 Byte)
	      // ==============================================================
	      uint8_t pb4_state = HAL_GPIO_ReadPin(PB4_GPIO_Port, PB4_Pin);
	      if (pb4_state == GPIO_PIN_RESET && pb4_last_state == GPIO_PIN_SET) {
	          HAL_Delay(50);
	          if (HAL_GPIO_ReadPin(PB4_GPIO_Port, PB4_Pin) == GPIO_PIN_RESET) {
	              LCD_Log("Phat Broadcast GW...");
	              uint8_t payload[4] = {0x01, 0xFF, 0x00, 0x00}; // GW mang ID 1, Co Alarm FF
	              E18_Broadcast_Data(payload, 4);
	          }
	      }
	      pb4_last_state = pb4_state;

	      // ==============================================================
	      // QUÉT NHẬN DATA & THÔNG BÁO TỪ NODE
	      // ==============================================================
	      if (E18_Available() >= 5 && rx_buffer[rx_tail] == 0x55) {
	          uint8_t r_len = rx_buffer[(rx_tail + 1) % RX_BUF_SIZE];

	          if (E18_Available() >= r_len + 2) {
	              uint8_t type = rx_buffer[(rx_tail + 2) % RX_BUF_SIZE];
	              uint8_t code = rx_buffer[(rx_tail + 3) % RX_BUF_SIZE];
	              //Cộng thêm node mới vào mạng
	               if (type == 0x80 && code == 0x04) {
	                   // Trích xuất 8 byte MAC Address từ buffer (vị trí 4 -> 11)[cite: 1, 23]
	                  uint8_t incoming_mac[8];
	                  for(int i = 0; i < 8; i++) {
	                     incoming_mac[i] = rx_buffer[(rx_tail + 4 + i) % RX_BUF_SIZE];
	                   }

	                        // Kiểm tra xem địa chỉ MAC này đã tồn tại trong mảng chưa
	                     uint8_t is_exist = 0;
	                     for(int i = 0; i < joined_nodes; i++) {
	                    	 if(memcmp(joined_mac_list[i], incoming_mac, 8) == 0) {
	                    		 is_exist = 1;
	                    		 break;
	                      }
	                   }

	                  // Nếu là MAC mới và chưa vượt quá số lượng tối đa
	                  if(!is_exist && joined_nodes < MAX_NODES) {
	                     memcpy(joined_mac_list[joined_nodes], incoming_mac, 8);
	                     joined_nodes++;

	                     char log_buf[21];
	                     // Hiển thị 2 byte cuối của địa chỉ MAC lên LCD cho gọn
	                     snprintf(log_buf, sizeof(log_buf), "Node IN: %02X%02X", incoming_mac[6], incoming_mac[7]);
	                     LCD_Log(log_buf);
	                     snprintf(log_buf, sizeof(log_buf), "Tong Node: %d", joined_nodes);
	                     LCD_Log(log_buf);
	                  }
	               }
	               // 2. NHẬN THÔNG BÁO NODE CHỦ ĐỘNG RỜI MẠNG (Async Command 0x80 0x06)
	               else if (type == 0x80 && code == 0x06) {
	                   // Trích xuất 8 byte MAC Address của thiết bị vừa rời mạng
	                   uint8_t leaving_mac[8];
	                   for(int i = 0; i < 8; i++) {
	                      leaving_mac[i] = rx_buffer[(rx_tail + 4 + i) % RX_BUF_SIZE];
	                   }

	                    // Tìm kiếm MAC này trong danh sách các node đang kết nối
	                   for(int i = 0; i < joined_nodes; i++) {
	                       if(memcmp(joined_mac_list[i], leaving_mac, 8) == 0) {
	                         // Nếu tìm thấy, tiến hành xóa bằng cách dịch các node phía sau lên
	                         for(int j = i; j < joined_nodes - 1; j++) {
	                            memcpy(joined_mac_list[j], joined_mac_list[j+1], 8);
	                         }
	                         // Cập nhật lại tổng số node
	                         joined_nodes--;

	                        char log_buf[21];
	                        snprintf(log_buf, sizeof(log_buf), "Node OUT: %02X%02X", leaving_mac[6], leaving_mac[7]);
	                        LCD_Log(log_buf);
	                        snprintf(log_buf, sizeof(log_buf), "Tong Node: %d", joined_nodes);
	                        LCD_Log(log_buf);

	                        break; // Thoát vòng lặp tìm kiếm
	                        }
	                    }
	                 }
	              // 2. NHẬN BẢN TIN ZCL TỪ NODE (Async Command 0x82 0x0F)
	              else if (type == 0x82 && code == 0x0F) {
	                  uint8_t data_len = r_len - 14;
	                  if (data_len >= 4) { // Đúng định dạng 4 byte của End Node
	                      uint8_t dev_id = rx_buffer[(rx_tail + 16) % RX_BUF_SIZE];
	                      uint8_t status = rx_buffer[(rx_tail + 17) % RX_BUF_SIZE];
	                      uint8_t alarm  = rx_buffer[(rx_tail + 18) % RX_BUF_SIZE];
	                      uint8_t temp   = rx_buffer[(rx_tail + 19) % RX_BUF_SIZE];

	                      char log_buf[21];
	                      snprintf(log_buf, sizeof(log_buf), "ID%02d S:%02X A:%02X T:%dC", dev_id, status, alarm, temp);
	                      LCD_Log(log_buf);

	                      if (alarm == 0xFF) {
	                          HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);
	                          HAL_GPIO_WritePin(REL1_PORT, REL1_PIN, GPIO_PIN_SET);
	                      } else {
	                          HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
	                          HAL_GPIO_WritePin(REL1_PORT, REL1_PIN, GPIO_PIN_RESET);
	                      }
	                  }
	              }

	              // Đọc xong thì vứt gói tin đó khỏi bộ đệm
	              for(int i = 0; i < r_len + 2; i++) E18_ReadByte();
	          }
	      } else if (E18_Available() > 0 && rx_buffer[rx_tail] != 0x55) {
	          E18_ReadByte(); // Xóa rác
	      }
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  LCD_Init();
  LCD_Backlight(1);
  LCD_Log("--- FIRE ALARM GW ---");

  HAL_UART_Receive_IT(&huart3, &rx_byte, 1);

    // Đầu tiên: Cấu hình Gateway
  E18_Gateway_Init();


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1)
    {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    	E18_Test();
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, LCD_D2_Pin|LCD_D3_Pin|LCD_D4_Pin|LCD_D5_Pin
                          |LCD_D6_Pin|LCD_D7_Pin|LCD_RS_Pin|LCD_RW_Pin
                          |LCD_E_Pin|LCD_BLC_Pin|BUZZER_Pin|LCD_D0_Pin
                          |LCD_D1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, IC_74HC595_LATCH_Pin|IC_74HC595_SCK_Pin|IC_74HC595_Data_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, ZB_NWK_Pin|ZB_RST_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_12|GPIO_PIN_13|IC_74HC595_CLEAR_Pin|GPIO_PIN_0
                          |GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, RELAY1_Pin|RELAY2_Pin|OUT1_Pin|OUT2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LCD_D2_Pin LCD_D3_Pin LCD_D4_Pin LCD_D5_Pin
                           LCD_D6_Pin LCD_D7_Pin LCD_RS_Pin LCD_RW_Pin
                           LCD_E_Pin LCD_BLC_Pin LCD_D0_Pin LCD_D1_Pin */
  GPIO_InitStruct.Pin = LCD_D2_Pin|LCD_D3_Pin|LCD_D4_Pin|LCD_D5_Pin
                          |LCD_D6_Pin|LCD_D7_Pin|LCD_RS_Pin|LCD_RW_Pin
                          |LCD_E_Pin|LCD_BLC_Pin|LCD_D0_Pin|LCD_D1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PB3_Pin PB4_Pin PB5_Pin PB6_Pin
                           PB7_Pin PB8_Pin PB9_Pin */
  GPIO_InitStruct.Pin = PB3_Pin|PB4_Pin|PB5_Pin|PB6_Pin
                          |PB7_Pin|PB8_Pin|PB9_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : IC_74HC595_LATCH_Pin IC_74HC595_SCK_Pin IC_74HC595_Data_Pin */
  GPIO_InitStruct.Pin = IC_74HC595_LATCH_Pin|IC_74HC595_SCK_Pin|IC_74HC595_Data_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : BUZZER_Pin */
  GPIO_InitStruct.Pin = BUZZER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BUZZER_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ZB_NWK_Pin ZB_RST_Pin RELAY1_Pin RELAY2_Pin
                           OUT1_Pin OUT2_Pin */
  GPIO_InitStruct.Pin = ZB_NWK_Pin|ZB_RST_Pin|RELAY1_Pin|RELAY2_Pin
                          |OUT1_Pin|OUT2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB13 PB14 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PD8 PD9 PD10 PD11
                           PD12 PD13 IC_74HC595_CLEAR_Pin PD0
                           PD1 PD2 PD3 PD7 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_12|GPIO_PIN_13|IC_74HC595_CLEAR_Pin|GPIO_PIN_0
                          |GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : PB2_Pin AC_DETECT_Pin */
  GPIO_InitStruct.Pin = PB2_Pin|AC_DETECT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
