/*
THE DOZERS -- Colebot Final Code

Dennis Porter
Owen Rolo
Lily Orth
Carlton Engelhardt
*/

// SETTINGS
const int configuration = 4;             // 1 for original, 2 for autopilot, 3 for dance, 4 for buttons with joystick
const int motorControlConfiguration = 2; // 1 for original, 2 for modded
// END SETTING

// LIBRARIES
#include "WifiPort.h" // Include WifiPort library for WiFi communication
#include <Servo.h>    // Include Servo library for servo motor control
#include <Arduino.h>  // Include Arduino core library
// END LIBRARIES

// STRUCTURE PACKET
struct DataPacket
{
    int AnalogCheck;          // Timestamp or analog check value
    int movementJoystick_y;   // Y-axis value of movement joystick
    int movementJoystick_x;   // X-axis value of movement joystick
    int armJoystick;          // Value of arm joystick axis
    int clampButton;          // State of clamp button
    int spinLeft;             // State of spin left button
    int spinRight;            // State of spin right button
    int armButton;            // State of arm button
    int clampJoystick;        // Value of clamp joystick axis
} data;
// END STRUCTURE PACKET

// START WIFI DECLARATIONS
WifiPort<DataPacket> WifiSerial;
WifiPortType portType = WifiPortType::Receiver; // WifiPortType::Transmitter, WifiPortType::Receiver, WifiPortType::Emulator
// END WIFI DECLARATIONS

// START RECEIVER DECLARATIONS
// int spinLeftLastState = 0;
// int spinRightLastState = 0;
float speedOf255 = 45.0 / 5.3;
const float pi = 3.1415926535897932384626433832795;

const int maxTurnSubtracter = 200; // Maximum adjustment for turning
const int minTurnSubtracter = 50;  // Minimum adjustment for turning
const int upperJoystickLimit = 529; // Upper limit for joystick neutral zone
const int lowerJoystickLimit = 505; // Lower limit for joystick neutral zone
const int joystickCustomMod = 100; // Custom modifier for joystick sensitivity

const int dumpServoPos = 135; // Servo position for dumping
const int objectServoPos = 65; // Servo position for grabbing objects
const int clampClosed = 76;    // Servo position for clamp closed
const int clampOpen = 20;      // Servo position for clamp open
const int spinSpeed = 150;     // Speed for spinning motors

/*
const int dumpServoPos = 120; // Servo position for dumping
const int objectServoPos = 71; // Servo position for grabbing objects
const int clampClosed = 64;    // Servo position for clamp closed
const int clampOpen = 20;      // Servo position for clamp open
const int spinSpeed = 150;     // Speed for spinning motors
*/
int servo1Limits[2] = {objectServoPos, dumpServoPos + 20}; // Limits for servo1 movement
int servo2Limits[2] = {clampOpen - 5, clampClosed}; // Limits for servo2 movement

class DCMotor
{
public:
    DCMotor(int pwmPin, int pin1, int pin2) // Constructor to initialize motor pins
        : pwmPin(pwmPin), pin1(pin1), pin2(pin2)
    {
        pinMode(pwmPin, OUTPUT); // Set PWM pin as output
        pinMode(pin1, OUTPUT);  // Set pin1 as output
        pinMode(pin2, OUTPUT);  // Set pin2 as output
    }

    void setSpeed(int speed) // Set motor speed and direction
    {
        analogWrite(pwmPin, abs(speed)); // Write PWM signal based on speed
        if (speed > 0) // Forward direction
        {
            digitalWrite(pin1, HIGH); // Set pin1 HIGH
            digitalWrite(pin2, LOW);  // Set pin2 LOW
        }
        else if (speed < 0) // Reverse direction
        {
            digitalWrite(pin1, LOW);  // Set pin1 LOW
            digitalWrite(pin2, HIGH); // Set pin2 HIGH
        }
        else // Stop motor
        {
            digitalWrite(pin1, LOW);  // Set pin1 LOW
            digitalWrite(pin2, LOW);  // Set pin2 LOW
        }
    }

private:
    int pwmPin; // PWM pin for speed control
    int pin1;   // Pin1 for motor direction
    int pin2;   // Pin2 for motor direction
};

const int servo1Pin = A0; // 9
const int servo2Pin = A1; // 10

DCMotor motor2(6, 5, 7); // DCMotor motor2(5, 4, 3); // RIGHT
DCMotor motor1(3, 2, 4); // DCMotor motor1(6, 7, 8); // LEFT

Servo servo1;
Servo servo2;

int lastArmServoPos = 0;
int lastClampServoPos = 0;

void setArmPos(int newPos) // Set the arm servo position with constraints
{
    lastArmServoPos = constrain(newPos, servo1Limits[0], servo1Limits[1]); // Constrain position within servo limits
    servo1.write(lastArmServoPos); // Write the constrained position to the servo
    delay(20); // Small delay for servo movement
}
void setClampPos(int newPos) // Set the clamp servo position with constraints
{
    lastClampServoPos = constrain(newPos, servo2Limits[0], servo2Limits[1]); // Constrain position within servo limits
    servo2.write(lastClampServoPos); // Write the constrained position to the servo
    delay(20); // Small delay for servo movement
}
void setArmPosDelayless(int newPos) // Set the arm servo position without additional delay
{
    lastArmServoPos = constrain(newPos, servo1Limits[0], servo1Limits[1]); // Constrain position within servo limits
    servo1.write(lastArmServoPos); // Write the constrained position to the servo
    
}
void setClampPosDelayless(int newPos) // Set the clamp servo position without additional delay
{
    lastClampServoPos = constrain(newPos, servo2Limits[0], servo2Limits[1]); // Constrain position within servo limits
    servo2.write(lastClampServoPos); // Write the constrained position to the servo
    
}

