#ifndef SNTP_H
#define SNTP_H

#include <stdlib.h>
#include "esp_sntp.h"
#include "esp_timer.h"

#include "esp_log.h"

extern struct tm timeinfo;

void Get_time(void);
void SNTP_config(void);

#endif // SNTP_S