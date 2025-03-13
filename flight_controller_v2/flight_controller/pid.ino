// //  
// // // PID related
// // 
void pid(){
  calculate_pid();

  float roll, pitch, yaw, altitude = 0;

  roll  = pid_out[roll_index];
  pitch = pid_out[pitch_index];

  // yaw   = pid_out[yaw_index];
  // altitude = pid_out[alt_index] + base_throttle;

  altitude += base_throttle;

  // Motor mixing for quadcopter in X configuration
  motors_speed[0] = altitude - roll + pitch + yaw;  // Front-Left  (Motor 1)
  motors_speed[1] = altitude + roll + pitch - yaw;  // Front-Right (Motor 2)
  motors_speed[2] = altitude - roll - pitch - yaw;  // Back-Left   (Motor 3)
  motors_speed[3] = altitude + roll - pitch + yaw;  // Back-Right  (Motor 4)

  Serial.printf("m1->%d, m2->%d, m3->%d, m4->%d\n", motors_speed[0],motors_speed[1], motors_speed[2], motors_speed[3]);
}

void calculate_pid(){  
  dt = (micros() - lastPIDTime)/(1e-6);
  lastPIDTime = micros();
  // Serial.println(dt, 9);
  // below code is given by chatgpt

  for (int index = 0; index < 4; index++){
    errors[index] =  orientation[index] - setpoints[index];  // Ensure correct error sign

    // Integrate error with time scaling and anti-windup
    float integral = error_sum[index] + errors[index] * dt;
    if (pid_out[index] < max_values[index] && pid_out[index] > -max_values[index]) {
        error_sum[index] = integral;  // Only integrate when not saturated
    }

    // Apply integral windup constraint
    error_sum[index] = constrain(error_sum[index], -max_values[index], max_values[index]);

    // Differentiate error with time scaling and noise filtering
    float error_dif = (errors[index] - prev_errors[index]) / (dt + 1e-6);
    static float filtered_d = 0;
    float alpha = 0.1;  
    filtered_d = alpha * error_dif + (1 - alpha) * filtered_d;

    // Compute PID output and clamp it
    pid_out[index] = p_gains[index] * errors[index] +
                    i_gains[index] * error_sum[index] +
                    d_gains[index] * filtered_d;

    pid_out[index] = constrain(pid_out[index], -max_values[index], max_values[index]);

    prev_errors[index] = errors[index];

    Serial.print(pid_out[index]); Serial.print("\t");
  }
  // Serial.println();
}
