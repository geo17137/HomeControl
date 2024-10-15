#include "loop_prog.h"
#include "io.h"

/*
  L'IHM en local est réalisée à l'aide d'un afficheur LCD 20x4
  https://fr.aliexpress.com/w/wholesale-20x4-lcd-arduino.html
  équipé d'un interface I2C
  et d'un encodeur rotatif dont l'axe  vertical est possède un contact on/off permettant
  des commandes de type single clic ou double clic. La détection des clics et faite
  par interruption (ISR)

  L'interface lcd et le bouton rotary permettent de réaliser toutes les actions faites depuis
  un smartphone sauf la programmation des tâches horaires.

  Remarque générale :
  A chaque rotation du bouton rotary la fonction affectée au pointeur de fonction onRotary est appelé.
  A chaque appuy sur l'axe du bouton les fonctions affectées pointeurs de fonctions onSingleClick et onDoubleClic sont appelées
  - les fonctions appelées par le pointeur de fonction onRotary sont hors ISR
  - les fonctions appelées par les pointeurs de fonction onSingleClick et onDoubleClic sont dans une ISR

  Affichage des opérations et des états sur lcd display
  -----------------------------------------------------
  L'affichage sur l'écran LCD doit se faire dans un contexte d'execution hors ISR
  du fait de la communication avec l'interface de l'écran utilisant le protocole I2C.
  Les échanges avec ce protoloce sont sous interruptions.
  Ceci interdit d'utiliser l'afficheur LCD dans les fonctions déclanchées par un clic.

  Choix des opérations à exécuter affichées sur l'écran LCD:
  Les opérations sont choisies par un encodeur rotatif.
  A chaque rotation (changement de valeur de l'encodeur), on_rotary est appelé
  avec pour paramètre la valeur encodée dans la gamme setBoundaries(min, max)
  Cette valeur est mémorisée dans une variable globale rotary
  puis la tâche d'affichage a executer est appelée via le pointeur de fonction onRotary.
  initialisé sur la fonction à exécuter. Le contexte d'exécution est hors ISR (il fait parti de loop).

  Déclenchement des opérations à exécuter:
  Les opérations sont déclenchées par appuy sur le bouton de l'encodeur.
  Suivant le type d'appuy, les fonctions pointées par onSingleClick ou onDoubleClick sont appelées.
  Le contexte d'execution de ces fonctions est dans une ISR, interdisant l'utilisation directe de l'afficheur LCD.
  (à cause du protocole I2C sous ISR utilisé pour communiquer avec l'afficheur).
  Pour permettre l'utilisation de l'afficheur LCD il est necessaire d'effectuer un changement de contexte IRS->loop
  Le changement de contexte est effectué de façon suivante:
  Les fonctions appelées initialisent des pointeurs de fonction sur la tâche à exécuter, puis mettent en place
  un crochet appelé dans loop. 
  Ce crochet permet un changement de contexte et appele les fonctions via les pointeurs de fonctions initialisés
  dans l'ISR. 
  
  // Dans loop
      onLoopTic();  // le crochet 

  Déclaration de onLoopTic
  onLoopTic est un pointeur de fonction initialisé sur la fonction à exécuter

  void (*onLoopTic)();

  Exemple : le pointeur de fonction onDoubleClick reçoit en paramètre la fonction à appeler
  lors du double clic (voir ci dessous le mécanisme permettant de passer un paramètre à un pointeur de fonction).
  onDoubleClick = loopTicParam; // loopTicParam() sera appelé sur le doubleClick

  // Déclaration du pointeur de fonction utilisé par loopTicParam
  void (*funcToCall)();

  //Appelé par onDoubleClick
  void loopTicParam() {
    // Déclaration d'un pointeur de fonction
    void (*myFunctionPtr)(void (*function)());
    myFunctionPtr = &loopTic;
    (*myFunctionPtr)(funcToCall);
  }

  Cette fonction déclare un pointeur de fonction void (*myFunctionPtr)
  recevant en paramètre un pointeur de fonction ((void (*function)());

  myFunctionPtr est initialisé avec l'adresse de loopTic
  LoopTic doit être une fonction de type void f((*func)()) :

  void loopTic(void (*function)())

  Conformément à la déclaration void (*myFunctionPtr)(void (*function)());
  lopTic est ensuite appelé par (*myFunctionPtr)(funcToCall) avec comme
  paramètre l'adresse d'une fonction funcToCall (voir plus haut).
  loopTic affecte cette valeur au crochet : onLoopTic = function

  void loopTic(void (*function)()) {
     onLoopTic = function;
  }

  // Puis dans loop le crochet onLoopTic() appele funcToCall

  Exemple :
  funcToCall = func;
  onDoubleClick = loopTicParam;
  // doubleClick sur le bouton rotaty
  loopTicParam() -> loopTic(func) => met en place le crochet
  // Dans loop
   void loop {
     ....
     onLoopTic(); // appel de func
   }

  Le crochet est annihilé par onLoopTic = nullFunc

   void nullFunc() {
   }

  Nota :
  Pour exécuter des fonctions diverses (avec passage de paramètre) sur le même appel de fonction onRotary,
  on utilise le même mécanisme (sans necessiter de crochet dans loop [onRotary est hors isr])
  Il suffit de faire pointer onRotary sur différentes fonction suivant les besoins
*/

