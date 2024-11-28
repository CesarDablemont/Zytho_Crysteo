#include <WiFi.h>
#include <esp_now.h>

#include "espNow.hpp"
#include "portal.hpp"
#include "sd.hpp"
#include "utils.hpp"

EspNow espNow;
Sd sd;

void setup() {
  Serial.begin(115200);
  espNow.Setup();
  sd.Setup();
}

void loop() {
  dnsServer.processNextRequest();  // Gérer les requêtes DNS
  server.handleClient();           // Gérer les requêtes HTTP

  // Serial.println("Début de la boucle principale.");

  // // Affichage des informations
  // if (isMaster) {
  //   Serial.println("Je suis MASTER");
  //   Serial.println("Liste des Slaves : ");
  //   for (int i = 0; i < espNow.slaveCount; i++) {
  //     Serial.print("Slave ");
  //     for (int j = 0; j < 6; j++) {
  //       Serial.printf("%02X", espNow.slaveMacAddresses[i][j]);
  //       if (j < 5) Serial.print(":");
  //     }
  //     Serial.println();
  //   }
  // } else {
  //   Serial.println("Je suis SLAVE");
  //   Serial.print("Adresse MAC du MASTER : ");
  //   for (int i = 0; i < 6; i++) {
  //     Serial.printf("%02X", espNow.masterMacAddress[i]);
  //     if (i < 5) Serial.print(":");
  //   }
  //   Serial.println();
  // }

  // delay(3000);  // Pause de 3 secondes
  // Serial.println("Fin de la boucle principale.");
}
