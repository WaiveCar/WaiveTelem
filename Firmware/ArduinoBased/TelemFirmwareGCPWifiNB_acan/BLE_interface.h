#include <STBLE.h>
//note, STBLE.h and corresponding header files have been editted to put the BLE chip onto it's own SPI bus.
// this SPI bus is set for MOSI as pin D0, SCK as pin D1, MISO as pin D6.  
// `CS is hardcoded as A1, IRQ as A2, and `RESET as A3

uint8_t ble_rx_buffer[21];
uint8_t ble_rx_buffer_len = 0;
uint8_t ble_connection_state = false;
#define PIPE_UART_OVER_BTLE_UART_TX_TX 0
