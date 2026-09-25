//
// File:		FWKeyEvents.h
//
// Maps a Cocoa key event to the FreeWRL key actions from cdllFreeWRL.h.
//
// FreeWRL's keyboard contract (fwl_do_keyPress0 in MainLoop.c):
//   KEYDOWN / KEYUP  - held-key state: fly/walk navigation keys, SHIFT/CTRL,
//                      KeySensor.
//   KEYPRESS         - one-shot character: command hotkeys ('q', 'v', 'h', ...),
//                      the ':' command line, StringSensor.
// The Win32 frontend sends WM_KEYDOWN as KEYDOWN followed by WM_CHAR as KEYPRESS;
// X11 sends KEYDOWN/KEYUP plus KEYPRESS for printable keys. Cocoa has one keyDown:
// for both, so it sends KEYDOWN then KEYPRESS.
//

#ifndef FWKEYEVENTS_H
#define FWKEYEVENTS_H

#include "../../../freex3d/src/dllFreeWRL/cdllFreeWRL.h"

#define FW_MAX_KEY_ACTIONS 2

// Cocoa reports arrows, function keys, Home/End etc. as characters in this
// private-use range (NSUpArrowFunctionKey = 0xF700 ...). They are not text.
#define FW_COCOA_FUNCTION_KEY_FIRST 0xF700
#define FW_COCOA_FUNCTION_KEY_LAST  0xF8FF

// Fills actions[] with the FreeWRL key actions to send, in order; returns the count.
// Command-key chords belong to the application menu, so they never produce a
// KEYPRESS (otherwise an unbound Cmd+N would clear the world).
static inline int fw_cocoa_key_actions(int isKeyUp, int commandDown,
                                       unsigned int character,
                                       int actions[FW_MAX_KEY_ACTIONS])
{
	int n = 0;
	if (isKeyUp) {
		actions[n++] = KEYUP;
		return n;
	}
	actions[n++] = KEYDOWN;
	if (!commandDown && !(character >= FW_COCOA_FUNCTION_KEY_FIRST &&
	                      character <= FW_COCOA_FUNCTION_KEY_LAST))
		actions[n++] = KEYPRESS;
	return n;
}

#endif
