#include "osapi.h"
#include "user_interface.h"
#include "driver/uart.h"
/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABBBCDDD
 *                A : rf cal
 *                B : at parameters
 *                C : rf init data
 *                D : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
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

void ICACHE_FLASH_ATTR
sw_timer_cb(void *arg)
{
  // const char* test_str;
  // test_str = "\r\n El Timer por software funciona! \r\n";
  // uart0_sendStr(test_str);

  uint8 status; //Variable para guardar el status de conexión del wifi
  os_timer_disarm(&sw_timer);  //Deshabilitación del timer

  status = wifi_station_get_connect_status(); //Funcion para obtener el status de conexión wifi

  if(status == STATION_GOT_IP)
  {
    os_printf("\r\n Conexión realizada al wifi en modo estación \r\n"); //si se conecta muestra este mensaje
    return;
  }
  else
  {
    os_timer_arm(&sw_timer, 2000, 1); // sino se conecta reinicia el timer
  }
}



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
    // Inicia los uart a 115200 baudios y muestra la version del SDK
    uart_init(BIT_RATE_115200, BIT_RATE_115200);
    os_printf("SDK version:%s\n", system_get_sdk_version());

    //WIFI
    user_set_station_config();

    // Software Timer!
    //1. Se desactiva el timer para luego poder activarlo
    os_timer_disarm(&sw_timer);
    //2. Se define la función "sw_timer_cb" para ser llamada cuando el timer se desborde
    os_timer_setfn(&sw_timer, sw_timer_cb, NULL);
    //3. Se activa el timer de forma repetitiva con un tiempo de 2 segundos
    os_timer_arm(&sw_timer, 2000, 1);
}
