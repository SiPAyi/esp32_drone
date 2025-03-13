#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <Wire.h>
#include <ArduinoJson.h>

////////////////// WiFi credentials (SSID and Password pairs) ////////////////////////////
const char* wifiNetworks[][2] = {
    {"Saik", "........"},
    {"AndroidShare_D5", ""},  // No password
    {"another network", "its password"}
};

const int numNetworks = sizeof(wifiNetworks) / sizeof(wifiNetworks[0]);
bool isReconnecting = false;
unsigned long reconnectStartTime = 0;
const unsigned long reconnectInterval = 10000; // Try reconnecting every 10 sec


////////////////// WebSocket server on port 81 ////////////////////////////
WebSocketsServer webSocket = WebSocketsServer(81);


////////////////// Orientation related constants /////////////////////////////////////
float roll_b = 0, pitch_b = 0, yaw_b = 0, X_b = 0, Y_b = 0, Z_b = 0; // bias values for mpu6050
float roll_cf = 1, pitch_cf = 1, yaw_cf = 1, X_cf = 0, Y_cf = 0, Z_cf = 0; // bias values for mpu6050

int mpu6050_status = 0;
int roll_index = 0, pitch_index = 1, yaw_index = 2, alt_index = 3;
int16_t raw_roll, raw_pitch, raw_yaw, raw_accX, raw_accY, raw_accZ;  // raw values

float orientation_from_gyro[3] = {0, 0, 0};
float orientation_from_acc[3] = {0, 0, 0};
float orientation[4] = {0, 0, 0, 0};


///////////////////////// PID related constants //////////////////////////////////
float p_gains[4] = {0, 0, 0, 0};
float i_gains[4] = {0, 0, 0, 0};
float d_gains[4] = {0, 0, 0, 0};
float setpoints[4] = {0, 0, 0, 0};
float prev_errors[4] = {0, 0, 0, 0};
float error_sum[4] = {0, 0, 0, 0};
int pid_out[4] = {0, 0, 0, 0};

//////////////////////// ESC related /////////////////////////////////////
// some settings
int order_of_motors[4] = {1, 2, 3, 4}; // order of the motors -> {top_left, top_right, bottom_left, bottom_right}
int motor_min_speed = 1000;
int motor_max_speed = 2000;
int max_values[3] = {0, 0, 0}; // roll, pitch, yaw

// run time variables
int motor_states[4] = {0, 0, 0, 0};
int motors_speed = 1000;


/////////////////////// execution related //////////////////////
int time_step = 4;
double prev_time = millis();
int count = 0;
int mode = 0;// 0->standby, 1->motor_calib, 2->imu_calib, 3->guided, 4-> auto


void setup() {
// Serial monitor setup
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println("(: Drone started :)");

// wifi setup
  while (!connectToWiFi()) {
    Serial.println("Retrying WiFi connection...");
    delay(5000); // Wait 5 sec before retrying
  }

// websocket setup
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  // Check WiFi status
  if (WiFi.status() != WL_CONNECTED) {
      if (!isReconnecting) {
          Serial.println("WiFi lost! Initiating emergency landing...");
          landDrone(); // Start the landing sequence
          isReconnecting = true;
          reconnectStartTime = millis(); // Record time to retry connection later
      }
      // Non-blocking reconnection attempt at intervals
      else if (millis() - reconnectStartTime >= reconnectInterval) {
          hover_at_this_pose();
          Serial.println("Attempting to reconnect to WiFi...");
          connectToWiFi();
          reconnectStartTime = millis(); // Reset reconnect timer
      }
      return;
  }

  webSocket.loop();

  switch (mode){  //modes(0->standby, 1->guided, 2->auto, 3->calibration, 4-> motor_testing, 5->landing)
    case 0:{  // standby mode
      // stop spinning the motors
      for (int i=0; i<=4; i++){
        spin_motor(i, motor_min_speed);
      }
      find_orientation();
      send_orientation();

      break;
    }
    
    case 1:{  // guided mode
      find_orientation(); 
      send_orientation();
      // calculate PID
      // spin the motors
      break;
    }

    case 2:{  // auto mode
      find_orientation(); 
      send_orientation();
      // calculate PID
      // spin the motors
      break;
    }

    case 3:{  // calibration mode
      // this work can be done in the websocket loop itself
      break;
    }

    case 4:{  // motor_testing mode
      // spin motors
      for (int i=0; i<=4; i++){
        spin_motor(i, motors_speed);
      }
      break;
    }

    case 5:{  // landing mode
      Serial.println("add logic to land the drone");
      break;
    }

    default:
      break;
  }

}

