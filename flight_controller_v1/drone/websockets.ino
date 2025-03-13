//  
// // websockets related
// 
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:{
      Serial.printf("Client %u connected\n", num);
      // send the ip address to the client
      // Convert variables to JSON format
      StaticJsonDocument<200> jsonDoc;  // Create a JSON document
      jsonDoc["ip"] = ipStr;

      // Serialize the JSON document to a string
      String jsonString;
      serializeJson(jsonDoc, jsonString);
      webSocket.broadcastTXT(jsonString); // Broadcast data to all connected clients
      
      Serial.println("sent IP address");
      break;}

    case WStype_DISCONNECTED:{
      Serial.printf("Client %u disconnected\n", num);
      break;}

    case WStype_TEXT:{
      // Parse incoming PID values (JSON format)
      String msg = String((char*)payload);
      if (msg == "ping") {
        webSocket.broadcastTXT("pong");
      }else{
        handle_msg(msg);
      }
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

      case 0:{ // 0(drone<-GCS): set_modes{mode(0->STABLE, 1->GUIDED, 2->AUTO, 3->CALIB, 4-> MOTOR_TEST, 5->LAND, 6->EMERGENCY)}
        Serial.println("message ID 0(drone<-GCS): set_modes{(0->STABLE, 1->GUIDED, 2->AUTO, 3->CALIB, 4-> MOTOR_TEST, 5->LAND)}");

        mode = parseJsonValue(msg, "mode");
        String modes[] = {"standby", "guided", "auto", "calibration", "motor_testing", "landing"};
        
        if (mode == 0 || mode == 3 || mode == 4){
          disarm();
        }else if(mode == 1 || mode == 2 || mode == 5){
          arm();
        }

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
        jsonDoc["base_throttle"] = base_throttle;        // Add yawD
        // Serialize the JSON document to a string
        String jsonString;
        serializeJson(jsonDoc, jsonString);       
        webSocket.broadcastTXT(jsonString); // Broadcast data to all connected clients
        
        Serial.println("Sent PID values");
        break;
        }

      case 2:{ // 2(drone<>GCS): set_PIDs{save, proll, iroll, droll, ppitch, ipitch, dpitch, pyaw, iyaw, dyaw, base_throttle}
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
        base_throttle = parseJsonValue(msg, "base_throttle");

        Serial.printf("set_PIDs{%d, %f, %f, %f, %f, %f, %f, %f, %f, %f, %d}\n", save, p_gains[roll_index], i_gains[roll_index], d_gains[roll_index], p_gains[pitch_index], i_gains[pitch_index], d_gains[pitch_index], p_gains[yaw_index], i_gains[yaw_index], d_gains[yaw_index], base_throttle);

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
        reconfig_motors();

        Serial.println("motors_order edited to: ");
        Serial.printf("set_motors_order{%d, %d, %d, %d}\n", order_of_motors[0], order_of_motors[1], order_of_motors[2], order_of_motors[3]);

        if(save){
          saveMotorOrderToEEPROM();
        }else{
          Serial.println("Motors order updated");
        }
        break;
      }

      case 5:{ // 5(drone<-GCS): motor_test{m1_state, m2_state, m3_state, m4_state, speed}
        Serial.println("message ID 5(drone<-GCS): motor_test{m1_state, m2_state, m3_state, m4_state, speed}");
        Serial.println(msg);
        motor_states[0] = parseJsonValue(msg, "m1_state");
        motor_states[1] = parseJsonValue(msg, "m2_state");
        motor_states[2] = parseJsonValue(msg, "m3_state");
        motor_states[3] = parseJsonValue(msg, "m4_state");
        int test_speed = parseJsonValue(msg, "speed");

        motors_speed[0] = test_speed;
        motors_speed[1] = test_speed;
        motors_speed[2] = test_speed;
        motors_speed[3] = test_speed;
        
        Serial.printf("motor_test{%d, %d, %d, %d, %d}\n", motor_states[0], motor_states[1], motor_states[2], motor_states[3], test_speed);
        Serial.println("Motors states updated");

        // spin motors
      for (int i=0; i<4; i++){
        motors_speed[i] = motor_states[i] ? constrain(motors_speed[i], motor_min_speed, motor_max_speed) : motor_min_speed;
        
        switch (i) {
            case 0: motor_tl.writeMicroseconds(motors_speed[i]); break;
            case 1: motor_tr.writeMicroseconds(motors_speed[i]); break;
            case 2: motor_bl.writeMicroseconds(motors_speed[i]); break;
            case 3: motor_br.writeMicroseconds(motors_speed[i]); break;
            default: Serial.printf("Invalid motor number: %d\n", i); return;
        }

        Serial.print(" \t motor");
        Serial.print(i);
        Serial.print("-> ");
        Serial.print(motors_speed[i]);
      }
      Serial.println();
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

        Serial.printf("set_settings{%d, %d, %d, %d, %d}\n", max_values[0], max_values[1], max_values[2], motor_min_speed, motor_min_speed);
        if(save_or_not){
          saveSettingsToEEPROM();
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
        Serial.println("message ID 11(drone->GCS): drone_orientation{roll, pitch, yaw, altitude}");
        Serial.println("this step is only on GCS side (drone->GCS)");
        break;}

      case 12:{ // 12(drone<-GCS): setpoints{roll, pitch, yaw, altitude}
        Serial.println("message ID 12(drone<-GCS): setpoints{roll, pitch, yaw, altitude}");
        Serial.println(msg);
        setpoints[0] = parseJsonValue(msg, "roll");
        setpoints[1] = parseJsonValue(msg, "pitch");
        setpoints[2] = parseJsonValue(msg, "yaw");
        setpoints[3] = parseJsonValue(msg, "altitude");
        Serial.printf("setpoints {%d, %d, %d, %d}\n", setpoints[0],setpoints[1],setpoints[2],setpoints[3]);

        Serial.println("Setpoints updated");
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

      case 15:{// 15(drone<-GCS): get_imu_offsets
        Serial.println("message ID 15(drone<-GCS): get_imu_offsets");
        
        // Convert variables to JSON format
        StaticJsonDocument<200> jsonDoc;  // Create a JSON document
        jsonDoc["id"] = 16;              // Add id
        jsonDoc["roll_offset"] = roll_b;
        jsonDoc["pitch_offset"] = pitch_b;
        jsonDoc["yaw_offset"] = yaw_b;
        jsonDoc["accx_offset"] = X_b;
        jsonDoc["accy_offset"] = Y_b;
        jsonDoc["accz_offset"] = X_b;
        jsonDoc["roll_scalling"] = roll_cf;
        jsonDoc["pitch_scalling"] = pitch_cf;
        jsonDoc["yaw_scalling"] = yaw_cf;
        jsonDoc["accx_scalling"] = X_cf;
        jsonDoc["accy_scalling"] = Y_cf;
        jsonDoc["accz_scalling"] = Z_cf;
        // Serialize the JSON document to a string
        String jsonString;
        serializeJson(jsonDoc, jsonString);
        webSocket.broadcastTXT(jsonString); // Broadcast data to all connected clients
        
        Serial.println("sent bias values and calibration factors");
      }

      case 16:{ // 16(drone->GCS): set_imu_offsets{roll_offset,pitch_offset,yaw_offset,accx_offset,accy_offset,accz_offset, roll_scalling, pitch_scalling, yaw_scalling, accx_scalling, accy_scalling, accz_scalling}
        Serial.println("message ID 16(drone->GCS): set_imu_offsets{roll_offset,pitch_offset,yaw_offset,accx_offset,accy_offset,accz_offset, roll_scalling, pitch_scalling, yaw_scalling, accx_scalling, accy_scalling, accz_scalling}");
        Serial.println("this step is only on GCS");
      }

      case 17:{// 17(drone->GCS): pid_status{error1, error2, error3, error4, pid_out1, pid_out2, pid_out3, pid_out4, motor_speed1, motor_speed2, motor_speed3, motor_speed4}
        Serial.println("message ID 17(drone->GCS): pid_status{error1, error2, error3, error4, pid_out1, pid_out2, pid_out3, pid_out4, motor_speed1, motor_speed2, motor_speed3, motor_speed4}");
        Serial.println("this step is only on GCS");
        break;
      }

      default:
        Serial.printf("Unknown message ID: %d\n", msg_id);
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


void send_pid_status(){
  // pid_status{timestamp, roll, pitch, yaw, altitude, error1, error2, error3, error4, pid_out1, pid_out2, pid_out3, pid_out4, motor_speed1, motor_speed2, motor_speed3, motor_speed4}
  // Ensure WebSocket is initialized before sending data
  if (!webSocket.connectedClients()) {
    return;  // Exit if no clients are connected
  }

  // Create a JSON document with increased size for float values
  StaticJsonDocument<256> jsonDoc;
  // Add data to JSON
  jsonDoc["id"] = 17; // Message identifier
  jsonDoc["timestamp"] = millis();  // Add current time in milliseconds
  jsonDoc["roll"] = orientation[roll_index];       
  jsonDoc["pitch"] = orientation[pitch_index];       
  jsonDoc["yaw"] = orientation[yaw_index];       
  jsonDoc["altitude"] = orientation[alt_index];  // Add altitude data
  jsonDoc["error1"] = errors[0];
  jsonDoc["error2"] = errors[1];
  jsonDoc["error3"] = errors[2];
  jsonDoc["error4"] = errors[3];
  jsonDoc["pid_out1"] = pid_out[0];
  jsonDoc["pid_out2"] = pid_out[1];
  jsonDoc["pid_out3"] = pid_out[2];
  jsonDoc["pid_out4"] = pid_out[3];
  jsonDoc["motor_speed1"] = motors_speed[0];
  jsonDoc["motor_speed2"] = motors_speed[1];
  jsonDoc["motor_speed3"] = motors_speed[2];
  jsonDoc["motor_speed4"] = motors_speed[3];

  // Serialize JSON to string
  String jsonString;
  serializeJson(jsonDoc, jsonString);
  webSocket.broadcastTXT(jsonString); // Broadcast data to all connected clients

  // Serial.printf("errors->(%f, %f, %f, %f)\t", errors[0], errors[1], errors[2], errors[3]);
  // Serial.printf("pid_outs->(%d, %d, %d, %d)\t", pid_out[0], pid_out[1], pid_out[2], pid_out[3]);
  // Serial.printf("motors_speeds->(%d, %d, %d, %d)\n", motors_speed[0], motors_speed[1], motors_speed[2], motors_speed[3]);
}

// // wifi related things
////////////////// WiFi credentials (SSID and Password pairs) ////////////////////////////
// add the below things in the main loop if you want this function
// const char* wifiNetworks[][2] = {
//     {"Saik", "........"},
//     // {"AndroidShare_D5", ""},  // No password
//     {"Sai", "........"}
// };

// const int numNetworks = sizeof(wifiNetworks) / sizeof(wifiNetworks[0]);
// bool isReconnecting = false;
// unsigned long reconnectStartTime = 0;
// const unsigned long reconnectInterval = 10000; // Try reconnecting every 10 sec
// int connectToWiFi() {
//     WiFi.mode(WIFI_STA);
//     for (int i = 0; i < numNetworks; i++) {
//         Serial.print("Trying to connect to: ");
//         Serial.println(wifiNetworks[i][0]);

//         if (strlen(wifiNetworks[i][1]) > 0) {
//             WiFi.begin(wifiNetworks[i][0], wifiNetworks[i][1]); // With password
//         } else {
//             WiFi.begin(wifiNetworks[i][0]); // Open network (no password)
//         }

//         // Give WiFi time to connect asynchronously
//         unsigned long startAttemptTime = millis();
//         while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 5000) {
//             delay(100); // Short delay to avoid CPU overuse
//         }

//         if (WiFi.status() == WL_CONNECTED) {
//             Serial.print("\nConnected to WiFi: ");
//             Serial.println(wifiNetworks[i][0]); // Print connected WiFi name
//             Serial.print("IP Address: ");
//             Serial.println(WiFi.localIP());
//             isReconnecting = false; // Stop reconnect attempts
//             return 1; // Exit function on success
//         }
//     }
//     Serial.println("Could not connect to any WiFi network, will retry...");
//     return 0;
// }