/*

  FreeWRL support library.
  UI declarations.

*/

/****************************************************************************
    This file is part of the FreeWRL/FreeX3D Distribution.

    Copyright 2009 CRC Canada. (http://www.crc.gc.ca)

    FreeWRL/FreeX3D is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FreeWRL/FreeX3D is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FreeWRL/FreeX3D.  If not, see <http://www.gnu.org/licenses/>.
****************************************************************************/



#ifndef __FREEWRL_MAINLOOP_MAIN_H__
#define __FREEWRL_MAINLOOP_MAIN_H__
#include <resources.h>

int fw_exit(int val);
void setDisplayed(int);
void resetSensorEvents(void);

void fwl_replaceWorldNeededRes(resource_item_t *multiResWithParent);

void fwl_gotoCurrentViewPoint();

char* fwl_currentBoundVPname();
void fwl_do_keyPress0(int key, int type);
int fwl_handle_mouse0(int mev, int butnum, int mouseX, int mouseY, int windex);
int fwl_handle_touch0(int mev, unsigned int ID, int mouseX, int mouseY, int windex);
void pushnset_framebuffer(int ibuffer);
void popnset_framebuffer();

#endif /* __FREEWRL_MAINLOOP_MAIN_H__ */
