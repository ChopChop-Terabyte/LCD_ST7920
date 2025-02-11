#ifndef ST7920_H
#define ST7920_H

#include <string.h>
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "font_LCD_4x6.h"
#include "frame.h"

#include "esp_log.h"

#define RST_BIT (1 << 8)
#define RS_BIT (1 << 9)
#define RW_BIT (1 << 10)
#define EN_BIT (1 << 11)
#define DATA_MASK 0xFF

#define SCREEN_NUM_1 1
#define SCREEN_NUM_2 2
#define SCREEN_NUM_3 3
#define L1 1
#define L2 2
#define L3 3
#define L4 4

extern bool Lock;
extern uint8_t Current_screen;
extern uint8_t Screen[1024];

// For Screen 3
extern bool mode;
extern uint8_t position;
extern uint8_t p_value[6];
extern uint8_t p_enable[6], p_state[6], p_min[6], p_hour[6], p_mday[6], p_mon[6];

typedef struct {
    gpio_num_t SCL_Pin;
    gpio_num_t SDA_Pin;
    i2c_port_num_t Port_num;
    i2c_clock_source_t Clock_freq;
    uint16_t LCD_Address;
}LCD_config_t;

void LCD_I2C_init(LCD_config_t LCD_cfg);
void Graphic_mode_ON();
void Graphic_mode_OFF();
void LCD_draw_screen(const uint8_t *bitmap);
void LCD_config();
void LCD_send_text(const char *str);
void LCD_Graphic_send_text(uint8_t *screen, char *data, uint8_t x, uint8_t y);
void Center(uint8_t *cent, float phase_1, float phase_2, float phase_3);
void Clean_screen(uint8_t *screen);
void Merge_screen(uint8_t *screen_1, const uint8_t *screen_2);

void Date_DMY(uint8_t *screen, uint8_t screen_num, uint8_t day, uint8_t date, uint8_t month, uint16_t year);
void HMS(uint8_t *screen, uint8_t screen_num, uint8_t hour, uint8_t minute, uint8_t second);

// Screen 1
void Screen_1(uint8_t *screen);
void Carrier_name(uint8_t *screen, uint8_t *data);
void Network_signal(uint8_t *screen, uint8_t num);
void Real_time();
void Temp_hum_set(uint8_t *screen, uint8_t temp, uint8_t hum);
void Set_value(uint8_t *screen, float phase_1, float phase_2, float phase_3, uint8_t *cent, uint8_t y);
void Volt_set(uint8_t *screen, float phase_1, float phase_2, float phase_3);
void Ampe_set(uint8_t *screen, float phase_1, float phase_2, float phase_3);
void Pow_set(uint8_t *screen, float phase_1, float phase_2, float phase_3);
void Pow_per_time_set(uint8_t *screen, float p, float ph, float cos);

// Screen 2
void Screen_2(uint8_t *screen);
void History_time_set(uint8_t *screen, uint8_t L, uint8_t hour, uint8_t minute, uint8_t day, uint8_t date, uint8_t month, uint16_t year);
void Status_set(uint8_t *screen, uint8_t L, uint8_t st);
void IN_set(uint8_t *screen, uint8_t L, uint8_t st1, uint8_t st2);
void OUT_set(uint8_t *screen, uint8_t L, uint8_t st);

// Screen 3
void Screen_3(uint8_t *screen);
/* 
    mode = 0:       Print
    mode = 1:       Switch and print
*/
void Switch_MODE(uint8_t *screen, bool sw);

#endif // ST7920_H