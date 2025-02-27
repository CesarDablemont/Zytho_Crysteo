#ifndef GPIO_HPP
#define GPIO_HPP

#include <Arduino.h>

#if ESP_TYPE == 1
enum class Button { None = -1, CupSensor = 36, ArmSensor = 33 };  // ESP dev module
enum class Led { Red = 25, Green = 26, Blue = 27 };
#elif ESP_TYPE == 2
enum class Button { None = -1, CupSensor = 18, ArmSensor = 16 };  // ESP s2 mini
enum class Led { Red = 35, Green = 37, Blue = 39 };
#else
enum class Button { None = -1, CupSensor = 2, ArmSensor = 15 };  // ESP devkit-v1
enum class Led { Red = 17, Green = 16, Blue = 4 };
#endif

class Gpio {
 public:
  Gpio();  // Constructor

  void Setup();

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
