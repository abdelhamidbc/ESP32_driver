/*
 * ESP32.h
 *
 *  Created on: November 14, 2022
 *      Author: Ben Chedly Abdelhamid & Akkari Nada
 */


#ifndef ESP32_H_INCLUDED
#define ESP32_H_INCLUDEDv



typedef struct
{
  uint8_t Mode[10] ;                  /*!< This is the mode.
                                           The baud rate is computed using the following formula:
                                           - IntegerDivider = ((PCLKx) / (8 * (OVR8+1) * (huart->Init.BaudRate)))
                                           - FractionalDivider = ((IntegerDivider - ((uint32_t) IntegerDivider)) * 8 * (OVR8+1)) + 0.5
                                           Where OVR8 is the "oversampling by 8 mode" configuration bit in the CR1 register. */
  uint8_t SSID[20];                /*!< Specifies the number of data bits transmitted or received in a frame.
                                           This parameter can be a value of @ref UART_Word_Length */

  uint8_t PASSWD[20];

  UART_HandleTypeDef UART;

} PARAM;


static void debugLog (char *str);
static void uartSend (UART_HandleTypeDef uart ,char *str);
void ESP_Init (PARAM parameter);
void Server_Start (void);
void ESP_COM(PARAM param);
int Look_for (char *str, char *buffertolookinto);


#endif /* INC_ESP8266_HAL_H_ */

