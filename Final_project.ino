#include "DHT.h"
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h> 

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

long duration;
float distance;

DHT dht(DHTPIN, DHTTYPE);

//void TaskPushValsDb( void *pvParameters );
//void TaskSaveHumidityLocally( void *pvParameters );
void TaskBlinkLED( void *pvParameters );

WebServer server(80);
WiFiClient client;
HTTPClient http;

void setup() {
  Serial.begin(115200);
  xTaskCreatePinnedToCore(
    TaskPushValsDb
    ,  "DbTask"   // A name just for humans
    ,  50024  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  3  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL 
    ,  ARDUINO_RUNNING_CORE);

//  xTaskCreatePinnedToCore(
//    TaskSaveHumidityLocally
//    ,  "SaveHumidityLocallyTask"
//    ,  1024  // Stack size
//    ,  NULL
//    ,  2  // Priority
//    ,  NULL 
//    ,  ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
    TaskBlinkLED
    ,  "BlinkLEDTask"
    ,  1024  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL 
    ,  ARDUINO_RUNNING_CORE);

}

void loop() {
//  Serial.print("Humidity: ");
//  Serial.println(getHumidityValue());
//  Serial.print("LDR: ");
//  Serial.println(getLDRValue());
//  Serial.print("Distance: ");
//  Serial.println(getDistance());
//  Serial.println();
//  delay(1000);
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

//void TaskSaveHumidityLocally(void *pvParameters)
//{
//  (void) pvParameters;
//
////  dht.begin();
//
//  // Procedure
//  // Save last value in temporal memory
//  // Compare current reading to last reading
//  // If different, then append to an array (possibly 2D to allow timestamp as well)
//  // Every minute, rewrite local file with array values (and possibly timestamps
//}

void TaskBlinkLED(void *pvParameters)
{
  (void) pvParameters;
    
  int delayVal = 1000;
  pinMode(ledPin, OUTPUT);

  for(;;) {
      digitalWrite(ledPin, HIGH);
      vTaskDelay(delayVal);
      digitalWrite(ledPin, LOW);
      vTaskDelay(delayVal);
  }
}

float getHumidityValue() {
  return dht.readHumidity();
}

String getLDRValue() {
  return analogRead(ldrPin) < 1000 ? "Dark" : "Bright";
}

void postToDb(){
  String serverName = "http://192.168.102.178:8080/new-reading";
  digitalWrite(trigPin, LOW);
  vTaskDelay(2);
  digitalWrite(trigPin, HIGH);
  vTaskDelay(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034/2;
  
  if(WiFi.status()== WL_CONNECTED){
      // Your Domain name with URL path or IP address with path
      http.begin(client, serverName);
      
      http.addHeader("Content-Type", "application/json");
      // String json = "{\"distance\":\"Isaac_and_Daniel\",\"waterLevel\":\" ";
      String json = " {\"distance\": \"";
      json += String(int(distance));
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
