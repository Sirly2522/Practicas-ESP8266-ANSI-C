#include "osapi.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "ets_sys.h"
#include "c_types.h"
#include "espconn.h"
#include "mem.h"
static struct espconn user_tcp_conn;
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
static os_timer_t sw_timer;

/*Rutina de respuesta ante una recepción TCP*/
LOCAL void ICACHE_FLASH_ATTR
user_tcp_recv_cb(void *arg, char *pusrdata, unsigned short length)
{
  struct espconn *pespconn = arg;
  os_printf("tcp received !!! %s\r\n", pusrdata);
  //Envia de vuelta la información recibida
  espconn_send(pespconn, pusrdata, length);
}

/*Rutina de respuesta ante un envío de transmisión*/
LOCAL void ICACHE_FLASH_ATTR
user_tcp_sent_cb(void *arg)
{
  os_printf("\r\nEnvío exitoso de información!!! \r\n");
}

/*Rutina de respuesta ante la desconexión TCP*/
LOCAL void ICACHE_FLASH_ATTR
user_tcp_discon_cb(void *arg)
{
    os_printf("\r\nTCP desconectado!!! \r\n");
}

/*Rutina de respuesta si se carga información en el buffer TCP*/
LOCAL void ICACHE_FLASH_ATTR
user_tcp_write_finish(void *arg)
{
}

/*Rutína de respuesta para envío de información TCP*/
LOCAL void ICACHE_FLASH_ATTR
user_sent_data(struct espconn *pespconn)
{
  espconn_send(pespconn, "Hola mundo! desde el microcontrolador esp8266",46);
}

/*Rutina de respuesta ante una conexión TCP*/
LOCAL void ICACHE_FLASH_ATTR
user_tcp_connect_cb(void *arg)
{
  struct espconn *pespconn = arg;
  os_printf("\r\nConexión TCP exitosa!!! \r\n");
//Declaración de rutinas de respuesta ante distintos eventos
  espconn_regist_recvcb(pespconn, user_tcp_recv_cb);
  espconn_regist_sentcb(pespconn, user_tcp_sent_cb);
  espconn_regist_disconcb(pespconn, user_tcp_discon_cb);
//Habilita la escritura en el buffer
  espconn_set_opt(pespconn, 0x04);
  espconn_regist_write_finish(pespconn, user_tcp_write_finish);
//Envío de información por TCP
  user_sent_data(pespconn);
}

/*Rutina de respuesta en caso de error de conexión TCP*/
LOCAL void ICACHE_FLASH_ATTR
user_tcp_recon_cb(void *arg, sint8 err)
{
  // Error en la conexión TCP.
  os_printf("\r\nreconnect callback, código de error %d !!! \r\n",err);
}

//Rutina para conectarse a TCP
void ICACHE_FLASH_ATTR
espconn_tcp_client_connect()
{
  user_tcp_conn.proto.tcp = &user_tcp;
  user_tcp_conn.type = ESPCONN_TCP;
  user_tcp_conn.state = ESPCONN_NONE;
  os_printf("\r\n Cliente TCP conectado\r\n");
/*Dirección de IP a donde se quiera conectar,
en este caso 192 168 43 232*/
  uint8 esp_tcp_server_ip[4] = {192, 168, 43, 232};
  os_memcpy(user_tcp_conn.proto.tcp->remote_ip, esp_tcp_server_ip, 4);
//Puerto al que se quiera conectar,en este caso 8234
  user_tcp_conn.proto.tcp->remote_port = 8234;
  user_tcp_conn.proto.tcp->local_port = espconn_port();
//Declaración de rutinas de respuesta ante distintos eventos
  espconn_regist_connectcb(&user_tcp_conn, user_tcp_connect_cb);
  espconn_regist_reconcb(&user_tcp_conn, user_tcp_recon_cb);
// Conexión tcp
  espconn_connect(&user_tcp_conn);
  os_printf("\r\n Conectando a servidor TCP/IP\r\n");
}

//Rutina del protocolo para empezar a conectarse a TCP IP
void ICACHE_FLASH_ATTR
sw_timer_cb(void *arg)
{
  //Variable para guardar el status de conexión del wifi
  uint8 status;
  struct ip_info ipconfig;
  //Deshabilitación del timer
  os_timer_disarm(&sw_timer);
  //Funcion para obtener el status de conexión wifi
  status = wifi_station_get_connect_status();
  // función para obtener datos de la dirección de IP
  wifi_get_ip_info(STATION_IF, &ipconfig);
 //Si la IP obtenida es distinta de 0 y esta en modo estación de WIFI
  if(status = STATION_GOT_IP && ipconfig.ip.addr != 0)
  {
    os_printf("\r\n Conexión realizada al wifi en modo estación \r\n");
 //Funcion para tratar de conectarse a TCP IP
    espconn_tcp_client_connect();
    return;
      }
  else
  {
 // si aún no se cumplen las condiciones reinicia el timer
    os_timer_arm(&sw_timer, 2000, 1);
  }
}
//Rutina para conexión a la red WIFI
void ICACHE_FLASH_ATTR
user_set_station_config(void)
{
  struct station_config station_cfg;
  char ssid[] = "ESP8266";
  char password[] = "esp82661";
  wifi_set_opmode(STATION_MODE);
  station_cfg.bssid_set = 1;
  os_strcpy(station_cfg.ssid, ssid);
  os_strcpy(station_cfg.password, password);
  wifi_station_set_config(&station_cfg);
}
void ICACHE_FLASH_ATTR
user_init(void)
{
    uart_init(BIT_RATE_115200, BIT_RATE_115200);
    os_printf("SDK version:%s\n", system_get_sdk_version());
    //WIFI
    user_set_station_config();

    os_timer_disarm(&sw_timer);
    os_timer_setfn(&sw_timer, sw_timer_cb, NULL);
    os_timer_arm(&sw_timer, 2000, 1);
}
