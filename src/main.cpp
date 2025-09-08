/**
 * @file main.cpp
 * @brief Automate ESP32 remplaçant le ZELIO
 * 
 * Ce fichier contient le code pour un automate basé sur ESP32 qui remplace un ZELIO.
 * 
 * @details
 * IMPORTANT: Si la taille des messages dépasse 256 octets, modifier dans la librairie PubSubClient.h
 * La chaine param fait actuellement 279 octets. MQTT_MAX_PACKET_SIZE = 300 est insuffisant !!
 * #define MQTT_MAX_PACKET_SIZE 256 par #define MQTT_MAX_PACKET_SIZE 512 ou appeler setBufferSize(MQTT_MAX_BUFFER_SIZE);
 * Solution adoptée dans initMQTTClient().
 * 
 * Définir CONFIG_ARDUINO_LOOP_STACK_SIZE 8192 dans .platformio\packages\framework-arduinoespressif32\tools\sdk\esp32\dio_qspi\include
 * Voir https://community.platformio.org/t/esp32-stack-configuration-reloaded/20994/2
 * 
 * @version 31/3/23
 * - Encapsulation des variables chaines globales de paramétrage
 * - Unification des classes de paramétrage Dlyparam, Persistant et GlobalScheduledParam dans SimpleParam
 * - Augmentation de la taille de la pile
 * - Unification de l'envoi des logs (nouveau fichier logs.cpp)
 * - Logs sur perte de WiFi
 * 
 * @version 2.6.5 3/9/23
 * - Configuration dynamique des monostables arrosage, remplissage réservoir, irrigation vanne est
 * 
 * @version 2.6.6 11/9/23
 * - Correction mineure dans les messages de loopProg
 * 
 * @version 2.7.0 27/10/23
 * - Utilisation de la librairie LittleFS Intégrée au noyau
 * 
 * @version 2.8.0 30/10/23
 * - Réduction de l'allumage lcd lors des reboot
 * - Timeout de l'allumage lcd lorsque le panneau electrique est ouvert
 * 
 * @version 2.9.0 30/11/23
 * - Remplacement contacteur PAC ferme au repos par contacteur ouvert au repos
 * - Inversion de la logique de commande de la PAC
 * - Correction d'un bug bloquant la maj de l'affichage local (variable tpsProg mise à jour dans loop test wifi)
 * 
 * @version 2.9.1 08/1/24
 * - Force l'affichage par definition du symbole FORCE_DISPLAY (necessite une recompilation)
 * 
 * @version 2.9.2 28/3/24
 * - INTERVAL_RESET_WDT 100 -> 500
 * - INTERVAL_WIFI_TEST 60*1000 -> 3*60*1000
 * 
 * @version 2.10.0 12/5/24
 * - Ajout du reglage de débit vanne est par reglage du rapport cyclique on/off
 * 
 * @version 3.0.0 9/6/24
 * - Génération d'une version pour carte ES32A08 en definissant ES32A08 dans const.h
 * - Regroupement de toutes les E/S gpio dans io.cpp
 * 
 * @version 2024.06.22
 * - Changement numérotation version (aaa.mm.jj)
 * - Affichage rssi sur l'écran lcd et envoi de message mqtt homecontrol/wifi_streng
 * 
 * @version 2024.06.25
 * - Possibilité de bloquer le remplissage du supresseur
 * 
 * @version 2024.06.25
 * - Test périodique d'envoi de messages mqtt. En cas de non réponse reboot
 * 
 * @version 2024.09.19
 * - Correction bug : commande PAC depuis la console, commande arrosage et irrigation avec la télécommande
 * 
 * @version 2024.09.20
 * - Regroupement des messages lcd
 * - Correction de bugs mineurs
 * 
 * @version 2024.10.15
 * - Envoi de message on/off homecontrol/status_cuisine lors des commandes
 * 
 * @version 2024.12.31
 * - Modification monostable commande vmc
 * 
 * @version 2025.01.04
 * - Optimisation de la fonction schedule
 * 
 * @version 2025.01.07
 * - Optimisation de l'affichage des capteurs et actionneurs
 * 
 * @version 2025.01.09
 * - Signale séquence d'arrêt de la PAC sur l'écran LCD (K2 ->0->1->0..)
 * 
 * @version 2025.01.11
 * - Persistance commande EV EST, suppression mémorisation (persistance) commande PAC dans loop_prog
 * - Optimisations diverses
 * 
 * @version 2025.01.13
 * - Optimisation gestion surpresseur, modification messages supresseur MQTT
 * 
 * @version 2025.01.18
 * - Correction bug mise à jour logs supresseur
 * 
 * @version 2025.01.18.1
 * - Mise à jour de la documentation
 *  
 * @version 2025.02.07
 * - Ajout d'un destructeur pour les objets simpleParam
 *  
 * @version 2025.02.07.1
 * - Mise à jour de la documentation
 * 
 * @version 2025.02.11
 * - Modification de SinglePParam:split (init _motif)
 * - Modification allocation dynamique (malloc -> new)
 * 
 * @version 2025.02.17
 * - Envoi message TOPIC_DEFAUT_SUPRESSEUR off vers HA
 *   à l'alimentation du surpresseur 
 *
 * @version 2025.03.30
 * - Serveillance mise en route trop fréquente surpresseur
 *   (détection rupture canalisation, fuite...)
 *    Envoi message TOPIC_SUPRESSEUR_SECURITY on vers HA
 * 
 * @version 2025.06.13
 * - Supression du fichier log si > MAX_LOG_SIZE 
 * 
 * @version 2025.06.15
 * - Modification enregistrement log pour irrigation façade sud
 *  
 * @version 2025.07.07
 * - Autorise un fonctionnement hors ligne si coupure réseau 
 * - Correction bug dans fileSize
 * 
 * @version 2025.07.15
 * - Ajustement timeout mqtt
 * 
 *  @version 2025.07.16
 * - Power cooking ON/OFF programming
 * 
 *  @version 2025.07.23
 * - Set default parameter main.h : PARAM 
 * - Create printStrParam in class Param
 * 
 *  @version 2025.07.28
 * - reconnexion auto WiFi et Mqtt
 * 
 *  @version 2025.07.29
 * - Suppress serial message on reconnect
 * 
 *  @version 2025.09.04
 * - Ajout commande alexa pour les appareils electro ménager
 *   Modifier dans la librairie Estalexa.cpp fonction
 *   EspalexaDevice::EspalexaDevice(String deviceName, BrightnessCallbackFunction gnCallback, uint8_t initialValue) { //constructor for dimmable device
 *   _type = EspalexaDeviceType::onoff;
 * 
 *  - Mettre en commentaire la ligne 638 du fichier 
 *    C:\Users\xxx\.platformio\packages\framework-arduinoespressif32\libraries\WebServer\src\WebServer.cpp
 *    (fonction void WebServer::_handleRequest()) 
 *    // log_e("request handler not found");
 *
 *  @version 2025.09.06
 *  - Ajout commande alexa pour les appareils electro ménager librairie	vintlabs/FauxmoESP @ ^3.4.1
 *   Supression librairie esphome/AsyncTCP-esphome @ ^2.0.0 remplacement par esp32async/AsyncTCP@^3.4.7
 *   Ajout des devices Alexa: Vmc, Fats Vmc, Vmc Prog, Arrosage
 *   Rétablisement à l'original de void WebServer::_handleRequest()) dans
 *   C:\Users\xxx\.platformio\packages\framework-arduinoespressif32\libraries\WebServer\src\WebServer.cpp
 *
 *  @version 2025.09.07
 *  - Optimisation des appels aux fonction Alexa
 *
 *  @version 2025.09.10
 *  - Fusion des commandes vmc : 
 *     vmc 10%  -> vmc programmée
 *     vmc 50%  -> vmc ventilation faible
 *     vmc 100% -> vmc ventilation max 
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

/**
 *@brief 
 * Delai non bloquant
 * @param ms 
 */
