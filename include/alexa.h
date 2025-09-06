#ifndef ALEXA_H
#define ALEXA_H
#include <Arduino.h>
#include <PubSubClient.h>
#include "io.h"
#include "fauxmoESP.h"
#include "const.h"

#define CUISINE "Four"
#define LOW_VMC "Vmc"
#define FAST_VMC "Fast Vmc"
#define PROG_VMC "Vmc prog"
#define LANCE_ARROSAGE "Arrosage"

extern fauxmoESP fauxmo;
void initAlexa();
void addDevices();
extern void setVmc(int cmd);
// extern void publish(const char* topic, const char* payload);
extern PubSubClient mqttClient;

// extern void gpioOff(int adr);
// extern void gpioOn(int adr);
extern void startWatering(int timeout);
extern void stopWatering();
extern unsigned int wateringNoTimeOut;
#endif