#include "sd.hpp"

#include <vector>

void Sd::ajouterEntreeCSV(const char* pseudo, float temps, const char* categorie) {
  // Vérifier si le fichier existe
  if (!SD.exists("/leaderboard.csv")) {
    // Si le fichier n'existe pas, le créer avec les en-têtes
    DEBUG_WARN("Fichier leaderboard.csv introuvable. Création...");
    File fichier = SD.open("/leaderboard.csv", FILE_WRITE);
    if (fichier) {
      fichier.close();
      DEBUG_SD("Fichier créé avec succès.");
    } else {
      DEBUG_ERROR("Erreur : impossible de créer le fichier leaderboard.csv.");
      return;
    }
  }

  // Ajouter l'entrée directement
  File fichier = SD.open("/leaderboard.csv", FILE_APPEND);
  if (fichier) {
    // Vérifier que les valeurs ne sont pas des valeurs par défaut non souhaitées
    if (strcmp(pseudo, "Pseudo") != 0 && temps != 0.0f && strcmp(categorie, "Categorie") != 0) {
      fichier.printf("%s,%.3f,%s\n", pseudo, temps, categorie);
      DEBUG_SD("Entrée ajoutée avec succès !");
    } else {
      DEBUG_ERROR("Erreur : tentative d'ajout d'une entrée avec des valeurs par défaut.");
    }
    fichier.close();
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
  std::vector<std::tuple<String, float, String>> leaderboardData;

  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();  // Supprimer les espaces ou sauts de ligne en trop

    if (line.length() == 0) {
      continue;  // Ignorer les lignes vides
    }

    int firstSeparatorIndex = line.indexOf(',');
    int secondSeparatorIndex = line.indexOf(',', firstSeparatorIndex + 1);

    if (firstSeparatorIndex != -1 && secondSeparatorIndex != -1) {
      String pseudo = line.substring(0, firstSeparatorIndex);
      float time = line.substring(firstSeparatorIndex + 1, secondSeparatorIndex).toFloat();
      String category = line.substring(secondSeparatorIndex + 1);

      // Ignorer les entrées avec des valeurs par défaut non souhaitées
      if (pseudo == "Pseudo" || time == 0.0f || category == "Categorie") { continue; }

      leaderboardData.push_back({pseudo, time, category});
    }
  }

  file.close();

  // Trier par temps (du plus petit au plus grand)
  std::sort(leaderboardData.begin(), leaderboardData.end(),
            [](const std::tuple<String, float, String>& a, const std::tuple<String, float, String>& b) {
              return std::get<1>(a) < std::get<1>(b);  // Trie par le temps
            });

  // Réécrire le fichier CSV trié
  file = SD.open("/leaderboard.csv", FILE_WRITE);

  if (file) {
    for (const auto& entry : leaderboardData) {
      file.println(std::get<0>(entry) + "," + String(std::get<1>(entry), 3) + "," + std::get<2>(entry));
    }
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
