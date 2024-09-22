#include "logs.h"
#include "io.h"

//----------------------------------------------------------
// Mise   jour des logs
//----------------------------------------------------------
void logsUpdate() {
  boolean outputState;
  // Commande de la PAC
  static boolean flagP = true;
  static boolean flagNP;
  outputState = !gpioState(O_PAC);
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
  static boolean flagE;
  static boolean flagNE = true;
  outputState = gpioState(O_EV_EST);
  if (outputState && !flagE) {
    flagE = true;
    flagNE = false;
#ifdef DEBUG_OUTPUT_LOOP2
    Serial.printf("\n%02d:%02d EV EST sous tension\n", h, m);
#endif
    writeLogs("Irrigation façade SUD");
  }
  else if (!outputState && !flagNE) {
    flagE = false;
    flagNE = true;
#ifdef DEBUG_OUTPUT_LOOP2
    Serial.printf("\n%02d:%02d EV EST hors tension\n", h, m);
#endif
    writeLogs("Fin irrigation façade SUD");
  }

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