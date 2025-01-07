#ifndef __MAIN_H
#define __MAIN_H

#define WEB_PRINT false
#define LOCAL_PRINT true
#define OUTPUT_PRINT LOCAL_PRINT

#include <Arduino.h>
#include <string>
#include "const.h"
#include <WiFiUdp.h>
#ifdef WIFI_MANAGER
#include <WiFiManager.h>
#endif
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include "files.h"
#include <ESP32Time.h>
#include <esp_task_wdt.h>
#include <LiquidCrystal_I2C.h>
#include "display.h"
#include "mtr86.h"
#include "param.h"
#include "simple_param.h"
#include "display.h"
#include "rotary_encoder.h"
#include "loop_prog.h"
#include "local_loop.h"
#include "const.h"

#include "../secret/password.h"
/*
secret/password.h
#define SSId "xxxxxxx"
#define PASSWORD    "xxxxxxx"
#define MQTT_SERVER "XXX.XXX.XXXX.XXX"
#define MQTT_PORT xxxx
#define MQTT_USER "xxxxxxxx"
#define MQTT_PASSWORD "xxxxxxxxxx"
#define HOSTNAME  "xxxxxxxxxx"
*/
const char *mqttServer = MQTT_SERVER;
const int  mqttPort = MQTT_PORT;
const char *mqttUser = MQTT_USER;
const char *mqttPassword = MQTT_PASSWORD;
const char *ssid = SSId;
const char *password = PASSWORD;
const char *hostname = HOSTNAME;

WiFiClient wifiClient;

#ifdef TIME_SIMULATOR
static int h;
static int m;
// #define MUL_FAC 1
// #else
// #define MUL_FAC 59 * 1000
#endif

// Format de la chaine paramétre des cycles programmés sur 24H enregistrés sur flash
const char *PARAM = "0:00:00:00:00:0:00:00:00:00:0:00:00:00:00:0:00:00:00:00:"
                    "0:00:00:00:00:0:00:00:00:00:0:00:00:00:00:0:00:00:00:00:"
                    "0:00:00:00:00:0:00:00:00:00:0:00:00:00:00:0:00:00:00:00:"
                    "0:00:00:00:00:0:00:00:00:00:0:00:00:00:00:0:00:00:00:00:"
                    "0:00:00:00:00:0:00:00:00:00:0:00:00:00:00:0:00:00:00:00";

// Index d'activité sur l'afficheur LCD
const char progress[3] = {'-', '|', '/'};

// Messages affiches par le bouton rotatif sur tableau électrique
// L'utilisation de l'anglais permet d'avoir des messages plus courts et évite les accents
// de la langue française
const char* msgRotary[7] = {   "-Force output",
                               "-Set timer param",
                               "-Set scheduled act.", 
                               "-Daylight timeOffset",
                               "-Log setting",
                               "-Reboot", 
                               "-Quit"};
// Messages des actions correspondantes
const char* msgSetOutput[7] = {"-Watering lance",
                               "-Irrigation tank",
                               "-Cooking devices",
                               "-VMC power",
                               "-Watering lemon tree",
                               "-Power Heat pump",
                               "-Quit"};
// Messages actions
const char* msgOnOff[2] = {   "-On",
                              "-Off"};
// Messages validation
const char* msgYesNo[2] = {"-Yes",
                           "-No"};
// Durée des temporisateurs
const char* msgTimer[5] =    {"-Time wattering",
                              "-Tank filling time",
                              "-Lemon watering tm",
                              "-Supres. filling tm",
                              "-Quit"};
// Activation heure d'été/hivers
const char* msgSummerTm[1] = {"-Summer time ?"};
// Activation logs
const char* msgLogSetting[1] = {"-Log setting ?"};
// Activations des des cycles programmés sur 24H
const char* msgScheduledAction[5] =
                            { "-Power cook schedule",
                              "-Irrigation schedule",
                              "-Lemon water schedul",
                              "-Power PAC schedule", 
                              "-VMC schedule"};

const char* MSG_OK = "set";

// Jours courant pour la commande du circuit2 
// Paramètre persistant
static unsigned joursCircuit2;

boolean isWatering;
boolean isTankFilling;
boolean startSupressorFilling2;
boolean startSupressorFilling;
boolean irSendOn;
boolean msgRearm;
boolean erreurSupresseur;
boolean erreurSupresseurEvent;
boolean erreurPompe;
boolean erreurPompEvent;
boolean isLcdDisplayOn;

boolean electricalPanelOpen;
boolean bootDisplayOff;
boolean vmcFast;
boolean mqttConnect;

unsigned cmdVanneEst = 1;
unsigned onVmc;
unsigned wateringNoTimeOut;
int vmcMode;
char rssi_buffer[10];