bool armButtonLastState = 0; // Last state of the arm button
bool clampButtonLastState = 0; // Last state of the clamp button
bool isClampClosed = 0; // Indicates if the clamp is closed
bool isArmDown = 0; // Indicates if the arm is down

// START AUTOPILOT FUNCTIONS
void forward(int length)
{
    float delayX = 1 / speedOf255 * length * 1000; // Calculate delay based on speed and length
    Serial.print("Forward (in): "); // Print forward movement details
    Serial.println(length);
    Serial.println(delayX);
    motor1.setSpeed(235); // Set motor1 speed for forward movement
    motor2.setSpeed(-255); // Set motor2 speed for forward movement
    delay(delayX); // Delay for calculated duration
    motor1.setSpeed(0); // Stop motor1
    motor2.setSpeed(0); // Stop motor2
    delay(500); // Small delay after stopping
}

void turn(int radians, int multiplier = 1)
{ // Turn function with optional multiplier for direction
    float radius = 5.0 / 2.0; // Calculate radius for turning
    float angularSpeed = speedOf255 / radius; // Calculate angular speed
    float delayX = 1 / angularSpeed * radians * 1000; // Calculate delay based on radians
    Serial.print("Turn (rads): "); // Print turning details
    Serial.println(radians);
    Serial.println(delayX);
    motor1.setSpeed(-255 * multiplier); // Set motor1 speed for turning
    motor2.setSpeed(-255 * multiplier); // Set motor2 speed for turning
    delay(delayX); // Delay for calculated duration
    motor1.setSpeed(0); // Stop motor1
    motor2.setSpeed(0); // Stop motor2
}

void turn180()
{
    float radius = 10.6 / 2.0; // Calculate radius for 180-degree turn
    float angularSpeed = speedOf255 / radius; // Calculate angular speed
    float delayX = 1 / angularSpeed * degToRad(90) * 1000; // Calculate delay for 180-degree turn
    Serial.print("Turn (rads): pi"); // Print 180-degree turn details
    Serial.println(delayX);
    motor1.setSpeed(-255); // Set motor1 speed for turning
    motor2.setSpeed(-255); // Set motor2 speed for turning
    delay(delayX); // Delay for calculated duration
    motor1.setSpeed(0); // Stop motor1
    motor2.setSpeed(0); // Stop motor2
    delay(500); // Small delay after stopping
}

void turn90(int mod)
{
    float radius = 4.9 / 2.0; // Calculate radius for 90-degree turn
    float angularSpeed = speedOf255 / radius; // Calculate angular speed
    float delayX = 1 / angularSpeed * degToRad(90) * 1000; // Calculate delay for 90-degree turn
    Serial.print("Turn (rads): pi/2"); // Print 90-degree turn details
    Serial.println(delayX);
    motor1.setSpeed(-255 * mod); // Set motor1 speed for turning
    motor2.setSpeed(-255 * mod); // Set motor2 speed for turning
    delay(delayX); // Delay for calculated duration
    motor1.setSpeed(0); // Stop motor1
    motor2.setSpeed(0); // Stop motor2
    delay(500); // Small delay after stopping
}

// CHA CHA SLIDE; FUNCTION NAMES ARE BASED ON THE TIME IN THE SONG AND DESCRIBE THE ACTION
void openClap()
{
    Serial.println("Clap");
    setClampPos(clampClosed);
}
void closeClap()
{
    Serial.println("Clap");
    setClampPos(clampOpen);
}
int toDelay(int seconds, int frame)
{
    Serial.println("Delay");
    return seconds * 1000 + (frame * 1000.0 / 60.0);
}
void intro()
{
    Serial.println("Cha Cha Slide");
    delay(toDelay(17, 35));
}
void funky1()
{ // 17:35s to 24:42s
    Serial.println("Funky");
    motor1.setSpeed(150);
    motor2.setSpeed(-150);
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(1000);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(1000);
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(1000);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    motor1.setSpeed(-150);
    motor2.setSpeed(150);
    delay(1000);
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(1000);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(1000);
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    motor1.setSpeed(0);
    motor2.setSpeed(0);
    delay(500);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(toDelay(24, 42) - toDelay(17, 35) - 6500);
}
void everybodyClapYourHands()
{ // 24:42s to 33:25s
    Serial.println("Clap");

    setClampPosDelayless(clampClosed);
    delay(700);
    setClampPosDelayless(clampOpen);
    delay(700);
    setClampPosDelayless(clampClosed);
    delay(700);
    setClampPosDelayless(clampOpen);
    delay(700);
    setClampPosDelayless(clampClosed);
    delay(700);
    setClampPosDelayless(clampOpen);
    delay(700);
    setClampPosDelayless(clampClosed);
    delay(700);
    setClampPosDelayless(clampOpen);
    delay(700);
    setClampPosDelayless(clampClosed);
    delay(700);
    setClampPosDelayless(clampOpen);
    delay(700); // 7000
    setClampPosDelayless(clampClosed);
    delay(700);
    setClampPosDelayless(clampOpen);
    delay(700);
    setClampPosDelayless(clampClosed);
    delay(toDelay(33, 25) - toDelay(24, 42) - (700 * 12));
}

