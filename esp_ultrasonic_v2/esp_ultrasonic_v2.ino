// Define pins for Trigger and Echo
#define TRIG_PIN 32 // GPIO pin connected to the Trigger pin of the sensor
#define ECHO_PIN 33 // GPIO pin connected to the Echo pin of the sensor

void setup() {
  Serial.begin(115200);               // Start the Serial Monitor at 115200 baud rate
  pinMode(TRIG_PIN, OUTPUT);          // Set Trigger pin as OUTPUT
  pinMode(ECHO_PIN, INPUT);           // Set Echo pin as INPUT
}

void loop() {
  // Trigger the sensor
  digitalWrite(TRIG_PIN, LOW);        // Ensure Trigger pin is LOW
  delayMicroseconds(2);               // Wait for 2 microseconds to stabilize the signal
  digitalWrite(TRIG_PIN, HIGH);       // Set Trigger pin HIGH to send an ultrasonic pulse
  delayMicroseconds(10);              // Keep it HIGH for 10 microseconds
  digitalWrite(TRIG_PIN, LOW);        // Set Trigger pin back to LOW

  // Read the duration of the Echo signal
  long duration = pulseIn(ECHO_PIN, HIGH); // Measure the time Echo pin is HIGH

  // Calculate the distance based on the duration
  float distance = (duration / 2.0) * 0.0343; // Convert duration to distance in cm

  // Check for out-of-range readings and print the result
  if (duration == 0) {
    Serial.println("Out of range!"); // No object detected or object out of range
  } else {
    Serial.print("Distance: ");
    Serial.print(distance);         // Print the distance in cm
    Serial.println(" cm");
  }

  delay(500); // Wait 500 milliseconds before the next reading
}
