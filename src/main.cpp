
/*
   Automate remplaçant le ZELIO
   ******************************************************
   IMPORTANT Si la taille des messages dépasse 256 octets
   modifier dans la librairie PubSubClient.h
   La chaine param fait actuellement 279 octets
   MQTT_MAX_PACKET_SIZE = 300 est insuffisant !!
   #define MQTT_MAX_PACKET_SIZE 256
   par
   #define MQTT_MAX_PACKET_SIZE 512
   ou appeler setBufferSize(MQTT_MAX_BUFFER_SIZE);
   Define stack_size in
   C:\Users\dtsch\.platformio\packages\framework-arduinoespressif32\tools\sdk\esp32\dio_qspi\include
   See
   https://community.platformio.org/t/esp32-stack-configuration-reloaded/20994/2
   ******************************************************
   Versions: 31/3/23
    - Encapsulation des variables chaines globales de paramétrage
    - Unification des classe de paramétrage Dlyparam, Persistant et GlobalScheduledParam dans SimpleParam
    - Augmentation de la taille de la pile
    - Unification de l'envoi des logs (nouveau fichier logs.cpp)
    - Logs sur perte de WiFi
   Version 2.6.5 3/9/23
    - Configuration dynamique des monostables arrosage, remplissage réservoir, irrigation vanne est
   Version 2.6.6 11/9/23
    - Correction mineure dans les messages de loopProg
   Version 2.7.0 27/10/23
    - Utilisation de la librairie LittleFS Intégrée au noyau
   Version 2.8.0 30/10/23
    - Réduction de l'allumage lcd lors des reboot
    - Timeout de l'allumage lcd lorsque le panneau electrique est ouvert
   Version 2.9.0 30/11/23
    - Remplacement contacteur PAC ferme au repos par contacteur ouvert au repos
      inversion de la logique de commande de la PAC
    - Correction d'un bug bloquant la maj de l'affichage local (variable tpsProg mise à jour dans loop test wifi)
   Version 2.9.1 08/1/24
    - Force l'affichage par definition du symbole FORCE_DISPLAY
      Necessite une recompilation
   Version 2.9.2 28/3/24
    - INTERVAL_RESET_WDT  100 -> 500
    // _ INTERVAL_WIFI_TEST 60*1000 -> 3*60*1000
   Version 2.10.0 12/5/24
    - Ajout du regagle de débit vanne est par regalge du rapport cyclique on/off
   Version 3.0.0 9/6/24
    - Génération d'une version pour carte ES32A08 en definissant ES32A08 dans const.h
    - Regrouppement de toutes les E/S gpio dans io.cpp
   Version 2024.6.22
    - Changment numérotation version
    - Affichage rssi sur l'écran lcd et envoi de message mqtt homecontrol/wifi_streng
   Version 2024.6.25
    - Possibilité de bloquer le remplissage du supresseur
   Version 2024.6.25
    - Test périodique d'envoi de messages mqtt. En cas de non réponse reboot
   Version 2024.9.19
    - Correction bug :
      commande PAC depuis la console 
      commande arrosage et irrigation avec la télécommande 
   Version 2024.9.20
      regroupement des messages lcd
      correction bug mineurs 
   Version 2024.10.15
      envoi de message on/off homecontrol/status_cuisine lors des commandes 
   Version 2024.12.31
       modification monostable commande vmc  
   Version 2025.01.04
       optimisation de la fonction schedule   
   Version 2025.01.07
       Optimisation de l'affichage des capteurs et actionneurs                 
   Version 2025.01.09
       Signale séquence d'arrêt de la PAC sur l'écran LCD (K2 ->0->1->0..)     
*/
#include "main.h"
#include "io.h"

#ifdef WEB_SERIAL
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSerial.h>
AsyncWebServer server(80);

void recvMsg(uint8_t* data, size_t len) {
  WebSerial.println("Received Data...");
  String d = "";
  for (int i = 0; i < len; i++) {
    d += char(data[i]);
  }
  WebSerial.println(d);
}
#endif

#if defined(DEBUG_OUTPUT) || defined(DEBUG_OUTPUT_LOOP2) || defined(DEBUG_OUTPUT_SCHEDULE)
void print(char* msg, boolean local) {
  if (local)
    Serial.println(msg);
#ifdef WEB_SERIAL    
  else
    WebSerial.println(msg);
#endif    
}

void print(String msg, boolean local) {
  if (local)
    Serial.println(msg);
#ifdef WEB_SERIAL
  else
    WebSerial.println(msg);
#endif
}

void print(const char* msg, boolean local) {
  if (local)
    Serial.println(msg);
#ifdef WEB_SERIAL
  else
    WebSerial.println(msg);
#endif
}
#endif

void printMqttDebugValue(const char* value) {
  mqttClient.publish(TOPIC_DEBUG_VALUE, value);
}

const char* bootRaison() {
  esp_reset_reason_t reason = esp_reset_reason();
  switch (reason) {
  case ESP_RST_UNKNOWN:
    return "Reset unknown";
    break;
  case ESP_RST_POWERON:
    return "Reset power-on";
    break;
    // case ESP_RST_EXT:
    //   return "Reset by external pin (not applicable for ESP32)";
    //   break;
  case ESP_RST_SW:
    return "Reset via esp_restart";
    break;
  case ESP_RST_PANIC:
    return "Reset exception/panic";
    break;
  case ESP_RST_INT_WDT:
    return "Reset interrupt watchdog";
    break;
  case ESP_RST_TASK_WDT:
    return "Reset due to task watchdog";
    break;
  case ESP_RST_WDT:
    return "Reset due other watchdogs";
    break;
  case ESP_RST_DEEPSLEEP:
    return "Reset after exiting deep sleep mode";
    break;
  case ESP_RST_BROWNOUT:
    return "Brownout reset";
    break;
  case ESP_RST_SDIO:
    return "Reset over SDIO";
    break;
  default:
    return "";
  }
}

FileLittleFS* initParam(boolean force) {
  FileLittleFS* fileParam = new FileLittleFS(PARAM_FILE_NAME);
  if (!FileLittleFS::exist(PARAM_FILE_NAME) || force) {
    fileParam->writeFile(PARAM, "w");
  }
  cParam = new Param(fileParam->readFile().c_str());
  fileParam->close();
#ifdef DEBUG_OUTPUT
  cParam->print();
#endif
  return fileParam;
}

FileLittleFS* initDlyParam(boolean force) {
  FileLittleFS* fileDlyParam = new FileLittleFS(DLY_PARAM_FILE_NAME);
  if (!FileLittleFS::exist(DLY_PARAM_FILE_NAME) || force) {
    fileDlyParam->writeFile(DEFAUT_DELAY_PARAM, "w");
  }
  cDlyParam = new SimpleParam(fileDlyParam->readFile().c_str(), ":", N_DLY_PARAM);
  fileDlyParam->close();
#ifdef DEBUG_OUTPUT
  cDlyParam->print();
#endif
  return fileDlyParam;
}

FileLittleFS* initPersitantFileDevice(boolean force) {
  FileLittleFS* filePersistantParam = new FileLittleFS(FILE_PERSISTANT_DEVICE);
  if (!FileLittleFS::exist(FILE_PERSISTANT_DEVICE) || force) {
    filePersistantParam->writeFile(PERSISTANT, "w");
  }
  cPersistantParam = new SimpleParam(filePersistantParam->readFile().c_str(), ":", N_PERSISTANT_PARAM);
  filePersistantParam->close();
#ifdef DEBUG_OUTPUT
  cPersistantParam->print();
#endif
  return filePersistantParam;
}

