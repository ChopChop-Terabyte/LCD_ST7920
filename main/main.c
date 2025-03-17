#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#include "SNTP.h"
#include "WIFI.h"
#include "GPIO_cfg.h"
#include "ST7920.h"

#define SW1 GPIO_NUM_12
#define SW2 GPIO_NUM_14
#define LED GPIO_NUM_19
#define GPIO_INPUT_MAP  ((1ULL << SW1) | (1ULL << SW2))
#define GPIO_OUTPUT_MAP (1ULL << LED)

// #define SSID_WIFI_STA "HattenNeverDie"
// #define PASS_WIFI_STA "hatten123"
#define SSID_WIFI_STA "SkyTech"
#define PASS_WIFI_STA "skytech2024"
WIFI_STA_connection Wifi_z = {
    .SSID_STA = SSID_WIFI_STA,
    .PASS_STA = PASS_WIFI_STA
};

#define SCL GPIO_NUM_22
#define SDA GPIO_NUM_21
// #define SCL GPIO_NUM_9
// #define SDA GPIO_NUM_8
#define MASTER_NUM I2C_NUM_0
#define FREQ_HZ 400000
#define I2C_LCD_ADDRESS 0x20
LCD_config_t LCD_cfg = {
    .SCL_Pin = SCL,
    .SDA_Pin = SDA,
    .Port_num = MASTER_NUM,
    .Clock_freq = FREQ_HZ,
    .LCD_Address = I2C_LCD_ADDRESS
};

void Real_time() {
    uint8_t screen_change = 0;
    uint8_t s_current = timeinfo.tm_sec;
    uint8_t d_current = timeinfo.tm_mday;

    while (1) {
        while (Lock) vTaskDelay(1 / portTICK_PERIOD_MS);
        // while (Ready_change);
        Lock = 1;
        Get_time();

        if (screen_change != Current_screen) {
            screen_change = Current_screen;

            HMS(Screen, Current_screen, timeinfo.tm_hour, timeinfo.tm_min, s_current);
            Date_DMY(Screen, Current_screen, timeinfo.tm_wday, timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
        }

        if (s_current != timeinfo.tm_sec) {
            s_current = timeinfo.tm_sec;
            HMS(Screen, Current_screen, timeinfo.tm_hour, timeinfo.tm_min, s_current);
        }
        if (d_current != timeinfo.tm_mday) {
            d_current = timeinfo.tm_mday;
            Date_DMY(Screen, Current_screen, timeinfo.tm_wday, timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
        }
        Lock = 0;
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void Switch_watching() {
    Screen_3(Screen);

    while (1) {
        if (gpio_get_level(SW1) == 0) {
            switch (Current_screen)
            {
                case SCREEN_NUM_1:
                    Screen_2(Screen);
                    break;
                case SCREEN_NUM_2:
                    Screen_3(Screen);
                    break;
                case SCREEN_NUM_3:
                    Screen_1(Screen);
                    break;
                default:
                    break;
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void app_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    GPIO_Input_init(GPIO_INPUT_MAP, 1);
    GPIO_Output_init(GPIO_OUTPUT_MAP);
    WIFI_STA_config(Wifi_z);

/* LCD init */
    LCD_I2C_init(LCD_cfg);
    LCD_config();
    Graphic_mode_ON();

    // LOGO Skytech
    // Skytechnology_logo();

/* Real time */
    // xTaskCreate(&Real_time, "Update time every second", 4 * 1024, NULL, 1, NULL);

/* Button event */
    // xTaskCreate(&Switch_watching, "Switch watching", 4 * 1024, NULL, 2, NULL);


///////////////////////////////////////////////////////////////////////////////////////////
// /* For Screen 1 */
//     Screen_1(Screen);
//     Carrier_name(Screen, (uint8_t *)"MobiPhone");
//     Network_signal(Screen, 5);
//     Date_DMY(Screen, Current_screen, timeinfo.tm_wday, timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
//     HMS(Screen, Current_screen, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
//     Temp_hum_set(Screen, 24, 7);

//     /*  Group  */
//     Volt_set(Screen, 1.32, 23.68, 215.4);
//     Ampe_set(Screen, 1.32, 23.68, 215.4);
//     Pow_set(Screen, 1.32, 23.68, 215.4);
//     Pow_per_time_set(Screen, 126.32, -1.23453, 1.7);

//     LCD_draw_screen(Screen);
//     /*  END Group  */

///////////////////////////////////////////////////////////////////////////////////////////
// /* For Screen 2 */
//     Screen_2(Screen);
//     Date_DMY(Screen, Current_screen, timeinfo.tm_wday, timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
//     HMS(Screen, Current_screen, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

//     IN_set(Screen, 1, 1, 1);
//     IN_set(Screen, 2, 1, 0);
//     IN_set(Screen, 3, 0, 1);
//     IN_set(Screen, 4, 1, 1);
//     OUT_set(Screen, 1, 1);
//     OUT_set(Screen, 2, 0);
//     OUT_set(Screen, 3, 1);
//     OUT_set(Screen, 4, 0);

//     /*  Group  */
//     History_time_set(Screen, 1, 12, 30, 2, 7, 1, 2025);
//     Status_set(Screen, 1, 1);
//     History_time_set(Screen, 2, 12, 30, 2, 7, 1, 2025);
//     Status_set(Screen, 2, 0);
//     History_time_set(Screen, 3, 12, 30, 2, 7, 1, 2025);
//     Status_set(Screen, 3, 1);
//     History_time_set(Screen, 4, 12, 30, 2, 7, 1, 2025);
//     Status_set(Screen, 4, 0);

//     LCD_draw_screen(Screen);
//     /*  END Group  */

///////////////////////////////////////////////////////////////////////////////////////////
/* For Screen 3 */
    Screen_3(Screen);
    Date_DMY(Screen, Current_screen, timeinfo.tm_wday, timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
    HMS(Screen, Current_screen, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

    DC_set(Screen, 1234, 120, 10, -1);

    RF485_status(Screen, 1);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    RF485_status(Screen, 0);

    Switch_MODE(Screen, 0); // Print
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    Switch_MODE(Screen, 1); // Switch and print
}