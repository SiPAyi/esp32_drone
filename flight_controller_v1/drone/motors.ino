#include <ESP32Servo.h>

Servo motor_tl;
Servo motor_tr;
Servo motor_bl;
Servo motor_br;

int speed_factor = (motor_max_speed - motor_min_speed) / 2;

int esc_pins[4] = {14, 27, 26, 25};

void setup_motors() {
    Serial.println("Motor Setup");

    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);

    motor_tl.setPeriodHertz(50);  // 50Hz PWM signal
    motor_tr.setPeriodHertz(50);
    motor_bl.setPeriodHertz(50);
    motor_br.setPeriodHertz(50);

    motor_tl.attach(esc_pins[order_of_motors[0]], motor_min_speed, motor_max_speed);
    motor_tr.attach(esc_pins[order_of_motors[1]], motor_min_speed, motor_max_speed);
    motor_bl.attach(esc_pins[order_of_motors[2]], motor_min_speed, motor_max_speed);
    motor_br.attach(esc_pins[order_of_motors[3]], motor_min_speed, motor_max_speed);

    delay(100);  // Allow ESCs to initialize properly
    arm();
    delay(2000);  // to prevent motor beeps distrub the gyro
}

void reconfig_motors(){
    motor_tl.detach();
    motor_tr.detach();
    motor_bl.detach();
    motor_br.detach();

    motor_tl.attach(esc_pins[order_of_motors[0]], motor_min_speed, motor_max_speed);
    motor_tr.attach(esc_pins[order_of_motors[1]], motor_min_speed, motor_max_speed);
    motor_bl.attach(esc_pins[order_of_motors[2]], motor_min_speed, motor_max_speed);
    motor_br.attach(esc_pins[order_of_motors[3]], motor_min_speed, motor_max_speed);

    Serial.println("motors reconfigured");
}

void spin_motors(){
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

        // Serial.print(" \t motor");
        // Serial.print(i);
        // Serial.print("-> ");
        // Serial.print(motors_speed[i]);
      }
      // Serial.println();
}

void disarm(){
int current_speed = (motors_speed[0] + motors_speed[1] + motors_speed[2] + motors_speed[3])/4; 

while (current_speed > motor_min_speed) {
    current_speed -= 50;  // Increase in small steps

    motors_speed[0] = current_speed;
    motors_speed[1] = current_speed;
    motors_speed[2] = current_speed;
    motors_speed[3] = current_speed;

    spin_motors();
    delay(250);  // Small delay to allow smooth transition
}

  motor_states[0] = 0;
  motor_states[1] = 0;
  motor_states[2] = 0;
  motor_states[3] = 0;

  Serial.println("Disarmed");
}

void arm(){
  motor_states[0] = 1;
  motor_states[1] = 1;
  motor_states[2] = 1;
  motor_states[3] = 1;

  motors_speed[0] = motor_min_speed;
  motors_speed[1] = motor_min_speed;
  motors_speed[2] = motor_min_speed;
  motors_speed[3] = motor_min_speed;
  spin_motors();

  Serial.println("Armed");
}

bool motor_calibration(int step_number) {
    Serial.println("Calibrating ESCs...");
    Serial.println("Step 1: Disconnect battery and remove props.");
    Serial.println("Step 2: Sending MAX throttle...");
    
    motors_speed[0] = motor_max_speed;
    motors_speed[1] = motor_max_speed;
    motors_speed[2] = motor_max_speed;
    motors_speed[3] = motor_max_speed;
    
    spin_motors();
    delay(3000);
    
    Serial.println("Step 3: Sending MIN throttle...");
    motors_speed[0] = motor_min_speed;
    motors_speed[1] = motor_min_speed;
    motors_speed[2] = motor_min_speed;
    motors_speed[3] = motor_min_speed;
    
    spin_motors();

    delay(3000);
    Serial.println("ESC calibration complete!");

    return 0;
}

void landDrone() {
    disarm();
    // apply the logic here to landing the drone
    Serial.println("add logic to land the drone ...");
}

void hover_at_this_pose() {
    Serial.println("apply the logic here to hover the drone ...");
    // apply the logic here to hover the drone
    disarm();
}