void toTheLeft1()
{ // 33:25s to 34:50s
    float radius = 4.9 / 2.0;
    float mod = 1;
    float angularSpeed = speedOf255 / radius;
    float delayX = 1 / angularSpeed * degToRad(90) * 1000;
    Serial.print("Turn (rads): pi/2");
    Serial.println(delayX);
    motor1.setSpeed(-255 * mod);
    motor2.setSpeed(-255 * mod);
    delay(delayX);
    motor1.setSpeed(0);
    motor2.setSpeed(0);
    delay(toDelay(34, 50) - toDelay(33, 25) - (delayX));
}
void takeItBack()
{ // 34:50 to 36:50
    Serial.println(1000);
    motor1.setSpeed(-235);
    motor2.setSpeed(255);
    delay(1000);
    motor1.setSpeed(0);
    motor2.setSpeed(0);
    delay(toDelay(36, 50) - toDelay(34, 50) - 1000);
}
void oneHopThisTime()
{ // 36:50 to 38:50
    delay(200);
    Serial.println("Hop");
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(1000);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(toDelay(38, 50) - toDelay(36, 50) - 1200);
}
void rightFootLeftStomp()
{ // 38:50 to 40:25
    delay(500);
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(500);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(toDelay(40, 25) - toDelay(38, 50) - 1000);
}
void leftFootLeftStomp()
{ // 40:30 to 43:00
    delay(500);
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(500);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(toDelay(43, 00) - toDelay(40, 25) - 1000);
}
void chaChaRealSmooth()
{ // 43:00 to 46:26
    Serial.println("Cha Cha");
    motor1.setSpeed(-150);
    motor2.setSpeed(-150);
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(700);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(700);
    motor1.setSpeed(150);
    motor2.setSpeed(150);
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(700);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(700);
    motor1.setSpeed(0);
    motor2.setSpeed(0);
    delay(toDelay(46, 26) - toDelay(43, 0) - (700 * 4));
}
void turnItOut()
{ // 46:26 to 48:15
    float radius = 4.9 / 2.0;
    float mod = -1;
    float angularSpeed = speedOf255 / radius;
    float delayX = 1 / angularSpeed * degToRad(90) * 1000;
    Serial.print("Turn (rads): pi/2");
    Serial.println(delayX);
    motor1.setSpeed(-255 * mod);
    motor2.setSpeed(-255 * mod);
    delay(delayX);
    motor1.setSpeed(0);
    motor2.setSpeed(0);
    delay(toDelay(48, 15) - toDelay(46, 26) - (delayX));
}
void toTheLeft2()
{ // 48:15 to 49:50
    float radius = 4.9 / 2.0;
    float mod = 1;
    float angularSpeed = speedOf255 / radius;
    float delayX = 1 / angularSpeed * degToRad(90) * 1000;
    Serial.print("Turn (rads): pi/2");
    Serial.println(delayX);
    motor1.setSpeed(-255 * mod);
    motor2.setSpeed(-255 * mod);
    delay(delayX);
    motor1.setSpeed(0);
    motor2.setSpeed(0);
    delay(toDelay(49, 50) - toDelay(48, 15) - (delayX));
}
void takeItBack2()
{ // 49:50 to 51:50
    Serial.println(1000);
    motor1.setSpeed(-235);
    motor2.setSpeed(255);
    delay(1000);
    motor1.setSpeed(0);
    motor2.setSpeed(0);
    delay(toDelay(51, 50) - toDelay(49, 50) - 1000);
}
void oneHopThisTime2()
{ // 51:50 to 53:50
    delay(200);
    Serial.println("Hop");
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(1000);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(toDelay(53, 50) - toDelay(51, 50) - 1200);
}
void rightFootLeftStomp2()
{ // 53:50 to 55:50
    delay(500);
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(500);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(toDelay(55, 50) - toDelay(53, 50) - 1000);
}
void leftFootLeftStomp2()
{ // 55:50 to 57:50
    delay(500);
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(500);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(toDelay(57, 50) - toDelay(55, 50) - 1000);
}
void chaChaNowYall()
{ // 57:50 to 1:01:00
    Serial.println("Cha Cha");
    motor1.setSpeed(-150);
    motor2.setSpeed(-150);
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(700);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(700);
    motor1.setSpeed(150);
    motor2.setSpeed(150);
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(700);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(700);
    motor1.setSpeed(0);
    motor2.setSpeed(0);
    delay(toDelay(61, 0) - toDelay(57, 50) - (700 * 4));
}
void toTheRight()
{ // 1:01:00 to 1:02:50
    float radius = 4.9 / 2.0;
    float mod = -1;
    float angularSpeed = speedOf255 / radius;
    float delayX = 1 / angularSpeed * degToRad(90) * 1000;
    Serial.print("Turn (rads): pi/2");
    Serial.println(delayX);
    motor1.setSpeed(-255 * mod);
    motor2.setSpeed(-255 * mod);
    delay(delayX);
    motor1.setSpeed(0);
    motor2.setSpeed(0);
    delay(toDelay(62, 50) - toDelay(61, 0) - (delayX));
}
void toTheLeft3()
{ // 1:02:50 to 1:04:50
    float radius = 4.9 / 2.0;
    float mod = 1;
    float angularSpeed = speedOf255 / radius;
    float delayX = 1 / angularSpeed * degToRad(90) * 1000;
    Serial.print("Turn (rads): pi/2");
    Serial.println(delayX);
    motor1.setSpeed(-255 * mod);
    motor2.setSpeed(-255 * mod);
    delay(delayX);
    motor1.setSpeed(0);
    motor2.setSpeed(0);
    delay(toDelay(64, 50) - toDelay(62, 50) - (delayX));
}
void takeItBack3()
{ // 1:04:50 to 1:06:50
    Serial.println(1000);
    motor1.setSpeed(-235);
    motor2.setSpeed(255);
    delay(1000);
    motor1.setSpeed(0);
    motor2.setSpeed(0);
    delay(toDelay(66, 50) - toDelay(64, 50) - 1000);
}
void oneHopThisTime3()
{ // 1:06:39 to 1:08:26
    delay(200);
    Serial.println("Hop");
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(1000);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(toDelay(68, 26) - toDelay(66, 39) - 1200);
}
void oneHopThisTime4()
{ // 1:08:26 to 1:10:21
    delay(200);
    Serial.println("Hop");
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(1000);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(toDelay(70, 21) - toDelay(68, 26) - 1200);
}
void rightFootTwoStomps()
{ // 1:10:31 to 1:12:21
    delay(500);
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(500);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(toDelay(72, 21) - toDelay(70, 21) - 1000);
}
void leftFootTwoStomps()
{ // 1:12:21 to 1:14:11
    delay(500);
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(500);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(toDelay(74, 11) - toDelay(72, 21) - 1000);
}
void slideToTheLeft()
{ // 1:14:11 to 1:16:00
    float radius = 4.9 / 2.0;
    float mod = 1;
    float angularSpeed = speedOf255 / radius;
    float delayX = 1 / angularSpeed * degToRad(90) * 1000;
    Serial.print("Turn (rads): pi/2");
    Serial.println(delayX);
    motor1.setSpeed(-255 * mod);
    motor2.setSpeed(-255 * mod);
    delay(delayX);
    motor1.setSpeed(0);
    motor2.setSpeed(0);
    delay(toDelay(76, 0) - toDelay(74, 11) - (delayX));
}
void slideToTheRight()
{ // 1:16:00 to 1:17:50
    float radius = 4.9 / 2.0;
    float mod = -1;
    float angularSpeed = speedOf255 / radius;
    float delayX = 1 / angularSpeed * degToRad(90) * 1000;
    Serial.print("Turn (rads): pi/2");
    Serial.println(delayX);
    motor1.setSpeed(-255 * mod);
    motor2.setSpeed(-255 * mod);
    delay(delayX);
    motor1.setSpeed(0);
    motor2.setSpeed(0);
    delay(toDelay(77, 50) - toDelay(76, 0) - (delayX));
}
void crissCross()
{ // 1:17:50 to 1:19:50
    setClampPosDelayless(clampClosed);
    delay(500);
    setClampPosDelayless(clampOpen);
    delay(500);
    setClampPosDelayless(clampClosed);
    delay(500);
    setClampPosDelayless(clampOpen);
    delay(toDelay(79, 50) - toDelay(77, 50) - (500 * 4));
}
void crissCross2()
{ // 1:19:50 to 1:21:50
    setClampPosDelayless(clampClosed);
    delay(500);
    setClampPosDelayless(clampOpen);
    delay(500);
    setClampPosDelayless(clampClosed);
    delay(500);
    setClampPosDelayless(clampOpen);
    delay(toDelay(81, 50) - toDelay(79, 50) - (500 * 4));
}
void chaChaSlide2()
{ // 1:21:50 to 1:26:50
    Serial.println("Cha Cha");
    motor1.setSpeed(-150);
    motor2.setSpeed(-150);
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(700);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(700);
    motor1.setSpeed(150);
    motor2.setSpeed(150);
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(700);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(700);
    motor1.setSpeed(0);
    motor2.setSpeed(0);
    delay(toDelay(86, 50) - toDelay(81, 50) - (700 * 4));
}
void toTheLeft4()
{ // 1:26:50 to 1:28:50
    float radius = 4.9 / 2.0;
    float mod = 1;
    float angularSpeed = speedOf255 / radius;
    float delayX = 1 / angularSpeed * degToRad(90) * 1000;
    Serial.print("Turn (rads): pi/2");
    Serial.println(delayX);
    motor1.setSpeed(-255 * mod);
    motor2.setSpeed(-255 * mod);
    delay(delayX);
    motor1.setSpeed(0);
    motor2.setSpeed(0);
    delay(toDelay(88, 50) - toDelay(86, 50) - (delayX));
}
void takeItBack4()
{ // 1:28:50 to 1:30:50
    Serial.println(1000);
    motor1.setSpeed(-235);
    motor2.setSpeed(255);
    delay(1000);
    motor1.setSpeed(0);
    motor2.setSpeed(0);
    delay(toDelay(90, 50) - toDelay(88, 50) - 1000);
}
void twoHopsThisTime()
{ // 1:30:50 to 1:32:00
    delay(200);
    Serial.println("Hop");
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(1000);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(toDelay(92, 0) - toDelay(90, 50) - 1200);
}
void twoHopsThisTime2()
{ // 1:32:00 to 1:33:50
    delay(200);
    Serial.println("Hop");
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(1000);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(toDelay(93, 10) - toDelay(92, 0) - 1200);
}
void rightFootTwoStomps2()
{ // 1:33:50 to 1:35:00
    delay(500);
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(500);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(toDelay(95, 0) - toDelay(93, 10) - 1000);
}
void leftFootTwoStomps2()
{ // 1:35:00 to 1:36:20
    delay(200);
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(200);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(toDelay(96, 20) - toDelay(95, 0) - 1000);
}
void handsOnYourKnees()
{ // 1:36:20 to 1:39:50
    Serial.println("Hands on your knees");
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(2000);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(toDelay(99, 50) - toDelay(96, 20) - 200);
}
void getFunkyWitIt()
{ // 1:39:50 to 1:46:50
    Serial.println("Get Funky Wit It");
    motor1.setSpeed(-150);
    motor2.setSpeed(-150);
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(1200);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(1200);
    motor1.setSpeed(150);
    motor2.setSpeed(150);
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(1200);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(1200);
    motor1.setSpeed(0);
    motor2.setSpeed(0);
    delay(toDelay(106, 50) - toDelay(99, 50) - (1200 * 4));
}
void chaChaRealSmooth2()
{
    Serial.println("Cha Cha");
    motor1.setSpeed(150);
    motor2.setSpeed(-150);
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(700);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(700);
    motor1.setSpeed(150);
    motor2.setSpeed(150);
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(700);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(700);
    motor1.setSpeed(0);
    motor2.setSpeed(0);
}
void chaChaSlide()
{
    intro();
    funky1();
    everybodyClapYourHands();
    toTheLeft1();
    takeItBack();
    oneHopThisTime();
    rightFootLeftStomp();
    leftFootLeftStomp();
    chaChaRealSmooth();
    turnItOut();
    toTheLeft2();
    takeItBack2();
    oneHopThisTime2();
    rightFootLeftStomp2();
    leftFootLeftStomp2();
    chaChaNowYall();
    toTheRight();
    toTheLeft3();
    takeItBack3();
    oneHopThisTime3();
    oneHopThisTime4();
    rightFootTwoStomps();
    leftFootTwoStomps();
    slideToTheLeft();
    slideToTheRight();
    crissCross();
    crissCross2();
    chaChaSlide2();
    toTheLeft4();
    takeItBack4();
    twoHopsThisTime();
    twoHopsThisTime2();
    rightFootTwoStomps2();
    leftFootTwoStomps2();
    handsOnYourKnees();
    getFunkyWitIt();
    chaChaRealSmooth2();
}
// END CHA CHA

