
#include "simple_param.h"

void SimpleParam::split() {
  // !!!! strok modifie la chaine source
  strcpy(_buffer, _sparam);
  char* ptr;
  int count = 0;
  ptr = strtok(_buffer, _motif); // diviser la chaîne
  while (ptr != NULL) {
    _param[count] = atoi(ptr);
    count++;
    ptr = strtok(NULL, ":");
    if (count == _nparam) {
      break;
    }
  }
}

SimpleParam::SimpleParam(const char* initStr, const char* motif, unsigned nParam) {
  _nparam = nParam;
  _motif =  (char*)malloc(strlen(motif) + 1);
  _sparam = (char*)malloc(strlen(initStr) + 1);
  _buffer = (char*)malloc(strlen(initStr) + 1);
  _param  = (int*)malloc((sizeof(int) * nParam));
  strcpy(_motif, motif);
  strcpy(_sparam, initStr);
  split();
}

SimpleParam::~SimpleParam() {
  free(_motif);
  free(_sparam);
  free(_buffer);
  free(_param);
  _motif = NULL;
  _sparam = NULL;
  _buffer = NULL;
  _param = NULL; 
}

int SimpleParam::get(int numParam) {
  return _param[numParam];
}

void SimpleParam::set(int numParam, int val) {
  _param[numParam] = val;
  update();
}

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

char* SimpleParam::getStr() {
  return _sparam;
}

void  SimpleParam::setStr(const char* str) {
  strcpy(_sparam, str);
  split();
}

void SimpleParam::print() {
  Serial.println(_sparam);
  Serial.print(_param[0]);
  for (int i = 1; i < _nparam; i++) {
    Serial.print(',');
    Serial.print(_param[i]);
  }
  Serial.println();
}