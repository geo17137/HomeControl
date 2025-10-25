/**
 * @file loop_prog.cpp
 * @brief Implementation of the main loop program for the ESP32 Home Control Display.
 * 
 * This file contains the implementation of the main loop program for the ESP32 Home Control Display.
 * It includes functions for handling the local human-machine interface (HMI) using a 20x4 LCD display
 * with an I2C interface and a rotary encoder with a vertical axis that has an on/off contact for single
 * and double click commands. The detection of clicks is done via interrupts (ISR).
 * 
 * The LCD interface and rotary button allow performing all actions that can be done from a smartphone,
 * except for scheduling tasks.
 * 
 * General Remarks:
 * - Each rotation of the rotary button calls the function pointed to by the onRotary function pointer.
 * - Each press on the axis of the button calls the functions pointed to by onSingleClick and onDoubleClick
 *   function pointers.
 * - Functions called by the onRotary function pointer are outside the ISR context.
 * - Functions called by the onSingleClick and onDoubleClick function pointers are within an ISR context.
 * 
 * Displaying Operations and States on the LCD:
 * - Displaying on the LCD screen must be done outside the ISR context due to the I2C protocol used for
 *   communication with the screen interface.
 * - This prohibits using the LCD display in functions triggered by clicks, which is circumvented by a
 *   context switch (see below).
 * 
 * Choosing Operations to Execute Displayed on the LCD:
 * - Operations are chosen by a rotary encoder.
 * - Each rotation (change in encoder value) calls the on_rotary function with the encoded value within
 *   the range set by setBoundaries(min, max).
 * - This value is stored in a global variable rotary, and the display is updated via a function called
 *   by the onRotary function pointer initialized to the function to execute.
 * - The execution context is outside the ISR (part of the loop).
 * 
 * Triggering Operations to Execute:
 * - Operations are triggered by pressing the rotary button.
 * - Two types of presses are considered: single click and double click.
 * - Depending on the type of press, the functions pointed to by onSingleClick or onDoubleClick are called.
 * - The execution context of these functions is within the ISR managing the clicks, prohibiting direct use
 *   of the LCD display (due to the I2C protocol under ISR used for communication with the display).
 * - To allow the use of the LCD display, a context switch from ISR to loop is necessary.
 * - The context switch is done as follows:
 *   - The called functions initialize function pointers to the task to execute, then set up a hook called
 *     in the loop.
 *   - This hook calls the desired function(s) via the function pointers initialized in the ISR.
 * 
 * Example:
 * - The function pointer onDoubleClick receives the function to call on a double click as a parameter.
 * - onDoubleClick = loopTicParam; // loopTicParam() will be called on doubleClick
 * 
 * - Declaration of the function pointer used by loopTicParam:
 *   void (*funcToCall)();
 * 
 * - Called by onDoubleClick:
 *   void loopTicParam() {
 *     void (*myFunctionPtr)(void (*function)());
 *     myFunctionPtr = &loopTic;
 *     (*myFunctionPtr)(funcToCall);
 *   }
 * 
 * - This function declares a function pointer *myFunctionPtr receiving a function pointer *function as a parameter.
 * - myFunctionPtr is initialized with the address of loopTic.
 * - loopTic must be a function receiving a function pointer as a parameter:
 *   void loopTic(void (*function)())
 * 
 * - loopTic is then called by (*myFunctionPtr)(funcToCall) with the address of a function funcToCall.
 * - loopTic assigns this value to the hook: onLoopTic = function
 * 
 * - In the loop, the hook onLoopTic() calls funcToCall.
 * 
 * Example:
 * - funcToCall = myFunc;
 * - onDoubleClick = loopTicParam;
 * - doubleClick on the rotary button
 * - loopTicParam() -> loopTic(func) => sets up the hook
 * - In the loop:
 *   void loop {
 *     ...
 *     onLoopTic(); // calls myFunc
 *     ...
 *   }
 * 
 * - Once used, the hook is nullified by pointing onLoopTic to an empty function:
 *   onLoopTic = nullFunc;
 * 
 *   void nullFunc() {
 *   }
 * 
 * Note:
 * - To execute various functions (with parameter passing) on the same onRotary function call,
 *   the same mechanism is used without requiring a hook in the loop [onRotary is outside ISR].
 * - It is sufficient to point onRotary to different functions as needed.
 * 
 * Functions:
 * - nullFunc: Used to nullify hooks in the loop.
 * - readPortIo_O: Concatenates output ports into a string for display on the first line of the LCD.
 * - testPortIO_O: Detects state changes on output ports to trigger display updates.
 * - readPortIo_I: Reads the status of three GPIO inputs and formats them into a string.
 * - testPortIO_I: Reads the state of three GPIO pins and combines their values into a single unsigned integer.
 * - resetDlyDefaultDisplay: Resets the default display delay.
 * - loopTicParam: Resets the default display delay and calls another function via a function pointer.
 * - loopTicParam2: Executes the loopTic2 function by using a function pointer.
 * - commun2_1: Common function called by buttonFuncLevel2_1_X functions.
 * - _startWatering, _stopWatering, _startIrrigation, _stopIrrigation, _startPowerCooking, _stopPowerCooking,
 *   _startVMC, _stopVMC, _startWateringLemon, _stopWateringLemon, _startPowerPac, _stopPowerPac: Functions
 *   called by buttonFuncLevel2_1_X functions.
 * - _ioDisplay: Returns to the default display.
 * - _ioDisplayAndValidateDlyParam: Returns to the default display after setting timer parameters.
 * - reboot: Reboots the ESP32.
 * - _reboot: Sets onLoopTic to reboot.
 * - _summerTimeOn, _summerTimeOff: Functions called by buttonFuncLevel2_3_X functions.
 * - _logOn, _logOff: Functions called by buttonFuncLevel2_4_X functions.
 * - updateScheduledGlobalParam: Updates scheduled global parameters.
 * - _enablePowerCookSched, _disablePowerCookSched, _enableIrrigationSched, _disableIrrigationSched,
 *   _enableLemonWaterringSched, _disableLemonWaterringSched, _enablePowerPacSched, _disablePowerPacSched,
 *   _enableVmcSched, _disableVmcSched: Functions called by buttonFuncLevel2_5_X functions.
 * - apply: Common function called by buttonFuncLevel2_2_X functions.
 * - setParamWatering, setParamTankFilling, setParamLemonWaterring, setParamSuppressorFilling: Functions
 *   called by buttonFuncLevel2_2_X functions.
 * - encoderSetTime: Sets up the encoder to handle time setting with single and double click actions.
 * - encoderOutputTask2: Handles the encoder output task with specified functions for on and off states.
 * - encoderOutputTask: Task to handle encoder output and update display accordingly.
 * - encoderScheduledActionTask: Handles the encoder scheduled action task.
 * - encoderLogSettingsTask: Handles the encoder log settings task.
 * - encoderTimeSummerTask: Task function to handle the encoder time summer logic.
 * - encoderTimerTask: Task function to handle the encoder timer logic.
 * - encoderLevel0Task: Handles the encoder level 0 task.
 * - on_rotary: Updates the encoded value and calls the function pointed to by onRotary.
 * - loopTic: Prepares the context switch by setting up a function pointer used in the loop.
 * - loopTic2: Prepares the context switch by setting up a function pointer used in the loop.
 * - buttonFuncLevel0: Entry point called by displayPrint1 on rotary button press.
 * - communFuncLevel1: Executes a function within specified boundaries and sets up rotary encoder handling.
 * - buttonFuncLevel1_1, buttonFuncLevel1_2, buttonFuncLevel1_3, buttonFuncLevel1_4, buttonFuncLevel1_5:
 *   Functions called by the main menu.
 * - buttonFuncParam: Allows passing two parameters to a function pointer.
 * - prepareOnOff: Prepares the on/off functions and initializes the encoder.
 * - buttonFuncLevel2_1_0, buttonFuncLevel2_1_1, buttonFuncLevel2_1_2, buttonFuncLevel2_1_3, buttonFuncLevel2_1_4,
 *   buttonFuncLevel2_1_5: Functions called by the second level menu for on/off commands.
 * - prepareTimerSet: Prepares the timer settings and initializes the encoder.
 * - buttonFuncLevel2_2_0, buttonFuncLevel2_2_1, buttonFuncLevel2_2_2, buttonFuncLevel2_2_3: Functions called by
 *   the second level menu for timer settings.
 * - buttonFuncLevel2_3_0: Function called by the second level menu for summer/winter time settings.
 * - buttonFuncLevel2_4_0: Function called by the second level menu for log settings.
 * - buttonFuncLevel2_5_0, buttonFuncLevel2_5_1, buttonFuncLevel2_5_2, buttonFuncLevel2_5_3, buttonFuncLevel2_5_4:
 *   Functions called by the second level menu for scheduled actions.
 */
