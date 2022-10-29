//lass graphen erscheinen wenn fertig mit allen kalibrierungen.

#include <SensorFusion.h>
#include <ESPmDNS.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <esp_task_wdt.h>
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <Arduino_JSON.h>
TaskHandle_t Task1;
SF fusion;

float gx, gy, gz, ax, ay, az, mx, my, mz;
float pitch, roll, yaw;
float deltat;

float xCalib = 4;
float zCalib = 4;
bool OverCalibX;
bool OverCalibZ;
float x;
float z;
// Replace with your network credentials
const char* ssid = "BN";
const char* password = "Nyon-bosch";

const char* ssidAP = "SportSystem";
const char* passwordAP = "";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
// Create a WebSocket object
Adafruit_MPU6050 mpu;
AsyncWebSocket ws("/ws");

String message = "";
String sliderValue1 = "0";
String sliderValue2 = "0";
String sliderValue3 = "0";

float xRot;
float zRot;
float xRotMin;
float xRotMax;
float zRotMin;
float zRotMax;


int dutyCycle1;
int dutyCycle2;
int dutyCycle3;


//Json Variable to Hold Slider Values
JSONVar sliderValues;

//Get Slider Values
String getSliderValues() {
  sliderValues["sliderValue1"] = String(sliderValue1);
  sliderValues["sliderValue2"] = String(sliderValue2);
  sliderValues["sliderValue3"] = String(sliderValue3);

  String jsonString = JSON.stringify(sliderValues);
  return jsonString;
}

// Initialize SPIFFS
void initFS() {
  if (!SPIFFS.begin()) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  else {
    Serial.println("SPIFFS mounted successfully");
  }
}

// Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void initWifiAP() {
  WiFi.softAP(ssidAP, passwordAP);
  server.begin();
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
}

void notifyClients(String sliderValues) {
  ws.textAll(sliderValues);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    message = (char*)data;
    if (message.indexOf("1s") >= 0) {
      sliderValue1 = message.substring(2);
      dutyCycle1 = sliderValue1.toInt();
      Serial.println(dutyCycle1);
      Serial.print(getSliderValues());
      notifyClients(getSliderValues());
    }
    if (message.indexOf("2s") >= 0) {
      sliderValue2 = message.substring(2);
      dutyCycle2 = sliderValue2.toInt();
      Serial.println(dutyCycle2);
      Serial.print(getSliderValues());
      notifyClients(getSliderValues());
    }
    if (message.indexOf("3s") >= 0) {
      sliderValue3 = message.substring(2);
      dutyCycle3 = sliderValue3.toInt();
      Serial.println(dutyCycle3);
      Serial.print(getSliderValues());
      notifyClients(getSliderValues());
    }
    if (strcmp((char*)data, "getValues") == 0) {
      notifyClients(getSliderValues());
    }
  }
}
void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}
float stabilize;

void setup() {
  Serial.begin(115200);
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  delay(100);
  initFS();
  
  //initWiFi();
  initWifiAP();
  initWebSocket();

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });
  server.on("/stepsStatistic", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", readSteps().c_str());
  });

    server.on("/yaw", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", yawReturn().c_str());
  });




  server.serveStatic("/", SPIFFS, "/");

  // Start server
  server.begin();


  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);



  //MultiTask for reading Data
  xTaskCreatePinnedToCore(
    Sensors, /* Function to implement the task */
    "Task1", /* Name of the task */
    10000,  /* Stack size in words */
    NULL,  /* Task input parameter */
    0,  /* Priority of the task */
    &Task1,  /* Task handle. */
    1); /* Core where the task should run */

  delay(3000);
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  z = g.gyro.z;
  stabilize = a.acceleration.x;
}
String readSteps() {
  return String(xRot);

}
String yawReturn()
{
  return String(zRot);
}

void loop() {

  ws.cleanupClients();
  //Serial.println(x);
  // Serial.print("Z is: ");
  // Serial.println(z);
  delay(100);
}



void Sensors(void * parameter) {
  for (;;) {
   

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);


    
  
    ax = a.acceleration.x;
    ay = a.acceleration.y;
    az = a.acceleration.z;
    gx =  g.gyro.x;
    gy =  g.gyro.y;
    gz =  g.gyro.z;
  deltat = fusion.deltatUpdate();
    fusion.MahonyUpdate(gx, gy, gz, ax, ay, az, deltat);
    
  roll = fusion.getRoll();
  pitch = fusion.getPitch();
  yaw = fusion.getYaw();
    
    
    xRot = pitch;
    zRot = yaw;
//    Serial.print(" Pitch: ");
//    Serial.print(xRot);
    Serial.print(" Y  aw : ");
    Serial.println(zRot);

  }
}
