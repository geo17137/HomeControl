/*
  Classe permettant de gérer les paramètres
  représentés sous la forme "param1<motif>param2<motif>...
  param = int
  motif = chaine de caractères
*/
#ifndef SIMPLE_PARAM_H
#define SIMPLE_PARAM_H
#include <Arduino.h>
#include "const.h"

class SimpleParam {
private:
  char* _motif;
  char* _sparam;
  char* _buffer;
  int*  _param;
  unsigned _nparam;

public:
  /**
   * @brief Construct a new Simple Param object
   * 
   * @param str chaine contenant les paramètres
   * @param motif chaine contenant le motif
   * @param nParam nombre de paramètres dans str
   */
  SimpleParam(const char* str, const char* motif, unsigned nParam);
  /**
   *@brief Get param
   * 
   * @param nParam numéro du paramètre
   * @return int valeur du paramtre
   */
  int get(int nParam);
  /**
   *@brief modifie un paramètre
   * 
   * @param nParam  numéro du paramètre
   * @param val   valeur du paramètre
   */
  void set(int nParam, int val);
  /**
   *@brief Get the Str param
   * 
   * @return char* 
   */
  char* getStr();
  /**
   *@brief Set the Str param
   * 
   * @param str str param
   */
  void  setStr(const char* str);
  void  print();
private:
  void  split();
  int   update();
};
#endif