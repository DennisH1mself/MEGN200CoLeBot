#include "WifiPort.h"

// A structure, similar to our servo and stepper motors, but this one conatins variables to be transmitted
// Any variable you want to transmit/recieve must be initalized in the DataPacket structure
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
  WifiSerial.begin("ssid_UPDATE_FOR_YOUR_GROUP", "123", WifiPortType::Emulator); // one board to rule them all debugging
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
