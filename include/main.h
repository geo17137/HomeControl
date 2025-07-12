/**
 * @file main.h
 * @brief Header file for the Esp32_HomeCtrlDisplayA08 project.
 * 
 * This file contains the necessary includes, definitions, global variables, 
 * function prototypes, and class instances for the project.
 * 
 * @note This project uses various libraries such as Arduino, WiFi, MQTT, 
 * LiquidCrystal, and others. Ensure all dependencies are installed.
 * 
 * @details The project involves controlling home automation devices using an 
 * ESP32 microcontroller. It includes functionalities for WiFi connectivity, 
 * MQTT communication, OTA updates, LCD display, rotary encoder input, and 
 * various scheduled tasks.
 * 
 * @author Daniel
 * @date 2023
 */

#ifndef __MAIN_H
#define __MAIN_H

// Definitions for print modes
#define WEB_PRINT false
#define LOCAL_PRINT true
#define OUTPUT_PRINT LOCAL_PRINT

// Include necessary libraries and headers
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
//#include <Preferences.h>
// WiFi and MQTT credentials
const char *mqttServer = MQTT_SERVER;
const int  mqttPort = MQTT_PORT;
const char *mqttUser = MQTT_USER;
const char *mqttPassword = MQTT_PASSWORD;
const char *ssid = SSId;
const char *password = PASSWORD;
const char *hostname = HOSTNAME;

// WiFi client instance
WiFiClient wifiClient;

#ifdef TIME_SIMULATOR
static int h;
static int m;
#endif

// Parameter string for 24-hour scheduled cycles
const char *PARAM = "0:00:00:00:00:0:00:00:00:00:0:00:00:00:00:0:00:00:00:00:"
                    "0:00:00:00:00:0:00:00:00:00:0:00:00:00:00:0:00:00:00:00:"
                    "0:00:00:00:00:0:00:00:00:00:0:00:00:00:00:0:00:00:00:00:"
                    "0:00:00:00:00:0:00:00:00:00:0:00:00:00:00:0:00:00:00:00:"
                    "0:00:00:00:00:0:00:00:00:00:0:00:00:00:00:0:00:00:00:00";

// LCD display progress indicators
const char progress[3] = {'-', '|', '/'};

// Messages for rotary button actions
const char* msgRotary[7] = {"-Force output",
                            "-Set timer param",
                            "-Set scheduled act.", 
                            "-Daylight timeOffset",
                            "-Log setting",
                            "-Reboot", 
                            "-Quit"};

// Messages for setting outputs
const char* msgSetOutput[7] = {"-Watering lance",
                               "-Irrigation tank",
                               "-Cooking devices",
                               "-VMC power",
                               "-Watering lemon tree",
                               "-Power Heat pump",
                               "-Quit"};

// On/Off messages
const char* msgOnOff[2] = {"-On",
                           "-Off"};

// Yes/No messages
const char* msgYesNo[2] = {"-Yes",
                           "-No"};

// Timer duration messages
const char* msgTimer[5] =    {"-Time wattering",
                              "-Tank filling time",
                              "-Lemon watering tm",
                              "-Supres. filling tm",
                              "-Quit"};

// Summer time activation message
const char* msgSummerTm[1] = {"-Summer time ?"};

// Log setting activation message
const char* msgLogSetting[1] = {"-Log setting ?"};

// Scheduled action messages
const char* msgScheduledAction[5] ={"-Power cook schedule",
                                    "-Irrigation schedule",
                                    "-Lemon water schedul",
                                    "-Power PAC schedule", 
                                    "-VMC schedule"};

// OK message
const char* MSG_OK = "set";

// Current day for circuit2 command (persistent parameter)
static unsigned joursCircuit2;

char date[20]; // Buffer for date string


