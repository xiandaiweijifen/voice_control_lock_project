#include <Wire.h>
#include <Kalman.h>
#include <MPU6050.h>

const int trigPin = 2;      // Trigger pin for ultrasonic sensor
const int echoPin = 3;      // Echo pin for ultrasonic sensor
const int pirPin = 4;       // PIR sensor pin
const int alarmPin = 6;     // Alarm pin
const int warnPin = 7;      // Warning pin
unsigned long pirTime = 0;  // Time when PIR sensor detects motion

long duration, distance;
int pirState;

// Initialize Kalman filter with 3 parameters
Kalman kalmanX;
Kalman kalmanY;
Kalman kalmanZ;

// Initialize MPU6050 object
MPU6050 mpu;

// Initialize angles
double roll, pitch;

void getDistance() {
  // Generate a 10 microsecond high pulse signal to start distance measurement
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Measure the duration of the ultrasonic signal echo using pulseIn function, in microseconds
  duration = pulseIn(echoPin, HIGH);

  // Calculate distance in centimeters
  distance = duration / 58.0;

  // Print distance value to serial monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
}

void setup() {
  // Start serial communication for debugging
  Serial.begin(115200);

  // Initialize MPU6050 object
  mpu.initialize();

  // Calculate Q and R matrices for Kalman filter
  kalmanX.setAngle(roll);
  kalmanY.setAngle(pitch);
  kalmanZ.setAngle(0);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(pirPin, INPUT);
  pinMode(alarmPin, OUTPUT);
  pinMode(warnPin, OUTPUT);
}


void loop() {
  // Get acceleration and gyroscope data from MPU6050
  int16_t ax, ay, az;
  int16_t gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  pirState = digitalRead(pirPin);

  pirTime = millis() + 20000;

  // If PIR sensor does not detect any motion, reset the tim
  if (pirState == LOW) {
    pirTime = millis();
  }

  // Convert raw data to angle values
  double accX = (double)ax / 16384;
  double accY = (double)ay / 16384;
  double accZ = (double)az / 16384;
  double gyroX = (double)gx / 131;
  double gyroY = (double)gy / 131;
  double gyroZ = (double)gz / 131;

  // Calculate angles from acceleration and gyroscope values
  double accRoll = atan2(accY, accZ) * 180 / PI;
  double accPitch = atan2(-accX, sqrt(accY * accY + accZ * accZ)) * 180 / PI;
  roll = kalmanX.getAngle(accRoll, gyroX, 0.01);
  pitch = kalmanY.getAngle(accPitch, gyroY, 0.01);

  // Print angle values to serial monitor
  Serial.print("Roll: ");
  Serial.print(roll);
  Serial.print(", Pitch: ");
  Serial.println(pitch);

  if (abs(roll) > 20 || abs(pitch) > 20) {
    Serial.println("The lock is being broken!");
    digitalWrite(warnPin, HIGH);
  }

  // If the PIR sensor detects an object for more than 20 seconds continuously, an alarm is triggered
  if (millis() > pirTime) {
    digitalWrite(warnPin, HIGH);
  }

  getDistance();
  delay(500);
}