void dly(unsigned long ms) {
  uint32_t start = micros();

  while (ms > 0) {
    yield();
    while (ms > 0 && (micros() - start) >= 1000) {
      ms--;
      start += 1000;
    }
  }
}

void printMqttDebugValue(const char* value) {
  mqttClient.publish(TOPIC_DEBUG_VALUE, value);
}

// void publish(const char* topic, const char* payload) {
//   mqttClient.publish(topic, payload);
// }

// void gpioOff(int adr) {
//   off(adr);
// }
// void gpioOn(int adr) {
//   on(adr);
// }

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

/**
 * @brief Récupère les paramètres des actions programmées et mémorisés 
 *        dans la mémoire flash.
 *        Instancie cParam de classe Param.
 * @param force : initialise avec les valeurs par défaut.
 * @return FileLittleFS* pointeur vers les fonctions fichiers
 *         de manipulation des paramètres 
 */
FileLittleFS* initParam(boolean force) {
  auto* fileParam = new FileLittleFS(PARAM_FILE_NAME);
  if (!FileLittleFS::exist(PARAM_FILE_NAME) || force) {
    fileParam->writeFile(PARAM, "w");
  } 
  cParam = new Param(fileParam->readFile().c_str());
  fileParam->close();
#ifdef DEBUG_OUTPUT
//  cParam->print();
//  cParam->printStrParam();
#endif
  return fileParam;
}
/**
 * @brief Récupère les paramètres de temposisation mémorisés
 *        dans la mémoire flash.
 *        Instancie cDlyParam de classe SimpleParam ces paramètres.
 * @param force : initialise avec les valeurs par défaut
 * @return FileLittleFS* pointeur vers les fonctions fichiers de manipulation
 *         des tempos
 */
FileLittleFS* initDlyParam(boolean force) {
  auto* fileDlyParam = new FileLittleFS(DLY_PARAM_FILE_NAME);
  if (!FileLittleFS::exist(DLY_PARAM_FILE_NAME) || force) {
    fileDlyParam->writeFile(DEFAUT_DELAY_PARAM, "w");
  }
  cDlyParam = new SimpleParam(fileDlyParam->readFile().c_str(), ":", N_DLY_PARAM);
  fileDlyParam->close();
  // cDlyParam->print();
#ifdef DEBUG_OUTPUT
  cDlyParam->print();
#endif
  return fileDlyParam;
}
/**
 * @brief Récupère les paramètres de persistance mémorisés 
 *        dans la mémoire flash.
 *        Instancie cPersistantParam de classe SimpleParam ces paramètres.
 * @param force : initialise avec les valeurs par défaut
 * @return FileLittleFS* pointeur vers les fonctions fichiers de manipulation
 *         des éléments de persistance.
 */
