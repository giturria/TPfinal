/* ##################################################################
**     Filename    : application.c
**     Author	   : German Iturria
**
** ###################################################################*/

#include "application.h"
#include "esp8266.h"
#include "html.h"


// Baudrate por defecto de debug
#define BAUD_RATE_ESP 	115200
//#define BAUD_R_DEB 115200
//#define UART_USB		UART_USB
#define WIFI_MODULE		UART_232
#define WIFI_MAX_DELAY  6000
#define MAX_HTTP_WEB_LENGHT 1024
#define MAXBUFF 30

#define WIFI_DEFAULT_NAME 	"Invernadero"
#define WIFI_DEFAULT_PASS 	""
#define WIFI_CH				"1"
#define WIFI_ENC 			"0"


static delay_t wifiDelay;
static delay_t ConfigDelay;
typedef enum{CONFIG_INIT, CONFIG_WEB, ESP_INIT, OK}configState_t;
static configState_t configState=CONFIG_INIT;



static bool sendWebPage(uint8_t ch_id, char* HttpWebPage, uartMap_t module) {

  bool error = false;
  /* construct web page content */

  			// Configura un delay para salir de la configuracion en caso de error.
  			delayConfig(&wifiDelay, WIFI_MAX_DELAY);

  			// Mientras no termine el envio o mientras no pase el tiempo maximo, ejecuta el envio.
  			while (!WriteHttpServer(HttpWebPage) && !error){
  				if (delayRead(&wifiDelay)){
  					error = true;
  				}
			}

  			// Avisa al usuario como fue el envio
  			if (!error){
  				stdioPrintf(UART_USB, "\n\rPeticion respondida al cliente HTTP %d.", GetConnectionId());
  			} else {
  				stdioPrintf(UART_USB, "\n\rPeticion no respondida al cliente HTTP %d.", GetConnectionId());
  			}


  return !error;
}


void WebProcess(uint8_t temperatura, uint8_t humedadSuelo, uint8_t humedadAmb, uint8_t* temperaturaSet, uint8_t* humedadSet) {

  uint16_t size=0;
  uint8_t ch_Id;
  uint8_t *pdataGet;
  uint8_t *datasize;
  bool error=false;
  bool res=false;
  uint8_t *p;
  uint32_t i,j;
  char buff[MAXBUFF];
  bool data=false;
  get_st get;
  char HttpWebPage[MAX_HTTP_WEB_LENGHT];
  uint8_t dato[5];

	dato[0]=temperatura;
	dato[1]=humedadSuelo;
	dato[2]=humedadAmb;
	dato[3]=*temperaturaSet;
	dato[4]=*humedadSet;


  if(DataGet(&pdataGet)){

  	  ch_Id=GetCurrentConnectionId(pdataGet);

  	  if( strstr(pdataGet, "GET /action_page.php?") !=NULL){
  		  /*valido datos recibidos de configuracion*/
  		  	  data=true;
  			  p=strstr(pdataGet, "HUMEDADAMB=");
    	  	  	  if(p!=0) {
  				  while(*p!='=')p++;
  				  i=0;
  				  p++;
  				  while(*p!='&' && *p!=' ' && i<MAXBUFF){
  					  buff[i]=*p;
  					  i++;
  					  p++;
  				  	  }
  				  buff[i]='\0';
  				  dato[4]=atoi(buff);
  				  if(dato[4]>100 || dato[4]<0)data=false;
    	  	  	  }

    		      p=strstr(pdataGet, "TEMP=");
    	  	  	 	  if(p!=0) {
    	  	  	  		  while(*p!='=')p++;
    					  i=0;
    					  p++;
    					  while(*p!='&' && *p!=' ' & i<MAXBUFF){
    						  buff[i]=*p;
    						  i++;
    						  p++;
    					  }
    					  buff[i]='\0';
    	  				  dato[3]=atoi(buff);
    	  				  if(dato[3]>100 || dato[3]<0)data=false;
    	  	  	  	  }


   /*Error de Parametros*/
  			if(data==false){

  				HttpWebPage[0]='\0';
  				strcat(HttpWebPage ,HttpWebPageHeader);
  				strcat(HttpWebPage ,"<legend><b><big>Estado actual de Invernadero - Error en la carga de datos!</b></big></legend>");
  				strcat(HttpWebPage ,HttpWebPageBody);
  				strcat(HttpWebPage ,HttpWebPageEnd);

 			   p=HttpWebPage;
 			   for (j=0; p=strstr(p, "~~~~"); j++)
 			   {
 				   itoa(dato[j], buff, 10);
 				   for(i=0; i<=4; p++){
 					   if(buff[i]!='\0' && (strlen(buff)>=i))*p=buff[i];
 					   else *p=' ';
 				   }
 				}

  				res = sendWebPage(ch_Id, HttpWebPage, WIFI_MODULE);
  				while(!closeConnection(ch_Id));
  				}

  /*Aviso ACTUALIZADO!*/
  			else{

  			*temperaturaSet=dato[3];
  			*humedadSet=dato[4];

  				HttpWebPage[0]='\0';
  				strcat(HttpWebPage ,HttpWebPageHeader);
  				strcat(HttpWebPage ,"<legend><b><big>Estado actual de Invernadero - Actualizado!</b></big></legend>");
  				strcat(HttpWebPage ,HttpWebPageBody);
  				strcat(HttpWebPage ,HttpWebPageEnd);

 			   p=HttpWebPage;
 			   for (j=0; p=strstr(p, "~~~~"); j++)
 			   {
 				   itoa(dato[j], buff, 10);
 				   for(i=0; i<=4; p++){
 					   if(buff[i]!='\0' && (strlen(buff)>=i))*p=buff[i];
 					   else *p=' ';
 				   }
 				}

  				res = sendWebPage(ch_Id, HttpWebPage, WIFI_MODULE);
  				while(!closeConnection(ch_Id));

  			}
  	  }
		else if( strstr(pdataGet, "GET / HTTP/") !=NULL) {
	          /*Si recibo solo peticion de GET*/
			HttpWebPage[0]='\0';
			strcat(HttpWebPage ,HttpWebPageHeader);
			strcat(HttpWebPage ,"<legend><b><big>Estado actual de Invernadero</b></big></legend>");
			strcat(HttpWebPage ,HttpWebPageBody);
			strcat(HttpWebPage ,HttpWebPageEnd);

			   p=HttpWebPage;
			   for (j=0; p=strstr(p, "~~~~"); j++)
			   {
				   itoa(dato[j], buff, 10);
				   for(i=0; i<=4; p++){
					   if(buff[i]!='\0' && (strlen(buff)>=i))*p=buff[i];
					   else *p=' ';
				   }
				}


			res = sendWebPage(ch_Id, HttpWebPage, WIFI_MODULE);
			while(!closeConnection(ch_Id));

	   }
  }
return data;
  return;
}

