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
}

void calculate_pid(){  // Pass time difference in seconds
  for (int index = 0; index < 4; index++){
    errors[index] = orientation[index] - setpoints[index];

    // Integrate error with time scaling and clamping
    error_sum[index] += errors[index] * dt;
    error_sum[index] = constrain(error_sum[index], -max_values[index], max_values[index]);

    // Differentiate error with time scaling
    float error_dif = (errors[index] - prev_errors[index]) / dt;  // Use float for precision

    // Compute PID output and clamp it
    pid_out[index] = p_gains[index] * errors[index] +
                     i_gains[index] * error_sum[index] +
                     d_gains[index] * error_dif;

    pid_out[index] = constrain(pid_out[index], -max_values[index], max_values[index]);

    prev_errors[index] = errors[index];
  }
}
