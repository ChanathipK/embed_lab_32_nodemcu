#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>
#include <WiFiClientSecure.h>

// Define custom RX and TX pins for SoftwareSerial
#define RX_PIN D1
#define TX_PIN D2

// Create SoftwareSerial object
SoftwareSerial mySerial(RX_PIN, TX_PIN);
const char* ssid = "cocoa_ultima_e";
const char* password = "MOSDONGO";
const char* serverName = "embbed-lab-survivor-project.vercel.app"; // Example endpoint
BearSSL::WiFiClientSecure secureClient;
HTTPClient http;
unsigned long lastConnectionTime = 0;  
unsigned long postingInterval = 5000;
String sensorData;
String payload;

int check_int(String data){
  for (int i=0;i<data.length();i++){
    if (!isdigit(data[i]) && !isspace(data[i])){
      return 0;
      }
  }
  return 1;
}

String format_data(String receivedData){
  int sensor1Start = receivedData.indexOf("1: ");
  int sensor2Start = receivedData.indexOf("2: ");
  if (sensor1Start > 0){
    sensorData = receivedData.substring(sensor1Start+3);
    if (check_int(sensorData)){
      payload = "{\"sensorId\": \"1\",\"value\": " + sensorData + "}";
    }
    else{
      payload = "None";
      Serial.println("SENSOR NOT INT "+sensorData);
    }
  }
  else if (sensor2Start > 0){
    sensorData = receivedData.substring(sensor2Start+3);
    if (check_int(sensorData)){
     payload = "{\"sensorId\": \"2\",\"value\": " + sensorData + "}";
    }
    else{
      payload = "None";
      Serial.println("SENSOR NOT INT "+sensorData);
    }
  }
  else {
    payload = "None";
  }

  return payload;
}
void real_send_post_request(String payload) {
  secureClient.stop();
  secureClient.setInsecure();

  if (secureClient.connect(serverName, 443)) {

    Serial.println("connecting...");

    secureClient.println("POST /api/data HTTP/1.1");
    secureClient.println("Host: embbed-lab-survivor-project.vercel.app");
    secureClient.println("User-Agent: Arduino/1.0");
    secureClient.println("Connection: close");
    secureClient.println("Content-Type: application/json");
    secureClient.print("Content-Length: ");
    secureClient.println(payload.length());
    secureClient.println();
    secureClient.println(payload);
    lastConnectionTime = millis();

  } else {
    Serial.println("connection failed");
  }
}

void send_post_request(String payload){
  if (WiFi.status() == WL_CONNECTED) {
      real_send_post_request(payload);
      while (secureClient.available()) {
        char c = secureClient.read();
        Serial.print(c);
      }
      Serial.println("Success SENDING POST");
      if (!secureClient.available()){
        Serial.println("DISCONNECT");
      }
    } else {
      Serial.println("WiFi Disconnected");
    }
}



void setup() {
  // // Start the I2C bus
  Serial.begin(115200);
  mySerial.begin(115200);
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  String receivedData;
  mySerial.println("__________________");
  // Parse the received data
  if (mySerial.available()) {
    Serial.println("");
    receivedData = mySerial.readStringUntil('\n');
    payload = format_data(receivedData);
    if (payload != "None"){
      send_post_request(payload);
      Serial.println("Send data: " + payload);
    } else{
      Serial.println("Send ERROR data: " + receivedData);
      Serial.println("Send ERROR payload: " + payload);
    }
  }
  else{
    Serial.print(".");
    // delay(100);
  }
}