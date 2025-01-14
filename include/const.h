#ifndef CONST_H
#define CONST_H

const String version = "2025.01.14";

// #define IO_TEST
//#define FORCE_DISPLAY
#define PRODUCT_DEVICE
#define ES32A08
#define ENABLE_WATCHDOG
#ifdef  PRODUCT_DEVICE
#define TOPIC_PREFIX ""
#ifdef ES32A08
#define I2C_ADR  PCF8574_ADDR_A21_A11_A01
#else
#define I2C_ADR  PCF8574A_ADDR_A21_A11_A01
#endif
#else
#define TOPIC_PREFIX "_"
#ifdef ES32A08
#define I2C_ADR  PCF8574_ADDR_A21_A11_A01
#else
#define I2C_ADR  PCF8574A_ADDR_A21_A11_A01
#endif
#endif

// Décommenter les conditions d'exécution désirées
//------------------------------------------------
// #define EXEC_TIME_MEASURE

// Versions à générer
// #define DEBUG_OUTPUT
// #define DEBUG_OUTPUT_LOOP2
// #define DEBUG_OUTPUT_SCHEDULE
// #define TIME_SIMULATOR
// #define WEB_SERIAL
// #define DEBUG_HEAP
//------------------------

#define FORCE_INIT_PARAM false  
#define FORCE_INIT_DLY_PARAM false 
#define FORCE_GLOBAL_SCHEDULED_PARAM false
#define FORCE_PERSISTANT_PARAM false
#define MAX_LOG_SIZE 2048

// Taille max des packet MQTT
#define MQTT_MAX_BUFFER_SIZE 512

// Remplacer avec les caractéristiques de votre réseau
// #define WIFI_MANAGER
#ifdef WIFI_MANAGER
#define SSID_AP   "Esp32HomeCtrl"
#endif

// #define  CONFIG_ARDUINO_LOOP_STACK_SIZE 2*8192

// Delay de mise hors tension PAC (s)
#define DLY_PAC_OFF 5 * 60
// Retard envoi commande carte IR après mise sous tension PAC (s)
#define DLY_PAC_ON  20
// Retard commande carte deportée fast vmc après mise sa mise sous tension (s)
#define DLY_VMC_BOARD_ON 10
// Délai COUPURE ECLAIRAGE LCD (s)
#define DLY_BACK_LIGHT 10
// Délai COUPURE ECLAIRAGE LCD panneau electrique ouvert (s)
#define DLY_BACK_LIGHT2 180
// Délai reprise affichage par défaut (s)
#define DLY_DEFAULT_SCREEN 6 * 60
// Délai de coupure circuit2 (s)
#define DLY_DEFAULT_OFF_CIRCUIT2 60*60
// Période bistable reglage débit (s)
const int PAS_PERIODE_DEBIT = 5;
// Période du rapport cyclique vanne EST en s
const int PERIODE_DEBIT = 100;
const int MAX_PAS_PERIODE_DEBIT = (PERIODE_DEBIT / PAS_PERIODE_DEBIT);

// Utilisé par wattering
#define TIMEOUT    1
#define NO_TIMEOUT 0

// Délai des appels dans loop en ms
#define INTERVAL_PORT_READ  1000
#define INTERVAL_IO_SCRUT   100
#define INTERVAL_RESET_WDT  500
#define INTERVAL_ROTATE_DISPLAY  500
#define INTERVAL_ROTATARY_SCHEDULE 10
#define INTERVAL_IR_SEND 15*1000
#define INTERVAL_WIFI_TEST 10*60*10000
#define INTERVAL_WIFI_STRENG_SEND 30*1000
#define INTERVAL_MQTT_CONNECT_TEST 5*60*1000
#ifdef TIME_SIMULATOR
#define INTERVAL_SCHEDULE 1000
#else
#define INTERVAL_SCHEDULE 60*1000
#endif

