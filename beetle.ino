/* BEETLE
 * Operates motion-sensor driven beetle
 * Beetle turns it head and clicks its mandibles at those walking by
 * Beetle is a product of the Backstage Beetles (Otto, Ruisha, Wes, Nik, John)
 * Last Updated 10 April 2025
 *
 * NOTICE:
 * The head-turning mechanism and eye LEDs are things we plan to implement later on as we continue to develop the beetle.
 * The code for these systems is complete, but it has not actually been constructed and tested.
*/

// Include Servo Library:
#include <Servo.h>

// Define Pins:
#define headTurnPin 10
#define mandiblesPin 9
#define eyesLED 13
#define leftEye A0
#define rightEye A1
#define potentiometerPin A2

// Define Values:
#define left 40 // Head turn angle to look left
#define center 90 // Head turn angle to look straight
#define right 140 // Head turn angle to look right
#define open 55 // Mandibles max open angle
#define close 120 // Mandibles min open angle


// Object Declarations:
Servo headTurn;
Servo mandibles;

// Dynamic Variables:
int eyesBrightness = 255; // 0 to 255, brightness of eye LEDs
int setEyesBrightness = 0; // The brightness we want the eyes LEDs to be at (will transition to this brightness slowly)
double headAngle = center; // The initial head angle
double mandiblesAngle = open; // The initial mandible state
double turnSpeed = 1; // Degrees per ten milliseconds (step size for turning the head)
double mandibleOpenSpeed = 2; // Degrees per ten milliseconds (step size for opening mandibles)
double mandibleCloseSpeed = 4; // Degrees per ten milliseconds (step size for closing mandibles)
bool mandiblesDone; // Mandibles reached the set position
int motionDuration = 2000; // Milliseconds (any motion detected must be registered for at least this much time)
unsigned long lastMotion = 0;
int lastMotionType;

// Motion Function:
#define NONE 0
#define LEFT 1
#define RIGHT 2
#define BOTH 3
bool motionL() { // Return true if motion dectected in left eye
  return (analogRead(leftEye) > 700);
}
bool motionR() { // Return true if motion dectected in right eye
  return (analogRead(rightEye) > 700);
}
int motion() { // Return summary of motion (can be NONE, LEFT, RIGHT, or BOTH)
  if (millis() - lastMotion > motionDuration) { // If exceeded time limit since last motion received
    bool l = motionL();
    bool r = motionR();
    if (random(0, 2*analogRead(potentiometerPin)) == 0 || analogRead(potentiometerPin) < 100) { // Let potentiometer adjust frequency of scare
      motionDuration = random(500, 5000);
      if (l) {
        lastMotion = millis();
        if (r) {
          lastMotionType = BOTH;
          return BOTH;
        } else {
          lastMotionType = LEFT;
          return LEFT;
        }
      } else if (r) {
        lastMotion = millis();
        lastMotionType = RIGHT;
        return RIGHT;
      } else {
        return NONE;
      }
    } else {
      return NONE;
    }
  } else { // Motion was detected and processed recently
    return lastMotionType;
  }
}

void setup() { // Runs immediately when beetle is booted
  pinMode(eyesLED, OUTPUT);
  analogWrite(eyesLED, eyesBrightness);
  headTurn.attach(headTurnPin);
  headTurn.write(center);
  mandibles.attach(mandiblesPin);
  // Mandibles close, wait, and then open on boot as a reset indicator:
  mandibles.write(close);
  delay(motionDuration);
  mandibles.write(open);
}

void loop() { // The actual code that does the things we want forever:

  delay(10); // Wait ten milliseconds (so we only run loop() once every ten milliseconds)

  // Manage Eye LED Brightness:
  if (setEyesBrightness > eyesBrightness) { // If we set the eyes to be brighter
    eyesBrightness += 4; // Increase brightness by 4/255
  } else if (setEyesBrightness < eyesBrightness) { // If we set the eyes to be less bright
    eyesBrightness -= 1; // Decrease brightness by 1/255
  }
  analogWrite(eyesLED, eyesBrightness);

  // Manage Mandibles:
  if (abs(mandiblesAngle - mandibles.read()) > mandibleCloseSpeed) { // If the mandibles aren't where we have set them to be
    mandiblesDone = false; // Note that we are not at the desired position
    // Go towards the desired postition:
    if (mandiblesAngle > mandibles.read()) {
      mandibles.write(mandibles.read() + mandibleCloseSpeed);
    } else if (mandiblesAngle < mandibles.read()) {
      mandibles.write(mandibles.read() - mandibleOpenSpeed);
    }
  } else {
    mandiblesDone = true;
  }

  // Manage Head:
  if (abs(headAngle - headTurn.read()) > turnSpeed) { // If the head isn't where we have set it to be
    // Go towards the desired position:
    if (headAngle > headTurn.read()) {
      headTurn.write(headTurn.read() + turnSpeed);
    } else if (headAngle < headTurn.read()) {
      headTurn.write(headTurn.read() - turnSpeed);
    }
  }

  // Think:
  if (motion() == NONE) {
    // Return to default state:
    headAngle = center;
    turnSpeed = 0.5; // Slowly
    mandiblesAngle = open;
    setEyesBrightness = 0;
  } else {
    // Act according to motion detected:
    setEyesBrightness = 255;
    if (mandiblesDone) { // If the mandibles reached the set position, swap the position to the other position:
      if (mandiblesAngle == open) {
        mandiblesAngle = close;
      } else {
        mandiblesAngle = open;
      }
    }
    if (motion() == LEFT) {
      headAngle = left;
      if (random(0,5) == 0) {
        turnSpeed = 4;
      } else {
        turnSpeed = 1;
      }
    } else if (motion() == RIGHT) {
      headAngle = right;
      if (random(0,5) == 0) {
        turnSpeed = 4;
      } else {
        turnSpeed = 1;
      }
    } else if (motion() == BOTH) {
      headAngle = center;
      if (random(0,5) == 0) {
        turnSpeed = 4;
      } else {
        turnSpeed = 1;
      }
    }
  }
}
