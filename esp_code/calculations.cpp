#include "calculations.h"
#include <math.h>

// Convert raw ADC value to lux
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

// Convert lux to analog value for PWM output
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

// Calculate the lamp brightness adjustment
float calculateLlamp(float lux, float Ltarget) {
  float result = -pow(lux, 2) / Ltarget + Ltarget;
  return result > 0 ? result : 0;
}

// Blinks built-in
void blinkLED(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
}