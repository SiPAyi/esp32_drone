#define LED_PIN LED_BUILTIN

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
unsigned long prevTime = micros();
unsigned long lastPIDTime = micros();
int mode = 10;//10->setup 0->standby, 1->motor_calib, 2->imu_calib, 3->guided, 4-> auto

int sampling_time = 4000; // microseconds
float dt = sampling_time / 1000000.0;  // Correct conversion to seconds

int count = 0;
unsigned long countTime = micros();

void setup() {
    Serial.begin(115200);
    Serial2.begin(57600, SERIAL_8N1, 16, 17);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    Serial.println("(: Drone started :)");

    // **Step 2: Motor Setup**
    setup_motors();

    // **Step 5: Gyro setup**
    gyro_setup();

    // **Step 6: Ready**
    Serial.println("Drone Ready!");
    digitalWrite(LED_PIN, HIGH); // Solid ON for Wi-Fi connected

    mode = 0;
}

void loop() {
  count++; 

// communication to get commands and pid
  unsigned long t1 = micros();
  if (Serial2.available()) {
    String receivedData = Serial2.readStringUntil('\n'); // Read the full line

    Serial.println(receivedData);

    // Parse the received string
    sscanf(receivedData.c_str(), "%d,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f",
            &mode, &base_throttle, &p_gains[roll_index], &i_gains[roll_index], &d_gains[roll_index], &p_gains[pitch_index], &i_gains[pitch_index], &d_gains[pitch_index]);

      // // Print parsed values
      // Serial.println("Parsed Values:");
      // Serial.print("Mode: "); Serial.println(mode);
      // Serial.print("Roll PID: "); Serial.print(p_gains[roll_index]); Serial.print(", "); Serial.print(i_gains[roll_index]); Serial.print(", "); Serial.println(d_gains[roll_index]);
      // Serial.print("Pitch PID: "); Serial.print(p_gains[pitch_index]); Serial.print(", "); Serial.print(i_gains[pitch_index]); Serial.print(", "); Serial.println(d_gains[pitch_index]);
      // Serial.print("Yaw PID: "); Serial.print(p_gains[yaw_index]); Serial.print(", "); Serial.print(i_gains[yaw_index]); Serial.print(", "); Serial.println(d_gains[yaw_index]);
      // Serial.print("Base Throttle: "); Serial.println(base_throttle);
  }

  // unsigned long t2 = micros();
  find_orientation();

  // unsigned long t3 = micros();
  pid();

  // unsigned long t4 = micros();
  if(mode != 1){
    motors_speed[0] = motor_min_speed;
    motors_speed[1] = motor_min_speed;
    motors_speed[2] = motor_min_speed;
    motors_speed[3] = motor_min_speed;
  }
  spin_motors();

  // unsigned long t5 = micros();

  // if(micros() - countTime >= 1000000){
  //   Serial.println(count);
  //   countTime = micros();
  // }


  // Measure execution time
  while(micros() - prevTime < sampling_time);
  // unsigned long t6 = micros();


  // Serial.print(count);
  // Serial.print(" Execution Time(in micro seconds)-> ");
  // Serial.print(" uart: "); Serial.print(t2 - t1);
  // Serial.print(" gyro: "); Serial.print(t3 - t2);
  // Serial.print(" pid: "); Serial.print(t4 - t3);
  // Serial.print(" motors: "); Serial.print(t5 - t4);
  // Serial.print(" while: "); Serial.println(t6 - t5);

  prevTime = micros();
}
