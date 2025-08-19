/*
  Classe permettant d'extraire les paramètres temporels
  de la chaine de caractères param
*/
#ifndef PARAM_H
#define PARAM_H
#include <Arduino.h>
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
    HMax = HMin = MMax = MMin = enable = 0;
  }
  void print();
};

/**
 * @class Param
 * @brief A class to manage parameters and their string representation.
 * 
 * This class provides methods to get, set, and update parameters, as well as
 * to convert them to and from a string representation.
 */
class Param {
  ItemParam _param[N_PARAM]; ///< Array of parameters.
  char _sparam[(TAILLE_PLAGE * N_PLAGES * N_DEVICES) + 2]; ///< String containing the parameters.

public:
  /**
   * @brief Constructor that initializes the parameters from a string.
   * @param param A string containing the initial parameters.
   */
  Param(const char* param);

  /**
   * @brief Get a parameter at a specific index and time.
   * @param nParam The index of the parameter.
   * @param timeSet The time associated with the parameter.
   * @return The parameter at the specified index and time.
   */
  ItemParam get(int nParam, int timeSet);

  /**
   * @brief Set a parameter at a specific index and time.
   * @param nParam The index of the parameter.
   * @param timeSet The time associated with the parameter.
   * @param ItemParam The parameter to set.
   */
  void set(int nParam, int timeSet, ItemParam ItemParam);

  /**
   * @brief Get the string representation of the parameters.
   * @return A pointer to the string containing the parameters.
   */
  char* getStr();

  void printStrParam();

  /**
   * @brief Set the parameters from a string.
   * @param str A string containing the parameters.
   */
  void setStr(const char* str);

  /**
   * @brief Set the parameters.
   */
  void setParam();

  /**
   * @brief Update the string representation of the parameters.
   * @param param A string containing the new parameters.
   */
  void updateStringParam(char* param);

  /**
   * @brief Print the parameters.
   */
  void print();
};

#endif