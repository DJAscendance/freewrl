/*
 * Regression test for the Cocoa -> FreeWRL keyboard action mapping
 * (OSX_gui/FreeWRL-Desktop/FreeWRL/FWKeyEvents.h). Run ./run.sh.
 */
#include <stdio.h>
#include "../../OSX_gui/FreeWRL-Desktop/FreeWRL/FWKeyEvents.h"

#define NSUpArrowFunctionKey 0xF700
#define NSF1FunctionKey      0xF704

static int failures = 0;

static void expect(const char *name, int isKeyUp, int commandDown, unsigned int ch,
                   int n_expected, int a0, int a1)
{
	int actions[FW_MAX_KEY_ACTIONS] = {0, 0};
	int n = fw_cocoa_key_actions(isKeyUp, commandDown, ch, actions);
	int ok = n == n_expected && (n < 1 || actions[0] == a0) && (n < 2 || actions[1] == a1);
	printf("%s %s: n=%d actions=[%d,%d]\n", ok ? "PASS" : "FAIL", name, n, actions[0], actions[1]);
	if (!ok) failures++;
}

int main(void)
{
	/* The canonical API values fwl_do_keyPress0 and KeySensor dispatch on. */
	if (KEYPRESS != 1 || KEYDOWN != 2 || KEYUP != 3) {
		printf("FAIL canonical KeyAction values changed: PRESS=%d DOWN=%d UP=%d\n",
		       KEYPRESS, KEYDOWN, KEYUP);
		failures++;
	}
	if (KEYPRESS == KEYDOWN || KEYPRESS == KEYUP || KEYDOWN == KEYUP) {
		printf("FAIL KeyAction values are not distinct\n");
		failures++;
	}

	/* 'q' reaches the one-shot command path after the held-key down event. */
	expect("q down -> KEYDOWN, KEYPRESS", 0, 0, 'q', 2, KEYDOWN, KEYPRESS);
	expect("q up -> KEYUP only", 1, 0, 'q', 1, KEYUP, 0);
	expect("space down (command line)", 0, 0, ' ', 2, KEYDOWN, KEYPRESS);
	expect("return down", 0, 0, '\r', 2, KEYDOWN, KEYPRESS);
	/* Command chords are menu shortcuts, never hotkeys. */
	expect("cmd+n down -> KEYDOWN only", 0, 1, 'n', 1, KEYDOWN, 0);
	expect("cmd+n up -> KEYUP", 1, 1, 'n', 1, KEYUP, 0);
	/* Arrows / function keys keep held-key state but are not text. */
	expect("up arrow down -> KEYDOWN only", 0, 0, NSUpArrowFunctionKey, 1, KEYDOWN, 0);
	expect("up arrow up -> KEYUP", 1, 0, NSUpArrowFunctionKey, 1, KEYUP, 0);
	expect("F1 down -> KEYDOWN only", 0, 0, NSF1FunctionKey, 1, KEYDOWN, 0);

	printf(failures ? "FAILED (%d)\n" : "ALL PASS\n", failures);
	return failures ? 1 : 0;
}
