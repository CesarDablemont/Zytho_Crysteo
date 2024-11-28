#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstddef>
#include <cstdint>

// ----------------------------------------------------------------------
// Communication ESP
#define MESSAGE_QUEUE_SIZE 10
#define discoveryTimeout 5000
#define MAX_SLAVES 10

// Structure pour les messages ESP-NOW
struct __attribute__((packed)) DiscoveryMessage {
  char role[20];       // Rôle : "MASTER", "SLAVE", ou "SEARCHING"
  uint8_t macAddr[6];  // Adresse MAC de l'expéditeur
};

extern bool isMaster;
extern bool roleLocked;

// ----------------------------------------------------------------------
// portail captif
#define SSID "Zytho Crysteo !"
#define PASSWORD "pierroux"
#define DNS_PORT 53

// ----------------------------------------------------------------------
// Pin CS pour la carte SD
#define CS_PIN 5

#endif  // UTILS_HPP