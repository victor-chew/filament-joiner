/*
 * autotune.ino
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
// - Arduino-PID-AutoTune-Library (br3ttb) [Source: https://github.com/br3ttb/Arduino-PID-AutoTune-Library]

#include <NTC_Thermistor.h>
#include <SmoothThermistor.h>
#include <PID_AutoTune_v0.h>

#define SMOOTHING_WINDOW 5
#define MOSFET_GATE_PIN 3
#define THERMISTOR_PIN A6

NTC_Thermistor* thermistor = new NTC_Thermistor(THERMISTOR_PIN, 10000, 100000, 25, 3950);
SmoothThermistor* sthermistor = new SmoothThermistor(thermistor, SMOOTHING_WINDOW);
double curTemp=25, pwmOut=255.0/2, aTuneStep=255.0/2, aTuneNoise=1;
PID_ATune aTune(&curTemp, &pwmOut);
unsigned int aTuneLookBack=20;
unsigned long serialms;

void setup(void) {
  Serial.begin(115200);
  pinMode(MOSFET_GATE_PIN, OUTPUT);
  aTune.SetControlType(1); // PID
  aTune.SetNoiseBand(aTuneNoise);
  aTune.SetOutputStep(aTuneStep);
  aTune.SetLookbackSec(aTuneLookBack);
  serialms = millis() + 1000;
}

void loop(void) { 
  curTemp = sthermistor->readCelsius();
  // Auto-tune in progress
  if (!aTune.Runtime()) {
    analogWrite(MOSFET_GATE_PIN, pwmOut);
    if (millis() >= serialms) {
      Serial.print("pwmOut = "); Serial.print(pwmOut); Serial.print(", ");
      Serial.print("curTemp = "); Serial.print(curTemp); Serial.println();
      serialms += 1000;
    }
  // Auto-tune done
  } else {
    Serial.print("kp: "); Serial.print(aTune.GetKp()); Serial.print(", ");
    Serial.print("ki: "); Serial.print(aTune.GetKi()); Serial.print(", ");
    Serial.print("kd: "); Serial.print(aTune.GetKd()); Serial.println();
    analogWrite(MOSFET_GATE_PIN, 0);
    aTune.Cancel();
    delay(60*1000);
  }
}
