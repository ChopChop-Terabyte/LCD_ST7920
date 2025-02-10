#include "GPIO_cfg.h"

gpio_config_t io_conf = {};

void GPIO_Input_init(uint64_t GPIO_Input_map, bool Pull_up) {
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    io_conf.pin_bit_mask = GPIO_Input_map;
    io_conf.mode = GPIO_MODE_INPUT;

    if (Pull_up == 1) {
        io_conf.pull_up_en = 1;
        io_conf.pull_down_en = 0;
    } else {
        io_conf.pull_up_en = 0;
        io_conf.pull_down_en = 1;
    }
    gpio_config(&io_conf);
}

void GPIO_Output_init(uint64_t GPIO_Output_map) {
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_Output_map;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
}