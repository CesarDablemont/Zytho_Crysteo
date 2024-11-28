#include "portal.hpp"

// Définitions des variables globales
WebServer server(80);  // Serveur web sur le port 80
DNSServer dnsServer;   // Serveur DNS

void Portal::Setup() {
  IPAddress apIP = WiFi.softAPIP();
  Serial.println("Point d'accès WiFi créé !");
  Serial.print("Adresse IP : ");
  Serial.println(apIP);

  // Configuration du serveur DNS
  dnsServer.start(DNS_PORT, "*", apIP);  // Redirige toutes les requêtes DNS vers l'ESP32

  // Configuration de mDNS
  if (MDNS.begin("zytho")) {  // Définir le sous-domaine "zytho"
    Serial.println("mDNS démarré : http://zytho.local/");
  } else {
    Serial.println("Erreur : mDNS non démarré.");
  }

  // Route pour afficher le leaderboard
  server.on("/", afficherLeaderboard);

  // Redirige toutes les requêtes inconnues vers la page principale
  server.onNotFound([]() {
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "Redirection vers le leaderboard");
  });

  // Démarrage du serveur
  server.begin();
  Serial.println("Serveur web démarré.");
}

void Portal::afficherLeaderboard() {
  String page = "<!DOCTYPE html><html><head><title>Leaderboard</title></head><body>";
  page += "<h1>Leaderboard</h1>";
  page += "<table border='1'><tr><th>Pseudo</th><th>Temps</th></tr>";

  File fichier = SD.open("/leaderboard.csv");
  if (fichier) {
    // Lire chaque ligne et l'ajouter au tableau HTML
    while (fichier.available()) {
      String ligne = fichier.readStringUntil('\n');
      int virgule = ligne.indexOf(',');
      if (virgule > 0) {
        String pseudo = ligne.substring(0, virgule);
        String temps = ligne.substring(virgule + 1);
        page += "<tr><td>" + pseudo + "</td><td>" + temps + "</td></tr>";
      }
    }
    fichier.close();
  } else {
    page += "<tr><td colspan='2'>Erreur : impossible de lire le fichier.</td></tr>";
  }

  page += "</table></body></html>";

  // Envoi de la page web
  server.send(200, "text/html", page);
}