FileLittleFS* initGlobalScheduledParam(boolean force) {
  FileLittleFS* fileGlobalScheduledParam = new FileLittleFS(GLOBAL_SCHEDULED_PARAM_FILE_NAME);
  if (!FileLittleFS::exist(GLOBAL_SCHEDULED_PARAM_FILE_NAME) || force) {
    fileGlobalScheduledParam->writeFile(DEFAUT_GLOBAL_SCHEDULED_PARAM, "w");
  }
  cGlobalScheduledParam = new SimpleParam(fileGlobalScheduledParam->readFile().c_str(), ":", MAX_ITEMS_GLOBAL_SCHEDULED_PARAM);
  fileGlobalScheduledParam->close();
#ifdef DEBUG_OUTPUT
  cGlobalScheduledParam->print();
#endif
  return fileGlobalScheduledParam;
}

void initRotaryEncoder() {
  rotaryEncoder.begin();
  rotaryEncoder.setup(readEncoderISR);
}

void initTime() {
  // Mise à jour de l'heure
  static long gmtOffset_sec = 0, daylightOffset_sec;
  rtc = new ESP32Time(cDlyParam->get(SUMMER_TIME) * 3600);
  configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org");
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    rtc->setTimeStruct(timeinfo);
  }
  // Test 02/01/2022 14:59:11
  // rtc.setTime(11, 59, 12, 2, 2, 2022);
}

const char* getDate() {
  static char date[80];
  sprintf(date, "%02d/%02d/%4d %02d:%02d:%02d",
    rtc->getDay(),
    rtc->getMonth() + 1,
    rtc->getYear(),
    rtc->getHour(true), // true -> format 24H
    rtc->getMinute(),
    rtc->getSecond());
  return date;
}

void initGpio() {
#ifdef ES32A08
  pinMode(POWER_LED, OUTPUT);
  digitalWrite(POWER_LED, LOW);

  pinMode(CLK_165, OUTPUT);
  digitalWrite(CLK_165, LOW);
  pinMode(LOAD_165, OUTPUT);
  digitalWrite(LOAD_165, HIGH);
  pinMode(DATA_165, INPUT);

  pinMode(CLOCK_955, OUTPUT);
  pinMode(LATCH_955, OUTPUT);
  digitalWrite(NOE_955, 1);
  pinMode(NOE_955, OUTPUT);

  // Mettre les registres à zéro  
  clear74HC595();
  digitalWrite(NOE_955, 0);
  pinMode(DATA_955, OUTPUT);

#else  
  pinMode(O_TRANSFO, OUTPUT);
  digitalWrite(O_TRANSFO, HIGH);

  pinMode(O_FOUR, OUTPUT);
  digitalWrite(O_FOUR, HIGH);

  pinMode(O_VMC, OUTPUT);
  digitalWrite(O_VMC, HIGH);

  pinMode(O_PAC, OUTPUT);
  digitalWrite(O_PAC, HIGH);

  pinMode(O_EV_EST, OUTPUT);
  digitalWrite(O_EV_EST, HIGH);

  pinMode(O_POMPE, OUTPUT);
  digitalWrite(O_POMPE, HIGH);

  pinMode(O_EV_ARROSAGE, OUTPUT);
  digitalWrite(O_EV_ARROSAGE, HIGH);

  pinMode(O_EV_IRRIGATION, OUTPUT);
  digitalWrite(O_EV_IRRIGATION, HIGH);

  pinMode(I_ARROSAGE, INPUT_PULLUP);
  pinMode(I_SURPRESSEUR, INPUT_PULLUP);
  pinMode(I_IRRIGATION, INPUT_PULLUP);
  pinMode(I_LCD_CMD, INPUT_PULLUP);
  pinMode(I_REARM, INPUT);

  // Programmer les broches non utilisées en sortie
  // pour diminuer la sensibilité aux parasites EM

  pinMode(NC_32, OUTPUT);
  pinMode(NC_33, OUTPUT);
  pinMode(NC_25, OUTPUT);
  pinMode(NC_00, OUTPUT);
  pinMode(NC_02, OUTPUT);
  pinMode(NC_15, OUTPUT);
  // 
  digitalWrite(NC_32, LOW);
  digitalWrite(NC_33, LOW);
  digitalWrite(NC_25, LOW);
  digitalWrite(NC_00, LOW);
  digitalWrite(NC_02, LOW);
  digitalWrite(NC_15, LOW);
#endif
}

void logsWrite(const char* log) {
  char buffer[80];
  if (fileLogs->size() > MAX_LOG_SIZE)
    fileLogs->writeFile("", "w");
  sprintf(buffer, "%s - %s\n", getDate(), log);
  fileLogs->writeFile(buffer, "a");
}

void writeLogs(const char* log) {
  if (cDlyParam->get(LOG_STATUS))
    logsWrite(log);
}

#ifdef WIFI_MANAGER
void initWifiStation() {
  WiFiManager wm;
  // bool res;
  Serial.begin(115200);
  delay(500);
  Serial.println();

  WiFi.softAP(SSID_AP, PASSWD);

  if (!wm.autoConnect(SSID_AP, PASSWD))
    Serial.println("Pas de connexion");
  else
    Serial.println("Connexion établie");
  WiFi.enableAP(false);
  WiFi.setHostname(HOSTNAME);
  // WiFi.setAutoReconnect(true);
  // WiFi.persistent(true);
  initOTA();

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Code source exemple wifi manager
  // WiFiManager, Local intialization. Once its business is done, there is no need to keep it aroun
  //* WiFiManager wm;
  // bool res;
  // WiFi.mode(WIFI_MODE_NULL);
  // WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
  //* WiFi.mode(WIFI_STA);
  // explicitly set mode, esp defaults to STA+AP
  // reset settings - wipe stored credentials for testing
  // these are stored by the esp library
  // wm.resetSettings();

  // Automatically connect using saved credentials,
  // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
  // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
  // then goes into a blocking loop awaiting configuration and will return success result

  // res = wm.autoConnect(); // auto generated AP name from chipid
  // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
  //* res = wm.autoConnect(ssidAp, password); // password protected ap
  //* wm.setHostname(HOSTNAME);

  //* if (!res) {
  // Serial.println("Failed to connect");
  // ESP.restart();
  //* }

  // WiFi.setHostname(HOSTNAME);
  // WiFi.setAutoReconnect(true);
  // WiFi.persistent(true);
  // ArduinoOTA.begin();
  // Serial.begin(115200);
  // Serial.print("IP address: ");
  // Serial.println(WiFi.localIP());
  // WiFi.enableAP(false);
}
#else
void initWifiStation() {
  char buffer[21];
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.printf("Wifi %s not connected! Rebooting...", ssid);
#ifndef IO_DEBUG    
    backLightOff();
#endif    
    delay(5000);
    ESP.restart();
  }
  WiFi.setHostname(hostname);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  initOTA();
  sprintf(buffer, "SSID : %s", WiFi.SSID().c_str());
#ifndef IO_DEBUG    
  lcdPrintString(buffer, 1, 0, true);
