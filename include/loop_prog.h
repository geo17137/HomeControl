/**
 * @file loop_prog.h
 * @brief Header file for loop program functions and definitions.
 * 
 * This file contains function declarations, macros, and global variables 
 * used in the loop program for the ESP32 Home Control Display project.
 * 
 * Includes:
 * - Arduino.h
 * - esp_task_wdt.h
 * - files.h
 * - rotary_encoder.h
 * - param.h
 * - simple_param.h
 * - display.h
 * - const.h
 * - io.h
 * 
 * Defines:
 * - MaxTimeOutWaterring: Maximum timeout for watering in seconds.
 * - MinTimeOutWaterring: Minimum timeout for watering in seconds.
 * - MaxTimeTankFilling: Maximum time for tank filling.
 * - MinTimeTankFilling: Minimum time for tank filling.
 * - MaxTimeOutWaterringEV_Est: Maximum timeout for watering EV_Est.
 * - MinTimeOutWaterringEV_Est: Minimum timeout for watering EV_Est.
 * - MaxTimeOutSupressor: Maximum timeout for suppressor.
 * - MinTimeOutSupressor: Minimum timeout for suppressor.
 * 
 * Global Variables:
 * - isWatering: Boolean indicating if watering is active.
 * - rotary: Integer for rotary encoder values.
 * - msgOk: Constant character pointer for OK message.
 * - unit: Constant character pointer for unit string.
 * - Push_to_validate: Constant character pointer for "Push to validate" message.
 * - Dbl_Push_to_validate: Constant character pointer for "Dbl Push to validate" message.
 * - Push_to_Quit: Constant character pointer for "Push to Quit" message.
 * - Select_function: Constant character pointer for "Select function" message.
 * - msgRotary: Array of constant character pointers for rotary messages.
 * - msgSetOutput: Array of constant character pointers for set output messages.
 * - msgOnOff: Array of constant character pointers for on/off messages.
 * - msgTimer: Array of constant character pointers for timer messages.
 * - msgYesNo: Array of constant character pointers for yes/no messages.
 * - msgSummerTm: Array of constant character pointers for summer time messages.
 * - msgLogSetting: Array of constant character pointers for log setting messages.
 * - msgScheduledAction: Array of constant character pointers for scheduled action messages.
 * - MSG_OK: Constant character pointer for OK message.
 * - fileDlyParam: Pointer to FileLittleFS for delay parameters.
 * - filePersistantParam: Pointer to FileLittleFS for persistent parameters.
 * - fileGlobalScheduledParam: Pointer to FileLittleFS for global scheduled parameters.
 * - wateringNoTimeOut: Unsigned integer for watering no timeout.
 * - cParam: Pointer to Param object.
 * - cDlyParam: Pointer to SimpleParam object for delay parameters.
 * - cGlobalScheduledParam: Pointer to SimpleParam object for global scheduled parameters.
 * - cPersistantParam: Pointer to SimpleParam object for persistent parameters.
 * - irSendPacOff: Boolean indicating if IR send PAC is off.
 * - display: Function pointer for display function.
 * - onSingleClick: Function pointer for single click handler.
 * - onDoubleClick: Function pointer for double click handler.
 * - onRotary: Function pointer for rotary handler.
 * - onLoopTic: Function pointer for loop tic handler.
 * - onLoopTic2: Function pointer for second loop tic handler.
 * - funcToCall: Function pointer for a function to call.
 * - ioDisplay: Function pointer for IO display function.
 * - funcToCall2: Function pointer for a second function to call.
 * - startWatering: Function to start watering with a timeout.
 * - stopWatering: Function to stop watering.
 * - startTankFilling: Function to start tank filling.
 * - stopTankFilling: Function to stop tank filling.
 * - localLoop: Function for local loop.
 * - setVmc: Function to set VMC command.
 * - printMqttDebugValue: Function to print MQTT debug value.
 * - tache_t_watering: TACHE_T structure for watering task.
 * - tache_t_monoPacOff: TACHE_T structure for mono PAC off task.
 * - tache_t_monoPacOn: TACHE_T structure for mono PAC on task.
 * - tache_t_tankFilling: TACHE_T structure for tank filling task.
 * - tache_t_surpressorFilling: TACHE_T structure for suppressor filling task.
 * - tache_t_cmdEvEst: TACHE_T structure for EV_Est command task.
 * - tache_t_cmdVmcBoard: TACHE_T structure for VMC board command task.
 * - tache_t_backLight: TACHE_T structure for backlight task.
 * - tache_t_encoderTask: TACHE_T structure for encoder task.
 * - tache_t_backLight2: TACHE_T structure for second backlight task.
 * 
 * Function Declarations:
 * - apply: Apply function.
 * - loopTic: Loop tic function with a function pointer parameter.
 * - loopTic2: Second loop tic function with a function pointer parameter.
 * - _ioDisplayAndValidateDlyParam: IO display and validate delay parameters function.
 * - buttonFuncParam: Button function for parameters.
 * - setEncoder: Set encoder value function.
 * - loopProg: Loop program function.
 * - buttonFuncLevel1_1: Button function for level 1.1.
 * - buttonFuncLevel1_2: Button function for level 1.2.
 * - buttonFuncLevel1_3: Button function for level 1.3.
 * - buttonFuncLevel1_4: Button function for level 1.4.
 * - buttonFuncLevel1_5: Button function for level 1.5.
 * - buttonFuncLevel2_1_0: Button function for level 2.1.0.
 * - buttonFuncLevel2_1_1: Button function for level 2.1.1.
 * - buttonFuncLevel2_1_2: Button function for level 2.1.2.
 * - buttonFuncLevel2_1_3: Button function for level 2.1.3.
 * - buttonFuncLevel2_1_4: Button function for level 2.1.4.
 * - buttonFuncLevel2_1_5: Button function for level 2.1.5.
 * - buttonFuncLevel2_2_0: Button function for level 2.2.0.
 * - buttonFuncLevel2_2_1: Button function for level 2.2.1.
 * - buttonFuncLevel2_2_2: Button function for level 2.2.2.
 * - buttonFuncLevel2_2_3: Button function for level 2.2.3.
 * - buttonFuncLevel2_2_4: Button function for level 2.2.4.
 * - buttonFuncLevel2_2_5: Button function for level 2.2.5.
 * - buttonFuncLevel2_3_0: Button function for level 2.3.0.
 * - buttonFuncLevel2_4_0: Button function for level 2.4.0.
 * - buttonFuncLevel2_5_0: Button function for level 2.5.0.
 * - buttonFuncLevel2_5_1: Button function for level 2.5.1.
 * - buttonFuncLevel2_5_2: Button function for level 2.5.2.
 * - buttonFuncLevel2_5_3: Button function for level 2.5.3.
 * - buttonFuncLevel2_5_4: Button function for level 2.5.4.
 * - encoderSetTimeWattering: Set time for watering using encoder.
 * 
 * Inline Functions:
 * - n0gpioRead: Read GPIO value as a string ("1" or "0").
 * - n1gpioRead: Read GPIO value as a string with one leading space (" 1" or " 0").
 * - ngpioRead: Read GPIO value as a string with two leading spaces ("  1" or "  0").
 * - ugpioRead: Read GPIO value as an unsigned integer (1 or 0).
 * - gpioSRead: Read combined GPIO value with O_TRANSFO as a string with two leading spaces ("  1" or "  0").
 * - ugpioSRead: Read combined GPIO value with O_TRANSFO as an unsigned integer (1 or 0).
 */
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
extern boolean irSendPacOff;
extern boolean vmcFast;

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
