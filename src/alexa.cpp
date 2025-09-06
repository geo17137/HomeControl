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

void addDevices(){
  fauxmo.addDevice(CUISINE);
  fauxmo.addDevice(LOW_VMC);
  fauxmo.addDevice(FAST_VMC);
  fauxmo.addDevice(PROG_VMC);
  fauxmo.addDevice(LANCE_ARROSAGE);

  fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
      
  // Rappel lorsqu'une commande Alexa est reçue.
  // Vous pouvez utiliser device_id ou device_name pour choisir l'élément sur lequel effectuer une action (relais, LED, etc.).
  // L'état est un booléen (ON/OFF) et la valeur un nombre compris entre 0 et 255 (si vous dites « mettre l'éclairage de la cuisine à 50 % », vous recevrez un 128).
  // Attention à ne pas trop tarder : il s'agit d'un rappel. Quittez dès que possible.
  // Si vous devez effectuer une action plus complexe, définissez un indicateur et traitez-le dans votre boucle principale..
        
  // Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);
  // La vérification de l'identifiant de l'appareil est plus simple si vous êtes certain de l'ordre dans lequel ils sont chargés et qu'il ne change pas.
  // Sinon, la comparaison du nom de l'appareil est plus sûre.
    if (strcmp(device_name, CUISINE)==0) {
      if (state) {
        // Serial.printf("[VMC] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);      
        on(O_FOUR);
        mqttClient.publish(TOPIC_STATUS_CUISINE, "on");
      }
      else {
        // Serial.printf("[VMC] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);      
        off(O_FOUR);
        mqttClient.publish(TOPIC_STATUS_CUISINE, "off");   
      }
    } 
    else if (strcmp(device_name, LOW_VMC)==0) {
      if (state) {
        // Serial.printf("[VMC] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);
        setVmc(3);
      }
      else {
      //  Serial.printf("[VMC] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);
       setVmc(0);
      }    
    } 
    else if (strcmp(device_name, FAST_VMC)==0) {
      if (state) {
        // Serial.printf("[FAST] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);
        setVmc(2);
      }
      else {
        // Serial.printf("[FAST] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);
        setVmc(0);
      }      
    } 
    else if (strcmp(device_name, PROG_VMC)==0) {
      if (state) {
        // Serial.printf("[PROG] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);
        setVmc(1);
      }
      else {
        // Serial.printf("[PROG] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);
        setVmc(0);
      }      
    } 
    else if (strcmp(device_name, LANCE_ARROSAGE)==0) {
      if (state) {
        // Serial.printf("[LANCE_ARROSAGE] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);
        startWatering(TIMEOUT);
      }
      else {
        // Serial.printf("[LANCE_ARROSAGE] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);
        stopWatering();
      }      
      wateringNoTimeOut = 0;
    } 
  });
}
#endif
