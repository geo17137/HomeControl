/**
 * @file io.h
 * @brief Header file for GPIO input/output operations compatible with direct GPIO port access and shift register operations on the ES32A08 board.
 * 
 * This file provides functions to read and write GPIO ports, handle relay states, and manage the shift register on the ES32A08 board.
 * 
 * @details
 * - If ES32A08 is defined, the functions will operate on the shift register.
 * - If ES32A08 is not defined, the functions will operate on the direct GPIO ports.
 * 
 * @note The ES32A08 board uses a shift register for GPIO operations.
 * 
 * @author Daniel
 * @date October 2023
 */

#ifndef __IO_H
#define __IO_H
#include <ArduinoOTA.h>
#include "const.h"

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
 *  GIOP PORT 0 : PAC arretée 
 *  GIOP PORT 1 : PAC sous tension 
 *  GIOP PORT 2 : PAC en cours d'arrèt 
 * @return const char* 
 */
inline const char* gpioReadPac() {
  if (digital_read(O_PAC) == 0)
    return irSendPacOff ? "2" : "1";
  return "0";
}

/**
 * @brief Coupure relais
 * adr : numéro du port E/S ou masque si ES32A08
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
 * adr : numéro du port E/S ou masque si ES32A08
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