// Boolean flags for various states
boolean isWatering;
boolean isTankFilling;
boolean startSupressorFilling2;
boolean startSupressorFilling;
boolean irSendPacOff;
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
boolean supressorFillingMonoStart;
boolean supressorFillingSecurity;
boolean monoSurpressorSecurityStarted;
boolean wifiConnected;

unsigned cmdVanneEst = 1;
unsigned onVmc;
unsigned wateringNoTimeOut;
unsigned n_supressorFillingInTime;
int vmcMode;
char rssi_buffer[10];

// Function pointers for various callbacks
void (*display)();
void (*onSingleClick)();
void (*onDoubleClick)();
void (*onRotary)();
void (*onLoopTic)();
void (*onLoopTic2)();
void (*funcToCall)();
void (*funcToCall2)();

// Pointers to dynamically allocated classes
static FileLittleFS* fileParam;
static FileLittleFS *fileLogs;
       FileLittleFS *fileDlyParam;
       FileLittleFS* fileGlobalScheduledParam;
       FileLittleFS* filePersistantParam;
       FileLittleFS* fileDateParam;

// Static instances of various classes
LiquidCrystal_I2C lcd(I2C_ADR, 4, 5, 6, 16, 11, 12, 13, 14, POSITIVE);
WiFiUDP ntpUDP;
PubSubClient mqttClient(wifiClient);
AiEsp32RotaryEncoder rotaryEncoder =
               AiEsp32RotaryEncoder(
               ROTARY_ENCODER_A_PIN,
               ROTARY_ENCODER_B_PIN,
               ROTARY_ENCODER_BUTTON_PIN,
               ROTARY_ENCODER_VCC_PIN,
               ROTARY_ENCODER_STEPS);
OneButton button(ROTARY_ENCODER_BUTTON_PIN, true);

// Pointers to dynamically allocated objects
static ESP32Time* rtc;
Param* cParam;
SimpleParam* cDlyParam;
SimpleParam* cGlobalScheduledParam;
SimpleParam* cPersistantParam;

// Pre-declared function prototypes
void PubSubCallback(char* topic, byte* payload, unsigned int length);
void writeLogs(const char * msg);
void deleteLogs(void);
const char *getDate(void);
void localLoop(void);
void schedule();
boolean isEdge(int nButton);
void startWatering(int timeout);
void stopWatering();
void startWatering(int timeout);
void stopWatering(void);
void startTankFilling(void);
void stopTankFilling(void);
void localLoop(void);
void setVmc(int cmd);

// External function prototypes
extern void backLightOn();
extern void backLightOff();
extern void _ioDisplay();
extern char* readPortIo_O();
extern char* readPortIo_I();
extern unsigned testPortIO_O();
extern unsigned testPortIO_I();
extern void ioDisplay();
extern void initDisplay();

// RTOS task IDs (monostable) format Mtr86
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
TACHE_T tache_t_monoSurpressorSecurity;

// RTOS task functions (monostable) format Mtr86
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
void monoSurpressorSecurity(TimerHandle_t xTimer);
#ifdef EXEC_TIME_MEASURE
unsigned long Temps_stop_us;
unsigned long Temps_start_us;
unsigned long Duree_us;

/**
 * @brief Stop execution time measurement and print the duration.
 */
inline void time_exec_stop() {
  Temps_stop_us = micros();
  Duree_us = Temps_stop_us - Temps_start_us;
  Serial.print(Duree_us);
  Serial.print("    \rus:");
}

/**
 * @brief Start execution time measurement.
 */
inline void time_exec_start() {
  Temps_start_us = micros();
}
#endif

// Inline functions
/**
 * @brief Initialize OTA updates.
 */
inline void initOTA() {
  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.begin();
}

/**
 * @brief Compare two C strings.
 * @param s1 First string.
 * @param s2 Second string.
 * @return boolean True if strings are equal, false otherwise.
 */
inline boolean cmp(const char* s1, const char* s2) {
  return strcmp(s1, s2) == 0;
}

#endif