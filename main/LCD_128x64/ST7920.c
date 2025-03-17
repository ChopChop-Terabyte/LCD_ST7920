#include "ST7920.h"

#define DELAY 100

char mess[100];

// Lock = 1;
bool Lock = 0;
uint8_t Current_screen = SCREEN_NUM_1;
uint8_t Screen[1024] = {[0 ... 1023] = 0x00};

// For Screen 3
bool mode = 0;
uint8_t position = 0;
uint8_t p_value[6] = {[0 ... 5] = 0x00};
uint8_t p_enable[6], p_state[6], p_min[6], p_hour[6], p_mday[6], p_mon[6];

i2c_master_bus_handle_t bus_handle;
i2c_master_dev_handle_t LCD_handle;

void LCD_I2C_init(LCD_config_t LCD_cfg) {
    i2c_master_bus_config_t bus_config = {
        .i2c_port = LCD_cfg.Port_num,
        .sda_io_num = LCD_cfg.SDA_Pin,
        .scl_io_num = LCD_cfg.SCL_Pin,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = LCD_cfg.LCD_Address,
        .scl_speed_hz = LCD_cfg.Clock_freq
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_config, &LCD_handle));
}

void LCD_I2C_transmit(uint16_t data) {
    uint8_t data_LOW = data & 0x00FF;
    uint8_t data_HIGHT = (data >> 8) & 0xFF;
	uint8_t to_send[2] = {data_LOW, data_HIGHT};

	i2c_master_transmit(LCD_handle, to_send, 2, 100);
}

void LCD_I2C_transmit_alotof_data(uint8_t *data, uint16_t len) {

	i2c_master_transmit(LCD_handle, data, len, 1000);
}

void LCD_pulse_enable(uint16_t data) {
    LCD_I2C_transmit(data | EN_BIT);
    LCD_I2C_transmit(data & ~EN_BIT);
}

void LCD_write(uint8_t value, uint8_t rs) {
    uint16_t data = value & DATA_MASK;
    data |= RST_BIT;
    if (rs) {
        data |= RS_BIT;
    }
    data &= ~RW_BIT;
    LCD_pulse_enable(data);
}

void Screen_ON(void) {
    LCD_write(0x36, 0); // Display off
    vTaskDelay(2 / portTICK_PERIOD_MS);
}

void Screen_OFF(void) {
    LCD_write(0x34, 0); // Display off
    vTaskDelay(2 / portTICK_PERIOD_MS);
}

void Graphic_mode_ON() {
    LCD_write(0x36, 0); // 8 bit and Graphic mode ON
    vTaskDelay(2 / portTICK_PERIOD_MS);
    LCD_write(0x36, 0);
    vTaskDelay(1 / portTICK_PERIOD_MS);
}

void Graphic_mode_OFF() {
    LCD_write(0x30, 0); // 8 bit and Graphic mode OFF
    vTaskDelay(2 / portTICK_PERIOD_MS);
    LCD_write(0x30, 0);
    vTaskDelay(1 / portTICK_PERIOD_MS);
}

void LCD_Rotate_bitmap(const uint8_t *bitmap, uint8_t *rotate) {
    for (uint16_t i = 0; i < 1024; i++) {
        for (uint8_t j = 0; j < 8; j++) {
            rotate[i] <<= 1;
            if (bitmap[1023 - i] & (0x01 << j)) {
                rotate[i] |= 0x01;
            }
        }
    }
}

void Data_handler(uint8_t value, uint8_t *to_data, uint16_t loca, uint8_t rs) {
    uint16_t data = value & DATA_MASK;
    data |= RST_BIT;
    if (rs) {
        data |= RS_BIT;
    }
    data &= ~RW_BIT;

    data |= EN_BIT;
    to_data[loca] = data & 0x00FF;
    to_data[loca + 1] = (data >> 8) & 0xFF;

    data &= ~EN_BIT;
    to_data[loca + 2] = data & 0x00FF;
    to_data[loca + 3] = (data >> 8) & 0xFF;
}