// Utilisé pour annihiler les crochets
void nullFunc() {
}

// Affiché par défaut de la première ligne
// Etat des relais L1..K8
// Note K6, K6, K8 sont associés a K5 (alimentation primaire transfo)
// Exemple S6 = K6 & K5

// Appelé dans dans loop
// Concatène les ports de sortie ds une chaine
// Valeur affichée sur la première ligne lcd
char* readPortIo_O() {
  static char str[25];
  strcpy(str, n1gpioRead(O_VMC));
  strcat(str, ngpioRead(O_PAC));
  strcat(str, ngpioRead(O_FOUR));
  strcat(str, ngpioRead(O_POMPE));
  strcat(str, gpioSRead(O_EV_ARROSAGE));
  strcat(str, gpioSRead(O_EV_IRRIGATION));
  strcat(str, gpioSRead(O_EV_EST));
  return str;
}

// Concatène les ports d'entrée dans une chaine
// Nota l'entrée E4 de l'interface opto est utilisée
// pour autoriser l'affichage
// Affichage par défaut de la dernière ligne
char* readPortIo_I() {
  static char str[31];
  sprintf(str, "E1:%s E2:%s E3:%s", n0gpioRead(I_ARROSAGE), n0gpioRead(I_IRRIGATION), n0gpioRead(I_SURPRESSEUR));
  return str;
}

// Permet de passer un paramètre à un pointeur de fonction
// Appelé selon les besoins par onSignelClick ou onDoubleClick()
void loopTicParam() {
  t_stop(tache_t_defaultDisplay);
  t_start(tache_t_defaultDisplay);
  // Déclaration d'un pointeur de fonction 
  // recevant en paramètre un pointeur de fonction
  void (*myFunctionPtr)(void (*func)());
  // Pointer le pointeur de fonction sur la fonction à appeler
  myFunctionPtr = &loopTic;
  // Appeler la fonction via le pointeur en lui passant une adresse de fonction
  (*myFunctionPtr)(funcToCall);
}

// Permet de passer un paramètre à un pointeur de fonction
// Appelé par onSingleClick() dans encoderOutputTask2
void loopTicParam2() {
  t_stop(tache_t_defaultDisplay);
  t_start(tache_t_defaultDisplay);
  // Déclaration d'un pointeur de fonction 
  // recevant en paramètre un pointeur de fonction
  void (*myFunctionPtr)(void (*func)());
  // Pointer le pointeur de fonction sur la fonction à appeler
  myFunctionPtr = &loopTic2;
  // Appeler la fonction via le pointeur en lui passant une adresse de fonction
  (*myFunctionPtr)(funcToCall2);
}

