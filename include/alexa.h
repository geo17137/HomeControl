#ifndef ALEXA_H
#define ALEXA_H
#include <Arduino.h>
#include <PubSubClient.h>
#include "io.h"
#include "fauxmoESP.h"
#include "const.h"
#include "simple_param.h"
#include "files.h"
#include "mtr86.h"

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
extern SimpleParam* cPersistantParam;
extern FileLittleFS* filePersistantParam;
extern TACHE_T tache_t_monoPacOn;
extern TACHE_T tache_t_monoPacOff;

boolean _state;
unsigned char _value;

#define S_CUISINE "Four"
#define S_VMC "Vmc"
#define S_LANCE_ARROSAGE "Arrosage"
#define S_CLIM "Clim"
#define S_PAC_TEMP "Température"

const unsigned CUISINE = 0;
const unsigned VMC_DEV = 1;
const unsigned LANCE_ARROSAGE = 2;
const unsigned CLIM_ID = 3;
// const unsigned PAC_TEMP_ID = 4;
const unsigned SIZE = CLIM_ID + 1;

void cuisine();
void vmc();
void arrosage();
void clim();
void setClimTemp();

typedef void (*FuncPtr)();
static FuncPtr funcToCall[SIZE] = {cuisine, vmc, arrosage, clim};
#endif