#include <Adafruit_VL53L0X.h>
#include <Preferences.h>
#include "WiFi.h"
#include "AdafruitIO_WiFi.h"
#include "info.h"

Adafruit_VL53L0X sensor1 = Adafruit_VL53L0X();
Adafruit_VL53L0X sensor2 = Adafruit_VL53L0X();

#define XSHUT1 25
#define XSHUT2 32
#define I2C_SDA 21
#define I2C_SCL 22
#define THRESHOLD 2000
#define LED_PIN 2
#define RELAY_PIN 14
#define PEOPLE_COUNT_THRESHOLD 3

#define LDR_PIN 36
#define LDR_SEND_INTERVAL 30000
#define OUTPUT_PIN 23

float targetLux = 600;
Preferences prefs;

int peopleCount = 0;
bool triggered1 = false, triggered2 = false;
bool waitingReset = false;

unsigned long lastLdrSend = 0;

AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);
AdafruitIO_Feed *countFeed = io.feed("count");
AdafruitIO_Feed *lightFeed = io.feed("light");
AdafruitIO_Feed *llampFeed = io.feed("llamp");

void setup() {
  Serial.begin(115200);
  pinMode(XSHUT1, OUTPUT);
  pinMode(XSHUT2, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);

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

  prefs.begin("people", false);
  peopleCount = prefs.getInt("count", 0);
  Serial.println("Both sensors started.");
  Serial.print("Starting people count: ");
  Serial.println(peopleCount);

  io.connect();
  while (io.status() < AIO_CONNECTED) {
    delay(500);
    Serial.println("Connecting to Adafruit IO...");
  }
  Serial.println("Connected to Adafruit IO!");

  updateRelayState();
}

void loop() {
  VL53L0X_RangingMeasurementData_t measure1, measure2;
  sensor1.rangingTest(&measure1, false);
  sensor2.rangingTest(&measure2, false);

  int dist1 = measure1.RangeMilliMeter;
  int dist2 = measure2.RangeMilliMeter;

  if (!waitingReset) {
    if (dist1 < THRESHOLD && !triggered1 && !triggered2) triggered1 = true;
    if (dist2 < THRESHOLD && !triggered2 && !triggered1) triggered2 = true;

    if (triggered1 && dist2 < THRESHOLD) {
      peopleCount++;
      prefs.putInt("count", peopleCount);
      Serial.println(peopleCount);
      blinkLED(2);
      waitingReset = true;
      sendPeopleCount();
    }

    if (triggered2 && dist1 < THRESHOLD) {
      if (peopleCount > 0) peopleCount--;
      prefs.putInt("count", peopleCount);
      Serial.println(peopleCount);
      blinkLED(1);
      waitingReset = true;
      sendPeopleCount();
    }
  }

  if (waitingReset && dist1 > 2000 && dist2 > 2000) {
    triggered1 = triggered2 = false;
    waitingReset = false;
  }

  updateRelayState();
  sendLdrIfDue();
  io.run();
  delay(30);
}

void blinkLED(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
}

void updateRelayState() {
  if (peopleCount > PEOPLE_COUNT_THRESHOLD)
    digitalWrite(RELAY_PIN, HIGH);
  else
    digitalWrite(RELAY_PIN, LOW);
}

float convertToLux(int raw) {
  if (raw <= 0) return 0.0;
  if (raw <= 45) return float(raw) * (5.0 / 45.0);
  if (raw <= 190) return 5 + float(raw - 45) / (190 - 45) * (10 - 5);
  if (raw <= 220) return 10 + float(raw - 190) / (220 - 190) * (21 - 10);
  if (raw <= 1100) return 21 + float(raw - 220) / (1100 - 220) * (132 - 21);
  if (raw <= 2200) return 132 + float(raw - 1100) / (2200 - 1100) * (2000 - 132);
  float slope = float(2000 - 132) / (2200 - 1100);
  return 2000 + (raw - 2200) * slope;
}

void sendLdrIfDue() {
  unsigned long now = millis();
  if (now - lastLdrSend >= LDR_SEND_INTERVAL) {
    int raw = analogRead(LDR_PIN);
    float lux = convertToLux(raw);
    Serial.print("LDR Raw: ");
    Serial.print(raw);
    Serial.print(" -> Lux: ");
    Serial.println(lux);
    float Llamp = calculateLlamp(lux, targetLux);

    int analogwritevalue = convertToAnalog(Llamp);
    analogWrite(OUTPUT_PIN, analogwritevalue);

    Serial.print("Analog write Value: ");
    Serial.print(analogwritevalue);
    Serial.print(" | Lux: ");
    Serial.print(lux);
    Serial.print(" | Llamp: ");
    Serial.println(Llamp);
    delay(10);
    raw = analogRead(LDR_PIN);
    lux = convertToLux(raw) + Llamp;
    Serial.print("LDR Raw: ");
    Serial.print(raw);
    Serial.print(" -> Lux: ");
    Serial.println(lux);

    lightFeed->save(lux);
    llampFeed->save(Llamp);

    lastLdrSend = now;
  }
}

void sendPeopleCount() {
  countFeed->save(peopleCount);
}

int convertToAnalog(float lux) {
  if (lux <= 0) return 0;
  if (lux <= 35.51) return 5;
  if (lux <= 49.38) return 10;
  if (lux <= 62.63) return 15;
  if (lux <= 74.23) return 20;
  if (lux <= 84.45) return 25;
  if (lux <= 94.54) return 30;
  if (lux <= 104.00) return 35;
  if (lux <= 112.32) return 40;
  if (lux <= 120.77) return 45;
  if (lux <= 131.37) return 50;
  if (lux <= 208.42) return 55;
  if (lux <= 284.84) return 60;
  if (lux <= 408.80) return 65;
  if (lux <= 483.52) return 70;
  if (lux <= 515.79) return 75;
  if (lux <= 536.17) return 80;
  if (lux <= 558.24) return 85;
  if (lux <= 619.38) return 90;
  if (lux <= 636.36) return 95;
  if (lux <= 734.85) return 100;
  if (lux <= 731.46) return 105;
  if (lux <= 738.25) return 110;
  if (lux <= 762.03) return 115;
  if (lux <= 953.92) return 120;
  if (lux <= 1008.26) return 125;
  if (lux <= 841.84) return 130;
  if (lux <= 870.71) return 135;
  if (lux <= 826.56) return 140;
  if (lux <= 760.33) return 145;
  if (lux <= 765.42) return 150;
  if (lux <= 807.88) return 155;
  if (lux <= 811.27) return 160;
  if (lux <= 821.46) return 165;
  if (lux <= 867.31) return 170;
  if (lux <= 870.71) return 175;
  if (lux <= 918.26) return 180;
  if (lux <= 952.22) return 190;
  if (lux <= 981.09) return 195;
  if (lux <= 976.00) return 200;
  if (lux <= 1057.51) return 205;
  if (lux <= 1108.45) return 210;
  if (lux <= 1123.74) return 215;
  if (lux <= 1215.44) return 220;
  if (lux <= 1229.03) return 225;
  if (lux <= 1247.71) return 230;
  if (lux <= 1276.57) return 235;
  if (lux <= 1303.75) return 240;
  if (lux <= 1329.22) return 245;
  return 250;
}

float calculateLlamp(float lux, float Ltarget) {
  float result = -pow(lux, 2) / Ltarget + Ltarget;
  return result > 0 ? result : 0;
}
