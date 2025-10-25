/**
 * @file local_loop.cpp
 * @brief This file contains the implementation of the local loop functions for the ESP32 Home Control Display project.
 * 
 * The local loop is executed every second and performs the following tasks:
 * - Updates MQTT messages to keep the remote application interface up-to-date even after a disconnection/reconnection.
 * - Manages inputs through polling:
 *   - Remote control for watering and reservoir
 *   - Supressor contact
 * - Updates logs
 * 
 * Functions:
 * - unsigned Q_D1(int nButton): D-type flip-flop command used by the RF remote control.
 * - unsigned Q_D2(int nButton): D-type flip-flop command used by the RF remote control.
 * - void lcdSupressorWaitMsg(): Displays the supressor filling message on the LCD.
 * - void lcdSupressorFaultMsg(): Displays the supressor fault message on the LCD.
 * - void lcdPumpFault(): Displays the pump fault message on the LCD.
 * - void reArm(): Re-arms the supressor pump.
 * - void lcdReboot(): Displays the reboot message on the LCD and restarts the ESP32.
 * - void localLoop(void): The main local loop function called every INTERVAL_IO_SCRUT (100 ms).
 * 
 * The local loop function handles:
 * - Manual watering control with priority management.
 * - Manual irrigation reservoir filling control.
 * - LCD activation when the electrical panel door is open.
 * - Supressor control based on the supressor contact state.
 * - Supressor error management.
 * - Pump error management.
 * - Log updates.
 */
#include "display.h"
#include "mtr86.h"
#include "simple_param.h"
#include "local_loop.h"
#include "io.h"

/*
	Boucle exécutée toutes les secondes
	Met à jour les messages MQTT pour
	permettre à l'appli distante d'avoir une IHM à jour
	même après une déconnexion/reconnexion
	Gère les entrées par scrutation:
	- télécommande arrosage, réservoir
	- contact supresseur
    Met à jour les logs
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
 * @brief Message remplissage surpresseur sur l'écran LCD
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
 * @brief Information erreur remplissage surpresseur sur l'écran LCD
 */
void lcdSupressorFaultMsg() {
	display = nullFunc;
	backLightOn();
	lcdPrintString(SURPRESSOR_FAULT, 0, 1, true);
	lcdPrintString(PRESS_THE_BUTTON, 1, 0, true);
	lcdPrintString(SEE_NOTICE, 2, 0, true);
}
/**
 * @brief Message défaut sur pompe supresseur sur l'écran LCD
 */
void lcdPumpFault() {
	display = nullFunc;
	backLightOn();
	lcdPrintString(PUMP_DEFAULT, 0, 1, true);
	lcdPrintString(CORRECT_THE_DEFECT, 1, 1, true);
	lcdPrintString(THEN_PRESS_BUTTON, 2, 1, true);
	lcdPrintString(SEE_NOTICE2, 3, 1, true);
}

/**
 * @brief Relance pompe surpresseur
 * 
 */
void reArm() {
	msgRearm = true;
}

/**
 * @brief Signale le reboot sur l'écran LCD
 * suite à la correction erreur pompe
 */
void lcdReboot() {
	backLightOn();
	lcdClear();
	lcdPrintString(REBOOT, 1, 0, true);
	delay(2000);
	ESP.restart();
}

/**
 * @brief Appelé toutes les INTERVAL_IO_SCRUT (100 ms)
 * 
 */
