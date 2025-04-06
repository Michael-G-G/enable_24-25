/**********************************************************************
  Filename    : Camera Web Serrver
  Description : ESP32 connects to WiFi and prints a url through a serial port.
                Users visit the site to view the image data ESP32 camera.
  Auther      : www.freenove.com
  Modification: 2024/06/20
**********************************************************************/
#include "esp_camera.h"
#include <WiFi.h>

// Define pins for Trigger and Echo
#define TRIG_PIN 32  // GPIO pin connected to the Trigger pin of the sensor
#define ECHO_PIN 33   // GPIO pin connected to the Echo pin of the sensor

// ===================
// Select camera model
// ===================
#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
//#define CAMERA_MODEL_ESP32S3_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
//#define CAMERA_MODEL_M5STACK_UNITCAM // No PSRAM
//#define CAMERA_MODEL_AI_THINKER // Has PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM
// ** Espressif Internal Boards **
//#define CAMERA_MODEL_ESP32_CAM_BOARD
//#define CAMERA_MODEL_ESP32S2_CAM_BOARD
//#define CAMERA_MODEL_ESP32S3_CAM_LCD


#include "camera_pins.h"

const char *ssid_Router     = "Abraham lincoln's Telephone Line (1202)";  //input your wifi name
const char *password_Router = "joemanscho";  //input your wifi passwords
camera_config_t config;

void startCameraServer();
void camera_init();

int overlayValue = 0; // Single overlay logic variable

float lastDistance = 0;     // Set an arbitrary last distance reading
int changeCount = 0;        // Counts the number of times the threshold is exceeded before accepting the reading
float valuesExceeded[3] = {0, 0, 0}; // Store last three changes

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  camera_init();

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  s->set_vflip(s, 0);        //1-Upside down, 0-No operation
  s->set_hmirror(s, 0);      //1-Reverse left and right, 0-No operation
  s->set_brightness(s, 1);   //up the blightness just a bit
  s->set_saturation(s, -1);  //lower the saturation

  WiFi.begin(ssid_Router, password_Router);
  WiFi.setSleep(false);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}

void loop() {
  // Trigger the sensor
  Serial.println("In the loop");
  float duration = getAverageDuration();
  Serial.print("Average duration = ");
  Serial.println(duration);
  // Convert duration to distance in cm
  float distance = (duration / 2.0) * 0.0343;

  // Threshold allows smaller  changes at close distances and larger changes at far distances
  float threshold = max(float(10), float(lastDistance * 0.2));  // Allow 20% variation of the last distance reading

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

void camera_init() {
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_QVGA;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 10;
  config.fb_count = 2;
}

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
