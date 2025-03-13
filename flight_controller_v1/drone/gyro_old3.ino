// #include <Wire.h>

// #define MPU6050_ADDR 0x68
// #define ACCEL_XOUT_H 0x3B
// #define PWR_MGMT_1   0x6B

// float pitch, roll, yaw = 0;  // Initialize yaw
// float bias_pitch = 0, bias_roll = 0, bias_yaw = 0;
// unsigned long gyro_timer;

// /////////////////////////////////////////////////////////
// ////////////// kalman filter related ////////////////////
// float RateRoll, RatePitch, RateYaw;
// float RateCalibrationRoll, RateCalibrationPitch, RateCalibrationYaw;
// int RateCalibrationNumber;
// float AccX, AccY, AccZ;
// float AngleRoll, AnglePitch;
// float KalmanAngleRoll=0, KalmanUncertaintyAngleRoll=2*2;
// float KalmanAnglePitch=0, KalmanUncertaintyAnglePitch=2*2;
// float Kalman1DOutput[]={0,0};

// void kalman_1d(float KalmanState, float KalmanUncertainty, float KalmanInput, float KalmanMeasurement, float dt) {
//   KalmanState=KalmanState+dt*KalmanInput;
//   KalmanUncertainty=KalmanUncertainty + 16*dt*dt;
//   float KalmanGain=KalmanUncertainty/(KalmanUncertainty + 9);
//   KalmanState=KalmanState+KalmanGain * (KalmanMeasurement-KalmanState);
//   KalmanUncertainty=(1-KalmanGain) * KalmanUncertainty;
//   Kalman1DOutput[0]=KalmanState; 
//   Kalman1DOutput[1]=KalmanUncertainty;
// }
// /////////////////////////////////////////////////////////

// // Function to read all sensor data in one burst
// void readMPU(int16_t &ax, int16_t &ay, int16_t &az, int16_t &gx, int16_t &gy, int16_t &gz) {
//     Wire.beginTransmission(MPU6050_ADDR);
//     Wire.write(ACCEL_XOUT_H);
//     Wire.endTransmission(false);
//     Wire.requestFrom(MPU6050_ADDR, 14, true);  // Read 14 bytes at once

//     ax = (Wire.read() << 8) | Wire.read();
//     ay = (Wire.read() << 8) | Wire.read();
//     az = (Wire.read() << 8) | Wire.read();
//     Wire.read(); Wire.read();  // Skip temperature bytes
//     gx = (Wire.read() << 8) | Wire.read();
//     gy = (Wire.read() << 8) | Wire.read();
//     gz = (Wire.read() << 8) | Wire.read();
// }

// void gyro_setup() {
//     Serial.begin(115200);
//     Wire.begin();
//     Wire.setClock(400000);  // Increase I2C speed to 800 kHz
//     Wire.beginTransmission(MPU6050_ADDR);
//     Wire.write(PWR_MGMT_1);
//     Wire.write(0); // Wake up MPU6050
//     Wire.endTransmission(true);

//     if(!isMPUConnected()){
//       Serial.println("please connect mpu6050");
//       while(true);
//     }

//     int samples = 1000;
//     for(int i = 0; i < samples; i++){
//       // Read MPU6050 data 
//       int16_t ax, ay, az, gx, gy, gz;
//       // unsigned long t1 = micros();
//       readMPU(ax, ay, az, gx, gy, gz);
//       // unsigned long t2 = micros();
  
//       RateCalibrationRoll += gx;
//       RateCalibrationPitch += gy;
//       RateCalibrationYaw += gz;
//     }

//     RateCalibrationRoll /= samples;
//     RateCalibrationPitch /= samples;
//     RateCalibrationYaw /= samples;

//     gyro_timer = micros();

// }

// void find_orientation() {    
//     // unsigned long startTime = micros();

//     // Read MPU6050 data
//     int16_t ax, ay, az, gx, gy, gz;
//     // unsigned long t1 = micros();
//     readMPU(ax, ay, az, gx, gy, gz);
//     // unsigned long t2 = micros();

//     // Convert to angles
//     float accelPitch = atan2(ay, az) * 180 / PI;
//     float accelRoll  = atan2(-ax, az) * 180 / PI;
    
//     float gx_dps = (gx - RateCalibrationRoll)/131.0;
//     float gy_dps = (gy - RateCalibrationPitch)/131.0;
//     float gz_dps = (gz - RateCalibrationYaw)/131.0;

//     // Calculate time step (delta t)
//     float dt = (micros() - gyro_timer) / 1000000.0;
//     gyro_timer = micros();

//     // unsigned long t3 = micros();
//     kalman_1d(roll, KalmanUncertaintyAngleRoll, gx_dps, accelRoll, dt);
//     roll=Kalman1DOutput[0]; 
//     KalmanUncertaintyAngleRoll=Kalman1DOutput[1];
   
//     kalman_1d(pitch, KalmanUncertaintyAnglePitch, gy_dps, accelPitch, dt);
//     pitch=Kalman1DOutput[0]; 
//     KalmanUncertaintyAnglePitch=Kalman1DOutput[1];
   
//     yaw += gz_dps * dt;  // Integrate yaw over time

//     // // normal calculations
//     // roll += gx_dps * dt;  // Integrate roll over time
//     // pitch += gy_dps * dt;  // Integrate pitch over time
//     // yaw += gz_dps * dt;  // Integrate yaw over time
//     // unsigned long t4 = micros();
//     // unsigned long endTime = micros();


//     // // Print output
//     Serial.print("Pitch: "); Serial.print(roll);
//     Serial.print(" | Roll: "); Serial.print(pitch);
//     Serial.print(" | Yaw: "); Serial.println(yaw);
//     // Serial.print(" | MPU Read Time: "); Serial.print(t2 - t1);
//     // Serial.print(" | Processing Time: "); Serial.print(t4 - t3);
//     // Serial.print(" | Loop Time: "); Serial.println(endTime - startTime);
//     // while (micros() - startTime < 4000);

//     orientation[roll_index] = roll;
//     orientation[pitch_index] = pitch;
//     orientation[yaw_index] = 0.0;
// }

// bool isMPUConnected() {
//   Wire.beginTransmission(0x68); // MPU6050 I2C address
//   uint8_t error = Wire.endTransmission();
//   error = 0; // delete this after debugging
//   if (error == 0){ // 0 means successful communication
//     mpu6050_status = 0;
//     return 1;
//   }else{
//     if(mpu6050_status == 0){
//       Serial.println("Error: MPU6050 not connected!");
//     }
//     mpu6050_status++;
//     return 0;
//   }
// }