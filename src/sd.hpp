#ifndef SD_HPP
#define SD_HPP

#include <FS.h>
#include <SD.h>
#include <SPI.h>

#include "utils.hpp"

class Sd {
 public:
  Sd() {};

  void Setup();
  void ajouterEntreeCSV(const char* pseudo, float temps);
  void trierLeaderboard();
};

#endif  // SD_HPP