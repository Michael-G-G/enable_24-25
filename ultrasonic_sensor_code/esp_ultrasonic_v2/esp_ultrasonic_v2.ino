// Define pins for Trigger and Echo
#define TRIG_PIN 10  // GPIO pin connected to the Trigger pin of the sensor
#define ECHO_PIN 9   // GPIO pin connected to the Echo pin of the sensor

float lastDistance = 0;     // Set an arbitrary last distance reading
int changeCount = 0;        // Counts the number of times the threshold is exceeded before accepting the reading
float valuesExceeded[3] = {0, 0, 0}; // Store last three changes

// Trigger the sensors. Returns the average of 5 duration readings
float getAverageDuration() {
  long sumDuration = 0;

  for (int i = 0; i < 5; i++) {
    digitalWrite(TRIG_PIN, LOW);   // Ensure Trigger pin is LOW
    delayMicroseconds(2);          // Wait for 2 microseconds to stabilize the signal
    digitalWrite(TRIG_PIN, HIGH);  // Set Trigger pin HIGH to send an ultrasonic pulse
    delayMicroseconds(10);         // Keep it HIGH for 10 microseconds
    digitalWrite(TRIG_PIN, LOW);   // Set Trigger pin back to LOW

    long duration = pulseIn(ECHO_PIN, HIGH);  // Measure the time Echo pin is HIGH
    if (duration == 0) {
      i--;  // Ignore failed Reading. Try again.
    } else {
      sumDuration += duration;  // Add to sum of 5 durations
    }
    delay(50);
  }

  long avDuration = sumDuration / 5;  // Find the average duration
  return avDuration;
}

void setup() {
  Serial.begin(115200);       // Start the Serial Monitor at 115200 baud rate
  pinMode(TRIG_PIN, OUTPUT);  // Set Trigger pin as OUTPUT
  pinMode(ECHO_PIN, INPUT);   // Set Echo pin as INPUT
}

void loop() {
  // Trigger the sensor
  float duration = getAverageDuration();

  // Convert duration to distance in cm
  float distance = (duration / 2.0) * 0.0343;

  // Threshold allows smaller  changes at close distances and larger changes at far distances
  float threshold = max(10, lastDistance * 0.2);  // Allow 20% variation of the last distance reading

  // Checks if the change in distance is within an acceptable range to avoid large sudden changes
  if (lastDistance > 0 && abs(lastDistance - distance) > threshold) {
    
    // Store the last three threshold-exceeding values
    valuesExceeded[changeCount % 3] = distance;  

    // Counts up until there are three exceeded readings
    if (changeCount < 2) {  
        changeCount++;
        return;
    }

    // Three values are recorded, check if they are either increasing or decreasing
    bool increasing = (valuesExceeded[0] < valuesExceeded[1]) && (valuesExceeded[1] < valuesExceeded[2]);
    bool decreasing = (valuesExceeded[0] > valuesExceeded[1]) && (valuesExceeded[1] > valuesExceeded[2]);
    
    if (increasing || decreasing) {
        changeCount = 0;   // Accept the reading and reset the counter
    }
    else {
        changeCount = 0;  // Ignore the reading and resent the counter
        return;
    }
}

  // Print close, medium, far
  if (distance < 30) {
    Serial.print("Close, ");
  } 
  else if (distance < 100) {
    Serial.print("Medium, ");
  } 
  else {
    Serial.print("Far, ");
  }
  
  // Print the distance in cm
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Update the last distance
  lastDistance = distance;

  delay(500);  // Wait 500 milliseconds before the next reading
}
