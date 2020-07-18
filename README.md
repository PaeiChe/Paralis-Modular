# Paralis Modular

Modules for VCV Rack

## Modules

### AtenuSight
Based on Befaco's Dual Atenuverter and Mutable Instruments' Blinds (Quad Polarizer)
Utilizes Andrew Belt's code for both Befaco and 'Audible Instruments' modules.
[Dual Atenuverter](https://www.befaco.org/dual-atenuverter/) and
[Blinds](https://mutable-instruments.net/modules/blinds), ( [Manual](https://mutable-instruments.net/modules/blinds/manual/) )

- Outputs are normaled similar to Blinds
- CV controls the polarizer/attenuverter

### Mini Modal Synthesizer
Based on the [Antumbra Atom] (https://www.antumbra.eu/redesign/atom), [Modular Grid] (https://www.modulargrid.net/e/antumbra-atom)
Utilizes Andrew Belt's code for [Elements](https://mutable-instruments.net/modules/elements), [Manual](https://mutable-instruments.net/modules/elements/manual/)

### Stiks
Based on and utilizes Andrew Belt's code for [Branches](https://mutable-instruments.net/modules/branches), [Manual](https://mutable-instruments.net/modules/branches/manual/)


### Zzzh
Based on RND Step from DivKid & Steady State Fate [RND Step](https://divkidvideo.com/rnd-step-the-third-divkid-eurorack-module/)
Utilizes southpole (gbrandt1) code for Sssh [southpole](https://github.com/gbrandt1/southpole-vcvrack)
Sssh is based  on the lower section of [Kinks](https://mutable-instruments.net/modules/kinks), [Manual](https://mutable-instruments.net/modules/kinks/manual/)

- There are 4 identical sections consisting of Trigger, Input, Right Output (-+), Left Output (+) and Lower Output (Noise)

- Triggers are normalled

- Right Output (-+) will sample and hold internal noise and output a bipolar signal. If connected it will sample & hold the Input.

- Left Output (+) is unipolar. 
  (i) By default the switch is toggled to the left (i.e. away from bipolar output). In this state (+) will sample & hold internal noise.
  (ii) When the switch toggled to the right (i.e. toward the bipolar output) it will sample & hold the bipolar (-+) output and return a unipolar signal

- Lower Output (Noise) is... you guessed it... Noise (although each noise output is unique/separate)
  (i) By default the switch is toggled to the left (i.e. away from bipolar output). In this state (noise) will output an audio-rate, gaussian noise signal.
  (ii) When the switch toggled to the right (i.e. toward the bipolar output) it will sample & hold the bipolar (-+) output and return the inverse of the signal