#include "loop_prog.h"
#include "io.h"

/*
  L'IHM locale est réalisée à l'aide d'un afficheur LCD 20x4
  https://fr.aliexpress.com/w/wholesale-20x4-lcd-arduino.html
  équipé d'un interface I2C et d'un encodeur rotatif dont l'axe
  vertical est possède un contact on/off permettant des commandes
  de type single clic ou double clic. 
  La détection des clics et faite par interruption (ISR)

  L'interface lcd et le bouton rotary permettent de réaliser toutes les actions faites depuis
  un smartphone exepté la programmation des tâches horaires.

  Remarques générales :
  A chaque rotation du bouton rotary la fonction affectée au pointeur de fonction onRotary est appelé.
  A chaque appuy sur l'axe du bouton, les fonctions affectées pointeurs de fonctions onSingleClick et 
  onDoubleClic sont appelées
  - les fonctions appelées par le pointeur de fonction onRotary sont hors contexte ISR
  - les fonctions appelées par les pointeurs de fonction onSingleClick et onDoubleClic sont dans un
    contexte ISR

  Affichage des opérations et des états sur lcd display
  -----------------------------------------------------
  L'affichage sur l'écran LCD doit se faire dans un contexte d'execution hors ISR
  du fait de la communication avec l'interface de l'écran utilisant le protocole I2C.
  Les échanges avec ce protoloce sont sous interruptions.
  Ceci interdit d'utiliser l'afficheur LCD dans les fonctions déclenchées par des clics,
  ce problème est contourné par un changement de contexte (voir plus loin).

  Choix des opérations à exécuter affichées sur l'écran LCD
  ---------------------------------------------------------
  Les opérations sont choisies par un encodeur rotatif.
  A chaque rotation (changement de valeur de l'encodeur), la fonction on_rotary est appelé
  avec pour paramètre la valeur encodée située dans la gamme setBoundaries(min, max).
  Cette valeur est mémorisée dans une variable globale rotary,
  puis l'affichage est mis à jour via une fonction appelée par le pointeur de fonction onRotary
  initialisé sur la fonction à exécuter. 
  Le contexte d'exécution est hors ISR (il fait parti de loop).

  Déclenchement des opérations à exécuter
  ---------------------------------------
  Les opérations sont déclenchées par appuy sur le bouton de l'encodeur.
  Deux type d'appuy sont pris en compte : single clic et double clic.
  Suivant le type d'appuy, les fonctions pointées par onSingleClick ou onDoubleClick sont appelées.
  Le contexte d'execution de ces fonctions dans l'ISR gérant les clics, interdisant l'utilisation directe
  de l'afficheur LCD (à cause du protocole I2C sous ISR utilisé pour communiquer avec l'afficheur).
  Pour permettre l'utilisation de l'afficheur LCD il est necessaire d'effectuer un changement de contexte IRS->loop
  Le changement de contexte est effectué de façon suivante:
  Les fonctions appelées initialisent des pointeurs de fonction sur la tâche à exécuter, puis mettent en place
  un crochet appelé dans la boucle loop. 
  Ce crochet appele la (les) fonction(s) souhaitées via les pointeurs de fonctions initialisés dans l'ISR. 
  
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

  Cette fonction déclare un pointeur de fonction *myFunctionPtr
    void (*myFunctionPtr)(void (*function)());
  recevant en paramètre un pointeur de fonction *function
    void (*function)();

  puis myFunctionPtr est initialisé avec l'adresse de loopTic
  Nota : loopTic doit être une fonction recevant comme paramêtre 
  un pointeur de fonction :

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
  funcToCall = myFunc;
  onDoubleClick = loopTicParam;
  // doubleClick sur le bouton rotaty
  // loopTicParam() -> loopTic(func) => met en place le crochet
  // Dans loop
   void loop {
     ...
     onLoopTic(); // appel de myFunc
     ...
   }

  Une fois utilisé, le crochet est annihilé
   en faisant pointer onLoopTic
  sur une fonction vide :
    onLoopTic = nullFunc

    void nullFunc() {
    }

  Nota :
  Pour exécuter des fonctions diverses (avec passage de paramètre) sur le même appel de fonction onRotary,
  on utilise le même mécanisme sans necessiter de crochet dans loop [onRotary est hors isr].
  Il suffit de faire pointer onRotary sur différentes fonctions suivant les besoins.

*/

