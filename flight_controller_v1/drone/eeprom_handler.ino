#define EEPROM_SIZE 128

// Define addresses for initialization flags
#define EEPROM_BIAS_FLAG_ADDR 0
#define EEPROM_CALIB_FLAG_ADDR 20
#define EEPROM_PID_FLAG_ADDR 40
#define EEPROM_MOTOR_ORDER_FLAG_ADDR 60
#define EEPROM_SETTINGS_FLAG_ADDR 80

#define EEPROM_FLAG_VALUE 0xA5 // Arbitrary value to indicate initialization

// ========================== VARIABLES ==========================
// float roll_b = 0, pitch_b = 0, yaw_b = 0, X_b = 0, Y_b = 0, Z_b = 0; 
// float roll_cf = 1, pitch_cf = 1, yaw_cf = 1, X_cf = 0, Y_cf = 0, Z_cf = 0; 
// float p_gains[4] = {0, 0, 0, 0};
// float i_gains[4] = {0, 0, 0, 0};
// float d_gains[4] = {0, 0, 0, 0};
// int order_of_motors[4] = {1, 2, 3, 4};
// int motor_min_speed = 1000;
// int motor_max_speed = 2000;
// int max_values[3] = {0, 0, 0}; // roll, pitch, yaw

// ========================== FLAG FUNCTIONS ==========================

void setEEPROMFlag(int addr) {
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.put(addr, EEPROM_FLAG_VALUE);
    EEPROM.commit();
}

bool isEEPROMFlagSet(int addr) {
    EEPROM.begin(EEPROM_SIZE);
    byte flag;
    EEPROM.get(addr, flag);
    return flag == EEPROM_FLAG_VALUE;
}

// ========================== SAVE FUNCTIONS ==========================

// Save MPU6050 Bias
void saveBiasToEEPROM() {
    EEPROM.begin(EEPROM_SIZE);
    int addr = EEPROM_BIAS_FLAG_ADDR + 1;
    EEPROM.put(addr, roll_b); addr += sizeof(roll_b);
    EEPROM.put(addr, pitch_b); addr += sizeof(pitch_b);
    EEPROM.put(addr, yaw_b); addr += sizeof(yaw_b);
    EEPROM.put(addr, X_b); addr += sizeof(X_b);
    EEPROM.put(addr, Y_b); addr += sizeof(Y_b);
    EEPROM.put(addr, Z_b); addr += sizeof(Z_b);
    EEPROM.commit();
    setEEPROMFlag(EEPROM_BIAS_FLAG_ADDR);
    Serial.println("MPU6050 Bias saved to EEPROM");
}

// Save Calibration Factors
void saveCalibrationFactorsToEEPROM() {
    EEPROM.begin(EEPROM_SIZE);
    int addr = EEPROM_CALIB_FLAG_ADDR + 1;
    EEPROM.put(addr, roll_cf); addr += sizeof(roll_cf);
    EEPROM.put(addr, pitch_cf); addr += sizeof(pitch_cf);
    EEPROM.put(addr, yaw_cf); addr += sizeof(yaw_cf);
    EEPROM.put(addr, X_cf); addr += sizeof(X_cf);
    EEPROM.put(addr, Y_cf); addr += sizeof(Y_cf);
    EEPROM.put(addr, Z_cf); addr += sizeof(Z_cf);
    EEPROM.commit();
    setEEPROMFlag(EEPROM_CALIB_FLAG_ADDR);
    Serial.println("Calibration Factors saved to EEPROM");
}

// Save PID Gains
void savePIDToEEPROM() {
    EEPROM.begin(EEPROM_SIZE);
    int addr = EEPROM_PID_FLAG_ADDR + 1;
    for (int i = 0; i < 4; i++) {
        EEPROM.put(addr, p_gains[i]); addr += sizeof(p_gains[i]);
        EEPROM.put(addr, i_gains[i]); addr += sizeof(i_gains[i]);
        EEPROM.put(addr, d_gains[i]); addr += sizeof(d_gains[i]);
    }
    EEPROM.commit();
    setEEPROMFlag(EEPROM_PID_FLAG_ADDR);
    Serial.println("PID Gains saved to EEPROM");
}