//-------------------------------------------
// Fonctions appelées par un double click
// Ces fonctions sont mises hors ISR par un 
// crochet dans loop
//-------------------------------------------

//-------------------------------------------
// Fonctions applées par buttonFuncLevel2_1_X
//-------------------------------------------
void commun2_1() {
  display = nullFunc;
  msgOk = MSG_OK;
  onLoopTic = buttonFuncParam;
}

void _startWatering() {
  commun2_1();
  startWatering(TIMEOUT);
}

void _stopWatering() {
  commun2_1();
  stopWatering();
}

void _startIrrigation() {
  // onLoopTic = nullFunc;
  // display = nullFunc;
  // Serial.println("Start irrigatuion");
  commun2_1();
  startTankFilling();
  // msgOk = "Set";
  // onLoopTic = buttonFuncParam;
}

void _stopIrrigation() {
  commun2_1();
  stopTankFilling();
}

void _startPowerCooking() {
  commun2_1();
  on(O_FOUR);
}

void _stopPowerCooking() {
  commun2_1();
  off(O_FOUR);
}

void _startVMC() {
  commun2_1();
  setVmc(CMD_VMC_ON);
}

void _stopVMC() {
  commun2_1();
  setVmc(CMD_VMC_OFF);
}

void _startWateringLemon() {
  commun2_1();
  on(O_TRANSFO);
  on(O_EV_EST);
  t_start(tache_t_cmdEvEst);
}

void _stopWateringLemon() {
  commun2_1();
  off(O_EV_EST);
  off(O_TRANSFO);
  t_stop(tache_t_cmdEvEst);
}

void _startPowerPac() {
  commun2_1();
  irSendOn = false;
  on(O_PAC);
  cPersistantParam->set(PAC, 1);
  filePersistantParam->writeFile(cPersistantParam->getStr(), "w");
  // filePowerPac->writeFile("1", "w");
  t_start(tache_t_monoPacOn);
  t_stop(tache_t_monoPacOff);
}

void _stopPowerPac() {
  commun2_1();
  t_start(tache_t_monoPacOff); // Logique inversée pour relai PAC
  irSendOn = true;
  cPersistantParam->set(PAC, 0);
  filePersistantParam->writeFile(cPersistantParam->getStr(), "w");
  // filePowerPac->writeFile("0", "w");
}

//----------------------------------------------------------------------
// Retour à l'affichage par défaut 
//----------------------------------------------------------------------

void _ioDisplay() {
  onLoopTic = nullFunc;
  onLoopTic2 = nullFunc;
  onRotary = nullFunc;
  ioDisplay();
  display = ioDisplay;
}

//----------------------------------------------------------------------
// Retour à l'affichage par défaut après paramétrage des temporisateurs
//----------------------------------------------------------------------
void _ioDisplayAndValidateDlyParam() {
  onLoopTic = nullFunc;
  onRotary = nullFunc;
  fileDlyParam->writeFile(cDlyParam->getStr(), "w");
  ioDisplay();
}

void reboot() {
  ESP.restart();
}

// Sur les ESP32E le reboot doit être appelé hors ISR !!!
void _reboot() {
  onLoopTic = reboot;
}

// Fonction appelées par buttonFuncLevel2_3_X
void _summerTimeOn() {
  cDlyParam->set(SUMMER_TIME, SUMMER_TIME_OFFSET);
  apply();
  // Serial.println("_summerTimeOn");
  ESP.restart();
}

void _summerTimeOff() {
  onLoopTic = nullFunc;
  cDlyParam->set(SUMMER_TIME, WINTER_TIME_OFFSET);
  apply();
  // Serial.println("_summerTimeOff");
  ESP.restart();
}

void _logOn() {
  // onLoopTic = nullFunc;
  cDlyParam->set(LOG_STATUS, 1);
  apply();
  // Serial.println("_logOn");
  onLoopTic = buttonFuncParam;
}

void _logOff() {
  // onLoopTic = nullFunc;
  cDlyParam->set(LOG_STATUS, 0);
  apply();
  // Serial.println("_logOff");
  onLoopTic = buttonFuncParam;
}

