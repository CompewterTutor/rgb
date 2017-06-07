/*
   Sonoff - Wi-Fi switch https://github.com/tretyakovsa/Sonoff_WiFi_switch
   Arduino core for ESP8266 WiFi chip https://github.com/esp8266/Arduino
   Arduino ESP8266 filesystem uploader https://github.com/esp8266/arduino-esp8266fs-plugin
*/
#include <ESP8266WiFi.h>             //Содержится в пакете
#include <ESP8266WebServer.h>        //Содержится в пакете
#include <ESP8266SSDP.h>             //Содержится в пакете
#include <FS.h>                      //Содержится в пакете
#include <time.h>                    //Содержится в пакете
#include <Ticker.h>                  //Содержится в пакете
#include <WiFiUdp.h>                 //Содержится в пакете
#include <ESP8266HTTPUpdateServer.h> //Содержится в пакете
#include <ESP8266httpUpdate.h>
#include <ESP8266HTTPClient.h>       //Содержится в пакете
#include <DNSServer.h>               //Содержится в пакете
#include <StringCommand.h>           //https://github.com/tretyakovsa/ESP8266-StringCommand

#include <ArduinoJson.h>             //Ставим через менеджер библиотек
#include <DHT.h>                     //https://github.com/markruys/arduino-DHT
#include <OneWire.h>                 //Ставим через менеджер библиотек
#include <DallasTemperature.h>       //Ставим через менеджер библиотек

#include <PubSubClient.h>           //https://github.com/Imroy/pubsubclient

#include <Adafruit_NeoPixel.h>       //https://github.com/adafruit/Adafruit_NeoPixel
#include <WS2812FX.h>                //https://github.com/kitesurfer1404/WS2812FX


// DHT C автоматическим определением датчиков.Поддержка датчиков DHT11,DHT22, AM2302, RHT03.
DHT dht;

DNSServer dnsServer;

// Web интерфейсы для устройства
ESP8266WebServer HTTP(80);
ESP8266WebServer HTTPWAN;

// Обнавление прошивки
ESP8266HTTPUpdateServer httpUpdater;

// Для файловой системы
File fsUploadFile;

// Для тикера
Ticker ticker30sec;
Ticker ticker1sec;

// Для поиска других устройств по протоколу SSDP
WiFiUDP udp;
/*
// Куда что подключено в RGB
#define TACH_PIN 0    // Кнопка управления
#define BUZER_PIN 3   // Бузер
#define LED_PIN 2     // RGB лента
// If you use ESP8266 12 you can add
#define PIR_PIN 14    // RIR sensors
*/

  // Куда что подключено в sonoff
  #define TACH_PIN 0    // Кнопка управления
  #define PIR_PIN 16     // RIR sensors
  #define RELE1_PIN 12  // Реле
  #define LED_PIN 13    // Светодиод
  #define DHTPIN 14     // DHT сенсор.
  #define RGB_PIN 2     // WS2811/WS2812/NeoPixel LEDs
  #define impuls_PIN 3  //электросчетчик

/*
  // Куда что подключено в Smart-Room
  #define TACH_PIN 0    // Кнопка управления
  #define PIR_PIN 2     // RIR sensors
  #define RELE1_PIN 12  // Реле 1
  #define RELE2_PIN 13  // Реле 2
  #define RELE3_PIN 15  // Реле 3
  #define RELE4_PIN 12  // Реле 4
  #define LED_PIN 16    // Светодиод
  #define DHTPIN 4      // DHT сенсор.
  #define RGB_PIN 5
*/
//#define SERIALCOMMAND_DEBUG true
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(DHTPIN);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

int ledCount = 15;              // Количество лед огней
WS2812FX ws2812fx = WS2812FX(ledCount, RGB_PIN, NEO_GRB + NEO_KHZ800);

