/*
 * heater.ino
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

#include <NTC_Thermistor.h>
#include <SmoothThermistor.h>
#include <PID_v1.h>

#define KP 17.07
#define KI 0.75
#define KD 96.57

#define SMOOTHING_WINDOW 5
#define MOSFET_GATE_PIN 3
#define THERMISTOR_PIN A6

NTC_Thermistor* thermistor = new NTC_Thermistor(THERMISTOR_PIN, 10000, 100000, 25, 3950);
SmoothThermistor* sthermistor = new SmoothThermistor(thermistor, SMOOTHING_WINDOW);
bool pOnM = false;
double pwmOut, curTemp, setTemp = 180;
PID pid(&curTemp, &pwmOut, &setTemp, KP, KI, KD, P_ON_E, DIRECT);

void setup(void) {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(MOSFET_GATE_PIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pid.SetMode(AUTOMATIC);
}

void loop(void) {
  
  // Turn on LED if we are within 1% of target temperature
  curTemp = sthermistor->readCelsius();
  digitalWrite(LED_BUILTIN, abs(curTemp-setTemp)/setTemp <= 0.01);

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
    analogWrite(MOSFET_GATE_PIN, pwmOut);
  }
  else {
    Serial.println("Thermal overrun detected!");
    analogWrite(MOSFET_GATE_PIN, 0);
  }
  
  // Display stats
  Serial.print("pwmOut = "); Serial.print(pwmOut); Serial.print(", "); 
  Serial.print("diff = "); Serial.print(setTemp - curTemp); Serial.print(", "); 
  Serial.print("curTemp = "); Serial.print(round(curTemp)); 
  Serial.print(" / "); Serial.println(round(setTemp));
  
  delay(500);
}
