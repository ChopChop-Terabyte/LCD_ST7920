set (
    Header
    "WIFI" "SNTP" "LCD_128x64" "GPIO"
)
set (
    Source
    "WIFI/WIFI.c"
    "SNTP/SNTP.c"
    "LCD_128x64/ST7920.c"
    "GPIO/GPIO_cfg.c"
)

idf_component_register(SRCS "main.c" ${Source}
                    INCLUDE_DIRS "." ${Header})