//---------------------------------------------
// Fonctions appelées par buttonFuncLevel2_5_x
//---------------------------------------------
void updateScheduledGlobalParam() {
  // onLoopTic = nullFunc;
  display = nullFunc;
  fileGlobalScheduledParam->writeFile(cGlobalScheduledParam->getStr(), "w");
  msgOk = MSG_OK;
  onLoopTic = buttonFuncParam;
}

void _enablePowerCookSched() {
  cGlobalScheduledParam->set(POWER_COOK, 1);
  updateScheduledGlobalParam();
}

void _disablePowerCookSched() {
  cGlobalScheduledParam->set(POWER_COOK, 0);
  updateScheduledGlobalParam();
}

void _enableIrrigationSched() {
  cGlobalScheduledParam->set(IRRIGATION, 1);
  updateScheduledGlobalParam();
}

void _disableIrrigationSched() {
  cGlobalScheduledParam->set(IRRIGATION, 0);
  updateScheduledGlobalParam();
}

void _enableLemonWaterringSched() {
  cGlobalScheduledParam->set(VANNE_EST, 1);
  updateScheduledGlobalParam();
}

void _disableLemonWaterringSched() {
  cGlobalScheduledParam->set(VANNE_EST, 0);
  updateScheduledGlobalParam();
}

void _enablePowerPacSched() {
  cGlobalScheduledParam->set(PAC, 1);
  updateScheduledGlobalParam();
}

void _disablePowerPacSched() {
  cGlobalScheduledParam->set(PAC, 0);
  updateScheduledGlobalParam();
}

void _enableVmcSched() {
  cGlobalScheduledParam->set(VMC, 1);
  updateScheduledGlobalParam();
}

void _disableVmcSched() {
  cGlobalScheduledParam->set(VMC, 0);
  updateScheduledGlobalParam();
}

//----------------------------------------------------------------------
// Commun aux fonctions appelées par buttonFuncLevel2_2_X
//----------------------------------------------------------------------
void apply() {
  fileDlyParam->writeFile(cDlyParam->getStr(), "w");
  msgOk = MSG_OK;
  onLoopTic = buttonFuncParam;
}

//----------------------------------------------------------------------
// Fonctions applées par buttonFuncLevel2_2_X
//----------------------------------------------------------------------
void setParamWatering() {
  cDlyParam->set(TIME_WATERING, rotary * 60);
  // Serial.println(cDlyParam->getStr());
  apply();
}

void setParamTankFilling() {
  cDlyParam->set(TIME_TANK_FILLING, rotary);
  apply();
}

void setParamLemonWaterring() {
  cDlyParam->set(EAST_VALVE_ON_TIME, rotary * 60);
  apply();
}

void setParamSuppressorFilling() {
  cDlyParam->set(TIME_SUPRESSOR, rotary);
  apply();
}

//--------------------------------------------------------

void encoderSetTime(void (*funcToCallOnSingleClick)(), void (*funcToCallOnDoubleClick)()) {
  onLoopTic = nullFunc;
  // Serial.println(rotary);
  char buffer[21];
  sprintf(buffer, "Max time = %d %s", rotary, unit);
  lcdPrintString(Dbl_Push_to_validate, 0, 0, true);
  lcdPrintString(Push_to_Quit, 1, 0, true);
  lcdPrintString(buffer, 2, 0, true);
  lcdPrintString(msgOk, 3, 2, true);
  msgOk = "";
  // Pour passer Hors IRS
  funcToCall = funcToCallOnSingleClick;
  onSingleClick = loopTicParam;
  onDoubleClick = funcToCallOnDoubleClick;
}