void LCD_draw_screen(const uint8_t *bitmap) {
    int count = 0, index = 0;
    uint8_t Rotate[1024] = {[0 ... 1023] = 0x00};
    uint8_t Data[1024] = {[0 ... 1023] = 0x00};

    // memcpy(Rotate, bitmap, 1024);       // Not Rotate
    LCD_Rotate_bitmap(bitmap, Rotate);  // Rotate

    for (uint8_t i = 0; i < 32; i++) {
        for (uint8_t j = 0; j < 8; j++) {
            Data_handler(0x80 + i, Data, index++ * 4, 0);
            Data_handler(0x80 + j, Data, index++ * 4, 0);
            Data_handler(Rotate[count++], Data, index++ * 4, 1);
            Data_handler(Rotate[count++], Data, index++ * 4, 1);
            if (index * 4 > 1023) {
                index = 0;
                LCD_I2C_transmit_alotof_data(Data, 1024);
            }
        }
    }
    for (uint8_t i = 0; i < 32; i++) {
        for (uint8_t j = 8; j < 16; j++) {
            Data_handler(0x80 + i, Data, index++ * 4, 0);
            Data_handler(0x80 + j, Data, index++ * 4, 0);
            Data_handler(Rotate[count++], Data, index++ * 4, 1);
            Data_handler(Rotate[count++], Data, index++ * 4, 1);
            if (index * 4 > 1023) {
                index = 0;
                LCD_I2C_transmit_alotof_data(Data, 1024);
            }
        }
    }
}

void LCD_config() {
    vTaskDelay(50 / portTICK_PERIOD_MS);
    LCD_I2C_transmit(0x0001); // HIGHT xreset after >40ms

    Graphic_mode_ON();

    Graphic_mode_OFF();
    LCD_write(0x0C, 0); // Display on, off Cursor display
    vTaskDelay(2 / portTICK_PERIOD_MS);
    LCD_write(0x01, 0); // Clean screen
    vTaskDelay(4 / portTICK_PERIOD_MS);
    LCD_write(0x06, 0); // Cursor turn right 1
    vTaskDelay(2 / portTICK_PERIOD_MS);
}

void LCD_send_text(const char *str) {
    while (*str) LCD_write((uint8_t)*str++, 1);
}

void LCD_handle_text(uint8_t *bitmap, uint8_t x, uint8_t y, const uint8_t *font) {
    uint8_t row = y;
    for (uint8_t i = 0; i < 6; i++) {
        for (uint8_t j = 0; j < 5; j++) {
            int lo = row * 128 + x + j;
            int mer_in = lo / 8;
            uint8_t lost = lo % 8;
            if (bitmap[mer_in] & (0x80 >> lost)) {
                bitmap[mer_in] &= ~(0x80 >> lost);
            }
        }
        row++;
    }
    row = y;
    for (uint8_t i = 0; i < 6; i++) {
        for (uint8_t j = 0; j < 5; j++) {
            int lo = row * 128 + x + j;
            int mer_in = lo / 8;
            uint8_t lost = lo % 8;
            if (font[i] & (0x80 >> j)) {
                bitmap[mer_in] |= (0x80 >> lost);
            }
        }
        row++;
    }
}

void LCD_Graphic_send_text(uint8_t *screen, char *data, uint8_t x, uint8_t y) {
    for (uint8_t i = 0; i < strlen(data); i++) {
        const uint8_t *c = get_font(data[i]);

        LCD_handle_text(screen, x, y, c);

        switch (data[i]) {
            case ' ':
                x += 1;
                break;
            case '.':
                x += 2;
                break;
            case '(':
            case ')':
            case '*':
            case '|':
                x += 4;
                break;
            case ':':
                x += 2;
                break;
            default:
                x += 5;
                break;
		}
    }
}

