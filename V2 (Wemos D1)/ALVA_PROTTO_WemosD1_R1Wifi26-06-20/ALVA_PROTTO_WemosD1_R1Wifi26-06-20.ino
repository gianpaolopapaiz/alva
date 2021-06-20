 
//Programa: ALVA V2.0 (R1) 
//Autor: PROTTO
//Atenção: Esse código utiliza bibliotecas adicionais ou partes delas, que não foram desenvolvidas pela PROTTO.
//Objetivo: Código de funcionamento da luminária ALVA - Versão 2.0
//Desenvolvido para Wemos D1 R1 (ESP8266)
//As portas definidas para os componentes seguem o nome do GPIO -> Ex:GPIO 0 = porta D8

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------

//ÁREA PRÉ SET UP
//Área responsável por adicionar bibliotecas, definir variáveis globais e definir as portas nas quais os módulos
//eletrônicos estão conectados.

//Wifi -> definições relacionadas ao funcionamento do Wifi -> Um ESP-01 é utilizado no projeto
#include <ESP8266mDNS.h>  
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 
#include <ESP8266WiFi.h>
#include <elapsedMillis.h>
String ip;
int lenip = 0;
int menuwifi = 0;
int menuwifi2 = 0;
int conectado = 0;
String request = "";
// Cria uma instancia do servidor e especifica uma porta
WiFiServer server(80);
elapsedMillis timeout;


//Sensor de Ditância -> definições relacionadas ao funcionamento do Sensor de Distância (HC-SR04)
#include <NewPingESP8266.h> //incluindo biblioteca adicional
#define TRIGGER_PIN 2 //Define o pino trigger do sensor na porta D4
#define ECHO_PIN 15 //Define o pino Echo do sensor na porta D8
#define MAX_DISTANCE 10 // Máxima distância (em cm) em que o sensor irá atuar. Máximo permitido esta entre 400 a 500 cm
NewPingESP8266 sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); //Define o sensor nos pinos D4 e D8, com a distância máxima
int mediasonaratual = 0; 
int mediasonaranterior = 0; 
int mediasonaranterior2 = 0; 

// Relógio -> definições relacionadas ao módulo Relógio (DS3231)
#include <Wire.h> //incluindo biblioteca adicional
#include "RTClib.h"
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Dom", "Seg", "Ter", "Qua", "Qui", "Sex", "Sab"};

//Variáveis para configuração de data e hora
int minuto = 1;
int hora = 1;
int diasemana = 1;
int diames = 1;
int mes = 1;
int ano = 2020;

//Variáveis para configuração do despertador
int despminuto = 0;
int desphora = 0;
int estadodesp = 0;

//Variáveis para configuração do desligamento
int deslminuto = 0;
int deslhora = 0;
int estadodesl = 0;

//Variáveis para o menu de configuração
int menu = 0;
int estadodespdesl = 0;

//Variável para os Menus do Diplay
int estadomenu = 0;

//Display LCD
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C  lcd(0x27,2,1,0,4,5,6,7,3,POSITIVE); // Define o Display LCD
int estadolcd = 1;

//Sensor de Temperatura e Umidade (DHT11)
#include "DHT.h"
#define DHTPIN 0 // Define o sensor no pino D8
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);
float umid = 0.0;
float temp = 0.0;

//Módulo relé
#define rele 13 // Define o módulo relé no pino D11
int estadorele = HIGH; 

//Botão set
#define botaoset 14  // Define o botão Set no pino D5
int anteriorset = 0;
int estadobotaoset = 0;
int anteriorestadobotaoset = 0;

//Botão mais
#define botaomais 12  // Define o botão Mais no pino D6
int anteriormais = 0;
int estadobotaomais = 0;
int anteriorestadobotaomais = 0;

//Variáveis para a Tela Customizada
 //Linha 1
 String linha1 = "";
 //Linha 2
 String linha2 = "";

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------

//ÁREA DE SET UP
//Área responsável pelo preparo (set up) dos componentes do projeto.

