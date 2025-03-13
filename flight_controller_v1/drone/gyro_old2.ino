// #include <Wire.h>
// float RateRoll, RatePitch, RateYaw;
// float RateCalibrationRoll, RateCalibrationPitch, RateCalibrationYaw;
// int RateCalibrationNumber;
// float AccX, AccY, AccZ;
// float AngleRoll, AnglePitch;
// // float KalmanAngleRoll=0, KalmanUncertaintyAngleRoll=2*2;
// // float KalmanAnglePitch=0, KalmanUncertaintyAnglePitch=2*2;
// float Kalman1DOutput[]={0,0};

// void kalman_1d(float KalmanState, float KalmanUncertainty, float KalmanInput, float KalmanMeasurement) {
//   KalmanState=KalmanState+0.004*KalmanInput;
//   KalmanUncertainty=KalmanUncertainty + 0.000256;
//   float KalmanGain=KalmanUncertainty/(KalmanUncertainty + 9);
//   KalmanState=KalmanState+KalmanGain * (KalmanMeasurement-KalmanState);
//   KalmanUncertainty=(1-KalmanGain) * KalmanUncertainty;
//   Kalman1DOutput[0]=KalmanState; 
//   Kalman1DOutput[1]=KalmanUncertainty;
// }

// void gyro_signals(void) {
//   Wire.beginTransmission(0x68);
//   Wire.write(0x1A);
//   Wire.write(0x05);
//   Wire.endTransmission();
//   Wire.beginTransmission(0x68);
//   Wire.write(0x1C);
//   Wire.write(0x10);
//   Wire.endTransmission();
//   Wire.beginTransmission(0x68);
//   Wire.write(0x3B);
//   Wire.endTransmission(); 
//   Wire.requestFrom(0x68,6);
//   int16_t AccXLSB = Wire.read() << 8 | Wire.read();
//   int16_t AccYLSB = Wire.read() << 8 | Wire.read();
//   int16_t AccZLSB = Wire.read() << 8 | Wire.read();
//   Wire.beginTransmission(0x68);
//   Wire.write(0x1B); 
//   Wire.write(0x8);
//   Wire.endTransmission();     
//   Wire.beginTransmission(0x68);
//   Wire.write(0x43);
//   Wire.endTransmission();
//   Wire.requestFrom(0x68,6);
//   int16_t GyroX=Wire.read()<<8 | Wire.read();
//   int16_t GyroY=Wire.read()<<8 | Wire.read();
//   int16_t GyroZ=Wire.read()<<8 | Wire.read();
//   RateRoll=(float)GyroX*0.024535878;
//   RatePitch=(float)GyroY*0.024535878;
//   RateYaw=(float)GyroZ*0.024535878;
//   AccX=(float)AccXLSB/4096;
//   AccY=(float)AccYLSB/4096;
//   AccZ=(float)AccZLSB/4096;
//   AngleRoll=atan(AccY/sqrt(AccX*AccX+AccZ*AccZ))*1/(3.142/180);
//   AnglePitch=-atan(AccX/sqrt(AccY*AccY+AccZ*AccZ))*1/(3.142/180);
 
//   // Serial.print("AccX ");
//   // Serial.print(AccX);
//   // Serial.print(" | AccY ");
//   // Serial.print(AccY);
//   // Serial.print(" | AccZ ");
//   // Serial.println(AccZ);
// }

// void find_orientation() {
//   if (isMPUConnected()) {
//   gyro_signals();
//   RateRoll = (RateRoll - RateCalibrationRoll);
//   RatePitch = (RatePitch - RateCalibrationPitch);
//   RateYaw = (RateYaw - RateCalibrationYaw);
//   kalman_1d(KalmanAngleRoll, KalmanUncertaintyAngleRoll, RateRoll, AngleRoll);
//   KalmanAngleRoll=Kalman1DOutput[0]; 
//   KalmanUncertaintyAngleRoll=Kalman1DOutput[1];
//   kalman_1d(KalmanAnglePitch, KalmanUncertaintyAnglePitch, RatePitch, AnglePitch);
//   KalmanAnglePitch=Kalman1DOutput[0]; 
//   KalmanUncertaintyAnglePitch=Kalman1DOutput[1];


//   // orientation[roll_index] = KalmanAngleRoll;
//   // orientation[pitch_index] = KalmanAnglePitch;
//   // orientation[yaw_index] = 0;

// // // uncomment the below code for debugging, and comment the above 3 lines
// //   orientation[roll_index] += RateRoll*0.004;
// //   orientation[pitch_index] += RatePitch*0.004;
// //   orientation[yaw_index] += RateYaw*0.004;

// //   Serial.print("Roll ");
// //   Serial.print(orientation[roll_index]);
// //   Serial.print(" | Pitch ");
// //   Serial.print(orientation[pitch_index]);
// //   Serial.print(" | Yaw ");
// //   Serial.print(orientation[yaw_index]);

// //   Serial.print(" ||  Roll ");
// //   Serial.print(KalmanAngleRoll);
// //   Serial.print(" | Pitch ");
// //   Serial.println(KalmanAnglePitch);
//   }
// }

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
//   Wire.setClock(400000);
//   Wire.begin();
//   delay(250);
//   Wire.beginTransmission(0x68); 
//   Wire.write(0x6B);
//   Wire.write(0x00);
//   Wire.endTransmission();
//   Serial.println("Don't move, calibrating");
//   int n = 100;
//   for (RateCalibrationNumber=0; RateCalibrationNumber<n; RateCalibrationNumber ++) {
//     if(isMPUConnected()){
//     updateLEDIndicator();
//     gyro_signals();
//     RateCalibrationRoll+=RateRoll;
//     RateCalibrationPitch+=RatePitch;
//     RateCalibrationYaw+=RateYaw;
//     delay(1);
//     }
//   }
//   RateCalibrationRoll/=n;
//   RateCalibrationPitch/=n;
//   RateCalibrationYaw/=n;
//   Serial.println("Calibration completed");
// }