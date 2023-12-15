# Daisy Seed Reverb Guitar Pedal


Combined a few examples from the ElectroSmith [DaisyExamples](https://github.com/electro-smith/DaisyExamples)
repo to make a polyphonic synth with a nice Costello reverb, just for tinkering.


# Cloning

This project assumes you have cloned the DaisyExamples repo as a sibling to this directory. Hoping to get around to the snazzier submodule based approach set up in my Daisy Polysynth repo so you can clone with just a single command, but not there yet.

# Building

Currently assumes the Daisy libs have already been built at the sibling location.

Development so far has been done on Ubuntu Linux 22.0.4.1, but should in theory work 
on macOS and Windows as well.

The project itself can be compiled with `make`.

You can deploy to the target using one of these commands:
```
# using USB (after entering bootloader mode)
make program-dfu
# using JTAG/SWD adaptor (like STLink). No need to enter bootloader mode
make program
```

Highly recommended to use a JTAG/SWD debugger rather than USB, since the USB power introduces nasty noise into the audio path.
You should be powering the board with a 9V center-negative wall wart power supply, and powering the ADCs with the Daisy's 3.3V output.
