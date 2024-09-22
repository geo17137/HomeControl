/*
  Classe permettant d'extraire les paramètres temporels
  de la chaine de caractères param
*/
#ifndef PARAM_H
#define PARAM_H
#include <arduino.h>
#include "const.h"

class ItemParam {
  public:
  int enable;
  int HMin;
  int MMin;
  int HMax;
  int MMax;

public:
  ItemParam() {
  }
  void print();
};

class Param {
  ItemParam _param[N_PARAM];
  // Taille de la chaine de caractère contenant les paramètres
  char _sparam[(TAILLE_PLAGE * N_PLAGES * N_DEVICES) + 2];

public:
  Param(const char* param);
  ItemParam get(int nParam, int timeSet);
  void set(int nParam, int timeSet, ItemParam ItemParam);
  char* getStr();
  void  setStr(const char* str);
  void setParam();
  void updateStringParam(char* param);
  void print();
};

#endif