// Pointeurs de fonctions
void (*display)();
void (*onSingleClick)();
void (*onDoubleClick)();
void (*onRotary)();
void (*onLoopTic)();
void (*onLoopTic2)();
void (*funcToCall)();
void (*funcToCall2)();

// Pointeurs vers les classes allouées dynamiquement
static FileLittleFS* fileParam;
static FileLittleFS *fileLogs;
       FileLittleFS *fileDlyParam;
       FileLittleFS* fileGlobalScheduledParam;
       FileLittleFS* filePersistantParam;

// Instanciations statique 
       // Afficheur LCD
LiquidCrystal_I2C lcd(I2C_ADR, 4, 5, 6, 16, 11, 12, 13, 14, POSITIVE);
       // WiFi
WiFiUDP ntpUDP;
       // Client MQTT
PubSubClient mqttClient(wifiClient);
       // Encodeur rotatif 
AiEsp32RotaryEncoder rotaryEncoder =
         AiEsp32RotaryEncoder(
         ROTARY_ENCODER_A_PIN,
         ROTARY_ENCODER_B_PIN,
         ROTARY_ENCODER_BUTTON_PIN,
         ROTARY_ENCODER_VCC_PIN,
         ROTARY_ENCODER_STEPS);
       // Bouton poussoir encodeur rotatif
OneButton button(ROTARY_ENCODER_BUTTON_PIN, true);
       
// Pointeurs vers les objets alloués dynamiquement
static ESP32Time* rtc;
Param* cParam;
SimpleParam* cDlyParam;
SimpleParam* cGlobalScheduledParam;
SimpleParam* cPersistantParam;


// Fonctions prédéclarées        
void PubSubCallback(char* topic, byte* payload, unsigned int length);
void openClose(void);
void writeLogs(const char * msg);
void deleteLogs(void);
const char *getDate(void);
void openClose(void);
void localLoop(void);
// void schedule(TimerHandle_t xTimer);
void schedule();
boolean isEdge(int nButton);
void startWatering(int timeout);
void stopWatering();
char* readPortIo_O();
char* readPortIo_I();
unsigned testPortIO_O();
unsigned testPortIO_I();
void ioDisplay();
void initDisplay();
void startWatering(int timeout);
void stopWatering(void);
void startTankFilling(void);
void stopTankFilling(void);
void localLoop(void);
void setVmc(int cmd);
void backLightOn();
void backLightOff();
void _ioDisplay();

// id Taches RTOS (monostables) format Mtr86
TACHE_T tache_t_monoPacOff;
TACHE_T tache_t_monoPacOn;
TACHE_T tache_t_watering;
TACHE_T tache_t_tankFilling;
TACHE_T tache_t_surpressorFilling;
TACHE_T tache_t_cmdEvEst;
TACHE_T tache_t_cmdVmcBoard;
TACHE_T tache_t_backLight;
TACHE_T tache_t_backLight2;
TACHE_T tache_t_encoderTask;
TACHE_T tache_t_defaultDisplay;
TACHE_T tache_t_offCircuit2;
TACHE_T tache_t_monoDebit;
// Taches RTOS (monostables) format Mtr86
void monoPacOff(TimerHandle_t xTimer);
void monoPacOn(TimerHandle_t xTimer);
void monoWatering(TimerHandle_t xTimer);
void monoTankFilling(TimerHandle_t xTimer);
void monoSurpressorFilling(TimerHandle_t xTimer);
void monoCmdEvEst(TimerHandle_t xTimer);
void monoCmdVmcBoard(TimerHandle_t xTimer);
void monoCmdBackLight(TimerHandle_t xTimer);
void monoCmdBackLight2(TimerHandle_t xTimer);
void monoDefaultDisplay(TimerHandle_t xTimer);
void monoOffCircuit2(TimerHandle_t xTimer);
void monoDebit(TimerHandle_t xTimer);


#ifdef EXEC_TIME_MEASURE
unsigned long Temps_stop_us;
unsigned long Temps_start_us;
unsigned long Duree_us;

inline void time_exec_stop() {
  Temps_stop_us = micros();
  Duree_us = Temps_stop_us - Temps_start_us;
  Serial.print(Duree_us);
  Serial.print("    \rus:");
}

inline void time_exec_start() {
  Temps_start_us = micros();
}
#endif

// Fonctions inline
/**
 * @brief initOTA
 * 
 */
inline void initOTA() {
  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.begin();
}
/**
 * @brief Comparaison de deux chaines C
 * @param s1 
 * @param s2 
 * @return boolean 
 */
inline boolean cmp(const char* s1, const char* s2) {
  return strcmp(s1, s2) == 0;
}

#endif