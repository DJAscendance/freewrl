

/* part of libfreewrl, usual libfreewrl license
not currently building this as a lib, 
-- just putting libmidi.cpp in libfreewrl list of source files
-- so could be in /lib/scenegraph
- called by Component_MIDI.c
General design: 
- similar to web audio, midi nodes will be implicitly connected by scenegraph hierarchy
- a separate thread will be recursing from MIDIsource through list of registered listener nodes
-- to MIDI destination or MIDI utility node that converts to ROUTEs or to web audio sound node
- the rendering thread will visit once per frame, scrape the latest data
-- and send it as field updates / routes or whatever.
- relies on libremidi https://github.com/jcelerier/libremidi
-- which uses C++ 17, so needs this wrapper libmidi
- libremidi is a close derivitive of rtmidi https://www.music.mcgill.ca/~gary/rtmidi/index.html 
-- rtmidi is a close 2nd choice / fallback, and apparently has an optional C interface
*/
#ifdef HAVE_LIBREMIDI
#define __STDC_LIMIT_MACROS
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <libremidi/libremidi.hpp>
#include <thread>

#else // HAVE_LIBREMIDI
#endif //HAVE_LIBREMIDI