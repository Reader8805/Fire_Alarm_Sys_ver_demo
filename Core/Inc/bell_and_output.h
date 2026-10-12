
#ifndef INC_BELL_AND_OUTPUT_H_
#define INC_BELL_AND_OUTPUT_H_

#include "main.h"

#define BUZZER_PORT GPIOE
#define BUZZER_PIN  GPIO_PIN_12 // PE12: DKBELL

#define REL1_PORT   GPIOB
#define REL1_PIN    GPIO_PIN_3  // PB3: REL1

#define REL2_PORT   GPIOB
#define REL2_PIN    GPIO_PIN_4  // PB4: REL2

#define OUT1_PORT   GPIOB
#define OUT1_PIN    GPIO_PIN_5  // PB5: OUT1

#define OUT2_PORT   GPIOB
#define OUT2_PIN    GPIO_PIN_6  // PB6: OUT2


void Test_Buzzer_And_Outputs(void);
#endif /* INC_BELL_AND_OUTPUT_H_ */
