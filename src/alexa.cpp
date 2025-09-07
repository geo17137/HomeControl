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
  if (state) {
    on(O_FOUR);
    mqttClient.publish(TOPIC_STATUS_CUISINE, "on");
    return;
  }
  off(O_FOUR);
  mqttClient.publish(TOPIC_STATUS_CUISINE, "off");
}

void lowVmc() {
  if (state) {
    setVmc(3);
    return;
  }
  setVmc(0);
}

void fastVmc() {
  if (state) {
    setVmc(2);
    return;
  }
  setVmc(0);
}

void vmcProg() {
  if (state) {
    setVmc(1);
    return;
  }
  setVmc(0);
}

void arrosage() {
  if (state) {
    startWatering(TIMEOUT);
    wateringNoTimeOut = 0;
    return;
  }
  stopWatering();
}

void addDevices() {
  // Ne pas modifier l'ordre
  fauxmo.addDevice(S_CUISINE);
  fauxmo.addDevice(S_LOW_VMC);
  fauxmo.addDevice(S_FAST_VMC);
  fauxmo.addDevice(S_VMC_PROG);
  fauxmo.addDevice(S_LANCE_ARROSAGE);

  fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool p_state, unsigned char value) {
      
  // Rappel lorsqu'une commande Alexa est reçue.
  // Vous pouvez utiliser device_id ou device_name pour choisir l'élément sur lequel effectuer une action (relais, LED, etc.).
  // L'état est un booléen (ON/OFF) et la valeur un nombre compris entre 0 et 255 (si vous dites « mettre l'éclairage de la cuisine à 50 % », vous recevrez un 128).
  // Attention à ne pas trop tarder : il s'agit d'un rappel. Quittez dès que possible.
  // Si vous devez effectuer une action plus complexe, définissez un indicateur et traitez-le dans votre boucle principale..
        
  // Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);
  // La vérification de l'identifiant de l'appareil est plus simple si vous êtes certain de l'ordre dans lequel ils sont chargés et qu'il ne change pas.
  // Sinon, la comparaison du nom de l'appareil est plus sûre.

    state = p_state;
    // Par prudence on vérifie l'indice
    if (device_id < size)
      funcToCall[device_id]();

    // switch (device_id) {
    // case cuisine:
    //   if (state) {
    //     on(O_FOUR);
    //     mqttClient.publish(TOPIC_STATUS_CUISINE, "on");
    //   }
    //   else {
    //     off(O_FOUR);
    //     mqttClient.publish(TOPIC_STATUS_CUISINE, "off");
    //   }
    //   break;
    // case low_vmc:
    //   if (state) {
    //     setVmc(3);
    //   }
    //   else {
    //     setVmc(0);
    //   }
    //   break;
    // case fast_vmc:
    //   if (state) {
    //     setVmc(2);
    //   }
    //   else {
    //     setVmc(0);
    //   }
    //   break;
    // case prog_vmc:
    //   if (state) {
    //     setVmc(1);
    //   }
    //   else {
    //     setVmc(0);
    //   }
    //   break;
    // case lance_arrosage:
    //   if (state) {
    //     startWatering(TIMEOUT);
    //   }
    //   else {
    //     stopWatering();
    //   }
    //   wateringNoTimeOut = 0;
    //   break;
    // default:
    //   break;
    // }
  });
}
#endif
