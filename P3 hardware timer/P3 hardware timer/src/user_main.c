#include "osapi.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "driver/hw_timer.h"
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

void   hw_test_timer_cb(void)
{
  const char* test_str;
  test_str = "\r\n El Timer por hardware funciona! \r\n";
  uart0_sendStr(test_str);
}

void ICACHE_FLASH_ATTR
user_init(void)
{

    // Inicia los uart a 115200 baudios y muestra la version del SDK
    uart_init(BIT_RATE_115200, BIT_RATE_115200);
    os_printf("SDK version:%s\n", system_get_sdk_version());


    // Disable WiFi
    wifi_set_opmode(NULL_MODE);


    // Hardware Timer!
    //1.
    hw_timer_init(FRC1_SOURCE, 1);
    hw_timer_set_func(hw_test_timer_cb);
    hw_timer_arm(2000000);

}
