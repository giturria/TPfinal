/* ##################################################################
**     Filename    : esp8266.c
**     Author	   : German Iturria
**
** ###################################################################
*/

/*==================[inlcusiones]============================================*/

#include "esp8266.h"
#include <string.h>

/*==================[definiciones y macros]==================================*/


#define ESP8266_UART        UART_232
#define ESP8266_BAUD_RATE   115200
#define ESP8266_TRR         100  /*tiempo entre respuesta y recepcion*/
#define ESP8266_TMO         1	 /*espera para enviar dato luego de TimeOut*/
#define ESP8266_WAIT        5000 /*tiempo de configuracion*/
#define ESP8266_WIFIWAIT    6000 /*tiempo de configuracion WIFI*/
#define ESP8266_WAITCLOSE	10000

#define MAX_COMMAND_LENGHT  40
#define MAX_HTTP_WEB_LENGHT 1024




typedef enum Esp8266State{
   ESP_INIT,
   ESP_SEND_AT,
   ESP_WAIT_AT,
   ESP_SEND_RST,
   ESP_WAIT_RST,
   ESP_SEND_CWJAP_CONS,
   ESP_WAIT_CWJAP_CONS_1,
   ESP_WAIT_CWJAP_CONS_2,
   ESP_SEND_CWMODE_SET,
   ESP_WAIT_CWMODE_SET,
   ESP_SEND_CWJAP_SET,
   ESP_WAIT_CWJAP_SET_1,
   ESP_WAIT_CWJAP_SET_2,
   ESP_WAIT_CWJAP_SET_3,
   ESP_WAIT_CWJAP_SET_4,
   ESP_CONNECTED,
   ESP_SEND_START,
   ESP_WAIT_START_1,
   ESP_WAIT_START_2,
   ESP_SEND_SEND,
   ESP_WAIT_SEND,
   ESP_SEND_DATA,
   ESP_WAIT_DATA_1,
   ESP_WAIT_DATA_2,
   ESP_CIPMUX_SEND,
   ESP_WAIT_CIPMUX,
   ESP_CIPSERVERMAXCONN,
   ESP_WAIT_CIPSERVERMAXCONN,
   ESP_SEND_CIPSERVER0,
   ESP_WAIT_CIPSERVER0,
   ESP_SEND_CIPSERVER,
   ESP_WAIT_CIPSERVER,
   ESP_LISTENING,
   ESP_WAIT_GET,
   ESP_PROCESSING,
   ESP_SEND_CIPSTATUS,
   ESP_WAIT_CIPSTATUS_X,
   ESP_WAIT_GET_ID,
   ESP_WAIT_CIPSTATUS2,
   ESP_WAIT_GET_ID2,
   ESP_SEND_CIPSEND,
   ESP_WAIT_CIPSEND,
   ESP_SEND_HTTP,
   ESP_WAIT_HTTP,
   ESP_WAIT_CIPSTATUS_OK,
   ESP_WAIT_CIPSTATUS,
   ESP_SEND_CIPCLOSE,
   ESP_WAIT_CIPCLOSE,
   ESP_SEND_CIPCLOSE1,
   ESP_WAIT_CIPCLOSE1,
   ESP_SEND_CIPCLOSE2,
   ESP_WAIT_CIPCLOSE2,
   ESP_SEND_CIPCLOSE3,
   ESP_WAIT_CIPCLOSE3,
   ESP_SEND_CIFSR,
   ESP_WAIT_CIFSR,
   ESP_LOAD_IP,
   ESP_SEND_CWSAP_SET,
   ESP_WAIT_CWSAP,
   ESP_SEND_CWMODE_SET_CONFIG,
   ESP_WAIT_CWMODE_SET_CONFIG

} Esp8266Status_t;

/*==================[definiciones de datos internos]=========================*/


static bool ESP_WebServerIsOn = false;

typedef enum {NO_SET, CONFIG, SERVER} init_t;

static init_t init=NO_SET;


bool isServerOn(void) {
  return ESP_WebServerIsOn;
}


