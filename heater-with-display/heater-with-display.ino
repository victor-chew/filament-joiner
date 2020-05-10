/*
 * heater-with-display.ino
 *
 * Copyright 2020 Victor Chew
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Libraries required:
// - NTC_Thermister (Yurii Salimov) [Source: Arduino Library Manager]
// - PID (Brett Beuuregard) [Source: Arduino Library Manager]
// - ss_oled (Larry Bank) [Source: Arduino Library Manager]
// - TimerOne (Paul Stoffregen et al.) [Source: Arduino Library Manager]
// - ClickEncoder (0xPIT) [Source: https://github.com/0xPIT/encoder/tree/arduino]

#include <ss_oled.h>
#include <TimerOne.h>
#include <ClickEncoder.h>
#include <NTC_Thermistor.h>
#include <SmoothThermistor.h>
#include <PID_v1.h>

#define KP 17.07
#define KI 0.75
#define KD 96.57

#define SMOOTHING_WINDOW 5
#define MOSFET_GATE_PIN 3
#define THERMISTOR_PIN A6

#define KY040_CLK 9
#define KY040_DT 8
#define KY040_SW 2
#define KY040_STEPS_PER_NOTCH 2

bool heaterOn = false, pOnM = false;
double pwmOut, curTemp, setTemp = 0;

NTC_Thermistor* thermistor = new NTC_Thermistor(THERMISTOR_PIN, 10000, 100000, 25, 3950);
SmoothThermistor* sthermistor = new SmoothThermistor(thermistor, SMOOTHING_WINDOW);
PID pid(&curTemp, &pwmOut, &setTemp, KP, KI, KD, P_ON_E, DIRECT);
ClickEncoder encoder(KY040_CLK, KY040_DT, KY040_SW, KY040_STEPS_PER_NOTCH);
SSOLED ssoled;

void refreshDisplay() {
  oledFill(&ssoled, 0, 1);
  char msg[64];
  sprintf(msg, "Hotend: %dc", (int)curTemp);
  oledWriteString(&ssoled, 0, 0, 1, msg, FONT_NORMAL, 0, 1);
  sprintf(msg, "Target: %dc", (int)setTemp);
  oledWriteString(&ssoled, 0, 0, 3, msg, FONT_NORMAL, 0, 1);
  oledWriteString(&ssoled, 0, 0, 6, heaterOn ? "Heating..." : "- Heater off -", FONT_NORMAL, heaterOn ? 1 : 0, 1);
}

void encoderISR() {
  encoder.service();
}

void setup() {
  Serial.begin(115200);

  pinMode(MOSFET_GATE_PIN, OUTPUT);
  pid.SetMode(AUTOMATIC);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  encoder.setAccelerationEnabled(true);
  Timer1.initialize(1000);
  Timer1.attachInterrupt(encoderISR); 

  oledInit(&ssoled, OLED_128x64, -1, 0, 0, 1, -1, -1, -1, 400000L);
  refreshDisplay();
}

void loop() {
  // Get rotary encoder values and update display
  int _curTemp = sthermistor->readCelsius();
  int _setTemp = setTemp - encoder.getValue()*5; // minus because signal from this rotary switch works in the opposite direction
  _setTemp = min(300, max(0, _setTemp));
  bool _heaterOn = heaterOn;
  if (encoder.getButton() == ClickEncoder::Clicked) {
    _heaterOn = !_heaterOn;
    if (!_heaterOn) _setTemp = 0;
  }

  // Update display only if something changes
  if (_curTemp != curTemp || _setTemp != setTemp || _heaterOn != heaterOn) {
    curTemp = _curTemp;
    setTemp = _setTemp;
    heaterOn = _heaterOn;
    refreshDisplay(); 
  }

  // Turn on LED if we are within 1% of target temperature
  digitalWrite(LED_BUILTIN, heaterOn ? abs(_curTemp-setTemp)/setTemp <= 0.01 : LOW);

  // To speed things up, only switch to proportional-on-measurement when we are near target temperature
  // See: http://brettbeauregard.com/blog/2017/06/introducing-proportional-on-measurement/
  if (!pOnM && abs(curTemp-setTemp) <= max(curTemp, setTemp)*0.2) {
    pOnM = true;
    pid.SetTunings(KP, KI, KD, P_ON_M);
    Serial.println("P_ON_M activated.");
  }

  // Prevent thermal overrun in case of PID malfunction
  pid.Compute();
  if (curTemp < 300) {
    analogWrite(MOSFET_GATE_PIN, heaterOn ? pwmOut : 0);
  }
  else {
    analogWrite(MOSFET_GATE_PIN, 0);
    Serial.println("Thermal overrun detected!");
  }

  // Display stats
  //Serial.print("pwmOut = "); Serial.print(pwmOut); Serial.print(", "); 
  //Serial.print("diff = "); Serial.print(setTemp - curTemp); Serial.print(", "); 
  //Serial.print("curTemp = "); Serial.print(round(curTemp)); 
  //Serial.print(" / "); Serial.println(round(setTemp));
  
  delay(200);
}