#endif
  Serial.println(buffer);
  sprintf(buffer, "IP : %s", WiFi.localIP().toString().c_str());
  Serial.println(buffer);
#ifndef IO_DEBUG    
  lcdPrintString(buffer, 2, 0, true);
#endif  
}
#endif

void initMQTTClient() {
  // Connecting to MQTT server
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setBufferSize(MQTT_MAX_BUFFER_SIZE);
  mqttClient.setCallback(PubSubCallback);

  while (!mqttClient.connected()) {
    Serial.println(String("Connecting to MQTT (") + mqttServer + ")...");
    // Pour un même courtier les clients doivent avoir un id différent
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Serial.println(clientId);
    if (mqttClient.connect(clientId.c_str(), mqttUser, mqttPassword)) {
      Serial.println("MQTT client connected");
    }
    else {
      Serial.print("\nFailed with state ");
      Serial.println(mqttClient.state());
      if (WiFi.status() != WL_CONNECTED) {
        initWifiStation();
      }
      delay(5000);
    }
  }
  mqttConnect = true;
  // Déclare Pub/Sub topics
  mqttClient.subscribe(TOPIC_GET_PARAM);
  mqttClient.subscribe(TOPIC_WRITE_PARAM);
  mqttClient.subscribe(TOPIC_GET_DLY_PARAM);
  mqttClient.subscribe(TOPIC_WRITE_DLY_PARAM);
  mqttClient.subscribe(TOPIC_GET_GPIO);

  mqttClient.subscribe(TOPIC_CMD_ARROSAGE);
  mqttClient.subscribe(TOPIC_CMD_IRRIGATION);
  mqttClient.subscribe(TOPIC_CMD_CUISINE);
  mqttClient.subscribe(TOPIC_CMD_VMC);
  mqttClient.subscribe(TOPIC_CMD_VANNE_EST);
  mqttClient.subscribe(TOPIC_CMD_PAC);
  mqttClient.subscribe(TOPIC_CMD_REAMORCER);

  mqttClient.subscribe(TOPIC_LOGS_GET);
  mqttClient.subscribe(TOPIC_CLEAR_LOGS);
  mqttClient.subscribe(TOPIC_REBOOT);
  mqttClient.subscribe(TOPIC_GET_VERSION);
  mqttClient.subscribe(TOPIC_WATCH_DOG_OFF);

  mqttClient.subscribe(TOPIC_GET_GLOBAL_SCHED);
  mqttClient.subscribe(TOPIC_WRITE_GLOBAL_SCHED);
  mqttClient.subscribe(TOPIC_MQTT_TEST);
}