// Respuestas del ESP8266
static const char Response_OK[]        = "OK";
static const char Response_CWJAP_OK[]  = "+CWJAP:";
static const char Response_CWJAP_1[]   = "WIFI DISCONNECT";
static const char Response_CWJAP_2[]   = "WIFI CONNECTED";
static const char Response_CWJAP_3[]   = "WIFI GOT IP";
static const char Response_CWSAP_OK[]  = "+CWSAP:";
static const char Response_SEND_OK[]   = "SEND OK\r\n";
static const char Response_STATUS_X[]  = "STATUS:";
static const char Response_CIPSTATUS[] = "+CIPSTATUS:";
static const char Response_CIPCLOSE[]  = "CLOSED\r\n";
static const char Response_CIFSR[]     = "+CIFSR:STAIP,\"";
// Memoria asociada a las conexiones
static uint8_t      CurrentConnectionId= 0xFF;
static char         WifiName [30];
static char         WifiPass [30];
static char	        WifiIp   [20];
static char	        WifiChannel[2];
static char	        WifiEnc[2];
// Punteros a la pagina web a mostrar
static char *       PointerOfHttp;
// Variables utilizadas en la maquina de estados.
static const char * Esp8266ResponseToWait;
static delay_t      Esp8266Delay;
static uint8_t      Esp8266Status = ESP_INIT;
static uartMap_t	Esp8266DebugUart = UART_232;
static uint32_t	    Esp8266DebugBaudRate = 0;
static bool_t		parametersReceived = FALSE;

/*==================[definiciones de datos externos]=========================*/

/*==================[declaraciones de funciones internas]====================*/

static bool_t 	IsWaitedResponse            (void);
static void 	SetEsp8622Status            (uint8_t statusToSend);
static void     ExcecuteHttpServerFsm       (void);

/*==================[declaraciones de funciones externas]====================*/

/**
 * Funcion para obtener la direccion IP del modulo Esp8266
 * @return devuelve un puntero al arreglo local donde esta alojado el dato.
 */
char * GetIpAddress    (){
	return WifiIp;
}

char * SetIpAddress    (char *WifiIp_){
	return strcpy(WifiIp, WifiIp_);
}

/**
 * Funcion para obtener el nombre de la red del modulo Esp8266
 * @return devuelve un puntero al arreglo local donde esta alojado el dato.
 */
char * GetWifiName     (){
	return WifiName;
}

char * SetWifiName     (char* WifiName_){
	return strcpy(WifiName, WifiName_);
}

uint8_t GetCurrentConnectionId(uint8_t *pdata){

	  uint8_t*p;

	  p=strstr(pdata, "+IPD,");
	  if(p) {
		  while(*p!=',')p++;
		  p++;

		  CurrentConnectionId=*p;
	  }

	return CurrentConnectionId;
}


/**
 * Funcion para obtener la contrasenia de la red del modulo Esp8266
 * @return devuelve un puntero al arreglo local donde esta alojado el dato.
 */
char * GetWifiPass     (){
	return WifiPass;
}

char * SetWifiPass     (char *WifiPass_){
    return strcpy(WifiPass, WifiPass_);
}

/**
 * Devuelve al usuario el indice de la conexion establecida
 * @return un entero correspondiente a
 */
uint8_t GetConnectionId (){
	return CurrentConnectionId - '0';
}

/**
 * Configura la conexion para que el modulo Esp8266 sea un servidor HTTP.
 * Realiza llamadas no bloqueantes a la maquina de estados que maneja la conexion.
 * La variable parametersReceived sirve para cargar por unica vez los datos de la red
 * @param wifiName puntero con el nombre de la red Wifi
 * @param wifiPass puntero con la contrasenia de la red Wifi
 * @param debugUart es la uart por donde va a reportar los datos. Este parametro es opcional, poner 0 sino se necesita.
 * @param debugBaudRate es la velocidad de la uart de debug. Poner en 0 si no se necesita.
 * @return TRUE si se configuro correctamente como servidor HTTP, FALSE caso contrario.
 */

