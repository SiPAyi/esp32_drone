// // // 
// // // // gyro related
// // // 
// bool isMPUConnected() {
//   Wire.beginTransmission(0x68); // MPU6050 I2C address
//   uint8_t error = Wire.endTransmission();
//   error = 0; // delete this after debugging
//   if (error == 0){ // 0 means successful communication
//     mpu6050_status = 0;
//     return 1;
//   }else{
//     if(mpu6050_status == 1){
//       Serial.println("Error: MPU6050 not connected!");
//     }
//     mpu6050_status++;
//     return 0;
//   }
// }

// void gyro_setup() {
//   // Initialize I2C
//   Wire.setClock(400000);
//   Wire.begin();

//   delay(500);
//   digitalWrite(LED_BUILTIN, HIGH);

//   if (!isMPUConnected()) {
//     Serial.println("Error: MPU6050 not connected!");
//     while (true) {
//       delay(1000); // Halt the program or handle the error appropriately
//     }
//   }
//   mpu6050_status = 1;


//   // Wake up MPU6050
//   Wire.beginTransmission(0x68);
//   Wire.write(0x6B);
//   Wire.write(0x00); // Set sleep register to 0
//   Wire.endTransmission();

//   // Configure gyro and accelerometer ranges
//   Wire.beginTransmission(0x68);
//   Wire.write(0x1B); // Gyro config
//   Wire.write(0x08); // ±500 degrees/sec
//   Wire.endTransmission();

//   Wire.beginTransmission(0x68);
//   Wire.write(0x1C); // Accelerometer config
//   Wire.write(0x10); // ±8g
//   Wire.endTransmission();

//   delay(1000);
//   digitalWrite(LED_BUILTIN, LOW);
// }


// void find_orientation() {
//   get_gyro_values();

//   // Convert raw data
//   // (angular velocity - bias) * calib_factor
//   raw_roll = (raw_roll - roll_b) * roll_cf * (sampling_time/1000);
//   raw_pitch = (raw_pitch - pitch_b) * pitch_cf * (sampling_time/1000);
//   raw_yaw = (raw_yaw - yaw_b) * yaw_cf * (sampling_time/1000);

//   // Serial.printf("roll(%d) pitch(%d) yaw(%d)", raw_roll, raw_pitch, raw_yaw);
//   // (linear acceleration - bias) * calib_factor
//   float accX = (raw_accX - X_b) * X_cf;
//   float accY = (raw_accY - Y_b) * Y_cf;
//   float accZ = (raw_accZ - Z_b) * Z_cf;

//   orientation_from_gyro[roll_index] += raw_roll;
//   orientation_from_gyro[pitch_index] += raw_pitch;
//   orientation_from_gyro[yaw_index] += raw_yaw;

//   orientation_from_acc[roll_index] = atan2(accY, accZ) * 180.0 / PI;
//   orientation_from_acc[pitch_index] = atan2(-accX, sqrt(accY * accY + accZ * accZ)) * 180.0 / PI;

//   // also we can apply kalman filter to get the correct results
//   float complimentary_gain = 0.98;
//   orientation[roll_index] = complimentary_gain * orientation_from_gyro[roll_index] + (1 - complimentary_gain) * orientation_from_acc[roll_index];
//   orientation[pitch_index] = complimentary_gain * orientation_from_gyro[pitch_index] + (1 - complimentary_gain) * orientation_from_acc[pitch_index];
//   orientation[yaw_index] = orientation_from_gyro[yaw_index];

//   Serial.println(String() + orientation[roll_index] + F("\t") + orientation[pitch_index] + F("\t") + orientation[yaw_index]);
// }


void get_gyro_values() {
  mpu6050_status = isMPUConnected();

  if (!mpu6050_status) {
    Serial.println("Error: MPU6050 not connected!");
  }
  else{
    // Read gyroscope data
    Wire.beginTransmission(0x68);
    Wire.write(0x43);
    Wire.endTransmission();
    Wire.requestFrom(0x68, 6);
    raw_roll = Wire.read() << 8 | Wire.read();
    raw_pitch = Wire.read() << 8 | Wire.read();
    raw_yaw = Wire.read() << 8 | Wire.read();

    // Read accelerometer data
    Wire.beginTransmission(0x68);
    Wire.write(0x3B);
    Wire.endTransmission();
    Wire.requestFrom(0x68, 6);
    raw_accX = Wire.read() << 8 | Wire.read();
    raw_accY = Wire.read() << 8 | Wire.read();
    raw_accZ = Wire.read() << 8 | Wire.read();
  }
}


