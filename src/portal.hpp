#ifndef Portal_HPP
#define Portal_HPP

#include <DNSServer.h>
#include <ESPmDNS.h>
#include <SD.h>
#include <WebServer.h>

#include <queue>

#include "sd.hpp"
#include "utils.hpp"

// Déclarations des objets externes
extern WebServer server;     // Serveur web sur le port 80
extern DNSServer dnsServer;  // Serveur DNS
extern Sd sd;                // Gestionnaire de carte SD

class Portal {
 public:
  Portal();

  void Setup();
  void ajouterTempsEnAttente(float temps);

 private:
  static std::queue<String> pendingQueue;  // File d'attente statique

  // Méthodes de gestion des routes (doivent être statiques pour fonctionner avec `server.on`)
  static void afficherPageIndex();
  static void servirFichierCSS();
  static void servirFichierJS();
  static void servirFichierCSV();
  static void gererPending();
  static void soumettrePseudo();
  static void ignorerTemps();
};

#endif  // Portal_HPP