bool_t configHttpServer(char * wifiName, char * wifiPass,char * wifiChannel, char * wifiEnc, uartMap_t debugUart, uint32_t debugBaudRate){

	if (!parametersReceived){
		strcpy(WifiName, wifiName);
		strcpy(WifiPass, wifiPass);
		strcpy(WifiChannel, wifiChannel);
		strcpy(WifiEnc, wifiEnc);
		SetEsp8622Status(ESP_INIT);
		parametersReceived = TRUE;
		if (debugBaudRate > 0){
			Esp8266DebugUart     = debugUart;
			Esp8266DebugBaudRate = debugBaudRate;
		}
	}
	ExcecuteHttpServerFsm();

	if (Esp8266Status == ESP_LISTENING)	return true;
	else return false;
}

/**
 * Funcion para determinar si hay alguna peticion HTTP realizada desde una
 * pagina web, realizada por algun cliente.
 * Ademas realiza llamadas no bloqueantes a la maquina de estados del
 * modulo que administra las conexiones y peticiones de clientes.
 * @return TRUE si se recibio una peticion, FALSE caso contrario.
 */
bool_t ReadHttpServer  (){
	ExcecuteHttpServerFsm();
	return (Esp8266Status == ESP_SEND_CIPSEND);
}

/**
 * Funcion para enviar una pagina web actualizada en respuesta a la
 * peticion del cliente.
 * Corrobora quue haya una peticion real del usuario antes de enviar datos.
 * Si hay una peticion, carga en los punteros correspondientes toda la
 * pagina web que se le enviara al cliente.
 * Un detalle importante es que tanto el header como el end de la web son
 * constantes, ya que se supone que no cambian a lo largo del programa.
 * Lo que si cambiara es el body HTTP que es el que contiene la informacion
 * actualizada del estado del sistema.
 * @param webHttpHeader puntero al header http (debe ser parte de la aplicacion de usuario).
 * @param webHttpBody puntero al body http (debe ser parte de la aplicacion de usuario).
 * @param webHttpEnd puntero al end http (debe ser parte de la aplicacion de usuario).
 * @return TRUE si pudo mandar la web correctamente, FALSE caso contrario.
 */
bool_t WriteHttpServer (char * webHttp){
	//antes de enviar se asegura que haya datos pendientes para enviar
	if (Esp8266Status == ESP_SEND_CIPSEND){
		PointerOfHttp = webHttp;
	}
	ExcecuteHttpServerFsm();

	return (Esp8266Status == ESP_LISTENING);
}


bool closeConnection(char id){

	if(Esp8266Status == ESP_LISTENING){
		Esp8266Status = ESP_SEND_CIPCLOSE;
	}
	ExcecuteHttpServerFsm();

	return (Esp8266Status == ESP_LISTENING);
}



#define DATABUFFERSIZE      512
static uint8_t      dataGetbuff [DATABUFFERSIZE];
//static uint32_t	    dataGetsize=0;
//static uint8_t*	pbuffread;
static uint8_t*	pbuffwrite=dataGetbuff;

bool DataGet(uint8_t **databuffer){

	ExcecuteHttpServerFsm();
	if(Esp8266Status == ESP_PROCESSING){
			*databuffer=dataGetbuff;
			return true;
	}
	else return false;
}


bool_t esp8266Get (void){

	uint8_t ch;
	bool isGet;
	uint8_t ch_id;
	uint16_t size;
    char a;
	size=0;
	bool flagGet=true;

	while(uartReadByte(UART_232, pbuffwrite)){
			pbuffwrite++;
			if(pbuffwrite-dataGetbuff == DATABUFFERSIZE-1){
				pbuffwrite=dataGetbuff;
			}//agregar tiempo MAXIMO
			if(strstr(dataGetbuff, "GET /") && flagGet) {
				while(!uartRxReady(UART_232)){}
				flagGet=false;
		   }
	}
	*pbuffwrite='\0';

	if( strstr(dataGetbuff, "GET /") && strstr(dataGetbuff, "Host"))return (true);
	else return false;

}


