#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

// Motor Driver Control Pins
const int motor1Pin1 = 32; // In1 (Reassigned from 12 to avoid bootstrapping conflicts)
const int motor1Pin2 = 13; // In2
const int enable1Pin = 14; // ENA (Speed Left)

const int motor2Pin1 = 26; // In3
const int motor2Pin2 = 25; // In4
const int enable2Pin = 27; // ENB (Speed Right)

typedef struct struct_message {
    int speedA;
    int speedB;
    bool dirA;
    bool dirB;
} struct_message;

struct_message incomingData;
unsigned long lastRecvTime = 0; // Tracks the last received packet time for the failsafe

void driveMotors(int sA, int sB, bool dA, bool dB) {
  // Set Direction Motor A
  if(dA) {
    digitalWrite(motor1Pin1, HIGH);
    digitalWrite(motor1Pin2, LOW);
  } else {
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, HIGH);
  }

  // Set Direction Motor B
  if(dB) {
    digitalWrite(motor2Pin1, HIGH);
    digitalWrite(motor2Pin2, LOW);
  } else {
    digitalWrite(motor2Pin1, LOW);
    digitalWrite(motor2Pin2, HIGH);
  }

  // Apply dynamic PWM duty cycle values to the Enable lines
  analogWrite(enable1Pin, sA);
  analogWrite(enable2Pin, sB);
}

#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
void OnDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *incomingDataPtr, int len) {
#else
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingDataPtr, int len) {
#endif
  memcpy(&incomingData, incomingDataPtr, sizeof(incomingData));
  lastRecvTime = millis(); // Refresh last packet arrival timestamp
  
  // Directly write settings to the H-Bridge
  driveMotors(incomingData.speedA, incomingData.speedB, incomingData.dirA, incomingData.dirB);
}

void setup() {
  Serial.begin(115200);
  
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(enable1Pin, OUTPUT);
  
  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);
  pinMode(enable2Pin, OUTPUT);
  
  driveMotors(0, 0, true, true); // Safe startup fallback

  WiFi.mode(WIFI_STA);
  
  // Print local MAC Address so it can be easily configured in the controller
  Serial.println();
  Serial.print("ESP32 Car MAC Address: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}
 
void loop() {
  // Failsafe: Stop the motors if connection to controller is lost (no packet for >500ms)
  if (lastRecvTime > 0 && (millis() - lastRecvTime > 500)) {
    driveMotors(0, 0, true, true);
    Serial.println("Failsafe triggered: Connection lost. Stopping motors.");
    lastRecvTime = 0; // Prevent spamming log output while waiting to reconnect
  }
}