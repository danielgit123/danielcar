#define MOTO_DRIVER_I2C_ADDR 0x30
#define MOTO_DRIVER_PWM_FREQ 1000

#define MAX_MOTOR_SPEED 100



#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>


#include "WEMOS_Motor.h"

Motor M1(MOTO_DRIVER_I2C_ADDR,_MOTOR_A, MOTO_DRIVER_PWM_FREQ);//Motor A
Motor M2(MOTO_DRIVER_I2C_ADDR,_MOTOR_B, MOTO_DRIVER_PWM_FREQ);//Motor B


ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

#define CAR_STEP_MAX  3
static const char PROGMEM INDEX_HTML[] = R"rawliteral(

<!DOCTYPE html>
<html>
<head>
<meta name = "viewport" content = "width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0">
<title>ESP8266 WebSocket Car Remote</title>
<script>
var websock;
function start() {
  websock = new WebSocket('ws://' + window.location.hostname + ':81/');
  websock.onopen = function(evt) { console.log('websock open'); };
  websock.onclose = function(evt) { console.log('websock close'); };
  websock.onerror = function(evt) { console.log(evt); };
  websock.onmessage = function(evt) {
    console.log(evt);
  };
}
function updCar() {
 var cstr = document.getElementById('left').value + "_" + document.getElementById('right').value
 console.log('CAR: ' + cstr);
 websock.send(cstr);
}
</script>
<style >
input[type=range][orient=vertical]
{
    writing-mode: bt-lr; /* IE */
    -webkit-appearance: slider-vertical; /* WebKit */
    width: 8px;
    height: 75vh;
    padding: 0 15vh;
}
input[type=range]::-webkit-slider-thumb, ::-moz-range-thumb, ::-ms-thumb {
    -webkit-appearance: none;
    background-color: #666;
    width: 40px;
    height: 40px;
 }
</style>
</head>
<body onload="javascript:start();">
<div style="width: 100%; overflow: hidden;">
    <div style="width: 30%; float: left;">
<input oninput="updCar();" type="range" name="left" id="left" value="0" min="-3" max="3" orient="vertical">
	</div>
    <div align="right">
<input oninput="updCar();" type="range" name="right" id="right" value="0" min="-3" max="3" orient="vertical" >
	</div>
</div>

</body>
</html>

)rawliteral";


bool ld ;
bool rd ;
int lv ;
int rv ;
void setWheels(int lw, int rw)
{
	ld = lw>0?false:true ;
	rd = rw>0?true:false ;
	lv =  map(abs(lw) , 0 , CAR_STEP_MAX ,0,MAX_MOTOR_SPEED)  ;
	rv =  map(abs(rw) , 0 , CAR_STEP_MAX ,0,MAX_MOTOR_SPEED)  ;


	Serial.printf("Left  %u %u \n" ,ld,lv);
	Serial.printf("Right %u %u \n" ,rd,rv);
}


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
  Serial.printf("webSocketEvent(%d, %d, ...)\r\n", num, type);
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\r\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      }
      break;
    case WStype_TEXT:
		Serial.printf("[%u] get Text: %s\r\n", num, payload);

		String payloadString=(const char *)payload;
		byte separator=payloadString.indexOf('_');
		String ls=payloadString.substring(0,separator);
		String rs=payloadString.substring(separator+1);
		setWheels(ls.toInt(),rs.toInt()) ;

      break;

  }
}

void handleRoot()
{
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}


IPAddress local_IP(192,168,1,1);
IPAddress gateway(192,168,1,9);
IPAddress subnet(255,255,255,0);
void setup()
{
//   pinMode(LEDPIN, OUTPUT);
//   writeLED(false);

  Serial.begin(115200);

  //Serial.setDebugOutput(true);

  Serial.println();

  Serial.print("Setting soft-AP configuration ... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

  Serial.print("Setting soft-AP ... ");
  Serial.println(WiFi.softAP("daniel-car") ? "Ready" : "Failed!");

  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());

  Serial.println();
  Serial.println();
  Serial.println();


  Serial.print("Connect to http://");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);

  server.begin();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop()
{
  webSocket.loop();
  server.handleClient();



	if(lv == 0)
		M1.setmotor(_STOP);
	else
    	M1.setmotor( ld ? _CW : _CCW, lv);

	if(rv == 0)
		M2.setmotor(_STOP);
	else
		M2.setmotor( rd ? _CW : _CCW, rv);

}