//----------------------------
// Modes de fonctionnement VMC
//----------------------------
#define VMC_STOP          0
#define VMC_PROG_OFF      1
#define VMC_PROG_ON       2
#define VMC_PROG_ON_FAST  3
#define VMC_ON_FAST       4
#define VMC_ON            5

//--------------------------
// Modes de commande VMC
//--------------------------
#define CMD_VMC_OFF       0
#define CMD_VMC_PROG      1
#define CMD_VMC_ON_FAST   2
#define CMD_VMC_ON        3

//-----------------------
//    Sorties relais
//-----------------------
// Relais avec contacts sous 220V
#ifdef ES32A08
// Les sorties relais sont liée à la postion des bits
// dans le registre 74HC595 (U05)
#define O_VMC           (0x01 << 16)
#define O_PAC           (0x02 << 16)
#define O_FOUR          (0x04 << 16)
#define O_POMPE         (0x08 << 16)
#define O_TRANSFO       (0x10 << 16)
// Relais avec contacts sous 19V
#define O_EV_ARROSAGE   (0x20 << 16)
#define O_EV_IRRIGATION (0x40 << 16)
#define O_EV_EST        (0x80 << 16)
#else
// Les sorties relais sont aux gpio ESP32
#define O_POMPE   05
#define O_PAC     19
#define O_FOUR    18
#define O_VMC     23
#define O_TRANSFO 17
// Sorties 19V
#define O_EV_EST         13
#define O_EV_IRRIGATION  04
#define O_EV_ARROSAGE    16
#endif

//------------------------------
//    Sorties commande HCT74165
#ifdef ES32A08
//------CLK_165-----------------
#define CLK_165   GPIO_NUM_17
#define LOAD_165  GPIO_NUM_16
#define POWER_LED GPIO_NUM_15

#define CLOCK_955 GPIO_NUM_27
#define LATCH_955 GPIO_NUM_14
#define NOE_955   GPIO_NUM_4
#define DATA_955  GPIO_NUM_13
#endif

// Commande sur gpio
#define OFF 0
#define ON  1
// Commande mqtt
#define S_OFF "off"
#define S_ON  "on"

//-----------------------
//    Entrées
//-----------------------
#ifdef ES32A08
// Entrée série circuit HCT74165
#define DATA_165  GPIO_NUM_5

#define I_ARROSAGE    (1 << 0)
#define I_IRRIGATION  (1 << 1)
#define I_SURPRESSEUR (1 << 2)
#define I_LCD_CMD     (1 << 3)
#else // commande directe gpio esp32 
#define I_ARROSAGE    27
#define I_IRRIGATION  14
#define I_SURPRESSEUR 12
#define I_LCD_CMD     26
#endif

// Port rotary button validation
#define I_REARM       36 

// Affichage LCD
// Ports I2C de commande
#define I2C_SCL 22
#define I2C_SDA 21
// Caractéristiques affichage LCD
#define COLUMS           20   //LCD columns
#define ROWS             4    //LCD rows

#define LCD_SPACE_SYMBOL 0x20 //space symbol from LCD ROM, see p.9 of GDM2004D datasheet

// Position de l'affichage RSSI
#define RSSI_ROWS_POS   0
#define RSSI_COLUMS_POS 17

#ifndef ES32A08 
// Ports non utilisés
// programmés en sortie
#define NC_32 32
#define NC_33 33
#define NC_25 25
#define NC_00 00
#define NC_02 02
#define NC_15 15
#endif

// Le TOPIC_PREFIX utilisé pour un système sous test
// Chaine vide en production

