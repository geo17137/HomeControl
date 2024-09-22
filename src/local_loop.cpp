#include "local_loop.h"
#include "display.h"
#include "mtr86.h"
#include "simple_param.h"
#include "io.h"

extern SimpleParam* cDlyParam;
/*
	Boucle exécutée toutes les secondes
	Met à jour les messages MQTT
	Permet au smartphone d'avoir une IHM à jour
	même après une déconnexion/reconnexion
	Gère les entrées par scrutation :
	- télécommande arrosage, réservoir
	- contact supresseur
  Fourni les logs
*/

/**
 * @brief Commande de type bascule D
 *        Utilisé par la télécommande RF
 * @param nButton numéro du port cablé à la télécommande
 * @return unsigned 
 */
unsigned Q_D1(int nButton) {
	static boolean first;
	static unsigned count;
	int val = digital_read(nButton);
	if (val == 0 && !first) {
		first = true;
		count = ++count % 2;
	}
	if (val == 1) {
		first = false;
	}
	return count;
}

/**
 * @brief Commande de type bascule D
 *        Utilisé par la télécommande RF
 * @param nButton numéro du port cablé à la télécommande
 * @return unsigned
 */
unsigned Q_D2(int nButton) {
	static boolean first;
	static unsigned count;
	int val = digital_read(nButton);
	if (val == 0 && !first) {
		first = true;
		count = ++count % 2;
	}
	if (val == 1) {
		first = false;
	}
	return count;
}

/**
 * @brief Information remplissage surpresseeur 
 */
void lcdSupressorWaitMsg() {
	display = nullFunc;
	backLightOn();
	lcdPrintString(SUPRESSOR_FILLING, 0, 1, true);
	lcdPrintString(WAIT, 1, 0, true);
	lcdPrintString(SEE_NOTICE, 2, 0, true);
	lcdPrintString(readPortIo_I(), 3, 2, true);
}
/**
 * @brief Information erreur remplissage surpressur
 */
void lcdSupressorFaultMsg() {
	display = nullFunc;
	backLightOn();
	lcdPrintString(SURPRESSOR_FAULT, 0, 1, true);
	lcdPrintString(PRESS_THE_BUTTON, 1, 0, true);
	lcdPrintString(SEE_NOTICE, 2, 0, true);
}
/**
 * @brief Information pompe en défault 
 */
void lcdPumpFault() {
	display = nullFunc;
	backLightOn();
	lcdPrintString(PUMP_DEFAULT, 0, 1, true);
	lcdPrintString(CORRECT_THE_DEFECT, 1, 1, true);
	lcdPrintString(THEN_PRESS_BUTTON, 2, 1, true);
	lcdPrintString(SEE_NOTICE2, 3, 1, true);
}

void reArm() {
	msgRearm = true;
}

void lcdReboot() {
	backLightOn();
	lcdClear();
	lcdPrintString(REBOOT, 1, 0, true);
	delay(2000);
	ESP.restart();
}

/**
 * @brief Applé toutes les INTERVAL_IO_SCRUT ms
 * 
 */
