// // //  
// // // // websockets related
// // // 
// void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
//   switch (type) {
//     case WStype_CONNECTED:
//       Serial.printf("Client %u connected\n", num);
//       break;

//     case WStype_DISCONNECTED:
//       Serial.printf("Client %u disconnected\n", num);
//       break;

//     case WStype_TEXT:
//       // Parse incoming PID values (JSON format)
//       String msg = String((char*)payload);
//       handle_msg(msg);

//       // send message to client
//       // webSocket.sendTXT(num, "message here");

//       // send data to all connected clients
//       // webSocket.broadcastTXT("message here");
//       break;

//     case WStype_BIN:
//       Serial.printf("Binary message from client %u, length: %u\n", num, length);
//       break;

//     default:
//       Serial.printf("Unhandled event type %u from client %u\n", type, num);
//       break;
//   }
// }

// void handle_msg(String msg){
//   Serial.printf("Message from client: %s\n", msg);
//   // Serial.printf("Message from client %u: %s\n", num, msg);

//   //msg -> message id   : parameters
//   if (msg.indexOf("id") >= 0) {
//     msg_id = parseJsonValue(msg, "id");
//     Serial.printf("Message ID: %f", msg_id)
  
//     switch (msg_id) {
//       case 0:{ // 0(drone<-GCS): modes(0->standby, 1->motor_calib, 2->imu_calib, 3->guided, 4-> auto)
//         Serial.println("mode message");
//         mode = parseJsonValue(msg, "mode");
//         break;}

//       case 1:{ // 1(drone<>GCS): get_PIDs
//         // Convert variables to JSON format
//         StaticJsonDocument<200> jsonDoc;  // Create a JSON document
//         jsonDoc["id"] = 2;              // Add id
//         jsonDoc["save"] = 2;              // save the values to permenant storage or not
//         jsonDoc["proll"] = proll;        // Add proll
//         jsonDoc["iroll"] = iroll;        // Add iroll
//         jsonDoc["droll"] = droll;        // Add droll
//         jsonDoc["ppitch"] = ppitch;        // Add ppitch
//         jsonDoc["ipitch"] = ipitch;        // Add ipitch
//         jsonDoc["dpitch"] = dpitch;        // Add dpitch
//         jsonDoc["pyaw"] = pyaw;        // Add pyaw
//         jsonDoc["iyaw"] = iyaw;        // Add iyaw
//         jsonDoc["dyaw"] = dyaw;        // Add dyaw

//         // Serialize the JSON document to a string
//         String jsonString;
//         serializeJson(jsonDoc, jsonString);

//         // Print JSON to Serial Monitor
//         Serial.print("sending the pid values")
//         Serial.println(jsonString);

//         // send data to all connected clients
//         webSocket.broadcastTXT(jsonString);
//         break;}

//       case 2: {// 2(drone<>GCS): set_PIDs{save, proll, iroll, droll, ppitch, ipitch, dpitch, pyaw, iyaw, dyaw}
//         Serial.println("PID message");
        
//         save = parseJsonValue(msg, "save");
//         proll = parseJsonValue(msg, "proll");
//         iroll = parseJsonValue(msg, "iroll");
//         droll = parseJsonValue(msg, "droll");
//         ppitch = parseJsonValue(msg, "ppitch");
//         ipitch = parseJsonValue(msg, "ipitch");
//         dpitch = parseJsonValue(msg, "dpitch");
//         pyaw = parseJsonValue(msg, "pyaw");
//         iyaw = parseJsonValue(msg, "iyaw");
//         dyaw = parseJsonValue(msg, "dyaw");
//         Serial.printf("Updated PID: Roll(%f, %f, %f), Pitch(%f, %f, %f), Yaw(%f, %f, %f)\n", proll, iroll, droll, ppitch, ipitch, dpitch, pyaw, iyaw, dyaw);

//         if(save == 1){
//           Serial.println("saving the PID values to EEPROM");
//           // save_pids_to_eeprom();
//         }

//         break;}

//       case 3:{ // 3(drone<>GCS): get_motors_order
//         Serial.println("settings requested, so exporting them");
//         // Convert variables to JSON format
//         StaticJsonDocument<200> jsonDoc;  // Create a JSON document
//         jsonDoc["id"] = 4;              // Add id
//         jsonDoc["top_left"] = order_of_motors[0];
//         jsonDoc["top_right"] = order_of_motors[1];
//         jsonDoc["bottom_left"] = order_of_motors[2];
//         jsonDoc["bottom_right"] = order_of_motors[3];
        
//         // Serialize the JSON document to a string
//         String jsonString;
//         serializeJson(jsonDoc, jsonString);

//         // Print JSON to Serial Monitor
//         Serial.println(jsonString);

//         // send data to all connected clients
//         webSocket.broadcastTXT(jsonString);
//         break;}

//       case 4: {// 4(drone<-GCS): set_motors_order{top_left, top_right, bottom_left, bottom_right}
//         Serial.println("motors_order");
//         order_of_motors[0] = parseJsonValue(msg, "top_left");
//         order_of_motors[1] = parseJsonValue(msg, "top_right");
//         order_of_motors[2] = parseJsonValue(msg, "bottom_left");
//         order_of_motors[3] = parseJsonValue(msg, "bottom_right");
//         break;}

//       case 5:{ // 5(drone<-GCS): motor_test{m1_state, m2_state, m3_state, m4_state, m1_speed, m2_speed, m3_speed, m4_speed}
//         Serial.println("motor calib");
//         motor_states[0] = parseJsonValue(msg, "m1_state");
//         motor_states[1] = parseJsonValue(msg, "m2_state");
//         motor_states[2] = parseJsonValue(msg, "m3_state");
//         motor_states[3] = parseJsonValue(msg, "m4_state");