// END AUTOPILOT FUNCTIONS
const int dancePin = 8;
// END RECEIVER DECLARATIONS

// START TRANSMITTER DECLARATIONS

const int movementJoystick_y = A1; // Joystick Y-axis for movement
const int movementJoystick_x = A0; // Joystick X-axis for movement

const int armJoystick = A2; // Joystick for arm control
const int clampJoystick = A3; // Joystick for clamp control
/*
const int movementJoystick_y = A3; // Alternate Joystick Y-axis for movement
const int movementJoystick_x = A2; // Alternate Joystick X-axis for movement

const int armJoystick = A0; // Alternate Joystick for arm control
const int clampJoystick = A1; // Alternate Joystick for clamp control
*/

const int clampButton = A5; // Button for clamp control
const int armButton = A4; // Button for arm control
const int spinLeft = 2; // Button for spinning left
const int spinRight = 12; // Button for spinning right
// END TRANSMITTER DECLARATIONS
float degToRad(float deg) // Convert degrees to radians
{
    return deg * pi / 180.0; // Multiply degrees by pi/180 to get radians
}
void setup()
{
    // DONT USE PIN13 FOR ANY SENSOR OR ACTUATORS
    Serial.begin(115200); // Initialize serial communication at 115200 baud rate
    WifiSerial.begin("PairAP_CBG28", "GiggleSmurfs69", portType); // Initialize WiFi communication with SSID and password
    if (WifiSerial.getPortType() == WifiPortType::Receiver || WifiSerial.getPortType() == WifiPortType::Emulator) // Check if the port type is Receiver or Emulator
    {
        servo1.attach(servo1Pin); // Attach servo1 to its pin
        servo2.attach(servo2Pin); // Attach servo2 to its pin

        setArmPos(dumpServoPos); //(servo1Limits[0]); // Set initial arm position to dump position
        isArmDown = 0; // Initialize arm state as not down

        setClampPos(clampOpen); //(servo2Limits[0]); // Set initial clamp position to open
        isClampClosed = 0; // Initialize clamp state as not closed
        if (configuration == 3) {
          pinMode(dancePin, INPUT_PULLUP);
        }
        if (configuration == 2) // Check if configuration is set to autopilot
        {
            // forward(70); // Example forward movement

            turn180(); // Perform a 180-degree turn
            forward(10); // Move forward by 10 units
            turn90(1); // Perform a 90-degree turn to the right
            forward(15); // Move forward by 15 units
            turn90(1); // Perform another 90-degree turn to the right
            forward(70); // Move forward by 70 units
            turn90(-1); // Perform a 90-degree turn to the left
            forward(20); // Move forward by 20 units
            turn90(-1); // Perform another 90-degree turn to the left
            forward(70); // Move forward by 70 units
            /*
            forward(68); // Move forward by 68 units
            turn(degToRad(180)); // Perform a 180-degree turn in radians
            forward(15); // Move forward by 15 units
            turn(degToRad(90)); // Perform a 90-degree turn in radians
            forward(24); // Move forward by 24 units
            */
        }
    }
    if ((WifiSerial.getPortType() == WifiPortType::Transmitter || WifiSerial.getPortType() == WifiPortType::Emulator)) {
        pinMode(movementJoystick_x, INPUT); // Set movement joystick X-axis as input
        pinMode(movementJoystick_y, INPUT); // Set movement joystick Y-axis as input

        pinMode(armJoystick, INPUT); // Set arm joystick as input
        pinMode(clampJoystick, INPUT); // Set clamp joystick as input

        pinMode(spinLeft, INPUT_PULLUP); // Set spin left button as input with pull-up resistor
        pinMode(spinRight, INPUT_PULLUP); // Set spin right button as input with pull-up resistor

        pinMode(armButton, INPUT_PULLUP); // Set arm button as input with pull-up resistor
        pinMode(clampButton, INPUT_PULLUP); // Set clamp button as input with pull-up resistor
    }
}

