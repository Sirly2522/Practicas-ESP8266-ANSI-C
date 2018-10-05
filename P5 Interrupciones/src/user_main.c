#include "osapi.h"
#include "user_interface.h"
#include "driver/uart.h"
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
void ICACHE_FLASH_ATTR
user_rf_pre_init(void)
{
}
//bandera para determinar en que flanco esta la interrupción
static char flag_de_flanco = 0;
//Rutina de interrupción
static void
gpio_intr_handler()
{
  uint32 status_gpio = GPIO_REG_READ(GPIO_STATUS_ADDRESS); //Lee el estado de la interrupción
//Deshabilita las interrupciones por IO
  ETS_GPIO_INTR_DISABLE();
//Protocolo de respuesta si la interrupción de IO fue en el puerto 2
  if( status_gpio & BIT(2))
  {
    if(flag_de_flanco==0)
    {
      os_printf("Interrupcion activada en nivel bajo \n");
//Cambia el modo de la interrupción a nivel alto
      gpio_pin_intr_state_set(GPIO_ID_PIN(2), GPIO_PIN_INTR_HILEVEL);
      flag_de_flanco = 1;
    }
    else
    {
      os_printf("Interrupcion activada en nivel alto \n");
//Cambia el modo de la interrupción a nivel bajo
      gpio_pin_intr_state_set(GPIO_ID_PIN(2), GPIO_PIN_INTR_LOLEVEL);
      flag_de_flanco = 0;
    }
  }
// antirebote por software de 0.1 segundo
  os_delay_us(100000);
// Limpia la bandera de interrupcón
  GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, status_gpio);
// Habilita nuevamente las interrupciones
  ETS_GPIO_INTR_ENABLE();
}
void ICACHE_FLASH_ATTR
user_init(void)
{
    uart_init(BIT_RATE_115200, BIT_RATE_115200);
    os_printf("SDK version:%s\n", system_get_sdk_version());
    wifi_set_opmode(NULL_MODE);
    os_printf("\r\n''''''''''''''''''''''''''''''''''''''''''''''''''''\n\r");
    os_printf("\r\n''''''''''Esperando servicio de interrupción''''''''\n\r");
    os_printf("\r\n''''''''''''''''''''''''''''''''''''''''''''''''''''\n\r");
  //Declaraciones para el puerto 2
  //Función de GPIO
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
  //Pull up
    PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO2_U);
  //configuración como entrada
    GPIO_DIS_OUTPUT(GPIO_ID_PIN(2));
  //Interrupciones
  //1. Se deshabilita la interrupción
    ETS_GPIO_INTR_DISABLE();
  /*2. Se declara la funcion gpio_intr_handler como callback
  para las interrupción externas y de forma incondicional*/
    ETS_GPIO_INTR_ATTACH(gpio_intr_handler, NULL);
  //3. Se declara el puerto 2 como bandera para activar la interrucion en nivel bajo de entrada
    gpio_pin_intr_state_set(GPIO_ID_PIN(2), GPIO_PIN_INTR_LOLEVEL);
  //4. Se habilita la interrupción
    ETS_GPIO_INTR_ENABLE();
}
