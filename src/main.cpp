#include <WiFi.h>
#include <esp_now.h>

#include "espNow.hpp"
#include "gpio.hpp"
#include "portal.hpp"
#include "sd.hpp"
#include "utils.hpp"

EspNow espNow;
Gpio gpio;

unsigned long lastDebounceTime0;
const unsigned long debounceDelay0 = 100;

unsigned long lastDebounceTime1;
const unsigned long debounceDelay1 = 10;

unsigned long timerCupStart = 0;

void setup() {
  Serial.begin(115200);
  espNow.Setup();
  gpio.Setup();
  // lastDebounceTime0 = millis();
  // lastDebounceTime1 = millis();
}

void loop() {
  if (isMaster) {
    dnsServer.processNextRequest();  // Gérer les requêtes DNS
    server.handleClient();           // Gérer les requêtes HTTP
  }

  unsigned long currentTime = millis();
  if (currentTime - lastDebounceTime0 > debounceDelay0) {
    lastDebounceTime0 = currentTime;

    if (Gpio::interruptsEnabled) {
#if ESP_TYPE == 1
      if (digitalRead((int)Led::Red) == HIGH) {  // si on change d'etat
#else
      if (digitalRead((int)Led::Red) == LOW) {  // si on change d'etat
#endif
        gpio.timerReady();
        attachInterrupt(digitalPinToInterrupt((int)Button::CupSensor), Gpio::handleButtonPress, CHANGE);
        DEBUG_GPIO("Interruptions activées pour BUTTON_PIN1.");
      }
    } else {
#if ESP_TYPE == 1
      if (digitalRead((int)Led::Red) == LOW) {  // si on change d'etat
#else
      if (digitalRead((int)Led::Red) == HIGH) {  // si on change d'etat
#endif
        gpio.TimerDisabled();
        detachInterrupt(digitalPinToInterrupt((int)Button::CupSensor));
        DEBUG_GPIO("Interruptions désactivées pour BUTTON_PIN1.");
      }
    }
  }

  if (currentTime - lastDebounceTime1 > debounceDelay1) {
    lastDebounceTime1 = currentTime;
    if (Gpio::interruptsEnabled && !Gpio::interruptsCupRissing) {
      if (timerCupStart == 0) {  // Démarre le chronomètre uniquement si ce n'est pas déjà fait
        timerCupStart = millis();
        gpio.TimerRunning();
        DEBUG_INFO("Timer start");
      }
    }

    if (Gpio::interruptsEnabled && Gpio::interruptsCupRissing && timerCupStart != 0) {
      DEBUG_INFO("Timer stop");

      unsigned long delta = millis() - timerCupStart;
      if (isMaster) {
        DEBUG_INFO("Nouveau temps : %.3f", delta / 1000.0);
        portal.ajouterTempsEnAttente(delta / 1000.0, -1);  // Convertit delta en secondes (float)
      } else {
        // send to master
        espNow.sendTime(delta / 1000.0);
      }

      timerCupStart = 0;                // Réinitialise le chronomètre
      Gpio::interruptsEnabled = false;  // Réinitialise le flag
    }
  }
}
