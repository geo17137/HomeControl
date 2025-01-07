#ifndef LOOP_PROG_H
#define LOOP_PROG_H
#include <Arduino.h>
#include <esp_task_wdt.h>
#include "files.h"
#include "rotary_encoder.h"
#include "param.h"
#include "simple_param.h"
#include "display.h"
#include "const.h"
#include "io.h"

// Butées temporelles 
// Temps en secondes
#define MaxTimeOutWaterring 60*60  
#define MinTimeOutWaterring 10*60
#define MaxTimeTankFilling  200
#define MinTimeTankFilling  100
#define MaxTimeOutWaterringEV_Est 20*60
#define MinTimeOutWaterringEV_Est  5*60
#define MaxTimeOutSupressor  100
#define MinTimeOutSupressor  50

extern boolean isWatering;
// Un entier suffit pour contenir les valeurs
// obtenues par l'encodeur
static int rotary;
static const char* msgOk;
static const char* unit = "";

// Messages dans loop_prog
static const char* Push_to_validate = "Push to validate";
static const char* Dbl_Push_to_validate = "Dbl Push to validate";
static const char* Push_to_Quit = "Push to Quit";
static const char* Select_function = "Select function";

extern const char* msgRotary[7];
extern const char* msgSetOutput[7];
extern const char* msgOnOff[2];
extern const char* msgTimer[5];
extern const char* msgYesNo[2];
extern const char* msgSummerTm[1];
extern const char* msgLogSetting[1];
extern const char* msgScheduledAction[5];
extern const char* MSG_OK;

extern FileLittleFS* fileDlyParam;
extern FileLittleFS* filePersistantParam;
extern FileLittleFS* fileGlobalScheduledParam;
extern unsigned wateringNoTimeOut;
extern Param* cParam;
extern SimpleParam* cDlyParam;
extern SimpleParam* cGlobalScheduledParam;
extern SimpleParam* cPersistantParam;
extern boolean irSendOn;

extern void (*display)();
extern void (*onSingleClick)();
extern void (*onDoubleClick)();
extern void (*onRotary)();
extern void (*onLoopTic)();
extern void (*onLoopTic2)();
extern void (*funcToCall)();
extern void ioDisplay();
extern void (*funcToCall2)();
extern void startWatering(int timeout);
extern void stopWatering(void);
extern void startTankFilling(void);
extern void stopTankFilling(void);
extern void localLoop(void);
extern void setVmc(int cmd);
extern void printMqttDebugValue(const char* value);
extern TACHE_T tache_t_watering;
extern TACHE_T tache_t_monoPacOff;
extern TACHE_T tache_t_monoPacOn;
extern TACHE_T tache_t_watering;
extern TACHE_T tache_t_tankFilling;
extern TACHE_T tache_t_surpressorFilling;
extern TACHE_T tache_t_cmdEvEst;
extern TACHE_T tache_t_cmdVmcBoard;
extern TACHE_T tache_t_backLight;
extern TACHE_T tache_t_encoderTask;
extern TACHE_T tache_t_backLight2;

extern void setBoundaries(int min, int max);

void apply();
void loopTic(void (*func)());
void loopTic2(void (*func)());
void _ioDisplayAndValidateDlyParam();
void buttonFuncParam();
void setEncoder(int val);
void loopProg();
void buttonFuncLevel1_1();
void buttonFuncLevel1_2();
void buttonFuncLevel1_3();
void buttonFuncLevel1_4();
void buttonFuncLevel1_5();

void buttonFuncLevel2_1_0();
void buttonFuncLevel2_1_1();
void buttonFuncLevel2_1_2();
void buttonFuncLevel2_1_3();
void buttonFuncLevel2_1_4();
void buttonFuncLevel2_1_5();

void buttonFuncLevel2_2_0();
void buttonFuncLevel2_2_1();
void buttonFuncLevel2_2_2();
void buttonFuncLevel2_2_3();
void buttonFuncLevel2_2_4();
void buttonFuncLevel2_2_5();

void buttonFuncLevel2_3_0();

void buttonFuncLevel2_4_0();

void buttonFuncLevel2_5_0();
void buttonFuncLevel2_5_1();
void buttonFuncLevel2_5_2();
void buttonFuncLevel2_5_3();
void buttonFuncLevel2_5_4();


void encoderSetTimeWattering();

/**
 * @brief Valeur port gpio sous forme de chaine 
 * @param gpio 
 * @return const char* 
 */
inline const char* n0gpioRead(int gpio) {
  return digital_read(gpio) == 0 ? "1" : "0";
}
/**
 * @brief Valeur port gpio sous forme de chaine
 *       complétée à gauche par un blanc
 * @param gpio 
 * @return const char* 
 */
inline const char* n1gpioRead(int gpio) {
  return digital_read(gpio) == 0 ? " 1" : " 0";
}
/**
 * @brief Valeur port gpio sous forme de chaine
 *       complétée à gauche par deux blancs 
 * @param gpio 
 * @return const char* 
 */
inline const char* ngpioRead(int gpio) {
  return digital_read(gpio) == 0 ? "  1" : "  0";
}
/**
 * @brief Valeur port gpio en valeur inversée
 * @param gpio 
 * @return unsigned 
 */
inline unsigned ugpioRead(int gpio) {
  return digital_read(gpio) == 0 ? 1 : 0;
}

/**
 * @brief Valeur port gpio O_TRANSFO combinée avec un
 *        autre port gpio complétée à gauche par deux blancs
 *        Valeur active 0.
 * @param gpio 
 * @return const char* 
 */
inline const char* gpioSRead(int gpio) {
  unsigned val = digital_read(O_TRANSFO) + digital_read(gpio);
  // Si les deux sorties sont actives
  return val == 0 ? "  1" : "  0";
}
/**
 * @brief Valeur port gpio O_TRANSFO combinée avec un
 *        autre port gpio
 *        Valeur active 0.
 * @param gpio 
 * @return unsigned 
 */
inline unsigned ugpioSRead(int gpio) {
  return digital_read(O_TRANSFO) + digital_read(gpio) == 0 ? 1 : 0;
}
#endif