// Utilisé pour annihiler les crochets dans loop()
void nullFunc() {
}

// Affiché par défaut de la première ligne
// Etat des relais L1..K8
// Note K6, K6, K8 sont associés a K5 (alimentation primaire transfo)
// Exemple S6 = K6 & K5

// Appelé dans dans loop via display()
// Concatène les ports de sortie ds une chaine
// Valeur affichée sur la première ligne lcd
char* readPortIo_O() {
  static char str[25];
  if (!vmcFast)
    strcpy(str, n1gpioRead(O_VMC));
  else
    strcpy(str, " 2");  // mode fast
  strcat(str, ngpioRead(O_PAC));
  strcat(str, ngpioRead(O_FOUR));
  strcat(str, ngpioRead(O_POMPE));
  strcat(str, gpioSRead(O_EV_ARROSAGE));
  strcat(str, gpioSRead(O_EV_IRRIGATION));
  strcat(str, gpioSRead(O_EV_EST));
  return str;
}

/**
 * @brief Utilisé pour détecter les changements d'état sur les ports de sortie
 *        afin de déclencher l'affichage.
 * @return unsigned 
 */
unsigned testPortIO_O() {
  return 
  (vmcFast) ? 2 :  ugpioRead(O_VMC) + 
  (ugpioRead(O_PAC) << 1) + 
  (ugpioRead(O_FOUR) << 2) + 
  (ugpioRead(O_POMPE) << 3) +
  (ugpioRead(O_EV_ARROSAGE) << 4) +
  (ugpioRead(O_EV_IRRIGATION) << 5) +
  (ugpioRead(O_EV_EST) << 6);
}

