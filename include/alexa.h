#ifndef ALEXA_H
#define ALEXA_H
#include <Arduino.h>
#include <PubSubClient.h>
#include "io.h"
#include "fauxmoESP.h"
#include "const.h"

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

boolean _state;
unsigned char _value;

#define S_CUISINE "Four"
#define S_VMC "Vmc"
#define S_LANCE_ARROSAGE "Arrosage"
const unsigned CUISINE = 0;
const unsigned VMC_DEV = 1;
const unsigned LANCE_ARROSAGE = 2;
const unsigned size = LANCE_ARROSAGE + 1;

void cuisine();
void vmc();
void arrosage();

typedef void (*FuncPtr)();
static FuncPtr funcToCall[size] = {cuisine, vmc, arrosage};
#endif