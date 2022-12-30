/*
 * ESP32.c
 *
 *  Created on: November 2022
 *      Author: Ben Chedly Abdelhamid & Akkari Nada
 */


#include "stm32f3xx_hal.h"
#include "stdio.h"
#include "string.h"
#include "stm32f3xx_hal_uart.h"
#include <string.h>
#include "ESP32.h"


extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;

extern PARAM param;

#define wifi_uart &huart3
#define pc_uart &huart2

char buffer[20];


#define RxBuf_SIZE   50
#define MainBuf_SIZE 100

int port = 80;
uint8_t RxBuf[RxBuf_SIZE];
uint8_t MainBuf[MainBuf_SIZE];
uint16_t oldPos = 0;
uint16_t newPos = 0;
uint8_t isOk = 0;
uint8_t Res[20];


uint8_t CWMODE[] = "AT+CWMODE=1\r\n";
uint8_t data[50];
uint8_t OK[] = "OK \r\n";
uint8_t Fail[] = "Failed \r\n";
uint8_t reset[] = "AT+RST\r\n";
uint8_t stat = 0;
uint8_t AT[] = "AT\r\n";
uint8_t disp[] = "\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2 Projet Tutoré \xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2\xB2 \n\r";



char *GET_req = "GET/handler.php?";
char IP[] = "192.168.1.12";
char *variable = "temp=%d,%d&hum=%d";



void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
  /* Prevent unused argument(s) compilation warning */
	  oldPos = newPos; //Updating the last position before copying new data
	  if (oldPos+Size > MainBuf_SIZE)
	  {
		  uint16_t datatocopySize = MainBuf_SIZE - oldPos;
		  memcpy((uint8_t * )MainBuf+oldPos, RxBuf, datatocopySize);
		  oldPos = 0;  // point to the start of the buffer
		  memcpy((uint8_t *)MainBuf, (uint8_t *)RxBuf+datatocopySize, (Size-datatocopySize));  //Copying the remaining data
		  newPos = (Size - datatocopySize);  //updating the position
	  }

	  else
	  {
		  memcpy((uint8_t *)MainBuf+oldPos , RxBuf, Size);
		  newPos = oldPos + Size;
	  }

	  for (int i=0; i<Size; i++)
	  {
		  if((RxBuf[i] == 'O') && (RxBuf[i+1] == 'K'))
		{
			isOk = 1;
		}
	  }
	  // start the DMA receive again
	  HAL_UARTEx_ReceiveToIdle_DMA(huart, RxBuf, RxBuf_SIZE);
	  __HAL_DMA_DISABLE_IT(huart->hdmarx, DMA_IT_HT);
  }


static void uartSend (UART_HandleTypeDef uart ,char *str)
{
	HAL_UART_Transmit_IT(&uart, (uint8_t *)str, strlen (str));
	HAL_Delay(100);
	HAL_UART_Receive_DMA(&uart,RxBuf, sizeof(RxBuf));
	__HAL_DMA_DISABLE_IT(uart.hdmarx, DMA_IT_HT);
}

static void debugLog (char *str)
{
	HAL_UART_Transmit_IT(pc_uart, (uint8_t *) str, strlen (str));
}

/*****************************************************************************************************************************************/