/**
 * @brief Reads the status of three GPIO inputs and formats them into a string.
 *
 * This function reads the status of three GPIO inputs (I_ARROSAGE, I_IRRIGATION, I_SURPRESSEUR)
 * and formats their values into a single string. The formatted string contains the values
 * of the three inputs in the format "E1:<value> E2:<value> E3:<value>".
 * Appelé dans dans loop via display()
 *  Nota l'entrée E4 de l'interface opto est utilisée
 * pour autoriser l'affichage (non affichée)
 * @return A pointer to a static character array containing the formatted string.
 */
char* readPortIo_I() {
  static char str[31];
  sprintf(str, "E1:%s E2:%s E3:%s", n0gpioRead(I_ARROSAGE), n0gpioRead(I_IRRIGATION), n0gpioRead(I_SURPRESSEUR));
  return str;
}

/**
 * @brief Reads the state of three GPIO pins and combines their values into a single unsigned integer.
 *
 * This function reads the state of three specific GPIO pins (I_ARROSAGE, I_IRRIGATION, and I_SURPRESSEUR)
 * and combines their values into a single unsigned integer. Each pin's state is represented by a single bit
 * in the returned integer:
 * - Bit 0: State of I_ARROSAGE
 * - Bit 1: State of I_IRRIGATION
 * - Bit 2: State of I_SURPRESSEUR
 *
 * @return An unsigned integer representing the combined state of the three GPIO pins.
 */
