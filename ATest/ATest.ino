#include <Servo.h>
// A structure, similar to our servo and stepper motors, but this one conatins variables to be transmitted
// Any variable you want to transmit/recieve must be initalized in the DataPacket structure

const int servo1Pin = 9;
const int servo2Pin = 10;

class DCMotor {
public:
  DCMotor(int pwmPin, int pin1, int pin2) 
    : pwmPin(pwmPin), pin1(pin1), pin2(pin2) {
    pinMode(pwmPin, OUTPUT);
    pinMode(pin1, OUTPUT);
    pinMode(pin2, OUTPUT);
  }

  void setSpeed(int speed) {
    analogWrite(pwmPin, abs(speed));
    if (speed > 0) {
      digitalWrite(pin1, HIGH);
      digitalWrite(pin2, LOW);
    } else if (speed < 0) {
      digitalWrite(pin1, LOW);
      digitalWrite(pin2, HIGH);
    } else {
      digitalWrite(pin1, LOW);
      digitalWrite(pin2, LOW);
    }
  }

private:
  int pwmPin;
  int pin1;
  int pin2;
};

DCMotor motor2(5, 4, 3);
DCMotor motor1(6, 7, 8);

Servo servo1;
Servo servo2;

void setup() {
  //DONT USE PIN13 FOR ANY SENSOR OR ACTUATORS

  Serial.begin(9600);  //preferred transmission rate for Arduino UNO R4
  servo1.attach(servo1Pin);
  servo2.attach(servo2Pin);
}
void loop() {
  // put your main code here, to run repeatedly:
  servo1.write(90);
  servo2.write(90);
  motor1.setSpeed(100);
  motor2.setSpeed(100);
  delay(1000);
  servo1.write(0);
  servo2.write(0);
  motor1.setSpeed(-100);
  motor2.setSpeed(-100);
  delay(1000);
}