#ifndef ESP_NOW_HPP
#define ESP_NOW_HPP

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

#include "portal.hpp"
#include "utils.hpp"

extern Portal portal;

class EspNow {
 public:
  EspNow() {};

  void Setup();
  void sendDiscovery(const char* role);
  void sendTime(const float time);

  int findSlave(const uint8_t* mac);
  void addSlave(const uint8_t* mac);
  void addMasterPeer();

  static void onReceive(const uint8_t* mac, const uint8_t* incomingData, int len);
  static void processMessages(void* parameter);

  uint8_t slaveMacAddresses[MAX_SLAVES][6] = {};  // Liste des adresses MAC des Slaves (pour le Master)
  int slaveCount = 0;
  uint8_t masterMacAddress[6] = {};  // Adresse du Master pour le Slave

  QueueHandle_t messageQueue = nullptr;  // File de messages

  static EspNow* instance;  // Instance statique pour accéder aux membres
};

#endif  // ESP_NOW_HPP
