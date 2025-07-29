#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H
#include "AiEsp32RotaryEncoder.h"
#include "OneButton.h"
#include "const.h"

#ifdef ES32A08
#define ROTARY_ENCODER_A_PIN 19
#define ROTARY_ENCODER_B_PIN 18
#define ROTARY_ENCODER_BUTTON_PIN 23
#else
#define ROTARY_ENCODER_A_PIN 39
#define ROTARY_ENCODER_B_PIN 34
#define ROTARY_ENCODER_BUTTON_PIN 36
#endif
#define ROTARY_ENCODER_VCC_PIN -1
#define ROTARY_ENCODER_STEPS 4

extern AiEsp32RotaryEncoder rotaryEncoder;
extern OneButton button;
extern void (*foncToCall)();
extern void (*onSingleClick)();
extern void (*onDoubleClick)();
extern void (*rotaryTask)();

// (ROTARY_ENCODER_BUTTON_PIN, true);
void IRAM_ATTR readEncoderISR();
void rotary_loop();
void tick();
void initRotary();
void singleClick();
void doubleClick();
void multiClick();
void pressStart();
void pressStop();
void on_singleClick();
void on_doubleClick();
void on_rotary(long value);
void setBoundaries(int min, int max);
void setEncoder(int val);
#endif


