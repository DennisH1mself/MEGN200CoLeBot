/*
THE DOZERS -- BASIC FUNCTION TEST CODE

Dennis Porter
Owen Rolo
Lily Orth
Carlton Engelhardt
*/


#include <Servo.h>
#include "WifiPort.h"
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
} data;
// END STRUCTURE PACKET

// START WIFI DECLARATIONS
WifiPort<DataPacket> WifiSerial;
WifiPortType portType = WifiPortType::Receiver; // WifiPortType::Transmitter, WifiPortType::Receiver, WifiPortType::Emulator
// END WIFI DECLARATIONS

// START RECEIVER DECLARATIONS
int spinLeftLastState = 0;
int spinRightLastState = 0;
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

const int servo1Pin = 9;
const int servo2Pin = 10;

DCMotor motor2(5, 4, 3);
DCMotor motor1(6, 7, 8);

Servo servo1;
Servo servo2;
int lastArmServoPos = 0;
void setArmPos(int newPos) {
  lastArmServoPos = newPos;
  if (newPos >= 180) {
    lastArmServoPos = 180;
  } else if (newPos <= 0) {
    lastArmServoPos = 0;
  }
  
  servo1.write(lastArmServoPos);
}
// END RECEIVER DECLARATIONS

// START TRANSMITTER DECLARATIONS
const int movementJoystick_y = A0;
const int movementJoystick_x = A1;
const int armJoystick = A2;
const int armJoystickButton = 12;
// const int JoystickButtonState = 0;
const int spinLeft = A4;
const int spinRight = A5;
// END TRANSMITTER DECLARATIONS

void setup()
{
  //motor1.setSpeed(255);
  // DONT USE PIN13 FOR ANY SENSOR OR ACTUATORS
  Serial.begin(115200);
  WifiSerial.begin("PairAP_CBG28", "GiggleSmurfs69", portType);
  
  if (WifiSerial.getPortType() == WifiPortType::Receiver || WifiSerial.getPortType() == WifiPortType::Emulator)
  {
    servo1.attach(servo1Pin);
    servo2.attach(servo2Pin);
    servo2.write(0);
    setArmPos(0);
  }

  if ((WifiSerial.getPortType() == WifiPortType::Transmitter || WifiSerial.getPortType() == WifiPortType::Emulator))
  {
    pinMode(movementJoystick_y, INPUT);
    pinMode(movementJoystick_x, INPUT);
    pinMode(armJoystick, INPUT);
    pinMode(armJoystickButton, INPUT_PULLUP);
    pinMode(spinLeft, INPUT_PULLUP);
    pinMode(spinRight, INPUT_PULLUP);
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
    int motor1DirectionalModifier = 1;
    int motor2DirectionalModifier = 1;

    if (data.spinLeft > spinLeftLastState)
    {
      motor1DirectionalModifier = -1;
    }
    else
    {
      motor1DirectionalModifier = 1;
    }
    if (data.spinRight > spinRightLastState)
    {
      motor2DirectionalModifier = -1;
    }
    else
    {
      motor2DirectionalModifier = 1;
    }

    if (data.movementJoystick_y > 520)
    {
      int motorSpeed = map(data.movementJoystick_y, 520, 1023, 0, 255);
      motor1.setSpeed(motor1DirectionalModifier * motorSpeed);
      motor2.setSpeed(motor2DirectionalModifier * motorSpeed);
    }
    else if (data.movementJoystick_y < 504)
    {
      int motorSpeed = map(data.movementJoystick_y, 0, 504, -255, 0);
      motor1.setSpeed(motor1DirectionalModifier * motorSpeed);
      motor2.setSpeed(motor2DirectionalModifier * motorSpeed);
    }
    if (data.armJoystick_y >= 520) {
      setArmPos(lastArmServoPos + map(data.armJoystick_y, 520, 1023, 1, 3));
    }
    if (data.armJoystick_y <= 508) {
      setArmPos(lastArmServoPos + map(data.armJoystick_y, 0, 508, -3, -1));
    }
    // END RECEIVER CODE

    delay(10); // update delay after you get it working to be a smaller number like 10ms to account for WiFi transmission overhead
  }
}