//  
// // websockets related
// 
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:{
      Serial.printf("Client %u connected\n", num);
      break;}

    case WStype_DISCONNECTED:{
      Serial.printf("Client %u disconnected\n", num);
      break;}

    case WStype_TEXT:{
      // Parse incoming PID values (JSON format)
      String msg = String((char*)payload);
      handle_msg(msg);
      break;}

    case WStype_BIN:{
      Serial.printf("Binary message from client %u, length: %u\n", num, length);
      break;}

    default:
      Serial.printf("Unhandled event type %u from client %u\n", type, num);
      break;
  }
}

void handle_msg(String msg){
  // Serial.print("Message from client:");
  // Serial.println(msg);
  // Serial.printf("Message from client %u: %s\n", num, msg);

  //msg -> message id   : parameters
  if (msg.indexOf("id") >= 0) {
    int msg_id = parseJsonValue(msg, "id");
    Serial.println(); // for new line between two messages
    switch (msg_id) {
      case -1:{ // -1(drone<-GCS): get_mode
        Serial.println("message ID -1(drone<-GCS): get_mode");

        // Convert variables to JSON format
        StaticJsonDocument<200> jsonDoc;  // Create a JSON document
        jsonDoc["id"] = 0;              // Add id
        jsonDoc["mode"] = mode;              // Add mode
        // Serialize the JSON document to a string
        String jsonString;
        serializeJson(jsonDoc, jsonString);
        webSocket.broadcastTXT(jsonString); // Broadcast data to all connected clients
        
        Serial.print("Sent drone current mode");
        Serial.println(mode);
        break;
      }

      case 0:{ // 0(drone<-GCS): set_modes{mode(0->standby, 1->guided, 2->auto, 3->calibration, 4-> motor_testing, 5->landing)}
        Serial.println("message ID 0(drone<-GCS): set_modes{mode(0->standby, 1->motor_calib, 2->imu_calib, 3->guided, 4-> auto, 5->motor_testing)}");

        mode = parseJsonValue(msg, "mode");
        String modes[] = {"standby", "guided", "auto", "calibration", "motor_testing", "landing"};
        
        Serial.printf("Mode %d: %s\n", mode, modes[mode].c_str());
        break;
        }

      case 1:{ // 1(drone<>GCS): get_PIDs
        Serial.println("message ID 1(drone<>GCS): get_PIDs");

        // Convert variables to JSON format
        StaticJsonDocument<200> jsonDoc;  // Create a JSON document
        jsonDoc["id"] = 2;              // Add id
        jsonDoc["save"] = 1;              // save the values to permenant storage or not
        jsonDoc["rollP"] = p_gains[roll_index];        // Add rollP
        jsonDoc["rollI"] = i_gains[roll_index];        // Add rollI
        jsonDoc["rollD"] = d_gains[roll_index];        // Add rollD
        jsonDoc["pitchP"] = p_gains[pitch_index];        // Add pitchP
        jsonDoc["pitchI"] = i_gains[pitch_index];        // Add pitchI
        jsonDoc["pitchD"] = d_gains[pitch_index];        // Add pitchD
        jsonDoc["yawP"] = p_gains[yaw_index];        // Add yawP
        jsonDoc["yawI"] = i_gains[yaw_index];        // Add yawI
        jsonDoc["yawD"] = d_gains[yaw_index];        // Add yawD
        // Serialize the JSON document to a string
        String jsonString;
        serializeJson(jsonDoc, jsonString);       
        webSocket.broadcastTXT(jsonString); // Broadcast data to all connected clients
        
        Serial.println("Sent PID values");
        break;
        }

      case 2:{ // 2(drone<>GCS): set_PIDs{save, proll, iroll, droll, ppitch, ipitch, dpitch, pyaw, iyaw, dyaw}
        Serial.println("message ID 2(drone<>GCS): set_PIDs{save, proll, iroll, droll, ppitch, ipitch, dpitch, pyaw, iyaw, dyaw}");
        
        int save = parseJsonValue(msg, "save");
        p_gains[roll_index] = parseJsonValue(msg, "rollP");
        i_gains[roll_index] = parseJsonValue(msg, "rollI");
        d_gains[roll_index] = parseJsonValue(msg, "rollD");
        p_gains[pitch_index] = parseJsonValue(msg, "pitchP");
        i_gains[pitch_index] = parseJsonValue(msg, "pitchI");
        d_gains[pitch_index] = parseJsonValue(msg, "pitchD");
        p_gains[yaw_index] = parseJsonValue(msg, "yawP");
        i_gains[yaw_index] = parseJsonValue(msg, "yawI");
        d_gains[yaw_index] = parseJsonValue(msg, "yawD");        
        Serial.printf("Updated PID: Roll(%f, %f, %f), Pitch(%f, %f, %f), Yaw(%f, %f, %f)\n", p_gains[roll_index], i_gains[roll_index], d_gains[roll_index], p_gains[pitch_index], i_gains[pitch_index], d_gains[pitch_index], p_gains[yaw_index], i_gains[yaw_index], d_gains[yaw_index]);

        if(save == 1){
          savePIDToEEPROM();
        }else{
          Serial.println("Updated the PID values");
        }
        break;
      }

      case 3:{ // 3(drone<>GCS): get_motors_order
        Serial.println("message ID 3(drone<>GCS): get_motors_order");

        // Convert variables to JSON format
        StaticJsonDocument<200> jsonDoc;  // Create a JSON document
        jsonDoc["id"] = 4;              // Add id
        jsonDoc["save"] = 1;              // Add id
        jsonDoc["top_left"] = order_of_motors[0];
        jsonDoc["top_right"] = order_of_motors[1];
        jsonDoc["bottom_left"] = order_of_motors[2];
        jsonDoc["bottom_right"] = order_of_motors[3];
        // Serialize the JSON document to a string
        String jsonString;
        serializeJson(jsonDoc, jsonString);
        webSocket.broadcastTXT(jsonString); // Broadcast data to all connected clients
        
        Serial.println("Motors order sent");
        break;
        }

      case 4: {// 4(drone<-GCS): set_motors_order{top_left, top_right, bottom_left, bottom_right}
        Serial.println("message ID 4(drone<-GCS): set_motors_order{top_left, top_right, bottom_left, bottom_right}");
        
        int save = parseJsonValue(msg, "save");
        order_of_motors[0] = parseJsonValue(msg, "top_left");
        order_of_motors[1] = parseJsonValue(msg, "top_right");
        order_of_motors[2] = parseJsonValue(msg, "bottom_left");
        order_of_motors[3] = parseJsonValue(msg, "bottom_right");
        setup_motors();
        Serial.print("motors_order edited to: ");
        for (int i = 0; i < 4; i++) {
            Serial.print(order_of_motors[i]);
            if (i < 3) Serial.print(", "); // Add a comma except after the last element
        }
        Serial.println();

        if(save){
          saveMotorOrderToEEPROM();
        }else{
          Serial.println("Motors order updated");
        }
        break;
      }

      case 5:{ // 5(drone<-GCS): motor_test{m1_state, m2_state, m3_state, m4_state, speed}
        Serial.println("message ID 5(drone<-GCS): motor_test{m1_state, m2_state, m3_state, m4_state, speed}");
       
        motor_states[0] = parseJsonValue(msg, "m1_state");
        motor_states[1] = parseJsonValue(msg, "m2_state");
        motor_states[2] = parseJsonValue(msg, "m3_state");
        motor_states[3] = parseJsonValue(msg, "m4_state");
        motors_speed = parseJsonValue(msg, "speed");

        Serial.println("Motors states updated");
        break;
        }

      case 6:{ // 6(drone<>GCS): get_settings
        Serial.println("message ID 6(drone<>GCS): get_settings");

        // Convert variables to JSON format
        StaticJsonDocument<200> jsonDoc;  // Create a JSON document
        jsonDoc["id"] = 7;              // Add id
        jsonDoc["save"] = 1;              // Add id
        jsonDoc["max_roll"] = max_values[0];
        jsonDoc["max_pitch"] = max_values[1];
        jsonDoc["max_yaw"] = max_values[2];
        jsonDoc["min_speed"] = motor_min_speed;
        jsonDoc["max_speed"] = motor_max_speed;
        // Serialize the JSON document to a string
        String jsonString;
        serializeJson(jsonDoc, jsonString);
        webSocket.broadcastTXT(jsonString); // Broadcast data to all connected clients

        Serial.println("Settings sent to GCS");
        break;
      }    

      case 7:{ // 7(drone<>GCS): set_settings{max_roll, max_pitch, max_yaw, min_speed, max_speed}
        Serial.println("message ID 7(drone<>GCS): set_settings{max_roll, max_pitch, max_yaw, min_speed, max_speed}");

        int save_or_not = parseJsonValue(msg, "save");
        max_values[0] = parseJsonValue(msg, "max_roll");
        max_values[1] = parseJsonValue(msg, "max_pitch");
        max_values[2] = parseJsonValue(msg, "max_yaw");
        motor_min_speed = parseJsonValue(msg, "min_speed");
        motor_max_speed = parseJsonValue(msg, "max_speed");

        if(save_or_not){
          saveMaxValuesToEEPROM();
        }else{
          Serial.println("Settings Updated");
        }
        break;
      }

      case 8:{// 8(drone<-GCS): imu_calib_command{step}
        Serial.println("message ID 8(drone<-GCS): imu_calib_command{step}");
        
        int step = parseJsonValue(msg, "step");
        int calibration_status = imu_calibration(step);

        // Convert variables to JSON format
        StaticJsonDocument<200> jsonDoc;  // Create a JSON document
        jsonDoc["id"] = 9;              // Add id
        jsonDoc["step"] = step;              // Add id
        jsonDoc["response"] = calibration_status;
        // Serialize the JSON document to a string
        String jsonString;
        serializeJson(jsonDoc, jsonString);
        webSocket.broadcastTXT(jsonString); // Broadcast data to all connected clients
        
        if(calibration_status){
          Serial.printf("step %d completed\n", step);
        }else{
          Serial.printf("step %d failed\n", step);
        }
        break;
      }

      case 9:{// 9(drone->GCS): imu_calib_response{step, response}
        Serial.println("message ID 9(drone->GCS): imu_calib_response{step, response}");
        Serial.println("this step is only on GCS");
        break;
      }

      case 10:{ // 10(drone<-GCS): get_orientation
        Serial.println("message ID 10(drone<-GCS): get_orientation");

        send_orientation();

        Serial.println("orientation data sent");
        break;}

      case 11:{ // 11(drone->GCS): drone_orientation{roll, pitch, yaw}
        Serial.println("message ID 11(drone->GCS): drone_orientation{roll, pitch, yaw}");
        Serial.println("this step is only on GCS side (drone->GCS)");
        break;}

      case 12:{ // 12(drone<-GCS): setpoints{roll, pitch, yaw, altitude}
        Serial.println("message ID 12(drone<-GCS): setpoints{roll, pitch, yaw, altitude}");
        
        setpoints[0] = parseJsonValue(msg, "roll");
        setpoints[1] = parseJsonValue(msg, "pitch");
        setpoints[2] = parseJsonValue(msg, "yaw");
        setpoints[3] = parseJsonValue(msg, "altitude");
        Serial.printf("setpoints {%f, %f, %f, %f}", setpoints[0],setpoints[1],setpoints[2],setpoints[3]);

        Serial.print("Setpoints updated");
        break;
      }

      case 13:{// 13(drone<-GCS): motor_calib_command{step}
        Serial.println("message ID 13(drone<-GCS): motor_calib_command{step}");
        
        int step = parseJsonValue(msg, "step");
        int calibration_status = motor_calibration(step);

        // Convert variables to JSON format
        StaticJsonDocument<200> jsonDoc;  // Create a JSON document
        jsonDoc["id"] = 14;              // Add id
        jsonDoc["step"] = step;              // Add id
        jsonDoc["response"] = calibration_status;
        // Serialize the JSON document to a string
        String jsonString;
        serializeJson(jsonDoc, jsonString);
        webSocket.broadcastTXT(jsonString); // Broadcast data to all connected clients
        
        if(calibration_status){
          Serial.printf("step %d completed\n", step);
        }else{
          Serial.printf("step %d failed\n", step);
        }
        break;
      }

      case 14:{// 14(drone->GCS): motor_calib_response{step, response}
        Serial.println("message ID 14(drone->GCS): motor_calib_response{step, response}");
        Serial.println("this step is only on GCS");
        break;
      }


      default:
        break;
  }
  }
}

