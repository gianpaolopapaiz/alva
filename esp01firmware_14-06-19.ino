
//Programa: ESP-01 Firmware V1.0
//Autor: PROTTO
//Atenção: Esse código utiliza bibliotecas adicionais ou partes delas, que não foram desenvolvidas pela PROTTO.
//Objetivo: Código de funcionamento do módulo wifi (ESP-01)da luminária ALVA - Versão 1.0
//Desenvolvido para ESP-01

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------

//ÁREA PRÉ SET UP
//Área responsável por adicionar bibliotecase e definir variáveis globais 

//Bibliotecas Adicionais
#include <ESP8266mDNS.h>  
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 
#include <ESP8266WiFi.h>
#include <elapsedMillis.h>

//Variáveis para armazenar o endereço IP
String ip = "";
int lenip = 0;

// Cria uma instancia do servidor e especifica uma porta
WiFiServer server(80);
elapsedMillis timeout;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------

//ÁREA DE SET UP
//Área responsável pelo preparo (set up) do projeto.

void setup() {

delay(5000);

Serial.begin(9600);
Serial.setTimeout(1000);

WiFiManager wifiManager;
wifiManager.autoConnect("ALVA"); //Nome da rede a ser criada pelo módulo Wifi

while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(".");
  }
Serial.println("");
Serial.println("WiFi Conectado");

// Incia o Servidor
server.begin();
Serial.println("Servidor Iniciado");

// Endereço IP
timeout = 0;
while (!Serial.available() && timeout < 3000) {
 ip = WiFi.localIP().toString();
 lenip = ip.length();
 Serial.print("IP:");
 if (lenip < 10) {
  Serial.print("0");
  Serial.print(lenip);
  } 
   else {
   Serial.print(lenip);
   }
Serial.println(ip);
delay(500);
}
delay(2000);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------

//ÁREA DO LOOP
//Área responsável pelo processamento continuo do ESP-01.

void loop() {

//Checa se existe informação na Serial
String txtserial = "";
timeout = 0;
while (!Serial.available() && timeout < 500) {
 delay(1);
 }
timeout = 0;
int repete = 0;
while (txtserial.indexOf("###") == -1 && timeout < 500) {
 txtserial = Serial.readStringUntil('\r');
 //reset wifi via botao
 if (txtserial.indexOf("rst1") != -1) {
  repete = 0;
  while (repete < 10) {
   Serial.print("IP:13NAO Conectado");
   delay(200);
   repete = repete + 1;
   }
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  delay(5000);
  ESP.restart();
  }
  
 //Mostra o endereço IP
 if (txtserial.indexOf("ipn1") != -1) {
  repete = 0;
   while (repete < 10) {
    Serial.print("IP:");
    if (lenip < 10) {
     Serial.print("0");
     Serial.print(lenip);
     }
     else {
      Serial.print(lenip);
      }
    Serial.println(ip);
    repete = repete + 1;
    delay(200);
    }
   }
 }

//Checa se o cliente esta conectado
WiFiClient client = server.available();
if (!client) {
 return;
 }

//Espera o cliente mandar alguma informação
//Serial.println("Novo Cliente ...");
while (!client.available()) {
 delay(1);
 }

// Recebe a primeira linha da requisição
String request = client.readStringUntil('\r');
Serial.println(request); 

//Reset do wifi via app
if (request.indexOf("rst") != -1) {
 Serial.print("IP:13NAO Conectado");
 delay(500);
 WiFiManager wifiManager;
 wifiManager.resetSettings();
 delay(5000);
 ESP.restart();
 }
Serial.flush();

// Prepara a resposta
client.println("HTTP/1.1 200 OK");
client.println("Content-Type: text/html");
client.println("");
String response = "";
timeout = 0;
while (!Serial.available() && timeout < 500) {
 delay(1);
 }
timeout = 0;
while (response.indexOf("***") == -1 && timeout < 500) {
 response = Serial.readStringUntil('\r');
 client.print(response);
 }
client.flush();
delay(1);
}
