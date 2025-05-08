#ifndef LUX_CONVERSION_H_
#define LUX_CONVERSION_H_

#include "info.h"
#include "iot_enable.h"

// Function prototypes
float convertToLux(int raw);
int convertToAnalog(float lux);
float calculateLlamp(float lux, float Ltarget);
void blinkLED(int times);

#endif
