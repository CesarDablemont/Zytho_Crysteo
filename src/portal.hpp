#ifndef Portal_HPP
#define Portal_HPP

#include <DNSServer.h>
#include <ESPmDNS.h>
#include <SD.h>
#include <WebServer.h>

#include "utils.hpp"
#include "sd.hpp"

extern WebServer server;  // Serveur web sur le port 80
extern DNSServer dnsServer;   // Serveur DNS
extern Sd sd;

class Portal {
 public:
  Portal() {};

  void Setup();
  static void afficherLeaderboard();
  void ajouterTempsEnAttente(float temps);
};

#endif  // Portal_HPP