void encoderOutputTask2(void (*funcOn)(), void (*funcOff)()) {
  onLoopTic = nullFunc;
  lcdPrintString(Dbl_Push_to_validate, 0, 0, true);
  lcdPrintString(Push_to_Quit, 1, 0, true);
  lcdPrintString(msgOnOff[rotary], 2, 0, true);
  lcdPrintString(msgOk, 3, 2, true);
  msgOk = "";
  onSingleClick = loopTicParam2;
  funcToCall2 = _ioDisplay;
  onDoubleClick = loopTicParam;
  switch (rotary) {
  case 0:
    funcToCall = funcOn;
    break;
  case 1:
    funcToCall = funcOff;
    break;
  }
}

void encoderOutputTask() {
  onLoopTic = nullFunc;
  lcdPrintString(msgSetOutput[rotary], 2, 0, true);
  // Execute la fonction loopTicParam définie dans dyplay.cpp
  onSingleClick = loopTicParam;
  switch (rotary) {
  case 0:
    // Met à jour la fonction à appler dans loopTicParam
    funcToCall = buttonFuncLevel2_1_0;
    break;
  case 1:
    funcToCall = buttonFuncLevel2_1_1;
    break;
  case 2:
    funcToCall = buttonFuncLevel2_1_2;
    break;
  case 3:
    funcToCall = buttonFuncLevel2_1_3;
    break;
  case 4:
    funcToCall = buttonFuncLevel2_1_4;
    break;
  case 5:
    funcToCall = buttonFuncLevel2_1_5;
    break;
  case 6:
    funcToCall = _ioDisplay;
    break;
  }
}

void encoderScheduledActionTask() {
  onLoopTic = nullFunc;
  lcdPrintString(msgScheduledAction[rotary], 2, 0, true);
  onSingleClick = loopTicParam;
  switch (rotary) {
  case 0:
    funcToCall = buttonFuncLevel2_5_0;
    break;
  case 1:
    funcToCall = buttonFuncLevel2_5_1;
    break;
  case 2:
    funcToCall = buttonFuncLevel2_5_2;
    break;
  case 3:
    funcToCall = buttonFuncLevel2_5_3;
    break;
  case 4:
    funcToCall = buttonFuncLevel2_5_4;
    break;
  }
}

void encoderLogSettingsTask() {
  onLoopTic = nullFunc;
  lcdPrintString(msgLogSetting[rotary], 2, 0, true);
  onSingleClick = loopTicParam;
  switch (rotary) {
  case 0:
    funcToCall = buttonFuncLevel2_4_0;
    break;
  case 1:
    funcToCall = _ioDisplay;
    break;
  }
}

void encoderTimeSummerTask() {
  onLoopTic = nullFunc;
  lcdPrintString(msgSummerTm[rotary], 2, 0, true);
  onSingleClick = loopTicParam;
  switch (rotary) {
  case 0:
    funcToCall = buttonFuncLevel2_3_0;
    break;
  case 1:
    funcToCall = _ioDisplay;
    break;
  }
}

void encoderTimerTask() {
  onLoopTic = nullFunc;
  lcdPrintString(msgTimer[rotary], 2, 0, true);
  onSingleClick = loopTicParam;
  switch (rotary) {
  case 0:
    // Paramétrage max time wattering
    funcToCall = buttonFuncLevel2_2_0;
    break;
  case 1:
    // Tank filling time
    funcToCall = buttonFuncLevel2_2_1;
    break;
  case 2:
    funcToCall = buttonFuncLevel2_2_2;
    break;
  case 3:
    funcToCall = buttonFuncLevel2_2_3;
    break;
  case 4:
    funcToCall = _ioDisplay;
    break;
  }
}

//--------------------------------------------------------
// Premier niveau de menu
//--------------------------------------------------------
void encoderLevel0Task() {
  // Affichage lcd sans timeout
  t_stop(tache_t_backLight2);
  // Bloquer la rotation de l'index
  lcd.displayOn();
  isLcdDisplayOn = false;
  lcdPrintString(Select_function, 0, 1, true);
  lcdPrintString(Push_to_validate, 1, 1, true);
  lcdPrintString(msgRotary[rotary], 2, 0, true);
  onSingleClick = loopTicParam;
  // En fonction de la valeur encodée
  switch (rotary) {
  case 0:
    // -Force output
    funcToCall = buttonFuncLevel1_1;
    break;
  case 1:
    // -Set timer param
    funcToCall = buttonFuncLevel1_2;
    break;
  case 2:
    // Set scheduled actions
    funcToCall = buttonFuncLevel1_5;
    break;
  case 3:
    // -Daylight timeOffset
    funcToCall = buttonFuncLevel1_3;
    break;
  case 4:
    // -Log setting
    funcToCall = buttonFuncLevel1_4;
    break;
  case 5:
    // - Reboot
    funcToCall = _reboot;
    break;
  case 6:
    // - Quit
    funcToCall = _ioDisplay;
    break;
  }
}