//-------------Abonnements---------------------
#define TOPIC_GET_PARAM       TOPIC_PREFIX "homecontrol/param_get"
#define TOPIC_WRITE_PARAM     TOPIC_PREFIX "homecontrol/write_param"
#define TOPIC_GET_DLY_PARAM   TOPIC_PREFIX "homecontrol/get_dly_param"
#define TOPIC_WRITE_DLY_PARAM TOPIC_PREFIX "homecontrol/write_dly_param"
#define TOPIC_GET_GPIO        TOPIC_PREFIX "homecontrol/get_gpio"
#define TOPIC_CMD_ARROSAGE    TOPIC_PREFIX "homecontrol/arrosage"
#define TOPIC_CMD_IRRIGATION  TOPIC_PREFIX "homecontrol/irrigation"
#define TOPIC_CMD_CUISINE     TOPIC_PREFIX "homecontrol/cuisine"
#define TOPIC_CMD_VMC         TOPIC_PREFIX "homecontrol/vmc"
#define TOPIC_CMD_VANNE_EST   TOPIC_PREFIX "homecontrol/vanne_est"
#define TOPIC_CMD_PAC         TOPIC_PREFIX "homecontrol/pac"
#define TOPIC_CMD_REAMORCER   TOPIC_PREFIX "homecontrol/rearmorcer"
#define TOPIC_LOGS_GET        TOPIC_PREFIX "homecontrol/logs_get"
#define TOPIC_CLEAR_LOGS      TOPIC_PREFIX "homecontrol/clear_logs"
#define TOPIC_REBOOT          TOPIC_PREFIX "homecontrol/reboot"
#define TOPIC_GET_VERSION     TOPIC_PREFIX "homecontrol/versions_get"
#define TOPIC_WATCH_DOG_OFF   TOPIC_PREFIX "homecontrol/watch_dog_off"
#define TOPIC_GET_GLOBAL_SCHED TOPIC_PREFIX "homecontrol/global_sched_get" 
#define TOPIC_WRITE_GLOBAL_SCHED TOPIC_PREFIX "homecontrol/global_sched_write" 
#define TOPIC_MQTT_TEST       TOPIC_PREFIX "homecontrol/mqtt_test" 

//-------------Publications--------------------
#define TOPIC_READ_VERSION   TOPIC_PREFIX  "homecontrol/readVersion"
#define TOPIC_READ_LOGS      TOPIC_PREFIX  "homecontrol/readLogs"
#define TOPIC_PARAM          TOPIC_PREFIX  "homecontrol/param"
#define TOPIC_DLY_PARAM      TOPIC_PREFIX  "homecontrol/dly_param"
#define TOPIC_GPIO           TOPIC_PREFIX  "homecontrol/gpio"
#define TOPIC_DEFAUT_SUPRESSEUR            "homecontrol/default_surpressor"
#define TOPIC_GLOBAL_SCHED   TOPIC_PREFIX  "homecontrol/global_sched" 
#define TOPIC_DEBUG_VALUE    TOPIC_PREFIX  "homecontrol/debug"
#define TOPIC_WIFI_STRENG    TOPIC_PREFIX  "homecontrol/wifi_streng"
#define TOPIC_STATUS_CUISINE TOPIC_PREFIX  "homecontrol/cuisine_status"
#define TOPIC_STATUS_VMC                   "homecontrol/vmc_status"
//-------------Publications pour l'appli circuit2 (carte déportée irrigation) ---
#define SUB_GPIO0_ACTION                   "circuit2/action"

//-------------Publications pour l'appli MitsubishiPAC_IR--------------------
#define TOPIC_PAC_IR_PARAM_APPLY TOPIC_PREFIX "mitsubishi/param/apply"
#define TOPIC_PAC_IR_ON          TOPIC_PREFIX "mitsubishi/param/on"
#define TOPIC_PAC_IR_OFF         TOPIC_PREFIX "mitsubishi/param/off"

//-------------Publications pour l'appli VmcCmd (carte déportée command marche rapide vmc)
#define VMC_BOARD_ACTION TOPIC_PREFIX TOPIC_PREFIX "vmc_board/action"

// Time out watch dog (ms)
#define WDT_TIMEOUT 50

