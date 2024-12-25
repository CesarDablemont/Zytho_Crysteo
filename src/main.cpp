#include <WiFi.h>
#include <esp_now.h>

#include "debugger.hpp"
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

  // portal.ajouterTempsEnAttente(15.85);
}

void loop() {
  dnsServer.processNextRequest();  // Gérer les requêtes DNS
  server.handleClient();           // Gérer les requêtes HTTP

  // Button button = gpio.IsTouch();
  // if (button != Button::None) {
  //   if (DEBUG_GPIO && (button == Button::TEST1)) Serial.println("DEBUG_GPIO: TEST1 button press");
  //   if (DEBUG_GPIO && (button == Button::TEST2)) Serial.println("DEBUG_GPIO: TEST2 button press");
  //   if (DEBUG_GPIO && (button == Button::TEST3)) Serial.println("DEBUG_GPIO: TEST3 button press");
  //   if (DEBUG_GPIO && (button == Button::TEST4)) Serial.println("DEBUG_GPIO: TEST4 button press");
  //   if (DEBUG_GPIO && (button == Button::TEST5)) Serial.println("DEBUG_GPIO: TEST5 button press");
  //   if (DEBUG_GPIO && (button == Button::TEST6)) Serial.println("DEBUG_GPIO: TEST6 button press");

  //   gpio.WaitForRelease();
  // }

  unsigned long currentTime = millis();
  if (currentTime - lastDebounceTime0 > debounceDelay0) {
    lastDebounceTime0 = currentTime;

    if (Gpio::interruptsEnabled) {
      digitalWrite((int)Led::Red, LOW);
      digitalWrite((int)Led::Green, HIGH);
      attachInterrupt(digitalPinToInterrupt((int)Button::TEST1), Gpio::handleButtonPress, CHANGE);
      Serial.println("Interruptions activées pour BUTTON_PIN1.");

    } else {
      digitalWrite((int)Led::Red, HIGH);
      digitalWrite((int)Led::Green, LOW);
      detachInterrupt(digitalPinToInterrupt((int)Button::TEST1));
      Serial.println("Interruptions désactivées pour BUTTON_PIN1.");
    }
  }

  if (currentTime - lastDebounceTime1 > debounceDelay1) {
    lastDebounceTime1 = currentTime;
    if (Gpio::interruptsEnabled && Gpio::interruptsCupRissing) {
      if (timerCupStart == 0) {  // Démarre le chronomètre uniquement si ce n'est pas déjà fait
        timerCupStart = millis();
      }
    }

    if (Gpio::interruptsEnabled && !Gpio::interruptsCupRissing && timerCupStart != 0) {
      unsigned long delta = millis() - timerCupStart;
      portal.ajouterTempsEnAttente(delta / 1000.0);  // Convertit delta en secondes (float)
      timerCupStart = 0;                             // Réinitialise le chronomètre
      Gpio::interruptsEnabled = false;               // Réinitialise le flag
    }
  }
}
