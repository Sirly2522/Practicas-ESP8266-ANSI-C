#include "osapi.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "ets_sys.h"
#include "c_types.h"
#include "espconn.h"
#include "mem.h"

LOCAL struct espconn user_tcp_espconn;
LOCAL struct _esp_tcp user_tcp;


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


//subrutina para enviar de vuelta lo que el uart cargo en el buffer de recepción cada 4 segundos
static os_timer_t buff_timer_t;
void uart_test_rx()
{
    //os_printf("\nfase A\n");
    char uart_buf[128]={0};
    char len = 0;
    len = rx_buff_deq(uart_buf, 128 );
    tx_buff_enq(uart_buf,len);
    //os_printf("\nRecibido:%s\n", uart_buf);
}

//subrutina para transmitir una cadena cada 2 segundos
static os_timer_t buff_timer_t1;
void ICACHE_FLASH_ATTR
transf_txfifo(void *arg)
{
  const char* test_str;
  test_str = "\r\n La transmision por uart funciona! \r\n";
  uart0_sendStr(test_str);
}


void ICACHE_FLASH_ATTR
user_init(void)
{
    // Inicia los uart a 115200 baudios y muestra la version del SDK
    uart_init(BIT_RATE_115200, BIT_RATE_115200);

    // Disable WiFi
    //wifi_set_opmode(NULL_MODE);

    os_printf("\nSDK version:%s\n", system_get_sdk_version());

  /*
  uint16 rx_buff_deq(char* pdata, uint16 data_len )
  */
  /*
  tx_start_uart_buffer(UART0); //txbuffer to -> txfifo (for fifo empty interrupt)
  */
  /*
  Uart_rx_buff_enq();   //fifo to -> rxbuffer
  */
  /*
  tx_buff_enq(uart_buf,len);  //tx_buff_enq(char* pdata, uint16 data_len ) fill tx buffer
  */

  //Envío dato a dato al buffer de transmisión
  #if 0  // transmiter
  {
  char mar, bae, hol, kin, tel, var;
  const char* et;
  uint8 a = 0;
  et = "hola_mundo!\n";
  bae = '\n';
  mar = 'h';
  hol = 'o';
  kin = 'l';
  tel = 'a';
  tx_buff_enq(&bae, sizeof(bae));
  tx_buff_enq(&mar, sizeof(mar));
  tx_buff_enq(&hol, sizeof(hol));
  tx_buff_enq(&kin, sizeof(kin));
  tx_buff_enq(&tel, sizeof(tel));
  //os_printf("\nfase A\n");
    while(a<=11)
    {
      var = et[a];
      tx_buff_enq(&var, sizeof(var));

      a = a + 1;
    }
    //os_printf("\nfase b\n");
  //tx_start_uart_buffer(UART0);

  }
  #endif

  // envío de cadena al txfifo de forma repetitiva cada dos segundos
  #if 1 //transmisor
  {


    os_timer_disarm(&buff_timer_t1);
    os_timer_setfn(&buff_timer_t1, transf_txfifo, NULL);
    os_timer_arm(&buff_timer_t1, 2000, 1);
  }
  #endif
  // recepcion por buffer
  #if 0 //receiver
  {
    // Declaración del puerto como receptor para el uart0
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U , FUNC_U0RXD);



    os_timer_disarm(&buff_timer_t);
    os_timer_setfn(&buff_timer_t, uart_test_rx , NULL);   //a demo to process the data in uart rx buffer
    os_timer_arm(&buff_timer_t,4000,1);
  }
  #endif

}