// Helper function to extract values from JSON. Example JSON: {"id":0, "rollP":1.0,"rollI":0.1,"rollD":0.01}
float parseJsonValue(String json, String key) {
    DynamicJsonDocument doc(1024); // Can be larger or even estimated dynamically
    DeserializationError error = deserializeJson(doc, json);
    
    if (error) {
        Serial.print("Deserialization failed: ");
        Serial.println(error.c_str());
        return 0.0; // Handle error case
    }

    return doc[key]; // Assumes the key exists and holds a float value
}

void send_orientation() {
  // Ensure WebSocket is initialized before sending data
  if (!webSocket.connectedClients()) {
    return;  // Exit if no clients are connected
  }

  // Create a JSON document with increased size for float values
  StaticJsonDocument<256> jsonDoc;

  // Add data to JSON
  jsonDoc["id"] = 11; // Message identifier
  jsonDoc["timestamp"] = millis();  // Add current time in milliseconds
  jsonDoc["roll"] = orientation[roll_index];       
  jsonDoc["pitch"] = orientation[pitch_index];       
  jsonDoc["yaw"] = orientation[yaw_index];       
  jsonDoc["altitude"] = orientation[alt_index];  // Add altitude data

  // Serialize JSON to string
  String jsonString;
  serializeJson(jsonDoc, jsonString);
  webSocket.broadcastTXT(jsonString); // Broadcast data to all connected clients
}

