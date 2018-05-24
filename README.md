cw-trainer

CW Training program (Morse Code)

This is a work in progress!

All settings are currently hard coded, including
The alsa device, which you can set by changing
the ALSADEV define in alsa.c.  You can also change the
WPM (words per minute), tone frequency, volume, and
rise/fall time in the symbols_create() call in threads.c.

To build, type "make"
