#include "osapi.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "ets_sys.h"
#include "c_types.h"
#include "espconn.h"
#include "mem.h"
LOCAL struct espconn user_tcp_espconn;
LOCAL struct _esp_tcp user_tcp;
uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;
    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;
        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;
        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;
        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;
        case FLASH_SIZE_64M_MAP_1024_1024:
            rf_cal_sec = 2048 - 5;
            break;
        case FLASH_SIZE_128M_MAP_1024_1024:
            rf_cal_sec = 4096 - 5;
            break;
        default:
            rf_cal_sec = 0;
            break;
    }
    return rf_cal_sec;
}
void ICACHE_FLASH_ATTR
user_rf_pre_init(void)
{
}
/*Funcion de respuesta ante recepción de información
incluye mostrar por uart lo recibido y devolver un
mensaje de acuse que notifique que la información
llegó correctamente*/
void ICACHE_FLASH_ATTR
server_recv(void *arg, char *pdata, unsigned short len)
{
  os_printf("\n\r Información: %s \r\n", pdata);
  espconn_sent((struct espconn *) arg, "Informacion recibida\r\n", strlen("Información recibida\r\n"));
}
//Función de respuesta ante envío de información
void ICACHE_FLASH_ATTR
server_sent(void *arg)
{
  os_printf("Envío exitoso!\r\n");
}
//Función de respuesta ante desconexión
void ICACHE_FLASH_ATTR
server_discon(void *arg)
{
  os_printf("Desconectado! \r\n");
}
/*Función de respuesta ante un servidor
creado exitosamente*/
void ICACHE_FLASH_ATTR
server_listen(void *arg)
{
  struct espconn *pespconn = arg;
/*Declaración de las funciones de respuesta ante
transmisión y recepción de información*/
  espconn_regist_recvcb(pespconn, server_recv);
  espconn_regist_sentcb(pespconn, server_sent);
  espconn_regist_disconcb(pespconn, server_discon);
}
/*Función de respuesta ante un error en la
creación del servidor*/
void ICACHE_FLASH_ATTR
server_recon(void *arg, sint8 err)
{
  os_printf("Error de conexión, código de error: %d\r\n", err);
}
/*Función de  configuración para la creación
del servidor*/
void init_tcp(uint32_t Local_port)
{
  user_tcp_espconn.proto.tcp = &user_tcp;
  user_tcp_espconn.type = ESPCONN_TCP;
  user_tcp_espconn.proto.tcp->local_port = Local_port;
  /*Declaración de las funciones de respuesta ante
  la creación del servidor*/
  espconn_regist_connectcb(&user_tcp_espconn, server_listen);
  espconn_regist_reconcb(&user_tcp_espconn, server_recon);
  //Inicio del servidor
  espconn_accept(&user_tcp_espconn);
}
//Función para configurar el WIFI
void ap_config_func()
{
  struct softap_config config;
//Modo estación+punto de acceso
  wifi_set_opmode(STATIONAP_MODE);
  wifi_softap_get_config(&config);
//Nombre y contraseña de servidor que se creará
  os_memcpy(config.ssid, "esp8266_go",strlen("esp8266_go"));
  os_memcpy(config.password,"esp123",strlen("esp123"));
  config.ssid_len = strlen("esp8266_go");
  wifi_softap_set_config(&config);
}

void ICACHE_FLASH_ATTR
user_init(void)
{
    uart_init(BIT_RATE_115200, BIT_RATE_115200);
    os_printf("SDK version:%s\n", system_get_sdk_version());
//Llamada de función para la configuración del servidor
    ap_config_func();
/*Inicio del protocolo de comuicación por tcp
el argumento de la función es el puerto por el que se transmitirá
la información*/
    init_tcp(8266);
}
