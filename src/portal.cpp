#include "portal.hpp"

// Déclaration des objets externes
WebServer server(80);
DNSServer dnsServer;
Sd sd;

// File d'attente partagée (statique)
std::queue<PendingTime> Portal::pendingQueue;

Portal::Portal() {}

void listerFichiersSD() {
  File root = SD.open("/");
  if (!root) {
    DEBUG_ERROR("Erreur : Impossible d'ouvrir le répertoire racine.");
    return;
  }
  File fichier = root.openNextFile();
  while (fichier) {
    DEBUG_PORTAL("Fichier trouvé : %s", fichier.name());
    fichier = root.openNextFile();
  }
}

void Portal::Setup() {
  // Initialisation du WiFi
  IPAddress apIP = WiFi.softAPIP();
  DEBUG_INFO("Point d'accès WiFi créé !");
  DEBUG_INFO("Adresse IP : %s", apIP.toString().c_str());

  // Initialisation de la carte SD
  sd.Setup();
  listerFichiersSD();  // Vérifie les fichiers disponibles sur la SD

  // Configuration DNS
  dnsServer.start(DNS_PORT, "*", apIP);
  if (MDNS.begin(DNS_NAME)) {
    DEBUG_INFO("mDNS démarré : http://%s.local/", DNS_NAME);
  } else {
    DEBUG_ERROR("Erreur : mDNS non démarré.");
  }

  // Routes HTTP
  server.on("/", [this]() {
    DEBUG_PORTAL("Requête reçue pour /");
    this->afficherPageIndex();
  });

  server.on("/style.css", Portal::servirFichierCSS);
  server.on("/script.js", Portal::servirFichierJS);
  server.on("/leaderboard.csv", Portal::servirFichierCSV);
  server.on("/pending", Portal::gererPending);
  server.on("/submit", Portal::soumettrePseudo);
  server.on("/ignore-time", Portal::ignorerTemps);
  // server.on("/hotspot-detect.html", []() {
  //   server.send(200, "text/html", "<html><head><title>Captive Portal</title></head><body>Connexion réussie</body></html>");
  //   Serial.println("Requête traitée pour /hotspot-detect.html");
  //   Serial.println("(rediriger vers \"/\"");
  // });
  server.on("/hotspot-detect.html", []() {
    DEBUG_PORTAL("Requête traitée pour /hotspot-detect.html");
    server.sendHeader("Location", "/", true);  // Redirection vers "/"
    server.send(302, "text/plain", "Redirection vers la page principale");
  });

  // Gestion des erreurs 404
  server.onNotFound([]() {
    String uri = server.uri();
    DEBUG_PORTAL("Requête non gérée pour : %s", uri.c_str());

    File fichier = SD.open(uri);
    if (fichier) {
      String contentType = "text/plain";
      if (uri.endsWith(".html"))
        contentType = "text/html";
      else if (uri.endsWith(".css"))
        contentType = "text/css";
      else if (uri.endsWith(".js"))
        contentType = "application/javascript";
      else if (uri.endsWith(".csv"))
        contentType = "text/csv";

      server.streamFile(fichier, contentType);
      fichier.close();
      DEBUG_PORTAL("Fichier servi avec succès : %s", uri.c_str());
    } else {
      DEBUG_ERROR("Erreur : fichier introuvable sur la SD pour : %s", uri.c_str());
      server.sendHeader("Location", "/", true);  // Redirection vers "/"
      server.send(302, "text/plain", "Redirection vers la page principale");
    }
  });

  // Démarrer le serveur
  server.begin();
  DEBUG_INFO("Serveur web démarré.");
}

// Ajout d'un temps en attente
void Portal::ajouterTempsEnAttente(float temps, int slaveNumber) {
  PendingTime nouvelElement = {temps, slaveNumber};
  pendingQueue.push(nouvelElement);

  DEBUG_PORTAL("Ajouté à la file : Temps = %.3f, Slave = %d\n", temps, slaveNumber);
}

// Route : afficher l'index
void Portal::afficherPageIndex() {
  File fichier = SD.open("/index.html");
  if (!fichier) {
    server.send(500, "text/plain", "Erreur : fichier index.html non trouvé !");
    return;
  }
  server.streamFile(fichier, "text/html");
  fichier.close();
}

// Route : servir le CSS
// void Portal::servirFichierCSS() {
//   File fichier = SD.open("/style.css");
//   if (!fichier) {
//     server.send(500, "text/plain", "Erreur : fichier style.css non trouvé !");
//     return;
//   }
//   server.streamFile(fichier, "text/css");
//   fichier.close();
// }
void Portal::servirFichierCSS() {
  DEBUG_PORTAL("Requête pour /style.css reçue.");
  File fichier = SD.open("/style.css");
  if (!fichier) {
    DEBUG_ERROR("Erreur : fichier style.css non trouvé !");
    server.send(500, "text/plain", "Erreur : fichier style.css non trouvé !");
    return;
  }
  server.streamFile(fichier, "text/css");
  fichier.close();
  DEBUG_PORTAL("Fichier style.css servi avec succès.");
}

// Route : servir le JS
void Portal::servirFichierJS() {
  File fichier = SD.open("/script.js");
  if (!fichier) {
    server.send(500, "text/plain", "Erreur : fichier script.js non trouvé !");
    return;
  }
  server.streamFile(fichier, "application/javascript");
  fichier.close();
}

// Route : servir le CSV
void Portal::servirFichierCSV() {
  File fichier = SD.open("/leaderboard.csv");
  if (!fichier) {
    server.send(500, "text/plain", "Erreur : fichier leaderboard.csv non trouvé !");
    return;
  }
  server.streamFile(fichier, "text/csv");
  fichier.close();
}

// Route : gérer les données en attente
void Portal::gererPending() {
  if (!pendingQueue.empty()) {
    PendingTime pendingTime = pendingQueue.front();

    String timeStr = String(pendingTime.time, 3);        // Convertit float en String avec 3 décimales
    String sourceStr = String(pendingTime.slaveNumber);  // Convertit int en String

    server.send(200, "application/json",
                "{\"status\":\"pending\",\"time\":\"" + timeStr + "\", \"source\":\"" + sourceStr + "\"}");
  } else {
    server.send(200, "application/json", "{\"status\":\"none\"}");
  }
}

// Route : soumettre un pseudo
void Portal::soumettrePseudo() {
  if (server.hasArg("pseudo") && !pendingQueue.empty()) {
    String pseudo = server.arg("pseudo");

    // Validation du pseudo
    if (pseudo.length() < 3 || pseudo.length() > 50) {
      server.send(400, "text/plain", "Pseudo invalide (3-50 caractères requis)");
      return;
    }

    PendingTime pendingTime = pendingQueue.front();
    pendingQueue.pop();

    // Ajout au CSV
    sd.ajouterEntreeCSV(pseudo.c_str(), pendingTime.time);
    sd.trierLeaderboard();

    // Redirection
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "Redirection vers le leaderboard");
  } else {
    server.send(400, "text/plain", "Pseudo manquant ou pas de temps en attente");
  }
}

// Route : ignorer le temps en attente
void Portal::ignorerTemps() {
  if (!pendingQueue.empty()) {
    pendingQueue.pop();  // Retirer le premier temps de la file d'attente
    server.send(200, "application/json", "{\"message\": \"Temps ignoré avec succès\"}");
    DEBUG_PORTAL("Temps en attente ignoré avec succès.");
  } else {
    server.send(400, "application/json", "{\"message\": \"Aucun temps en attente à ignorer\"}");
    DEBUG_PORTAL("Aucun temps en attente à ignorer.");
  }
}