FileLittleFS* initPersitantFileDevice(boolean force) {
  auto* filePersistantParam = new FileLittleFS(FILE_PERSISTANT_DEVICE);
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
/**
 * @brief Récupère les paramètres de d'autorisation de programmation mémorisés 
 *        dans la mémoire flash.
 *        Instancie cGlobalScheduledParam de classe SimpleParam ces paramètres.
 * @param force : initialise avec les valeurs par défaut
 * @return FileLittleFS* pointeur vers les fonctions fichiers de manipulation
 *         des éléments de persistance.
 */
FileLittleFS* initGlobalScheduledParam(boolean force) {
  auto* fileGlobalScheduledParam = new FileLittleFS(GLOBAL_SCHEDULED_PARAM_FILE_NAME);
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

/**
 * @brief Instancie initDateParam
 * @param force : initialise avec les valeurs par défaut
 * @return fileDateParam* pointeur vers fichiers ou est stocké la date lors du dernier arret.
 */
// FileLittleFS* initDateParam(boolean force) {
//   auto* fileDateParam = new FileLittleFS(FILE_DATE);
//   if (!FileLittleFS::exist(FILE_DATE) || force) {
//     fileDateParam->writeFile(DEFAULT_DATE, "w");
//     fileDateParam->close();
//   }
//   //strcpy(date, fileDateParam->readFile().c_str());  
//   return fileDateParam;
// }

/**
 * @brief Instancie fileLogs.
 * @param force : initialise avec les valeurs par défaut
 * @return fileLogs*
 */
FileLittleFS* initLogs(boolean force) {
  auto* fileLogs = new FileLittleFS(LOG_FILE_NAME);
  if (!FileLittleFS::exist(LOG_FILE_NAME) || force) {
    fileLogs->writeFile(DEFAULT_DATE, "w");
    fileLogs->close();
  }
  return fileLogs;
}

/**
 * @brief 
 *  La fonction initTime() initialise l'heure en utilisant soit l'heure NTP
 *  si une connexion Wi-Fi est disponible, soit la date stockée dans un fichier
 *  en cas d'absence de connexion. Elle configure une instance de ESP32Time 
 *  avec les paramètres d'heure d'été et met à jour l'horloge interne en fonction
 *  des données obtenues.
 */
void initTime(boolean wifiConnected) {
  static long gmtOffset_sec = 0, daylightOffset_sec;
  rtc = new ESP32Time(cDlyParam->get(SUMMER_TIME) * 3600);
  if (wifiConnected) {
    configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org");
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      rtc->setTimeStruct(timeinfo);    
    }
    // strcpy(date, getDate());
    // fileDateParam->writeFile(date, "w");
    // fileDateParam->close();
  }
  // Le module RTC conserve l'heure après reboot 
  // else {
    // Serial.println(rtc->getDateTime());
    // Si pas de connexion wifi, initialiser avec la date du fichier
    //  strcpy(date, fileDateParam->readFile().c_str());  
    //  setDate(date);
  // }
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

void setDate(char* date) {
  int day, month, year, hour, minute, second; 
  sscanf(date, "%02d/%02d/%4d %02d:%02d:%02d", 
       &day, &month, &year, &hour, &minute, &second);
  // On initialise l'heure de l'horloge interne
  rtc->setTime(second, minute, hour, day, month - 1, year); 
}

/**
 * @brief Initialisation des GPIO
 * 
 * Initialise les GPIO utilisés par l'automate.
 * 
 * @details
 * - Pour la carte ES32A08, initialise les GPIO pour les registres à décalage 74HC595 et 74HC165
 * - Pour les autres cartes, initialise les GPIO pour les relais et entrées digitales
 */
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

  // Mettre les registres à décalage à zéro  
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

/**
 * @brief Ecriture inconditionnelle des logs
 * 
 * @param log  message de log
 */
void logsWrite(const char* log) {
  char buffer[80];
  if (fileLogs->fileSize() > MAX_LOG_SIZE)
    fileLogs->rmFile(); // Supprime le fichier si trop gros
  sprintf(buffer, "%s - %s\n", getDate(), log);
  fileLogs->writeFile(buffer, "a");
}

/**
 * @brief Ecriture conditionnelle des logs
 * 
 * @param log  message de log
 */
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
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  initOTA();
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
#else
/**
 * @brief Initialisation de la connexion WiFi en mode station
 * @return true si la connexion est établie, false sinon.
 */
boolean initWifiStation(boolean flagDisplay) {
  char buffer[32];
  int i;
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
#ifndef IO_DEBUG    
//  backLightOff();
#endif
    if (flagDisplay) {
      Serial.printf("Wifi %s not connected! ...\n", ssid);
      lcdPrintString("WiFi not connected", 1, 0, true);
    }
    if (++i > 3)
      return false;
  }
  WiFiClass::setHostname(hostname);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  initOTA();
  if (flagDisplay) {
    sprintf(buffer, "SSID : %s", WiFi.SSID().c_str());
    lcdPrintString(buffer, 1, 0, true);
    Serial.println(buffer);
    sprintf(buffer, "IP : %s", WiFi.localIP().toString().c_str());
    Serial.println(buffer);
    lcdPrintString(buffer, 2, 0, true);
  }
  return true;
}
#endif

/**
 * @brief Initialisation du client MQTT
 * 
 * @details
 * - Configure le client MQTT avec le serveur, le port et les callbacks.
 * - Tente de se connecter au serveur MQTT.
 * - En cas d'échec, réessaie plusieurs fois avec un délai.
 * - Abonne aux topics nécessaires.
 * 
 * @return true si la connexion est établie, false sinon.
 */

boolean initMQTTClient(boolean flagDisplay) {
  int i = 0;
  // Connecting to MQTT server
  mqttClient.setServer(mqttServer, mqttPort);
  // Fixé dans PubSubClient.h
  ///mqttClient.setBufferSize(MQTT_MAX_BUFFER_SIZE);
  mqttClient.setCallback(PubSubCallback);
  // Pour un même courtier les clients doivent avoir un id différent
  String clientId = "ESP32Client-";
  clientId += String(random(0xffff), HEX);
  while (!mqttClient.connect(clientId.c_str(), mqttUser, mqttPassword)) {
    if (++i > 3) {
      if (flagDisplay)
        Serial.println("Failed to connect to MQTT broker");
      return false;
    }
  }
  if (flagDisplay)
    Serial.println("MQTT client connected");
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
//  mqttClient.subscribe(TOPIC_MQTT_TEST);
  return true;
}


/**
 * @brief Point d'entrée du programme
 */
void setup() {
  char buffer[24];
  // Désactiver toutes les actions tant que pas initialisé
  onRotary = nullFunc;
  onDoubleClick = nullFunc;
  onLoopTic = nullFunc;
  onLoopTic2 = nullFunc;
  display = nullFunc;
  
  Serial.begin(115200);
  delay(100);
  Serial.println(bootRaison());
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
  sprintf(buffer, "HomeCtrl v%s", version);
  Serial.println(buffer);
  lcdPrintString(buffer, 0, 0, true);
  delay(500);
  // fileDateParam = initDateParam(false);
  fileLogs = initLogs(FORCE_LOGS);
  fileParam = initParam(FORCE_INIT_PARAM);
  fileDlyParam = initDlyParam(FORCE_INIT_DLY_PARAM);
  fileGlobalScheduledParam = initGlobalScheduledParam(FORCE_GLOBAL_SCHEDULED_PARAM);
  filePersistantParam = initPersitantFileDevice(FORCE_PERSISTANT_PARAM);
  
  if (wifiConnected = initWifiStation(true)) {
    sprintf(rssi_buffer, "RSSI:%ld", WiFi.RSSI());
    mqttConnect = initMQTTClient(true);
  }

  initTime(wifiConnected);
  initRotary();
  lcdPrintString(getDate(), 3, 0, true);
  Serial.println(getDate());
  // Serial.println(__LINE__);
  // Serial.println(__FILENAME__);
  // Serial.println(__FILE__);

  // Temps de lecture des infos
  delay(1000);
  logsWrite(bootRaison());
  // Serial.println(strlen(cParam->getStr()));
  // cParam->print();

#ifdef  WEB_SERIAL
  WebSerial.begin(&server);
  WebSerial.msgCallback(recvMsg);
  server.begin();
#endif

  //----------------------------------------- Init monostables ----------------------------------------------
  // ****** Pas d'accès fichier dans un monostable (kernel panic) ******
  // ****** Pas d'envois repété de messages MQTT (kernel panic)   ******
  // Monostable durée max lance arrosage
  tache_t_watering = t_cree(monoWatering, cvrtic(cDlyParam->get(TIME_WATERING) * 1000));
  // Monostable durée remplissage réservoir
  tache_t_tankFilling = t_cree(monoTankFilling, cvrtic(cDlyParam->get(TIME_TANK_FILLING) * 1000));
  // Monostable durée max mise sous pression surpresseur
  tache_t_surpressorFilling = t_cree(monoSurpressorFilling, cvrtic(cDlyParam->get(TIME_SUPRESSOR) * 1000));
  // Monostable durée ouverture electrovanne EST
  tache_t_cmdEvEst = t_cree(monoCmdEvEst, cvrtic(cDlyParam->get(EAST_VALVE_ON_TIME) * 1000));
  // Monostable délais de coupure de la PAC après commande d'arrét PAC
  tache_t_monoPacOff = t_cree(monoPacOff, cvrtic(DLY_PAC_OFF) * 1000);
  // Monostable délais d'envoi des commande vers la PAC après mise sous tension
  tache_t_monoPacOn = t_cree(monoPacOn, cvrtic(DLY_PAC_ON) * 1000);
  // Monostable delai avant envoi d'une commande mqtt vers la carte VMC déportée 
  tache_t_cmdVmcBoard = t_cree(monoCmdVmcBoard, cvrtic(DLY_VMC_BOARD_ON) * 1000);
  // Monostable delai avant envoi d'une commande mqtt vers la carte VMC déportée 
  tache_t_cmdVmcBoardOff = t_cree(monoCmdVmcBoardOff, cvrtic(DLY_VMC_BOARD_ON_OFF) * 1000);
  // Monostable durée affichage LCD local après modification d'une E/S
  tache_t_backLight = t_cree(monoCmdBackLight, cvrtic(DLY_BACK_LIGHT) * 1000);
  // Monostable durée d'affichage local si porte armoire éléctrique est restée ouverte 
  tache_t_backLight2 = t_cree(monoCmdBackLight2, cvrtic(DLY_BACK_LIGHT2) * 1000);
  // Monostable délai avant affichage par défaut
  tache_t_defaultDisplay = t_cree(monoDefaultDisplay, cvrtic(DLY_DEFAULT_SCREEN) * 1000);
  // Monostable durée d'ouverture de l'électrovanne déportée d'irrigation (circuit d'arrosage des tomates)
  tache_t_offCircuit2 = t_cree(monoOffCircuit2, cvrtic(DLY_DEFAULT_OFF_CIRCUIT2) * 1000);
  // Astable reglage débit, durée d'un pas de rapport cyclique
  tache_t_monoDebit = t_cree(monoDebit, cvrtic(PAS_PERIODE_DEBIT) * 1000, pdTRUE);
  // Monostable mise en sécurité supresseur
  tache_t_monoSurpressorSecurity = t_cree(monoSurpressorSecurity, cvrtic(DLY_DEFAUT_SUPRESSOR_SECURITY_TIMEOUT) * 1000);

  // Set watch dog timeout (10s)
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
  if (cPersistantParam->get(VANNE_EST)) {
    on(O_TRANSFO);
    on(O_EV_EST);
  }
#endif  

// Persistance obligatoire pour ce paramètre
// Utilisé par la vanne déportée arrosage des tomates
  ItemParam item = cParam->get(IRRIGATION, 0);
  joursCircuit2 = item.MMax;

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
// cDlyParam->print();
#ifdef ALEXA
  initAlexa();
  addDevices();
  Serial.println("Alexa command");
#endif  
  Serial.println("End setup");
}

/**
 * @brief Monostable mise en route carte VMC
 * Fermeture relai carte VMC  après delai de mise sous tension
 * Force VMC à plein vitesse
 * Nota : la marche rapide de la VMC est assurée par une carte esp01s déportée.
 * La carte esp01s déportée est alimentée par la mise sous
 * temsion de la VMC et necessite quelques secondes pour accepter les messages MQTT
 * ACCES AUX FICHIERS NON AUTORISES
 * UN SEUL MESSAGE MQTT A LA FOIS (débordement de la pile ISR)
 */
void monoCmdVmcBoard(TimerHandle_t xTimer) {
#ifdef DEBUG_OUTPUT
  print("start board\n", OUTPUT_PRINT);
#endif
  // t_stop(tache_t_cmdVmcBoard);
  // Envoi de la commande vers la carte déportée
  mqttClient.publish(VMC_BOARD_ACTION, S_ON);
  if (!vmcFast)
    t_start(tache_t_cmdVmcBoardOff);  
}

void monoCmdVmcBoardOff(TimerHandle_t xTimer) {
#ifdef DEBUG_OUTPUT
  print("Stop board\n", OUTPUT_PRINT);
#endif
  // Envoi de la commande vers la carte déportée
  if (!vmcFast)
    mqttClient.publish(VMC_BOARD_ACTION, S_OFF);
}

/*
 * Monostable arrêt PAC
 *
 */
void monoPacOff(TimerHandle_t xTimer) {
#ifdef DEBUG_OUTPUT
  print("Mono Arret PAC\n", OUTPUT_PRINT);
#endif
  off(O_PAC);
  irSendPacOff = false;
}

/**
 * @brief  Monostable mise en route de la PAC
 *         Envoi d'une commande IR retardée après mise sous tension
 * @param xTimer 
 */
void monoPacOn(TimerHandle_t xTimer) {
  mqttClient.publish(TOPIC_PAC_IR_ON, "");
}

/**
 * @brief  Monostable arrêt arrosage après xx mn (règlable dans dlyParam)
 * @param xTimer 
 */
void monoWatering(TimerHandle_t xTimer) {
  stopWatering();
}

/**
 * @brief  Monostable arrêt remplissage réservoir (règlable dans dlyParam)
 * @param xTimer 
 */
void monoTankFilling(TimerHandle_t xTimer) {
  stopTankFilling();
}

/**
 * @brief Monostable assurant l'arrêt pompe si problème sur le surpresseur (règlable dans dlyParam)
 * @param xTimer 
 */
void monoSurpressorFilling(TimerHandle_t xTimer) {
  // t_stop(tache_t_surpressorFilling);
  // startSupressorFilling = false;
  off(O_POMPE);
  if (!startSupressorFilling2) {
    // Echec première tentative
    // Possibilité de reprise par bouton réarmement local ou à distance
    mqttClient.publish(TOPIC_DEFAUT_SUPRESSOR, "on");
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
    mqttClient.publish(TOPIC_DEFAUT_SUPRESSOR, "on2");
    erreurPompe = true;
#ifdef DEBUG_OUTPUT
    print("TOPIC_GPIO_DEFAUT_SUPRESSEUR_N2  on\n", OUTPUT_PRINT);
#endif
  }
}

/**
 * @brief Monostable assurant l'arrêt de l'EV placée sur le robinet EST (règlable dans dlyParam)
 * en cas de commande hors mode programmation horaire
 * @param xTimer
 */
void monoCmdEvEst(TimerHandle_t xTimer) {
#ifdef DEBUG_OUTPUT
  print("monoCmdEvEst\n", OUTPUT_PRINT);
#endif
  off(O_TRANSFO);
  off(O_EV_EST);
}

/**
 * @brief Monostable assurant l'arrêt éclairage LCD
 * @param xTimer 
 */
void monoCmdBackLight(TimerHandle_t xTimer) {
  if (!electricalPanelOpen)
    backLightOff();
}

/**
 * @brief Monostable assurant l'arrêt eclairage LCD
 * @param xTimer 
 */
void monoCmdBackLight2(TimerHandle_t xTimer) {
  backLightOff();
}

/**
 * @brief Monostable assurant le rétablissement de l'affichage par défaut
 * @param xTimer 
 */
void monoDefaultDisplay(TimerHandle_t xTimer) {
  _ioDisplay();
}

/**
 * @brief Monostable assurant la coupure de l'électrovanne du circuit2 d'irrigation
 * @param xTimer 
 */   
 void monoOffCircuit2(TimerHandle_t xTimer) {
  mqttClient.publish(SUB_GPIO0_ACTION, "off");
}

/**
 * @brief Astable de réglage du débit l'électrovanne sud
 *        Réalise une commande PWM sur la vanne par pas de 5s
 *        20 rapports cycliques possible 5%...100%
 *        Période  = MAX_PAS_PERIODE_DEBIT * 5s = 100s
 *        cyclicalReport et le même pour toutes les commandes
 *        (manuelles ou programmées)
 *
 * @param xTimer 
 */
void monoDebit(TimerHandle_t xTimer) {
  int cyclicalReport = cParam->get(VANNE_EST, 3).HMin;
  static int count;
  if (count < cyclicalReport) {
    on(O_EV_EST);
  //  Serial.printf("cyclicalReport=%02d, count=%02d, ON \r", cyclicalReport, count);
  }
  else {
    off(O_EV_EST);
  //  Serial.printf("cyclicalReport=%02d, count=%02d, OFF\r", cyclicalReport, count);
  }
  count = ++count % MAX_PAS_PERIODE_DEBIT;
}

/**
 * @brief Monostable de surveillance surpresseur
 *  
 * @param xTimer 
 */
void monoSurpressorSecurity(TimerHandle_t xTimer) {
  if (cDlyParam->get(SURPRESSOR_SECURIT_EN) && 
      n_supressorFillingInTime >= MAX_SUPRESSOR_FILLING_IN_TIME ) {
    off(O_POMPE);
    cDlyParam->set(SUPRESSOR_EN, 0);
    mqttClient.publish(TOPIC_SUPRESSOR_SECURITY, "on");
    supressorFillingSecurity = true;
    monoSurpressorSecurityStarted = false;
  }
  else {
    supressorFillingSecurity = false;
  }
}

/**
 * @brief commande de la vanne EST par l'astable monoDebit 
 * 
 */
void onVanneEst() {
  on(O_TRANSFO);
  t_start(tache_t_monoDebit);
  // Logique inversée, utilisé pour signaler commande vanne EST
  cmdVanneEst = 0;
  writeLogs("Irrigation façade SUD");
}

/**
 * @brief Arrêt de la vanne EST
 * 
 */
void offVanneEst() {
  // Logique inversée
  cmdVanneEst = 1;
  off(O_TRANSFO);
  off(O_EV_EST);
  t_stop(tache_t_monoDebit);
  writeLogs("Fin irrigation façade SUD");
}

//--------------------------------------------------------------------------------------------
/**
 * @brief Exécuté toutes les minutes
 *        Exécute les actions temporelles programmées dans param
 * Appelé par loop (on ne peut pas utiliser les timers RTOS (schedule accède aux fichiers))
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
  // Mise à jour du jours courant (persistant) utilisé par circuit secondaire irrigation
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
    // Programmation autorisée pour ce device ?
    if (!cGlobalScheduledParam->get(deviceId)) {
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
          case POWER_COOK:
          #ifdef DEBUG_OUTPUT_SCHEDULE
          Serial.printf("%02d:%02d off(O_FOUR)\n", h, m);
#endif
#ifdef PERSISTANT_POWER_COOK            
          cPersistantParam->set(POWER_COOK, 0);
          filePersistantParam->writeFile(cPersistantParam->getStr(), "w");
#endif
          on(O_FOUR);
          // Mise à jour de l'IHM déportée
          mqttClient.publish(TOPIC_STATUS_CUISINE, "on"); 
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
          // La durée d'ouverture de la vanne est gérée par un monostable
#ifdef PERSISTANT_VANNE_EST
          cPersistantParam->set(VANNE_EST, 1);
          filePersistantParam->writeFile(cPersistantParam->getStr(), "w");
#endif          
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
          // Envoi d'une commande IR retardée
          t_start(tache_t_monoPacOn);
          break;

        case VMC:
#ifdef DEBUG_OUTPUT_SCHEDULE
          Serial.printf("%02d:%02d, vmcMode=%d \n", h, m, vmcMode);
#endif
          onVmc = 1;
          if (item.enable == 2)
            onVmc = 2; // marche rapide
          switch (vmcMode) {
          case VMC_STOP:
          case VMC_ON_FAST:
          case VMC_ON: break;
          default:
            on(O_VMC);
            t_start(tache_t_cmdVmcBoard);
            vmcMode = VMC_PROG_ON;
            if (onVmc == 2) {
              vmcFast = true;
              vmcMode = VMC_PROG_ON_FAST;
              // Démarrage carte VMC marche rapide
              // t_start(tache_t_cmdVmcBoard);
            }
            break;
          }
          break;
        default: ;
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
          // Mise à jour de l'IHM déportée
          mqttClient.publish(TOPIC_STATUS_CUISINE, "off"); 
          break;
        // L a durée du remplissage du réservoir gérée par un monostable
        case IRRIGATION:
          break;

        case VANNE_EST:
#ifdef DEBUG_OUTPUT_SCHEDULE
          Serial.printf("%02d:%02d off(O_EV_EST)\n", h, m);
#endif
#ifdef PERSISTANT_VANNE_EST
          cPersistantParam->set(VANNE_EST, 0);
          filePersistantParam->writeFile(cPersistantParam->getStr(), "w");
#endif 
          offVanneEst();
          break;

        case PAC:
#ifdef DEBUG_OUTPUT_SCHEDULE
          Serial.printf("%02d:%02d PAC ON", h, m);
#endif
          // Envoi de la commande d'aarêt sur l'emetteur IR
          mqttClient.publish(TOPIC_PAC_IR_OFF, "");
          // Les commandes IR TOPIC_PAC_IR_OFF sont publiées dans loop
          // irSendPacOff est mis à false dans monoPacOff
          irSendPacOff = true; 
          // Coupure retardée de l'alimentation de la PAC 
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
        default: ;
        }
      }
    }
  }
}

//-----------------------------------
// Démarrage et arrêt lance arrosage 
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
 * @brief  Arrêt lance arrosage
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

//-----------------------------------------
// Démarrage et arrêt remplissage réservoir
//-----------------------------------------
/**
 * @brief  Démarrage remplissage réservoir
 * @note   
 * @retval None
 */
void startTankFilling() {
  setDelay(tache_t_tankFilling, cvrtic(cDlyParam->get(TIME_TANK_FILLING) * 1000));
  t_start(tache_t_tankFilling);
  isTankFilling = true;
  // Priorité au remplissage si utilisation de la pompe en cours
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
  // vmcMode = cmd;
#ifdef PERSISTANT_VMC
  cPersistantParam->set(VMC, cmd);
  filePersistantParam->writeFile(cPersistantParam->getStr(), "w");
#endif
  // Serial.printf("SetVmc : %d\n", cmd);
  switch (cmd) {
  case CMD_VMC_OFF:
    // Mode off (pas de vcm active, même ne mode programmée)
    off(O_VMC);
    // vmcMode est utilisé pour signaler le mode en cours sur l'IMH déportée
    vmcMode = VMC_STOP;
    break;
  case CMD_VMC_PROG:
      // Mode programmé, onVmc et mise à jour par schedule
    switch (onVmc) {
    case 0:
      // Arrèt programmé 
      off(O_VMC);
      vmcFast = false;
      vmcMode = VMC_PROG_OFF;
      break;
    case 1:
      // VMC marche lente
      on(O_VMC);
      t_start(tache_t_cmdVmcBoard);      
      vmcFast = false;
      vmcMode = VMC_PROG_ON;
      break;
    case 2:
      // VMC marche rapide
      on(O_VMC);
      t_start(tache_t_cmdVmcBoard);
      vmcFast = true;
      vmcMode = VMC_PROG_ON_FAST;
      break;
    }
    break;
  case CMD_VMC_ON_FAST:
    // Mode forcé VMC (hors programmation) en vitesse rapide
    // Serial.printf("SetVmc CMD_VMC_ON_FAST: %d\n", CMD_VMC_ON_FAST);
    vmcFast = true;
    vmcMode = VMC_ON_FAST;
    on(O_VMC);
    t_start(tache_t_cmdVmcBoard);
    break;
  case CMD_VMC_ON:
    // Mode forcé VMC (hors programmation) en vitesse lente
    on(O_VMC);
    vmcFast = false;
    vmcMode = VMC_ON;
    t_start(tache_t_cmdVmcBoard);
    break;
  default: ;
  }
  char buffer[4];
  itoa(vmcMode, buffer, 10);
  mqttClient.publish(TOPIC_STATUS_VMC, buffer); 
}

//-----------------------------------
// Boucle de scrutation
//-----------------------------------
void loop() {
  static ulong tps = 0;
  static ulong tpsIr = 0;
  static ulong tpsRot = 0;
  static ulong tpsRotary = 0;
  static ulong tpsProg = 0;
  static ulong tpsRotaryUpdt = 0;
  static ulong tpsSchedule = 0;
  static ulong tpsWifiTest = 0;
  static ulong tpsWDTReset = 0;
  static ulong tpsWifiSignalStreng = INTERVAL_WIFI_STRENG_SEND;
   static unsigned mqttConnectTest=0;
  static boolean esp_task_wdt = true;
  static unsigned rotate;

#ifdef EXEC_TIME_MEASURE
  time_exec_start();
#endif
 
  ArduinoOTA.handle();
#ifdef IO_TEST
  return;
#endif    
  mqttClient.loop();
#ifdef ALEXA  
  fauxmo.handle();
#endif  
#ifdef ENABLE_WATCHDOG
  // Reset du chien de garde
  if (millis() - tpsWDTReset > INTERVAL_RESET_WDT) {
    tpsWDTReset = millis();
    esp_task_wdt_reset();
    delay(1); // Pour éviter le kernel panic
  }
#endif

  // Test wifi et reconnexion si nécessaire
  //static unsigned wifiTest = 0;
  // if (tpsWifiTest > INTERVAL_WIFI_TEST) {
  //   int i = 0;
  //   tpsWifiTest = millis();
  //   while (!WiFi.isConnected() && i++ < 3) { 
  //     Serial.println("WiFi not connected, retrying...");
  //     WiFi.reconnect();
  //     dly(200);
  //     if (WiFi.isConnected()) {
  //       wifiConnected = true;
  //       Serial.println("WiFi connected");
  //       break;
  //     }
  //     else {
  //       Serial.println("WiFi not connected");
  //       wifiConnected = false;
  //     }
  //   }
  // }

  // Vérification cnx broker MQTT
  if (!mqttClient.connected() && 
      millis() - mqttConnectTest > INTERVAL_MQTT_CONNECT_TEST) {
    mqttConnectTest = millis();
    if (initWifiStation(false)) 
      if (mqttConnect = initMQTTClient(false))
        lcdPrintChar('c', 2, 0);
      else
        lcdPrintChar('n', 2, 0);
    else
        lcdPrintChar('n', 2, 0);
  }

  // Scruter les ports E/S toutes les s pour les afficher sur LCD
  // Affichage uniquemment sur nouvel état des ports E/S
  static unsigned uPortIn_1  = 0xFFFFFFFF;
  static unsigned uPortOut_1 = 0xFFFFFFFF;

  if (millis() - tpsProg > INTERVAL_PORT_READ) {
    tpsProg = millis();

    static boolean bPac;
    // Faire clignoter le bit PAC sur lcd si en cours d'arret

    if (irSendPacOff) {
      bPac = !bPac;
      lcdPrintChar((bPac) ? '0' : '1', 2, 4);
    }

    // Ne mettre à jour l'affichage que si changement
    // testPortIO_0() calcule la somme des bits IO en 
    // tenant compte de leur poids
    boolean setDisplay = false;
    unsigned uPortOut_0 = testPortIO_O();

    if (uPortOut_1 != uPortOut_0) {
      uPortOut_1 = uPortOut_0;
      setDisplay = true;
    }

    // Ne mettre à jour l'affichage que si changement
    unsigned uPortIn_0 = testPortIO_I();
   
    if (uPortIn_1 != uPortIn_0) {
      uPortIn_1 = uPortIn_0;
      setDisplay = true;
    }
    if (setDisplay) display();
  }

  // Scrutation évenements E/S toutes les 100 ms
  if (millis() - tps > INTERVAL_IO_SCRUT) {
    // static int i = 0;
    tps = millis();
    // Serial.printf("Loop %d\n", xPortGetCoreID());
    localLoop();
  }

  // Mise à jour de l'indicateur tournant sur l'écran lcd toute les 500 ms
  // isLcdDisplayOn = true;
  if (isLcdDisplayOn && millis() - tpsRot > INTERVAL_ROTATE_DISPLAY ) {
    tpsRot = millis();
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

  // Envoi toutes les 15s d'une commande IR off sur la PAC si cycle d'arrèt
  if (irSendPacOff && millis() - tpsIr > INTERVAL_IR_SEND) {
    tpsIr = millis();
    mqttClient.publish(TOPIC_PAC_IR_OFF, "");  
  }

  // Appel de schedule toutes les minutes
  // pour les actions programmées
  if (millis() - tpsSchedule > INTERVAL_SCHEDULE) {
    tpsSchedule = millis();
    schedule();
  }
  
  // Envoi du niveau en db WiFi RSSI
  if (millis() - tpsWifiSignalStreng > INTERVAL_WIFI_STRENG_SEND ) {
    tpsWifiSignalStreng = millis();
    if (!isLcdDisplayOn || !wifiConnected) {
      return;
    //long rssi = WiFi.RSSI();
      sprintf(rssi_buffer, "RSSI:%ld", WiFi.RSSI());
    // Test niveau RSSI
    // mqttClient.publish(TOPIC_WIFI_STRENG, rssi_buffer);
      lcdPrintRssi(rssi_buffer);
    }
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
 *        supprimé. Voir commentaires en-tête de ce fichier
 * 
 * @param topic 
 * @param payload 
 * @param length 
 */
void PubSubCallback(char* topic, byte* payload, unsigned int length) {
  String strPayload = "";
  // Préserver le payload dans un buffer, utile que si on se réutilise payload
  // dans une commande MQTT
  // char buffer[80];
  // static char bufferPayload[512];
  // memcpy(bufferPayload, payload, length + 4);

  for (unsigned int i = 0; i < length; i++) {
    strPayload += static_cast<char>(payload[i]);
  }

#ifdef DEBUG_TOPIC
  Serial.print(topic);
  Serial.print(" : ");
  Serial.println(strPayload.c_str());
#endif

  //------------------  TOPIC_GET_PARAM ----------------------
  if (cmp(topic, TOPIC_GET_PARAM)) {
#ifdef DEBUG_OUTPUT
    print(cParam->getStr(), OUTPUT_PRINT);
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
      mqttClient.publish(TOPIC_DEFAUT_SUPRESSOR, "on");
    }
    if (erreurPompEvent) {
      erreurPompEvent = false;
      mqttClient.publish(TOPIC_DEFAUT_SUPRESSOR, "on2");
    }
    // if (supressorFillingSecurity){
    //   mqttClient.publish(TOPIC_SUPRESSOR_SECURITY, "on");
    // }
    // else {
    //   mqttClient.publish(TOPIC_SUPRESSOR_SECURITY, "off");
    // }
    return;
  }
  //------------------  TOPIC_WRITE_PARAM ----------------------
  // Attention taille du message importante 
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
    fileDlyParam->writeFile(strPayload.c_str(), "w");
    if (cDlyParam->get(SUPRESSOR_EN)) {
       mqttClient.publish(TOPIC_SUPRESSOR_SECURITY, "off");
       supressorFillingSecurity = false;
    }
    // Mis à jour si modification de auto surpresseur
    // cDlyParam->print();
    ioDisplay();
		display = ioDisplay;
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
      // Serial.println(tabValue);
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

    // sprintf(buffer, "topic : %s, port %s\n", topic, bufferPayload.c_str());
    // print(buffer, OUTPUT_PRINT);

    // cmdVanneEst = cmp(strPayload.c_str(), "1");
    return;
  }
  //------------------  TOPIC_CMD_PAC ----------------------
  if (cmp(topic, TOPIC_CMD_PAC)) {
    unsigned cmd = atoi(strPayload.c_str());
    // Logique inversée pour relai PAC
    // 1 = arret
    if (cmd) {
      // Arrèter la PAC via IR
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
    // La taille du message est limitée par l'implémentation de MQTT
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
    // long rssi = WiFi.RSSI();
    // sprintf(rssi_buffer, "RSSI:%ld", WiFi.RSSI());
    static char info[128];
    sprintf(info, "%s\n%s, RSSI:%lddb\n%s", 
                   version, WiFi.localIP().toString().c_str(), WiFi.RSSI(), getDate());
    // String info = String(version) + "\n" + WiFi.localIP().toString() + ", " + rssi_buffer + "db\n" + String(getDate());
    // Serial.println(info);
    mqttClient.publish(TOPIC_READ_VERSION, info);
    return;
  }
  //------------------  TOPIC_WATCH_DOG_OFF ----------------------
  if (cmp(topic, TOPIC_WATCH_DOG_OFF)) {
    esp_task_wdt_delete(NULL);
    delay(1);
    //Serial.println("WD disabled"); 
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
  // if (cmp(topic, TOPIC_MQTT_TEST)) {
  //   mqttConnect = true;   
  //   // Serial.println("INTERVAL_MQTT_CONNECT_TEST_OK"); 
  //   return;
  // }
}
