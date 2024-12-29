#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstddef>
#include <cstdint>

#include "debugger.hpp"

// ----------------------------------------------------------------------
// Communication ESP
#define DISCOVERY_TIMEOUT 5000
#define MESSAGE_QUEUE_SIZE 10
#define MAX_SLAVES 10
#define WIFI_CHANNEL 1

// Structure pour les messages ESP-NOW
enum class MessageType : uint8_t { DISCOVERY = 1, TIME = 2 };

struct __attribute__((packed)) GenericMessage {
  MessageType type;    // Type du message (DISCOVERY, TIME, etc.)
  uint8_t data[64];    // Tableau générique pour stocker les données du message
  uint8_t macAddr[6];  // Adresse MAC de l'expéditeur
};

struct __attribute__((packed)) DiscoveryMessage {
  MessageType type = MessageType::DISCOVERY;  // Type de message
  char role[32];                              // Rôle : "MASTER", "SLAVE", etc.
  uint8_t macAddr[6];                         // Adresse MAC de l'expéditeur
};

struct __attribute__((packed)) TimeMessage {
  MessageType type = MessageType::TIME;  // Type de message
  float time;                            // Temps en secondes
  uint8_t macAddr[6];                    // Adresse MAC de l'expéditeur
};

extern bool isMaster;
extern bool roleLocked;

// ----------------------------------------------------------------------
// portail captif
#define SSID "Zytho Crysteo !"
#define PASSWORD "pierroux"
#define DNS_PORT 53
#define DNS_NAME "zytho"

// ----------------------------------------------------------------------
// Pin CS pour la carte SD
#define CS_PIN 5

#endif  // UTILS_HPP