void setup() {
  char buffer[24];
  // Desactiver toutes les actions tant que pas initialisé
  onRotary = nullFunc;
  onDoubleClick = nullFunc;
  onLoopTic = nullFunc;
  onLoopTic2 = nullFunc;
  display = nullFunc;
  
  Serial.begin(115200);
  // delay(100);
  initGpio();
#ifdef IO_TEST
  Serial.println("Esp start");
  int val = O_VMC;
  for (int i=0; i < 8; i++) {
    on(val);
    val <<= 1;
    delay(1000);
  }
  for (int i=0; i < 8; i++) {
    val >>= 1;
    off(val);
    delay(1000);
  }
  for (;;) {
    Serial.printf("%04x\r", readByteInput());
    // Serial.print(readBitsInput(I_ARROSAGE));
    // Serial.print(readBitsInput(I_IRRIGATION));
    // Serial.print(readBitsInput(I_SURPRESSEUR));
    // Serial.print(digitalRead(GPIO_NUM_36));
    // Serial.println(analogRead(GPIO_NUM_36));
    // Serial.print(digitalRead(GPIO_NUM_32));
    // Serial.println(analogRead(GPIO_NUM_32));
    delay(500);
    Serial.print('\r');
  }
  return;
#endif
#ifndef FORCE_DISPLAY
  if (!gpioState(I_LCD_CMD))
    bootDisplayOff = true;
  #endif
  initDisplay();
  display = ioDisplay;

  sprintf(buffer, "HomeCtrl v%s", version.c_str());
  Serial.println(buffer);
  lcdPrintString(buffer, 0, 0, true);
  delay(500);
  fileLogs = new FileLittleFS(LOG_FILE_NAME);
  fileParam = initParam(FORCE_INIT_PARAM);
  fileDlyParam = initDlyParam(FORCE_INIT_DLY_PARAM);
  fileGlobalScheduledParam = initGlobalScheduledParam(FORCE_GLOBAL_SCHEDULED_PARAM);
  filePersistantParam = initPersitantFileDevice(FORCE_PERSISTANT_PARAM);
  initWifiStation();
  initMQTTClient();
  initTime();
  initRotary();
  lcdPrintString(getDate(), 3, 0, true);
  Serial.println(getDate());
  // Serial.println(__LINE__);
  // Serial.println(__FILENAME__);
  // Serial.println(__FILE__);

  delay(1000);
  logsWrite(bootRaison());
  // Serial.println(strlen(cParam->getStr()));
  // cParam->print();

#ifdef  WEB_SERIAL
  WebSerial.begin(&server);
  WebSerial.msgCallback(recvMsg);
  server.begin();
#endif

  //----------------------------------------- Monostables -------------------------------------------------------------
  // ****** Pas d'accès fichier dans un monostable (kernel panic) ******
  // Durée arrosage
  tache_t_watering = t_cree(monoWatering, cvrtic(cDlyParam->get(TIME_WATERING) * 1000));
  // Durée remplissage réservoir
  tache_t_tankFilling = t_cree(monoTankFilling, cvrtic(cDlyParam->get(TIME_TANK_FILLING) * 1000));
  // Durée max mise sous pression surpresseur
  tache_t_surpressorFilling = t_cree(monoSurpressorFilling, cvrtic(cDlyParam->get(TIME_SUPRESSOR) * 1000));
  // Durée ouverture electrovanne EST
  tache_t_cmdEvEst = t_cree(monoCmdEvEst, cvrtic(cDlyParam->get(EAST_VALVE_ON_TIME) * 1000));
  // Délais de coupure de la PAC après commande d'arrét PAC
  tache_t_monoPacOff = t_cree(monoPacOff, cvrtic(DLY_PAC_OFF) * 1000);
  // Délais d'envoi des commande vers la PAC après mise sous tension
  tache_t_monoPacOn = t_cree(monoPacOn, cvrtic(DLY_PAC_ON) * 1000);
  // Delai avant envoi d'une commande mqtt vers la carte VMC déportée 
  tache_t_cmdVmcBoard = t_cree(monoCmdVmcBoard, cvrtic(DLY_VMC_BOARD_ON) * 1000);
  // Durée affichage LCD local après modification d'une E/S
  tache_t_backLight = t_cree(monoCmdBackLight, cvrtic(DLY_BACK_LIGHT) * 1000);
  // Durée d'affichage local si porte armoire éléctrique est restée ouverte 
  tache_t_backLight2 = t_cree(monoCmdBackLight2, cvrtic(DLY_BACK_LIGHT2) * 1000);
  // Délai avant affichage par défaut
  tache_t_defaultDisplay = t_cree(monoDefaultDisplay, cvrtic(DLY_DEFAULT_SCREEN) * 1000);
  // Durée d'ouverture de l'electrovanne déportée d'irrigation (circuit d'arriosage des tomates)
  tache_t_offCircuit2 = t_cree(monoOffCircuit2, cvrtic(DLY_DEFAULT_OFF_CIRCUIT2) * 1000);
  // Bistable reglage débit, durée d'un pas de rapport cyclique
  tache_t_monoDebit = t_cree(monoDebit, cvrtic(PAS_PERIODE_DEBIT) * 1000, true);

  // t_start(tache_t_monoDebit);

  // Set watch dog timeout
#ifdef ENABLE_WATCHDOG
  esp_task_wdt_init(WDT_TIMEOUT, true); // enable panic so ESP32 restarts
  esp_task_wdt_add(NULL);               // add current thread to WDT watch
#endif
#ifdef DEBUG_OUTPUT
  print("End setup\n", OUTPUT_PRINT);
#endif
#ifdef PERSISTANT_PAC
  if (!cPersistantParam->get(PAC)) {
    off(O_PAC);
  }
  else {
    on(O_PAC);
    t_start(tache_t_monoPacOn);
  }
#endif  
#ifdef PERSISTANT_VMC
  setVmc(cPersistantParam->get(VMC));
#endif  
#ifdef PERSISTANT_POWER_COOK
  if (cPersistantParam->get(POWER_COOK))
    on(O_FOUR);
#endif  
#ifdef PERSISTANT_VANNE_EST
  if (cPersistantParam->get(VANNE_EST) {
    on(O_TRANSFO);
    on(O_EV_EST);
  }
#endif  

  // Persistance obligatoire pour ce paramètre
  // Utilisé par la vanne déportée arrosage des tomates
ItemParam item = cParam->get(IRRIGATION, 0);
joursCircuit2 = item.MMax;

long rssi = WiFi.RSSI();
sprintf(rssi_buffer, "RSSI:%ld", rssi);
// item.print();
// Test de mémorisation de la variable persistante jour
// item.MMax = 2;
// cParam->set(IRRIGATION, 0, item);
// cParam->updateStringParam(cParam->getStr());
// fileParam->writeFile(cParam->getStr(), "w");

// 
// Affichage de la taille de la pile disponible
// Serial.println(uxTaskGetStackHighWaterMark(NULL));
// _ioDisplay();
}

/*
 * Monostable mise en route carte VMC
 * Laisse la carte est alimentée par la mise sous
 * temsion de la VMC. Laisse le temps à l'esp01 le temps de démarrer
 * ACCES AUX FICHIERS NON AUTORISES
 */
 // Fermeture relai carte VMC après delai de mise sous tension
 // Force VMC à plein vitesse
void monoCmdVmcBoard(TimerHandle_t xTimer) {
#ifdef DEBUG_OUTPUT
  print("start board\n", OUTPUT_PRINT);
#endif
  t_stop(tache_t_cmdVmcBoard);
  if (vmcFast) 
    mqttClient.publish(VMC_BOARD_ACTION, S_ON);
  else
    mqttClient.publish(VMC_BOARD_ACTION, S_OFF);
}

/*
 * Monostable arret PAC
 *
 */
void monoPacOff(TimerHandle_t xTimer) {
#ifdef DEBUG_OUTPUT
  print("Mono Arret PAC\n", OUTPUT_PRINT);
#endif
  t_stop(tache_t_monoPacOff);
  off(O_PAC);
  irSendPacOff = false;
}

/*
 * Monostable mise en route de la PAC
 * La commande IR est envoyée après x secondes via
 * un dispositif déporté
 */
void monoPacOn(TimerHandle_t xTimer) {
  static int count;
#ifdef DEBUG_OUTPUT
  print("Envoi IR\n", OUTPUT_PRINT);
#endif
  // Serial.println("TOPIC_PAC_IR_ON");
  // Envoi de 4 commandes au cas ou l'IR est masquée 
  if (count++ >= 4) {
    t_stop(tache_t_monoPacOn);
    count = 0;
  }
  mqttClient.publish(TOPIC_PAC_IR_ON, "");
  mqttClient.publish(TOPIC_PAC_IR_PARAM_APPLY, "");
}

/*
 * Monostable arrêt arrosage après xx mn (règlable dans dlyParam)
 */
void monoWatering(TimerHandle_t xTimer) {
  stopWatering();
}

/*
 * Monostable arrêt remplissage réservoir (règlable dans dlyParam)
 */
void monoTankFilling(TimerHandle_t xTimer) {
  stopTankFilling();
}

/*
 * Monostable assurant l'arrêt pompe si problème sur le surpresseur (règlable dans dlyParam)
 */
void monoSurpressorFilling(TimerHandle_t xTimer) {
  t_stop(tache_t_surpressorFilling);
  // startSupressorFilling = false;
  off(O_POMPE);
  if (!startSupressorFilling2) {
    // Echec première tentative
    // Possibilité de reprise par bouton réarmement local ou à distance
    mqttClient.publish(TOPIC_DEFAUT_SUPRESSEUR, "on");
    erreurSupresseur = true;
#ifdef DEBUG_OUTPUT
    print("TOPIC_GPIO_DEFAUT_SUPRESSEUR  on\n", OUTPUT_PRINT);
#endif
  }
  else {
    // Echec de la deuxième tentative
    // Problème sur le circuit hydraulique pompe surpresseur
    // Pas de troisième tentative, résolution du problème
    // par intervention physique
    mqttClient.publish(TOPIC_DEFAUT_SUPRESSEUR, "on2");
    erreurPompe = true;
#ifdef DEBUG_OUTPUT
    print("TOPIC_GPIO_DEFAUT_SUPRESSEUR_N2  on\n", OUTPUT_PRINT);
#endif
  }
}

/*
 * Monostable assurant l'arrêt de l'EV placée sur le robinet EST (règlable dans dlyParam)
 * en cas de commande hors mode programmation horaire
 */
void monoCmdEvEst(TimerHandle_t xTimer) {
#ifdef DEBUG_OUTPUT
  print("monoCmdEvEst\n", OUTPUT_PRINT);
#endif
  t_stop(tache_t_cmdEvEst);
  off(O_TRANSFO);
  off(O_EV_EST);
}

/*
 * Monostable assurant l'arrêt eclairage LCD
 */
void monoCmdBackLight(TimerHandle_t xTimer) {
  if (!electricalPanelOpen)
    backLightOff();
  t_stop(tache_t_backLight);
}

/*
 * Monostable assurant l'arrêt eclairage LCD
 */
void monoCmdBackLight2(TimerHandle_t xTimer) {
  backLightOff();
  t_stop(tache_t_backLight);
}

/*
 * Monostable assurant le rétablissement de l'affichage par défaut
 */
void monoDefaultDisplay(TimerHandle_t xTimer) {
  _ioDisplay();
  t_stop(tache_t_defaultDisplay);
}

/*
 * Monostable assurant la coupure de l'electrovanne du circuit2 d'irrigation
 */
void monoOffCircuit2(TimerHandle_t xTimer) {
  mqttClient.publish(SUB_GPIO0_ACTION, "off");
  t_stop(tache_t_offCircuit2);
}

/*
 * Multistable de reglage du débit l'electrovanne sud
 * Période PAS_PERIODE_DEBIT (5s)
 */
void monoDebit(TimerHandle_t xTimer) {
  int cyclcalReport = cParam->get(VANNE_EST, 3).HMin;
  static int count;
  if (count < cyclcalReport) {
    on(O_EV_EST);
    // Serial.printf("cyclcalReport=%02d, count=%02d, ON \r", cyclcalReport, count);
  }
  else {
    off(O_EV_EST);
    // Serial.printf("cyclcalReport=%02d, count=%02d, OFF\r", cyclcalReport, count);
  }
  count = ++count % MAX_PAS_PERIODE_DEBIT;
} 

void onVanneEst() {
  on(O_TRANSFO);
  t_start(tache_t_monoDebit);
  // Logique inversée
  cmdVanneEst = 0;
}

void offVanneEst() {
  // Logique inversée
  cmdVanneEst = 1;
  off(O_TRANSFO);
  off(O_EV_EST);
  t_stop(tache_t_monoDebit);
}

//--------------------------------------------------------------------------------------------
/*
 * Exécuté toutes les minutes
 * Exécute les actions temporelles programmées
 * Appelé par loop (on ne peut pas utiliser les timers RTOS (accès aux fichiers))
 */
void schedule() {
  static boolean vmcBoardOn = false;
  static boolean vmcFastR = false;
  static boolean onVmcExec = false;
  static boolean flagJours = false;

  ItemParam item;

#ifdef DEBUG_HEAP
  Serial.printf("Free heap %x : min free heap %x\n", ESP.getFreeHeap(), ESP.getMinFreeHeap());
#endif
#ifndef TIME_SIMULATOR
  int h = rtc->getHour(true); // true -> format 24H
  int m = rtc->getMinute();
  // Mise à jour du jours courant (persistant)
  if (h == 0 && !flagJours) {
    flagJours = true;
    joursCircuit2++;
    item = cParam->get(IRRIGATION, 0);
    item.MMax = joursCircuit2;
    cParam->set(IRRIGATION, 0, item);
    cParam->updateStringParam(cParam->getStr());
    fileParam->writeFile(cParam->getStr(), "w");
  }
  if (h == 1 && flagJours) {
    flagJours = false;
  }
#else  
  if (m++ == 60) {
    m = 0;
    if (h++ == 24)
      h = 0;
  }
  // Serial.println(ESP.getFreeHeap());
  // Serial.println(ESP.getHeapSize());
  Serial.printf("%02d:%02d\r", h, m);
#endif

  for (int deviceId = 0; deviceId < N_DEVICES; deviceId++) {
    // Programmation autorisée pour ce device
    if (!cGlobalScheduledParam->get(deviceId)) {
      // Serial.printf("deviceId=%d\n", deviceId);
      continue;
    }
    for (int timeSet = 0; timeSet < N_PLAGES; timeSet++) {
      item = cParam->get(deviceId, timeSet);
      // -----------------------------------------------------------
      //---------------- Mises sous tension ------------------------
      // -----------------------------------------------------------
      if (item.enable != 0 && item.HMin == h && item.MMin == m) {
#ifdef DEBUG_OUTPUT_SCHEDULE_
        Serial.printf("%02d:%02d\n", h, m);
        item.print();
#endif
        switch (deviceId) {
        // Power cook ne comporte qu'une programmation d'arrêt, mise en route par commande manuelle
        case POWER_COOK:
          break;

        case IRRIGATION: {
#ifdef DEBUG_OUTPUT_SCHEDULE
            Serial.printf("%02d:%02d startTankFilling..\n", h, m);
#endif    
            startTankFilling();
            // Gestion de la vanne du circuit 2 
            // Circuit 2 est une vanne intallée sur le circuit d'arrosage des tomates
            // et permet un arrossage non journalier
            // La périodicité en jour est mémorisée le champ libre item.HMax
            ItemParam item = cParam->get(IRRIGATION, 0);
            // Serial.printf("itemJ.HMax=%d, jours=%d\n", itemJ.HMax, jours);
            // Gestion de la commande déportée de la vanne gérant la période d'irrigation
            // non journalière
            if (item.HMax >= joursCircuit2) {
              mqttClient.publish(SUB_GPIO0_ACTION, "on");
              // La coupure est programmée dans le dispositif distant
              // Reset du compte de jours
              joursCircuit2 = 0;
              // Mémorisation
              item.MMax = 0;
              cParam->set(IRRIGATION, 0, item);
              // itemJ.print();
              cParam->updateStringParam(cParam->getStr());
              // cParam->print();
              fileParam->writeFile(cParam->getStr(), "w");
            }
          }
          break;

        case VANNE_EST:
#ifdef DEBUG_OUTPUT_SCHEDULE
          Serial.printf("%02d:%02d on(O_EV_EST)\n", h, m);
#endif      
          // La durée d'ouverture est gérée par un monostable
          onVanneEst();
          break;

        case PAC:
          // Serial.printf("%02d:%02d off(O_PAC)\n", h, m);
          on(O_PAC);
#ifdef PERSISTANT_PAC       
          // Attention pas de timer
          cPersistantParam->set(PAC, 1);
          filePersistantParam->writeFile(cPersistantParam->getStr(), "w");
#endif           
          t_start(tache_t_monoPacOn);
          break;

        case VMC:
#ifdef DEBUG_OUTPUT_SCHEDULE
          Serial.printf("%02d:%02d, vmcMode=%d \n", h, m, vmcMode);
#endif
          onVmc = 1;
          if (item.enable == 2)
            onVmc = 2;
          switch (vmcMode) {
          case VMC_STOP:
          case VMC_ON_FAST:
          case VMC_ON: break;
          default:
            on(O_VMC);
            vmcMode = VMC_PROG_ON;
            if (onVmc == 2) {
              vmcFast = true;
              vmcMode = VMC_PROG_ON_FAST;
              t_start(tache_t_cmdVmcBoard);
            }
            break;
          }
          break;
        }
      }
      // -----------------------------------------------------------
      // --------------- Mises hors tension ------------------------
      // -----------------------------------------------------------
      if (item.enable != 0 && item.HMax == h && item.MMax == m) {
#ifdef DEBUG_OUTPUT_
        Serial.printf("%02d:%02d\n", h, m);
        item.print();
#endif
        switch (deviceId) {

        case POWER_COOK:
#ifdef DEBUG_OUTPUT_SCHEDULE
          Serial.printf("%02d:%02d off(O_FOUR)\n", h, m);
#endif
#ifdef PERSISTANT_POWER_COOK            
          cPersistantParam->set(POWER_COOK, 0);
          filePersistantParam->writeFile(cPersistantParam->getStr(), "w");
#endif
          off(O_FOUR);
          mqttClient.publish(TOPIC_STATUS_CUISINE, "off"); 
          break;
        // Remplissage du réservoir gérée par un monostable
        case IRRIGATION:
          break;

        case VANNE_EST:
#ifdef DEBUG_OUTPUT_SCHEDULE
          Serial.printf("%02d:%02d off(O_EV_EST)\n", h, m);
#endif
          offVanneEst();
          break;

        case PAC:
#ifdef DEBUG_OUTPUT_SCHEDULE
          Serial.printf("%02d:%02d PAC ON", h, m);
#endif
          mqttClient.publish(TOPIC_PAC_IR_OFF, "");
          // Les commandes IR TOPIC_PAC_IR_OFF sont publiées dans loop
          // isSendOn est mis à false dans monoPacOff
          irSendPacOff = true; 
          t_start(tache_t_monoPacOff);
#ifdef PERSISTANT_PAC            
          cPersistantParam->set(PAC, 0);
          filePersistantParam->writeFile(cPersistantParam->getStr(), "w");
#endif
          break;

        case VMC:
#ifdef DEBUG_OUTPUT_SCHEDULE
          Serial.printf("%02d:%02d off(O_VMC)\n", h, m);
#endif
          onVmc = 0;
          switch (vmcMode) {
          case VMC_STOP:
          case VMC_ON:
          case VMC_ON_FAST: break;
          default:
            off(O_VMC);
            vmcFast = false;
            vmcMode = VMC_PROG_OFF;
            break;
          }
          break;
        }
      }
    }
  }
}

//-----------------------------------
// Démarrage et arrêt arrosage 
//-----------------------------------
/**
 * @brief  Démarrage arrosage
 * @note   
 * @param  timeout: durée arrosage
 * @retval None
 */
void startWatering(int timeout) {
  // Serial.println("Start waterring");
  if (timeout) {
    setDelay(tache_t_watering, cvrtic(cDlyParam->get(TIME_WATERING) * 1000));
    t_start(tache_t_watering);
    wateringNoTimeOut = 0;
  }
  else
    wateringNoTimeOut = 2;
  isWatering = true;
  on(O_TRANSFO);
  on(O_EV_ARROSAGE);
  on(O_POMPE);
}
/**
 * @brief  Arrêt arrosage
 * @note   
 * @retval None
 */
void stopWatering() {
  t_stop(tache_t_watering);
  off(O_POMPE);
  off(O_TRANSFO);
  off(O_EV_ARROSAGE);
  isWatering = false;
  wateringNoTimeOut = 0;
}

//-----------------------------------
// Démarrage et arrêt remplissage réservoir
//-----------------------------------
/**
 * @brief  Démarrage remplissage réservoir
 * @note   
 * @retval None
 */
void startTankFilling() {
  setDelay(tache_t_tankFilling, cvrtic(cDlyParam->get(TIME_TANK_FILLING) * 1000));
  t_start(tache_t_tankFilling);
  isTankFilling = true;
  // Priorité au remplissage
  if (isWatering) {
    on(O_EV_IRRIGATION);
    off(O_EV_ARROSAGE);
    t_stop(tache_t_watering);
    isWatering = false;
  }
  else {
    on(O_TRANSFO);
    on(O_EV_IRRIGATION);
    on(O_POMPE);  }
}
/**
 *@brief Arrêt remplissage réservoir
 * 
 */
void stopTankFilling() {
  // Serial.println("stopTankFilling");
  t_stop(tache_t_tankFilling);
  off(O_POMPE);
  off(O_TRANSFO);
  off(O_EV_IRRIGATION);
  isTankFilling = false;
}

/**
 * @brief  Détection des fronts montant sur ports gpio
 * @note   
 * @param  nInput: numéro port E/S ou masque si carte ES32A08
 * @retval true si front montant
 */
boolean isEdge(int nInput) {
  static boolean state;
  static boolean state_N_1;
  state = gpioState(nInput);
  if (state != state_N_1) {
    state_N_1 = state;
    return state;
  }
  return false;
}

/**
 *@brief Commande de la VMC
 * 
 * @param cmd : CMD_VMC_OFF | CMD_VMC_PROG | CMD_VMC_ON_FAST | CMD_VMC_ON
 */
void setVmc(int cmd) {
  vmcMode = cmd;
#ifdef PERSISTANT_VMC
  cPersistantParam->set(VMC, cmd);
  filePersistantParam->writeFile(cPersistantParam->getStr(), "w");
#endif
  // Serial.printf("SetVmc : %d\n", cmd);
  switch (cmd) {
  case CMD_VMC_OFF:
    // Mode off (pas de vcm active, même ne mode programmée)
    off(O_VMC);
    vmcMode = VMC_STOP;
    break;
  case CMD_VMC_PROG:
    switch (onVmc) {
    case 0:
      off(O_VMC);
      vmcFast = false;
      vmcMode = VMC_PROG_OFF;
      break;
    case 1:
      on(O_VMC);
      vmcFast = false;
      vmcMode = VMC_PROG_ON;
      break;
    case 2:
      on(O_VMC);
      t_start(tache_t_cmdVmcBoard);
      vmcFast = true;
      vmcMode = VMC_PROG_ON_FAST;
      break;
    }
    break;
  case CMD_VMC_ON_FAST:
    // Mode forcé vmc vitesse rapide (programmation off)
    // Serial.printf("SetVmc CMD_VMC_ON_FAST: %d\n", CMD_VMC_ON_FAST);
    vmcFast = true;
    vmcMode = VMC_ON_FAST;
    on(O_VMC);
    t_start(tache_t_cmdVmcBoard);
    break;
  case CMD_VMC_ON:
    // Serial.printf("SetVmc CMD_VMC_ON: %d\n", CMD_VMC_ON);
    // Mode forcé vmc vitesse lente (programmation off)
    on(O_VMC);
    vmcFast = false;
    vmcMode = VMC_ON;
    t_start(tache_t_cmdVmcBoard);
    break;
  }
}

//-----------------------------------
// Boucle de scrutation
//-----------------------------------
void loop() {
  static long tps = 0;
  static long tpsIr = 0;
  static long tpsRot = 0;
  static long tpsRotary = 0;
  static long tpsProg = 0;
  static long tpsRotaryUpdt = 0;
  static long tpsSchedule = 0;
  static long tpsWifiTest = 0;
  static long tpsWDTReset = 0;
  static long tpsWifiSignalStreng = INTERVAL_WIFI_STRENG_SEND;
  static unsigned wifiTest = 0;
  static unsigned mqttConnectTest=0;
  static boolean esp_task_wdt = true;

#ifdef EXEC_TIME_MEASURE
  time_exec_start();
#endif
#ifdef IO_TEST
  ArduinoOTA.handle();
  return;
#endif   
  ArduinoOTA.handle();
  mqttClient.loop();
  
#ifdef ENABLE_WATCHDOG
  // Reset du chien de garde
  if (millis() - tpsWDTReset > INTERVAL_RESET_WDT) {
    esp_task_wdt_reset();
    tpsWDTReset = millis();
  }
#endif
  // Reset du chien de garde mqtt
  if (millis() - mqttConnectTest > INTERVAL_MQTT_CONNECT_TEST) {
    // Serial.println("INTERVAL_MQTT_CONNECT_TEST");
    if (!mqttConnect) {
      ESP.restart();
    }
    mqttConnectTest = millis();
    mqttConnect = false;
    mqttClient.publish(TOPIC_MQTT_TEST, "");
  }
 
//  if (millis() - tpsWifiTest > INTERVAL_WIFI_TEST) {
//    tpsWifiTest = millis();
    // Test conx WiFi  
    // while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    //   WiFi.reconnect();
    //   esp_task_wdt_delete(NULL);
    //   esp_task_wdt = false;
    //   delay(5000);
    //   if (wifiTest++ == 6) {
    //     logsWrite("WiFi deconnected");
    //     ESP.restart();
    //   }
    // }
    // if (!esp_task_wdt) {
    //   wifiTest = 0;
    //   esp_task_wdt_init(WDT_TIMEOUT, true); // enable panic so ESP32 restarts
    //   esp_task_wdt_add(NULL);               // add current thread to WDT watch
    //   esp_task_wdt = true;
    // }

    // if (!mqttClient.connected()) {
    //   initMQTTClient();
    // }
  //}

  // Scruter les ports E/S toutes les s pour afficher sur lcd
  // La mise à jour de l'affichage uniquemment sur nouvel état des
  // ports E/S
  static unsigned uPortIn_1 = 0xFFFFFFFF;
  static unsigned uPortOut_1 = 0xFFFFFFFF;

  if (millis() - tpsProg > INTERVAL_PORT_READ) {
    tpsProg = millis();

    static boolean bPac;
    if (irSendPacOff) {
      bPac = !bPac;
      lcdPrintChar((bPac) ? '0' : '1', 2, 4);
    }

    // Ne mettre à jour l'affichage que si changement
    unsigned uPortOut_0 = testPortIO_O();
    if (uPortOut_1 != uPortOut_0) {
      uPortOut_1 = uPortOut_0;
      display();
    }

    // Ne mettre à jour l'affichage que si changement
    unsigned uPortIn_0 = testPortIO_I();
    if (uPortIn_1 != uPortIn_0) {
      uPortIn_1 = uPortIn_0;
      display();
    }

  }

  // Scrutation évenements E/S toutes les 100 ms
  if (millis() - tps > INTERVAL_IO_SCRUT) {
    // static int i = 0;
    tps = millis();
    // Serial.printf("Loop %d\n", xPortGetCoreID());
    localLoop();
  }

  // Start rotate schedule
  // Mise à jour de l'indicateur tournant sur le lcd toute les 500 ms
  // isLcdDisplayOn = true;
  if (millis() - tpsRot > INTERVAL_ROTATE_DISPLAY && isLcdDisplayOn) {
    tpsRot = millis();
    static int rotate;
    lcdPrintChar(progress[rotate++ % 3], 0, 0);  // 2, 19
  }
  // Test appuy sur bouton rotary  
  if (millis() - tpsRotary > INTERVAL_ROTATARY_SCHEDULE) {
    tpsRotary = millis();
    rotary_loop();
    button.tick();
    // Pointeurs de fonctions mis à jour en fonction des actions à exécuter
    onLoopTic();
    onLoopTic2();
  }

  // Envoi toutes les 15s d'une commande IR sur la PAC
  if (irSendPacOff && (millis() - tpsIr > INTERVAL_IR_SEND)) {
    tpsIr = millis();
    mqttClient.publish(TOPIC_PAC_IR_OFF, "");
  }

  // Appel de schedule toutes les minutes
  // pour les actions programmées
  if (millis() - tpsSchedule > INTERVAL_SCHEDULE) {
    tpsSchedule = millis();
    schedule();
  }

  if (millis() - tpsWifiSignalStreng > INTERVAL_WIFI_STRENG_SEND) {
    tpsWifiSignalStreng = millis();
    long rssi = WiFi.RSSI();
    sprintf(rssi_buffer, "RSSI:%ld", rssi);
    mqttClient.publish(TOPIC_WIFI_STRENG, rssi_buffer);
    if (!isLcdDisplayOn)
      return;
    lcdPrintRssi(&rssi_buffer[5]);
    // Serial.println(&rssi_buffer[5]);
  }
  // Suspend pour 5s, ne pas utiliser ici
  // esp_sleep_enable_timer_wakeup(5000000); // 5 seconds
  // esp_light_sleep_start();
#ifdef EXEC_TIME_MEASURE
  time_exec_stop();
#endif
}

/**
 *@brief  Réception des messages MQTT
 *        Attention si le buffer MQTT est trop petit le message correspondant est
 *        supprimé. Voir commantaire en-tête de ce fichier
 * 
 * @param topic 
 * @param payload 
 * @param length 
 */
void PubSubCallback(char* topic, byte* payload, unsigned int length) {
  String strPayload = "";
  // boolean cmdVanneEst;
#ifdef DEBUG_OUTPUT
  char buffer[80];
#endif
  for (unsigned int i = 0; i < length; i++) {
    strPayload += (char)payload[i];
  }
#ifdef DEBUG_OUTPUT_
  Serial.print(topic);
  Serial.println(strPayload.c_str());
#endif

  //------------------  TOPIC_GET_PARAM ----------------------
  if (cmp(topic, TOPIC_GET_PARAM)) {
#ifdef DEBUG_OUTPUT
    // print(cParam->getStr(), OUTPUT_PRINT);
#endif
    // Modifier la chaine param pour refléter la valeur de jours courant
    ItemParam item = cParam->get(IRRIGATION, 0);
    item.MMax = joursCircuit2;
    cParam->set(IRRIGATION, 0, item);
    cParam->updateStringParam(cParam->getStr());
    // cParam->print();
    mqttClient.publish(TOPIC_PARAM, cParam->getStr());
    if (erreurSupresseurEvent) {
      erreurSupresseurEvent = false;
      mqttClient.publish(TOPIC_DEFAUT_SUPRESSEUR, "on");
    }
    if (erreurPompEvent) {
      erreurPompEvent = false;
      mqttClient.publish(TOPIC_DEFAUT_SUPRESSEUR, "on2");
    }
    return;
  }
  //------------------  TOPIC_WRITE_PARAM ----------------------
  // Attention taille du message important 
  if (cmp(topic, TOPIC_WRITE_PARAM)) {
    cParam->setStr(strPayload.c_str());
#ifdef DEBUG_OUTPUT
    print(cParam->getStr(), OUTPUT_PRINT);
#endif
    fileParam->writeFile(strPayload.c_str(), "w");
    return;
  }
  //------------------  TOPIC_GET_DLY_PARAM ----------------------
  if (cmp(topic, TOPIC_GET_DLY_PARAM)) {
    // cDlyParam->print();
    mqttClient.publish(TOPIC_DLY_PARAM, cDlyParam->getStr());
    return;
  }
  //------------------  TOPIC_WRITE_DLY_PARAM ----------------------
  if (cmp(topic, TOPIC_WRITE_DLY_PARAM)) {
    cDlyParam->setStr(strPayload.c_str());
#ifdef DEBUG_OUTPUT
    print(cDlyParam->getStr(), OUTPUT_PRINT);
#endif
    // cDlyParam->print();
    fileDlyParam->writeFile(strPayload.c_str(), "w");
    return;
  }
  //------------------  TOPIC_GET_GPIO ----------------------
  if (cmp(topic, TOPIC_GET_GPIO)) {
    static char tabValue[24];
    char str[4];
    // unsigned arrosage = itoa(digital_read(O_EV_ARROSAGE) + wateringNoTimeOut);
    // gpioRead(O_EV_ARROSAGE);
    sprintf(tabValue, "%s;%s;%s;%u;%d;%s",
      itoa(digital_read(O_EV_ARROSAGE) + wateringNoTimeOut, str, 10),
      gpioRead(O_EV_IRRIGATION),
      gpioRead(O_FOUR),
      cmdVanneEst,
      // gpioRead(O_EV_EST),
      vmcMode,
      gpioReadPac());
    mqttClient.publish(TOPIC_GPIO, tabValue);
    return;
  }
  //------------------  TOPIC_CMD_ARROSAGE ----------------------
  if (cmp(topic, TOPIC_CMD_ARROSAGE)) {
    unsigned cmd = atoi(strPayload.c_str());
    switch (cmd) {
    case 0:
      // Serial.println("TOPIC_CMD_ARROSAGE Stop watering");
      stopWatering();
      wateringNoTimeOut = 0;
      break;
    case 1:
      // Serial.println("Watering timeout");
      startWatering(TIMEOUT);
      wateringNoTimeOut = 0;
      break;
    case 2:
      // Serial.println("Watering no timeout");
      startWatering(NO_TIMEOUT);
      wateringNoTimeOut = 2;
      break;
    }
    return;
  }
  //------------------  TOPIC_CMD_IRRIGATION ----------------------
  if (cmp(topic, TOPIC_CMD_IRRIGATION)) {
    unsigned cmd = atoi(strPayload.c_str());
    if (cmd)
      startTankFilling();
    else
      stopTankFilling();
    return;
  }
  //------------------  TOPIC_CMD_CUISINE ----------------------
  if (cmp(topic, TOPIC_CMD_CUISINE)) {
    unsigned cmd = atoi(strPayload.c_str());
#ifdef PERSISTANT_POWER_COOK   
    cPersistantParam->set(POWER_COOK, cmd);
    filePersistantParam->writeFile(cPersistantParam->getStr(), "w");
#endif  
    if (cmd==1) {
      on(O_FOUR);
      mqttClient.publish(TOPIC_STATUS_CUISINE, "on");      
    }

    else  if (cmd==0){
      off(O_FOUR);
      mqttClient.publish(TOPIC_STATUS_CUISINE, "off");   
    }
    return;
  }
  //------------------  TOPIC_CMD_VMC ----------------------
  if (cmp(topic, TOPIC_CMD_VMC)) {
    unsigned cmd = atoi(strPayload.c_str());
    setVmc(cmd);
    return;
  }
  //------------------  TOPIC_CMD_VANNE_EST ----------------------
  if (cmp(topic, TOPIC_CMD_VANNE_EST)) {
    unsigned cmd = atoi(strPayload.c_str());
    if (cmd) {
      on(O_TRANSFO);
      on(O_EV_EST);
      setDelay(tache_t_cmdEvEst, cvrtic(cDlyParam->get(EAST_VALVE_ON_TIME) * 1000));
      t_start(tache_t_cmdEvEst);
      onVanneEst();
    }
    else {
      // off(O_TRANSFO);
      // off(O_EV_EST);
      t_stop(tache_t_cmdEvEst);
      offVanneEst();
    }
    return;
#ifdef DEBUG_OUTPUT
    sprintf(buffer, "topic : %s, port %s\n", topic, strPayload.c_str());
    print(buffer, OUTPUT_PRINT);
#endif
    // cmdVanneEst = cmp(strPayload.c_str(), "1");
    return;
  }
  //------------------  TOPIC_CMD_PAC ----------------------
  if (cmp(topic, TOPIC_CMD_PAC)) {
    unsigned cmd = atoi(strPayload.c_str());
    // Logique inversée pour relai PAC
    // 1 = arret
    if (cmd) {
      // Arreter la PAC via IR
      // Serial.println("Arreter la PAC via IR");
      mqttClient.publish(TOPIC_PAC_IR_OFF, "");
      // Couper alim PAC après DLY_OFF secondes
      t_start(tache_t_monoPacOff); // Logique inversée pour relai PAC
      t_stop(tache_t_monoPacOn);
      irSendPacOff = true;
#ifdef PERSISTANT_PAC
      cPersistantParam->set(PAC, 0);
#endif
    }
    else {
      // mettre la PAC sous tension
      irSendPacOff = false;
      on(O_PAC);
#ifdef PERSISTANT_PAC
      cPersistantParam->set(PAC, 1);
#endif
      t_start(tache_t_monoPacOn);
      t_stop(tache_t_monoPacOff);
    }
#ifdef PERSISTANT_PAC
    filePersistantParam->writeFile(cPersistantParam->getStr(), "w");
#endif
    return;
  }
  //---------------  TOPIC_CMD_REAMORCER  -------------------
  if (cmp(topic, TOPIC_CMD_REAMORCER)) {
    #ifdef DEBUG_OUTPUT
    Serial.println(TOPIC_CMD_REAMORCER);
    #endif
    msgRearm = true;
    return;
  }
  //------------------  TOPIC_LOGS_GET ----------------------
  if (cmp(topic, TOPIC_LOGS_GET)) {
    if (!fileLogs->exist())
      return;
    String logs = fileLogs->readFile();
    // Serial.println(logs);
    unsigned i = 0;
    // La taille du message limitéel'implémentation de MQTT
    // pour les ESP, il faut fractionner le message lignes/lignes
    while (i < logs.length()) {
      String partial = logs.substring(i, logs.indexOf("\n", i) + 1);
      mqttClient.publish(TOPIC_READ_LOGS, partial.c_str());
      // Serial.print(partial);
      i = logs.indexOf("\n", i) + 1;
    }
    // Message de fin
    mqttClient.publish(TOPIC_READ_LOGS, "#####");
    return;
  }
  //------------------  TOPIC_CLEAR_LOGS ----------------------
  if (cmp(topic, TOPIC_CLEAR_LOGS)) {
    fileLogs->writeFile("Log cleared\n", "w");
    return;
  }
  //------------------  TOPIC_REBOOT ----------------------
  if (cmp(topic, TOPIC_REBOOT)) {
    ESP.restart();
    return;
  }
  //------------------  TOPIC_GET_VERSION ----------------------
  if (cmp(topic, TOPIC_GET_VERSION)) {
    long rssi = WiFi.RSSI();
    sprintf(rssi_buffer, "RSSI:%ld", rssi);
    String info = version + "\n" + WiFi.localIP().toString() + ", " + rssi_buffer + "db\n" + String(getDate());
    // Serial.println(info);
    mqttClient.publish(TOPIC_READ_VERSION, info.c_str());
    return;
  }
  //------------------  TOPIC_WATCH_DOG_OFF ----------------------
  if (cmp(topic, TOPIC_WATCH_DOG_OFF)) {
    esp_task_wdt_delete(NULL);
    return;
  }
  //------------------  TOPIC_GLOBAL_SCHED_GET ----------------------
  if (cmp(topic, TOPIC_GET_GLOBAL_SCHED)) {
    mqttClient.publish(TOPIC_GLOBAL_SCHED, cGlobalScheduledParam->getStr());
    return;
  }
  //------------------  TOPIC_GLOBAL_SCHED_WRITE ----------------------
  if (cmp(topic, TOPIC_WRITE_GLOBAL_SCHED)) {
    // Mettre à jour l'objet
    cGlobalScheduledParam->setStr(strPayload.c_str());
#ifdef DEBUG_OUTPUT
    print(cGlobalScheduledParam->getStr(), OUTPUT_PRINT);
#endif
    fileGlobalScheduledParam->writeFile(strPayload.c_str(), "w");
    return;
  }
  //------------------  TOPIC_TEST_RESULT ----------------------
  if (cmp(topic, TOPIC_MQTT_TEST)) {
    mqttConnect = true;   
    // Serial.println("INTERVAL_MQTT_CONNECT_TEST_OK"); 
    return;
  }
}
