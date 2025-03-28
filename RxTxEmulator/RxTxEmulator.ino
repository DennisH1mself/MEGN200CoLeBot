/*
THE DOZERS -- Colebot #3 Code

Dennis Porter
Owen Rolo
Lily Orth
Carlton Engelhardt
*/

// SETTINGS
const int configuration = 1; // 1 for original, 2 for 2 axel with trigger buttons, 3 for dance
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

const int maxTurnSubtracter = 200;
const int minTurnSubtracter = 50;
const int upperJoystickLimit = 525;
const int lowerJoystickLimit = 510;
const int joystickCustomMod = 100;

const int dumpServoPos = 125;
const int objectServoPos = 56; // 65; // 56;
const int clampClosed = 71; //65; // 71;
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

bool armButtonLastState = 0;
bool clampButtonLastState = 0;
bool isClampClosed = 0;
bool isArmDown = 0;
// END RECEIVER DECLARATIONS

// START TRANSMITTER DECLARATIONS
const int movementJoystick_y = A0;
const int movementJoystick_x = A1;

const int armJoystick = A2;
const int clampJoystick = A3;

const int clampButton = A5;
const int armButton = A4;

const int spinLeft = 12;
const int spinRight = 2;
// END TRANSMITTER DECLARATIONS

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
    }
    if ((WifiSerial.getPortType() == WifiPortType::Transmitter || WifiSerial.getPortType() == WifiPortType::Emulator))
    {
        pinMode(movementJoystick_y, INPUT);
        pinMode(movementJoystick_x, INPUT);

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
        data.movementJoystick_y = analogRead(movementJoystick_y);
        data.movementJoystick_x = analogRead(movementJoystick_x);

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
        if (configuration == 1)
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

            if (data.spinLeft)
            {
                motor1.setSpeed(spinSpeed);
                motor2.setSpeed(spinSpeed);
            }
            else if (data.spinRight)
            {
                motor1.setSpeed(-spinSpeed);
                motor2.setSpeed(-spinSpeed);
            }
            else
            {
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
                        turnAdjustment = -constrain(map(xPosition, 0, lowerLimMod, -150, 0), -baseSpeed, 0);
                    }
                    else if (xPosition > upperLimMod)
                    {
                        turnAdjustment = -constrain(map(xPosition, upperLimMod, 1023, 0, 150), 0, baseSpeed);
                    }
                } else {
                    if (xPosition < lowerLimMod)
                    {
                        turnAdjustment = map(xPosition, 0, lowerLimMod, -100, 0);
                    }
                    else if (xPosition > upperLimMod)
                    {
                        turnAdjustment = map(xPosition, upperLimMod, 1023, 0, 100);
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

                motor1Speed = -constrain(baseSpeed - turnAdjustment, -255, 255);
                motor2Speed = constrain(baseSpeed + turnAdjustment, -255, 255);
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
    }
}
