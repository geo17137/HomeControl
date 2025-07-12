/*
   Écriture/lecture d'un fichier (LITTLEFS) sur ESP8266
*/
#include <Arduino.h>
#ifndef  __FILE_H
#include "files.h"

#define FORMAT_LITTLEFS_IF_FAILED true
/**
 * @brief Construct a new File Little F S:: File Little F S object
 * 
 * @param name 
 */
FileLittleFS::FileLittleFS(const char *name) {
  if (!LittleFS_.begin(FORMAT_LITTLEFS_IF_FAILED)) {
    Serial.println("LITTLEFS Mount Failed");
    return;
  }
  strcpy(path, name);
}
/**
 * @brief Ouvre un fichier existant
 * 
 * @param mode 
 * @return true fichier ouvert
 * @return false 
 */
bool FileLittleFS::open(const char *mode) {
  file = LittleFS_.open(F(path), mode);
  return file != 0;
}
/**
 * @brief Ecrit un fichier text
 * 
 * @param message (char*) text à mémoriser
 * @param mode "w"
 * @return boolean 
 */
boolean FileLittleFS::writeFile(const char *message, const char* mode) {
  file = LittleFS_.open(F(path), mode);
  if (!file.print(message)) {
    Serial.println("Erreur ecriture");
  }
  file.close();
  return file != 0;
}
/**
 * @brief Ecrit un fichier text
 * 
 * @param message (String) text à mémoriser
 * @param mode "w"
 * @return boolean 
 */
boolean FileLittleFS::writeFile(String message, const char *mode) {
  file = LittleFS_.open(F(path), mode);
  if (!file.print(message)) {
    Serial.println("Erreur ecriture");
  }
  file.close();
  return file != 0;
}

// Liste des fichiers présents
// void FileLittleFS::listDir() {
//
// }

/**
 * @brief lecture d'un fichier
 * 
 * @return String 
 */
String FileLittleFS::readFile() {
  file = LittleFS_.open(F(path), "r");
  return file.readString();
}

/**
 * @brief Lit une chaine et la mémorise dans buffer
 * @param buffer de sortie
 */
void FileLittleFS::readFile(char* buffer) {
  file = LittleFS_.open(F(path), "r");
  String content = file.readString();
  strcpy(buffer, content.c_str());
}


boolean FileLittleFS::exist(const char *name) {
  return LittleFS_.exists(name);
}

boolean FileLittleFS::exist() {
  return LittleFS_.exists(path);
}

void FileLittleFS::rmFile() {
  LittleFS_.remove(path);
}

void FileLittleFS::rmFile(const char *path) {
  LittleFS_.remove(path);
}

void FileLittleFS::rmDir(const char *dir) {
  LittleFS_.rmdir(dir);
}

void FileLittleFS::mkDir(const char *dirName) {
  LittleFS_.mkdir(dirName);
}

void FileLittleFS::close() {
  file.close();
}


size_t FileLittleFS::fileSize() {
  auto file = LittleFS_.open(path, "r");
  size_t filesize = file.size();
  // Don't forget to clean up!
  file.close();
  return filesize;
}



#endif