bool ConfigWebPage(){

	  bool error=false;
	  bool res=false;
	  uint8_t *pdataGet;
	  uint8_t *p;
	  uint8_t i;
	  uint8_t ch_Id;
	  char buff[MAXBUFF];
	  bool data=false;
	  get_st get;
	  char HttpWebPage[MAX_HTTP_WEB_LENGHT];


	  if(DataGet(&pdataGet)){

	  ch_Id=GetCurrentConnectionId(pdataGet);

	  if( strstr(pdataGet, "GET /action_page.php?") !=NULL){
		  /*valido datos recibidos de configuracion*/

			  p=strstr(pdataGet, "SSID=");
  	  	  	  if(p!=0) {
  	  	  		  data=true;
				  while(*p!='=')p++;
				  i=0;
				  p++;
				  while(*p!='&' && *p!=' ' && i<MAXBUFF){
					  buff[i]=*p;
					  i++;
					  p++;
				  	  }
				  buff[i]='\0';
				  SetWifiName(buff);
				  if(i==0)data=false;
  	  	  	  	  }

  				  p=strstr(pdataGet, "PASS=");
  	  	  	  	  if(p!=0) {
  	  	  	  		  while(*p!='=')p++;
  					  i=0;
  					  p++;
  					  while(*p!='&' && *p!=' ' & i<MAXBUFF){
  						  buff[i]=*p;
  						  i++;
  						  p++;
  					  }
  					  buff[i]='\0';
  					  SetWifiPass(buff);
   				//	  if(i==0)data=false; puede ser red sin seguridad
  	  	  	  	  }
  				  else data=false;



// 	  	  	  	  if(strstr(pdataGet, "DinamicIP=")) {
// 	  	  	  		  SetIpAddress("0.0.0.0");
// 	  	  	  	  }else{
// 	  				  p=strstr(pdataGet, "StaticIP=");
// 	  				  if(p!=0) {
// 	  					  while(*p!='=')p++;
// 	  					  i=0;
// 	  					  p++;
// 	  					  while(*p!='&' && *p!=' ' && i<MAXBUFF){
// 	  						  buff[i]=*p;
// 	  						  i++;
// 	  						  p++;
// 	  					  }
// 	  					  buff[i]='\0';
// 	  					  SetIpAddress(buff);
// 	  					  if(i==0)data=false;
// 	  	  	  	  	  }
// 	  				  else data=false;
// 	  	  	  	  }


 	  	  	  delayConfig(&wifiDelay, WIFI_MAX_DELAY);
/*Pagina de ERROR*/
			if(data==false){

				HttpWebPage[0]='\0';
				strcat(HttpWebPage ,HttpConfigErrorWebPageHeader);
				strcat(HttpWebPage ,HttpConfigWebPageBody);
				strcat(HttpWebPage ,HttpWebPageEnd);

				res = sendWebPage(ch_Id, HttpWebPage, WIFI_MODULE);
				while(!closeConnection(ch_Id));
				}
/*Aviso CONFIGURANDO!*/
			else{
				HttpWebPage[0]='\0';
				strcat(HttpWebPage ,HttpWaitWebPageHeader);
				strcat(HttpWebPage ,HttpWaitWebPageBody);
				strcat(HttpWebPage ,HttpWebPageEnd);

				res = sendWebPage(ch_Id, HttpWebPage, WIFI_MODULE);
				while(!closeConnection(ch_Id));

			}
	  }
			else if( strstr(pdataGet, "GET / HTTP/") !=NULL) {
		          /*Si recibo solo peticion de GET*/
				HttpWebPage[0]='\0';
				strcat(HttpWebPage ,HttpConfigWebPageHeader);
				strcat(HttpWebPage ,HttpConfigWebPageBody);
				strcat(HttpWebPage ,HttpWebPageEnd);

				res = sendWebPage(ch_Id, HttpWebPage, WIFI_MODULE);
				while(!closeConnection(ch_Id));

		   }
	  }
	return data;
}


