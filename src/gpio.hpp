#ifndef GPIO_HPP
#define GPIO_HPP

#include <Arduino.h>

// enum class Button { None = -1, CupSensor = 18, ArmSensor = 16 };  // ESP s2 mini
// enum class Led { Red = 33, Green = 35, Blue = 37 };

enum class Button { None = -1, CupSensor = 36, ArmSensor = 33 };  // ESP dev module
enum class Led { Red = 25, Green = 26, Blue = 27 };

class Gpio {
 public:
  Gpio();  // Constructor

  void Setup();

  // USELESS (car interrupt)
  // Button IsTouch();
  // void WaitForTouch();
  // void WaitForTouchAndRelease();
  // void WaitForRelease();

  void timerReady();
  void TimerRunning();
  void TimerDisabled();

  static volatile bool interruptsEnabled;
  static volatile bool interruptsCupRissing;

  static void IRAM_ATTR handleButtonPress();

 private:
  static void IRAM_ATTR toggleInterrupts();
};

#endif  // GPIO_HPP