// Save Motor Order
void saveMotorOrderToEEPROM() {
    EEPROM.begin(EEPROM_SIZE);
    int addr = EEPROM_MOTOR_ORDER_FLAG_ADDR + 1;
    for (int i = 0; i < 4; i++) {
        EEPROM.put(addr, order_of_motors[i]); addr += sizeof(order_of_motors[i]);
    }
    EEPROM.commit();
    setEEPROMFlag(EEPROM_MOTOR_ORDER_FLAG_ADDR);
    Serial.println("Motor Order saved to EEPROM");
}

// Save Settings
void saveSettingsToEEPROM() {
    EEPROM.begin(EEPROM_SIZE);
    int addr = EEPROM_SETTINGS_FLAG_ADDR + 1;
    EEPROM.put(addr, motor_min_speed); addr += sizeof(motor_min_speed);
    EEPROM.put(addr, motor_max_speed); addr += sizeof(motor_max_speed);
    EEPROM.put(addr, max_values[0]); addr += sizeof(max_values[0]);
    EEPROM.put(addr, max_values[1]); addr += sizeof(max_values[1]);
    EEPROM.put(addr, max_values[2]); addr += sizeof(max_values[2]);
    EEPROM.commit();
    setEEPROMFlag(EEPROM_SETTINGS_FLAG_ADDR);
    Serial.println("Settings saved to EEPROM");
}

// ========================== LOAD FUNCTIONS ==========================

// Load MPU6050 Bias
void loadBiasFromEEPROM() {
    if (!isEEPROMFlagSet(EEPROM_BIAS_FLAG_ADDR)) {
        Serial.println("EEPROM not initialized. Loaded default values for MPU6050 Bias");
        return;
    }
    EEPROM.begin(EEPROM_SIZE);
    int addr = EEPROM_BIAS_FLAG_ADDR + 1;
    EEPROM.get(addr, roll_b); addr += sizeof(roll_b);
    EEPROM.get(addr, pitch_b); addr += sizeof(pitch_b);
    EEPROM.get(addr, yaw_b); addr += sizeof(yaw_b);
    EEPROM.get(addr, X_b); addr += sizeof(X_b);
    EEPROM.get(addr, Y_b); addr += sizeof(Y_b);
    EEPROM.get(addr, Z_b); addr += sizeof(Z_b);
    Serial.print("MPU6050 Bias loaded: ");
    Serial.print("Roll: "); Serial.print(roll_b);
    Serial.print(", Pitch: "); Serial.print(pitch_b);
    Serial.print(", Yaw: "); Serial.print(yaw_b);
    Serial.print(", X: "); Serial.print(X_b);
    Serial.print(", Y: "); Serial.print(Y_b);
    Serial.print(", Z: "); Serial.println(Z_b);
}

// Load Calibration Factors
void loadCalibrationFactorsFromEEPROM() {
    if (!isEEPROMFlagSet(EEPROM_CALIB_FLAG_ADDR)) {
        Serial.println("EEPROM not initialized. Loaded default values for Calibration Factors");
        return;
    }
    EEPROM.begin(EEPROM_SIZE);
    int addr = EEPROM_CALIB_FLAG_ADDR + 1;
    EEPROM.get(addr, roll_cf); addr += sizeof(roll_cf);
    EEPROM.get(addr, pitch_cf); addr += sizeof(pitch_cf);
    EEPROM.get(addr, yaw_cf); addr += sizeof(yaw_cf);
    EEPROM.get(addr, X_cf); addr += sizeof(X_cf);
    EEPROM.get(addr, Y_cf); addr += sizeof(Y_cf);
    EEPROM.get(addr, Z_cf); addr += sizeof(Z_cf);
    Serial.print("Calibration Factors loaded: ");
    Serial.print("Roll: "); Serial.print(roll_cf);
    Serial.print(", Pitch: "); Serial.print(pitch_cf);
    Serial.print(", Yaw: "); Serial.print(yaw_cf);
    Serial.print(", X: "); Serial.print(X_cf);
    Serial.print(", Y: "); Serial.print(Y_cf);
    Serial.print(", Z: "); Serial.println(Z_cf);
}

