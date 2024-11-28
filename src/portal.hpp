#ifndef Portal_HPP
#define Portal_HPP

#include <DNSServer.h>
#include <ESPmDNS.h>
#include <SD.h>
#include <WebServer.h>

#include "utils.hpp"

extern WebServer server;  // Serveur web sur le port 80
extern DNSServer dnsServer;   // Serveur DNS

class Portal {
 public:
  Portal() {};

  void Setup();
  static void afficherLeaderboard();
};

#endif  // Portal_HPP