void Center(uint8_t *cent, float phase_1, float phase_2, float phase_3) {
    if (phase_1 > 99.99) cent[0] = 0;
    else if (phase_1 > 9.99) cent[0] = 2;
    else cent[0] = 5;

    if (phase_2 > 99.99) cent[1] = 0;
    else if (phase_2 > 9.99) cent[1] = 2;
    else cent[1] = 5;

    if (phase_3 > 99.99) cent[2] = 0;
    else if (phase_3 > 9.99) cent[2] = 2;
    else cent[2] = 5;
}

void Clean_data_screen(uint8_t *screen) {
    memset(screen, 0x00, 1024 * sizeof(uint8_t));
}

void Merge_screen(uint8_t *screen_1, const uint8_t *screen_2) {
    for (uint16_t i = 0; i < 1024; i++) {
        screen_1[i] |= screen_2[i];
    }
}

void Date_DMY(uint8_t *screen, uint8_t screen_num, uint8_t day, uint8_t date, uint8_t month, uint16_t year) {      // date: 1 = Sunday, 2 = Monday, ... , 7 = Saturday
    uint8_t str_day[4];
    uint8_t x[2], y[2];

    switch (screen_num) {
        case SCREEN_NUM_1:
            x[0] = 77; x[1] = 75;
            y[0] = 0; y[1] = 8;
            break;
        case SCREEN_NUM_2:
        case SCREEN_NUM_3:
            x[0] = 79; x[1] = 0;
            y[0] = 0; y[1] = 0;
            break;
        default:
            break;
    }

    switch (day) {
        case 0: strcpy((char *)str_day, "Sun"); break;
        case 1: strcpy((char *)str_day, "Mon"); break;
        case 2: strcpy((char *)str_day, "Tue"); break;
        case 3: strcpy((char *)str_day, "Wed"); break;
        case 4: strcpy((char *)str_day, "Thu"); break;
        case 5: strcpy((char *)str_day, "Fri"); break;
        case 6: strcpy((char *)str_day, "Sat"); break;
        default: strcpy((char *)str_day, "Day"); break;
    }

    sprintf(mess, "%02d/%02d/%04d", date, month, year);
    LCD_Graphic_send_text(screen, mess, x[0], y[0]);
    sprintf(mess, "%s.", str_day);
    LCD_Graphic_send_text(screen, mess, x[1], y[1]);
    LCD_draw_screen(screen);
}

void HMS(uint8_t *screen, uint8_t screen_num, uint8_t hour, uint8_t minute, uint8_t second) {
    uint8_t x = 0, y = 0;

    switch (screen_num) {
        case SCREEN_NUM_1:
            x = 94; y = 8;
            break;
        case SCREEN_NUM_2:
        case SCREEN_NUM_3:
            x = 18; y = 0;
            break;
        default:
            break;
    }

    sprintf(mess, "%02d:%02d:%02d", hour, minute, second);
    LCD_Graphic_send_text(screen, mess, x, y);
    LCD_draw_screen(screen);
}

/* Screen 1 */
void Screen_1(uint8_t *screen) {
    while (Lock) vTaskDelay(1 / portTICK_PERIOD_MS);
    Lock = 1;

    Current_screen = SCREEN_NUM_1;

    Clean_data_screen(screen);
    LCD_Graphic_send_text(screen, "             XXXXXXXX                         DD/MM/YYYY", 0, 0);
    LCD_Graphic_send_text(screen, "        TT|C        HH%                         Day.  HH:MM:SS", 0, 8);
    LCD_Graphic_send_text(screen, " Phase               1                             2                             3", 0, 18);
    LCD_Graphic_send_text(screen, "    U(V)", 0, 27);
    LCD_Graphic_send_text(screen, "    I(A)", 0, 36);
    LCD_Graphic_send_text(screen, "  P(kW)", 0, 45);
    LCD_Graphic_send_text(screen, "  0.000kW    000000kWh    Cos: 0.000", 0, 55);
    Merge_screen(screen, Frame_1);
    Screen_OFF();
    LCD_draw_screen(screen);
    vTaskDelay(DELAY / portTICK_PERIOD_MS);
    Screen_ON();

    Lock = 0;
    vTaskDelay(10 / portTICK_PERIOD_MS);
}