// values got from this function are not used
int imu_calibration(int stepNumber) {
  Serial.printf("IMU calibration step-%d :", stepNumber);
  int current_step_status = 0;
  float g = 9.8; // acceleration due to gravity
  int duration = 5; // Set duration in seconds
  int n = 100; // number of iterations

  // Check if MPU6050 is connected
  if (!isMPUConnected()) {
    Serial.println("MPU6050 not connected! Aborting calibration.");
    current_step_status = -1;
    return current_step_status;
  }

  switch (stepNumber) {
    case 1: {
      Serial.println("Flat surface calibration...");
      float roll_avg = 0, pitch_avg = 0, yaw_avg = 0, X_avg = 0, Y_avg = 0;
      for (int i = 0; i < n; i++) {
        get_gyro_values();
        roll_avg += raw_roll;
        pitch_avg += raw_pitch;
        yaw_avg += raw_yaw;
        X_avg += raw_accX;
        Y_avg += raw_accY;
      }
      roll_avg /= n;
      pitch_avg /= n;
      yaw_avg /= n;
      X_avg /= n;
      Y_avg /= n;

      // Bias calculation
      roll_b = roll_avg;
      pitch_b = pitch_avg;
      yaw_b = yaw_avg;
      X_b = X_avg;
      Y_b = Y_avg;

      current_step_status = 1;
      break;
    }

    case 2: {
      Serial.println("Back position calibration...");
      float Z_avg = 0, Y_avg_without_bias = 0;
      for (int i = 0; i < n; i++) {
        get_gyro_values();
        Z_avg += raw_accZ;
        Y_avg_without_bias += (raw_accY - Y_b);
      }
      Z_avg /= n;
      Y_avg_without_bias /= n;

      Z_b = Z_avg;
      if (Y_avg_without_bias != 0) {
        Y_cf = g / Y_avg_without_bias;
        current_step_status = 1;
      } else {
        Serial.println("Error: Division by zero in Y_cf calculation.");
      }
      break;
    }

    case 3: {
      Serial.println("Side position calibration...");
      float X_avg_without_bias = 0;
      for (int i = 0; i < n; i++) {
        get_gyro_values();
        X_avg_without_bias += (raw_accX - X_b);
      }
      X_avg_without_bias /= n;

      if (X_avg_without_bias != 0) {
        X_cf = g / X_avg_without_bias;
        current_step_status = 1;
      } else {
        Serial.println("Error: Division by zero in X_cf calculation.");
      }
      break;
    }

    case 4: {
      Serial.println("Flat surface Z-acceleration calibration...");
      int n = 100;
      float Z_avg_without_bias = 0;
      for (int i = 0; i < n; i++) {
        get_gyro_values();
        Z_avg_without_bias += (raw_accZ - Z_b);
      }
      Z_avg_without_bias /= n;

      if (Z_avg_without_bias != 0) {
        Z_cf = g / Z_avg_without_bias;
        current_step_status = 1;
      } else {
        Serial.println("Error: Division by zero in Z_cf calculation.");
      }
      break;
    }

    case 5: {
      Serial.println("Roll axis calibration...");
      Serial.printf("Rotate the drone around 'ROLL-AXIS' for once in %d seconds\n", duration);
      int roll_avg_without_bias = 0;
      unsigned long startTime = millis();
      while (millis() - startTime < duration * 1000) {
        get_gyro_values();
        roll_avg_without_bias += (raw_roll - roll_b);
        yield(); // Feeds the watchdog
      }

      if (roll_avg_without_bias != 0) {
        roll_cf = 360.0 / roll_avg_without_bias;
        current_step_status = 1;
      } else {
        Serial.println("Error: Division by zero in roll_cf calculation.");
      }
      break;
    }

    case 6: {
      Serial.println("Pitch axis calibration...");
      Serial.printf("Rotate the drone around 'PITCH-AXIS' for once in %d seconds\n", duration);
      int pitch_avg_without_bias = 0;
      unsigned long startTime = millis();
      while (millis() - startTime < duration * 1000) {
        get_gyro_values();
        pitch_avg_without_bias += (raw_pitch - pitch_b);
        yield(); // Feeds the watchdog
      }

      if (pitch_avg_without_bias != 0) {
        pitch_cf = 360.0 / pitch_avg_without_bias;
        current_step_status = 1;
      } else {
        Serial.println("Error: Division by zero in pitch_cf calculation.");
      }
      break;
    }

    case 7: {
      Serial.println("Yaw axis calibration...");
      Serial.printf("Rotate the drone around 'YAW-AXIS' for once in %d seconds\n", duration);
      int yaw_avg_without_bias = 0;
      unsigned long startTime = millis();
      while (millis() - startTime < duration * 1000) {
        get_gyro_values();
        yaw_avg_without_bias += (raw_yaw - yaw_b);
        yield(); // Feeds the watchdog
      }

      if (yaw_avg_without_bias != 0) {
        yaw_cf = 360.0 / yaw_avg_without_bias;
        current_step_status = 1;
        saveBiasToEEPROM();
        saveCalibrationFactorsToEEPROM();
      } else {
        Serial.println("Error: Division by zero in yaw_cf calculation.");
      }
      break;
    }

    default: {
      Serial.println("Invalid step!");
      break;
    }
  }

  return current_step_status;
}

bool isMPUConnected() {
  Wire.beginTransmission(0x68); // MPU6050 I2C address
  uint8_t error = Wire.endTransmission();
  error = 0; // delete this after debugging
  if (error == 0){ // 0 means successful communication
    mpu6050_status = 0;
    return 1;
  }else{
    if(mpu6050_status == 0){
      Serial.println("Error: MPU6050 not connected!");
    }
    mpu6050_status++;
    return 0;
  }
}

