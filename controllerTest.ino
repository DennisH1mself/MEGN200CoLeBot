int movementJoystick_y = A0;
int movementJoystick_x = A1;
int armJoystick = A2;
int armJoystickButton = 12;
int JoystickButtonState = 0;
int spinLeft = A4;
int spinRight = A5;
void setup(){
    pinMode(movementJoystick_y, INPUT);
    pinMode(movementJoystick_x,INPUT);
    pinMode(armJoystick,INPUT);
    pinMode(armJoystickButton,INPUT);
    pinMode(spinLeft,INPUT_PULLUP);
    pinMode(spinRight, INPUT_PULLUP);
    Serial.begin(9600);
}
void loop(){
JoystickButtonState = digitalRead(armJoystickButton);
delay(10);
if(JoystickButtonState == HIGH){
  Serial.println("Button pressed");
}
else if(JoystickButtonState == LOW){
  Serial.println("Button not pressed");
}
Serial.println("Y-movement: ");
Serial.println(analogRead(movementJoystick_y));
Serial.println("X-movement: ");
Serial.println(analogRead(movementJoystick_x));
Serial.println("Arm-movement: ");
Serial.println(analogRead(armJoystick));
delay(1000);
if(digitalRead(spinLeft)==LOW){
  Serial.println("Spinning Left");
  }else{}

if(digitalRead(spinRight) == LOW){
  Serial.println("Spinning Right");
  }else{}

}