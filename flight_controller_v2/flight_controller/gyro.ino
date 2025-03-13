#include "Wire.h"
#include "MPU6050_6Axis_MotionApps20.h"

MPU6050 mpu;
bool dmpReady = false;
uint16_t packetSize;
uint8_t fifoBuffer[64];

Quaternion q;
VectorFloat gravity;
float ypr[3];

void gyro_setup() {
    Serial.begin(115200);
    Wire.begin();
    Wire.setClock(400000);
    mpu.initialize();

    Serial.println("Calibrating MPU6050...");
    mpu.CalibrateAccel(10);
    mpu.CalibrateGyro(10);

    if (mpu.dmpInitialize() != 0) {
        Serial.println("DMP Initialization Failed!");
        while (1);
    }

    Serial.println("MPU6050 Initialized!");
    mpu.setDMPEnabled(true);
    dmpReady = true;
    packetSize = mpu.dmpGetFIFOPacketSize();
}

void find_orientation() {
    if (!dmpReady) return;

    if (mpu.getFIFOCount() == 1024) { // Handle FIFO overflow
        mpu.resetFIFO();
        Serial.println("FIFO Overflow! Resetting...");
        return;
    }

    if (mpu.getFIFOCount() < packetSize) return;

    mpu.getFIFOBytes(fifoBuffer, packetSize);

    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

    // // Axis corrections (if needed)
    // ypr[0] = -ypr[0]; // Invert Yaw if necessary
    // ypr[1] = -ypr[1]; // Invert Pitch if necessary
    // ypr[2] = ypr[2];  // Keep Roll as is

    // Axis corrections (if needed)
    orientation[yaw_index] = ypr[0] * 180/M_PI; // Invert Yaw if necessary
    orientation[pitch_index] = ypr[1] * 180/M_PI; // Invert Pitch if necessary
    orientation[roll_index] = ypr[2] * 180/M_PI;  // Keep Roll as is

    // Serial.print("Yaw: ");
    // Serial.print(ypr[0] * 180/M_PI);
    // Serial.print(" | Pitch: ");
    // Serial.print(ypr[1] * 180/M_PI);
    // Serial.print(" | Roll: ");
    // Serial.println(ypr[2] * 180/M_PI);
}