unsigned testPortIO_I() {
  return ugpioRead(I_ARROSAGE) + (ugpioRead(I_IRRIGATION) << 1) + (ugpioRead(I_SURPRESSEUR) << 2);
}

inline void resetDlyDefaultDisplay() {
  t_stop(tache_t_defaultDisplay);
  t_start(tache_t_defaultDisplay);
}

/**
 * @brief Function to reset the default display delay and call another function via a function pointer.
 * Permet de passer un paramètre à un pointeur de fonction
 * Appelé selon les besoins par onSignelClick ou onDoubleClick()
 * This function first resets the default display delay by calling `resetDlyDefaultDisplay()`.
 * It then declares a function pointer `myFunctionPtr` that points to a function taking another function pointer as a parameter.
 * The `myFunctionPtr` is assigned to point to the `loopTic` function.
 * Finally, it calls the function pointed to by `myFunctionPtr`, passing `funcToCall` as the parameter.
 * @note Ensure that loopTic and funcToCall are defined and accessible in the scope
 */
void loopTicParam() {
  resetDlyDefaultDisplay(); 
  // Déclaration d'un pointeur de fonction 
  // recevant en paramètre un pointeur de fonction
  void (*myFunctionPtr)(void (*func)());
  // Pointer le pointeur de fonction sur la fonction à appeler
  // loopTic prend en paramètre un pointeur de fonction
  myFunctionPtr = &loopTic;
  // Appeler la fonction via le pointeur en lui passant une adresse de fonction
  (*myFunctionPtr)(funcToCall);
}

/**
 * @brief Executes the loopTic2 function by using a function pointer.
 * 
 * This function resets the default display delay and then declares a function pointer
 * that takes another function pointer as a parameter. It assigns the loopTic2 function
 * to the function pointer and calls it, passing funcToCall2 as the argument.
 * Appelé par onSingleClick() dans encoderOutputTask2
 * @note Ensure that loopTic2 and funcToCall2 are defined and accessible in the scope
 * where this function is called.
 */
void loopTicParam2() {
  resetDlyDefaultDisplay(); 
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
// Fonctions appelées par buttonFuncLevel2_1_X
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
  // Serial.println("Start irrigation");
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
  setVmc(CMD_VMC_SLOW);
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
  irSendPacOff = false;
  on(O_PAC);
#ifdef PERSISTANT_PAC  
// A vérifier si c'est judicieux 
//  cPersistantParam->set(PAC, 1);
//  filePersistantParam->writeFile(cPersistantParam->getStr(), "w");
#endif  
  t_stop(tache_t_monoPacOff);
  t_start(tache_t_monoPacOn);
}

