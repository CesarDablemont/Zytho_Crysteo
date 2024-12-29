#include "sd.hpp"

#include <vector>

void Sd::ajouterEntreeCSV(const char* pseudo, float temps) {
  // Vérifier si le fichier existe
  if (!SD.exists("/leaderboard.csv")) {
    // Si le fichier n'existe pas, le créer avec les en-têtes
    DEBUG_WARN("Fichier leaderboard.csv introuvable. Création...");
    File fichier = SD.open("/leaderboard.csv", FILE_WRITE);
    if (fichier) {
      // fichier.println("Pseudo,Temps");
      fichier.close();
      DEBUG_SD("Fichier créé avec succès.");
    } else {
      DEBUG_ERROR("Erreur : impossible de créer le fichier leaderboard.csv.");
      return;
    }
  }

  // Maintenant, ouvrir le fichier pour vérification et ajout
  File fichier = SD.open("/leaderboard.csv", FILE_READ);
  if (fichier) {
    String ligne;
    bool existeDeja = false;

    while (fichier.available()) {
      ligne = fichier.readStringUntil('\n');
      int virgule = ligne.indexOf(',');
      if (virgule > 0) {
        String pseudoExistant = ligne.substring(0, virgule);
        String tempsExistant = ligne.substring(virgule + 1);

        if (pseudoExistant == pseudo && tempsExistant == String(temps)) {
          existeDeja = true;
          break;  // Sortir de la boucle si trouvé
        }
      }
    }
    fichier.close();

    if (existeDeja) {
      DEBUG_ERROR("Erreur : l'entrée avec ce pseudo et ce temps existe déjà.");
      return;
    }
  } else {
    DEBUG_ERROR("Erreur : impossible de lire le fichier.");
    return;
  }

  // Ajouter l'entrée si elle n'existe pas encore
  fichier = SD.open("/leaderboard.csv", FILE_APPEND);
  if (fichier) {
    fichier.printf("%s,%.3f\n", pseudo, temps);
    fichier.close();
    DEBUG_SD("Entrée ajoutée avec succès !");
  } else {
    DEBUG_ERROR("Erreur : impossible d'écrire dans le fichier.");
  }
}

void Sd::trierLeaderboard() {
  File file = SD.open("/leaderboard.csv", FILE_READ);

  if (!file) {
    Serial.println("Erreur : fichier leaderboard.csv introuvable !");
    return;
  }

  // Lire toutes les lignes et stocker les données
  std::vector<std::pair<String, float>> leaderboardData;

  while (file.available()) {
    String line = file.readStringUntil('\n');
    int separatorIndex = line.indexOf(',');

    if (separatorIndex != -1) {
      String pseudo = line.substring(0, separatorIndex);
      float time = line.substring(separatorIndex + 1).toFloat();
      leaderboardData.push_back({pseudo, time});
    }
  }

  file.close();

  // Trier par temps (du plus petit au plus grand)
  std::sort(leaderboardData.begin(), leaderboardData.end(),
            [](const std::pair<String, float>& a, const std::pair<String, float>& b) {
              return a.second < b.second;  // Trie par le temps
            });

  // Réécrire le fichier CSV trié
  file = SD.open("/leaderboard.csv", FILE_WRITE);

  if (file) {
    for (const auto& entry : leaderboardData) { file.println(entry.first + "," + String(entry.second, 3)); }
    file.close();
    DEBUG_SD("Leaderboard trié et réécrit avec succès !");
  } else {
    DEBUG_ERROR("Erreur : impossible de réécrire leaderboard.csv");
  }
}

void Sd::Setup() {
  if (!SD.begin(CS_PIN)) {
    DEBUG_ERROR("Erreur : carte SD introuvable !");
    return;
  }
  DEBUG_INFO("Carte SD initialisée avec succès.");
}
