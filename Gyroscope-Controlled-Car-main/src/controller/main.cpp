#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// HARDCODED RECEIVER MAC ADDRESS
uint8_t receiverAddress[] = {0x28, 0x56, 0x2F, 0x77, 0x9F, 0x5C}; 

Adafruit_MPU6050 mpu;
#define LED_PIN 2

typedef struct struct_message {
    int speedA;      
    int speedB;      
    bool dirA;       
    bool dirB;       
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) { 
      digitalWrite(LED_PIN, HIGH); delay(100);
      digitalWrite(LED_PIN, LOW); delay(100);
    }
  }
  
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  delay(1000); 

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  // Flash onboard LED to signify calibration/connection ready
  for(int i=0; i<5; i++) {
    digitalWrite(LED_PIN, HIGH); delay(100);
    digitalWrite(LED_PIN, LOW); delay(100);
  }
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float ax = a.acceleration.x; 
  float ay = a.acceleration.y; 

  int targetSpeed = 0;
  float threshold = 1.5; 

  // Default: Stopped
  myData.speedA = 0;
  myData.speedB = 0;
  myData.dirA = true;
  myData.dirB = true;

  if (abs(ax) > threshold && abs(ax) >= abs(ay)) {
    targetSpeed = map(constrain(abs(ax), threshold, 8.0) * 100, threshold * 100, 800, 100, 255);
    
    if (ax > 0) { // Forward tilt
      myData.speedA = targetSpeed;
      myData.speedB = targetSpeed;
      myData.dirA = true;
      myData.dirB = true;
    } else { // Backward tilt
      myData.speedA = targetSpeed;
      myData.speedB = targetSpeed;
      myData.dirA = false;
      myData.dirB = false;
    }
  } 
  else if (abs(ay) > threshold && abs(ay) > abs(ax)) {
    targetSpeed = map(constrain(abs(ay), threshold, 8.0) * 100, threshold * 100, 800, 100, 255);
    
    if (ay > 0) { // Right tilt -> Spin Right
      myData.speedA = targetSpeed;
      myData.speedB = targetSpeed;
      myData.dirA = true;
      myData.dirB = false;
    } else { // Left tilt -> Spin Left
      myData.speedA = targetSpeed;
      myData.speedB = targetSpeed;
      myData.dirA = false;
      myData.dirB = true;
    }
  }

  esp_now_send(receiverAddress, (uint8_t *) &myData, sizeof(myData));
  delay(30); 
}