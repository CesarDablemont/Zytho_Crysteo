#include "gpio.hpp"

#include "debugger.hpp"

volatile bool Gpio::interruptsEnabled = false;
volatile bool Gpio::interruptsCupRissing = true;

volatile unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 100;

// Constructor
Gpio::Gpio() {}

// Init
void Gpio::Setup() {
  pinMode((int)Button::CupSensor, INPUT_PULLDOWN);
  pinMode((int)Button::ArmSensor, INPUT_PULLDOWN);

  pinMode((int)Led::Red, OUTPUT);
  pinMode((int)Led::Green, OUTPUT);
  pinMode((int)Led::Blue, OUTPUT);

#if ESP_TYPE == 1
  digitalWrite((int)Led::Red, LOW);
  digitalWrite((int)Led::Green, LOW);
  digitalWrite((int)Led::Blue, LOW);
#else
  digitalWrite((int)Led::Red, HIGH);
  digitalWrite((int)Led::Green, HIGH);
  digitalWrite((int)Led::Blue, HIGH);
#endif

  attachInterrupt(digitalPinToInterrupt((int)Button::ArmSensor), toggleInterrupts, RISING);  // Quand on appuis
}

#if ESP_TYPE == 1
void Gpio::timerReady() {
  digitalWrite((int)Led::Red, LOW);
  digitalWrite((int)Led::Green, HIGH);
}

void Gpio::TimerRunning() {
  digitalWrite((int)Led::Green, LOW);
  digitalWrite((int)Led::Blue, HIGH);
}

void Gpio::TimerDisabled() {
  digitalWrite((int)Led::Blue, LOW);
  digitalWrite((int)Led::Red, HIGH);
}
#else
void Gpio::timerReady() {
  digitalWrite((int)Led::Red, HIGH);
  digitalWrite((int)Led::Green, LOW);
  digitalWrite((int)Led::Blue, HIGH);
}

void Gpio::TimerRunning() {
  digitalWrite((int)Led::Red, HIGH);
  digitalWrite((int)Led::Green, HIGH);
  digitalWrite((int)Led::Blue, LOW);
}

void Gpio::TimerDisabled() {
  digitalWrite((int)Led::Red, LOW);
  digitalWrite((int)Led::Green, HIGH);
  digitalWrite((int)Led::Blue, HIGH);
}

#endif

void IRAM_ATTR Gpio::toggleInterrupts() {
  // unsigned long currentTime = millis();
  // if (currentTime - lastDebounceTime > debounceDelay) {
  //   lastDebounceTime = currentTime;
  interruptsEnabled = !interruptsEnabled;
  // }
}

// Gestion de l'interruption pour TEST1
void IRAM_ATTR Gpio::handleButtonPress() {
  if (!interruptsEnabled) return;  // normalement meme pas besoin car on a detacher l'interutption

  if (digitalRead((int)Button::CupSensor) == HIGH)
    interruptsCupRissing = true;  // interruptsCupRissing
  else
    interruptsCupRissing = false;  // interruptsCupFalling
}