/**
 * @file logs.cpp
 * @brief Implementation of log updates based on the state of GPIO ports.
 * 
 * This file contains the implementation of the logsUpdate function, which updates
 * the logs based on the state of various GPIO ports. The function checks the state
 * of each port and logs messages when the state changes.
 * 
 * The following GPIO ports are monitored:
 * - O_PAC: Power control for the PAC (Pump and Compressor)
 * - O_EV_EST: Valve control for the EST (East) irrigation
 * - O_FOUR: Power control for the oven (Electroménager)
 * - O_VMC: Power control for the VMC (Ventilation)
 * - O_EV_ARROSAGE: Valve control for the arrosage (watering)
 * - O_EV_IRRIGATION: Valve control for the irrigation
 * 
 * The function uses static boolean flags to keep track of the previous state of each
 * port and logs messages when the state changes from off to on or from on to off.
 * 
 * The log messages are written using the writeLogs function, and debug messages are
 * printed to the Serial output if DEBUG_OUTPUT_LOOP2 is defined.
 * 
 * @note The function assumes that the gpioState function is available to get the
 *       current state of a GPIO port, and the writeLogs function is available to
 *       write log messages.
 */
#include "logs.h"
#include "io.h"

//----------------------------------------------------------
// Mise à jour des logs
// Basé sur l'état des ports gpio
//----------------------------------------------------------
void logsUpdate() {
    // Commande de la PAC
  static boolean flagP = true;
  static boolean flagNP;
  boolean outputState = !gpioState(O_PAC);
  if (outputState && !flagP) {
    flagP = true;
    flagNP = false;
#ifdef DEBUG_OUTPUT_LOOP2
    Serial.printf("\n%02d:%02d PAC sous tension\n", h, m);
#endif
    writeLogs("PAC sous tension");
  }
  else if (!outputState && !flagNP) {
    flagP = false;
    flagNP = true;
#ifdef DEBUG_OUTPUT_LOOP2
    Serial.printf("\n%02d:%02d PAC hors tension\n", h, m);
#endif
    writeLogs("PAC hors tension");
  }

  // Vanne EST
//   static boolean flagE;
//   static boolean flagNE = true;
//   outputState = gpioState(O_EV_EST);
//   if (outputState && !flagE) {
//     flagE = true;
//     flagNE = false;
// #ifdef DEBUG_OUTPUT_LOOP2
//     Serial.printf("\n%02d:%02d EV EST sous tension\n", h, m);
// #endif
//     writeLogs("Irrigation façade SUD");
//   }
//   else if (!outputState && !flagNE) {
//     flagE = false;
//     flagNE = true;
// #ifdef DEBUG_OUTPUT_LOOP2
//     Serial.printf("\n%02d:%02d EV EST hors tension\n", h, m);
// #endif
//     writeLogs("Fin irrigation façade SUD");
//   }

  // Electroménager
  static boolean flagF;
  static boolean flagNF = true;
  outputState = gpioState(O_FOUR);
  if (outputState && !flagF) {
    flagF = true;
    flagNF = false;
#ifdef DEBUG_OUTPUT_LOOP2
    Serial.printf("\n%02d:%02d App. cuisson sous tension\n", h, m);
#endif
    writeLogs("App. cuisson sous tension");
  }
  else if (!outputState && !flagNF) {
    flagF = false;
    flagNF = true;
#ifdef DEBUG_OUTPUT_LOOP2
    Serial.printf("\n%02d:%02d App. cuisson sous tension\n", h, m);
#endif
    writeLogs("App. cuisson hors tension");
  }

  // VMC
  static boolean flagV;
  static boolean flagNV = true;
  outputState = gpioState(O_VMC);
  if (outputState && !flagV) {
    flagV = true;
    flagNV = false;
#ifdef DEBUG_OUTPUT_LOOP2
    Serial.printf("\n%02d:%02d VMC sous tension\n", h, m);
#endif
    writeLogs("VMC sous tension");
  }
  else if (!outputState && !flagNV) {
    flagV = false;
    flagNV = true;
#ifdef DEBUG_OUTPUT_LOOP2
    Serial.printf("\n%02d:%02d VMC hors tension\n", h, m);
#endif
    writeLogs("VMC hors tension");
  }

  // Arrosage
  static boolean flagA;
  static boolean flagNA = true;
  outputState = gpioState(O_EV_ARROSAGE);
  if (outputState && !flagA) {
    flagA = true;
    flagNA = false;
#ifdef DEBUG_OUTPUT_LOOP2
    Serial.printf("\n%02d:%02d Arrosage\n", h, m);
#endif
    writeLogs("Arrosage en cours");
  }
  else if (!outputState && !flagNA) {
    flagA = false;
    flagNA = true;
#ifdef DEBUG_OUTPUT_LOOP2
    Serial.printf("\n%02d:%02d Fin arrosage\n", h, m);
#endif
    writeLogs("Fin arrosage");
  }

  // Irrigation
  static boolean flagR;
  static boolean flagNR = true;
  outputState = gpioState(O_EV_IRRIGATION);
  if (outputState && !flagR) {
    flagR = true;
    flagNR = false;
#ifdef DEBUG_OUTPUT_LOOP2
    Serial.printf("\n%02d:%02d Remplissage réservoir\n", h, m);
#endif
    writeLogs("Remplissage réservoir");
  }
  else if (!outputState && !flagNR) {
    flagR = false;
    flagNR = true;
#ifdef DEBUG_OUTPUT_LOOP2
    Serial.printf("\n%02d:%02d Arrêt remplissage réservoir\n", h, m);
#endif
    writeLogs("Arrêt remplissage");
  }
}