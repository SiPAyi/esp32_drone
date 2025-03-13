#include <WebSocketsServer.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>

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
float p_gains[4] = {1.4, 1.4, 0, 0};
float i_gains[4] = {0.05, 0.05, 0, 0};
float d_gains[4] = {15, 15, 0, 0};
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

// timing related debugging
// int count = 0;

void setup() {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    Serial.begin(115200);
    Serial.println("(: Drone started :)");

    // **Step 1: Load Configurations**
    // load_configs_from_eeprom();

    // **Step 2: Motor Setup**
    setup_motors();
    
    // **Step 3: Wi-Fi Setup**
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
    
    // **Step 4: WebSocket Setup**
    Serial.println("WebSocket setup");
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

    // **Step 5: Gyro setup**
    gyro_setup();

    // **Step 6: Ready**
    Serial.println("Drone Ready!");
    blinkLED(5, 100);  // Fast blink 5 times -> indicating setup completed
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
      }
  }

  updateLEDIndicator();

  webSocket.loop();


  switch (mode){  //modes(0->STABLE, 1->GUIDED, 2->AUTO, 3->CALIB, 4-> MOTOR_TEST, 5->LAND, 6->EMERGENCY)
    case 0:{  // STABLE mode
    find_orientation();

    send_orientation();

      break;
    }
    
    case 1:{  // GUIDED mode
      find_orientation(); 
      
      // calculate PID
      pid();
      
      send_pid_status();

      // spin the motors
      spin_motors();
      break;
    }

    case 2:{  // AUTO mode
      find_orientation(); 
      send_orientation();
      // calculate PID
      // spin the motors
      break;
    }

    case 3:{  // CALIB mode
      // this work can be done in the websocket loop itself
      break;
    }

    case 4:{  // MOTOR_TEST mode
      find_orientation(); 
      send_orientation();
      spin_motors();
      break;
    }

    case 5:{  // LAND mode
      find_orientation(); 
      send_orientation();

      // Decrease altitude setpoint -> (setpoints[3]) by 1 and reset roll, pitch, yaw -> (setpoints[0], setpoints[1], and setpoints[2]) to 0
      setpoints[0] = 0;
      setpoints[1] = 0;
      setpoints[2] = 0;
      setpoints[3] = max((float)0, orientation[3] - 1);  // Ensure setpoints[3] doesn't go negative
    
      // calculate PID
      pid();
      // spin the motors
      spin_motors();

      break;
    }

    case 6:{  // EMERGENCY mode
      find_orientation(); 
      send_orientation();

      // Decrease altitude setpoint -> (setpoints[3]) by 1 and reset roll, pitch, yaw -> (setpoints[0], setpoints[1], and setpoints[2]) to 0
      setpoints[0] = 0;
      setpoints[1] = 0;
      setpoints[2] = 0;
      setpoints[3] = max((float)0, orientation[3] - 1);  // Ensure setpoints[3] doesn't go negative
    
      // calculate PID
      pid();
      // spin the motors
      spin_motors();
      break;
    }

    default:
      break;
  }

  while (micros() - prev_time < sampling_time);
  prev_time = micros();
  

// ////////////////////////////////////////////////////// 
// // timing related debugging
// ////////////////////////////////////////////////////////
//   unsigned long t = micros();

//   if (WiFi.status() != WL_CONNECTED) {
//       if (millis() - lastReconnectAttempt > reAttemptDuration) {
//           Serial.println("Wi-Fi Disconnected! Reconnecting...");
//           WiFi.reconnect();
//           lastReconnectAttempt = millis();
//       } else {
//         mode = 6; // changing the mode to EMERGENCY mode
//       }
//   }
//   Serial.print("wifi: "); Serial.print(micros() - t);
//   t = micros();

//   updateLEDIndicator();
//   Serial.print(" led: "); Serial.print(micros() - t);
//   t = micros();

//   webSocket.loop();
//   Serial.print(" socket: "); Serial.print(micros() - t);
//   t = micros();


//   find_orientation(); 
//   Serial.print(" gyro: "); Serial.print(micros() - t);
//   t = micros();


//   pid();
//   Serial.print(" pid: "); Serial.print(micros() - t);
//   t = micros();


//   send_pid_status();
//   Serial.print(" gyro_socket: "); Serial.print(micros() - t);
//   t = micros();


//   spin_motors();
//   Serial.print(" motors: "); Serial.print(micros() - t);
//   t = micros();


//   while (micros() - prev_time < sampling_time);
//   Serial.print(" while: "); Serial.print(micros() - t);

//   Serial.print(" iteration: "); Serial.println(micros() - prev_time);
//   prev_time = micros();
}