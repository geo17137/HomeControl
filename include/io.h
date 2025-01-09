#ifndef __IO_H
#define __IO_H
#include <ArduinoOTA.h>
#include "const.h"

// Entrées/sorties compatibles avec E/S directe sur les ports GPIO
// et E/S via les registres à décalage de la carte ES32A08
// si ES32A08 est défini

extern boolean irSendPacOff;
unsigned readByteInput();
boolean readBitsInput(unsigned mask);

void writeBitOutput(unsigned num, int value);
void clear74HC595();

/**
 * @brief Lit un port E/S
 * gpio : numéro port E/S ou masque si ES32A08
 * @param gpio 
 * @return int valeur du bit
 */
inline int digital_read(int gpio) {
#ifdef ES32A08  
  return readBitsInput(gpio) ? 0 : 1;
#else  
  return digitalRead(gpio);
#endif 
}
/**
 * @brief Ecrit un port E/S
 * gpio : numéro port E/S ou masque si ES32A08
 * @param gpio 
 * @param val valeur du bit
 */
inline void digital_write(int gpio, unsigned val) {
#ifdef ES32A08   
  writeBitOutput(gpio, val);
#else
  digitalWrite(gpio, val);
#endif  
}

/**
 * @brief Résultat chaine lecture port GPIO 
 * 
 * @param gpio 
 * @return const char* 
 */
inline const char* gpioRead(int gpio) {
  return digital_read(gpio) == 0 ? "0" : "1";
}

/**
 * @brief Lecture pac pour savoir si envoi ir en cours
 * 
 * @return const char* 
 */
inline const char* gpioReadPac() {
  if (digital_read(O_PAC) == 0)
    return irSendPacOff ? "2" : "1";
  return "0";
}

/**
 * @brief Coupure relais
 * adr : numéro du port E/S ou masque
 * si ES32A08
 * @param adr 
 */
inline void off(int adr) {
#ifdef ES32A08  
  digital_write(adr, 0);
#else  
  digital_write(adr, 1);
#endif
}
/**
 * @brief Alimentation relais
 * adr : numéro du port E/S ou masque
 * si ES32A08
 * @param adr 
 */
inline void on(int adr) {
  // time_exec_start();
#ifdef ES32A08  
  digital_write(adr, 1);
#else  
  digital_write(adr, 0);
#endif
  // time_exec_stop();
}
/**
 * @brief Etat ports gpio relais (true relai fermé)
 * gpio : numéro port E/S ou masque si ES32A08
 * @param gpio 
 * @return boolean true si 1
 */
inline boolean gpioState(int gpio) {
  return digital_read(gpio) == 0;
}
#endif