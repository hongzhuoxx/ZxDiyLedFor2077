#include "Arduino.h"
#include <Wire.h>
#include "heltec.h"
#include "FastLED.h"

#define Fbattery 3700  //The default battery is 3700mv when the battery is fully charged.

#define LED_PIN 4
#define NUM_LEDS 60
#define BRIGHTNESS 220
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];
byte b[24] = {};


float XS = 0.0025;  //The returned reading is multiplied by this XS to get the battery voltage.
uint16_t MUL = 1000;
uint16_t MMUL = 100;
int counter = 1;

int is_low_power = false;
int v_close_led = false;

void drawProgressBarDemo() {
  int progress = counter;
  // draw the progress bar
  Heltec.display->drawProgressBar(0, 42, 120, 10, progress);

  // draw the percentage as String
  Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);

  Heltec.display->drawString(64, 0, "Cyberpunk 2077");
  Heltec.display->drawString(64, 10, "Weapon system starting...");
  Heltec.display->drawString(64, 25, String(progress) + "%");
}

void setup() {
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Enable*/, true /*Serial Enable*/);

  Heltec.display->init();
  Heltec.display->flipScreenVertically();
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->setColor(WHITE);
  Heltec.display->screenRotate(ANGLE_180_DEGREE);

  Heltec.display->drawString(0, 0, "Cyberpunk X 2077");
  Heltec.display->drawString(0, 10, "Weapon system starting...");
  Heltec.display->display();
  delay(1000);
  Heltec.display->clear();

  //analogSetClockDiv(255); // 1338mS
  //   analogSetCycles(8);                   // Set number of cycles per sample, default is 8 and provides an optimal result, range is 1 - 255
  //   analogSetSamples(1);                  // Set number of samples in the range, default is 1, it has an effect on sensitivity has been multiplied
  analogSetClockDiv(1);                   // Set the divider for the ADC clock, default is 1, range is 1 - 255
  analogSetAttenuation(ADC_11db);         // Sets the input attenuation for ALL ADC inputs, default is ADC_11db, range is ADC_0db, ADC_2_5db, ADC_6db, ADC_11db
  analogSetPinAttenuation(36, ADC_11db);  // Sets the input attenuation, default is ADC_11db, range is ADC_0db, ADC_2_5db, ADC_6db, ADC_11db
  analogSetPinAttenuation(37, ADC_11db);
  // ADC_0db provides no attenuation so IN/OUT = 1 / 1 an input of 3 volts remains at 3 volts before ADC measurement
  // ADC_2_5db provides an attenuation so that IN/OUT = 1 / 1.34 an input of 3 volts is reduced to 2.238 volts before ADC measurement
  // ADC_6db provides an attenuation so that IN/OUT = 1 / 2 an input of 3 volts is reduced to 1.500 volts before ADC measurement
  // ADC_11db provides an attenuation so that IN/OUT = 1 / 3.6 an input of 3 volts is reduced to 0.833 volts before ADC measurement
  //   adcAttachPin(VP);                     // Attach a pin to ADC (also clears any other analog mode that could be on), returns TRUE/FALSE result
  //   adcStart(VP);                         // Starts an ADC conversion on attached pin's bus
  //   adcBusy(VP);                          // Check if conversion on the pin's ADC bus is currently running, returns TRUE/FALSE result
  //   adcEnd(VP);

  adcAttachPin(36);
  adcAttachPin(37);

  Serial.begin(115200);

  if (true) {
    delay(300);
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    for (int i = 0; i < 24; i++) {
      b[i] = 0;
    }
    startup();
  }
}



void startup() {

  for (int f = 0; f < BRIGHTNESS; f++) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB(255, 55, 0);
      FastLED.setBrightness(255);
      leds[random(NUM_LEDS)].fadeToBlackBy(255);
    }
    FastLED.show();
    delay(8);
  }

  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(255, 55, 0);
    leds[i].fadeToBlackBy(0);
  }

  FastLED.show();
}



void loop() {

  if (counter < 100) {
    counter++;
    Heltec.display->clear();
    drawProgressBarDemo();
    Heltec.display->display();
    delay(100);

    if (counter >= 100) {
      Heltec.display->init();
      Heltec.display->flipScreenVertically();
      Heltec.display->setFont(ArialMT_Plain_10);
      Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
      Heltec.display->setColor(WHITE);
      Heltec.display->screenRotate(ANGLE_180_DEGREE);
      Heltec.display->clear();
    }
  } else {

    //WiFi LoRa 32        -- hardare versrion ≥ 2.3
    //WiFi Kit 32         -- hardare versrion ≥ 2
    //Wireless Stick      -- hardare versrion ≥ 2.3
    //Wireless Stick Lite -- hardare versrion ≥ 2.3
    //Battery voltage read pin changed from GPIO13 to GPI37
    //   adcStart(37);
    //   while(adcBusy(37));
    // Serial.printf("Battery power in GPIO 37: ");
    // Serial.println(analogRead(37));
    uint16_t c1 = analogRead(37) * XS * MUL;
    //   adcEnd(37);

    delay(100);

    uint16_t c2 = analogRead(36) * 0.769 + 150;

    uint16_t c = analogRead(13) * XS * MUL;
    // Serial.println(analogRead(13));

    if (is_low_power) {
      Heltec.display->drawString(0, 0, "System Low Power");
    } else {
      Heltec.display->drawString(0, 0, "Cyberpunk 2077");
    }

    Heltec.display->drawString(0, 10, "----------------------------------------------");
    Heltec.display->drawString(0, 20, "Vbat = ");
    Heltec.display->drawString(33, 20, (String)c1);
    Heltec.display->drawString(60, 20, "(mV)");

    Heltec.display->drawString(0, 30, "Vin   = ");
    Heltec.display->drawString(33, 30, (String)c2);
    Heltec.display->drawString(60, 30, "(mV)");

    Heltec.display->drawString(0, 40, "Remaining battery still has:");
    Heltec.display->drawString(0, 50, "VBAT:");
    Heltec.display->drawString(35, 50, (String)c);
    Heltec.display->drawString(60, 50, "(mV)");

    Heltec.display->display();
    delay(2000);
    Heltec.display->clear();

    if (c1 > 0 && c1 < 4500) {
      if (is_low_power == true && v_close_led == false) {
        for (int i = 0; i < NUM_LEDS; i++) {
          FastLED.setBrightness(0);
        }
        FastLED.show();
        v_close_led = true;
      }
      is_low_power = true;
    }

    // for (int i = 0; i < NUM_LEDS; i++) {
    //   leds[i] = CRGB(255, 255, 0);
    //   //leds[random(NUM_LEDS)].fadeToBlackBy(random(40, 60));
    // }

    // FastLED.show();
    delay(20);
  }
}