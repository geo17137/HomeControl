#ifndef DISPLAY_H
#define DISPLAY_H
#include <Arduino.h>
#include "const.h"
#include <LiquidCrystal_I2C.h>
#include "mtr86.h"

#define ROW1_1_2        "K1 K2 K3 K4 S6 S7 S8"
// #define ROW1_1_2        "  1 2 3 4 5 6 7 8"
#define TO_PROG         "To prog push rotary"
#define PROG_COOKING    "Prog cooking device"
#define TO_FONC         "-PROG: push rotary"
#define SURPRESSOR_OFF  "---SURPRESSOR OFF---"

extern boolean isLcdDisplayOn;
extern boolean bootDisplayOff;

extern LiquidCrystal_I2C lcd;
// extern const char* lines[4];
extern void (*onSingleClick)();
extern void buttonFuncLevel0();
extern void (*funcToCall)();
extern void (*funcToCall2)();
extern void loopTicParam();
extern char* readPortIo_O();
extern char* readPortIo_I();
extern void printMqttDebugValue(const char* value);

// extern void loopTic(void (*func)());
// extern void loopTic2(void (*func)());

extern TACHE_T tache_t_backLight;
extern TACHE_T tache_t_defaultDisplay;

void lcdClear();
void displayPrint1(const char* output, const char* input, const char* msg);
void lcdPrintString(const char* text, int l, int c, boolean clearLine);
void lcdPrintChar(char ch, int l, int c);
void lcdPrintRssi(char* rssi_buffer);
#endif