void Carrier_name(uint8_t *screen, uint8_t *data) {
    LCD_Graphic_send_text(screen, "                                                            ", 13, 0);
    LCD_Graphic_send_text(screen, (char *)data, 13, 0);
    LCD_draw_screen(screen);
}

void Network_signal(uint8_t *screen, uint8_t num) {        //num: 1, 2 = Low | 3 = Medium | 4, 5 = High | 0 = No signal
    for (uint8_t i = 0; i <= num; i++) {
        switch (i) {
            case 0: LCD_Graphic_send_text(screen, "         ", 0, 0); break;
            case 1: LCD_handle_text(screen, 0, 0, signal_1); break;
            case 2: LCD_handle_text(screen, 2, 0, signal_2); break;
            case 3: LCD_handle_text(screen, 4, 0, signal_3); break;
            case 4: LCD_handle_text(screen, 6, 0, signal_4); break;
            case 5: LCD_handle_text(screen, 8, 0, signal_5); break;
            default: break;
        }
    }

    LCD_draw_screen(screen);
}

void Temp_hum_set(uint8_t *screen, uint8_t temp, uint8_t hum) {
    sprintf(mess, "%02d", temp);
    LCD_Graphic_send_text(screen, mess, 8, 8);
    sprintf(mess, "%02d", hum);
    LCD_Graphic_send_text(screen, mess, 35, 8);
}

void Set_value(uint8_t *screen, float phase_1, float phase_2, float phase_3, uint8_t *cent, uint8_t y) {
    LCD_Graphic_send_text(screen, "                          ", 30, y);
    LCD_Graphic_send_text(screen, "                          ", 64, y);
    LCD_Graphic_send_text(screen, "                          ", 98, y);

    sprintf(mess, "%3.2f", phase_1);
    LCD_Graphic_send_text(screen, mess, 30 + cent[0], y);
    sprintf(mess, "%3.2f", phase_2);
    LCD_Graphic_send_text(screen, mess, 64 + cent[1], y);
    sprintf(mess, "%3.2f", phase_3);
    LCD_Graphic_send_text(screen, mess, 98 + cent[2], y);
}

void Volt_set(uint8_t *screen, float phase_1, float phase_2, float phase_3) {
    uint8_t cent[3];
    Center(cent, phase_1, phase_2, phase_3);
    Set_value(screen, phase_1, phase_2, phase_3, cent, 27);
}

void Ampe_set(uint8_t *screen, float phase_1, float phase_2, float phase_3) {
    uint8_t cent[3];
    Center(cent, phase_1, phase_2, phase_3);
    Set_value(screen, phase_1, phase_2, phase_3, cent, 36);
}

void Pow_set(uint8_t *screen, float phase_1, float phase_2, float phase_3) {
    uint8_t cent[3];
    Center(cent, phase_1, phase_2, phase_3);
    Set_value(screen, phase_1, phase_2, phase_3, cent, 45);
}

void Pow_per_time_set(uint8_t *screen, float p, float ph, float cos) {
    uint8_t cent[3];

    LCD_Graphic_send_text(screen, "                  ", 2, 55);
    LCD_Graphic_send_text(screen, "                          ", 38, 55);
    LCD_Graphic_send_text(screen, "                     ", 104, 55);

    if (p > 999) cent[0] = 0;
    else if (p > 99) cent[0] = 1;
    else if (p > 9 || p < 0) cent[0] = 2;
    else cent[0] = 3;

    if (ph > 9999) cent[1] = 0;
    else if (ph > 999) cent[1] = 1;
    else if (ph > 99) cent[1] = 2;
    else if (ph > 9) cent[1] = 3;
    else if (ph < 0) cent[1] = 3;
    else cent[1] = 4;

    if (cos < 0) cent[2] = 2;
    else cent[2] = 3;

    sprintf(mess, "%.*f", cent[0], p);
    LCD_Graphic_send_text(screen, mess, 2, 55);
    sprintf(mess, "%.*f", cent[1], ph);
    LCD_Graphic_send_text(screen, mess, 38, 55);
    sprintf(mess, "%.*f", cent[2], cos);
    LCD_Graphic_send_text(screen, mess, 105, 55);
}

