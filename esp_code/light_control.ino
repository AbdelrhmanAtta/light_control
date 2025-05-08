#include <Adafruit_VL53L0X.h>
#include "WiFi.h"
#include "AdafruitIO_WiFi.h"
#include "info.h"
#include "iot_enable.h"

Adafruit_VL53L0X sensor1 = Adafruit_VL53L0X();
Adafruit_VL53L0X sensor2 = Adafruit_VL53L0X();

bool triggered1 = false, triggered2 = false;
bool waitingReset = false;

unsigned long lastTargetLuxUpdate = 0;  // Time tracker for last targetLux update

void setup() {
  Serial.begin(115200);
  pinMode(XSHUT1, OUTPUT);
  pinMode(XSHUT2, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(XSHUT1, LOW);
  digitalWrite(XSHUT2, LOW);
  delay(10);

  Wire.begin(I2C_SDA, I2C_SCL);

  digitalWrite(XSHUT1, HIGH);
  delay(10);
  if (!sensor1.begin(0x41)) {
    Serial.println("Sensor 1 failed");
    while (1);
  }

  digitalWrite(XSHUT2, HIGH);
  delay(10);
  if (!sensor2.begin(0x42)) {
    Serial.println("Sensor 2 failed");
    while (1);
  }

  Serial.println("Both sensors started.");
  Serial.print("Starting people count: ");
  Serial.println(peopleCount);

  io.connect();
  while (io.status() < AIO_CONNECTED) {
    delay(500);
    Serial.println("Connecting to Adafruit IO...");
  }
  Serial.println("Connected to Adafruit IO!");

  // Subscribe to targetLux feed to get updates from Adafruit IO
  targetLuxFeed->onMessage(targetLuxHandler);  // Correct way to subscribe to the feed
}

void loop() {
  VL53L0X_RangingMeasurementData_t measure1, measure2;
  sensor1.rangingTest(&measure1, false);
  sensor2.rangingTest(&measure2, false);

  int dist1 = measure1.RangeMilliMeter;
  int dist2 = measure2.RangeMilliMeter;

  if (!waitingReset) {
    if (dist1 < DISTANCE_THRESHOLD && !triggered1 && !triggered2) triggered1 = true;
    if (dist2 < DISTANCE_THRESHOLD && !triggered2 && !triggered1) triggered2 = true;

    if (triggered1 && dist2 < DISTANCE_THRESHOLD) {
      peopleCount++;
      Serial.println(peopleCount);
      blinkLED(2);
      waitingReset = true;
      sendPeopleCount();
    }

    if (triggered2 && dist1 < DISTANCE_THRESHOLD) {
      if (peopleCount > 0) peopleCount--;
      Serial.println(peopleCount);
      blinkLED(1);
      waitingReset = true;
      sendPeopleCount();
    }
  }

  if (waitingReset && dist1 > DISTANCE_THRESHOLD && dist2 > DISTANCE_THRESHOLD) {
    triggered1 = triggered2 = false;
    waitingReset = false;
  }

  // Check if enough time has passed before sending targetLux
  unsigned long currentTime = millis();
  if (currentTime - lastTargetLuxUpdate >= LDR_SEND_INTERVAL) {
    // Send the current targetLux value to the Serial monitor
    Serial.print("Current targetLux: ");
    Serial.println(targetLux);
    lastTargetLuxUpdate = currentTime;
  }

  sendLdrIfDue();
  io.run();
  delay(30);
}