// Nom des fichiers
// Paramétres des cycles programmés sur 24H 
#define PARAM_FILE_NAME "/param.txt"
// Logs
#define LOG_FILE_NAME   "/logs.txt"
// Paramétres des temporistions
#define DLY_PARAM_FILE_NAME "/dlyParam.txt"
// Paramétres des autorisation des cycles programmés
#define GLOBAL_SCHEDULED_PARAM_FILE_NAME "/globalScheduled.txt"
// Fichier des données persistantes
#define FILE_PERSISTANT_DEVICE "/persistant.txt"

// Contenu du fichier DLY_PARAM_FILE_NAME (valeurs par défaut)
//  Temporisations monostables par défaut en secondes
//  et paramètres heure été/hivers, enregistrement logs
// -----------------------------------------------
// Durée arrosage 30mn (1800s)
// Durée commande EV Est 10mn (600s)
// Durée remplissage réservoir 2mn30 (150s)
// Durée maximale mise sous pression supresseur 65s
// Heure d'hivers 1, été 2
// Autorisation logs 1 sinon 0
// Autorisation mise en route supresseur 1 sinon 0
// -----------------------------------------------
#define DEFAUT_DELAY_PARAM "1800:600:150:65:1:0:1"

// Position des paramètres dans DEFAUT_DELAY_PARAM
#define TIME_WATERING       0
#define EAST_VALVE_ON_TIME  1
#define TIME_TANK_FILLING   2
#define TIME_SUPRESSOR      3
#define SUMMER_TIME         4
#define LOG_STATUS          5
#define SUPRESSOR_EN        6
#define N_DLY_PARAM         7

// Décalage GMTheure d'été/hivers
#define SUMMER_TIME_OFFSET 2
#define WINTER_TIME_OFFSET 1

// Contenu du fichier GLOBAL_SCHEDULED_PARAM_FILE_NAME (par défaut)
// Programmation activée par défaut
// #define DEFAUT_GLOBAL_SCHEDULED_PARAM "1:1:1:1:1:1"
// Indice des dispositifs associés aux paramètres
#define POWER_COOK 0
#define IRRIGATION 1
#define VANNE_EST  2
#define PAC        3
#define VMC        4
#define MAX_ITEMS_GLOBAL_SCHEDULED_PARAM 5

#define POWER_COOK_VALUE "1:"
#define IRRIGATION_VALUE "1:"
#define VANNE_EST_VALUE "1:"
#define PAC_VALUE "1:"
#define VMC_VALUE "1"

#define DEFAUT_GLOBAL_SCHEDULED_PARAM \
          POWER_COOK_VALUE\
          IRRIGATION_VALUE\
          VANNE_EST_VALUE\
          PAC_VALUE\
          VMC_VALUE

// Contenu du fichier FILE_PERSISTANT_DEVICE (par défaut)
// Utilise les mêmes indices que DEFAUT_GLOBAL_SCHEDULED_PARAM 
#define PERSISTANT "0:0:0:1:0"
#define N_PERSISTANT_PARAM MAX_ITEMS_GLOBAL_SCHEDULED_PARAM

// Paramétrage des éléments persistants après reboot
// La persistance n'est mis en oeuvre que pour les cycles programmés
// sauf pour la VMC
// Définir ces éléments pour les rendre persistant
// après reboot (recoompilation obligatoire)
#define PERSISTANT_POWER_COOK 
#define PERSISTANT_PAC        
#define PERSISTANT_VMC 
#define PERSISTANT_VANNE_EST  
// Non encore implémenté
// #define PERSISTANT_IRRIGATION 

