# SOUND-Interface

Sound generator based on the AY-3-8912 chip and implemented using the [Ayumi](https://github.com/true-grue/ayumi) library.

## Features

1. All functionalities of the AY-3-8912 chip are supported
2. Output device can be configured in the GUI and is saved to the configuration file. If no device is selected it seems that the default device is still used.
3. Both IO port ranges 0xFFFFFF40 - 0xFFFFFF41 (classical range used in some sample programs) and 0xFFFFFF50 - 0xFFFFFF51 (used by JADOS) are simultaniously supported.

## Configuration

The following section of the configuration file is used to configure the sound driver:

    - SoundDriver: Realtek Digital Output (Realtek(R) Audio)

This should not be edited manually but better configured using the GUI by clicking on the speaker symbol.

## Limitations

1. Some hardcoded value was added to adjust output levels using the AYUMI library. It is unclear if this has any side effects. Demo sounds and the JADOS Beep sound good.
2. On slow computers the amount of pre-rendered sound samples may need to be adjusted.
3. The parallel port A is not connectedd to anything.

## Future Enhancements

1. None currently planned.

## References

1. Description on NDR-NKC.de (https://www.ndr-nkc.de/compo/io/sound.htm)
