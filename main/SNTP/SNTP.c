#include "SNTP.h"

#define SNTP_TIMEOUT 5

struct tm timeinfo = {0};
static bool Time_synced = false;

void SNTP_Callback(struct timeval *tv) {
    ESP_LOGI("SNTP", "Sync successful");
    Time_synced = true;
}

void SNTP_Callback_timeout(void* arg) {
    if (!Time_synced) {
        ESP_LOGE("SNTP", "Time sync failed. You need to set the time manually!");
    }
}

void Get_time(void) {
    time_t now;
    time(&now);
    localtime_r(&now, &timeinfo);

    // while (1) {
    //     time(&now);
    //     localtime_r(&now, &timeinfo);

    //     ESP_LOGI("Time now", "%04d/%02d/%02d %1d %02d:%02d:%02d", timeinfo.tm_year + 1900, timeinfo.tm_mon, timeinfo.tm_mday, timeinfo.tm_wday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

    //     vTaskDelay(1000 / portTICK_PERIOD_MS);
    // }
    // vTaskDelete(NULL);
}

void SNTP_config(void) {
    setenv("TZ", "UTC-7", 1);
    tzset();

    ESP_LOGI("SNTP", "Time syncing...");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "time.google.com");
    esp_sntp_setservername(1, "pool.ntp.org");

    esp_sntp_set_time_sync_notification_cb(SNTP_Callback);
    esp_sntp_set_sync_interval(60 * 60 * 1000); // 60m * 60s * 1000ms = 3.600.000ms = 1h

    esp_sntp_init();

    // Time out when sync unsuccessful
    const esp_timer_create_args_t timeout_args = {
        .callback = &SNTP_Callback_timeout,
        .arg = NULL,
        .name = "SNTP sync failed"
    };
    esp_timer_handle_t timeout_handle;
    ESP_ERROR_CHECK(esp_timer_create(&timeout_args, &timeout_handle));
    ESP_ERROR_CHECK(esp_timer_start_once(timeout_handle, SNTP_TIMEOUT * 1000000));  // Timeout sau SNTP_TIMEOUT giây

    // xTaskCreate(&Get_time, "Get current time", 2 * 1024, NULL, 1, NULL);
}