#include "alexa.h"

#ifdef ALEXA

void initAlexa() {
  // By default, fauxmoESP creates it's own webserver on the defined port
  // The TCP port must be 80 for gen3 devices (default is 1901)
  // This has to be done before the call to enable()
  fauxmo.createServer(true); // not needed, this is the default value
  fauxmo.setPort(80); // This is required for gen3 devices
  // You have to call enable(true) once you have a WiFi connection
  // You can enable or disable the library at any moment
  // Disabling it will prevent the devices from being discovered and switched
  fauxmo.enable(true);
}

void cuisine() {
  if (_state) {
    on(O_FOUR);
    mqttClient.publish(TOPIC_STATUS_CUISINE, "on");
    return;
  }
  off(O_FOUR);
  mqttClient.publish(TOPIC_STATUS_CUISINE, "off");
}

void vmc() {
  if (_state) {
    if (_value < 130 && _value > 100) {
      // Marche forcée lente
      setVmc(CMD_VMC_SLOW);      
      return;
    }
    if (_value > 240) {
      // Marche forcée rapide
      setVmc(CMD_VMC_FAST);      
      return;
    }
    if (_value < 100) {
      setVmc(CMD_VMC_PROG);      
      return;
    }
  }
  setVmc(0);
}

void arrosage() {
  if (_state) {
    startWatering(TIMEOUT);
    wateringNoTimeOut = 0;
    return;
  }
  stopWatering();
}

void addDevices() {
  // Ne pas modifier l'ordre
  fauxmo.addDevice(S_CUISINE);
  fauxmo.addDevice(S_VMC);
  fauxmo.addDevice(S_LANCE_ARROSAGE);

  fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
      
  // Rappel lorsqu'une commande Alexa est reçue.
  // Vous pouvez utiliser device_id ou device_name pour choisir l'élément sur lequel effectuer une action (relais, LED, etc.).
  // L'état est un booléen (ON/OFF) et la valeur un nombre compris entre 0 et 255 (si vous dites « mettre l'éclairage de la cuisine à 50 % », vous recevrez un 128).
  // Attention à ne pas trop tarder : il s'agit d'un rappel. Quittez dès que possible.
  // Si vous devez effectuer une action plus complexe, définissez un indicateur et traitez-le dans votre boucle principale.
        
  // Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);
  // La vérification de l'identifiant de l'appareil est plus simple si vous êtes certain de l'ordre dans lequel ils sont chargés et qu'il ne change pas.
  // Sinon, la comparaison du nom de l'appareil est plus sûre.

    _state = state;
    _value = value;
    // Par prudence on vérifie l'indice
    if (device_id < size)
      funcToCall[device_id]();
  });
}
#endif