/*
 * Mise à jour de la valeur encodée
 * Appelé à chaque rotation de l'encodeur
 * Appelle la fonction pointée par onRotary
*/
void on_rotary(long value) {
  delay(20);
  rotary = (int)value;
  onRotary();
}

/*
 * Appelé par loopTicParam via un pointeur de fonction
 * Ne pas appeler directement
 * Appartient à une ISR
 * Prépare le changement de contexte par la mise
 * en place d'un pointeur de fonction utilisé dans loop
 * (loop appelle onLoopTic)
 * la fonction func passée en paramètre sera hors ISR
 */

void loopTic(void (*func)()) {
  // Fonction ISR : 
  // Serial.println("loopTic");
  onLoopTic = func;
}

void loopTic2(void (*func)()) {
  // Serial.println("loopTic");
  onLoopTic2 = func;
}

//------------------------------------------
// Point d'entrée
// Hors ISR appelé par displayPrint1 sur
// appuy du bouton rotary
//------------------------------------------
void buttonFuncLevel0() {
  onLoopTic = nullFunc;
  setBoundaries(0, (sizeof(msgRotary) / sizeof(msgRotary[0])) - 1);
  setEncoder(0);
  rotary = 0;
  lcd.clear();
  // Appel direct à l'init
  encoderLevel0Task();
  // Puis appel à chaque nouvelle rotation de l'encodeur
  onRotary = encoderLevel0Task;
}

// Détermine la plage de l'encodeur et
// la fonction func à executer
void communFuncLevel1(int h_boundarie, void (*func)()) {
  onLoopTic = nullFunc;
  setBoundaries(0, h_boundarie);
  setEncoder(0);
  rotary = 0;
  func();
  onRotary = func;
}

//--------------------------------------------------------
//   Menu principal
//   Fonctions communes aux buttonFuncLevel1_X
//   Prépare l'appel au paramètrage (hors ISR)
//--------------------------------------------------------
// Forçage des sorties
void buttonFuncLevel1_1() {
  communFuncLevel1((sizeof(msgSetOutput) / sizeof(msgSetOutput[0])) - 1, encoderOutputTask);
}

// Paramètrage des timer (mémorisés dans le fichier dlyparam.txt)
void buttonFuncLevel1_2() {
  communFuncLevel1((sizeof(msgTimer) / sizeof(msgTimer[0])) - 1, encoderTimerTask);
}

// Paramétrage heure été/hivers
void buttonFuncLevel1_3() {
  communFuncLevel1((sizeof(msgSummerTm) / sizeof(msgSummerTm[0])) - 1, encoderTimeSummerTask);
}

// Paramétrage des Logs
void buttonFuncLevel1_4() {
  communFuncLevel1((sizeof(msgLogSetting) / sizeof(msgLogSetting[0])) - 1, encoderLogSettingsTask);
}

// Paramétrage global des programmation
void buttonFuncLevel1_5() {
  communFuncLevel1((sizeof(msgScheduledAction) / sizeof(msgScheduledAction[0])) - 1, encoderScheduledActionTask);
}

//---------------------------------------------------------------------
// buttonFuncParam() permet d'utiliser un pointeur de fonction
// en utilisant deux paramètres
// Utilisé par toutes les fonctions buttonFuncLevel2_X_Y
//--------------------------------------------------------------------- 

