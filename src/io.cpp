
/**
 * @file io.cpp
 * @brief Implementation of I/O functions for ESP32 or ES32A08 board.
 * 
 * This file contains functions to handle input and output operations
 * either directly on an ESP32 or via shift registers on the ES32A08 board.
 * 
 * The functions include:
 * - readByteInput: Reads 8 opto-isolated inputs from the ES32A08 board.
 * - put74HC595: Writes an 8-bit field to the relay outputs.
 * - clear74HC595: Resets the shift registers.
 * - writeBitOutput: Writes a bit to a relay output.
 * - readBitsInput: Reads a specified input bit.
 * 
 * The ES32A08 board uses bits 16-23 for output values and bits 0-7 for input data.
 * 
 * @note The code is conditionally compiled with the ES32A08 macro.
 * 
 * @see io.h
 */
#include "io.h"

#ifdef ES32A08

// Les bits 16..23 contiennent les valeurs en sortie
// Les bits 0..7 contiennent les données en entrée.
static unsigned fieldBitIO;

// Valeur à écrire dans les regsitres à décalage
static unsigned outputShiftedValue;

/**
 * @brief Lit les 8 entrées opto-isolée sur
 * la carte ES32A8. 
 * Contact fermé = 1 dans le champ de 8 bits
 * @return unsigned  
 */
unsigned readByteInput() {
  unsigned data = 0;
  digitalWrite(LOAD_165, LOW);
  digitalWrite(LOAD_165, HIGH);
  for (int i = 0; i < 8; i++) {
    data = (data << 1) | (digitalRead(DATA_165));
    digitalWrite(CLK_165, HIGH);
    digitalWrite(CLK_165, LOW);
  }
  fieldBitIO = fieldBitIO & 0xFFFF00;
  // Contact fermé = 0 sur la sortie de l'optocoupleur
  fieldBitIO = fieldBitIO | (~data & 0xff);
  return fieldBitIO;
}

/**
 * @brief Ecrit un champ de 8 bits
 * sur les sorties commandant les relais
 * Les sorties relais sont sur les bits
 * 16..23 (trois registres à décalage en série)
 * @param value valeur sur 8 bits à ecrire
 */
void put74HC595(unsigned value) {
  unsigned mask = 0x800000;
  for (int i = 0; i < 24; i++) {
    bool bit = value & mask;
    mask >>= 1;
    digitalWrite(DATA_955, bit ? 1 : 0);
#ifdef IO_DEBUG    
    if (i % 8 == 0)
      Serial.print(' ');
    Serial.print(bit ? 1 : 0);
#endif    
    digitalWrite(CLOCK_955, 1);
    digitalWrite(CLOCK_955, 0);
  }
  digitalWrite(LATCH_955, 1);
  digitalWrite(LATCH_955, 0);
#ifdef IO_DEBUG 
  Serial.println();
#endif  
}

/**
 * @brief Reset des registres à décalage
 */
void clear74HC595() {
  for (int i = 0; i < 24; i++) {
    digitalWrite(DATA_955, 0);
    digitalWrite(CLOCK_955, 1);
    digitalWrite(CLOCK_955, 0);
  }
  digitalWrite(LATCH_955, 1);
  digitalWrite(LATCH_955, 0);
}

/**
 * @brief Ecrit un bit sur une sortie
 * commandant les relais
 * Le masque isole le bit 
 * Exemple 
 * 0x010000 = bit 0 (relais 1)
 * 0x800000 = bit 7 (relais 8)
 * @param mask  spécifie le bit 
 * @param value 0 ou 1
 */
void writeBitOutput(unsigned mask, int value) {
  if (value != 0)
    fieldBitIO = fieldBitIO | mask;
  else
    fieldBitIO = fieldBitIO & ~mask;
  // Isoler les 8 bits de poids forts
  // Les 8 bits de poids faible contient l'état des entrées
  unsigned outputShiftedValue_1 = fieldBitIO & 0x0FF0000;
  if (outputShiftedValue_1 != outputShiftedValue) {
    outputShiftedValue = outputShiftedValue_1;
    put74HC595(outputShiftedValue);
  }
}
/**
 * @brief Lit un bit d'entrée spécifiée 
 * par son masque. Exemple :
 * Bit 0 masque 0x01
 * Bit 7 masque 0x80
 * @param mask 
 * @return boolean valeur du bit
 */
boolean readBitsInput(unsigned mask) {
  return (mask <= 0x80) ? readByteInput() & mask : fieldBitIO & mask;
}
#endif