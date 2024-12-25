#include "espNow.hpp"

EspNow* EspNow::instance = nullptr;  // Initialisation de l'instance statique

Portal portal;

// Callback pour la réception des messages
void EspNow::onReceive(const uint8_t* mac, const uint8_t* incomingData, int len) {
  if (instance == nullptr) return;  // Vérification de l'instance
  if (len != sizeof(DiscoveryMessage)) {
    DEBUG_ERROR("Message reçu avec une taille incorrecte !");
    return;
  }

  DiscoveryMessage receivedMessage;
  memcpy(&receivedMessage, incomingData, len);

  if (xQueueSend(instance->messageQueue, &receivedMessage, 0) == pdTRUE) {
    DEBUG_INFO("Message reçu et ajouté à la file de tâches.");
  } else {
    DEBUG_WARN("File de tâches pleine. Message ignoré.");
  }
}

void EspNow::sendDiscovery(const char* role) {
  DiscoveryMessage message;
  strncpy(message.role, role, sizeof(message.role) - 1);
  message.role[sizeof(message.role) - 1] = '\0';  // Sécurité
  WiFi.macAddress(message.macAddr);

  const uint8_t broadcastAddress[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)&message, sizeof(message));
  if (result == ESP_OK) {
    DEBUG_INFO("Message %s envoyé avec succès !\n", role);
  } else {
    DEBUG_ERROR("Erreur d'envoi du message %s : %d\n", role, result);
  }
}

void EspNow::addSlave(const uint8_t* mac) {
  if (slaveCount < MAX_SLAVES) {
    memcpy(slaveMacAddresses[slaveCount], mac, 6);
    slaveCount++;
#if DEBUG_LEVEL >= DEBUG_LEVEL_INFO
    Serial.print("[DEBUG] [INFO] Slave ajouté : ");
    for (int i = 0; i < 6; i++) {
      Serial.printf("%02X", mac[i]);
      if (i < 5) Serial.print(":");
    }
    Serial.println();
#endif
  } else {
    DEBUG_ERROR("Liste des Slaves pleine.");
  }
}

void EspNow::processMessages(void* parameter) {
  if (instance == nullptr) return;  // Vérification de l'instance
  DiscoveryMessage receivedMessage;

  while (true) {
    if (xQueueReceive(instance->messageQueue, &receivedMessage, portMAX_DELAY) == pdTRUE) {
      DEBUG_INFO("--- Message reçu ---");
#if DEBUG_LEVEL >= DEBUG_LEVEL_INFO
      Serial.print("Adresse MAC de l'expéditeur : ");
      for (int i = 0; i < 6; i++) {
        Serial.printf("%02X", receivedMessage.macAddr[i]);
        if (i < 5) Serial.print(":");
      }
      Serial.println();
#endif
      DEBUG_INFO("Rôle : %s\n", receivedMessage.role);

      if (strcmp(receivedMessage.role, "SEARCHING") == 0 && isMaster) {
        DEBUG_INFO("Un nouvel ESP m'a trouvé. Je réponds en tant que MASTER.");
        instance->sendDiscovery("MASTER");
      } else if (strcmp(receivedMessage.role, "MASTER") == 0 && !roleLocked) {
        DEBUG_INFO("MASTER détecté. Je deviens SLAVE.");
        isMaster = false;
        roleLocked = true;
        memcpy(instance->masterMacAddress, receivedMessage.macAddr, 6);
        instance->sendDiscovery("SLAVE");
      } else if (strcmp(receivedMessage.role, "SLAVE") == 0 && isMaster) {
        instance->addSlave(receivedMessage.macAddr);
      }
    }
  }
}

void EspNow::Setup() {
  instance = this;  // Enregistrement de l'instance

  WiFi.mode(WIFI_AP_STA);  // AP (acces point) pour le serveur + STA (sation) esp-Now
  DEBUG_INFO("WiFi initialisé en mode station.");

  if (esp_now_init() != ESP_OK) {
    DEBUG_ERROR("Erreur lors de l'initialisation d'ESP-NOW.");
    return;
  }
  DEBUG_INFO("ESP-NOW initialisé avec succès.");

  esp_now_register_recv_cb(onReceive);

  const uint8_t broadcastAddress[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    DEBUG_ERROR("Erreur lors de l'ajout du pair pour la diffusion.");
    return;
  }

  messageQueue = xQueueCreate(MESSAGE_QUEUE_SIZE, sizeof(DiscoveryMessage));
  if (messageQueue == NULL) {
    DEBUG_ERROR("Erreur : Impossible de créer la file de tâches.");
    return;
  }

  xTaskCreatePinnedToCore(processMessages, "MessageProcessor", 4096, NULL, 1, NULL, 1);
  delay(random(100, 500));
  sendDiscovery("SEARCHING");

  unsigned long startTime = millis();
  while (millis() - startTime < discoveryTimeout) {
    delay(100);
    if (roleLocked) return;
  }

  if (!roleLocked) {
    DEBUG_INFO("Aucun autre ESP détecté. Je deviens MASTER.");
    isMaster = true;
    roleLocked = true;
    sendDiscovery("MASTER");
  }

  // Portail captif si master
  if (roleLocked && isMaster) {
    WiFi.softAP(SSID, PASSWORD, 1);
    portal.Setup();
  }
}
