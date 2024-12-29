#ifndef DEBUGGER_HPP
#define DEBUGGER_HPP

// Niveau de débogage global (modifiez ici pour changer le niveau)
#define DEBUG_LEVEL 4

// Niveaux de débogage
#define DEBUG_LEVEL_NONE 0   // Aucun message
#define DEBUG_LEVEL_ERROR 1  // Seulement les erreurs
#define DEBUG_LEVEL_WARN 2   // Erreurs et avertissements
#define DEBUG_LEVEL_INFO 3   // Infos générales
#define DEBUG_LEVEL_DEBUG 4  // Débogage détaillé

// Macros pour les messages de débogage
#if DEBUG_LEVEL >= DEBUG_LEVEL_ERROR
#define DEBUG_ERROR(fmt, ...)        \
  Serial.print("[ERROR] ");          \
  Serial.printf(fmt, ##__VA_ARGS__); \
  Serial.println();
#else
#define DEBUG_ERROR(fmt, ...)
#endif

#if DEBUG_LEVEL >= DEBUG_LEVEL_WARN
#define DEBUG_WARN(fmt, ...)         \
  Serial.print("[WARN] ");           \
  Serial.printf(fmt, ##__VA_ARGS__); \
  Serial.println();
#else
#define DEBUG_WARN(fmt, ...)
#endif

#if DEBUG_LEVEL >= DEBUG_LEVEL_INFO
#define DEBUG_INFO(fmt, ...)         \
  Serial.print("[INFO] ");           \
  Serial.printf(fmt, ##__VA_ARGS__); \
  Serial.println();
#else
#define DEBUG_INFO(fmt, ...)
#endif

#if DEBUG_LEVEL >= DEBUG_LEVEL_DEBUG
#define DEBUG_DEBUG(fmt, ...)        \
  Serial.print("[DEBUG] ");          \
  Serial.printf(fmt, ##__VA_ARGS__); \
  Serial.println();
#else
#define DEBUG_DEBUG(fmt, ...)
#endif

#if DEBUG_LEVEL >= DEBUG_LEVEL_DEBUG
#define DEBUG_GPIO(fmt, ...) DEBUG_DEBUG("[GPIO] " fmt, ##__VA_ARGS__)
#define DEBUG_SD(fmt, ...) DEBUG_DEBUG("[SD] " fmt, ##__VA_ARGS__)
#define DEBUG_ESP_NOW(fmt, ...) DEBUG_DEBUG("[ESP_NOW] " fmt, ##__VA_ARGS__)
#define DEBUG_PORTAL(fmt, ...) DEBUG_DEBUG("[PORTAL] " fmt, ##__VA_ARGS__)
#else
#define DEBUG_GPIO(fmt, ...)
#define DEBUG_SD(fmt, ...)
#define DEBUG_ESP_NOW(fmt, ...)
#define DEBUG_PORTAL(fmt, ...)
#endif

#endif  // DEBUGGER_HPP