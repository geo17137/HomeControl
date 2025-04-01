#ifndef LOCAL_LOOP_H
#define LOCAL_LOOP_H
#include <Arduino.h>
#include <esp_task_wdt.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include "files.h"
#include "display.h"
#include "mtr86.h"
#include "const.h"
#include "io.h"

extern boolean isEdge(int nButton);
extern boolean isWatering;
extern boolean isTankFilling;
extern boolean startSupressorFilling;
extern boolean startSupressorFilling2;
extern boolean msgRearm;
extern boolean erreurSupresseur;
extern boolean erreurSupresseurEvent;
extern boolean erreurPompe;
extern boolean erreurPompEvent;
extern boolean isLcdDisplayOn;
extern boolean electricalPanelOpen;
extern boolean ioDisplayFlag;
extern boolean bootDisplayOff;
extern boolean monoSurpressorSecurityStarted;
extern boolean supressorFillingSecurity;

extern unsigned n_supressorFillingInTime;
extern int     powerWrite;

extern void (*onSingleClick)();
//extern void (*rotaryTask)();

extern FileLittleFS* filePowerPac;
extern FileLittleFS* fileDlyParam;

extern void ioDisplay();
extern void ioDisplay2();

extern TACHE_T tache_t_monoPacOff;
extern TACHE_T tache_t_monoPacOn;
extern TACHE_T tache_t_watering;
extern TACHE_T tache_t_tankFilling;
extern TACHE_T tache_t_surpressorFilling;
extern TACHE_T tache_t_cmdEvEst;
extern TACHE_T tache_t_cmdVmcBoard;
extern TACHE_T tache_t_backLight2;
extern TACHE_T tache_t_monoSurpressorSecurity;

extern SimpleParam* cDlyParam;

extern PubSubClient mqttClient;
extern void startWatering(int timeout);
extern void stopWatering();
extern void startTankFilling();
extern void stopTankFilling();
extern void writeLogs(const char* log);
extern void logsWrite(const char* log);
extern void logsUpdate();
// void initMQTTClient();
void localLoop();

extern inline void off(int adr);
extern inline void on(int adr);
extern void (*display)();
extern void nullFunc();
extern void backLightOn();
extern void backLightOff();
extern char* readPortIo_I();
extern void backLightOff();
#endif