#include <Servo.h>

Servo radar;

// Pins
const int servoPin = 6;
const int westLED = 3;
const int northLED = 5;
const int eastLED = 11;
const int button = 2;
const int trigPin = 7;
const int echoPin = 8;

// Variables
int angle = 0;
int direction = 1;
bool lastButtonState = LOW;

// Smoothed distance reading (starts at "far away" so LEDs start off)
float smoothedDistance = 50.0;

// Smoothing factor: smaller = smoother/slower to react, larger = snappier/noisier.
const float smoothingFactor = 0.2;

unsigned long lastPingTime = 0;
const unsigned long pingInterval = 60; // ms

void setup() {
  radar.attach(servoPin);

  pinMode(westLED, OUTPUT);
  pinMode(northLED, OUTPUT);
  pinMode(eastLED, OUTPUT);

  pinMode(button, INPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  radar.write(angle);
}

void loop() {

  // ==========================
  // Ultrasonic Sensor
  // ==========================

  if (millis() - lastPingTime >= pingInterval) {
    lastPingTime = millis();

    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);

    digitalWrite(trigPin, LOW);

    // Timeout (~30ms, good for ~5m) so a missed echo doesn't hang the loop for a full second
    long duration = pulseIn(echoPin, HIGH, 30000);

    float rawDistance;
    if (duration == 0) {
      // No echo received -> nothing in range, treat as "far away" (not "very close")
      rawDistance = 999.0;
    } else {
      rawDistance = duration * 0.0343 / 2.0;
    }

    // Low-pass filter
    smoothedDistance += smoothingFactor * (rawDistance - smoothedDistance);
  }

  // Convert distance into brightness multiplier
  // 2 cm = 1.0 (100%)
  // 50 cm = 0.0 (0%)
  float multiplier = (50.0 - smoothedDistance) / 48.0;

  if (multiplier > 1.0) multiplier = 1.0;
  if (multiplier < 0.0) multiplier = 0.0;


  // ==========================
  // Button
  // ==========================

  bool currentButtonState = digitalRead(button);

  // Detect button press
  if (currentButtonState == HIGH && lastButtonState == LOW) {
    direction *= -1;
  }

  lastButtonState = currentButtonState;


  // ==========================
  // Servo Movement
  // ==========================

  if (currentButtonState == HIGH) {

    angle += direction;

    // Keep angle between 0 and 180
    if (angle > 180)
      angle = 180;

    if (angle < 0)
      angle = 0;

    delay(20);
  }

  // Keep servo at current position
  radar.write(angle);


// ==========================
// LED Brightness (Linear Interpolation)
// Flipped Left/Right
// ==========================

float westBrightness = 0;
float northBrightness = 0;
float eastBrightness = 0;

// East -> North
if (angle <= 90) {

  eastBrightness = (90.0 - angle) / 90.0;
  northBrightness = angle / 90.0;

}

// North -> West
else {

  northBrightness = (180.0 - angle) / 90.0;
  westBrightness = (angle - 90.0) / 90.0;

}

// Apply distance multiplier
westBrightness *= 255 * multiplier;
northBrightness *= 255 * multiplier;
eastBrightness *= 255 * multiplier;


// Send brightness to LEDs
analogWrite(westLED, (int)westBrightness);
analogWrite(northLED, (int)northBrightness);
analogWrite(eastLED, (int)eastBrightness);
}
