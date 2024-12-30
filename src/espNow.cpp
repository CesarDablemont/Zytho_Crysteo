#include "espNow.hpp"

EspNow* EspNow::instance = nullptr;  // Initialisation de l'instance statique
Portal portal;

// Callback de réception
void EspNow::onReceive(const uint8_t* mac, const uint8_t* incomingData, int len) {
  if (instance == nullptr) {
    DEBUG_ERROR("Instance ESP-NOW non initialisée !");
    return;
  }

  DEBUG_ESP_NOW("Message reçu - Taille : %d", len);

  if (len < sizeof(MessageType)) {
    DEBUG_ERROR("Message trop court pour contenir un type valide !");
    return;
  }

  GenericMessage msg;
  msg.type = *(MessageType*)incomingData;
  memcpy(msg.macAddr, mac, 6);

#if DEBUG_LEVEL >= DEBUG_LEVEL_DEBUG
  // Afficher l'adresse MAC de l'expéditeur
  Serial.printf("[DEBUG] [ESP_NOW] Adresse MAC de l'expéditeur : ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", mac[i]);
    if (i < 5) Serial.print(":");
  }
  Serial.println();
#endif

  // Copier les données restantes dans le buffer générique
  int dataSize = len - sizeof(MessageType);
  if (dataSize > sizeof(msg.data)) {
    DEBUG_ERROR("Données du message trop volumineuses !");
    return;
  }
  memcpy(msg.data, incomingData + sizeof(MessageType), dataSize);

  // Essayer d'ajouter le message à la file
  if (xQueueSend(instance->messageQueue, &msg, 0) == pdTRUE) {
    DEBUG_ESP_NOW("Message ajouté à la file de traitement.");
  } else {
    DEBUG_WARN("File pleine. Message ignoré.");
  }
}

// Envoi de messages DISCOVERY
void EspNow::sendDiscovery(const char* role) {
  DiscoveryMessage message;
  message.type = MessageType::DISCOVERY;
  strncpy(message.role, role, sizeof(message.role) - 1);
  message.role[sizeof(message.role) - 1] = '\0';  // Sécuriser la fin de la chaîne
  WiFi.macAddress(message.macAddr);

  DEBUG_INFO("Envoi de DISCOVERY - Rôle : %s", role);
  const uint8_t broadcastAddress[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)&message, sizeof(message));

  switch (result) {
    case ESP_OK: DEBUG_INFO("Message DISCOVERY envoyé avec succès."); break;
    case ESP_ERR_ESPNOW_NOT_INIT: DEBUG_ERROR("ESP-NOW n'est pas initialisé."); break;
    case ESP_ERR_ESPNOW_ARG: DEBUG_ERROR("Paramètres invalides dans l'envoi."); break;
    case ESP_ERR_ESPNOW_INTERNAL: DEBUG_ERROR("Erreur interne ESP-NOW."); break;
    case ESP_ERR_ESPNOW_NO_MEM: DEBUG_ERROR("Pas assez de mémoire pour l'envoi."); break;
    case ESP_ERR_ESPNOW_NOT_FOUND: DEBUG_ERROR("Peer non trouvé."); break;
    case ESP_ERR_ESPNOW_IF: DEBUG_ERROR("Erreur d'interface réseau."); break;
    default: DEBUG_ERROR("Erreur inconnue : %d", result); break;
  }
}

// Envoi de messages TIME
void EspNow::sendTime(float timeValue) {
  TimeMessage message;
  message.time = timeValue;
  WiFi.macAddress(message.macAddr);

  DEBUG_ESP_NOW("Envoi de TIME - Temps : %.3f secondes", timeValue);

  addMasterPeer();  // Ajout du MASTER en tant que peer si nécessaire

  esp_err_t result = esp_now_send(masterMacAddress, (uint8_t*)&message, sizeof(message));

  switch (result) {
    case ESP_OK: DEBUG_INFO("Message TIME envoyé avec succès (temps: %.3f).", timeValue); break;
    case ESP_ERR_ESPNOW_NOT_INIT: DEBUG_ERROR("ESP-NOW n'est pas initialisé."); break;
    case ESP_ERR_ESPNOW_ARG: DEBUG_ERROR("Paramètres invalides dans l'envoi TIME."); break;
    case ESP_ERR_ESPNOW_INTERNAL: DEBUG_ERROR("Erreur interne ESP-NOW lors de l'envoi TIME."); break;
    case ESP_ERR_ESPNOW_NO_MEM: DEBUG_ERROR("Pas assez de mémoire pour envoyer le message TIME."); break;
    case ESP_ERR_ESPNOW_NOT_FOUND: DEBUG_ERROR("Peer MASTER non trouvé pour envoyer le message TIME."); break;
    case ESP_ERR_ESPNOW_IF: DEBUG_ERROR("Erreur d'interface réseau pour l'envoi TIME."); break;
    default: DEBUG_ERROR("Erreur inconnue lors de l'envoi TIME : %d", result); break;
  }
}

