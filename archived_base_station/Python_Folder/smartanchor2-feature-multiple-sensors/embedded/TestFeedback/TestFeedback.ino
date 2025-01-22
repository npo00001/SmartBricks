int received;

void setup() {
  Serial.begin(9600);
}

void loop() {
  if (Serial.available()) {               // wait for serial communication
    received = Serial.parseInt();         // read an int out of the serial communication
    serialFlush();                        // flush remaining input from serial buffer **Acceptable in controlled envrionment, but not best practice**

    Serial.println(received * received);  // respond with the square of what was received
  }
}

void serialFlush(){
  while (Serial.available()) {
    Serial.read();
  }
}
