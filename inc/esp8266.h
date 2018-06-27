/* ###################################################################
**     Filename    : esp8266.h
**     Author	   : German Iturria
**
** ###################################################################*/

#ifndef ESP8266_H_
#define ESP8266_H_

/*==================[inclusions]=============================================*/

#include "sapi.h"

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif


typedef enum{GET_PAGE1, GET_PAGE2, GET_NO}get_st;
/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

bool_t ConfigHttpServer (char * wifiName, char * wifiPass,char * wifiChannel, char * wifiEnc, uartMap_t debugUart, uint32_t debugBaudRate);
bool_t ReadHttpServer   ();
bool_t WriteHttpServer  (char * webHttp);

char * GetIpAddress     ();
char * GetWifiName      ();
char * GetWifiPass      ();
char * SetIpAddress     (char *w);
char * SetWifiName      (char *w);
char * SetWifiPass      (char *w);
uint8_t GetConnectionId ();
uint8_t GetCurrentConnectionId(uint8_t *);
bool closeConnection(char id);
bool espDefaultInit( uartMap_t uartForDebug, uint32_t baudRate );
void auart232Callback(void);
void setConfigInit(void);
void setServerInit(void);

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif



#endif /* ESP8266_H_ */
