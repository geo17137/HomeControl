#include "param.h"

void ItemParam::print() {
  static char temp[48];
  sprintf(temp,"%d:%02d:%02d:%02d:%02d\n",enable,HMin,MMin,HMax,MMax);
  Serial.print(temp);
}

Param::Param(const char* param) {
  strcpy(_sparam, param);
  setParam();
}

/**
 *@brief 
 * 
 * @param numParam  0..4
 * @param timeSet  0..4
 * @return ItemParam item de la forme 0:00:00:00:00
 */
ItemParam Param::get(int numParam, int timeSet) {
  return _param[(numParam * N_PLAGES) + timeSet];
}

/**
 *@brief 
 * 
 * @param numParam 0..4 
 * @param timeSet  0..4
 * @param itemParam item de la forme 0:00:00:00:00
 */
void Param::set(int numParam, int timeSet, ItemParam itemParam) {
  _param[(numParam * N_PLAGES) + timeSet] = itemParam;
}

/*
  Construit le tableau d'ItemParam
  A partir de sa repésentaion sous forme de chaines
  0:00:00:00:00:0:00:00:00:00:0:00:00:00:00...
  <---Plage---> <---Plage---> <---Plage--->...
*/
void Param::setParam() {
  String str(_sparam);
  int index = -1;
  for (int i = 0; i < N_PARAM; i++) {
    str = str.substring(index + 1);
    index = str.indexOf(':');
    _param[i].enable = atoi(str.substring(0,index).c_str());
    str = str.substring(index + 1);
    index = str.indexOf(':');
    _param[i].HMin = atoi(str.substring(0,index).c_str());
    str = str.substring(index + 1);
    index = str.indexOf(':');
    _param[i].MMin = atoi(str.substring(0,index).c_str());
    str = str.substring(index + 1);
    index = str.indexOf(':');
    _param[i].HMax = atoi(str.substring(0,index).c_str());
    str = str.substring(index + 1);
    index = str.indexOf(':');
    _param[i].MMax = atoi(str.substring(0,index).c_str());
  }
}

/**
 *@brief Initialise la chaine des paramètres à partir d'une autre chaine
         Met à jour l'objet
 *
 * @param str chaine d'init
 */
void  Param::setStr(const char* str) {
  strcpy(_sparam, str);
  setParam();
}
/**
 *@brief Met à jour la chaine param avec le contenu de l'objet correspond
 *       Utilisation updateStringParam(cParam->getStr())
 *
 * @param param chaine à initialiser
 */
void Param::updateStringParam(char* param) {
  char buffer[60];
  ItemParam itemParam;
  param[0] = '\0';
  for (int i = 0; i < N_DEVICES; i++) {
    for (int j = 0; j < N_PLAGES; j++) {
      itemParam = get(i,j);
      sprintf(buffer,"%d:%02d:%02d:%02d:%02d:",
        itemParam.enable,
        itemParam.HMin,
        itemParam.MMin,
        itemParam.HMax,
        itemParam.MMax);
      strcat(param,buffer);
    }
  }
  // Supprimer le ':' en fin de chaine
  *(param + strlen(param) - 1) = '\0';
}

/**
 *@brief Retourne la représention des paramètres sous forme de chaine
 * 
 * @return char* chaine des paramètres
 */
char* Param::getStr() {
  return _sparam;
}

void Param::print() {
  int j = 0;
  for (int i = 0; i < N_PARAM; i++) {
    if (i % N_PLAGES == 0)
      Serial.printf("------%d------\n", j++);
    _param[i].print();
  }
}
