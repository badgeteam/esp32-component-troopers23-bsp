idf_component_register(
    SRCS "hardware.c"
         "wifi_connection.c"
         "wifi_connect.c"
        "rainbow.c"
    INCLUDE_DIRS "." "include"
    REQUIRES
        "bus-i2c"
        "spi-ili9341"
        "keyboard"
        "wpa_supplicant"
        "nvs_flash"
        "pax-graphics"
        "spi-cc1200"
        "i2c-controller"
        "i2c-ktd2052"
)
