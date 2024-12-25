#include <WiFi.h>
#include <esp_now.h>

#include "espNow.hpp"
#include "gpio.hpp"
#include "portal.hpp"
#include "sd.hpp"
#include "utils.hpp"

EspNow espNow;
Gpio gpio;

unsigned long lastDebounceTime0 = 0;
const unsigned long debounceDelay0 = 100;

unsigned long lastDebounceTime1 = 0;
const unsigned long debounceDelay1 = 10;

unsigned long timerCupStart = 0;

void setup() {
  Serial.begin(115200);
  sd.Setup();
  espNow.Setup();
  gpio.Setup();
}

void loop() {
  dnsServer.processNextRequest();  // Gérer les requêtes DNS
  server.handleClient();           // Gérer les requêtes HTTP

  unsigned long currentTime = millis();
  if (currentTime - lastDebounceTime0 > debounceDelay0) {
    lastDebounceTime0 = currentTime;

    if (Gpio::interruptsEnabled) {
      if (digitalRead((int)Led::Red) == HIGH) {  // si on change d'etat
        digitalWrite((int)Led::Red, LOW);
        digitalWrite((int)Led::Green, HIGH);
        attachInterrupt(digitalPinToInterrupt((int)Button::TEST1), Gpio::handleButtonPress, CHANGE);
        DEBUG_GPIO("Interruptions activées pour BUTTON_PIN1.");
      }
    } else {
      if (digitalRead((int)Led::Red) == LOW) {  // si on change d'etat
        digitalWrite((int)Led::Red, HIGH);
        digitalWrite((int)Led::Green, LOW);
        detachInterrupt(digitalPinToInterrupt((int)Button::TEST1));
        DEBUG_GPIO("Interruptions désactivées pour BUTTON_PIN1.");
      }
    }
  }

  if (currentTime - lastDebounceTime1 > debounceDelay1) {
    lastDebounceTime1 = currentTime;
    if (Gpio::interruptsEnabled && Gpio::interruptsCupRissing) {
      if (timerCupStart == 0) {  // Démarre le chronomètre uniquement si ce n'est pas déjà fait
        timerCupStart = millis();
        DEBUG_INFO("Timer start");
      }
    }

    if (Gpio::interruptsEnabled && !Gpio::interruptsCupRissing && timerCupStart != 0) {
      unsigned long delta = millis() - timerCupStart;
      portal.ajouterTempsEnAttente(delta / 1000.0);  // Convertit delta en secondes (float)
      timerCupStart = 0;                             // Réinitialise le chronomètre
      Gpio::interruptsEnabled = false;               // Réinitialise le flag
      DEBUG_INFO("Timer stop");
    }
  }
}
