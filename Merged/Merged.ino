#include "WifiPort.h"
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

struct DataPacket {

  int AnalogCheck;  //an initial check to show successful transmission
                    //YOU should wire up a simple resistor (220 ohm) circuit and manually probe it with a wire connected to A0
  //int OtherThing;
  //float AndAnotherThing;
  //char YetAnotherThing;


} data;

WifiPort<DataPacket> WifiSerial;

void setup() {
  //DONT USE PIN13 FOR ANY SENSOR OR ACTUATORS

  Serial.begin(115200);  //preferred transmission rate for Arduino UNO R4

  // WifiSerial.begin("ssid_UPDATE_FOR_YOUR_GROUP", "password_UPDATE", WifiPortType::Transmitter);
  // WifiSerial.begin("ssid_UPDATE_FOR_YOUR_GROUP", "password_UPDATE", WifiPortType::Receiver);
  WifiSerial.begin("PairAP_CBG28", "GiggleSmurfs69", WifiPortType::Emulator); // one board to rule them all debugging
}

void loop() {

  if (WifiSerial.getPortType() == WifiPortType::Transmitter || WifiSerial.getPortType() == WifiPortType::Emulator) {
    WifiSerial.autoReconnect();

    data.AnalogCheck = millis(); // analogRead(A0);
    Serial.print("Sending: " );
    Serial.println(data.AnalogCheck);

    if (!WifiSerial.sendData(data))
      Serial.println("Wifi Send Problem");


  }
  
  if ((WifiSerial.getPortType() == WifiPortType::Receiver || WifiSerial.getPortType() == WifiPortType::Emulator) && WifiSerial.checkForData()) {

    data = WifiSerial.getData();
    Serial.print("Received: " );
    Serial.println(data.AnalogCheck); 

    

  }

  delay(100); // update delay after you get it working to be a smaller number like 10ms to account for WiFi transmission overhead
}