// Load PID Gains
void loadPIDFromEEPROM() {
    if (!isEEPROMFlagSet(EEPROM_PID_FLAG_ADDR)) {
        Serial.println("EEPROM not initialized. Loaded default values for PID Gains");
        return;
    }
    EEPROM.begin(EEPROM_SIZE);
    int addr = EEPROM_PID_FLAG_ADDR + 1;
    for (int i = 0; i < 4; i++) {
        EEPROM.get(addr, p_gains[i]); addr += sizeof(p_gains[i]);
        EEPROM.get(addr, i_gains[i]); addr += sizeof(i_gains[i]);
        EEPROM.get(addr, d_gains[i]); addr += sizeof(d_gains[i]);
    }
    Serial.print("PID Gains loaded: ");
    for (int i = 0; i < 4; i++) {
        Serial.print("P[" + String(i) + "]: " + String(p_gains[i]) + ", ");
        Serial.print("I[" + String(i) + "]: " + String(i_gains[i]) + ", ");
        Serial.print("D[" + String(i) + "]: " + String(d_gains[i]) + "; ");
    }
    Serial.println();
}

// Load Motor Order
void loadMotorOrderFromEEPROM() {
    if (!isEEPROMFlagSet(EEPROM_MOTOR_ORDER_FLAG_ADDR)) {
        Serial.println("EEPROM not initialized. Loaded default motor order");
        return;
    }
    EEPROM.begin(EEPROM_SIZE);
    int addr = EEPROM_MOTOR_ORDER_FLAG_ADDR + 1;
    for (int i = 0; i < 4; i++) {
        EEPROM.get(addr, order_of_motors[i]); addr += sizeof(order_of_motors[i]);
    }
    Serial.print("Motor Order loaded: ");
    for (int i = 0; i < 4; i++) {
        Serial.print(order_of_motors[i]);
        if (i < 3) Serial.print(", ");
    }
    Serial.println();
}

// Load Settings
void loadSettingsFromEEPROM() {
    if (!isEEPROMFlagSet(EEPROM_SETTINGS_FLAG_ADDR)) {
        Serial.println("EEPROM not initialized. Loaded default settings");
        return;
    }
    EEPROM.begin(EEPROM_SIZE);
    int addr = EEPROM_SETTINGS_FLAG_ADDR + 1;
    EEPROM.get(addr, motor_min_speed); addr += sizeof(motor_min_speed);
    EEPROM.get(addr, motor_max_speed); addr += sizeof(motor_max_speed);
    EEPROM.get(addr, max_values[0]); addr += sizeof(max_values[0]);
    EEPROM.get(addr, max_values[1]); addr += sizeof(max_values[1]);
    EEPROM.get(addr, max_values[2]); addr += sizeof(max_values[2]);
    Serial.print("Settings loaded: ");
    Serial.print("Motor Min Speed: "); Serial.print(motor_min_speed);
    Serial.print(", Motor Max Speed: "); Serial.print(motor_max_speed);
    Serial.print(", Max Roll: "); Serial.print(max_values[0]);
    Serial.print(", Max Pitch: "); Serial.print(max_values[1]);
    Serial.print(", Max Yaw: "); Serial.println(max_values[2]);
}

// ========================== USAGE EXAMPLES ==========================

void load_configs_from_eeprom() {
    EEPROM.begin(EEPROM_SIZE);
    Serial.println("Loading stored values...");
    // loadBiasFromEEPROM();
    // loadCalibrationFactorsFromEEPROM();
    loadPIDFromEEPROM();
    // loadMotorOrderFromEEPROM();
    // loadSettingsFromEEPROM();
}