void setup() {

delay(2000);
Serial.begin(115200);

//Display LCD
lcd.setBacklightPin(3, POSITIVE);
lcd.setBacklight(HIGH); //configura a luz de fundo (backlight)do display LCD como ligado
lcd.begin(16,2); //inicializa um display LCD de tipo 16x2
lcd.clear(); //limpa a tela do display LCD

//Tela de introdução ("ALVA by Protto V1.0")
lcd.print(" A");
delay(100);
lcd.print("L");
delay(100);
lcd.print("V");
delay(100);
lcd.print("A");
delay(100);
lcd.print(" b");
delay(100);
lcd.print("y");
delay(100);
lcd.print(" P");
delay(100);
lcd.print("r");
delay(100);
lcd.print("o");
delay(100);
lcd.print("t");
delay(100);
lcd.print("t");
delay(100);
lcd.print("o");
delay(100);
lcd.setCursor(6, 1);
lcd.print("V");
delay(100);
lcd.print("2");
delay(100);
lcd.print(".");
delay(100);
lcd.print("0");
delay(1000);
lcd.clear();

//Sensor de Temperatura e Umidade (DHT11)
dht.begin(); //inicializa o sensor de temperatura e umidade (DHT11)

//Módulo Relógio (DS3231)
Wire.begin(); //inicializa o módulo relógio (DS3231)
rtc.begin();

//Módulo Relé
pinMode(rele, OUTPUT); //define o relé como um output
digitalWrite(rele, LOW); //o relé é definido para ficar apagado no inicio
estadorele = LOW;

//Botão Set
pinMode(botaoset, INPUT); //define o botao "set" como um input

//Botao Mais
pinMode(botaomais, INPUT); //define o botao "mais" como um input

//Wifi
WiFiManager wifiManager;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------

//ÁREA DO LOOP
//Área responsável pelo processamento continuo do Arduino.

void loop() {

//Testa se o WIfi está conectado
if (WiFi.status() == WL_CONNECTED && conectado == 0) {
 
 Serial.println("");
 Serial.println("WiFi Conectado");
  // Incia o Servidor
  server.begin();
  Serial.println("Servidor Iniciado");                    
  // Endereço IP
  timeout = 0;
  while (!Serial.available() && timeout < 3000) {
    ip = WiFi.localIP().toString();
    Serial.println(ip);
    delay(500);
    }
 conectado = 1;
 }

if (WiFi.status() != WL_CONNECTED && conectado == 1) {
 conectado = 0;
 ip = "";
 }


//testa se o relógio está funcionando
DateTime now = rtc.now();
int dsdisp = 0;
if (now.year() == 2165) {
  dsdisp = 0;
} else {
  dsdisp = 1;
}
if (dsdisp == 1){
 hora = now.hour();
 minuto = now.minute();
 diasemana = now.dayOfTheWeek();
 diames = now.day();
 mes = now.month();
 ano = now.year();
 }


//Acionamento do Relé por meio do Sensor de Distância
delay(29); //29ms deveria ser o menor tempo de espera entre pings.
mediasonaratual = sonar.ping_median(3);
if (mediasonaratual > 0 && mediasonaranterior == 0 && mediasonaranterior2 == 0){
 if (estadorele == HIGH) {
 estadorele = LOW;
  } else{
   estadorele = HIGH;
    }
 }
mediasonaranterior2 = mediasonaranterior;
mediasonaranterior = mediasonaratual;
digitalWrite(rele, estadorele);

//Sensor de Temperatura e Umidade
umid = dht.readHumidity(); //a variável "h" recebe a leitura atual de umidade do DHT11
temp = dht.readTemperature(); //a variável "t" recebe a leitura atual de temperatura do DHT11

//Botão Set -> acende e apaga a luz de fundo (backlight) do Display LCD ou aciona o despertador/desligamento na tela "Despertador/Desligamento"
estadobotaoset = digitalRead(botaoset);
if (estadobotaoset == HIGH  && estadomenu != 3 && anteriorestadobotaoset == LOW && anteriorestadobotaomais == LOW) {
 if (estadolcd == 1) {
 lcd.setBacklight(LOW);
 estadolcd = 0;
  } else if (estadolcd == 0) {
   lcd.setBacklight(HIGH);
   estadolcd = 1;
     }
  } else if (estadobotaoset == HIGH  && estadomenu == 3 && anteriorestadobotaoset == LOW) {
    switch (estadodespdesl) {
      case 0:
        estadodesp = estadodesp;
        estadodesl = estadodesl;
        break;
      case 1:
        estadodesp = 1;
        estadodesl = 0;
        break;
      case 2:
        estadodesp = 0;
        estadodesl = 1;
        break;
      case 3:
        estadodesp = 1;
        estadodesl = 1;
        break;
      case 4:
        estadodesp = 0;
        estadodesl = 0;
        break;
         }
    estadodespdesl = (++estadodespdesl < 5) ? estadodespdesl : 0;
     }
anteriorestadobotaoset = estadobotaoset;

//Botão Mais -> troca entre as telas de menu
estadobotaomais = digitalRead(botaomais);
if (estadobotaomais == HIGH && anteriorestadobotaomais == LOW){
 estadomenu = estadomenu + 1;
 lcd.clear();
 }
anteriorestadobotaomais = estadobotaomais;
if (estadomenu > 5) {
 estadomenu = 0;
 }
//------------------------------------------------------------------------
// Menu 1 -> Completo: Data, Hora, Temperatura e Umidade
if (estadomenu == 0) {
 //Testa se o sensor de temperatura e umidade está recebendo dados
 if (isnan(temp) || isnan(umid)) {
  lcd.setCursor(0, 1);
  lcd.print("Falha Temp/Umid");
  }
   //Caso esteja recebendo dados, mostrará a temperatura e a umidade em posições especificas do display LCD
   else {
    lcd.setCursor(0, 1);
    lcd.print("T:");
    lcd.print(temp, 1);
    lcd.setCursor(6, 1);
    lcd.print((char)223);
    lcd.print("C");
    lcd.setCursor(9, 1);
    lcd.print("U:");
    lcd.print(umid, 1);
    lcd.setCursor(15, 1);
    lcd.print("%");
    }
 //Funcionamento do Relógio no Display LCD
 lcd.setCursor(0, 0);
 if (dsdisp == 1) {
  //Dia da semana -> Define os dias da semana em números. Ex. Se o dia da semana fot igual a 1, aparecerá no display "Dom" (Domingo)
  lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
  lcd.print(" ");
  //Dia do Mês
  if (now.day() < 10) {
   lcd.print("0");
   }
  lcd.print(now.day()); //Coloca o número do dia do mês no Display LCD
  lcd.print("/");
  //Mês
  if (now.month() < 10) {
   lcd.print("0");
   }
  lcd.print(now.month()); //Coloca o número do mês no Display LCD
  lcd.print(" "); 
  //Hora
  if (now.hour() < 10) {
   lcd.print("0");
   }
  lcd.print(now.hour());//Coloca a hora no Display LCD
  lcd.print(":");
  //Minuto
  if (now.minute() < 10) {
   lcd.print("0");
   }
  lcd.print(now.minute()); //Coloca os minutos no Display LCD
  }
   else {
    lcd.print(" Falha Relogio ");  
    }
  }
//------------------------------------------------------------------------
// Menu 2 -> Data e Hora
if (estadomenu == 1) {
 if (dsdisp == 1) {
  //linha 0 do Display LCD
  lcd.setCursor(1, 0); 
  //Define os dias da semana em números. Ex. Se o dia da semana é igual a 1, aparecerá no Display LCD "Dom" (Domingo)
  lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
  lcd.print(" "); 
  if (now.day() < 10) {
   lcd.print("0");
   }
  lcd.print(now.day()); //Coloca o dia do mês no Display LCD
  lcd.print("/");
  if (now.month() < 10) {
   lcd.print("0");
   }
  lcd.print(now.month()); //Coloca o número do mês no display LCD
  lcd.print("/");
  lcd.print(now.year()); //Coloca o ano
  //linha 1 do Display LCD
  lcd.setCursor(4, 1);
  if (now.hour() < 10) {
   lcd.print("0");
   }
  lcd.print(now.hour()); //Coloca a hora no display LCD
  lcd.print(":");
  if (now.minute() < 10) {
   lcd.print("0");
   }
  lcd.print(now.minute()); //Coloca os minutos no Display LCD
  lcd.print(":"); 
  if (now.second() < 10) {
   lcd.print("0");
   }
  lcd.print(now.second()); //Coloca os segundos no Display LCD 
  } 
   else{
    lcd.print(" Falha Relogio ");
    }
  }
//------------------------------------------------------------------------
// Menu 3 -> Temperatura e Umidade  
if (estadomenu == 2) {
 //Testa se o sensor de temperatura e umidade está recebendo dados
 if (isnan(temp) || isnan(umid)) {\
  lcd.setCursor(0, 0);
  lcd.print("Falha Temp/Umid");
  }
  //Caso esteja recebendo dados, mostrará a temperatura e a umidade em posições especificas do display LCD
   else {
    lcd.setCursor(0, 0);
    lcd.print(" TEMP: ");
    lcd.print(temp);
    lcd.setCursor(13, 0);
    lcd.print((char)223);
    lcd.print("C");
    lcd.setCursor(0, 1);
    lcd.print(" UMID: ");
    lcd.print(umid);
    lcd.setCursor(13, 1);
    lcd.print("% ");
    }
 }
//------------------------------------------------------------------------
// Menu 4 -> Despertador e Desligamento 
// Indica horário e estado do despertador e desligamento automático
if (estadomenu == 3) {
 //para despertador
 lcd.setCursor(0, 0);
 lcd.print("Desper: ");
 if (desphora < 10) {
  lcd.print("0");
  lcd.print(desphora);
  } 
   else {
    lcd.print(desphora);
    }
 lcd.print(":");
 if (despminuto < 10) {
  lcd.print("0");
  lcd.print(despminuto);
  } 
   else {
    lcd.print(despminuto);
    }
 lcd.setCursor(14, 0);
 if (estadodesp == 0) {
  lcd.print("N");
  } 
   else {
    lcd.print("S");
    }
 //para desligamento
 lcd.setCursor(0, 1);
 lcd.print("Deslig: ");
 if (deslhora < 10) {
  lcd.print("0");
  lcd.print(deslhora);
  } 
   else {
    lcd.print(deslhora);
    }
 lcd.print(":");
 if (deslminuto < 10) {
  lcd.print("0");
  lcd.print(deslminuto);
  } 
   else {
    lcd.print(deslminuto);
    }
 lcd.setCursor(14, 1);
 if (estadodesl == 0) {
  lcd.print("N");
  } 
   else {
    lcd.print("S");
    }
 }
//------------------------------------------------------------------------
// Menu 5 -> Tela Customizada 
// Mostra a frase customizada pelo usuário, via aplicativo. Frase armazenada nas variáveis "linha1" e "linha2".
if (estadomenu == 4) {
 if (linha1 == "" && linha2 == ""){
  lcd.setCursor(0,0);
  lcd.print("Tela");
  lcd.setCursor(0,1);
  lcd.print("Personalizada");
 }
 lcd.setCursor(0,0);
 lcd.print(linha1);
 lcd.setCursor(0,1);
 lcd.print(linha2);  
 }
//------------------------------------------------------------------------
// Menu 6 -> IP Atual
// Indica o endereço IP no qual o projeto está conectado, se estiver conectado a uma rede Wifi.
if (estadomenu == 5) {
 if (WiFi.status() == WL_CONNECTED && conectado == 1 && ip != "") {
  lcd.setCursor(0,0);
  lcd.print("IP:            ");
  lcd.setCursor(0,1);
  lcd.print(ip);
  } else {
     lcd.setCursor(0,0);
     lcd.print("IP:            ");
     lcd.setCursor(0,1);
     lcd.print("Indisponivel   ");
     }
 }
//------------------------------------------------------------------------

//Acionamento do rele no horario definido para o despertador
if (now.hour() == desphora && now.minute() == despminuto && now.second() < 3 & estadodesp == 1) {
 estadorele = HIGH;
 digitalWrite(rele, estadorele);
 estadodesp = 0;
 }

//Acionamento do rele no horario definido para o desligamento
if (now.hour() == deslhora && now.minute() == deslminuto && now.second() < 3 && estadodesl == 1) {
 estadorele = LOW;
 digitalWrite(rele, estadorele);
 estadodesl = 0;
 }

//Indicador de despertador e desligamento
//Quando o despertador ou o desligamento está ativo, o símbolo "*" aparece em todas as telas de menu do Display LCD, na posição (15,0).
if (estadodesl == 0 & estadodesp == 0) {
 lcd.setCursor(15, 0);
 lcd.print(" ");
 } 
  else {
   lcd.setCursor(15, 0);
   lcd.print("*");
   }

//Chama a função que ativa o modo de configuração do projeto
if (digitalRead(botaoset) == HIGH & digitalRead(botaomais) == HIGH) {
 lcd.setBacklight(HIGH);
 estadolcd = 1;
 menuconfig(); //função de configuração de parâmetros do projeto
 }  

//Codigo Wifi para receber dados do Alvaapp -> Cliente = App
 
//Checa se o cliente esta conectado
WiFiClient client = server.available();
if (!client) {
 return;
 } 
 
//Espera o cliente mandar alguma informação
while (!client.available()) {
 delay(1);
 }

// Recebe a primeira linha da requisição do AlvaAPP
String request = client.readStringUntil('\r');
Serial.println(request); 

//Checa se precisa fazer algo a partir da requisição do cliente

//Reset do wifi via app
if (request.indexOf("rst") != -1) {
 Serial.print("Reser via Alva App");
 delay(500);
 WiFiManager wifiManager;
 wifiManager.resetSettings();
 delay(5000);
 ESP.restart();
 }

 //Acender/Apagar Rele via aplicativo
if (request.indexOf("rm") != -1) {
 if (digitalRead(rele) == HIGH) {
  digitalWrite(rele, LOW);
  estadorele = 0;
  }
   else {
    digitalWrite(rele,HIGH);
    estadorele = 1;
    }
 Serial.println("*");
 }

//Acender/Apagar Display LCD via aplicativo
if (request.indexOf("dym") != -1) {
 if(estadolcd == 1) {
  lcd.setBacklight(LOW);
  estadolcd = 0;
  }
   else {
    lcd.setBacklight(HIGH);
    estadolcd = 1;
    }
 Serial.println("*");
 }
  
//Troca de estado dos Menus do Display LCD via aplicativo
//Menu1
if (request.indexOf("em0") != -1) {
 estadomenu = 0;
 lcd.clear();
 Serial.println("*");
 }
//Menu2
if (request.indexOf("em1") != -1) {
 estadomenu = 1;
 lcd.clear();
 Serial.println("*");
 }
//Menu3
if (request.indexOf("em2") != -1) {
 estadomenu = 2;
 lcd.clear();
 Serial.println("*");
 }
//Menu4
if (request.indexOf("em3") != -1) {
  estadomenu = 3;
  lcd.clear();
  Serial.println("*");
  }
//Menu5
if (request.indexOf("em4") != -1) {
 estadomenu = 4;
 lcd.clear();
 Serial.println("*");
 }
//Menu6
if (request.indexOf("em5") != -1) {
 estadomenu = 5;
 lcd.clear();
 Serial.println("*");
 }

//Funções do Despertador via aplicativo
//Ativar Despertador
if (request.indexOf("edp1") != -1) {
 estadodesp = 1;
 //Serial.print("Despertador: ");
 //Serial.print(estadodesp);
 }
//Desativar Despertador
if (request.indexOf("edp0") != -1) {
 estadodesp = 0;
 //Serial.print("Despertador: ");
 //Serial.print(estadodesp);
 }
 
//Configuração de horário do Despertador (ex: dp0228)
if (request.indexOf("/dp") != -1) {
 String txtdesphora = request.substring(request.indexOf("/dp") + 3, request.indexOf("/dp") + 5);
 desphora = txtdesphora.toInt();
 //Serial.print("Desphora: ");
 //Serial.print(desphora);
 String txtdespminuto = request.substring(request.indexOf("/dp") + 5, request.indexOf("/dp") + 7);
 despminuto = txtdespminuto.toInt();
 //Serial.print("Despminuto: ");
 //Serial.print(despminuto);
 estadomenu = 3;
 }

//Funções do Desligamento via aplicativo
//Ativar Desligamento
if (request.indexOf("edl1") != -1) {
 estadodesl = 1;
 //Serial.print("Desligamento: ");
 //Serial.print(estadodesl);
 }
//Desativar Desligamento
if (request.indexOf("edl0") != -1) {
 estadodesl = 0;
 //Serial.print("Desligamento: ");
 //Serial.print(estadodesl);
 }
//Configuração de horário do Desligamento (ex: dl0228)
if (request.indexOf("/dl") != -1) {
 String txtdeslhora = request.substring(request.indexOf("/dl") + 3, request.indexOf("/dl") + 5);
 deslhora = txtdeslhora.toInt();
 //Serial.print("Deslhora: ");
 //Serial.print(deslhora);
 String txtdeslminuto = request.substring(request.indexOf("/dl") + 5, request.indexOf("/dl") + 7);
 deslminuto = txtdeslminuto.toInt();
 //Serial.print("Deslminuto: ");
 //Serial.print(deslminuto);
 estadomenu = 3;
 }

//Configuração de Horário e Data via aplicativo
//horário (ex: adh0228125122019)
// atualizar data hora, minuto, dia da semana, dia do mes, ano
if (request.indexOf("adh") != -1) {
 String txthora = request.substring(request.indexOf("adh") + 3, request.indexOf("adh") + 5);
 hora = txthora.toInt();
 //Serial.print("Hora: ");
 //Serial.print(hora);
 String txtminuto = request.substring(request.indexOf("adh") + 5, request.indexOf("adh") + 7);
 minuto = txtminuto.toInt();
 //Serial.print("Minuto: ");
 //Serial.print(minuto);
 String txtdiasemana = request.substring(request.indexOf("adh") + 7, request.indexOf("adh") + 8);
 diasemana = txtdiasemana.toInt();
 //Serial.print("Diasemana: ");
 //Serial.print(diasemana);
 String txtdiames = request.substring(request.indexOf("adh") + 8, request.indexOf("adh") + 10);
 diames = txtdiames.toInt();
 //Serial.print("Diames: ");
 //Serial.print(diames);
 String txtmes = request.substring(request.indexOf("adh") + 10, request.indexOf("adh") + 12);
 mes = txtmes.toInt();
 //Serial.print("Mes: ");
 //Serial.print(mes);
 String txtano = request.substring(request.indexOf("adh") + 12, request.indexOf("adh") + 16);
 ano = txtano.toInt();
 //Serial.print("Ano: ");
 //Serial.print(ano);
 rtc.adjust(DateTime(ano, mes, diames, hora, minuto, 0));
 }

//Recebe as informações da tela customizada via aplicativo. Armazena na variável "linha1" e "linha2". 
//Tela Customizada Linha 1(ex:tl116alolinha1)
if (request.indexOf("/tl1") != -1) {
 delay(30);
 String txtlentl1 = request.substring(request.indexOf("/tl1") + 4, request.indexOf("/tl1") + 6);
 int lentl1 = txtlentl1.toInt();
 String tl1 = request.substring(request.indexOf("/tl1") + 6, request.indexOf("/tl1") + 6 + lentl1);
 tl1.replace("*", " ");
 linha1 = tl1;
 String txtlentl2 = request.substring(request.indexOf("/tl2") + 4, request.indexOf("/tl2") + 6);
 int lentl2 = txtlentl2.toInt();
 String tl2 = request.substring(request.indexOf("/tl2") + 6, request.indexOf("/tl2") + 6 + lentl2);
 tl2.replace("*", " ");
 linha2 = tl2;
 estadomenu = 4;
 lcd.clear();
 lcd.setCursor(0,0);
 lcd.print(linha1);
 lcd.setCursor(0,1);
 lcd.print(linha2);
 }

Serial.flush();

// Prepara a resposta
client.println("HTTP/1.1 200 OK");
client.println("Content-Type: text/html");
client.println("");

client.print(",");

//Estado do rele
if (digitalRead(rele)) {
 client.print("r1");
 }
  else {
   client.print("r0");
   }
client.print(",");

//Estado do Display LCD
if (estadolcd == 1) {
 client.print("d1");
 }
  else {
   client.print("d0");
   }
client.print(",");

//Estado do menu
client.print("em");
client.print(estadomenu);
client.print(",");

//Estado do Despertador
if (estadodesp == 0) {
 client.print("edp0");
 } 
 else {
  client.print("edp1");
  }
client.print(",");

//Estado do Desligamento
if (estadodesl == 0) {
  client.print("edl0");
  }
  else {
   client.print("edl1");
   }
   client.print(",");

//Outras variáveis

//despertador
//desphora
client.print("dph");
if (desphora < 10) {
 client.print("0");
 client.print(desphora);
 }
  else {
   client.print(desphora);
   }
client.print(",");
//despminuto
client.print("dpm");
if (despminuto < 10) {
 client.print("0");
 client.print(despminuto);
 }
  else {
   client.print(despminuto);
   }
client.print(",");

//desligamento
//deslhora
client.print("dlh");
if (deslhora < 10) {
 client.print("0");
 client.print(deslhora);
 }
  else {
   client.print(deslhora);
   }
client.print(",");
//deslminuto
client.print("dlm");
if (deslminuto < 10) {
 client.print("0");
 client.print(deslminuto);
 }
  else {
   client.print(deslminuto);
   }
client.print(",");

//Data e Hora
//hora
client.print("hr");
if (hora < 10) {
 client.print("0");
 client.print(hora);
 }
  else {
   client.print(hora);
   }
client.print(",");
//minuto
client.print("mn");
if (minuto < 10) {
 client.print("0");
 client.print(minuto);
 } 
  else {
   client.print(minuto);
   }
client.print(",");
//diasemana
client.print("ds");
client.print(diasemana);
client.print(",");
//diames
client.print("dm");
if (diames < 10) {
 client.print("0");
 client.print(diames);
 } 
  else {
   client.print(diames);
   }
client.print(",");
//mes
client.print("ms");
if (mes < 10) {
 client.print("0");
 client.print(mes);
 }
  else {
   client.print(mes);
   }
client.print(",");
//ano
client.print("an");
if (ano < 10) {
 client.print("000");
 client.print(ano);
 }
  else {
   if (ano < 100) {
    client.print("00");
    client.print(ano);
    }
     else {
      if (ano < 1000) {
       client.print("0");
       client.print(ano);
       }
        else {
         client.print(ano);
         }
      }
   }

// Indicador de final da mensagem
client.println(",***");

timeout = 0;
client.flush();
delay(1);  
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------

//Função menuconfig
//Modo de configuração do Arduino, após acionar o botão "Set"e o botão "Mais", ao mesmo tempo.

void menuconfig() {

delay(1000);
lcd.clear();

//Escolha  inicial das configurações: Data e Hora, Despertador, Desligamento e Reset Wifi
while (digitalRead(botaoset) == LOW) {
 if (digitalRead(botaomais) == LOW) {
  menu = menu;
  }
   else {
    menu = menu + 1;
    if (menu > 3) {
     menu = 0;
     }
    }
 if (menu == 0) {
  lcd.setCursor(0, 0);
  lcd.print("Configuracao:   ");
  lcd.setCursor(0, 1);
  lcd.print("Data e Hora     ");
  delay(500);
  } 
   else if (menu == 1) {
    lcd.setCursor(0, 0);
    lcd.print("Configuracao:   ");
    lcd.setCursor(0, 1);
    lcd.print("Despertador     ");
    delay(500);
    }
      else if (menu == 2) {
       lcd.setCursor(0, 0);
       lcd.print("Configuracao:   ");
       lcd.setCursor(0, 1);
       lcd.print("Desligamento    ");
       delay(500);
       }
        else if (menu == 3) {
         lcd.setCursor(0, 0);
         lcd.print("Configuracao:   ");
         lcd.setCursor(0, 1);
         lcd.print("Wifi            ");
         delay(500);
         }
  }
  
//Cancela o modo de configuração e retorna ao menu inicial
if (digitalRead(botaoset) == HIGH & digitalRead(botaomais) == HIGH) {
 lcd.clear();
 delay(500);
 return;
 }

lcd.setCursor(14, 1);
lcd.print("OK");
delay(2000);
lcd.clear();

if (menu == 0) {
 relogio(); //chama função de configuração do horário e data
 } else if (menu == 1) {
   despertador(); //chama função de configuração do horário do despertador
 } else if (menu == 2) {
    desligamento(); //chama função de configuração do horário do desligamento
 } else if (menu == 3) {
    configwifi(); //chama função responsável por redefinir os padrões do Wifi
 }

}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------

//Função relogio
//Altera o horário e data do módulo relógio (DS3231)

void relogio() {
DateTime now = rtc.now();

lcd.setCursor(0, 0);
lcd.print("  Configurando  ");
lcd.setCursor(0, 1);
lcd.print("  Data e Hora   ");
delay(2000);
lcd.clear();

//Minuto
lcd.setCursor(0, 0);
lcd.print("Minuto:");
lcd.setCursor(0, 1);
minuto = now.minute();
while (digitalRead(botaoset) == LOW) {
 if (digitalRead(botaomais) == LOW) {
  minuto = minuto;
  } 
   else {
    minuto = minuto + 1;
    if (minuto > 59) {
     minuto = 0;
     }
    }
 if (minuto < 10) {
  lcd.setCursor(0, 1);
  lcd.print("0");
  lcd.setCursor(1, 1);
  lcd.print(minuto);
  delay(500);
  }
   else {
    lcd.setCursor(0, 1);
    lcd.print(minuto);
    delay(500);
    }

 }
//Cancela o modo de configuração e retorna ao menu inicial
if (digitalRead(botaoset) == HIGH & digitalRead(botaomais) == HIGH) {
 lcd.clear();
 delay(500);
 return;
 }
lcd.setCursor(14, 1);
lcd.print("OK");
delay(1000);
lcd.clear();
 
//Hora
lcd.setCursor(0, 0);
lcd.print("Hora:");
lcd.setCursor(0, 1);
hora = now.hour();
while (digitalRead(botaoset) == LOW) {
 if (digitalRead(botaomais) == LOW) {
  hora = hora;
  }
   else {
    hora = hora + 1;
    if (hora > 23) {
     hora = 0;
     }
    }
 if (hora < 10) {
  lcd.setCursor(0, 1);
  lcd.print("0");
  lcd.setCursor(1, 1);
  lcd.print(hora);
  delay(500);
  }
   else {
    lcd.setCursor(0, 1);
    lcd.print(hora);
    delay(500);
    }
 }

//Cancela o modo de configuração e retorna ao menu inicial
if (digitalRead(botaoset) == HIGH & digitalRead(botaomais) == HIGH) {
 lcd.clear();
 delay(500);
 return;
 }
 
lcd.setCursor(14, 1);
lcd.print("OK");
delay(1000);
lcd.clear();

 
//Dia do Mês
lcd.setCursor(0, 0);
lcd.print("Dia do Mes:");
lcd.setCursor(0, 1);
diames = now.day();
 while (digitalRead(botaoset) == LOW) {
  if (digitalRead(botaomais) == LOW) {
   diames = diames;
   }
   else {
    diames = diames + 1;
    if (diames > 31) {
     diames = 1;
     }
    }
  if (diames < 10) {
   lcd.setCursor(0, 1);
   lcd.print("0");
   lcd.setCursor(1, 1);
   lcd.print(diames);
   delay(500);
   }
    else {
     lcd.setCursor(0, 1);
     lcd.print(diames);
     delay(500);
     }
  }

//Cancela o modo de configuração e retorna ao menu inicial
if (digitalRead(botaoset) == HIGH & digitalRead(botaomais) == HIGH) {
 lcd.clear();
 delay(500);
 return;
 }
 
lcd.setCursor(14, 1);
lcd.print("OK");
delay(1000);
lcd.clear();

//Mês
lcd.setCursor(0, 0);
lcd.print("Mes:");
lcd.setCursor(0, 1);
mes = now.month();
 while (digitalRead(botaoset) == LOW) {
  if (digitalRead(botaomais) == LOW) {
   mes = mes;
   }
   else {
    mes = mes + 1;
    if (mes > 12) {
     mes = 1;
     }
    }
  if (mes < 10) {
   lcd.setCursor(0, 1);
   lcd.print("0");
   lcd.setCursor(1, 1);
   lcd.print(mes);
   delay(500);
   }
    else {
     lcd.setCursor(0, 1);
     lcd.print(mes);
     delay(500);
     }
  }

//Cancela o modo de configuração e retorna ao menu inicial
if (digitalRead(botaoset) == HIGH & digitalRead(botaomais) == HIGH) {
 lcd.clear();
 delay(500);
 return;
 }

lcd.setCursor(14, 1);
lcd.print("OK");
delay(1000);
lcd.clear();

//Ano
lcd.setCursor(0, 0);
lcd.print("Ano:");
lcd.setCursor(0, 1);
ano = now.year();
while (digitalRead(botaoset) == LOW) {
 if (digitalRead(botaomais) == LOW) {
  ano = ano;
  }
   else {
    ano = ano + 1;
    if (ano > 2099) {
     ano = 2000;
     }
    }
 lcd.setCursor(0, 1);
 lcd.print(ano);
 delay(500);
 }

//Cancela o modo de configuração e retorna ao menu inicial 
if (digitalRead(botaoset) == HIGH & digitalRead(botaomais) == HIGH) {
 lcd.clear();
 delay(500);
 return;
 }

lcd.setCursor(14, 1);
lcd.print("OK");
delay(1000);
lcd.clear();

//Envio efetivo das informações para o módulo relógio (DS3231)
rtc.adjust(DateTime(ano, mes, diames, hora, minuto, 0));

lcd.setCursor(0, 0);
lcd.print("  Configuracao  ");
lcd.setCursor(0, 1);
lcd.print("   Finalizada   ");
delay(2000);
lcd.clear();

}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------

//Função despertador
//Altera o horário do despertador

void despertador() {
DateTime now = rtc.now();

lcd.setCursor(0, 0);
lcd.print("  Configurando  ");
lcd.setCursor(0, 1);
lcd.print("  Despertador   ");
delay(2000);
lcd.clear();

//Hora do Despertador -> desphora
lcd.setCursor(0, 0);
lcd.print("Hora:");
lcd.setCursor(0, 1);
desphora = now.hour();
while (digitalRead(botaoset) == LOW) {
 if (digitalRead(botaomais) == LOW) {
  desphora = desphora;
  }
   else {
    desphora = desphora + 1;
    if (desphora > 23) {
     desphora = 0;
     }
    }
 if (desphora < 10) {
  lcd.setCursor(0, 1);
  lcd.print("0");
  lcd.setCursor(1, 1);
  lcd.print(desphora);
  delay(500);
  }
   else {
    lcd.setCursor(0, 1);
    lcd.print(desphora);
    delay(500);
    }
 }
 
//Cancela o modo de configuração e retorna ao menu inicial   
if (digitalRead(botaoset) == HIGH & digitalRead(botaomais) == HIGH) {
 lcd.clear();
 delay(500);
 return;
 }
 
lcd.setCursor(14, 1);
lcd.print("OK");
delay(1000);
lcd.clear();

//Minuto do Despertador -> despminuto
lcd.setCursor(0, 0);
lcd.print("Minuto:");
lcd.setCursor(0, 1);
despminuto = now.minute();
while (digitalRead(botaoset) == LOW) {
 if (digitalRead(botaomais) == LOW) {
  despminuto = despminuto;
  }
   else {
    despminuto = despminuto + 1;
    if (despminuto > 59) {
     despminuto = 0;
     }
    }
 if (despminuto < 10) {
  lcd.setCursor(0, 1);
  lcd.print("0");
  lcd.setCursor(1, 1);
  lcd.print(despminuto);
  delay(500);
  }
   else {
    lcd.setCursor(0, 1);
    lcd.print(despminuto);
    delay(500);
    }
 }

//Cancela o modo de configuração e retorna ao menu inicial  
if (digitalRead(botaoset) == HIGH & digitalRead(botaomais) == HIGH) {
 lcd.clear();
 delay(500);
 return;
 }

lcd.setCursor(14, 1);
lcd.print("OK");
delay(1000);
lcd.clear();

lcd.setCursor(0, 0);
lcd.print("Despertar:");
lcd.setCursor(0, 1);
if (desphora < 10) {
 lcd.print("0");
 lcd.print(desphora);
 }
  else {
   lcd.print(desphora);
   }
 lcd.print(":");
 if (despminuto < 10) {
  lcd.print("0");
  lcd.print(despminuto);
  }
   else {
    lcd.print(despminuto);
    }
estadodesp = 1;
delay (2000);
lcd.clear();

}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------

//Função desligamento
//Altera o horário do desligamento

void desligamento() {
DateTime now = rtc.now();

lcd.setCursor(0, 0);
lcd.print("  Configurando  ");
lcd.setCursor(0, 1);
lcd.print("  Desligamento  ");
delay(2000);
lcd.clear();

//Hora do Delisgamento -> deslhora
 lcd.setCursor(0, 0);
 lcd.print("Hora:");
 lcd.setCursor(0, 1);
 deslhora = now.hour();
 while (digitalRead(botaoset) == LOW) {
  if (digitalRead(botaomais) == LOW) {
   deslhora = deslhora;
   }
    else {
     deslhora = deslhora + 1;
     if (deslhora > 23) {
      deslhora = 0;
      }
     }
  if (deslhora < 10) {
   lcd.setCursor(0, 1);
   lcd.print("0");
   lcd.setCursor(1, 1);
   lcd.print(deslhora);
   delay(500);
   }
    else {
     lcd.setCursor(0, 1);
     lcd.print(deslhora);
     delay(500);
     }
  }

//Cancela o modo de configuração e retorna ao menu inicial   
if (digitalRead(botaoset) == HIGH & digitalRead(botaomais) == HIGH) {
 lcd.clear();
 delay(500);
 return;
 }
 
lcd.setCursor(14, 1);
lcd.print("OK");
delay(1000);
lcd.clear();

//Minuto do Desligamento -> deslminuto
lcd.setCursor(0, 0);
lcd.print("Minuto:");
lcd.setCursor(0, 1);
deslminuto = now.minute();
while (digitalRead(botaoset) == LOW) {
 if (digitalRead(botaomais) == LOW) {
  deslminuto = deslminuto;
  }
   else {
    deslminuto = deslminuto + 1;
    if (deslminuto > 59) {
     deslminuto = 0;
     }
    }
 if (deslminuto < 10) {
  lcd.setCursor(0, 1);
  lcd.print("0");
  lcd.setCursor(1, 1);
  lcd.print(deslminuto);
  delay(500);
  }
   else {
    lcd.setCursor(0, 1);
    lcd.print(deslminuto);
    delay(500);
    }
 }
 
//Cancela o modo de configuração e retorna ao menu inicial  
if (digitalRead(botaoset) == HIGH & digitalRead(botaomais) == HIGH) {
 lcd.clear();
 delay(500);
 return;
 }
 
lcd.setCursor(14, 1);
lcd.print("OK");
delay(1000);
lcd.clear();

lcd.setCursor(0, 0);
lcd.print("Desligar:");
lcd.setCursor(0, 1);
if (deslhora < 10) {
 lcd.print("0");
 lcd.print(deslhora);
 }
  else {
   lcd.print(deslhora);
   }
lcd.print(":");
if (deslminuto < 10) {
 lcd.print("0");
 lcd.print(deslminuto);
 }
  else {
   lcd.print(deslminuto);
   }
estadodesl = 1;
delay (2000);
lcd.clear();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------

//Função Configurar Wifi
//Redefine os padrões do wifi para conectar em uma nova rede
void configwifi() {

lcd.setCursor(0, 0);
lcd.print("  Configurando  ");
lcd.setCursor(0, 1);
lcd.print("      Wifi      ");
delay(2000);
lcd.clear();

//Confirma se o usuario quer mesmo configurar o Wifi
 lcd.setCursor(0, 0);
 lcd.print("Configurar Wifi:");
 while (digitalRead(botaoset) == LOW) {
  if (digitalRead(botaomais) == LOW) {
   menuwifi = menuwifi;
   }
    else {
     menuwifi = menuwifi + 1;
     if (menuwifi > 1) {
      menuwifi = 0;
      }
     }
  if (menuwifi == 0) {
   lcd.setCursor(0, 1);
   lcd.print("Sim");
   delay(500);
   } else if (menuwifi == 1) {
      lcd.setCursor(0, 1);
      lcd.print("Nao");
      delay(500);
      }
  }

//Cancela o modo de configuração e retorna ao menu inicial   
if (digitalRead(botaoset) == HIGH & digitalRead(botaomais) == HIGH) {
 lcd.clear();
 delay(500);
 return;
 }
 
lcd.setCursor(14, 1);
lcd.print("OK");
delay(1000);
lcd.clear();

//Se escolher "nao", sai da configuração
if (menuwifi == 1) {
lcd.setCursor(0, 0);
lcd.print("  Configuracao  ");
lcd.setCursor(0, 1);
lcd.print("   Finalizada   ");
delay(2000);
lcd.clear();
return;  
}

//Confirma se o usuario já resetou o Wifi:
 lcd.setCursor(0, 0);
 lcd.print("Ja efetuou reset");
 lcd.setCursor(0, 1);
 lcd.print("do Wifi? ");
 while (digitalRead(botaoset) == LOW) {
  if (digitalRead(botaomais) == LOW) {
   menuwifi2 = menuwifi2;
   }
    else {
     menuwifi2 = menuwifi2 + 1;
     if (menuwifi2 > 1) {
      menuwifi2 = 0;
      }
     }
  if (menuwifi2 == 0) {
   lcd.setCursor(9, 1);
   lcd.print("Sim");
   delay(500);
   } else if (menuwifi2 == 1) {
      lcd.setCursor(9, 1);
      lcd.print("Nao");
      delay(500);
      }
  }

if (menuwifi2 == 1) {
 //Reset do Wifi
 WiFiManager wifiManager;
 wifiManager.resetSettings();
 lcd.clear();
 lcd.setCursor(0, 0);
 lcd.print("Resetando Wifi  ");
 lcd.setCursor(0, 1);
 lcd.print("      .");
 delay(1000);
 lcd.print(".");
 delay(1000);
 lcd.print(".");
 delay(1000);
 lcd.clear();
 lcd.setCursor(0, 0);
 lcd.print("Reinicializacao ");
 lcd.setCursor(0, 1);
 lcd.print("necessaria");
 delay(1000);
 lcd.print(".");
 delay(1000);
 lcd.print(".");
 delay(1000);
 lcd.print(".");
 delay(1000);
 lcd.print(".");
 delay(1000);
 lcd.print(".");
 ESP.restart();   
}
 
//Ativa o portal wifi "Alva"
WiFiManager wifiManager;
wifiManager.resetSettings();
lcd.clear();
lcd.setCursor(0, 0);
lcd.print("Criando Portal");
lcd.setCursor(0, 1);
lcd.print("      .");
delay(1000);
lcd.print(".");
delay(1000);
lcd.print(".");
delay(1000);
lcd.clear();
lcd.setCursor(0, 0);
lcd.print("Acesse Wifi ALVA");
lcd.setCursor(0, 1);
lcd.print("e 192.168.4.1   ");
wifiManager.autoConnect("ALVA"); //Nome da rede a ser criada pelo módulo Wifi

//aguarda conexão wifi
lcd.clear();
lcd.setCursor(0, 0);
lcd.print("Conectando");
lcd.setCursor(0, 1);
while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  lcd.print(".");
  }
lcd.clear(); 
lcd.setCursor(0, 0);
lcd.print("Wifi Conectado! ");
delay(2000);
lcd.clear();
lcd.setCursor(0, 0);
lcd.print("  Configuracao  ");
lcd.setCursor(0, 1);
lcd.print("   Finalizada   ");
delay(2000);
lcd.clear();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------

//Fim do código
