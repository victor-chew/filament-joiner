# Filament Joiner

Code for setting up and driving a 3D printer hotend rigged up to an Arduino to provide a temperature-controlled platform for joining 3D filaments. Filaments joined this way have a strong and non-brittle bond compared to using uncontrolled heat sources such as a candle or lighter.

- [Video demo](https://youtu.be/rC_HjhM3sgA)
- [Building the circuit](https://www.randseq.org/2020/02/3d-printer-filament-joiner.html)

## Libraries required

- [SmoothThermistor](https://github.com/giannivh/SmoothThermistor) (install via Library Manager)
- [Arduino-PID-Library](https://github.com/br3ttb/Arduino-PID-Library/)  (install via Library Manager)
- [Arduino-PID-Autotune-Library](https://github.com/br3ttb/Arduino-PID-AutoTune-Library) (Add as ZIP library)

## Schematic

![Filament joiner schematic](https://github.com/victor-chew/filament-joiner/raw/master/images/schematic.png)

## Prototype

![Filament joiner prototype](https://github.com/victor-chew/filament-joiner/raw/master/images/filament-joiner.jpg)

## Forks

- [Alex Bagaglia created a fork using Nokia LCD 5110, and also modded the PID control for his setup](https://www.randseq.org/2020/03/filament-joiner-part-2-with-display-and.html)