void ESP_Init (PARAM param)
{
	  uint8_t Txt[] = "Please select the mode \n\r * 1 : Being the client mode \n\r * 2 : Being the Access Point Mode \n\r * 3 : Being the Client / Access Point Mode \n\r";
	  debugLog(disp);
	  HAL_Delay(100);
	  debugLog(Txt);

	  /********* AT+RST **********/
	  uartSend(param.UART, reset);

	  if (isOk == 1){
		  sprintf(Res,reset, OK);
	  }
	  else
	  {
		  sprintf(Res,reset, Fail);
	  }
	  debugLog(Res);
	  memset(Res,'0',sizeof(Res));

	  /********* AT **********/

	  uartSend(param.UART, AT);
	  debugLog(Res);
	  HAL_Delay(100);
	  HAL_UART_Transmit_IT(pc_uart,Res,sizeof(Res));
	  memset(Res,'0',sizeof(Res));

	  /********* AT+CWMODE=1 **********/

	  if (param.Mode == "1")
	  {
		  uartSend(param.UART, "AT+CWMODE=1\r\n");
		  /********* AT+CWJAP="SSID","PASSWD" **********/
		  sprintf (data, "AT+CWJAP=\"%s\",\"%s\"\r\n", param.SSID, param.PASSWD);
		  uartSend(param.UART, data);
		  memset(Res,'0',sizeof(Res));
		  sprintf (Res, "Connected to,\"%s\"\n\n", param.SSID);
		  debugLog(Res);
		  uartSend(param.UART, "AT+CIFSR\r\n");
		  int len = strlen (RxBuf);
		  RxBuf[len-1] = '\0';
		  sprintf (data, "IP ADDR: %s\n\n", RxBuf);
		  debugLog(data);

	  }

	     /********* AT+CWMODE=2 **********/

	  if (param.Mode == "2")
	  {
		  uartSend(param.UART, "AT+CWMODE=2\r\n");
		  /********* AT+CWJAP="SSID","PASSWD" **********/
		  sprintf (data, "AT+CWSAP=\"%s\",\"%s\"\r\n", param.SSID, param.PASSWD);
		  uartSend(param.UART, data);
		  memset(Res,'0',sizeof(Res));
		  sprintf (Res, "Connected to,\"%s\"\n\n", param.SSID);
		  debugLog(Res);
		  uartSend(param.UART, "AT+CIPMUX=1\r\n");
		  uartSend(param.UART, "AT+CIPSERVER=1\r\n");
		  uartSend(param.UART, "AT+CIFSR\r\n");
		  int len = strlen (RxBuf);
		  RxBuf[len-1] = '\0';
		  sprintf (data, "IP ADDR: %s\n\n", RxBuf);
		  debugLog(data);
	  }

	  if (param.Mode == "3")
	  {
		  uartSend(param.UART, "AT+CWMODE=3\r\n");

		  /********* AT+CWJAP="SSID","PASSWD" **********/
		  sprintf (data, "AT+CWJAP=\"%s\",\"%s\"\r\n", param.SSID, param.PASSWD);
		  uartSend(param.UART, data);
		  memset(Res,'0',sizeof(Res));
		  sprintf (Res, "Connected to,\"%s\"\n\n", param.SSID);
		  debugLog(Res);
		  uartSend(param.UART, "AT+CIFSR \r\n"); /* GET IP Address */
	  }
}

void ESP_COM(PARAM param)
{
	char connectdata[1024] = {0};
	char datatosend[1024];
	 if (param.Mode == "1")
	 {
		 sprintf (connectdata, "AT+CIPSTART=\"%d\",\"%s\", %d \r\n", "TCP", IP, port);
		 uartSend(param.UART, connectdata); /* START THE CONNECTION (TCP / UDP) */
		 uartSend(param.UART, "AT+CIPMODE=1 \r\n"); /* Enable Wifi passthrough mode */
	 }

	 memset(datatosend,"0",sizeof(datatosend));
	 sprintf(datatosend,GET_req, "?", variable, ' HTPP/1.1\t\n Host:',IP,"r\n\r\n");
	 sprintf (datatosend, "AT+CIPSEND=%d\r\n", sizeof(datatosend));
	 uartSend(param.UART, datatosend);
	 HAL_Delay(100);

	 uartSend(param.UART, GET_req);

	 sprintf (data, "AT+CIPCLOSE\r\n");
	 uartSend(param.UART, data);

	 if (Look_for("200", RxBuf) == "1")
		{
			debugLog("Request Send to server");
		}
}

int CheckforOK()
{
	if (isOk == 1)
		  {
			  sprintf(Res,reset, OK);
			  return 1;
		  }
		  else
		  {
			  sprintf(Res,reset, Fail);
			  return 0;
		  }
}

int Look_for (char *str, char *buffertolookinto)
{
	int stringlength = strlen (str);
	int bufferlength = strlen (buffertolookinto);
	int so_far = 0;
	int indx = 0;
repeat:
	while (str[so_far] != buffertolookinto[indx]) indx++;
	if (str[so_far] == buffertolookinto[indx]){
	while (str[so_far] == buffertolookinto[indx])
	{
		so_far++;
		indx++;
	}
	}

	else
		{
			so_far =0;
			if (indx >= bufferlength) return -1;
			goto repeat;
		}

	if (so_far == stringlength) return 1;
	else return -1;
}

