#ifndef ESP_NOW_HPP
#define ESP_NOW_HPP

#include <WiFi.h>
#include <esp_now.h>

#include "utils.hpp"
#include "portal.hpp"

extern Portal portal;

class EspNow {
 public:
  EspNow() {};

  void Setup();
  void sendDiscovery(const char* role);
  void addSlave(const uint8_t* mac);

  static void onReceive(const uint8_t* mac, const uint8_t* incomingData, int len);
  static void processMessages(void* parameter);

  // Liste des adresses MAC des Slaves (pour le Master)
  uint8_t slaveMacAddresses[MAX_SLAVES][6] = {};
  int slaveCount = 0;

  // Adresse du Master pour le Slave
  uint8_t masterMacAddress[6] = {};

  QueueHandle_t messageQueue = nullptr;  // File de messages

  static EspNow* instance;  // Instance statique pour accéder aux membres
};

#endif  // ESP_NOW_HPP
