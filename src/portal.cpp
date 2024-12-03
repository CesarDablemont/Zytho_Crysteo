#include "portal.hpp"

#include <queue>

WebServer server(80);  // Serveur web sur le port 80
DNSServer dnsServer;   // Serveur DNS
Sd sd;

// Donnée temporaire en attente
// String pendingTime = "";
std::queue<String> pendingQueue;

void Portal::Setup() {
  IPAddress apIP = WiFi.softAPIP();
  Serial.println("Point d'accès WiFi créé !");
  Serial.print("Adresse IP : ");
  Serial.println(apIP);

  // Configuration du serveur DNS
  dnsServer.start(DNS_PORT, "*", apIP);  // Redirige toutes les requêtes DNS vers l'ESP32

  // Configuration de mDNS
  if (MDNS.begin("zytho")) {
    Serial.println("mDNS démarré : http://zytho.local/");
  } else {
    Serial.println("Erreur : mDNS non démarré.");
  }

  // Route principale : affichage du leaderboard
  server.on("/", afficherLeaderboard);

  // API pour vérifier s'il y a une donnée en attente
  server.on("/pending", HTTP_GET, []() {
    if (!pendingQueue.empty()) {
      String pendingTime = pendingQueue.front();  // Récupérer le premier élément
      server.send(200, "application/json", "{\"status\":\"pending\",\"time\":\"" + pendingTime + "\"}");
    } else {
      server.send(200, "application/json", "{\"status\":\"none\"}");
    }
  });

  // Route pour soumettre le pseudo
  server.on("/submit", HTTP_POST, []() {
    if (server.hasArg("pseudo") && !pendingQueue.empty()) {
      String pseudo = server.arg("pseudo");

      // Validation du pseudo (entre 3 et 20 caractères)
      if (pseudo.length() < 3 || pseudo.length() > 20) {
        server.send(400, "text/plain", "Pseudo invalide (3-20 caracteres requis)");
        return;
      }

      String pendingTime = pendingQueue.front();  // Récupérer le premier élément
      pendingQueue.pop();                         // Retirer l'élément de la file

      // Ajout de l'entrée dans le fichier CSV
      sd.ajouterEntreeCSV(pseudo.c_str(), pendingTime.toFloat());  // Ajout de l'entrée

      server.sendHeader("Location", "/", true);  // Redirection vers la page principale
      server.send(302, "text/plain", "Redirection vers le leaderboard");
    } else {
      server.send(400, "text/plain", "Pseudo manquant");
    }
  });

  // Redirige toutes les requêtes inconnues vers la page principale
  server.onNotFound([]() {
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "Redirection vers le leaderboard");
  });

  // Démarrage du serveur
  server.begin();
  Serial.println("Serveur web démarré.");
}

void Portal::ajouterTempsEnAttente(float temps) {
  String tempsStr = String(temps, 2);
  pendingQueue.push(tempsStr);  // Ajouter le temps à la file
}

void Portal::afficherLeaderboard() {
  String page = "<!DOCTYPE html><html><head><title>Leaderboard</title>";
  page += "<script>";
  page += "async function checkPending() {";
  page += "  const response = await fetch('/pending');";
  page += "  const data = await response.json();";
  page += "  if (data.status === 'pending') {";
  page += "    const popup = document.getElementById('popup');";
  page += "    popup.classList.add('active');";  // Active la popup
  page += "    document.getElementById('time').textContent = data.time;";
  page += "  }";
  page += "}";
  page += "window.onload = checkPending;";
  page += "</script>";
  page += "<style>";
  page += "#popup { ";
  page += "  position: fixed; top: 0; left: 0; width: 100%; height: 100%; ";
  page += "  background: rgba(0, 0, 0, 0.7); color: white; ";
  page += "  display: none; justify-content: center; align-items: center; ";
  page += "}";
  page += "#popup.active { display: flex; }";  // Style pour activer la popup
  page += "</style>";
  page += "</head><body>";
  page += "<div id='popup'>";
  page += "<form action='/submit' method='POST' style='text-align: center;'>";
  page += "<h2>Nouveau Temps: <span id='time'></span></h2>";
  page += "<label for='pseudo'>Pseudo :</label>";
  page += "<input type='text' id='pseudo' name='pseudo' required>";
  page += "<br><br>";
  page += "<button type='submit'>Valider</button>";
  page += "</form>";
  page += "</div>";
  page += "<h1>Leaderboard</h1>";
  page += "<table border='1'><tr><th>Pseudo</th><th>Temps</th></tr>";

  File fichier = SD.open("/leaderboard.csv");
  if (fichier) {
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

  server.send(200, "text/html", page);
}