/* Screen 2 */
void Screen_2(uint8_t *screen) {
    while (Lock) vTaskDelay(1 / portTICK_PERIOD_MS);
    Lock = 1;

    Current_screen = SCREEN_NUM_2;

    Clean_data_screen(screen);
    LCD_Graphic_send_text(screen, "     IN", 0, 10);
    LCD_Graphic_send_text(screen, "   OUT", 0, 19);
    LCD_Graphic_send_text(screen, "   L 1", 0, 29);
    LCD_Graphic_send_text(screen, "   L 2", 0, 38);
    LCD_Graphic_send_text(screen, "   L 3", 0, 47);
    LCD_Graphic_send_text(screen, "   L 4", 0, 56);
    Merge_screen(screen, Frame_2);
    Screen_OFF();
    LCD_draw_screen(screen);
    vTaskDelay(DELAY / portTICK_PERIOD_MS);
    Screen_ON();

    Lock = 0;
    vTaskDelay(10 / portTICK_PERIOD_MS);
}

void History_time_set(uint8_t *screen, uint8_t L, uint8_t hour, uint8_t minute, uint8_t day, uint8_t date, uint8_t month, uint16_t year) {
    uint8_t str_day[4];
    uint8_t x = 18, y = 0;

    switch (day) {
        case 0: strcpy((char *)str_day, "Sun"); break;
        case 1: strcpy((char *)str_day, "Mon"); break;
        case 2: strcpy((char *)str_day, "Tue"); break;
        case 3: strcpy((char *)str_day, "Wed"); break;
        case 4: strcpy((char *)str_day, "Thu"); break;
        case 5: strcpy((char *)str_day, "Fri"); break;
        case 6: strcpy((char *)str_day, "Sat"); break;
        default: strcpy((char *)str_day, "Day"); break;
    }

    switch (L) {
        case 1: y = 29; break;
        case 2: y = 38; break;
        case 3: y = 47; break;
        case 4: y = 56; break;
        default: break;
    }

    LCD_Graphic_send_text(screen, "                          ", x, y);

    sprintf(mess, "%02d:%02d   %s. %02d/%02d/%04d", hour, minute, str_day, date, month, year);
    LCD_Graphic_send_text(screen, mess, x, y);

    // vTaskDelay(30 / portTICK_PERIOD_MS);
}

void Status_set(uint8_t *screen, uint8_t L, uint8_t st) {
    uint8_t str_status[7];
    uint8_t x = 113, y = 0;

    switch (st) {
        case 0: strcpy((char *)str_status, "OFF"); break;
        default: strcpy((char *)str_status, "  ON  "); break;
    }

    switch (L) {
        case 1: y = 29; break;
        case 2: y = 38; break;
        case 3: y = 47; break;
        default: y = 56; break;
    }

    LCD_Graphic_send_text(screen, "              ", x, y);

    sprintf(mess, "%s", str_status);
    LCD_Graphic_send_text(screen, mess, x, y);
}

void IN_set(uint8_t *screen, uint8_t L, uint8_t st1, uint8_t st2) {
    uint8_t str_st1[11];
    uint8_t str_st2[11];
    uint8_t x = 21, y = 10;

    switch (st1) {
        case 0: strcpy((char *)str_st1, "      "); break;
        default: strcpy((char *)str_st1, "<>"); break;
    }

    switch (st2) {
        case 0: strcpy((char *)str_st2, "      "); break;
        default: strcpy((char *)str_st2, "<>"); break;
    }

    switch (L) {
        case 1: x = 21; break;
        case 2: x = 48; break;
        case 3: x = 75; break;
        default: x = 102; break;
    }
    
    sprintf(mess, "%s", str_st1);
    LCD_Graphic_send_text(screen, mess, x, y);
    sprintf(mess, "%s", str_st2);
    LCD_Graphic_send_text(screen, mess, x + 13, y);
}

