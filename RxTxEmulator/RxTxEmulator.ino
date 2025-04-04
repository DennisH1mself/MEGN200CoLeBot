/*
THE DOZERS -- Colebot Final Code

Dennis Porter
Owen Rolo
Lily Orth
Carlton Engelhardt
*/

// SETTINGS
const int configuration = 4; // 1 for original, 2 for autopilot, 3 for dance, 4 for buttons with joystick
// END SETTING

// LIBRARIES
#include "WifiPort.h"
#include <Servo.h>
#include <Arduino.h>
// END LIBRARIES

// STRUCTURE PACKET
struct DataPacket
{
    int AnalogCheck;
    int movementJoystick_y;
    int movementJoystick_x;
    int armJoystick;
    int clampButton;
    int spinLeft;
    int spinRight;
    int armButton;
    int clampJoystick;
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

const int maxTurnSubtracter = 200;
const int minTurnSubtracter = 50;
const int upperJoystickLimit = 529;
const int lowerJoystickLimit = 505;
const int joystickCustomMod = 100;

const int dumpServoPos = 135;
const int objectServoPos = 65; // 65; // 3:56;
const int clampClosed = 74;    // 65; // 3:71;
const int clampOpen = 20;
const int spinSpeed = 150;
int servo1Limits[2] = {objectServoPos, dumpServoPos + 20};
int servo2Limits[2] = {clampOpen - 5, clampClosed};

class DCMotor
{
public:
    DCMotor(int pwmPin, int pin1, int pin2)
        : pwmPin(pwmPin), pin1(pin1), pin2(pin2)
    {
        pinMode(pwmPin, OUTPUT);
        pinMode(pin1, OUTPUT);
        pinMode(pin2, OUTPUT);
    }