void loop()
{
    if (WifiSerial.getPortType() == WifiPortType::Transmitter || WifiSerial.getPortType() == WifiPortType::Emulator)
    {
        WifiSerial.autoReconnect();

        // DATA PACKET
        data.AnalogCheck = millis(); // Set AnalogCheck to the current time in milliseconds
        data.movementJoystick_y = analogRead(movementJoystick_x); // Read Y-axis value of movement joystick
        data.movementJoystick_x = analogRead(movementJoystick_y); // Read X-axis value of movement joystick

        data.armJoystick = analogRead(armJoystick); // Read value of arm joystick
        data.clampJoystick = analogRead(clampJoystick); // Read value of clamp joystick

        data.armButton = !digitalRead(armButton); // Read and invert state of arm button
        data.clampButton = !digitalRead(clampButton); // Read and invert state of clamp button

        data.spinLeft = !digitalRead(spinLeft); // Read and invert state of spin left button
        data.spinRight = !digitalRead(spinRight); // Read and invert state of spin right button
        // END DATA PACKET

        // SERIAL OUTPUT CHECK
        Serial.print("Sending: "); // Print the current AnalogCheck value being sent
        Serial.println(data.AnalogCheck);

        Serial.print("Y-movement: "); // Print the Y-axis value of the movement joystick
        Serial.println(data.movementJoystick_y);
        Serial.print("X-movement: "); // Print the X-axis value of the movement joystick
        Serial.println(data.movementJoystick_x);

        Serial.print("Arm Joystick: "); // Print the value of the arm joystick
        Serial.println(data.armJoystick);
        Serial.print("Clamp Joystick: "); // Print the value of the clamp joystick
        Serial.println(data.clampJoystick);

        Serial.print("Arm Button State: "); // Print the state of the arm button
        Serial.println(data.armButton);
        Serial.print("Clamp Button State: "); // Print the state of the clamp button
        Serial.println(data.clampButton);

        Serial.print("Spin Left: "); // Print the state of the spin left button
        Serial.println(data.spinLeft); // Output the spin left button state
        Serial.print("Spin Right: "); // Print the state of the spin right button
        Serial.println(data.spinRight); // Output the spin right button state
        // END SERIAL OUTPUT CHECK

        if (!WifiSerial.sendData(data))
            Serial.println("Wifi Send Problem");
    }
    if (configuration == 3) // Check if configuration is set to autopilot (dance mode)
        {
            int danceYes = !digitalRead(dancePin); // Retrieve data from WifiSerial
            if (danceYes) // Check if the spinRight button is pressed (right trigger button)
            {   
                chaChaSlide(); // Execute the Cha Cha Slide routine
                delay(1000); // Delay for 1 second after the routine
            }
        }
    if ((WifiSerial.getPortType() == WifiPortType::Receiver || WifiSerial.getPortType() == WifiPortType::Emulator) && WifiSerial.checkForData()) // Check if the port is Receiver/Emulator and data is available
    {
        if (configuration == 1 || configuration == 4) // Check if configuration is set to original or buttons with joystick
        {
            data = WifiSerial.getData(); // Retrieve data from WifiSerial

            // SERIAL OUTPUT CHECK
            Serial.print("Received: "); // Print the received AnalogCheck value
            Serial.println(data.AnalogCheck); // Output the AnalogCheck value

            Serial.print("Y-movement: "); // Print the Y-axis value of the movement joystick
            Serial.println(data.movementJoystick_y); // Output the Y-axis value

            Serial.print("X-movement: "); // Print the X-axis value of the movement joystick
            Serial.println(data.movementJoystick_x); // Output the X-axis value

            Serial.print("Arm Joystick: "); // Print the value of the arm joystick
            Serial.println(data.armJoystick); // Output the arm joystick value

            Serial.print("Clamp Joystick: "); // Print the value of the clamp joystick
            Serial.println(data.clampJoystick); // Output the clamp joystick value

            Serial.print("Arm Button State: "); // Print the state of the arm button
            Serial.println(data.armButton); // Output the arm button state

            Serial.print("Clamp Button State: "); // Print the state of the clamp button
            Serial.println(data.clampButton); // Output the clamp button state

            Serial.print("Spin Left: "); // Print the state of the spin left button
            Serial.println(data.spinLeft); // Output the spin left button state

            Serial.print("Spin Right: "); // Print the state of the spin right button
            Serial.println(data.spinRight); // Output the spin right button state
            // END SERIAL OUTPUT CHECK

            // RECEIVER CODE
            // Motor Direction
            /*int motor1DirectionalModifier = 1;
            int motor2DirectionalModifier = 1;*/

            /*if (data.spinLeft) // > spinLeftLastState)
            {
                motor1DirectionalModifier = -1;
            }
            else
            {
                motor1DirectionalModifier = 1;
            }
            if (data.spinRight) // > spinRightLastState)
            {
                motor2DirectionalModifier = -1;
            }
            else
            {
                motor2DirectionalModifier = 1;
            }*/
            // Motor Movement
            if (configuration == 4) // Check if configuration is set to buttons with joystick
            {
                /*data.armButton = data.spinRight;
                data.clampButton = data.spinLeft;*/
                data.spinRight = 0; // Reset spinRight to 0
                data.spinLeft = 0; // Reset spinLeft to 0
            }
            if (motorControlConfiguration == 1) // Check if motor control configuration is set to original
            {
                if (data.spinLeft) // Check if spinLeft button is pressed
                {
                    motor1.setSpeed(-255); // Set motor1 speed to -255
                    motor2.setSpeed(-255); // Set motor2 speed to -255
                }
                else if (data.spinRight) // Check if spinRight button is pressed
                {
                    motor1.setSpeed(255); // Set motor1 speed to 255
                    motor2.setSpeed(255); // Set motor2 speed to 255
                }
                else // Default motor control logic
                {
                    int yPosition = data.movementJoystick_y; // Get Y-axis joystick position
                    int xPosition = data.movementJoystick_x; // Get X-axis joystick position
                    int motor1Speed; // Variable for motor1 speed
                    int motor2Speed; // Variable for motor2 speed
                    int baseSpeed; // Variable for base speed

                    if (yPosition > upperJoystickLimit || yPosition < lowerJoystickLimit) // Check if Y-axis is outside neutral zone
                    {
                        baseSpeed = map(yPosition, 0, 1023, -255, 255); // Map Y-axis to motor speed range
                    }
                    else
                    {
                        baseSpeed = 0; // Set base speed to 0 if in neutral zone
                    }
                    int turnAdjustment = 0; // Variable for turn adjustment
                    const int customMod = 100; // Custom modifier for joystick sensitivity
                    const int lowerLimMod = lowerJoystickLimit - customMod; // Lower limit with modifier
                    const int upperLimMod = upperJoystickLimit + customMod; // Upper limit with modifier
                    if (!(yPosition < upperLimMod && yPosition > lowerLimMod)) // Check if Y-axis is outside modified neutral zone
                    {
                        if (xPosition < lowerLimMod) // Check if X-axis is below lower limit
                        {
                            turnAdjustment = constrain(map(xPosition, 0, lowerLimMod, -130, 0), -130, 0); // Map X-axis to turn adjustment
                        }
                        else if (xPosition > upperLimMod) // Check if X-axis is above upper limit
                        {
                            turnAdjustment = constrain(map(xPosition, upperLimMod, 1023, 0, 130), 0, 130); // Map X-axis to turn adjustment
                        }
                    }
                    else // If Y-axis is within modified neutral zone
                    {
                        if (xPosition < lowerLimMod + 5) // Check if X-axis is slightly below lower limit
                        {
                            turnAdjustment = map(xPosition, 0, lowerLimMod, -150, 0); // Map X-axis to turn adjustment
                        }
                        else if (xPosition > upperLimMod - 5) // Check if X-axis is slightly above upper limit
                        {
                            turnAdjustment = map(xPosition, upperLimMod, 1023, 0, 150); // Map X-axis to turn adjustment
                        }
                    }

                    motor1Speed = -constrain(baseSpeed + turnAdjustment, -255, 255); // Calculate motor1 speed
                    motor2Speed = constrain(baseSpeed - turnAdjustment, -255, 255); // Calculate motor2 speed
                    motor1.setSpeed(motor1Speed); // Set motor1 speed
                    motor2.setSpeed(motor2Speed); // Set motor2 speed
                    Serial.print("Motor 1 Speed: "); // Print motor1 speed
                    Serial.println(motor1Speed); // Output motor1 speed
                    Serial.print("Motor 2 Speed: "); // Print motor2 speed
                    Serial.println(motor2Speed); // Output motor2 speed
                }
            } else if (motorControlConfiguration == 2) { // Check if motor control configuration is set to modded
                int yPosition = data.movementJoystick_y; // Get Y-axis joystick position
                int xPosition = data.movementJoystick_x; // Get X-axis joystick position
                int motor1Speed; // Variable for motor1 speed
                int motor2Speed; // Variable for motor2 speed
                int baseSpeed; // Variable for base speed
                int turnRatio; // Variable for turn ratio

                baseSpeed = map(yPosition, 0, 1023, -255, 255); // Map Y-axis to motor speed range
                turnRatio = map(xPosition, 0, 1023, -255, 255); // Map X-axis to turn ratio range
                if (abs(turnRatio) <= 20) { // Check if turn ratio is within dead zone
                    turnRatio = 0; // Set turn ratio to 0 if within dead zone
                }
                if (baseSpeed >= -40 && baseSpeed <= 40) { // Check if base speed is within dead zone
                    turnRatio = constrain(turnRatio, -150, 150); // Constrain turn ratio for turning in place
                    motor1Speed = turnRatio; // Set motor1 speed for turning
                    motor2Speed = -turnRatio; // Set motor2 speed for turning
                } else { // If base speed is outside dead zone
                    motor1Speed = baseSpeed + turnRatio; // Calculate motor1 speed with turn adjustment
                    motor2Speed = baseSpeed - turnRatio; // Calculate motor2 speed with turn adjustment
                }
                motor1Speed = -constrain(motor1Speed, -255, 255); // Constrain motor1 speed to valid range
                motor2Speed = constrain(motor2Speed, -255, 255); // Constrain motor2 speed to valid range
                motor1.setSpeed(motor1Speed); // Set motor1 speed
                motor2.setSpeed(motor2Speed); // Set motor2 speed
            }
            /*if (data.movementJoystick_y > 520)
            {
                int motorSpeed = map(data.movementJoystick_y, 520, 1023, 0, 255);
                motor1.setSpeed(motor1DirectionalModifier * motorSpeed);
                motor2.setSpeed(-motor2DirectionalModifier * motorSpeed);
            }
            else if (data.movementJoystick_y < 504)
            {
                int motorSpeed = map(data.movementJoystick_y, 0, 504, -255, 0);
                motor1.setSpeed(motor1DirectionalModifier * motorSpeed);
                motor2.setSpeed(-motor2DirectionalModifier * motorSpeed);
            }
            else
            {
                motor1.setSpeed(0);
                motor2.setSpeed(0);
            }*/

            // Arm Movement
            if (data.armJoystick >= upperJoystickLimit + 100) // Check if arm joystick is pushed upward beyond threshold
            {
                setArmPos(lastArmServoPos + map(data.armJoystick, upperJoystickLimit, 1023, 1, 3)); // Increment arm position based on joystick value
                delay(20); // Small delay for servo movement
            }
            if (data.armJoystick <= lowerJoystickLimit - 100) // Check if arm joystick is pushed downward beyond threshold
            {
                setArmPos(lastArmServoPos + map(data.armJoystick, 0, lowerJoystickLimit, -3, -1)); // Decrement arm position based on joystick value
                delay(20); // Small delay for servo movement
            }
            if (data.clampJoystick >= upperJoystickLimit + 100) // Check if clamp joystick is pushed upward beyond threshold
            {
                setClampPos(lastClampServoPos + map(data.clampJoystick, upperJoystickLimit, 1023, 1, 3)); // Increment clamp position based on joystick value
                delay(20); // Small delay for servo movement
            }
            if (data.clampJoystick <= lowerJoystickLimit - 100) // Check if clamp joystick is pushed downward beyond threshold
            {
                setClampPos(lastClampServoPos + map(data.clampJoystick, 0, lowerJoystickLimit, -3, -1)); // Decrement clamp position based on joystick value
                delay(20); // Small delay for servo movement
            }

            // Claw Movement
            if (data.clampButton > clampButtonLastState) // Check if clamp button state has changed
            {
                if (isClampClosed) // If clamp is currently closed
                {
                    setClampPos(clampOpen); // Open the clamp
                    delay(20); // Small delay for servo movement
                }
                else // If clamp is currently open
                {
                    setClampPos(clampClosed); // Close the clamp
                    delay(20); // Small delay for servo movement
                }
                isClampClosed = !isClampClosed; // Toggle clamp state
            }
            if (data.armButton > armButtonLastState) // Check if arm button state has changed
            {
                if (isArmDown) // If arm is currently down
                {
                    setArmPos(dumpServoPos); // Move arm to dump position
                    delay(20); // Small delay for servo movement
                }
                else // If arm is currently up
                {
                    setArmPos(objectServoPos); // Move arm to object position
                    delay(20); // Small delay for servo movement
                }
                isArmDown = !isArmDown; // Toggle arm state
            }
            /*if (data.armButton > clampButtonLastState)
            {
                if (abs(lastArmServoPos - dumpServoPos) < abs(lastArmServoPos - objectServoPos)) {
                    setArmPos(objectServoPos);
                } else {
                    setArmPos(dumpServoPos);
                }
                delay(100);
            }*/
            clampButtonLastState = data.clampButton; // Update the last state of the clamp button
            armButtonLastState = data.armButton; // Update the last state of the arm button
            // END RECEIVER CODE

            delay(10); // Small delay to account for WiFi transmission overhead, can be reduced after testing
        }
        /*if (configuration == 3) {
          void funky() {
            Serial.println("funky");
          }
        }*/
    }
}