// Déclaration des pointeurs de fonction à appeler par myFunctionPtr
void (*paramFunc1)();
void (*paramFunc2)();
// Pointeur de fonction recevant en paramètre deux pointeurs de fonction
void (*myFunctionPtr)(void (*func1)(), void (*func2)());

// Permet de passer deux paramètres à un pointeur de fonction
void buttonFuncParam() {
  (*myFunctionPtr)(paramFunc1, paramFunc2);
}

//--------------------------------------------------------
//   Fonctions commune aux buttonFuncLevel2_1_X et 
//   buttonFuncLevel2_3_X
//   Prépare l'appel au paramètrage (hors ISR)
//--------------------------------------------------------
void prepareOnOff(void (*funcOn)(), void (*funcOff)()) {
  onLoopTic = nullFunc;
  setBoundaries(0, (sizeof(msgOnOff) / sizeof(msgOnOff[0])) - 1);
  setEncoder(rotary);
  // rotary = 0;
  // Fonctions à appeler par encoderOutputTask2
  paramFunc1 = funcOn;
  paramFunc2 = funcOff;
  encoderOutputTask2(funcOn, funcOff);
  // Pointer le pointeur de fonction sur la fonction à appeler
  myFunctionPtr = &encoderOutputTask2;
  onRotary = buttonFuncParam;
}

//--------------------------------------------------------
//   Fonctions buttonFuncLevel2_1_X
//   Commande On Off sur les sorties
//   Fonction hors IRS appelées indirectement par loopTic
//--------------------------------------------------------
// Commande arrosage
void buttonFuncLevel2_1_0() {
  rotary = ugpioSRead(O_EV_ARROSAGE);
  prepareOnOff(_startWatering, _stopWatering);
}

// Commande remplissage réservoir
void buttonFuncLevel2_1_1() {
  rotary = ugpioSRead(O_EV_IRRIGATION);
  prepareOnOff(_startIrrigation, _stopIrrigation);
}
// Commande mise sous tension éléctroménager
void buttonFuncLevel2_1_2() {
  rotary = ugpioRead(O_FOUR);
  prepareOnOff(_startPowerCooking, _stopPowerCooking);
}
// Commande VMC
void buttonFuncLevel2_1_3() {
  rotary = ugpioRead(O_VMC);
  prepareOnOff(_startVMC, _stopVMC);
}
// Commande arrosage agrumes
void buttonFuncLevel2_1_4() {
  rotary = ugpioSRead(O_EV_EST);
  prepareOnOff(_startWateringLemon, _stopWateringLemon);
}
// Commande PAC
void buttonFuncLevel2_1_5() {
  rotary = ugpioRead(O_PAC);
  prepareOnOff(_startPowerPac, _stopPowerPac);
}

//--------------------------------------------------------
//   Fonctions commune aux buttonFuncLevel2_2_X
//   Prépare l'appel au paramètrage (hors ISR)
//--------------------------------------------------------

void prepareTimerSet(int min, int max,
  unsigned currentValue,
  void (*func1)(), void (*func2)()) {

  onLoopTic = nullFunc;
  setBoundaries(min, max);
  setEncoder(currentValue);
  // Serial.println(currentValue);
  rotary = currentValue;
  paramFunc1 = func1;
  paramFunc2 = func2;
  encoderSetTime(func1, func1);
  myFunctionPtr = &encoderSetTime;
  onRotary = buttonFuncParam;
}

//--------------------------------------------------------
//   Fonctions buttonFuncLevel2_2_X
//   Paramètrage des valeurs par défaut des temporisateurs
//   Fonction hors IRS (appelées indirectement par loopTic)
//--------------------------------------------------------

// Durée d'arrosage max
void buttonFuncLevel2_2_0() {
  // Temps en secondes -> minutes
  prepareTimerSet(
    MinTimeOutWaterring / 60,
    MaxTimeOutWaterring / 60,
    cDlyParam->get(TIME_WATERING) / 60,
    _ioDisplayAndValidateDlyParam,
    setParamWatering);
  unit = "mn";
}

