const int dumpServoPos = 135; // Servo position for dumping
const int objectServoPos = 65; // Servo position for grabbing objects
const int clampClosed = 76;    // Servo position for clamp closed
const int clampOpen = 20;      // Servo position for clamp open
const int spinSpeed = 150;     // Speed for spinning motors

int servo1Limits[2] = {objectServoPos, dumpServoPos + 20}; // Limits for servo1 movement
int servo2Limits[2] = {clampOpen - 5, clampClosed}; // Limits for servo2 movement

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