/*
  Format des paramètres des cycles programmés sur 24H
  Une ligne = un dispositif avec quatre plages de commande de format
  auto:h_min:m_min:h_max:m_max: N_PLAGE x
  Chaque plages comporte 5 items
  auto : "1" programmé, "0" non programmé ("2" pour la commande pleine vitesse VMC)
  h_min : heure de d'enclenchement de l'action
  m_min : minute d'enclenchement de l'action
  h_max : heure d'arrèt de l'action
  m_mmx : minute d'arrèt de l'action

  Format de la chaine enregistrée sur la mémoire flash esp32
  Déclaré dans main.h
const char *PARAM = "0:00:00:00:00:0:00:00:00:00:0:00:00:00:00:0:00:00:00:00" \
                    "0:00:00:00:00:0:00:00:00:00:0:00:00:00:00:0:00:00:00:00" \
                    "0:00:00:00:00:0:00:00:00:00:0:00:00:00:00:0:00:00:00:00" \
                    "0:00:00:00:00:0:00:00:00:00:0:00:00:00:00:0:00:00:00:00" \
                    "0:00:00:00:00:0:00:00:00:00:0:00:00:00:00:0:00:00:00:00";
  Les quatres premieres plages concernent la coupure des appareils de cuisson (seul h_max et m_max sont utilisés)
  Les quatres plages suivantes concernent le remplissage du réservoir pour l'irrigation (seul h_min et m_min sont utilisés)
  Les quatres plages suivantes concernent la commande de l'électrovanne EST pour l"arrosage facade SUD
  Les quatres plages suivantes concernent la commande de la PAC
  Les quatres dernières plages concernent la commande de la VMC

- Champs disponibles et utlisés pour d'autres usages
  POWER_COOK :  h_min, m_min pour toutes les plages pour 3 plages utilisées (1 plage libre)
  IRRIGATION :  h_max -> intervalle ouverture circuit2 (irrigation tomates), m_max -> jour courant (plage 0 uniquement) (3 plages utilisées)
  VANNE_EST  :  tout les champs sont utilisés pour 3 plages (1 plage libre)
  PAC        :  tout les champs sont utilisés pour 1 plage (3 plages libres)
  VMC        :  tout les champs sont utilisés (toutes les plages utilisées)

  La programmation des actions est faite toute les minutes dans schedule()

  Pour ajouter un dispositif:
  - ajouter une ligne dans PARAM
  - ajouter 1 à N_DEVICES

  Pour ajouter une plage:
  - ajouter 0:00:00:00:00 à chaque lignes
  - ajouter 1 à NB_PLAGES

0:00:00:00:00:0:00:00:00:00:0:00:00:00:00:0:00:00:00:00
<---Plage---> <---Plage---> <---Plage---> <---Plage--->
*/

// Nb de paramètres et de champs/paramètre
// Nombre d'items / plages
#define N_ITEMS_PLAGE 5
// Nombre de plages / device
// Toutes les N_PLAGES on accède à un nouveau dispositif
#define N_PLAGES 4
// Nombre de disposififs gérés
#define N_DEVICES 5
// Nombre total de paramètres pour tous les dispositifs
#define N_T_PARAM N_PLAGES * N_DEVICES
// Nombre de paramètres pour un dispositif
#define N_PARAM N_PLAGES * N_DEVICES
// Taille en octet d'une plage
// "0:00:00:00:00:"
#define TAILLE_PLAGE 14

// Messages utilisés dans local_loop pour l'affichage sur LCD
#define SUPRESSOR_FILLING   "FILL SUPRESSOR."
#define WAIT                "WAIT"
#define SEE_NOTICE          "SEE NOTICE"
#define SURPRESSOR_FAULT    "SURPRESSOR FAIL"
#define PRESS_THE_BUTTON    "PRESS THE BUTTON"
#define SEE_NOTICE          "SEE NOTICE"
#define PUMP_DEFAULT        "PUMP DEFAULT"
#define CORRECT_THE_DEFECT  "CORRECT THE DEFECT"
#define THEN_PRESS_BUTTON   "THEN PRESS BUTTON"
#define SEE_NOTICE2         "**SEE NOTICE**"
#define REBOOT              "REBOOT....."

#endif
