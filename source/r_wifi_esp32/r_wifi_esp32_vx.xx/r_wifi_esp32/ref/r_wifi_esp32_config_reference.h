#ifndef R_WIFI_ESP32_CONFIG_H
#define R_WIFI_ESP32_CONFIG_H

#define WIFI_CFG_SCI_CHANNEL			0

#define WIFI_CFG_SCI_INTERRUPT_LEVEL    14

#define WIFI_CFG_SCI_BOUDRATE           3750000

#define WIFI_CFG_SCI_USE_FLOW_CONTROL		1

#define WIFI_CFG_RESET_PORT					D
#define WIFI_CFG_RESET_PIN					0

#define WIFI_CFG_CREATABLE_SOCKETS   		5

#define WIFI_CFG_SOCKETS_RECEIVE_BUFFER_SIZE  4096

#define WIFI_CFG_USE_CALLBACK_FUNCTION   1

#define WIFI_CFG_CALLBACK_FUNCTION_NAME   R_wifi_callback


#endif /* #define R_WIFI_ESP32_CONFIG_H */