/**
 * @file display.cpp
 * @brief This file contains functions to manage the display of an ESP32-based home control system.
 * 
 * The functions include initialization of the display, clearing the display, controlling the backlight,
 * printing strings and characters to the display, and displaying RSSI values.
 * 
 * @details
 * - initDisplay: Initializes the display and creates custom characters.
 * - lcdClear: Clears the display.
 * - backLightOn: Turns on the backlight with a timer.
 * - backLightOff: Turns off the backlight.
 * - lcdPrintString: Prints a string to the display at a specified position.
 * - lcdPrintChar: Prints a character to the display at a specified position.
 * - lcdPrintRssi: Displays the RSSI value on the display.
 * - ioDisplay: Displays default I/O information.
 * - displayPrint1: Displays formatted output, input, and a message on the display.
 * 
 * @note The display is a 20x4 character LCD.
 * @note The backlight control is conditional based on the FORCE_DISPLAY macro.
 * @note The RSSI symbol is a custom character defined in rssi_char.
 * 
 * @dependencies
 * - Arduino.h
 * - display.h
 */
#include <Arduino.h>
#include "display.h"

/**
 * @brief Symbole graphique RSSI
 */
byte rssi_char[8] = {
  B00000,
  B00001,
  B00001,
  B00101,
  B00101,
  B10101,
  B10101,
};

extern char rssi_buffer[10];

void initDisplay() {
  Wire.begin(I2C_SDA, I2C_SCL);
  // initialize LCD
  lcd.createChar(0, rssi_char);   
  lcd.begin(COLUMS, ROWS, LCD_5x8DOTS);  
}

void lcdClear() {
  lcd.clear();
}
/**
 * @brief Active le rétro eclairage (temporisé)
 * 
 */
void backLightOn() {
  isLcdDisplayOn = true;
  lcd.displayOn(); // displayOn set backLight
  t_start(tache_t_backLight);
}

/**
 * @brief Coupe le rétro eclairage
 */
void backLightOff() {
#ifndef FORCE_DISPLAY
  isLcdDisplayOn = false;
  lcd.displayOff(); // dispplayOn set noBacklight
#endif  
}

/**
 * @brief Affiche une chaine sur l'écran 20x4
 * @param text texte à afficher
 * @param l ligne
 * @param c colonne
 * @param clearLine true = ligne effcée au préalable
 */
void lcdPrintString(const char* text, int l, int c, boolean clearLine) {
  if (bootDisplayOff)
      lcd.displayOff();
  if (clearLine) {
    lcd.setCursor(0, l);
    lcd.print("                    ");
  }
  lcd.setCursor(c, l);
  lcd.print(text);
}

/**
 * @brief Affiche un caractère sur l'écran 20x4
 * @param ch caractère à afficher
 * @param l ligne
 * @param c colonne
 */
void lcdPrintChar(char ch, int l, int c) {
  lcd.setCursor(c, l);
  lcd.print(ch);
}
/**
 * @brief Affiche le RSSI
 * @param rssi_buffer 
 */
void lcdPrintRssi(char *rssi_buffer) {
  lcd.setCursor(RSSI_COLUMS_POS, RSSI_ROWS_POS);
  lcd.print(rssi_buffer);
  lcd.setCursor(RSSI_COLUMS_POS-1, RSSI_ROWS_POS);
  lcd.write(byte(0));
}

/**
 * @brief  Affichage lcd par défaut (hors isr)
 */
void ioDisplay() {
  displayPrint1(readPortIo_O(), readPortIo_I(), TO_FONC);
}

/**
 * @brief Affichage par défaut
 *        Déclenchement du paramétrage local par appuy sur le bouton rotary
 * @param output données formatée sur les ports de sortie
 * @param input  données formatée sur les ports de d'entrée
 * @param msg message signalant la commande disponible 
 */
void displayPrint1(const char* output, const char* input, const char* msg) {
  lcd.clear();
  backLightOn();
  // t_start(tache_t_backLight);
  lcdPrintString(output, 2, 0, true);
  lcdPrintString(ROW1_1_2, 1, 0, true);
  lcdPrintString(msg, 3, 0, true);
  lcdPrintString(input, 0, 1, true);
  lcdPrintRssi(&rssi_buffer[5]);
  funcToCall = buttonFuncLevel0;
  onSingleClick = loopTicParam;
}