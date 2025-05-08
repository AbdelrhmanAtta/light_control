#include "iot_enable.h"

// Define variables
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);
AdafruitIO_Feed *countFeed = io.feed("count");
AdafruitIO_Feed *lightFeed = io.feed("light");
AdafruitIO_Feed *llampFeed = io.feed("llamp");
AdafruitIO_Feed *targetLuxFeed = io.feed("targetLux");  // Define targetLux feed
int peopleCount = 0;
float targetLux = 600;
unsigned long lastLdrSend = 0;

void sendPeopleCount(void) {
  countFeed->save(peopleCount);
}

void sendLdrIfDue(void) {
  unsigned long now = millis();
  if (now - lastLdrSend >= LDR_SEND_INTERVAL) {
    float Llamp = 0; // Declare Llamp here

    int raw = analogRead(LDR_PIN);
    float lux = convertToLux(raw);
    Serial.print("LDR Raw: ");
    Serial.print(raw);
    Serial.print(" -> Lux: ");
    Serial.println(lux);

    if (peopleCount >= 1) {
      Llamp = calculateLlamp(lux, targetLux);  // Calculate Llamp if peopleCount >= 1
    } else {
      Llamp = 0;  // Set Llamp to 0 if peopleCount is 0
    }

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




// This function will handle updates from the targetLux feed on Adafruit IO
void targetLuxHandler(AdafruitIO_Data *data) {
  targetLux = data->toFloat();
  Serial.print("Updated targetLux value: ");
  Serial.println(targetLux);
}
