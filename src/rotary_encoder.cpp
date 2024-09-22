#include "rotary_encoder.h"

/*
  Gestion de codeur rotatif et de son bouton poussoir
*/

void rotary_loop() {
  // dont print anything unless value changed
  if (rotaryEncoder.encoderChanged()) {
    on_rotary(rotaryEncoder.readEncoder());
  }
}

void IRAM_ATTR readEncoderISR() {
  rotaryEncoder.readEncoder_ISR();
}

unsigned long pressStartTime;

ICACHE_RAM_ATTR void checkTicks() {
  button.tick(); // just call tick() to check the state.
}

// this function will be called when the button was pressed 1 time only.
void singleClick() {
  // Serial.println("singleClick() detected.");
  // Fonction ISR : doit appeler des fonctions à exécution très courte
  onSingleClick();
} 

// this function will be called when the button was pressed 2 times in a short timeframe.
void doubleClick() {
  // Serial.println("doubleClick() detected.");
  // on_doubleClick();
  onDoubleClick();
} // doubleClick

// this function will be called when the button was pressed multiple times in a short timeframe.
// void multiClick() {
//   Serial.print("multiClick(");
//   Serial.print(button.getNumberClicks());
//   Serial.println(") detected.");
// } // multiClick

// this function will be called when the button was held down for 1 second or more.
// void pressStart() {
//   pressStartTime = millis() - 1000; // as set in setPressTicks()
// } // pressStart()

// this function will be called when the button was released after a long hold.
// void pressStop() {
//   Serial.print("pressStop(");
//   Serial.print(millis() - pressStartTime);
//   Serial.println(") detected.");
// } // pressStop()


void initRotary() {
  rotaryEncoder.begin();
  rotaryEncoder.setup(readEncoderISR);
  // Initialisation du bouton de l'encodeur rotatif
  // C'est OneButton qui gère ce bouton
  // OneButton button(ROTARY_ENCODER_BUTTON_PIN, true);
  // Gérer le bouton par interruption, si appelé le bouton est géré par scrutation
  attachInterrupt(digitalPinToInterrupt(ROTARY_ENCODER_BUTTON_PIN), checkTicks, CHANGE);
  button.attachClick(singleClick);
  button.attachDoubleClick(doubleClick);
  // button.attachMultiClick(multiClick);
  // button.setPressTicks(1000); // that is the time when LongPressStart is called
  // button.attachLongPressStart(pressStart);
  // button.attachLongPressStop(pressStop);
}

void setBoundaries(int min,int max) {
  rotaryEncoder.setBoundaries(min,max,true);
}

void setEncoder(int val) {
  rotaryEncoder.setEncoderValue(val);
}
