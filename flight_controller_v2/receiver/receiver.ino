#include <WebSocketsServer.h>
#include <WiFiManager.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <ArduinoJson.h>


#include <SoftwareSerial.h>
SoftwareSerial mySerial(D1, D2); // RX, TX (D1 to ESP32 TX, D2 to ESP32 RX)

#define LED_PIN LED_BUILTIN

////////////////// WiFi related things////////////////////////////
WiFiManager wifiManager;
unsigned long lastReconnectAttempt = 0;
unsigned long reAttemptDuration = 2000;


////////////////// WebSocket server on port 81 ////////////////////////////
WebSocketsServer webSocket = WebSocketsServer(81);
String ipStr = "192.168.188.251";


////////////////// Orientation related constants /////////////////////////////////////
float roll_b = 0, pitch_b = 0, yaw_b = 0, X_b = 0, Y_b = 0, Z_b = 0; // bias values for mpu6050
float roll_cf = 65.5, pitch_cf = 65.5, yaw_cf = 65.5, X_cf = 4096, Y_cf = 4096, Z_cf = 4096; // bias values for mpu6050

int mpu6050_status = 0;
int roll_index = 0, pitch_index = 1, yaw_index = 2, alt_index = 3;
int16_t raw_roll, raw_pitch, raw_yaw, raw_accX, raw_accY, raw_accZ;  // raw values

float orientation_from_gyro[3] = {0, 0, 0};
float orientation_from_acc[3] = {0, 0, 0};
float orientation[4] = {0, 0, 0, 0};

float KalmanAngleRoll=0, KalmanUncertaintyAngleRoll=2*2;
float KalmanAnglePitch=0, KalmanUncertaintyAnglePitch=2*2;


///////////////////////// PID related constants //////////////////////////////////
float p_gains[4] = {1, 0, 0, 0};
float i_gains[4] = {0, 0, 0, 0};
float d_gains[4] = {0, 0, 0, 0};
int setpoints[4] = {0, 0, 0, 0};
float errors[4] = {0, 0, 0, 0};
float prev_errors[4] = {0, 0, 0, 0};
float error_sum[4] = {0, 0, 0, 0};
int pid_out[4] = {0, 0, 0, 0};
int base_throttle = 1200;

//////////////////////// ESC related /////////////////////////////////////
// some settings
int order_of_motors[4] = {0, 1, 2, 3}; // order of the motors -> {top_left, top_right, bottom_left, bottom_right}
int motor_min_speed = 1000;
int motor_max_speed = 2000;
int speed_diff = motor_max_speed - motor_min_speed;
int max_values[4] = {400, 400, 400, speed_diff}; // roll, pitch, yaw, altitude

// run time variables
int motor_states[4] = {0, 0, 0, 0};
int motors_speed[4] = {motor_min_speed, motor_min_speed, motor_min_speed, motor_min_speed};

/////////////////////// execution related //////////////////////
int sampling_time = 4000; // microseconds
float dt = sampling_time / 1000000.0;  // Correct conversion to seconds
unsigned long prev_time = micros();
int mode = 10;//10->setup 0->standby, 1->motor_calib, 2->imu_calib, 3->guided, 4-> auto

void setup() {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    Serial.begin(115200);
    Serial.println("(: Drone started :)");

    // communication
    mySerial.begin(57600); // UART to ESP32
    
    // wifi setup
    Serial.println("Wi-Fi setup");
    wifiManager.autoConnect("ESP32_Setup");
    Serial.println("Connected to Wi-Fi!");
    digitalWrite(LED_PIN, HIGH); // Solid ON for Wi-Fi connected
    IPAddress localIP = WiFi.localIP(); // Store ESP32's IP address
    ipStr = localIP.toString();

    // Start mDNS responder
    if (!MDNS.begin("esp32")) {
        Serial.println("Error setting up MDNS responder!");
        return;
    }
    Serial.println("mDNS responder started. You can access it via http://esp32.local/");
    
    // WebSocket Setup
    Serial.println("WebSocket setup");
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

    // Ready
    Serial.println("Drone Ready!");
    digitalWrite(LED_PIN, HIGH); // Solid ON for Wi-Fi connected

    mode = 0;
}

void loop() {
    // Check WiFi status
  if (WiFi.status() != WL_CONNECTED) {
      if (millis() - lastReconnectAttempt > reAttemptDuration) {
          Serial.println("Wi-Fi Disconnected! Reconnecting...");
          WiFi.reconnect();
          lastReconnectAttempt = millis();
      } else {
        mode = 6; // changing the mode to EMERGENCY mode
        send_data();
      }
  }


  webSocket.loop();
  send_orientation();
}

void send_data(){
      // Format data into a string
    String data = String(mode) + "," + String(base_throttle) + "," +
                  String(p_gains[roll_index], 3) + "," + String(i_gains[roll_index], 3) + "," + String(d_gains[roll_index], 3) + "," +
                  String(p_gains[pitch_index], 3) + "," + String(i_gains[pitch_index], 3) + "," + String(d_gains[pitch_index], 3);

    mySerial.println(data);  // Send the encoded data
    Serial.println("Sent: " + data); // Debugging output
    
    // delay(1000); // Send every second
}