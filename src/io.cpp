#include "io.h"

// Fonctions permettant d'utiliser indifféremment les E/S gpio
// directes sur un ESP32 ou utiliser les E/S via les registres
// à décalage de la carte ES23A08

#ifdef ES32A08

// Les bits 16..23 contiennent les valeurs en sortie
// Les bits 0..7 contiennent les données en entrée
static unsigned fieldBitIO;

// Valeur à écrire dans les regsitres à décalage
static unsigned outputShiftedValue;

/**
 * @brief Lit les 8 entrées optoisolée sur
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
 * @brief Ecrit un champ de 8bits
 * sur les sorties commandant les relais
 * Les sorties relais sont sur les les bits
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
 * @brief Reset les registres à décalage
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
  unsigned outputShiftedValue_1;
  if (value != 0)
    fieldBitIO = fieldBitIO | mask;
  else
    fieldBitIO = fieldBitIO & ~mask;
  // Isoler les 8 bits de poids forts
  // Les 8 bits de poids faible contient l'état des entrées
  outputShiftedValue_1 = fieldBitIO & 0x0FF0000;
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