//         motor_speeds[0] = parseJsonValue(msg, "m1_speed");
//         motor_speeds[1] = parseJsonValue(msg, "m2_speed");
//         motor_speeds[2] = parseJsonValue(msg, "m3_speed");
//         motor_speeds[3] = parseJsonValue(msg, "m4_speed");
//         break;}


//       case 6: {// 6(drone<>GCS): get_settings
//         Serial.println("settings requested, so exporting them");
//         // Convert variables to JSON format
//         StaticJsonDocument<200> jsonDoc;  // Create a JSON document
//         jsonDoc["id"] = 6;              // Add id
//         jsonDoc["max_roll"] = max_values[roll_index];
//         jsonDoc["max_pitch"] = max_values[pitch_index];
//         jsonDoc["max_yaw"] = max_values[yaw_index];
//         jsonDoc["min_speed"] = motor_min_speed;
//         jsonDoc["max_speed"] = motor_max_speed;
        
//         // Serialize the JSON document to a string
//         String jsonString;
//         serializeJson(jsonDoc, jsonString);

//         // Print JSON to Serial Monitor
//         Serial.println(jsonString);

//         // send data to all connected clients
//         webSocket.broadcastTXT(jsonString);
//         break;}

//       case 7:{ // 7(drone<>GCS): set_settings{max_roll, max_pitch, max_yaw, min_speed, max_speed}
//         Serial.println("importing settings");
//         max_values[roll_index] = parseJsonValue(msg, "max_roll");
//         max_values[pitch_index] = parseJsonValue(msg, "max_pitch");
//         max_values[yaw_index] = parseJsonValue(msg, "max_yaw");

//         motor_min_speed = parseJsonValue(msg, "min_speed");
//         motor_max_speed = parseJsonValue(msg, "max_speed");
//         break;}

//       case 8:{ // 8(drone<-GCS): imu_calib_command{step}
//         int calibration_step = parseJsonValue(msg, "step");
//         Serial.println("imu calib command");
//         int result = imu_calibration(calibration_step);

//         if (result == 1) {
//           result_text = "IMU calibration step-" + String(calibration_step) + " is successfully completed";
//         } else {
//           result_text = "IMU calibration step-" + String(calibration_step) + " has failed";
//         }
//         Serial.println(result_text); 

//         // Convert variables to JSON format
//         StaticJsonDocument<200> jsonDoc;  // Create a JSON document
//         jsonDoc["id"] = 9;              // Add id
//         jsonDoc["step"] = calibration_step;
//         jsonDoc["response"] = result_text;
        
//         // Serialize the JSON document to a string
//         String jsonString;
//         serializeJson(jsonDoc, jsonString);

//         // Print JSON to Serial Monitor
//         Serial.println(jsonString);

//         // send data to all connected clients
//         webSocket.broadcastTXT(jsonString);
//         break;}

//       case 9: {// 9(drone->GCS): imu_calib_response{step, response}
//         Serial.println("imu calib response");
//         Serial.println("this step is only on GCS side (drone->GCS)");
//         break;}

//       case 10:{ // 10(drone<-GCS): get_orientation
//         Serial.println("Got a request for drone orientation");
//         Serial.println("Sending the Orientation data");
//         send_orientation();
//         break;}

//       case 11: {// 11(drone->GCS): drone_orientation{roll, pitch, yaw}
//         Serial.print("message ID: 11 (drone orientation)");
//         Serial.println("this step is only on GCS side (drone->GCS)");
//         break;}

//       case 12: {// 12(drone<-GCS): setpoints{roll, pitch, yaw}
//         Serial.println("set points");
//         setpoints[roll_index] = parseJsonValue(msg, "roll");
//         setpoints[pitch_index] = parseJsonValue(msg, "pitch");
//         setpoints[yaw_index] = parseJsonValue(msg, "yaw");
//         break;}
//       default:
//         break;
//   }
//   }
// }

// void send_orientation() {
//   // Ensure WebSocket is initialized before sending data
//   if (!webSocket.connectedClients()) {
//     return;  // Exit if no clients are connected
//   }

//   // Create a JSON document with increased size for float values
//   StaticJsonDocument<256> jsonDoc;

//   // Add data to JSON
//   jsonDoc["id"] = 11; // Message identifier
//   jsonDoc["timestamp"] = millis();  // Add current time in milliseconds
//   jsonDoc["roll"] = orientation[roll_index];       
//   jsonDoc["pitch"] = orientation[pitch_index];       
//   jsonDoc["yaw"] = orientation[yaw_index];       
//   jsonDoc["altitude"] = orientation[alt_index];  // Add altitude data

//   // Serialize JSON to string
//   String jsonString;
//   serializeJson(jsonDoc, jsonString);

//   // Print JSON for debugging
//   // Serial.println(jsonString);

//   // Broadcast data to all connected clients
//   webSocket.broadcastTXT(jsonString);
// }

// // Helper function to extract values from JSON. Example JSON: {"id":0, "rollP":1.0,"rollI":0.1,"rollD":0.01}
// float parseJsonValue(String json, String key) {
//     DynamicJsonDocument doc(1024); // Can be larger or even estimated dynamically
//     DeserializationError error = deserializeJson(doc, json);
    
//     if (error) {
//         Serial.print("Deserialization failed: ");
//         Serial.println(error.c_str());
//         return 0.0; // Handle error case
//     }

//     return doc[key]; // Assumes the key exists and holds a float value
// }

