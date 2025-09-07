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

boolean state;
#define S_CUISINE "Four"
#define S_LOW_VMC "Vmc"
#define S_FAST_VMC "Fast Vmc"
#define S_VMC_PROG "Vmc prog"
#define S_LANCE_ARROSAGE "Arrosage"
const unsigned CUISINE = 0;
const unsigned LOW_VMC = 1;
const unsigned FAST_VMC = 2;
const unsigned VMC_PROG = 3;
const unsigned LANCE_ARROSAGE = 4;
const unsigned size = LANCE_ARROSAGE + 1;

void cuisine();
void lowVmc();
void fastVmc();
void vmcProg();
void arrosage();

typedef void (*FuncPtr)();
static FuncPtr funcToCall[size] = {cuisine, lowVmc, fastVmc, vmcProg, arrosage};
#endif