    void setSpeed(int speed)
    {
        analogWrite(pwmPin, abs(speed));
        if (speed > 0)
        {
            digitalWrite(pin1, HIGH);
            digitalWrite(pin2, LOW);
        }
        else if (speed < 0)
        {
            digitalWrite(pin1, LOW);
            digitalWrite(pin2, HIGH);
        }
        else
        {
            digitalWrite(pin1, LOW);
            digitalWrite(pin2, LOW);
        }
    }

private:
    int pwmPin;
    int pin1;
    int pin2;
};

const int servo1Pin = A0; // 9
const int servo2Pin = A1; // 10

DCMotor motor2(6, 5, 7); // DCMotor motor2(5, 4, 3); // RIGHT
DCMotor motor1(3, 2, 4); // DCMotor motor1(6, 7, 8); // LEFT

Servo servo1;
Servo servo2;

int lastArmServoPos = 0;
int lastClampServoPos = 0;

void setArmPos(int newPos)
{
    lastArmServoPos = constrain(newPos, servo1Limits[0], servo1Limits[1]);
    servo1.write(lastArmServoPos);
    delay(20);
}
void setClampPos(int newPos)
{
    lastClampServoPos = constrain(newPos, servo2Limits[0], servo2Limits[1]);
    servo2.write(lastClampServoPos);
    delay(20);
}
void setArmPosDelayless(int newPos)
{
    lastArmServoPos = constrain(newPos, servo1Limits[0], servo1Limits[1]);
    servo1.write(lastArmServoPos);
    delay(20);
}
void setClampPosDelayless(int newPos)
{
    lastClampServoPos = constrain(newPos, servo2Limits[0], servo2Limits[1]);
    servo2.write(lastClampServoPos);
    delay(20);
}

bool armButtonLastState = 0;
bool clampButtonLastState = 0;
bool isClampClosed = 0;
bool isArmDown = 0;


// START AUTOPILOT FUNCTIONS
void forward(int length)
{
    float delayX = 1 / speedOf255 * length * 1000;
    Serial.print("Forward (in): ");
    Serial.println(length);
    Serial.println(delayX);
    motor1.setSpeed(235);
    motor2.setSpeed(-255);
    delay(delayX);
    motor1.setSpeed(0);
    motor2.setSpeed(0);
    delay(500);
}

void turn(int radians, int multiplier = 1)
{ // left is positive
    float radius = 5.0 / 2.0;
    float angularSpeed = speedOf255 / radius;
    float delayX = 1 / angularSpeed * radians * 1000;
    Serial.print("Turn (rads): ");
    Serial.println(radians);
    Serial.println(delayX);
    motor1.setSpeed(-255 * multiplier);
    motor2.setSpeed(-255 * multiplier);
    delay(delayX);
    motor1.setSpeed(0);
    motor2.setSpeed(0);
}
void turn180()
{
    float radius = 10.6 / 2.0;
    float angularSpeed = speedOf255 / radius;
    float delayX = 1 / angularSpeed * degToRad(90) * 1000;
    Serial.print("Turn (rads): pi");
    Serial.println(delayX);
    motor1.setSpeed(-255);
    motor2.setSpeed(-255);
    delay(delayX);
    motor1.setSpeed(0);
    motor2.setSpeed(0);
    delay(500);
}
void turn90(int mod)
{
    float radius = 4.9 / 2.0;
    float angularSpeed = speedOf255 / radius;
    float delayX = 1 / angularSpeed * degToRad(90) * 1000;
    Serial.print("Turn (rads): pi/2");
    Serial.println(delayX);
    motor1.setSpeed(-255 * mod);
    motor2.setSpeed(-255 * mod);
    delay(delayX);
    motor1.setSpeed(0);
    motor2.setSpeed(0);
    delay(500);
}

// CHA CHA
void openClap() {
    Serial.println("Clap");
    setClampPos(clampClosed);
}
void closeClap() {
    Serial.println("Clap");
    setClampPos(clampOpen);
}
int toDelay(int seconds, int frame) {
    Serial.println("Delay");
    return seconds * 1000 + (frame*1000.0/60.0);
}
void intro() {
    Serial.println("Cha Cha Slide");
    delay(toDelay(17, 35));
}
void funky1() { // 17:35s to 24:42s
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
void everybodyClapYourHands() { // 24:42s to 33:25s
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
    delay(toDelay(33, 25) - toDelay(24, 42) - (700*12));
}

void toTheLeft1() { // 33:25s to 34:50s
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
    delay(toDelay(34, 50) - toDelay(33, 25) - (delayX));
}
void takeItBack() { // 34:50 to 36:50
    Serial.println(1000);
    motor1.setSpeed(-235);
    motor2.setSpeed(255);
    delay(1000);
    motor1.setSpeed(0);
    motor2.setSpeed(0);
    delay(toDelay(36, 50) - toDelay(34, 50) - 1000);
}
void oneHopThisTime() { // 36:50 to 38:50
    delay(200);
    Serial.println("Hop");
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(1000);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(toDelay(38, 50) - toDelay(36, 50) - 1200);
}
void rightFootLeftStomp() { // 38:50 to 40:25
    delay(500);
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(500);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(toDelay(40, 25) - toDelay(38, 50) - 1000);

}
void leftFootLeftStomp() { // 40:30 to 43:00
    delay(500);
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(500);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(toDelay(43, 00) - toDelay(40, 25) - 1000);
}
void chaChaRealSmooth() { // 43:00 to 46:26
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
    delay(toDelay(46, 26) - toDelay(43, 0) - (700*4));
}
void turnItOut() { // 46:26 to 48:15
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
    delay(toDelay(48, 15) - toDelay(46, 26) - (delayX));
}
void toTheLeft2() { // 48:15 to 49:50
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
    delay(toDelay(49, 50) - toDelay(48, 15) - (delayX));
}
void takeItBack2() { // 49:50 to 51:50
    Serial.println(1000);
    motor1.setSpeed(-235);
    motor2.setSpeed(255);
    delay(1000);
    motor1.setSpeed(0);
    motor2.setSpeed(0);
    delay(toDelay(51, 50) - toDelay(49, 50) - 1000);
}
void oneHopThisTime2() { // 51:50 to 53:50
    delay(200);
    Serial.println("Hop");
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(1000);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(toDelay(53, 50) - toDelay(51, 50) - 1200);
} 
void rightFootLeftStomp2() { // 53:50 to 55:50
    delay(500);
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(500);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(toDelay(55, 50) - toDelay(53, 50) - 1000);

}
void leftFootLeftStomp2() { // 55:50 to 57:50
    delay(500);
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(500);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(toDelay(57, 50) - toDelay(55, 50) - 1000);
}
void chaChaNowYall() { // 57:50 to 1:01:00
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
    delay(toDelay(61, 0) - toDelay(57, 50) - (700*4));
}
void toTheRight() { // 1:01:00 to 1:02:50
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
    delay(toDelay(62, 50) - toDelay(61, 0) - (delayX));
}
void toTheLeft3() { // 1:02:50 to 1:04:50
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
    delay(toDelay(64, 50) - toDelay(62, 50) - (delayX));
}
void takeItBack3() { // 1:04:50 to 1:06:50
    Serial.println(1000);
    motor1.setSpeed(-235);
    motor2.setSpeed(255);
    delay(1000);
    motor1.setSpeed(0);
    motor2.setSpeed(0);
    delay(toDelay(66, 50) - toDelay(64, 50) - 1000);
}
void oneHopThisTime3() { // 1:06:39 to 1:08:26
    delay(200);
    Serial.println("Hop");
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(1000);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(toDelay(68, 26) - toDelay(66, 39) - 1200);
}
void oneHopThisTime4() { // 1:08:26 to 1:10:21
    delay(200);
    Serial.println("Hop");
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(1000);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(toDelay(70, 21) - toDelay(68, 26) - 1200);
}
void rightFootTwoStomps() { // 1:10:31 to 1:12:21
    delay(500);
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(500);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(toDelay(72, 21) - toDelay(70, 21) - 1000);
}
void leftFootTwoStomps() { // 1:12:21 to 1:14:11
    delay(500);
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(500);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(toDelay(74, 11) - toDelay(72, 21) - 1000);
}
void slideToTheLeft() { // 1:14:11 to 1:16:00
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
    delay(toDelay(76, 0) - toDelay(74, 11) - (delayX));
}
void slideToTheRight() { // 1:16:00 to 1:17:50
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
    delay(toDelay(77, 50) - toDelay(76, 0) - (delayX));
}
void crissCross() { // 1:17:50 to 1:19:50
    setClampPosDelayless(clampClosed);
    delay(500);
    setClampPosDelayless(clampOpen);
    delay(500);
    setClampPosDelayless(clampClosed);
    delay(500);
    setClampPosDelayless(clampOpen);
    delay(toDelay(79, 50) - toDelay(77, 50) - (500*4));
}
void crissCross2() { // 1:19:50 to 1:21:50
    setClampPosDelayless(clampClosed);
    delay(500);
    setClampPosDelayless(clampOpen);
    delay(500);
    setClampPosDelayless(clampClosed);
    delay(500);
    setClampPosDelayless(clampOpen);
    delay(toDelay(81, 50) - toDelay(79, 50) - (500*4));
}
void chaChaSlide2() { // 1:21:50 to 1:26:50
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
    delay(toDelay(86, 50) - toDelay(81, 50) - (700*4));
}
void toTheLeft4() { // 1:26:50 to 1:28:50
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
    delay(toDelay(88, 50) - toDelay(86, 50) - (delayX));
}
void takeItBack4() { // 1:28:50 to 1:30:50
    Serial.println(1000);
    motor1.setSpeed(-235);
    motor2.setSpeed(255);
    delay(1000);
    motor1.setSpeed(0);
    motor2.setSpeed(0);
    delay(toDelay(90, 50) - toDelay(88, 50) - 1000);
}
void twoHopsThisTime() { // 1:30:50 to 1:32:00
    delay(200);
    Serial.println("Hop");
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(1000);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(toDelay(92, 0) - toDelay(90, 50) - 1200);
}
void twoHopsThisTime2() { // 1:32:00 to 1:33:50
    delay(200);
    Serial.println("Hop");
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(1000);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(toDelay(93, 10) - toDelay(92, 0) - 1200);
}
void rightFootTwoStomps2() { // 1:33:50 to 1:35:00
    delay(500);
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(500);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(toDelay(95, 0) - toDelay(93, 10) - 1000);
}
void leftFootTwoStomps2() { // 1:35:00 to 1:36:20
    delay(200);
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(200);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(toDelay(96, 20) - toDelay(95, 0) - 1000);
}
void handsOnYourKnees() { // 1:36:20 to 1:39:50
    Serial.println("Hands on your knees");
    setArmPosDelayless(objectServoPos);
    setClampPosDelayless(clampClosed);
    delay(2000);
    setArmPosDelayless(dumpServoPos);
    setClampPosDelayless(clampOpen);
    delay(toDelay(99, 50) - toDelay(96, 20) - 200);
}
void getFunkyWitIt() { // 1:39:50 to 1:46:50
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
    delay(toDelay(106, 50) - toDelay(99, 50) - (1200*4));
}
void chaChaRealSmooth2() {
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

// END RECEIVER DECLARATIONS

// START TRANSMITTER DECLARATIONS

const int movementJoystick_y = A1;
const int movementJoystick_x = A0;

const int armJoystick = A2;
const int clampJoystick = A3;
/*
const int movementJoystick_y = A3;
const int movementJoystick_x = A2;

const int armJoystick = A0;
const int clampJoystick = A1;*/

const int clampButton = A5;
const int armButton = A4;

const int spinLeft = 2;
const int spinRight = 12;
// END TRANSMITTER DECLARATIONS
float degToRad(float deg)
{
    return deg * pi / 180.0;
}
void setup()
{
    // DONT USE PIN13 FOR ANY SENSOR OR ACTUATORS
    Serial.begin(115200);
    WifiSerial.begin("PairAP_CBG28", "GiggleSmurfs69", portType);
    if (WifiSerial.getPortType() == WifiPortType::Receiver || WifiSerial.getPortType() == WifiPortType::Emulator)
    {
        servo1.attach(servo1Pin);
        servo2.attach(servo2Pin);

        setArmPos(dumpServoPos); //(servo1Limits[0]);
        isArmDown = 0;

        setClampPos(clampOpen); //(servo2Limits[0]);
        isClampClosed = 0;

        if (configuration == 2)
        {
            // forward(70);

            turn180();
            forward(10);
            turn90(1);
            forward(15);
            turn90(1);
            forward(70);
            turn90(-1);
            forward(20);
            turn90(-1);
            forward(70);
            /*
            forward(68);
            turn(degToRad(180));
            forward(15);
            turn(degToRad(90));
            forward(24);*/
        }
    }
    if ((WifiSerial.getPortType() == WifiPortType::Transmitter || WifiSerial.getPortType() == WifiPortType::Emulator))
    {
        pinMode(movementJoystick_x, INPUT);
        pinMode(movementJoystick_y, INPUT);

        pinMode(armJoystick, INPUT);
        pinMode(clampJoystick, INPUT);

        pinMode(spinLeft, INPUT_PULLUP);
        pinMode(spinRight, INPUT_PULLUP);

        pinMode(armButton, INPUT_PULLUP);
        pinMode(clampButton, INPUT_PULLUP);
    }
}

void loop()
{
    if (WifiSerial.getPortType() == WifiPortType::Transmitter || WifiSerial.getPortType() == WifiPortType::Emulator)
    {
        WifiSerial.autoReconnect();

        // DATA PACKET
        data.AnalogCheck = millis(); // analogRead(A0);
        data.movementJoystick_y = analogRead(movementJoystick_x);
        data.movementJoystick_x = analogRead(movementJoystick_y);

        data.armJoystick = analogRead(armJoystick);
        data.clampJoystick = analogRead(clampJoystick);

        data.armButton = !digitalRead(armButton);
        data.clampButton = !digitalRead(clampButton);

        data.spinLeft = !digitalRead(spinLeft);
        data.spinRight = !digitalRead(spinRight);
        // END DATA PACKET

        // SERIAL OUTPUT CHECK
        Serial.print("Sending: ");
        Serial.println(data.AnalogCheck);

        Serial.print("Y-movement: ");
        Serial.println(data.movementJoystick_y);
        Serial.print("X-movement: ");
        Serial.println(data.movementJoystick_x);

        Serial.print("Arm Joystick: ");
        Serial.println(data.armJoystick);
        Serial.print("Clamp Joystick: ");
        Serial.println(data.clampJoystick);

        Serial.print("Arm Button State: ");
        Serial.println(data.armButton);
        Serial.print("Clamp Button State: ");
        Serial.println(data.clampButton);

        Serial.print("Spin Left: ");
        Serial.println(data.spinLeft);
        Serial.print("Spin Right: ");
        Serial.println(data.spinRight);
        // END SERIAL OUTPUT CHECK

        if (!WifiSerial.sendData(data))
            Serial.println("Wifi Send Problem");
    }

    if ((WifiSerial.getPortType() == WifiPortType::Receiver || WifiSerial.getPortType() == WifiPortType::Emulator) && WifiSerial.checkForData())
    {
        if (configuration == 3)
        {
            data = WifiSerial.getData();
            if (data.spinRight) { // Actually left
            // AUTOPILOT
                chaChaSlide();
                delay(1000);
            }
            // END AUTOPILOT
        }
        
        if (configuration == 1 || configuration == 4)
        {
            data = WifiSerial.getData();

            // SERIAL OUTPUT CHECK
            Serial.print("Received: ");
            Serial.println(data.AnalogCheck);

            Serial.print("Y-movement: ");
            Serial.println(data.movementJoystick_y);
            Serial.print("X-movement: ");
            Serial.println(data.movementJoystick_x);

            Serial.print("Arm Joystick: ");
            Serial.println(data.armJoystick);
            Serial.print("Clamp Joystick: ");
            Serial.println(data.clampJoystick);

            Serial.print("Arm Button State: ");
            Serial.println(data.armButton);
            Serial.print("Clamp Button State: ");
            Serial.println(data.clampButton);

            Serial.print("Spin Left: ");
            Serial.println(data.spinLeft);
            Serial.print("Spin Right: ");
            Serial.println(data.spinRight);
            // END SERIAL OUTPUT CHECK

            // RECEIVER CODE
            // Motor Direction
            int motor1DirectionalModifier = 1;
            int motor2DirectionalModifier = 1;

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
            if (configuration == 4)
            {
                data.armButton = data.spinRight;
                data.clampButton = data.spinLeft;
                data.spinRight = 0;
                data.spinLeft = 0;
            }
            
            if (data.spinLeft) {
                motor1.setSpeed(-255);
                motor2.setSpeed(-255);
            } else if (data.spinRight) {
                motor1.setSpeed(255);
                motor2.setSpeed(255);
            } else { 
                int yPosition = data.movementJoystick_y;
                int xPosition = data.movementJoystick_x;
                int motor1Speed;
                int motor2Speed;
                int baseSpeed;

                if (yPosition > upperJoystickLimit || yPosition < lowerJoystickLimit)
                {
                    baseSpeed = map(yPosition, 0, 1023, -255, 255);
                }
                else
                {
                    baseSpeed = 0;
                }
                int turnAdjustment = 0;
                const int customMod = 100;
                const int lowerLimMod = lowerJoystickLimit - customMod;
                const int upperLimMod = upperJoystickLimit + customMod;
                if (!(yPosition < upperLimMod && yPosition > lowerLimMod))
                {
                    if (xPosition < lowerLimMod)
                    {
                        turnAdjustment = constrain(map(xPosition, 0, lowerLimMod, -130, 0), -130, 0); // -
                    }
                    else if (xPosition > upperLimMod)
                    {
                        turnAdjustment = constrain(map(xPosition, upperLimMod, 1023, 0, 130), 0, 130); // -
                    }
                }
                else
                {
                    if (xPosition < lowerLimMod + 5)
                    {
                        turnAdjustment = map(xPosition, 0, lowerLimMod, -150, -30);
                    }
                    else if (xPosition > upperLimMod - 5)
                    {
                        turnAdjustment = map(xPosition, upperLimMod, 1023, 30, 150);
                    }
                }

                /*
                int turnAdjustment = 0;
                const int mod2 = 0;
                if (yPosition < upperJoystickLimit && yPosition > lowerJoystickLimit)
                {
                    if (xPosition < lowerJoystickLimit + mod2)
                    {
                        turnAdjustment = map(xPosition, 0, lowerJoystickLimit, -100, 0);
                    }
                    else if (xPosition > upperJoystickLimit - mod2)
                    {
                        turnAdjustment = map(xPosition, upperJoystickLimit, 1023, 0, 100);
                    }
                } else {
                    if (xPosition < lowerJoystickLimit + mod2)
                    {
                        turnAdjustment = map(xPosition, 0, lowerJoystickLimit, -150, -75);
                    }
                    else if (xPosition > upperJoystickLimit - mod2)
                    {
                        turnAdjustment = map(xPosition, upperJoystickLimit, 1023, 75, 150);
                    }
                }*/

                motor1Speed = -constrain(baseSpeed + turnAdjustment, -255, 255);
                motor2Speed = constrain(baseSpeed - turnAdjustment, -255, 255);
                /*
                if (yPosition > upperJoystickLimit || yPosition < lowerJoystickLimit)
                {
                    baseSpeed = map(yPosition, 0, 1023, -255, 255);
                }
                else
                {
                    baseSpeed = 0;
                }
                motor1Speed = baseSpeed;
                motor2Speed = -baseSpeed;

                if (xPosition < lowerJoystickLimit) {
                    int thero = map(xPosition, 0, lowerJoystickLimit, motor2Speed, minTurnSubtracter);
                    if (thero >= maxTurnSubtracter) {
                      thero = maxTurnSubtracter;
                    }
                    motor2Speed -= thero; // RIGHT
                }
                if (xPosition > upperJoystickLimit) {

                    int thero = map(xPosition, upperJoystickLimit, 1023, minTurnSubtracter, motor1Speed);
                    if (thero >= maxTurnSubtracter) {
                      thero = maxTurnSubtracter;
                    }

                    motor1Speed -= thero; // LEFT
                }*/
                motor1.setSpeed(motor1Speed);
                motor2.setSpeed(motor2Speed);
                Serial.print("Motor 1 Speed: ");
                Serial.println(motor1Speed);
                Serial.print("Motor 2 Speed: ");
                Serial.println(motor2Speed);
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
            if (data.armJoystick >= upperJoystickLimit + 100)
            {
                setArmPos(lastArmServoPos + map(data.armJoystick, upperJoystickLimit, 1023, 1, 3));
                delay(20);
            }
            if (data.armJoystick <= lowerJoystickLimit - 100)
            {
                setArmPos(lastArmServoPos + map(data.armJoystick, 0, lowerJoystickLimit, -3, -1));
                delay(20);
            }
            if (data.clampJoystick >= upperJoystickLimit + 100)
            {
                setClampPos(lastClampServoPos + map(data.clampJoystick, upperJoystickLimit, 1023, 1, 3));
                delay(20);
            }
            if (data.clampJoystick <= lowerJoystickLimit - 100)
            {
                setClampPos(lastClampServoPos + map(data.clampJoystick, 0, lowerJoystickLimit, -3, -1));
                delay(20);
            }

            // Claw Movement
            if (data.clampButton > clampButtonLastState)
            {
                if (isClampClosed)
                {
                    setClampPos(clampOpen); // Open, start at 2 to prevent servo from stalling
                    delay(20);
                }
                else
                {
                    setClampPos(clampClosed); // Close,
                    delay(20);
                }
                isClampClosed = !isClampClosed;
            }
            if (data.armButton > armButtonLastState)
            {
                if (isArmDown)
                {
                    setArmPos(dumpServoPos); // Open, start at 2 to prevent servo from stalling
                    delay(20);
                }
                else
                {
                    setArmPos(objectServoPos); // Close,
                    delay(20);
                }
                isArmDown = !isArmDown;
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
            clampButtonLastState = data.clampButton;
            armButtonLastState = data.armButton;
            // END RECEIVER CODE

            delay(10); // update delay after you get it working to be a smaller number like 10ms to account for WiFi transmission overhead
        }
        /*if (configuration == 3) {
          void funky() {
            Serial.println("funky");
          }
        }*/
    }
}
