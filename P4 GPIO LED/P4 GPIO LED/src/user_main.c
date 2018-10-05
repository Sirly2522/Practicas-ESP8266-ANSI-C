#include "osapi.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "ets_sys.h"
#include "eagle_soc.h"
#include "gpio.h"
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
//Presentación de las variables
static os_timer_t sw_timer;
static char flag_gpio = 0;
static const char* test_str;
static const char* cont;
unsigned int cont1 = 0;
//Rutina para la interrupción del timer
void ICACHE_FLASH_ATTR
sw_timer_cb(void *arg)
{
  if(flag_gpio == 0)
  {
    //Coloca el GPIO2 en nivel bajo
    GPIO_OUTPUT_SET(GPIO_ID_PIN(2), 0);
    //Bandera de fase de encendido
    flag_gpio = 1;
    //Cadena que muestra por el UART0
    os_printf("\n\nContador:%d\n",cont1);
    test_str = "\nApagado\n";
    uart0_sendStr(test_str);
  }
  else
  {
    //Coloca el GPIO2 en nivel alto
    GPIO_OUTPUT_SET(GPIO_ID_PIN(2), 1);
    //Bandera de fase de encendido
    flag_gpio = 0;
    //Cadena que muestra por el UART0
    test_str = "\nEncendido\n";
    uart0_sendStr(test_str);
//Mensaje que muestra una vez el led enciende cinco veces
        if(cont1==5)
        {
          os_printf("El led encendio %d veces!!!\n",cont1);
        }
//Incremento del contador de nivel alto
    cont1 = cont1 + 1;
//Reseteo de contador cuando cuenta mas de 5
        if(cont1>5)
        {
          cont1=0;
          os_printf("\n\nConteo reseteado!!!!!\n\n");
        }
  }
}
void ICACHE_FLASH_ATTR
user_init(void)
{
    uart_init(BIT_RATE_115200, BIT_RATE_115200);

    wifi_set_opmode(NULL_MODE);
    os_printf("SDK version:%s\n", system_get_sdk_version());

    //GPIO
    // Declaración del puerto 2 como GPIO
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);

    os_timer_disarm(&sw_timer);
    os_timer_setfn(&sw_timer, sw_timer_cb, NULL);
    os_timer_arm(&sw_timer, 2000, 1);
}