void localLoop() {
	// Commande manuelle arrosage
	// Tâche la moins prioritaire
  	//  - Priorité 1 : supresseur
  	//  - Priorité 2 : remplissage irrigation réservoir
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
		stopWatering();
	}

	// Commande manuelle remplissage irrigation réservoir
	// à condition que pas mise sous pression supresseur
	static boolean firstEdgeI1 = false;
	static boolean firstEdgeI2 = true;
	unsigned qd2 = Q_D2(I_IRRIGATION);
	if (qd2 == 1 && !firstEdgeI1 && !isTankFilling && !startSupressorFilling) {
		firstEdgeI1 = true;
		firstEdgeI2 = false;
		startTankFilling();
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
#ifdef DEBUG_OUTPUT_LOOP2
		Serial.printf("\n%02d:%02d stopTankFilling\n", h, m);
#endif
	}

	// Activation écran LCD si la porte du panneau est électrique ouverte
	static boolean flagL;
	static boolean flagNL = true;
	// Lecture contact porte
	boolean doorState = gpioState(I_LCD_CMD);

	if (doorState && !flagL) {
		flagL = true;
		flagNL = false;
		// Force l'affichage de l'index tournant
		isLcdDisplayOn = true;
		// Affichage lcd sans timeout
		lcd.displayOn(); // dispplayOn set backLight
		// Temporise la durée d'affichage sur LCD
 		t_start(tache_t_backLight2);
		electricalPanelOpen = true;
	}
	else if (!doorState && (!flagNL || bootDisplayOff)) {
		bootDisplayOff = false;
		flagL = false;
		flagNL = true;
		electricalPanelOpen = false;
		backLightOff();
	}
  
	//----------------------------------------------------------------------------
	// Commande surpresseur sur fermeture contact supresseur
	// - Si le remplissage échoue la première fois
	//   (timout trop faible, usure ou pompe défectueuse),
	//   Une relance est possible par le bouton sur panneau elec, ou l'appli Android
	//------------------------------------------------------------------------------
	// Evite log en boucle;
	static boolean bSupressorDisLog;
	static boolean bSupressorEnLog;
	// Ouverture du contact supresseur	
	if (gpioState(I_SURPRESSEUR)) {
		#ifdef DEBUG_OUTPUT
		Serial.printf("msgRearm=%1d, startSupressorFilling=%1d, startSupressorFilling2=%1d\n",
						msgRearm, startSupressorFilling, startSupressorFilling2);
		#endif
		static boolean first = true;
		if (!msgRearm && first) {
			// Reset message TOPIC_DEFAUT_SUPRESSEUR
			first = false;
			// Message vers HA et appli Android
			mqttClient.publish(TOPIC_DEFAUT_SUPRESSOR, "off");
		}
		// Si le surpresseur est autorisé	
		if (cDlyParam->get(SUPRESSOR_EN)) {
			if (!bSupressorEnLog) {
				// Enregistrer inconditionnelement 
				logsWrite("Surpresseur activé");
				bSupressorEnLog = true;
				#ifdef DEBUG_OUTPUT
				Serial.println("Surpresseur activé");
				#endif
			}
			// Préparation mise en remplissage surpresseur 
			// Mode normal et relance manuelle
			if (!startSupressorFilling) {
				startSupressorFilling = true;
				bSupressorDisLog = false;
				// Si arrosage ou remplissage en cours, privilégier le supresseur
				// - Couper les EV (ne pas couper la pompe pour limiter l'usure)
				//   Nota : si les EV sont coupées, la pompe débite dans le surpresseur.
				//   Voir plan hydraulique			
				if (isTankFilling || isWatering) {
					isWatering = false;
					isTankFilling = false;
					t_stop(tache_t_tankFilling);
					t_stop(tache_t_watering);
					off(O_TRANSFO);
					off(O_EV_IRRIGATION);
					off(O_EV_ARROSAGE);
					if (isTankFilling)
						writeLogs("Arrèt remplissage réservoir");
					if (isWatering)
						writeLogs("Arrèt arrosage");
				}
				lcdSupressorWaitMsg();
				// Mise à jour dynamique du timeout
				setDelay(tache_t_surpressorFilling, cvrtic(cDlyParam->get(TIME_SUPRESSOR) * 1000));
				t_start(tache_t_surpressorFilling);
				// Publier pour HA et appli Android
				mqttClient.publish(TOPIC_DEFAUT_SUPRESSOR, "off");
				// Mettre systématiquement la pompe en route
				on(O_POMPE);

				// Gérer la sécurité du supresseur (nombre excessif de démarrages)
				if (cDlyParam->get(SURPRESSOR_SECURIT_EN)) {
					// Démarrage monostable la première fois
					if (!monoSurpressorSecurityStarted) {
						// Serial.println("t_start(tache_t_monoSurpressorSecurity)");
						t_start(tache_t_monoSurpressorSecurity);
						monoSurpressorSecurityStarted = true;
						n_supressorFillingInTime = 1;
					}
					// Incrémenter le compteur de démarrage les fois suivantes
					else {
						n_supressorFillingInTime++;
						// Serial.println("n_supressorFillingInTime++");
					}
				}
				writeLogs("Remplissage surpresseur");
			}
		}
		else if (!bSupressorDisLog) {
			#ifdef DEBUG_OUTPUT
			Serial.println("Surpresseur désactivé");
			#endif
			bSupressorDisLog = true;
			bSupressorEnLog = false;
			// Enregistrer inconditionnelement 
			logsWrite("Surpresseur desactivé");
		}	
		// Relance manuelle pompe sur timeout (monoSurpressorFilling coupe la pompe)
		// Une relance autorisée
		// Réarmement par bouton local panneau elec ou appli Android (msgRearm->true)
		if (msgRearm && startSupressorFilling && !startSupressorFilling2) {	
			msgRearm = false;
			first = true;
			startSupressorFilling = false;
			// Interdit une autre tentative
			startSupressorFilling2 = true;
			mqttClient.publish(TOPIC_DEFAUT_SUPRESSOR, "off");
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
		if (cDlyParam->get(SUPRESSOR_EN)) {
			ioDisplay();
			display = ioDisplay;
			writeLogs("Fin remplissage surpresseur");
		}
	}

	// Enregistrement de l'autorisation surpresseur (état off) en cas de mise 
	// en sécurité surpresseur (défaut canalisation)
	static unsigned surpressorSecurityFlag;
	if (supressorFillingSecurity && !surpressorSecurityFlag) {
		surpressorSecurityFlag = true;
		fileDlyParam->writeFile(cDlyParam->getStr(), "w");
		// Serial.println("supressorFillingSecurity && !surpressorSecurityFlag");
	}
	if (!supressorFillingSecurity && surpressorSecurityFlag) {
		surpressorSecurityFlag = false;
		// Serial.println("!supressorFillingSecurity && surpressorSecurityFlag");
	}

	// Mise à jour affichage toutes les 13s si SURPRESSOR_OFF
	// Reset message dans PubSubCallback->TOPIC_WRITE_DLY_PARAM
	static unsigned disFrequency;
	if (!cDlyParam->get(SUPRESSOR_EN) && (disFrequency++ % 128 == 0)) {
		// Serial.println("SURPRESSOR_OFF");
		lcdPrintString(SURPRESSOR_OFF, 1, 0, false);
	} 
	// Gestion des erreurs surpresseur.
	// Ne peut pas se faire dans monoSurpressorFilling (ISR)
	if (erreurSupresseur) {
		erreurSupresseur = false;
		// Déclenche mise à jour appli Android (menu relance)
		erreurSupresseurEvent = true;
		lcdSupressorFaultMsg();
		onSingleClick = reArm;
		writeLogs("Erreur surpresseur");
	}
	if (erreurPompe) {
		erreurPompe = false;
		// Déclenche mise à jour appli Android		
		erreurPompEvent = true;
		lcdPumpFault();
		onSingleClick = lcdReboot;
		writeLogs("Erreur pompe");
	}
	logsUpdate();
}
