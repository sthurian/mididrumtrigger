# mididrumtrigger
A LV2-Plugin for triggering MIDI notes at a given threshold of an audio signal
## Installation
    $ make
    $ sudo make install
## Features
  - adjustable Threshold
  - adjustable ScanTime for measuring velocity
  - adjustable MaskTime to ignore signal after triggering
  - triggers MIDI Notes from General MIDI Drum Map (MIDI Note 35 to 81)
  - choose between amplitude, RMS or constant value for velocity
  - adjustable velocity gain