void OUT_set(uint8_t *screen, uint8_t L, uint8_t st) {
    uint8_t str_status[7];
    uint8_t x = 118, y = 19;

    switch (st) {
        case 0:
            strcpy((char *)str_status, "OFF");
            break;
        default:
            strcpy((char *)str_status, "  ON  ");
            break;
    }

    switch (L) {
        case 1: x = 26; break;
        case 2: x = 53; break;
        case 3: x = 80; break;
        default: x = 107; break;
    }
  
    sprintf(mess, "%s", str_status);
    LCD_Graphic_send_text(screen, mess, x, y);
}

/* Screen 3 */
void Screen_3(uint8_t *screen) {
    while (Lock) vTaskDelay(1 / portTICK_PERIOD_MS);
    Lock = 1;

    Current_screen = SCREEN_NUM_3;

    Clean_data_screen(screen);
    LCD_Graphic_send_text(screen, "DC", 2, 25);
    LCD_Graphic_send_text(screen, "1", 25, 20);
    LCD_Graphic_send_text(screen, "2", 54, 20);
    LCD_Graphic_send_text(screen, "3", 83, 20);
    LCD_Graphic_send_text(screen, "4", 112, 20);
    LCD_Graphic_send_text(screen, "RF485:", 47, 48);
    Merge_screen(screen, Frame_3);
    Screen_OFF();
    LCD_draw_screen(screen);
    vTaskDelay(DELAY / portTICK_PERIOD_MS);
    Screen_ON();

    Lock = 0;
    vTaskDelay(10 / portTICK_PERIOD_MS);
}

void Switch_MODE(uint8_t *screen, bool sw) {
    if (sw) {
        switch (mode) {
            case 1: mode = 0; break;
            default: mode = 1; break;
        }
    }

    if (mode) LCD_Graphic_send_text(screen, "Manual", 51, 8);
    else LCD_Graphic_send_text(screen, "     Auto     ", 51, 8);

    LCD_draw_screen(screen);
}

void DC_set(uint8_t *screen, float dc1, float dc2, float dc3, float dc4) {
    float ls[4] = {dc1, dc2, dc3, dc4};
    uint8_t cent[4];

    LCD_Graphic_send_text(screen, "                       ", 14, 30);
    LCD_Graphic_send_text(screen, "                       ", 43, 30);
    LCD_Graphic_send_text(screen, "                       ", 72, 30);
    LCD_Graphic_send_text(screen, "                       ", 101, 30);

    for (uint8_t i = 0; i < 4; i++) {
        if (ls[i] > 9999) cent[i] = 0;
        else if (ls[i] > 999) cent[i] = 1;
        else if (ls[i] > 99) cent[i] = 2;
        else if (ls[i] > 9) cent[i] = 3;
        else if (ls[i] < 0) cent[i] = 3;
        else cent[i] = 4;
    }

    sprintf(mess, "%.*f", cent[0], dc1);
    LCD_Graphic_send_text(screen, mess, 14, 30);
    sprintf(mess, "%.*f", cent[1], dc2);
    LCD_Graphic_send_text(screen, mess, 43, 30);
    sprintf(mess, "%.*f", cent[2], dc3);
    LCD_Graphic_send_text(screen, mess, 72, 30);
    sprintf(mess, "%.*f", cent[2], dc4);
    LCD_Graphic_send_text(screen, mess, 101, 30);
}

void RF485_status(uint8_t *screen, bool st) {
    if (st) LCD_Graphic_send_text(screen, "`", 78, 48);
    else LCD_Graphic_send_text(screen, "   ", 78, 48);
    LCD_draw_screen(screen);
}

void Skytechnology_logo(void) {
    Screen_OFF();
    vTaskDelay(DELAY / portTICK_PERIOD_MS);
    LCD_draw_screen(Logo_Skytech);
    Screen_ON();
    vTaskDelay(2000 / portTICK_PERIOD_MS);
}