void localLoop(void) {
	// Commande manuelle arrosage
	// Tâche la moins prioritaire
  	//  Priorité 1 : supresseur
  	//  Priorité 2 : remplissage réservoir
	static boolean firstEdgeA1 = false;
	static boolean firstEdgeA2 = true;
	static boolean startSupressorFilling = false;

	unsigned qd1 = Q_D1(I_ARROSAGE);
	if (qd1 == 1 && !firstEdgeA1 && !isWatering && !isTankFilling && !startSupressorFilling) {
		firstEdgeA1 = true;
		firstEdgeA2 = false;
		startWatering(TIMEOUT);
	}
	if (qd1 == 0 && !firstEdgeA2 && isWatering && !isTankFilling) {
		firstEdgeA1 = false;
		firstEdgeA2 = true;
		// Serial.printf("qd1 %d\n", qd1);
		stopWatering();
	}

	// Commande manuelle remplissage réservoir
	// A condition que pas mise sous pression supresseur
	static boolean firstEdgeI1 = false;
	static boolean firstEdgeI2 = true;
	unsigned qd2 = Q_D2(I_IRRIGATION);
	// Serial.printf("qd2 %d\n", qd2);
	if (qd2 == 1 && !firstEdgeI1 && !isTankFilling && !startSupressorFilling) {
		firstEdgeI1 = true;
		firstEdgeI2 = false;
		startTankFilling();
		// Serial.println("startTankFilling");
#ifdef DEBUG_OUTPUT_LOOP2
		Serial.printf("\n%02d:%02d startTankFilling\n", h, m);
#endif    
	}
	if (qd2 == 0 && !firstEdgeI2 && isTankFilling) {
		firstEdgeI1 = false;
		firstEdgeI2 = true;
		firstEdgeA1 = false;
    	isTankFilling = false;
		stopTankFilling();
		// Serial.println("stopTankFilling");
#ifdef DEBUG_OUTPUT_LOOP2
		Serial.printf("\n%02d:%02d stopTankFilling\n", h, m);
#endif
	}

	// Commande lcd
	static boolean flagL;
	static boolean flagNL = true;
	boolean outputState = gpioState(I_LCD_CMD);
	// Serial.println(interLcd);

	if (outputState && !flagL) {
		// Serial.println("interLcd && !flagL");
		flagL = true;
		flagNL = false;
		// Force l'affichage de l'index tournant
		isLcdDisplayOn = true;
		// Affichage lcd sans timeout

		lcd.displayOn(); // dispplayOn set backLight
 		t_start(tache_t_backLight2);
		electricalPanelOpen = true;
	}
	else if (!outputState && (!flagNL || bootDisplayOff)) {
		bootDisplayOff = false;
		// Serial.println("interLcd && !flagL");
		flagL = false;
		flagNL = true;
		electricalPanelOpen = false;
		backLightOff();
	}

	// Commande supresseur
	if (gpioState(I_SURPRESSEUR)) {
		// Fermeture du contact supresseur
		#ifdef DEBUG_OUTPUT
		Serial.printf("msgRearm=%1d, startSupressorFilling=%1d, startSupressorFilling2=%1d\n",
						msgRearm, startSupressorFilling, startSupressorFilling2);
		#endif
		if (msgRearm && startSupressorFilling && !startSupressorFilling2) {
#ifdef DEBUG_OUTPUT_
			Serial.printf("gpioState(I_REARM) %d, msgRearm %d\n",
				gpioState(I_REARM),
				msgRearm);
			Serial.println("startSupressorFilling2");
#endif		
			msgRearm = false;
			startSupressorFilling = false;
			startSupressorFilling2 = true;
			mqttClient.publish(TOPIC_DEFAUT_SUPRESSEUR, "off");
			#ifdef DEBUG_OUTPUT
			Serial.println(TOPIC_DEFAUT_SUPRESSEUR " off");
			#endif
		}  
		if (!startSupressorFilling) {
			startSupressorFilling = true;
			lcdSupressorWaitMsg();
			setDelay(tache_t_surpressorFilling, cvrtic(cDlyParam->get(TIME_SUPRESSOR) * 1000));
 			if (!cDlyParam->get(SUPRESSOR_EN))
				t_start(tache_t_surpressorFilling);
			// Si arrosage  ou remplissage en cours
			// Couper les EV (ne pas couper la pompe (limiter l'usure))
			if (isTankFilling || isWatering) {
				t_stop(tache_t_tankFilling);
				t_stop(tache_t_watering);
				off(O_TRANSFO);
				off(O_EV_IRRIGATION);
				off(O_EV_ARROSAGE);
				if (isTankFilling)
					writeLogs("Fin remplissage réservoir");
				if (isWatering)
					writeLogs("Fin arrosage");
				isWatering = false;
				isTankFilling = false;
			}
			// Sinon mettre la pompe en route
			else {
				// Version >= 2024.6.25   
				if (cDlyParam->get(SUPRESSOR_EN)) {
					#ifdef DEBUG_OUTPUT
					Serial.println("Remplissage surpresseur");
					#endif
					on(O_POMPE);
					writeLogs("Remplissage surpresseur");
				}
				else {
					#ifdef DEBUG_OUTPUT
					Serial.println("Surpresseur desactivé");
					#endif
					writeLogs("Surpresseur desactivé");					
				}
			}
		}
	}
	// Ouverture du contact supresseur (fin remplissage)
	else if (startSupressorFilling) {
		off(O_POMPE);
		t_stop(tache_t_surpressorFilling);
		startSupressorFilling = false;
		startSupressorFilling2 = false;
		// Afficher l'interface par défaut
		// Appel indispensable car l'appel à display()
		// ne se fait que sur modification des E/S
		ioDisplay();
		display = ioDisplay;
		writeLogs("Fin remplissage surpresseur");
	}
	if (erreurSupresseur) {
		erreurSupresseur = false;
		erreurSupresseurEvent = true;
		lcdSupressorFaultMsg();
		onSingleClick = reArm;
		writeLogs("Erreur surpresseur");
	}
	if (erreurPompe) {
		erreurPompe = false;
		erreurPompEvent = true;
		lcdPumpFault();
		onSingleClick = lcdReboot;
		writeLogs("Erreur pompe");
	}
	logsUpdate();
}