int EspNow::findSlave(const uint8_t* mac) {
  for (int i = 0; i < slaveCount; i++) {
    if (memcmp(slaveMacAddresses[i], mac, 6) == 0) {
      return i;  // L'adresse MAC existe déjà, retourne l'index.
    }
  }
  return -1;  // L'adresse MAC n'existe pas, retourne -1.
}

// Ajout d'un Slave
void EspNow::addSlave(const uint8_t* mac) {
  if (slaveCount < MAX_SLAVES) {
    if (findSlave(mac) == -1) {
      memcpy(slaveMacAddresses[slaveCount], mac, 6);
      slaveCount++;
      DEBUG_INFO("Slave ajouté avec succès.");

#if DEBUG_LEVEL >= DEBUG_LEVEL_DEBUG
      // Affichage des données brutes reçues pour déboguer
      Serial.println("[DEBUG] [ESP_NOW] Lites des slaves : ");
      for (int i = 0; i < slaveCount; i++) {
        Serial.printf("[DEBUG] [ESP_NOW] Slaves #%d : ", i);
        for (int j = 0; j < 6; j++) {
          Serial.printf("%02X ", slaveMacAddresses[i][j]);  // Affiche les données sous forme hexadécimale
        }
        Serial.println();
      }
#endif

    } else {
      DEBUG_WARN("Adresse MAC déjà présente dans la liste des Slaves. Ignorée.");
    }

  } else {
    DEBUG_ERROR("Liste des Slaves pleine.");
  }
}

void EspNow::addMasterPeer() {
  esp_now_peer_info_t peerInfo = {};
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, masterMacAddress, 6);
  peerInfo.channel = WIFI_CHANNEL;
  peerInfo.encrypt = false;

  if (!esp_now_is_peer_exist(masterMacAddress)) {
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      DEBUG_ERROR("Échec de l'ajout du peer MASTER.");
    } else {
      DEBUG_ESP_NOW("Peer MASTER ajouté avec succès.");
    }
  } else {
    DEBUG_ESP_NOW("Peer MASTER existe déjà.");
  }
}