void _stopPowerPac() {
  commun2_1();
  t_start(tache_t_monoPacOff); // arrêt temporisé de la PAC
  irSendPacOff = true;
#ifdef PERSISTANT_PAC 
// A vérifier si c'est judicieux    
//  cPersistantParam->set(PAC, 0);
//  filePersistantParam->writeFile(cPersistantParam->getStr(), "w");
#endif
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

/**
 * @brief Sets up the encoder to handle time setting with single and double click actions.
 * 
 * This function configures the encoder to display the maximum time and sets up the actions
 * to be performed on single and double clicks. It updates the LCD display with relevant
 * messages and sets the callback functions for single and double click events.
 * 
 * @param funcToCallOnSingleClick Function to be called when a single click is detected.
 * @param funcToCallOnDoubleClick Function to be called when a double click is detected.
 */
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

/**
 * @brief Handles the encoder output task with specified functions for on and off states.
 *
 * This function sets up the display messages and assigns the appropriate functions
 * to be called based on the rotary encoder's state. It updates the display with
 * instructions and sets the function pointers for single and double click actions.
 *
 * @param funcOn Function to be called when the rotary encoder is in the "on" state.
 * @param funcOff Function to be called when the rotary encoder is in the "off" state.
 */
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

/**
 * @brief Task to handle encoder output and update display accordingly.
 *
 * This function sets the `onLoopTic` to `nullFunc` and prints a message on the LCD
 * based on the current rotary position. It also sets the `onSingleClick` function
 * to `loopTicParam` and updates the `funcToCall` based on the rotary position.
 *
 * The rotary positions and corresponding functions are:
 * - 0: `buttonFuncLevel2_1_0`
 * - 1: `buttonFuncLevel2_1_1`
 * - 2: `buttonFuncLevel2_1_2`
 * - 3: `buttonFuncLevel2_1_3`
 * - 4: `buttonFuncLevel2_1_4`
 * - 5: `buttonFuncLevel2_1_5`
 * - 6: `_ioDisplay`
 */
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

/**
 * @brief Executes the scheduled action based on the current rotary encoder position.
 *
 * This function is called to handle the scheduled action when the rotary encoder is used.
 * It performs the following steps:
 * 1. Sets the `onLoopTic` function to `nullFunc`.
 * 2. Prints a message corresponding to the current rotary position on the LCD.
 * 3. Sets the `onSingleClick` function to `loopTicParam`.
 * 4. Based on the current rotary position, assigns the appropriate function to `funcToCall`.
 *
 * The rotary encoder positions correspond to the following actions:
 * - Position 0: Calls `buttonFuncLevel2_5_0`.
 * - Position 1: Calls `buttonFuncLevel2_5_1`.
 * - Position 2: Calls `buttonFuncLevel2_5_2`.
 * - Position 3: Calls `buttonFuncLevel2_5_3`.
 * - Position 4: Calls `buttonFuncLevel2_5_4`.
 */
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

/**
 * @brief Task function to handle the encoder time summer logic.
 *
 * This function is responsible for updating the display with the summer time message
 * based on the current rotary encoder position. It also sets the appropriate function
 * to be called on a single click event.
 *
 * The function performs the following actions:
 * - Sets the `onLoopTic` function pointer to `nullFunc`.
 * - Prints the summer time message corresponding to the current rotary position on the LCD.
 * - Sets the `onSingleClick` function pointer to `loopTicParam`.
 * - Based on the current rotary position, sets the `funcToCall` function pointer to the
 *   appropriate function.
 *
 * @note The `rotary` variable is used to determine the current position of the rotary encoder.
 */
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
/**
 * @brief Handles the encoder level 0 task.
 * 
 * This function updates the LCD display without a timeout, blocks the rotation of the index,
 * and sets the appropriate function to call based on the current rotary value.
 * 
 * The function performs the following actions:
 * - Stops the backlight timeout task.
 * - Turns on the LCD display.
 * - Prints selection and validation messages on the LCD.
 * - Sets the single click action to loopTicParam.
 * - Based on the rotary value, sets the function to call:
 *   - 0: Force output (buttonFuncLevel1_1)
 *   - 1: Set timer parameters (buttonFuncLevel1_2)
 *   - 2: Set scheduled actions (buttonFuncLevel1_5)
 *   - 3: Daylight time offset (buttonFuncLevel1_3)
 *   - 4: Log settings (buttonFuncLevel1_4)
 *   - 5: Reboot (_reboot)
 *   - 6: Quit (_ioDisplay)
 */
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
/**
 * @brief Executes a function within specified boundaries and sets up rotary encoder handling.
 *
 * This function sets up the environment by initializing the loop tick function to a null function,
 * setting the boundaries, resetting the encoder, and initializing the rotary variable. It then
 * executes the provided function and sets up the rotary encoder to call the provided function on
 * rotary events.
 *
 * @param h_boundarie The upper boundary value to be set.
 * @param func A pointer to the function to be executed and set for rotary events.
 */
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
//   Fonctions commune (hors ISR) aux buttonFuncLevel2_2_X
//   Prépare l'appel au paramètrage 
//--------------------------------------------------------

/**
 * @brief Prepares the timer settings and initializes the encoder.
 *
 * This function sets the boundaries for the timer, initializes the encoder with the current value,
 * and assigns the provided callback functions for the encoder.
 *
 * @param min The minimum value for the timer.
 * @param max The maximum value for the timer.
 * @param currentValue The current value to set the encoder to.
 * @param func1 A pointer to the first callback function to be used by the encoder.
 * @param func2 A pointer to the second callback function to be used by the encoder.
 */
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