// wifi related things
int connectToWiFi() {
    WiFi.mode(WIFI_STA);
    for (int i = 0; i < numNetworks; i++) {
        Serial.print("Trying to connect to: ");
        Serial.println(wifiNetworks[i][0]);

        if (strlen(wifiNetworks[i][1]) > 0) {
            WiFi.begin(wifiNetworks[i][0], wifiNetworks[i][1]); // With password
        } else {
            WiFi.begin(wifiNetworks[i][0]); // Open network (no password)
        }

        // Give WiFi time to connect asynchronously
        unsigned long startAttemptTime = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 5000) {
            delay(100); // Short delay to avoid CPU overuse
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.print("\nConnected to WiFi: ");
            Serial.println(wifiNetworks[i][0]); // Print connected WiFi name
            Serial.print("IP Address: ");
            Serial.println(WiFi.localIP());
            isReconnecting = false; // Stop reconnect attempts
            return 1; // Exit function on success
        }
    }
    Serial.println("Could not connect to any WiFi network, will retry...");
    return 0;
}


// =========================== functions from another files =================================
void landDrone() {
    Serial.println("Landing the drone safely...");
    // Add your drone landing logic here (e.g., reduce throttle gradually)
}

void hover_at_this_pose(){
    Serial.println("hovering the drone here...");
    // add logic to hover the drone at the current point
}

void find_orientation() {
  // replace it with the correct code
  orientation[roll_index] = random(-180, 181);  // Random value between -180 and 180
  orientation[pitch_index] = random(-90, 91);   // Random value between -90 and 90
  orientation[yaw_index] = random(-180, 181);   // Random value between -180 and 180
  orientation[alt_index] = random(0, 10); 
}

int imu_calibration(int step){
  Serial.println("need to add the IMU calibration code here");
  return 0;
}

int motor_calibration(int step){
  Serial.println("need to add the motor calibration code here");
  return 0;
}

void savePIDToEEPROM(){
  Serial.println("add code to save pid values to eeprom");
}

void saveMotorOrderToEEPROM(){
  Serial.println("add code to save Motor Order to eeprom");
}

void saveMaxValuesToEEPROM(){
  Serial.println("add code to save Max Values to eeprom");
}

void setup_motors(){
  Serial.println("attach the esc to the motors");
}

void spin_motor(int motor_number, int speed){
  // motor_number(1->top_left, 2->top_right, 3->bottom_left, 4->bottom_right)
  Serial.printf("add logic for spinning the motor %d with speed %d", motor_number, speed);
}