void espcleanbuffer(void){

	uint8_t c;

	stdioPrintf(UART_USB, "Clean Buffer \r\n");
	pbuffwrite=dataGetbuff;
	*pbuffwrite='\0';
	while(uartReadByte(UART_232, &c)){
			uartWriteByte(UART_USB, c);
	}
	stdioPrintf(UART_USB, "****\r\n");
	return;
}



/*==================[definiciones de funciones internas]=====================*/

/**
 * Funcion principal del modulo Wifi Esp8266 para funcionar como servidor HTTP.
 * Desde aca se manejan los comandos a enviar, los tiempos a esperar y
 * las respuestas a recibir.
 * Automaticamente cambia de estados en funcion de los eventos que ocurran.
 */
static void    ExcecuteHttpServerFsm   (void) {
	uint16_t lenghtOfHttpLines;
	static uint8_t byteReceived, auxIndex;

	switch (Esp8266Status) {

	case ESP_INIT:
		uartConfig(ESP8266_UART, ESP8266_BAUD_RATE);
		if (Esp8266DebugBaudRate > 0){
			uartConfig(Esp8266DebugUart, Esp8266DebugBaudRate);
		}
		delayConfig(&Esp8266Delay, ESP8266_TRR);
		SetEsp8622Status(ESP_SEND_RST);
		break;

	case ESP_SEND_RST:
			if (delayRead(&Esp8266Delay)) {
				stdioPrintf(ESP8266_UART, "AT+RST\r\n");
				Esp8266ResponseToWait = Response_OK;
				delayConfig(&Esp8266Delay, ESP8266_TRR);
				SetEsp8622Status(ESP_WAIT_RST);
			}
		break;

	case ESP_WAIT_RST:
		if (IsWaitedResponse()) {
			delayConfig(&Esp8266Delay, ESP8266_TRR);
			SetEsp8622Status(ESP_SEND_AT);
		}
		//Si no recibe OK vuelve a enviar RST
		if (delayRead(&Esp8266Delay)) {
			delayConfig(&Esp8266Delay, ESP8266_TMO);
			SetEsp8622Status(ESP_SEND_RST);
		}
		break;

	case ESP_SEND_AT:
			if (delayRead(&Esp8266Delay)) {
				stdioPrintf(ESP8266_UART, "AT\r\n");
				Esp8266ResponseToWait = Response_OK;
				delayConfig(&Esp8266Delay, ESP8266_TRR);
				SetEsp8622Status(ESP_WAIT_AT);
			}
		break;

	case ESP_WAIT_AT:
		if (IsWaitedResponse()) {
			delayConfig(&Esp8266Delay, ESP8266_TRR);
/*configuro el servidor o configuro AP para configuracion*/
			if(init==SERVER) SetEsp8622Status(ESP_SEND_CWMODE_SET);
			else SetEsp8622Status(ESP_SEND_CWMODE_SET_CONFIG);
		}
		//Si no recibe OK vuelve a enviar AT
		if (delayRead(&Esp8266Delay)) {
			delayConfig(&Esp8266Delay, ESP8266_TMO);
			SetEsp8622Status(ESP_SEND_AT);
		}
		break;

//	case ESP_SEND_CWJAP_CONS:
//		if (delayRead(&Esp8266Delay)) {
//			stdioPrintf(ESP8266_UART, "AT+CWJAP?\r\n");
//			Esp8266ResponseToWait = Response_CWJAP_OK;
//			delayConfig(&Esp8266Delay, ESP8266_TMO);
//			SetEsp8622Status(ESP_WAIT_CWJAP_CONS_1);
//		}
//		break;
//
//	case ESP_WAIT_CWJAP_CONS_1:
//		if (IsWaitedResponse()) {
//			Esp8266ResponseToWait = Response_OK;
//			SetEsp8622Status(ESP_WAIT_CWJAP_CONS_2);
//		}
//		if (delayRead(&Esp8266Delay)) {
//			delayConfig(&Esp8266Delay, ESP8266_PAUSE);
//			SetEsp8622Status(ESP_SEND_CWMODE_SET);
//		}
//		break;
//
//	case ESP_WAIT_CWJAP_CONS_2:
//		if (IsWaitedResponse()){
//			delayConfig(&Esp8266Delay, ESP8266_PAUSE);
//			SetEsp8622Status(ESP_CIPMUX_SEND);
//		}
//		if (delayRead(&Esp8266Delay)) {
//			delayConfig(&Esp8266Delay, ESP8266_PAUSE);
//			SetEsp8622Status(ESP_SEND_AT);
//		}
//		break;
/*************************************************************/

	case ESP_SEND_CWMODE_SET_CONFIG:
		if (delayRead(&Esp8266Delay)) {
			stdioPrintf(ESP8266_UART, "AT+CWMODE=3\r\n");
			Esp8266ResponseToWait = Response_OK;
			delayConfig(&Esp8266Delay, ESP8266_WAIT);
			SetEsp8622Status(ESP_WAIT_CWMODE_SET_CONFIG);
		}
		break;

	case ESP_WAIT_CWMODE_SET_CONFIG:
		if (IsWaitedResponse()) {
			delayConfig(&Esp8266Delay, ESP8266_TRR);
			SetEsp8622Status(ESP_SEND_CWSAP_SET);
		}
		if (delayRead(&Esp8266Delay)) {
			delayConfig(&Esp8266Delay, ESP8266_TMO);
			SetEsp8622Status(ESP_SEND_AT);
		}
		break;

	case ESP_SEND_CWSAP_SET:
		if (delayRead(&Esp8266Delay)) {
			stdioPrintf(ESP8266_UART, "AT+CWSAP=\"%s\",\"%s\",%s,%s\r\n", WifiName, WifiPass, WifiChannel, WifiEnc);
			Esp8266ResponseToWait = Response_OK;
			delayConfig(&Esp8266Delay, ESP8266_WIFIWAIT);
			SetEsp8622Status(ESP_WAIT_CWSAP);
		}
		break;

	case ESP_WAIT_CWSAP:
		if (IsWaitedResponse()) {
			delayConfig(&Esp8266Delay, ESP8266_TRR);
			SetEsp8622Status(ESP_CIPMUX_SEND);
		}
		if (delayRead(&Esp8266Delay)) {
			delayConfig(&Esp8266Delay, ESP8266_TMO);
			SetEsp8622Status(ESP_SEND_CWSAP_SET);
		}
		break;

/***************************************/

	case ESP_SEND_CWMODE_SET:
		if (delayRead(&Esp8266Delay)) {
			stdioPrintf(ESP8266_UART, "AT+CWMODE=3\r\n");
			Esp8266ResponseToWait = Response_OK;
			delayConfig(&Esp8266Delay, ESP8266_WAIT);
			SetEsp8622Status(ESP_WAIT_CWMODE_SET);
		}
		break;

	case ESP_WAIT_CWMODE_SET:
		if (IsWaitedResponse()) {
			delayConfig(&Esp8266Delay, ESP8266_TRR);
			SetEsp8622Status(ESP_SEND_CWJAP_SET);
		}
		if (delayRead(&Esp8266Delay)) {
			delayConfig(&Esp8266Delay, ESP8266_TMO);
			SetEsp8622Status(ESP_SEND_AT);
		}
		break;

	case ESP_SEND_CWJAP_SET:
		if (delayRead(&Esp8266Delay)) {
			stdioPrintf(ESP8266_UART, "AT+CWJAP=\"%s\",\"%s\"\r\n", WifiName, WifiPass);
			Esp8266ResponseToWait = Response_CWJAP_2;
			delayConfig(&Esp8266Delay, ESP8266_WIFIWAIT);
			SetEsp8622Status(ESP_WAIT_CWJAP_SET_2);
		}
		break;

//	case ESP_WAIT_CWJAP_SET_1:
//		if (IsWaitedResponse()) {
//			delayConfig(&Esp8266Delay, ESP8266_WIFIWAIT);
//			Esp8266ResponseToWait = Response_CWJAP_2;
//			SetEsp8622Status(ESP_WAIT_CWJAP_SET_2);
//		}
//		if (delayRead(&Esp8266Delay)) {
//			delayConfig(&Esp8266Delay, ESP8266_TMO);
//			SetEsp8622Status(ESP_SEND_AT);
//		}
//		break;

	case ESP_WAIT_CWJAP_SET_2:
		if (IsWaitedResponse()) {
			delayConfig(&Esp8266Delay, ESP8266_WIFIWAIT);
			Esp8266ResponseToWait = Response_CWJAP_3;
			SetEsp8622Status(ESP_WAIT_CWJAP_SET_3);
		}
		if (delayRead(&Esp8266Delay)) {
			delayConfig(&Esp8266Delay, ESP8266_TMO);
			SetEsp8622Status(ESP_SEND_AT);
		}
		break;

	case ESP_WAIT_CWJAP_SET_3:
		if (IsWaitedResponse()) {
			delayConfig(&Esp8266Delay, ESP8266_WIFIWAIT);
			Esp8266ResponseToWait = Response_OK;
			SetEsp8622Status(ESP_WAIT_CWJAP_SET_4);
		}
		if (delayRead(&Esp8266Delay)) {
			delayConfig(&Esp8266Delay, ESP8266_TMO);
			SetEsp8622Status(ESP_SEND_AT);
		}
		break;

	case ESP_WAIT_CWJAP_SET_4:
		if (IsWaitedResponse())
			SetEsp8622Status(ESP_CIPMUX_SEND);
		if (delayRead(&Esp8266Delay)) {
			delayConfig(&Esp8266Delay, ESP8266_WAIT);
			SetEsp8622Status(ESP_SEND_AT);
		}
		break;

	case ESP_CIPMUX_SEND:
		if (!delayRead(&Esp8266Delay)) {
			stdioPrintf(ESP8266_UART, "AT+CIPMUX=1\r\n");
			Esp8266ResponseToWait = Response_OK;
			delayConfig(&Esp8266Delay, ESP8266_WAIT);
			SetEsp8622Status(ESP_WAIT_CIPMUX);
			auxIndex=0;
		}
		break;

	case ESP_WAIT_CIPMUX:
		if (IsWaitedResponse()) {
			delayConfig(&Esp8266Delay, ESP8266_TRR);
			SetEsp8622Status(ESP_SEND_CIPSERVER);
		}
		if (delayRead(&Esp8266Delay)) {
			delayConfig(&Esp8266Delay, ESP8266_TRR);
			// cierra todas las posibles conexioNes
			stdioPrintf(ESP8266_UART, "AT+CIPCLOSE=%d\r\n", auxIndex);
			if (++auxIndex >= 4){
				SetEsp8622Status(ESP_CIPMUX_SEND);
			}
		}
		break;

	case ESP_SEND_CIPSERVER:
		if (delayRead(&Esp8266Delay)) {
			stdioPrintf(ESP8266_UART, "AT+CIPSERVER=1,80\r\n");
			Esp8266ResponseToWait = Response_OK;
			delayConfig(&Esp8266Delay, ESP8266_WAIT);
			SetEsp8622Status(ESP_WAIT_CIPSERVER);
		}
		break;

	case ESP_WAIT_CIPSERVER:
		if (IsWaitedResponse()) {
			delayConfig(&Esp8266Delay, ESP8266_TRR);
			SetEsp8622Status(ESP_SEND_CIFSR);
		}
		if (delayRead(&Esp8266Delay)) {
			delayConfig(&Esp8266Delay, ESP8266_TMO);
			SetEsp8622Status(ESP_SEND_AT);
		}
		break;

	case ESP_SEND_CIFSR:
		if (delayRead(&Esp8266Delay)) {
			stdioPrintf(ESP8266_UART, "AT+CIFSR\r\n");
			Esp8266ResponseToWait = Response_CIFSR;
			delayConfig(&Esp8266Delay, ESP8266_WAIT);
			SetEsp8622Status(ESP_WAIT_CIFSR);
		}
		break;

	case ESP_WAIT_CIFSR:
		if (IsWaitedResponse()) {
			SetEsp8622Status(ESP_LOAD_IP);
			auxIndex=0;
		}
		if (delayRead(&Esp8266Delay)) {
			delayConfig(&Esp8266Delay, ESP8266_TRR);
			SetEsp8622Status(ESP_SEND_CIFSR);
		}
		break;

		//Recibe byte a byte la direccion IP y la almacena en WifiIp
	case ESP_LOAD_IP:
		if (uartReadByte(ESP8266_UART, &byteReceived)) {
			if (byteReceived != '"') {
				WifiIp [auxIndex] = byteReceived;
				stdioPrintf(Esp8266DebugUart, "%c", byteReceived);
				auxIndex++;
			} else {
				WifiIp [auxIndex] = '\0';
				SetEsp8622Status(ESP_LISTENING);
			}
		}
		break;


	case ESP_LISTENING:
		 if(esp8266Get ()){
			Esp8266Status = ESP_PROCESSING;
		 }
		break;

	case ESP_PROCESSING:
			SetEsp8622Status(ESP_SEND_CIPSTATUS);

		break;


		// En este estado el modulo ya esta configurado como servidor HTTP
		// entonces cada vez que pasa el delay ESP8266_PAUSE mediante
		// el compando CIP_STATUS le pregunta al moduloWifi si hay una nueva peticion
	case ESP_SEND_CIPSTATUS:
		if (delayRead(&Esp8266Delay)) {
			stdioPrintf(ESP8266_UART, "AT+CIPSTATUS\r\n");
			Esp8266ResponseToWait = Response_STATUS_X;
			delayConfig(&Esp8266Delay, ESP8266_WAIT);
			SetEsp8622Status(ESP_WAIT_CIPSTATUS_X);
		}
		break;

	case ESP_WAIT_CIPSTATUS_X:
		if (IsWaitedResponse()) {
			delayConfig(&Esp8266Delay, ESP8266_TRR);
			Esp8266ResponseToWait = Response_CIPSTATUS;
			SetEsp8622Status(ESP_WAIT_CIPSTATUS);
		}
		if (delayRead(&Esp8266Delay)) {
			delayConfig(&Esp8266Delay, ESP8266_TMO);
			SetEsp8622Status(ESP_LISTENING);
			espcleanbuffer();
		}
		break;

	case ESP_WAIT_CIPSTATUS:
		if (IsWaitedResponse()){
			SetEsp8622Status(ESP_WAIT_GET_ID);
		}
		if (delayRead(&Esp8266Delay)) {
			delayConfig(&Esp8266Delay, ESP8266_TMO);
			SetEsp8622Status(ESP_LISTENING);
			espcleanbuffer();
		}
		break;

	case ESP_WAIT_GET_ID:
		if (uartReadByte(ESP8266_UART, &byteReceived)) {
//			CurrentConnectionId = byteReceived;
			Esp8266ResponseToWait = Response_OK;
			SetEsp8622Status(ESP_WAIT_CIPSTATUS_OK);
			auxIndex = 0;
		}
		break;

	case ESP_WAIT_CIPSTATUS_OK:
		if (IsWaitedResponse()){
			SetEsp8622Status(ESP_SEND_CIPSEND);

		}
		if (delayRead(&Esp8266Delay)) {
			delayConfig(&Esp8266Delay, ESP8266_TMO);
			SetEsp8622Status(ESP_LISTENING);
			espcleanbuffer();
		}
		break;

		//En este estado le dice al modulo cuantos bytes va a mandar
		// El comando esta compuesto por el comando CIPSEND, mas el ID
		// de la conexion abierta, mas la cantidad en ASCII de los bytes
		// que tiene la pagina web (o al menos los bytes de la primer linea).
	case ESP_SEND_CIPSEND:

		lenghtOfHttpLines = strlen(PointerOfHttp);
		// Si se pasa del maximo largo permitido lo avisa en la web
		if (lenghtOfHttpLines >= MAX_HTTP_WEB_LENGHT){
			stdioSprintf(PointerOfHttp, "ERROR: La longitud de datos HTTP supera el maximo permitido de %d bytes.", MAX_HTTP_WEB_LENGHT);
		}
		stdioPrintf(ESP8266_UART, "AT+CIPSEND=%c,%d\r\n", CurrentConnectionId, lenghtOfHttpLines);
		SetEsp8622Status(ESP_WAIT_CIPSEND);
		Esp8266ResponseToWait = Response_OK;
		break;

	case ESP_WAIT_CIPSEND:
		if (IsWaitedResponse()) {
			delayConfig(&Esp8266Delay, ESP8266_WAIT);
			SetEsp8622Status(ESP_SEND_HTTP);
		}
		if (delayRead(&Esp8266Delay)) {
			delayConfig(&Esp8266Delay, ESP8266_TMO);
			espcleanbuffer();
			SetEsp8622Status(ESP_LISTENING);
		}
		break;

	case ESP_SEND_HTTP:
		stdioPrintf(ESP8266_UART, "%s", PointerOfHttp);
		SetEsp8622Status(ESP_WAIT_HTTP);
		Esp8266ResponseToWait = Response_SEND_OK;
		break;

	case ESP_WAIT_HTTP:
		if (IsWaitedResponse()) {
			delayConfig(&Esp8266Delay, ESP8266_TMO);
			SetEsp8622Status(ESP_LISTENING);
			espcleanbuffer();
		}
		if (delayRead(&Esp8266Delay)) {
			delayConfig(&Esp8266Delay, ESP8266_TMO);
			SetEsp8622Status(ESP_LISTENING);
			espcleanbuffer();
		}
		break;

	case ESP_SEND_CIPCLOSE:
		//if (delayRead(&Esp8266Delay)) {
			stdioPrintf(ESP8266_UART, "AT+CIPCLOSE=%c\r\n", CurrentConnectionId);
			Esp8266ResponseToWait  = Response_CIPCLOSE;
			delayConfig(&Esp8266Delay, ESP8266_WAIT);
			SetEsp8622Status(ESP_WAIT_CIPCLOSE);
	//	}
		break;

	case ESP_WAIT_CIPCLOSE:
		if (IsWaitedResponse()) {
			delayConfig(&Esp8266Delay, ESP8266_WAIT);
			SetEsp8622Status(ESP_LISTENING);
			espcleanbuffer();
		}
		if (delayRead(&Esp8266Delay)) {
			delayConfig(&Esp8266Delay, ESP8266_TMO);
			SetEsp8622Status(ESP_LISTENING);
			espcleanbuffer();
		}
		break;

	}
}

