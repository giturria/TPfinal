/* ##################################################################
**     Filename    : main.c
**     Author	   : German Iturria
**
** ###################################################################*/

/* MODULE main */


/* Including needed modules to compile this module/procedure */

#include "esp8266.h"
#include "application.h"
#include "sapi.h"

int main(void)

{
  /* Write your local variable definition here */

	uint8_t temperatura=10, humedadSuelo=90, humedadAmb=90, temperaturaSet=88, humedadSet=88;


boardConfig();

  /* Inicializar las UART a 115200 baudios */


  APP_Run(temperatura, humedadSuelo, humedadAmb, &temperaturaSet, &humedadSet);
  for(;;){}

}

/* END main */
