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
  pinMode((int)Button::CupSensor, INPUT_PULLUP);
  pinMode((int)Button::ArmSensor, INPUT_PULLUP);

  pinMode((int)Led::Red, OUTPUT);
  pinMode((int)Led::Green, OUTPUT);
  pinMode((int)Led::Blue, OUTPUT);

  attachInterrupt(digitalPinToInterrupt((int)Button::ArmSensor), toggleInterrupts, RISING);  // Quand on appuis
}

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

// USELESS (car interrupt)
/*
Button Gpio::IsTouch() {
  if (digitalRead((int)Button::CupSensor) == HIGH) return Button::CupSensor;
  if (digitalRead((int)Button::ArmSensor) == HIGH) return Button::ArmSensor;
  return Button::None;
}

void Gpio::WaitForTouch() {
  while (IsTouch() == Button::None) { delay(10); }
}

void Gpio::WaitForTouchAndRelease() {
  WaitForTouch();
  WaitForRelease();
}

void Gpio::WaitForRelease() {
  while (IsTouch() != Button::None) { delay(10); }
}
*/