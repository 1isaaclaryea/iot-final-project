#include <SPIFFS.h>
#include "FS.h"

#include "DHT.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include "html.h"

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#define DHTPIN 4
#define DHTTYPE DHT22

#define ldrPin 34
#define trigPin 5
#define echoPin 18
#define ledPin 19

#define FORMAT_SPIFFS_IF_FAILED true

long duration;
float distance;

float lastHumidity = 0.0;

long timezone = 1; 
byte daysavetime = 1;

DHT_Unified dht(DHTPIN, DHTTYPE);

void TaskPushValsDb( void *pvParameters );
void TaskSaveHumidityLocally( void *pvParameters );
void TaskBlinkLED( void *pvParameters );
void TaskHandleWebServer(void *pvParameters);

WebServer server(80);
WiFiClient client;
HTTPClient http;

void setup() {
  Serial.begin(115200);
  dht.begin();
 
  xTaskCreatePinnedToCore(
    TaskPushValsDb
    ,  "DbTask"
    ,  50024
    ,  NULL
    ,  3  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL 
    ,  ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
    TaskSaveHumidityLocally
    ,  "SaveHumidityLocallyTask"
    ,  100024  // Stack size
    ,  NULL
    ,  2  // Priority
    ,  NULL 
    ,  ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
    TaskHandleWebServer
    ,  "HandleWebServer"
    ,  6024  // Stack size
    ,  NULL
    ,  3  // Priority
    ,  NULL 
    ,  ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
    TaskBlinkLED
    ,  "BlinkLEDTask"
    ,  1024  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL 
    ,  ARDUINO_RUNNING_CORE);

}

void loop() {}

// ---------------------------------------------- Tasks ----------------------------------------------------------- //
void TaskBlinkLED(void *pvParameters)
{
  (void) pvParameters;

  // Entry point for modifying blink delay value  
  int delayVal = 1000;
  
  pinMode(ledPin, OUTPUT);

  for(;;) {
      digitalWrite(ledPin, HIGH);
      vTaskDelay(delayVal);
      digitalWrite(ledPin, LOW);
      vTaskDelay(delayVal);
  }
}

void TaskPushValsDb(void *pvParameters)
{
  (void) pvParameters;

  pinMode(ldrPin, INPUT); // probably comment this out
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  const char* ssid = "DUFIE-HOSTEL";
  const char* password = "Duf1e@9723";
  Serial.println("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());

  for(;;) {
      postToDb();
      vTaskDelay(3000);
      postToDb();
      vTaskDelay(3000);
  }
}

void TaskSaveHumidityLocally(void *pvParameters)
{
  (void) pvParameters;  
   
  for(;;) {
    saveDataLocally();
    vTaskDelay(10000);
  }
}

void TaskHandleWebServer(void *pvParameters)
{
  Serial.println("Task activated");

  (void) pvParameters;
  server.on("/", displayDashboard);
  
  server.begin();
  Serial.println("HTTP server started");
  
  for(;;){
    server.handleClient();
  }
}

// ------------------------------------------------ End of tasks ------------------------------------------------------- //


// ------------------------------------------------ Server callbacks ------------------------------------------------------- //
void displayDashboard(){
  server.send(200, "text/html", dashboard);
}
// ------------------------------------------------ Server callbacks ------------------------------------------------------- //


// ------------------------------------------------ Helper functions --------------------------------------------------- //
void saveDataLocally() {
   if(WiFi.status()== WL_CONNECTED){
      if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
        Serial.println("SPIFFS Mount Failed");
        return;
      }

      configTime(3600*timezone, daysavetime*3600, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org");
      struct tm tmstruct ;
      vTaskDelay(2000);
      tmstruct.tm_year = 0;
      getLocalTime(&tmstruct, 5000);  
      
      String timestamp = String((tmstruct.tm_year)+1900) + "-" + String((tmstruct.tm_mon)+1) + "-" + String(tmstruct.tm_mday) + " " + String(tmstruct.tm_hour-1) + ":" + String(tmstruct.tm_min) + ":" + String(tmstruct.tm_sec);
      
      float humidity = getHumidityValue();
      String value = timestamp + " -> " + humidity + "\r\n";
      char dataArray[value.length()+1];
      value.toCharArray(dataArray, value.length()+1);
      
      if(lastHumidity != humidity){
        appendFile(SPIFFS, "/FinalExam.txt", dataArray);
        lastHumidity = humidity;
      }
   }
}

float getHumidityValue() {
  sensor_t sensor;
  dht.humidity().getSensor(&sensor);
  
  sensors_event_t event;
  dht.humidity().getEvent(&event);
  return event.temperature;
}

float getTemperature() {
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  return event.temperature;
}

String getLDRValue() {
  return analogRead(ldrPin) < 1000 ? "Dark" : "Bright";
}

void postToDb(){
  
  String serverName = "http://192.168.102.178:8080/new-reading";
  
  if(WiFi.status()== WL_CONNECTED){
      // Your Domain name with URL path or IP address with path
      http.begin(client, serverName);

      http.addHeader("Content-Type", "application/json");
      // String json = "{\"distance\":\"Isaac_and_Daniel\",\"waterLevel\":\" ";
      String json = " {\"temperature\": \"";
      Serial.println(getHumidityValue());
      json += String((getTemperature()));
      json += "\", \"illumination\": \"";
      json += getLDRValue();
      json += "\"}";
      Serial.println(json);
      int httpResponseCode = http.POST(json);
     
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
        
      // Free resources
      http.end();
    }
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\r\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("- failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("- message appended");
    } else {
        Serial.println("- append failed");
    }
    file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\r\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("- failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("- file written");
    } else {
        Serial.println("- write failed");
    }
    file.close();
}
