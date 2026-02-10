

/**
 * @file simple_param.cpp
 * @brief Implementation of the SimpleParam class for handling and manipulating parameter strings.
 */

#include "simple_param.h"

/**
 * @brief Splits the parameter string _sparam into individual integer parameters.
 * 
 * This function modifies the source string by using strtok to tokenize the string
 * based on the specified motif. The tokens are then converted to integers and stored
 * in the _param array.
 */
void SimpleParam::split() {
  // !!!! strok modifie la chaine source
  strcpy(_buffer, _sparam);
  char* ptr;
  int count = 0;
  ptr = strtok(_buffer, _motif); // diviser la chaîne
  while (ptr != NULL) {
    _param[count] = atoi(ptr);
    count++;
    ptr = strtok(NULL, _motif);
    if (count == _nparam) {
      break;
    }
  }
}

/**
 * @brief Constructs a SimpleParam object.
 * 
 * @param initStr Initial parameter string.
 * @param motif Delimiter used to split the parameter string.
 * @param nParam Number of parameters expected.
 * 
 * This constructor initializes the SimpleParam object by allocating memory for the
 * motif, parameter string, buffer, and parameter array. It then copies the initial
 * strings and splits the parameter string into individual parameters.
 */
SimpleParam::SimpleParam(const char* initStr, const char* motif, unsigned nParam) {
  _nparam = nParam;
  _motif = new char[strlen(motif) + 1];
  _sparam = new char[strlen(initStr) + 1];
  _buffer = new char[strlen(initStr) + 1];
  _param = new int[nParam];

 // _motif =  (char*)malloc(strlen(motif) + 1);
 // _sparam = (char*)malloc(strlen(initStr) + 1);
 // _buffer = (char*)malloc(strlen(initStr) + 1);
 // _param  = (int*)malloc((sizeof(int) * nParam));
  strcpy(_motif, motif);
  strcpy(_sparam, initStr);
  split();
}

/**
 * @brief Destructor for the SimpleParam class.
 * 
 * This destructor frees the allocated memory for the motif, parameter string, buffer,
 * and parameter array, and sets the pointers to NULL.
 */
SimpleParam::~SimpleParam() {
  delete  _motif;
  delete  _sparam;
  delete  _buffer;
  delete  _param;
 // free(_motif);
 // free(_sparam);
 // free(_buffer);
 // free(_param);
  _motif = NULL;
  _sparam = NULL;
  _buffer = NULL;
  _param = NULL; 
}

/**
 * @brief Gets the value of a specific parameter.
 * 
 * @param numParam Index of the parameter to retrieve.
 * @return int Value of the specified parameter.
 */
int SimpleParam::get(int numParam) {
  return _param[numParam];
}
/**
 * @brief Sets the value of a specific parameter.
 * 
 * @param numParam Index of the parameter to set.
 * @param val Value to set the parameter to.
 * 
 * This function sets the value of the specified parameter and updates the parameter
 * string to reflect the new value.
 */
void SimpleParam::set(int numParam, int val) {
  _param[numParam] = val;
  update();
}
/**
 * @brief Updates the parameter string based on the current parameter values.
 * 
 * @return int Always returns 0.
 * 
 * This function constructs the parameter string by concatenating the individual
 * parameter values, separated by the motif.
 */
int SimpleParam::update() {
  char bufferItoa[6];
  unsigned i = 0;
  _buffer[0] = '\0';
  for (; i < _nparam - 1; i++) {
    strcat(_buffer, itoa(get(i), bufferItoa, 10));
    strcat(_buffer, ":");
  }
  strcat(_buffer, itoa(get(i), bufferItoa, 10));
  // Serial.println(buffer);
  strcpy(_sparam, _buffer);
  return 0;
}
/**
 * @brief Gets the parameter string.
 * 
 * @return char* Pointer to the parameter string.
 */
char* SimpleParam::getStr() {
  return _sparam;
}
/**
 * @brief Sets the parameter string.
 * 
 * @param str New parameter string.
 * 
 * This function sets the parameter string to the specified value and splits it into
 * individual parameters.
 */
void  SimpleParam::setStr(const char* str) {
  strcpy(_sparam, str);
  split();
}
/**
 * @brief Prints the parameter string and individual parameters to the Serial.
 * 
 * This function prints the parameter string followed by the individual parameter
 * values, separated by commas.
 */
void SimpleParam::print() {
  //Serial.println(_sparam);
  Serial.print(_param[0]);
  for (int i = 1; i < _nparam; i++) {
    Serial.print(',');
    Serial.print(_param[i]);
  }
  Serial.println();
}