bool espInit( uartMap_t uartForDebug, uint32_t baudRate ){

	bool configurado = false;
	bool tmo= false;
	uartConfig(uartForDebug, baudRate);

	setServerInit();
	// Configura un delay para salir de la configuracion en caso de error.
	delayConfig(&wifiDelay, 8000);

	// Mientras no termine la configuracion o mientras no pase el tiempo maximo, ejecuta la configuracion.
	// A la configuracion se le pasa nombre y contrasenia de RED
	// En caso de querer ver los mensajes que se envian/reciben desde el modulo
	// se debe tambien mandar la UART por donde van a salir esos datos.

	while (!tmo && !configurado){
		configurado=configHttpServer(GetWifiName(),GetWifiPass(),"","", uartForDebug, baudRate);
		if (delayRead(&wifiDelay)){
			tmo= true;
		}
	}
   return configurado;
}


bool espConfigInit( uartMap_t uartForDebug, uint32_t baudRate ){

	bool tmo;
	bool configurado = false;
	uartConfig(uartForDebug, baudRate);

	setConfigInit();
	// Configura un delay para salir de la configuracion en caso de error.
	delayConfig(&wifiDelay, WIFI_MAX_DELAY);

	// Mientras no termine la configuracion o mientras no pase el tiempo maximo, ejecuta la configuracion.
	// A la configuracion se le pasa nombre y contrasenia de RED
	// En caso de querer ver los mensajes que se envian/reciben desde el modulo
	// se debe tambien mandar la UART por donde van a salir esos datos.

	while (!tmo && !configurado){
		configurado=configHttpServer(WIFI_DEFAULT_NAME,WIFI_DEFAULT_PASS,WIFI_CH,WIFI_ENC, uartForDebug, baudRate);
		if (delayRead(&wifiDelay)){
			tmo= true;
		}
	}
   return configurado;
}



bool FSM_Configuracion(){

	switch(configState){

		case CONFIG_INIT:
			if(espConfigInit(UART_USB, 115200))
				configState=CONFIG_WEB;
			break;

		case CONFIG_WEB:
			if(ConfigWebPage(WIFI_MODULE)){
				configState=ESP_INIT;
				delayConfig(&ConfigDelay, 6000);
			}
			break;

		case ESP_INIT:
			if(espInit(UART_USB, 115200)){
				configState=OK;
			}
			else if(delayRead(&ConfigDelay)){
				stdioPrintf(UART_USB , "\nError de configuracion\n");
				configState=CONFIG_INIT;
			}
			break;

		case OK:
			break;

	}
	return(configState==OK);
}


void APP_Run(uint8_t temperatura, uint8_t humedadSuelo, uint8_t humedadAmb, uint8_t* temperaturaSet, uint8_t* humedadSet) {
  bool config=false;
  bool error=false;
  char HttpWebPage[MAX_HTTP_WEB_LENGHT];


  uartConfig(UART_USB, 115200);
  uartConfig(UART_232, 115200);


  while(!FSM_Configuracion());


 /*Aviso de configurado*/

  stdioPrintf(UART_USB , "\nLa Ip asignada es: %s\n", GetIpAddress());


  while(1){
	 WebProcess(temperatura, humedadSuelo, humedadAmb, temperaturaSet, humedadSet);
  }

}
