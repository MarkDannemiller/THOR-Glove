#include <Arduino.h>
#include "../src/finger.h"

// Pin definitions
#define SERVO_PIN 5
#define FSR_PIN A0
#define STRETCH_PIN A12
#define CURRENT_PIN A7
#define LED_PIN 13

// Constants
#define VCC 3.3
#define SERVO_MIN_ANGLE 0
#define SERVO_MAX_ANGLE 130
#define SERVO_MIN_PULSE 500
#define SERVO_MAX_PULSE 2500
#define UPDATE_INTERVAL 200  // milliseconds

// Create finger object
Finger finger(SERVO_PIN, FSR_PIN, STRETCH_PIN, CURRENT_PIN, 
              VCC, 12, // VCC = 3.3V, 12-bit ADC
              SERVO_MIN_ANGLE, SERVO_MAX_ANGLE, 
              SERVO_MIN_PULSE, SERVO_MAX_PULSE,
              6.0, // 6 inch relaxed length
              350.0); // 350 ohms per inch when relaxed

unsigned long lastUpdate = 0;
unsigned long demoTimer = 0;
bool demoState = false;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // Turn on LED to indicate startup
  
  // Allow serial monitor to connect
  delay(2000);
  
  Serial.println("THOR Glove Finger Demo");
  Serial.println("======================");
  
  // Initialize finger
  finger.begin();
  
  // Initial position
  finger.setAngle(SERVO_MIN_ANGLE);
  
  Serial.println("Finger initialized");
  digitalWrite(LED_PIN, LOW); // Turn off LED after initialization
  
  demoTimer = millis();
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Update sensor readings every UPDATE_INTERVAL ms
  if (currentMillis - lastUpdate >= UPDATE_INTERVAL) {
    lastUpdate = currentMillis;
    
    // Update all sensor readings
    finger.update();
    
    // Print sensor values
    Serial.println("\n--- Sensor Readings ---");
    
    // FSR values
    Serial.print("FSR: Raw=");
    Serial.print(finger.getFSRRaw());
    Serial.print(", Voltage=");
    Serial.print(finger.getFSRVoltage(), 3);
    Serial.print("V, Force=");
    Serial.print(finger.getFSRForce(), 1);
    Serial.println("N");
    
    // Stretch sensor values
    Serial.print("Stretch: Raw=");
    Serial.print(finger.getStretchRaw());
    Serial.print(", Voltage=");
    Serial.print(finger.getStretchVoltage(), 3);
    Serial.print("V, Resistance=");
    Serial.print(finger.getStretchResistance(), 1);
    Serial.print("Ω, Length=");
    Serial.print(finger.getStretchLength(), 1);
    Serial.println("\"");
    
    // Current sensor values
    Serial.print("Current: Raw=");
    Serial.print(finger.getCurrentRaw());
    Serial.print(", Current=");
    Serial.print(finger.getCurrentValue() * 1000, 1); // Convert to mA
    Serial.println("mA");
    
    // Servo angle
    Serial.print("Servo: Angle=");
    Serial.println(finger.getAngle(), 1);
    
    // Demo: toggle between minimum and maximum angle every 2.5 seconds
    if (currentMillis - demoTimer >= 5000) {
      demoTimer = currentMillis;
      demoState = false;
    }
    else if (currentMillis - demoTimer >= 2500 && !demoState) {
      finger.setAngle(SERVO_MAX_ANGLE);
      demoState = true;
      digitalWrite(LED_PIN, HIGH);
    }
    else if (currentMillis - demoTimer < 2500 && demoState) {
      finger.setAngle(SERVO_MIN_ANGLE);
      demoState = false;
      digitalWrite(LED_PIN, LOW);
    }
    
    // Respond to FSR pressure (uncomment to enable)
    /*
    float force = finger.getFSRForce();
    if (force > 5.0) { // Only respond to significant force
      // Map force to angle (adjust thresholds as needed)
      float targetAngle = map(force, 5.0, 50.0, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
      finger.setAngle(targetAngle);
    }
    */
  }
} 