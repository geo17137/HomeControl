/**
 * @file simple_param.h
 * @brief Header file for the SimpleParam class.
 * 
 * This file contains the definition of the SimpleParam class, which is used to manage parameters
 * represented in the form "param1<motif>param2<motif>...".
 * 
 * @note param = int, motif = string
 */

/**
 * @class SimpleParam
 * @brief A class to manage parameters represented in a specific format.
 * 
 * The SimpleParam class allows for the management of parameters represented in the form
 * "param1<motif>param2<motif>...". It provides methods to get and set individual parameters,
 * as well as to retrieve and update the entire parameter string.
 * 
 * @note param = int, motif = string
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