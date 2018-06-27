/* ##################################################################
**     Filename    : html.h
**     Author	   : German Iturria
**
** ###################################################################*/



// Pagina HTML de configuracion

#define MAX_LENGTH_INPUT 30
#define MAX_LENGTH_NUM 5

const char HttpWaitWebPageHeader []=
		"<html>"
		"<head>"
		"<title>Configurando!</title>"
		"</head>"
		"<body width=50% height=50%>"
		"<center>";

const char HttpWaitWebPageBody []=
		"<b><big><big>Aguarde, configurando!</big></big></b>"
		"</center>"
		"</body>";

const char HttpWebPageEnd []="</html>";

const char HttpIpWebPageBody []=
		"<b><big><big>IP asignada: %s </big></big></b>"
		"</center>"
		"</body>";

const char HttpConfigWebPageHeader []=

		"<html>"
		"<head>"
		"<title>Bienvenido al modulo de configuracion</title>"
		"</head>"
		"<body width=50% height=50%>"
		"<center>"
		"<form action=\"/action_page.php\">"
		  "<fieldset>"
		    "<legend>Configuracion WIFI:</legend>";

const char HttpConfigErrorWebPageHeader []=

		"<html>"
		"<head>"
		"<title>Bienvenido al modulo de configuracion</title>"
		"</head>"
		"<body width=50% height=50%>"
		"<center>"
		"<form action=\"/action_page.php\">"
		  "<fieldset>"
		    "<legend><b><big>Configuracion WIFI-Error en parametros ingresados:</b></big></legend>";

const char HttpConfigWebPageBody []=
		    "SSID:<br>"
		    "<input type=\"text\" name=\"SSID\" value=\"\" maxlength=\"MAX_LENGTH_INPUT\"><br>"
		    "Password:<br>"
		    "<input type=\"text\" name=\"PASS\" value=\"\" maxlength=\"MAX_LENGTH_INPUT\"><br><br>"
//			"IP Estatica:<br>"
//		    "<input type=\"text\" name=\"StaticIP\" value=\"\" maxlength=\"MAX_LENGTH_INPUT\"><br><br>"
//		    "<input type=\"radio\" name=\"DinamicIP\" value=\"ON\" unchecked>IP Dinamica<br>"
			"<input type=\"submit\" value=\"Enviar\">"
		"</fieldset>"
		"</form>"
		"</center>"
		"</body>";


// Pagina de Estado HTML

const char HttpWebPageHeader [] =
		"<html>"
		"<head>"
		"<title>Bienvenido al control de invernadero</title>"
		"</head>"
		"<body width=50% height=50% bgcolor=\"008080\">"
				"<center>"
				"<form action=\"/action_page.php\">"
				  "<fieldset>";

const char HttpWebPageBody [] =

		"<b>Temperatura Ambiente: ~~~~ C</b><br>"
		"<b>Humedad del Suelo: ~~~~ %</b><br>"
		"<b>Humedad del Ambiente: ~~~~ %</b><br>"
		"<br><br><br><br>"
	    "<b><big>configuracion de estado:</b></big>"
		"<br><br>"
	    "<b>Temperatura:<br>"
	    "<input type=\"text\" name=\"TEMP\" value=\" ~~~~  \" maxlength=\"MAX_LENGTH_NUM\"><br><br>"
	    "<b>Humedad del Ambiente:</b><br>"
	    "<input type=\"text\" name=\"HUMEDADAMB\" value=\" ~~~~  \" maxlength=\"MAX_LENGTH_NUM\"><br>"
		"<input type=\"submit\" value=\"Enviar\">"
	    "</fieldset>"
		"</form>"
		"</center>"
		"</body>"
		"<br><br><br><br></center>"
		"</body>";

/*
 * Fin de la pagina web. Este string debe ser enviado pasandolo como
 * argumento en la funcion esp8266WriteHttpServer().
 * Este texto en HTML puede ser cambiado para mostrar otro contenido en la web.
 */


/*
 * Cuerpo HTML, en este array se editara la informacion variable a mostrar
 * en la web. Un detalle interesante a notar es que este array debe comenzar
 * con BEGIN_USER_LINE, luego el usuario debe meter su informacion, y
 * terminar con END_USER_LINE. De esta manera el texto enviado tendra formato
 * HTML.
 */



