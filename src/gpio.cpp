#include "gpio.hpp"

#include "debugger.hpp"

volatile bool Gpio::interruptsEnabled = false;
volatile bool Gpio::interruptsCupRissing = false;

volatile unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 100;

// Constructor
Gpio::Gpio() {}

// Init
void Gpio::Setup() {
  pinMode((int)Button::TEST1, INPUT_PULLUP);  // Bouton 1 avec pull-up interne
  pinMode((int)Button::TEST2, INPUT_PULLUP);  // Bouton 2 avec pull-up interne
  pinMode((int)Button::TEST3, INPUT_PULLUP);  // Bouton 3 avec pull-up interne
  pinMode((int)Button::TEST4, INPUT_PULLUP);  // Bouton 4 avec pull-up interne
  pinMode((int)Button::TEST5, INPUT_PULLUP);  // Bouton 5 avec pull-up interne
  pinMode((int)Button::TEST6, INPUT_PULLUP);  // Bouton 6 avec pull-up interne

  pinMode((int)Led::Red, OUTPUT);    // Bouton 6 avec pull-up interne
  pinMode((int)Led::Green, OUTPUT);  // Bouton 6 avec pull-up interne
  pinMode((int)Led::Blue, OUTPUT);   // Bouton 6 avec pull-up interne

  attachInterrupt(digitalPinToInterrupt((int)Button::TEST6), toggleInterrupts, RISING);
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
  if (!interruptsEnabled) return; // normalement meme pas besoin car on a detacher l'interutption

  if (digitalRead((int)Button::TEST1) == LOW)
    interruptsCupRissing = true;  // interruptsCupRissing
  else
    interruptsCupRissing = false;  // interruptsCupFalling
}

Button Gpio::IsTouch() {
  if (digitalRead((int)Button::TEST1) == HIGH) return Button::TEST1;
  if (digitalRead((int)Button::TEST2) == HIGH) return Button::TEST2;
  if (digitalRead((int)Button::TEST3) == HIGH) return Button::TEST3;
  if (digitalRead((int)Button::TEST4) == HIGH) return Button::TEST4;
  if (digitalRead((int)Button::TEST5) == HIGH) return Button::TEST5;
  if (digitalRead((int)Button::TEST6) == HIGH) return Button::TEST6;
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