// Traitement des messages
void EspNow::processMessages(void* parameter) {
  if (instance == nullptr) return;

  GenericMessage msg;

  DEBUG_ESP_NOW("Tâche processMessages démarrée.");

  while (true) {
    if (xQueueReceive(instance->messageQueue, &msg, portMAX_DELAY) == pdTRUE) {
      DEBUG_ESP_NOW("Message extrait de la file - Type : %d", msg.type);

#if DEBUG_LEVEL >= DEBUG_LEVEL_DEBUG
      // Affichage des données brutes reçues pour déboguer
      Serial.print("[DEBUG] [ESP_NOW] Données brutes reçues : ");
      for (int i = 0; i < sizeof(msg.data); i++) {
        Serial.printf("%02X ", msg.data[i]);  // Affiche les données sous forme hexadécimale
      }
      Serial.println();
#endif

      switch (msg.type) {
        case MessageType::DISCOVERY: {
          DiscoveryMessage* discoveryMsg = (DiscoveryMessage*)msg.data;

          // Afficher la chaîne brute qui correspond à `role`
          char rawRole[32] = {0};
          memcpy(rawRole, msg.data, 32);  // Copier les 32 premiers octets dans rawRole
          DEBUG_ESP_NOW("Rôle brut extrait : %s", rawRole);

          // Extraction du rôle
          strncpy(discoveryMsg->role, rawRole, sizeof(discoveryMsg->role) - 1);
          discoveryMsg->role[sizeof(discoveryMsg->role) - 1] = '\0';  // Assurez-vous que la chaîne est bien terminée

          // Vérification de la validité du rôle
          if (strlen(discoveryMsg->role) < 1) {
            DEBUG_ERROR("Rôle reçu invalide !");
          } else {
            DEBUG_ESP_NOW("Rôle reçu : %s", discoveryMsg->role);
          }

          // Traitement basé sur le rôle
          if (strcmp(discoveryMsg->role, "SEARCHING") == 0 && isMaster) {
            DEBUG_INFO("Réponse en tant que MASTER.");
            instance->sendDiscovery("MASTER");

          } else if (strcmp(discoveryMsg->role, "MASTER") == 0 && !roleLocked) {
            DEBUG_INFO("MASTER détecté. Deviens SLAVE.");
            isMaster = false;
            roleLocked = true;
            memcpy(instance->masterMacAddress, msg.macAddr, 6);
            instance->sendDiscovery("SLAVE");

          } else if (strcmp(discoveryMsg->role, "SLAVE") == 0 && isMaster) {
            DEBUG_INFO("Ajout d'un nouvel SLAVE.");
            instance->addSlave(msg.macAddr);
          }
          break;
        }

        case MessageType::TIME: {
          if (sizeof(float) > sizeof(msg.data)) {
            DEBUG_ERROR("Données TIME invalides : taille insuffisante pour un float.");
            return;
          }

          float receivedTime;
          memcpy(&receivedTime, msg.data, sizeof(float));

          DEBUG_INFO("[TIME] Temps reçu : %.3f secondes", receivedTime);
          // portal.ajouterTempsEnAttente(receivedTime, findSlave(msg.macAddr));
          portal.ajouterTempsEnAttente(receivedTime, instance->findSlave(msg.macAddr));

#if DEBUG_LEVEL >= DEBUG_LEVEL_DEBUG
          // Affichage de l'adresse MAC du message TIME
          Serial.print("[DEBUG] [ESP_NOW] Adresse MAC de TIME : ");
          for (int i = 0; i < 6; i++) {
            Serial.printf("%02X", msg.macAddr[i]);
            if (i < 5) Serial.print(":");
          }
          Serial.println();
#endif

          break;
        }

        default: DEBUG_WARN("Type de message inconnu : %d", msg.type); break;
      }
    }
  }
}

// Configuration initiale
void EspNow::Setup() {
  instance = this;

  WiFi.mode(WIFI_STA);  // Mode station pour ESP-NOW
  esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);

  DEBUG_INFO("Démarrage de l'initialisation d'ESP-NOW...");
  if (esp_now_init() != ESP_OK) {
    DEBUG_ERROR("Erreur lors de l'initialisation d'ESP-NOW.");
    return;
  }
  DEBUG_INFO("ESP-NOW initialisé avec succès.");

  esp_now_register_recv_cb(onReceive);

  // Ajouter un peer pour le broadcast
  esp_now_peer_info_t peerInfo = {};
  memset(&peerInfo, 0, sizeof(esp_now_peer_info_t));
  const uint8_t broadcastAddress[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = WIFI_CHANNEL;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    DEBUG_ERROR("Erreur lors de l'ajout du peer de diffusion.");
    return;
  }
  DEBUG_INFO("Peer de diffusion ajouté avec succès.");

  // Création de la file de messages
  messageQueue = xQueueCreate(MESSAGE_QUEUE_SIZE, sizeof(GenericMessage));
  if (messageQueue == NULL) {
    DEBUG_ERROR("Erreur : Impossible de créer la file de messages.");
    return;
  }

  // Démarrage de la tâche de traitement des messages
  xTaskCreatePinnedToCore(processMessages, "MessageProcessor", 4096, NULL, 2, NULL, 1);  // Priorité augmentée
  DEBUG_INFO("Tâche de traitement des messages lancée.");

  // Envoi de DISCOVERY et attente de réponses
  sendDiscovery("SEARCHING");

  unsigned long startTime = millis();
  while (millis() - startTime < DISCOVERY_TIMEOUT) {
    delay(100);  // Délai pour attendre une réponse
    if (roleLocked) {
      return;  // Si le rôle est bloqué, sortir de la boucle
    }
  }

  // Si aucun autre ESP n'a répondu dans le délai, se déclarer MASTER
  if (!roleLocked) {
    DEBUG_INFO("Aucun autre ESP détecté. Je deviens MASTER.");
    isMaster = true;
    roleLocked = true;
    // sendDiscovery("MASTER");
  }

  // Portail captif si master
  if (roleLocked && isMaster) {
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(SSID, PASSWORD, WIFI_CHANNEL);
    DEBUG_INFO("Portail captif démarré sur le canal : %d", WIFI_CHANNEL);

    portal.Setup();
  }
}
