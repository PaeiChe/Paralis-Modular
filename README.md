# Paralis Modular

Modules for VCV Rack - free & open-source

https://wot.fandom.com/wiki/Paralis-net

CPU usage testing based on 
- initialized and nothing connected 
- randomized preset and connecting all inputs and outputs to other modules.
<br>(Your mileage may vary)

## Modules

![Paralis Modular Collection July 2020](https://raw.githubusercontent.com/PaeiChe/Paralis-Modular/master/Paralis%20Modular%20July%202020%20V1.JPG)

### AtenuSight
Based on Befaco's [Dual Atenuverter](https://www.befaco.org/dual-atenuverter/) and Mutable Instruments' [Blinds](https://mutable-instruments.net/modules/blinds) ( [Manual](https://mutable-instruments.net/modules/blinds/manual/) )

Utilizes [Andrew Belt](https://github.com/VCVRack)'s code for both [Befaco](https://github.com/VCVRack/Befaco) and [Audible Instruments](https://github.com/VCVRack/AudibleInstruments) 

- Outputs are normalled similar to Blinds
- CV controls the polarizer/attenuverter
- Attenuate up to x2 for amplifiying signals and Offset up to 10V 
- 5 hp
- CPU: Init/Idle(0.5% 0.10µs) Max(0.6% 0.14µs) 

### Mini Modal Synthesizer
Based on the [Antumbra Atom](https://www.antumbra.eu/redesign/atom) ( [Modular Grid](https://www.modulargrid.net/e/antumbra-atom) )

Utilizes [Andrew Belt](https://github.com/VCVRack/AudibleInstruments)'s code for [Elements](https://mutable-instruments.net/modules/elements) ( [Manual](https://mutable-instruments.net/modules/elements/manual/) )

- 18 hp
- CPU: Init/Idle(3.1% 0.55µs) Max(goes up to 5.1% 0.81µs) 

### Stiks
Based on and utilizes [Andrew Belt](https://github.com/VCVRack/AudibleInstruments)'s code for [Branches](https://mutable-instruments.net/modules/branches) ( [Manual](https://mutable-instruments.net/modules/branches/manual/) )

- 2 hp
- CPU: Init/Idle(0.1% 0.03µs) Max(0.2% 0.04µs)

### Zzzh
Based on [RND Step](https://divkidvideo.com/rnd-step-the-third-divkid-eurorack-module/) from [DivKid](https://www.youtube.com/channel/UC1zIiKvJIi7aYl2N68pWF8g) & [Steady State Fate](http://www.steadystatefate.com/) (Check out DivKid's [Video](https://www.youtube.com/watch?v=hlcXvCN80jU) on RND Step)

Utilizes [southpole](https://github.com/gbrandt1/southpole-vcvrack) (gbrandt1) code for Sssh  or the lower section of [Kinks](https://mutable-instruments.net/modules/kinks) ( [Manual](https://mutable-instruments.net/modules/kinks/manual/) )

- There are 4 identical sections consisting of Trigger, Input, Right Output (-+), Left Output (+) and Lower Output (Noise)

- Triggers are normalled

- Right Output (-+) will sample and hold internal noise and output a bipolar signal. If connected it will sample & hold the Input.

- Left Output (+) is unipolar. 
  
    (i) By default the switch is toggled to the left (i.e. away from bipolar output). In this state (+) will sample & hold internal noise.
    
    (ii) When the switch toggled to the right (i.e. toward the bipolar output) it will sample & hold the bipolar (-+) output and return a unipolar signal

- Lower Output (Noise) is... you guessed it... Noise (although each noise output is unique/separate)

    (i) By default the switch is toggled to the left (i.e. away from bipolar output). In this state (noise) will output an audio-rate, gaussian noise signal.
  
    (ii) When the switch toggled to the right (i.e. toward the bipolar output) it will sample & hold the bipolar (-+) output and return the inverse of the signal
  
 - 4 hp
 
 - CPU: Init/Idle(3.7% 0.84µs) Max (3.8% 0.87µs... ya definitely needs optimization)
 
 ![Paralis Modular Collection July 2020 patched](https://raw.githubusercontent.com/PaeiChe/Paralis-Modular/master/Paralis%20Modular%20July%202020%20V1%20patched.JPG)
