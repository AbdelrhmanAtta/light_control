#ifndef IOT_ENABLE_H_
#define IOT_ENABLE_H_

#include "info.h"
#include <cmath>
#include "calculations.h"
#include "WiFi.h"
#include "AdafruitIO_WiFi.h"

// Declare external variables
extern AdafruitIO_WiFi io;
extern AdafruitIO_Feed *countFeed;
extern AdafruitIO_Feed *lightFeed;
extern AdafruitIO_Feed *llampFeed;
extern AdafruitIO_Feed *targetLuxFeed;  // Declare the targetLux feed
extern int peopleCount;
extern float targetLux;
extern unsigned long lastLdrSend;

// Function prototypes
void sendPeopleCount(void);
void sendLdrIfDue(void);
void targetLuxHandler(AdafruitIO_Data *data);  // Declare targetLuxHandler function prototype

#endif
