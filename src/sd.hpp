#ifndef SD_HPP
#define SD_HPP

#include <SD.h>

#include "utils.hpp"

class Sd {
 public:
  Sd() {};

  void Setup();
  void ajouterEntreeCSV(const char* pseudo, float temps);
};

#endif  // SD_HPP