// Определяем переменные
String configs="";
//Обшие настройки
String ipCurrent = "";
String jsonConfig = "{}";             // Строка json для живого обмена данными
String ssidName = "WiFi";            // Для хранения SSID
String ssidPass = "";                // Для хранения пароля сети
String ssidApName = "Sonoff";        // SSID AP точки доступа
String ssidApPass = "";              // пароль точки доступа
String ssdpName = "Sonoff";          // Имя SSDP
String spaceName = "";          // Пространство установки
String subnet = "";
String getway = "";
String dns = "";
String ip = "";
int checkboxIP = 0;
int timezone = 3;                    // часовой пояс GTM
String Language = "ru";              // язык web интерфейса
String Lang = "";                    // файлы языка web интерфейса
String calibrationTime = "00:00:00"; // Время колибровки часов
String Weekday = "";                 // Текущий день недели
String Time = "";                    // Текущее время
String spiffsData="";                      // дата релиза fs
String buildData="";                      // дата релиза build
boolean ddnsTest = true;
boolean  test = true;

StringCommand sCmd;     // The demo StringCommand object

String command="";

// Переменные для обнаружения модулей
//String modulesNew ="{}";
String modulesNew = "{\"ip\":\"\",\"SSDP\":\"\",\"space\":\"\",\"module\":[]}";
String Devices = "";            // Поиск IP адресов устройств в сети
String DevicesList = "";        // IP адреса устройств в сети
// Переменные для таймеров
int timeSonoff = 10;            // Время работы реле
String jsonTimer = "{}";
String Timerset = "";

// Переменные для ddns
String ddns = "";               // url страницы тестирования WanIP
String ddnsName = "";           // адрес сайта ddns
int pirTime = 0;                // 0 = PIR off; >1 = PIR on;
int ddnsPort = 8080; // порт для обращение к устройству с wan

//Переменные для Mqtt
String mqtt_server = "cloudmqtt.com"; // Имя сервера MQTT
int mqtt_port = 1883; // Порт для подключения к серверу MQTT
String mqtt_user = ""; // Логи от сервер
String mqtt_pass = ""; // Пароль от сервера
String chipID = "";
String prefix   = "/IoTmanager";
WiFiClient wclient;
PubSubClient client(wclient);

unsigned int localPort = 1901;
unsigned int ssdpPort = 1900;

volatile int chaingtime = LOW;
volatile int chaing = LOW;
int state0 = 0;
int task = 0;
String colorRGB = "ff6600";
String speedRGB = "100";
String BrightnessRGB = "255";
String ModeRGB = "0";
String timeRGB = "";
String timeBUZ = "";
int stateRGB = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("");
  chipID += String( ESP.getChipId() ) + "-" + String( ESP.getFlashChipId() );
  Serial.println(chipID);
  FS_init();         // Включаем работу с файловой системой
  loadConfig();      // Загружаем настройки из файла
  WiFi_init();       //Запускаем WIFI
  HTTP_init();       //настраиваем HTTP интерфейс
  SSDP_init();       //запускаем SSDP сервис
  ntp_init();        // Включаем время из сети
  timers_init();     // Синхронизируем время
  tachinit();        // Включаем кнопку
  //relay_init();      //Запускаем реле
  sensor_init();     // Запускаем сенсоры
  Movement_init();   // запускаем датчик движения
  ddns_init();       //запускаем DDNS сервис
  MQTT_init();
  initRGB();
  //electricMeter();
  ticker1sec.attach(1, sec); // Будет выполняться каждую секунду проверяя таймеры
  ticker30sec.attach(30, sec30); // Будет выполняться каждую секунду проверяя таймеры
  Serial.println(jsonArray("modules.config.json", "Smart-Socket"));
}

void loop() {
  sCmd.readStr(command);     // We don't do much, just process serial commands
  command ="";
  dnsServer.processNextRequest();
  HTTP.handleClient();
  delay(1);
  HTTPWAN.handleClient();
  delay(1);
  handleUDP();
  handleMQTT();
  ws2812fx.service();
 if (ddnsTest){
  searchSSDP();
  if (ddns!="") ip_wan();
  ddnsTest = false;
  }
}

void sec(){
  sectest();
  }

void sec30(){
  //searchSSDP();
  }
