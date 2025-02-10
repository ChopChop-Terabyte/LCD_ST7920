#ifndef GPIO_CFG_H
#define GPIO_CFG_H

#include "driver/gpio.h"

void GPIO_Input_init(uint64_t GPIO_INPUT_MAP, bool Pull_up);
void GPIO_Output_init(uint64_t GPIO_OUTPUT_MAP);

#endif // GPIO_CFG_H