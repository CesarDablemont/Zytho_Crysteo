#ifndef GPIO_HPP
#define GPIO_HPP

#include <Arduino.h>

enum class Button { None = -1, TEST1 = 36, TEST2 = 39, TEST3 = 34, TEST4 = 35, TEST5 = 32, TEST6 = 33 };
enum class Led { Red = 25, Green = 26, Blue = 27 };

class Gpio {
 public:
  Gpio();  // Constructor

  void Setup();

  Button IsTouch();
  void WaitForTouch();
  void WaitForTouchAndRelease();
  void WaitForRelease();

  static volatile bool interruptsEnabled;
  static volatile bool interruptsCupRissing;

  static void IRAM_ATTR handleButtonPress();

 private:
  static void IRAM_ATTR toggleInterrupts();
};

#endif  // GPIO_HPP
