#include "portal.hpp"

// Déclaration des objets externes
WebServer server(80);
DNSServer dnsServer;
Sd sd;

// File d'attente partagée (statique)
std::queue<String> Portal::pendingQueue;

Portal::Portal() {}

void listerFichiersSD() {
  File root = SD.open("/");
  if (!root) {
    Serial.println("Erreur : Impossible d'ouvrir le répertoire racine.");
    return;
  }
  File fichier = root.openNextFile();
  while (fichier) {
    Serial.print("Fichier trouvé : ");
    Serial.println(fichier.name());
    fichier = root.openNextFile();
  }
}

void Portal::Setup() {
  // Initialisation du WiFi
  IPAddress apIP = WiFi.softAPIP();
  Serial.println("Point d'accès WiFi créé !");
  Serial.print("Adresse IP : ");
  Serial.println(apIP);

  // Initialisation de la carte SD
  if (!SD.begin()) {
    Serial.println("Erreur : la carte SD n'a pas pu être initialisée !");
    return;
  }
  Serial.println("Carte SD initialisée avec succès.");
  listerFichiersSD();  // Vérifie les fichiers disponibles sur la SD

  // Configuration DNS
  dnsServer.start(DNS_PORT, "*", apIP);

  // Routes HTTP
  server.on("/", [this]() {
    Serial.println("Requête reçue pour /");
    this->afficherPageIndex();
  });

  server.on("/style.css", Portal::servirFichierCSS);
  server.on("/script.js", Portal::servirFichierJS);
  server.on("/leaderboard.csv", Portal::servirFichierCSV);
  server.on("/pending", Portal::gererPending);
  server.on("/submit", Portal::soumettrePseudo);
  // server.on("/hotspot-detect.html", []() {
  //   server.send(200, "text/html", "<html><head><title>Captive Portal</title></head><body>Connexion réussie</body></html>");
  //   Serial.println("Requête traitée pour /hotspot-detect.html");
  //   Serial.println("(rediriger vers \"/\"");
  // });
  server.on("/hotspot-detect.html", []() {
    Serial.println("Requête traitée pour /hotspot-detect.html");
    server.sendHeader("Location", "/", true);  // Redirection vers "/"
    server.send(302, "text/plain", "Redirection vers la page principale");
  });

  // Gestion des erreurs 404
  // server.onNotFound([]() { server.send(404, "text/plain", "Fichier non trouvé"); });
  server.onNotFound([]() {
    String uri = server.uri();
    Serial.print("Requête non gérée pour : ");
    Serial.println(uri);

    // Réponse par défaut pour captives portals ou requêtes inconnues
    // if (uri == "/hotspot-detect.html" || uri == "/generate_204") {
    //   server.send(200, "text/html", "<html><head><title>Captive Portal</title></head><body>Connexion réussie</body></html>");
    //   Serial.println("Requête traitée pour captive portal.");
    //   return;
    // }

    // Recherche du fichier correspondant
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
      Serial.println("Fichier servi avec succès : " + uri);
    } else {
      server.send(404, "text/plain", "Erreur : fichier non trouvé !");
      Serial.println("Erreur : fichier introuvable sur la SD pour : " + uri);
    }
  });

  // Démarrer le serveur
  server.begin();
  Serial.println("Serveur web démarré.");
}

// Ajout d'un temps en attente
void Portal::ajouterTempsEnAttente(float temps) {
  String tempsStr = String(temps, 2);
  pendingQueue.push(tempsStr);
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
  Serial.println("Requête pour /style.css reçue.");
  File fichier = SD.open("/style.css");
  if (!fichier) {
    Serial.println("Erreur : fichier style.css non trouvé !");
    server.send(500, "text/plain", "Erreur : fichier style.css non trouvé !");
    return;
  }
  server.streamFile(fichier, "text/css");
  fichier.close();
  Serial.println("Fichier style.css servi avec succès.");
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
    String pendingTime = pendingQueue.front();
    server.send(200, "application/json", "{\"status\":\"pending\",\"time\":\"" + pendingTime + "\"}");
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

    String pendingTime = pendingQueue.front();
    pendingQueue.pop();

    // Ajout au CSV
    sd.ajouterEntreeCSV(pseudo.c_str(), pendingTime.toFloat());
    sd.trierLeaderboard();

    // Redirection
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "Redirection vers le leaderboard");
  } else {
    server.send(400, "text/plain", "Pseudo manquant ou pas de temps en attente");
  }
}
