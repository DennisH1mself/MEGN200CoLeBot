void setup() {
  Serial.begin(9600);
}
void loop() {
    Serial.println(analogRead(A0));
    Serial.println(analogRead(A1));
}