/*
THE DOZERS -- Colebot #3 Code

Dennis Porter
Owen Rolo
Lily Orth
Carlton Engelhardt
*/

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
    int armJoystick_y;
    int armButtonState;
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
const int dumpServoPos = 125;
const int objectServoPos = 56;
const int clampClosed = 71;
const int clampOpen = 20;
const int spinSpeed = 150;
int servo1Limits[2] = {objectServoPos-5, dumpServoPos+20};
int servo2Limits[2] = {clampOpen-5, clampClosed+10};

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
}
void setClampPos(int newPos)
{
    lastClampServoPos = constrain(newPos, servo2Limits[0], servo2Limits[1]);
    servo2.write(lastClampServoPos);
}

bool armButtonLastState = 0;
bool armClosed = 0;
// END RECEIVER DECLARATIONS

// START TRANSMITTER DECLARATIONS
const int movementJoystick_y = A0;
const int movementJoystick_x = A1;
const int armJoystick = A2;
const int armJoystickButton = 12;
const int spinLeft = A4;
const int spinRight = A5;
const int clampJoystick = A3;
const int armButton = 2;
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
        setArmPos(dumpServoPos);//(servo1Limits[0]);
        ssetClampPos(clampClosed); //(servo2Limits[0]);
    }
    if ((WifiSerial.getPortType() == WifiPortType::Transmitter || WifiSerial.getPortType() == WifiPortType::Emulator))
    {
        pinMode(movementJoystick_y, INPUT);
        pinMode(movementJoystick_x, INPUT);
        pinMode(armJoystick, INPUT);
        pinMode(armJoystickButton, INPUT_PULLUP);
        pinMode(spinLeft, INPUT_PULLUP);
        pinMode(spinRight, INPUT_PULLUP);
        pinMode(armButton, INPUT_PULLUP);
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
        data.armJoystick_y = analogRead(armJoystick);
        data.armButtonState = !digitalRead(armJoystickButton);
        data.armButton = !digitalRead(armButton);
        data.clampJoystick = analogRead(clampJoystick);
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
        Serial.print("Arm-movement: ");
        Serial.println(data.armJoystick_y);
        Serial.print("Button State: ");
        Serial.println(data.armButtonState);
        Serial.print("Spin Left: ");
        Serial.println(data.spinLeft);
        Serial.print("Spin Right: ");
        Serial.println(data.spinRight);
        Serual.print("Arm Button: ");
        Serial.println(data.armButton);
        Serial.print("Clamp Joystick: ");
        Serial.println(data.clampJoystick);
        // END SERIAL OUTPUT CHECK

        if (!WifiSerial.sendData(data))
            Serial.println("Wifi Send Problem");
    }

    if ((WifiSerial.getPortType() == WifiPortType::Receiver || WifiSerial.getPortType() == WifiPortType::Emulator) && WifiSerial.checkForData())
    {
        data = WifiSerial.getData();

        // SERIAL OUTPUT CHECK
        Serial.print("Received: ");
        Serial.println(data.AnalogCheck);
        Serial.print("Y-movement: ");
        Serial.println(data.movementJoystick_y);
        Serial.print("X-movement: ");
        Serial.println(data.movementJoystick_x);
        Serial.print("Arm-movement: ");
        Serial.println(data.armJoystick_y);
        Serial.print("Button State: ");
        Serial.println(data.armButtonState);
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

        if (data.spinLeft) {
            motor1.setSpeed(spinSpeed);
            motor2.setSpeed(spinSpeed);
        } else if (data.spinRight) {
            motor1.setSpeed(-spinSpeed);
            motor2.setSpeed(-spinSpeed);
        } else {
            int yPosition = data.movementJoystick_y;
            int xPosition = data.movementJoystick_x;
            int motor1Speed;
            int motor2Speed;
            int baseSpeed;
            if (yPosition > 520 || yPosition < 504) {
                baseSpeed = map(yPosition, 0, 1023, -255, 255);
            } else {
                baseSpeed = 0;
            }
            motor1Speed = baseSpeed;
            motor2Speed = -baseSpeed;
            
            if (xPosition < 514) {
                int thero = map(xPosition, 0, 504, motor2Speed, minTurnSubtracter);
                if (thero >= maxTurnSubtracter) {
                  thero = maxTurnSubtracter;
                }
                motor2Speed -= thero; // RIGHT
            }
            if (xPosition > 518) {
              
                int thero = map(xPosition, 520, 1023, minTurnSubtracter, motor1Speed);
                if (thero >= maxTurnSubtracter) {
                  thero = maxTurnSubtracter;
                }
                
                motor1Speed -= thero; // LEFT
            }
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
        if (data.armJoystick_y >= 520)
        {
            setArmPos(lastArmServoPos + map(data.armJoystick_y, 520, 1023, 1, 5));
            delay(20);
        }
        if (data.armJoystick_y <= 508)
        {
            setArmPos(lastArmServoPos + map(data.armJoystick_y, 0, 508, -5, -1));
            delay(20);
        }
        if (data.clampJoystick >= 520)
        {
            setClampPos(lastClampServoPos + map(data.clampJoystick, 520, 1023, 1, 3));
            delay(20);
        }
        if (data.clampJoystick <= 508)
        {
            setClampPos(lastClampServoPos + map(data.clampJoystick, 0, 508, -3, -1));
            delay(20);
        }
        
        // Claw Movement
        if (data.armButtonState > armButtonLastState)
        { 
            if (armClosed)
            {
                setClampPos(clampOpen); // Open, start at 2 to prevent servo from stalling
                delay(20);
            }
            else
            {
                setClampPos(clampClosed); // Close, 
                delay(20);
            }
            armClosed = !armClosed;
        }
        if (data.armButton)
        {
            if (abs(lastArmServoPos - dumpServoPos) < abs(lastArmServoPos - objectServoPos)) {
                setArmPos(objectServoPos);
            } else {
                setArmPos(dumpServoPos);
            }
        }
        armButtonLastState = data.armButtonState;
        // END RECEIVER CODE

        delay(10); // update delay after you get it working to be a smaller number like 10ms to account for WiFi transmission overhead
    }
}