// Temps de remplissage réservoir
void buttonFuncLevel2_2_1() {
  // Temps en secondes
  prepareTimerSet(
    MinTimeTankFilling,
    MaxTimeTankFilling,
    cDlyParam->get(TIME_TANK_FILLING),
    _ioDisplayAndValidateDlyParam,
    setParamTankFilling);
  unit = "s";
}

// Durée arrosage agrumes
void buttonFuncLevel2_2_2() {
  // Temps en secondes -> minutes
  prepareTimerSet(
    MinTimeOutWaterringEV_Est / 60,
    MaxTimeOutWaterringEV_Est / 60,
    cDlyParam->get(EAST_VALVE_ON_TIME) / 60,
    _ioDisplayAndValidateDlyParam,
    setParamLemonWaterring);
  unit = "mn";
}

// Timeout de sécurité remplissage supresseur 
void buttonFuncLevel2_2_3() {
  prepareTimerSet(
    MinTimeOutSupressor,
    MaxTimeOutSupressor,
    cDlyParam->get(TIME_SUPRESSOR),
    _ioDisplayAndValidateDlyParam,
    setParamSuppressorFilling);
  unit = "s";
}

//--------------------------------------------------------
//   Fonctions buttonFuncLevel2_3_X
//   Paramètre heure d'été / hivers
//   Fonction hors IRS appelées indirectement par loopTic
//--------------------------------------------------------
// Heure été/hivers
void buttonFuncLevel2_3_0() {
  // rotary = cDlyParam->get(OFFSET_SUMMER_TIME_OFF) == 0 ? 1 : 0;
  // Heure d'été 1, hivers 2
    // Prépositionne l'actions probable
  rotary = cDlyParam->get(SUMMER_TIME) == WINTER_TIME_OFFSET ? SUMMER_TIME_OFFSET : WINTER_TIME_OFFSET;
  prepareOnOff(_summerTimeOn, _summerTimeOff);
}

//--------------------------------------------------------
//   Fonctions buttonFuncLevel2_4_X
//   Paramètre enregistrement logs
//   Fonction hors IRS appelées indirectement par loopTic
//--------------------------------------------------------
// Logs on/off
void buttonFuncLevel2_4_0() {
  // Prépositionne l'actions probable
  // On ->Off, Off->On
  rotary = cDlyParam->get(LOG_STATUS);
  prepareOnOff(_logOn, _logOff);
}

// Hors ISR
// Affichage lcd par défaut
// Déporté dans display.cpp
// void ioDisplay() {
//   displayPrint1(readPortIo_O(), readPortIo_I(), TO_FONC);
// }

//--------------------------------------------------------
//   Fonctions buttonFuncLevel2_5_X
//   Paramètrage des actions programmées
//   Fonction hors IRS appelées indirectement par loopTic
//--------------------------------------------------------
// Validation programmtion electroménager
void buttonFuncLevel2_5_0() {
  // Prépositionne l'actions probable
  // On ->Off, Off->On
  rotary = cGlobalScheduledParam->get(POWER_COOK);
  prepareOnOff(_enablePowerCookSched, _disablePowerCookSched);
}

// Validation programmation remplissage réservoir
void buttonFuncLevel2_5_1() {
  rotary = cGlobalScheduledParam->get(IRRIGATION);
  prepareOnOff(_enableIrrigationSched, _disableIrrigationSched);
}

// Validation programmtion arroage agrumes
void buttonFuncLevel2_5_2() {
  rotary = cGlobalScheduledParam->get(VANNE_EST);
  prepareOnOff(_enableLemonWaterringSched, _disableLemonWaterringSched);
}

// Validation programmtion PAC
void buttonFuncLevel2_5_3() {
  rotary = cGlobalScheduledParam->get(PAC);
  prepareOnOff(_enablePowerPacSched, _disablePowerPacSched);
}

// Validation programmtion VMC
void buttonFuncLevel2_5_4() {
  rotary = cGlobalScheduledParam->get(VMC);
  prepareOnOff(_enableVmcSched, _disableVmcSched);
}
