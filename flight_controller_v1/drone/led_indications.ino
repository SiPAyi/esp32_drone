unsigned long previousMillis = 0;
int ledState = LOW;
int blinkInterval = 1500;  // Default relaxed state
int blinkPatternState = 0; // Used for LAND and EMERGENCY modes

// Function to blink LED `count` times with `interval` ms
void blinkLED(int count, int interval) {
    for (int i = 0; i < count; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(interval);
        digitalWrite(LED_PIN, LOW);
        delay(interval);
    }
}

void updateLEDIndicator() {
    unsigned long currentMillis = millis();

    switch (mode) {
        case 10:{ // SETUP mode
          blinkInterval = 100;
          Serial.print(".");
        }

        case 0:  {// STABLE mode (Relaxed, slow blink)
            blinkInterval = 1500;
            break;}

        case 1:  {// GUIDED mode (Moderate blink)
            blinkInterval = 800;
            break;}

        case 2: { // AUTO mode (Focused, faster blink)
            blinkInterval = 500;
            break;}

        case 3: { // CALIB mode (Steady ON)
            digitalWrite(LED_PIN, HIGH);
            return;}

        case 4:  {// MOTOR_TEST mode (Very fast blink, high tension)
            blinkInterval = 200;
            break;}

        case 5:  {// LAND mode (Two quick blinks, then pause)
            if (blinkPatternState < 2) {
                digitalWrite(LED_PIN, HIGH);
            } else {
                digitalWrite(LED_PIN, LOW);
            }

            if (currentMillis - previousMillis >= 100) {
                previousMillis = currentMillis;
                blinkPatternState++;
                if (blinkPatternState >= 4) {
                    blinkPatternState = 0;  // Reset after 4 cycles
                }
            }
            return;}

        case 6:  {// EMERGENCY mode (Extreme alert, SOS pattern)
            int sosPattern[] = {100, 100, 100, 300, 300, 300, 100, 100, 100, 1000};
            if (currentMillis - previousMillis >= sosPattern[blinkPatternState]) {
                previousMillis = currentMillis;
                ledState = !ledState;
                digitalWrite(LED_PIN, ledState);
                blinkPatternState++;
                if (blinkPatternState >= 10) {
                    blinkPatternState = 0;  // Reset after full SOS cycle
                }
            }
            return;}

        default:
            blinkInterval = 1500;  // Default relaxed mode
            break;
    }

    // Handle normal blinking based on interval
    if (currentMillis - previousMillis >= blinkInterval) {
        previousMillis = currentMillis;
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState);
    }
}