/**
 * Determina si la respuesta que envio el modulo Esp8266 al
 * sistema embebido sea correcta.
 * Dentro de esta funcion va leyendo datos de la UART asociada
 * al modulo Esp8266 hasta encontrar el patron de datos necesario.
 * @return TRUE si es la respuesta esperada, FALSE caso contrario.
 */
static bool_t  IsWaitedResponse        (void) {
static uint8_t index = 0;
uint8_t byteReceived;
bool_t moduleResponse = FALSE;

	if (uartReadByte(ESP8266_UART, &byteReceived)) {
		if (Esp8266DebugBaudRate > 0){
			stdioPrintf(Esp8266DebugUart, "%c", byteReceived);
		}
		if (byteReceived == Esp8266ResponseToWait[index]) {
			index++;
			if (Esp8266ResponseToWait[index] == '\0') {
				index = 0;
				moduleResponse = TRUE;
				if (Esp8266DebugBaudRate > 0){
					stdioPrintf(Esp8266DebugUart, "\n\r", byteReceived);
				}
			}
		} else {
			index = 0;
		}
	}
	return moduleResponse;
}

/**
 * Setea el estado global del modulo Esp8266.
 * @param status Estado a setear.
 */
static void    SetEsp8622Status        (Esp8266Status_t status) {
	Esp8266Status = status;

}


void setConfigInit(void){

	init=CONFIG;
	parametersReceived = false;
	Esp8266Status = ESP_INIT;
}

void setServerInit(void){

	init=SERVER;
	Esp8266Status = ESP_INIT;
}




/*==================[fin del archivo]========================================*/


