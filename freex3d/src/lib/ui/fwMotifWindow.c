/*

  FreeWRL support library.
  Create Motif window, widget, menu. Manage events.

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

#include <config.h>
#if !(defined(_ANDROID))

#include <system.h>
#include <display.h>
#if KEEP_FV_INLIB
#include <internal.h>

#include <libFreeWRL.h>

#include <threads.h>

#include "../main/MainLoop.h"
#include "../vrml_parser/Structs.h"
#include "../opengl/OpenGL_Utils.h"
#include "../scenegraph/Viewer.h"
#include "../ui/common.h"

#ifdef CORE_VIEWER
#include "../main/localdefs.h"
#endif


#include <Xm/MainW.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/CascadeB.h>
#include <Xm/TextF.h>
#include <Xm/Separator.h>
#include <Xm/PanedW.h>
#include <Xm/Text.h>
#include <Xm/ScrolledW.h>
#include <Xm/FileSB.h>
#include <Xm/SelectioB.h>
#include <Xm/MessageB.h>
#include <Xm/DrawingA.h> /* simple drawing area */

#define ABOUT_FREEWRL "FreeWRL Version %s\n \
%s %s.\n \n \
FreeWRL is a VRML/X3D Browser for OS X and Unix.\n \n \
Thanks to the Open Source community for all the help received.\n \
http://freewrl.sf.net"

// Call this program "CoreViewer", or "PathPlanner", or just plain old "FreeWRL"
#ifdef CORE_VIEWER
        #define PROGRAM_NAME "CoreViewer"
#else
        #ifdef PATH_PLANNER
                #define PROGRAM_NAME "PathPlanner"
        #else
                #define PROGRAM_NAME "FreeWRL"
        #endif
#endif


#define ABOUT_SCENARIO \
"PathPlanner Version:  %s\n\
Scenario Name:        %s\n\
Description:          %s\n\
Creation Date:        %s\n"



#define DJ_KEEP_COMPILER_WARNING 0

/* static String defaultResources[200]; */
static int MainWidgetRealized = FALSE;

XtAppContext Xtcx;

#ifndef CORE_VIEWER
static Widget freewrlTopWidget, mainw, menubar;
#else
static Widget freewrlTopWidget, mainw;
Widget menubar;
#endif 

static Widget frame, freewrlDrawArea;
static Widget about_widget;
static Widget newFileWidget;


#ifdef CORE_VIEWER
static int consWindowOnscreen = FALSE;
static Widget consolemessageButton = NULL;
static Widget RF_cascade = NULL;
static Widget Content_cascade = NULL;
static Widget consoleTextWidget = NULL;
#endif //CORE_VIEWER


static Arg buttonArgs[10]; static int buttonArgc = 0;

extern char myMenuStatus[];

static void fv_createMenuBar(void);
static void fv_createDrawingFrame(void);
static void fv_createMenuBar(void);
static void fv_createDrawingFrame(void);
static void brightness_scaled(Widget, XtPointer, XtPointer);
static void eleHeight_scaled(Widget, XtPointer, XtPointer);

#ifdef PATH_PLANNER
void SubsetSettings_popUp (Widget w, XtPointer data, XtPointer callData);
void NodeToRFSettings_popUp (Widget w, XtPointer data, XtPointer callData);
void FreqSettings_popUp (Widget w, XtPointer data, XtPointer callData);
void TxHeightSettings_popUp(Widget w, XtPointer data, XtPointer callData);
void StatisticsSettings_popUp(Widget w, XtPointer data, XtPointer callData);
#endif

#ifdef CORE_VIEWER
void CoreViewer_ShowNodeType_popUp(Widget w, XtPointer data, XtPointer callData);
void CoreViewer_MakeNodeIdentifySettings_popUp(Widget w, XtPointer data, XtPointer callData);
void CoreViewer_LinkSettings_popUp(Widget w, XtPointer data, XtPointer callData);
void CoreViewer_GeoLayerSelection_popUp(Widget w, XtPointer data, XtPointer callData);
static void fv_toggleConsolebar (Widget, XtPointer, XtPointer);
#endif


//RF_Parameters_t radioValues[MAX_POSITION_SENSORS];


///////////////////////////////////////////////////////////////////
//
// When enough data is retrieved, we can enable some menu cascades
//
void CoreViewer_enable_RF_cascade(bool yup) {
	if (RF_cascade == NULL) {
		printf ("RF_cascade is NULL, just skipping this\n");
	} else {
		if (yup) XtSetSensitive(RF_cascade,True);
		else XtSetSensitive(RF_cascade,False);
	}
}

void CoreViewer_enable_Content_cascade(bool yup) {
	if (Content_cascade == NULL) {
		printf ("Content_cascade is NULL, just skipping this\n");
	} else {
		if (yup) XtSetSensitive(Content_cascade,True);
		else XtSetSensitive(Content_cascade,False);
	}
}

static void myXtManageChild (int c, Widget child)
{
#ifdef XTDEBUG
    printf ("at %d, managing %d\n",c, child);
#endif
    if (child != NULL) XtManageChild (child);
}


/* see if/when we become iconified - if so, dont bother doing OpenGL stuff */
static void StateWatcher (Widget w, XtPointer unused, XEvent *event, Boolean *cont)
{
#ifdef XEVENT_VERBOSE
    // Used to track down TouchSensor loosing event with Motif (direct X11 is ok)
    TRACE_MSG("freewrlTopWidget [StateWatch] went through (xm callback): widget %p event %p\n", (void*)w, (void*)event);
#endif

    if (event->type == MapNotify) setDisplayed (TRUE);
    else if (event->type == UnmapNotify) setDisplayed (FALSE);

	if (event->type == LeaveNotify) {
		// printf ("JAS - throwing a ButtonRelease from StateWatcher\n");
		void fwl_handle_mouse_window_leave();
		fwl_handle_mouse_window_leave();
	}
}

static void fv_DrawArea_events (Widget w, XtPointer unused, XEvent *event, Boolean *cont)
{
#ifdef XEVENT_VERBOSE 
    // Used to track down TouchSensor loosing event with Motif (direct X11 is ok)

    XWindowAttributes attr;
    XSetWindowAttributes set_attr;

    TRACE_MSG("fv_DrawArea event went through (xm callback): widget %p event %p\n", (void*)w, (void*)event);
    //printf ("fv_DrawArea_events, event->type %d\n",event->type);

    memset(&attr, 0, sizeof(attr));
    memset(&set_attr, 0, sizeof(set_attr));

    /* Get window attributes and examine the event mask */
    XGetWindowAttributes(Xdpy, Xwin, &attr);
    TRACE_MSG("DrawArea event mask: %lu\n", attr.your_event_mask);
    if (!(attr.your_event_mask & PointerMotionMask)) {
	TRACE_MSG("DrawArea window not configured to receive PointerMotionMask...\n");
    }
    /* Set event mask to catch mouse motion events */
    set_attr.event_mask = attr.your_event_mask | PointerMotionMask;
    XChangeWindowAttributes(Xdpy, Xwin, CWEventMask, &set_attr);

#endif

    //printf ("fv_DrawArea_events, event->type %d\n",event->type);

    /* This event should be passed to FreeWRL (MainLoop) control */
    DEBUG_XEV("EVENT through MOTIF\n");
	//printf ("fv_DrawArea_events... calling handle_Xevents %d\n",event->type);
    handle_Xevents(*event);
}



/////////////////////////////////////////////
//
// Change the application icon to whatever is in the
// file "icon.h". See the standalone app "iconToString.c"
// for instructions on how to create "icon.h".
//
// this HAS to be called after widget is realized, after the
//   XtRealizeWidget (toplevel); 
// call

#include "../../../icons/icon.h"

void set_app_icon(Widget top) {
	Display *d = XtDisplay(top);
	Atom net_wm_icon = XInternAtom(d, "_NET_WM_ICON", False);
	Atom cardinal = XInternAtom(d, "CARDINAL", False);
	Window w = XtWindow(top);
	
	//printf ("sizeof buffer %ld\n",sizeof(buffer));
	// first two elements of the icon.h buffer[] are the
	// width and height - not sure which is which, but
	// for this case, it does not matter.

	long wid = 0;
	long hei = 0;
	if (sizeof(buffer) > (sizeof(long) * 2)) {
		wid = buffer[0];
		hei = buffer[1];
	} else {
		printf ("ERROR IN ICON SIZE - nothing there??\n");
		exit(1);
	}

	// printf ("wid %ld hei %ld\n",wid,hei);
	
	// set the icon now.
	int length = 2 + (wid * hei);
	XChangeProperty(d, w, net_wm_icon, cardinal,
        32, PropModeReplace, (const unsigned char*) buffer, length);
}
/**
 *   create_main_window: (virtual) create the window with Motif.
 */
int fv_create_main_window(freewrl_params_t * params) //int argc, char *argv[])
{
	int argc_out = 0;
	char *argv_out[1] = { NULL };
	Dimension width, height;
	Arg initArgs[10]; int initArgc = 0;

	/* XtVaAppInitialize ??? */
	XtSetArg(initArgs[initArgc], XmNlabelString, XmStringCreate(getWindowTitle(), XmSTRING_DEFAULT_CHARSET)); initArgc++;
	XtSetArg(initArgs[initArgc], XmNheight, params->height); initArgc++;
	XtSetArg(initArgs[initArgc], XmNwidth, params->width); initArgc++;
	XtSetArg(initArgs[initArgc], XmNmappedWhenManaged, False); initArgc++;

	/**
	 *   This new initialization sequence let us create the Display and GLX context "à part" from Motif and use the
	 *   same routines for bare X11 and Motif ...
	 */
	XtToolkitInitialize();
	Xtcx = XtCreateApplicationContext();

#ifdef CORE_VIEWER
        XtDisplayInitialize(Xtcx, Xdpy, PROGRAM_NAME, "PathPlanner_class", NULL, 0, &argc_out, argv_out);

        freewrlTopWidget = XtAppCreateShell(PROGRAM_NAME, "PathPlanner_class", applicationShellWidgetClass, Xdpy, initArgs, initArgc);
#else
	XtDisplayInitialize(Xtcx, Xdpy, "FreeWRL", "FreeWRL_class", NULL, 0, &argc_out, argv_out);

	freewrlTopWidget = XtAppCreateShell("FreeWRL", "FreeWRL_class", applicationShellWidgetClass, Xdpy, initArgs, initArgc);
#endif //CORE_VIEWER


	if (!freewrlTopWidget) {
		ERROR_MSG("Can't initialize Motif\n");
		return FALSE;
	}

	/* Inform Motif that we have our visual and colormap already ... (before top level is realized) */
	XtVaSetValues(freewrlTopWidget,
		      XmNdepth, Xvi->depth,
		      XmNvisual, Xvi->visual,
		      XmNcolormap, colormap,
		      NULL);
	
	mainw = XmCreateMainWindow(freewrlTopWidget, getWindowTitle(), NULL, 0);
	if (!mainw)
		return FALSE;
	
	myXtManageChild(29, mainw);
	
	/* Create a menu bar. */
	fv_createMenuBar();
	
	/* Create a framed drawing area for OpenGL rendering. */
	fv_createDrawingFrame();
	
	/* Set up the application's window layout. */
	XtVaSetValues(mainw, 
		      XmNworkWindow, frame,
		      XmNcommandWindow,  NULL,
		      XmNmenuBar, menubar,
		      NULL);
	
	
	XtRealizeWidget (freewrlTopWidget);

	// JAS - set the icon here
	set_app_icon(freewrlTopWidget);

	/* FIXME: see fwBareWindow.c */
	/* Roberto Gerson */
	/* If -d is setted, so reparent the window */
	if (params->winToEmbedInto != INT_ID_UNDEFINED){
		printf("fwMotifWindow::Trying to reparent window: %ld, to new parent: %ld\n",
			XtWindow(freewrlTopWidget),
			params->winToEmbedInto);

		XReparentWindow(XtDisplay(freewrlTopWidget),
				XtWindow(freewrlTopWidget),
				(Window) params->winToEmbedInto, 0, 0);

		XMapWindow(XtDisplay(freewrlTopWidget), XtWindow(freewrlTopWidget));
	}

	XFlush(XtDisplay(freewrlTopWidget));

	MainWidgetRealized = XtIsRealized(freewrlTopWidget); /*TRUE;*/
	TRACE_MSG("fv_create_main_window: top widget realized: %s\n", BOOL_STR(MainWidgetRealized));
	
	Xwin = XtWindow(freewrlTopWidget);
	GLwin = XtWindow(freewrlDrawArea);
	
	/* now, lets tell the OpenGL window what its dimensions are */
	
	XtVaGetValues(freewrlDrawArea, XmNwidth, &width, XmNheight, &height, NULL);
	/* printf("%s,%d fv_create_main_window %d, %d\n",__FILE__,__LINE__,width,height); */
	fv_setScreenDim(width,height);
	
	/* lets see when this goes iconic */
printf ("adding LeaveWindowMask to StateWatcher\n");
	XtAddEventHandler(freewrlTopWidget, 
		LeaveWindowMask |
		StructureNotifyMask, FALSE, StateWatcher, NULL);
	/* all events for DrawArea should be passed to FreeWRL (MainLoop) control */
	XtAddEventHandler(freewrlDrawArea, event_mask, False, fv_DrawArea_events, NULL);

	return TRUE;
}

/************************************************************************

Callbacks to handle button presses, etc.

************************************************************************/

/* Label strings are "broken" on some Motifs. See:
 * http://www.faqs.org/faqs/motif-faq/part5/
 */
/* both of these fail on Ubuntu 6.06 */
/* diastring = XmStringCreateLtoR(ns,XmFONTLIST_DEFAULT_TAG); */
/*diastring = XmStringCreateLocalized(ns); */

static XmString xec_NewString(char *s)
{
    XmString xms1;
    XmString xms2;
    XmString line;
    XmString separator;
    char     *p;
    char     *t = XtNewString(s);   /* Make a copy for strtok not to */
                                    /* damage the original string    */

    separator = XmStringSeparatorCreate();
    p         = strtok(t,"\n");
    xms1      = XmStringCreateLocalized(p);

    /* FIXME: ???? why NULL here */
    while ((p = strtok(NULL,"\n")))
    {
        line = XmStringCreateLocalized(p);
        xms2 = XmStringConcat(xms1,separator);
        XmStringFree(xms1);
        xms1 = XmStringConcat(xms2,line);
        XmStringFree(xms2);
        XmStringFree(line);
    }

    XmStringFree(separator);
    XtFree(t);
    return xms1;
}

/* Callbacks */

// CORE_VIEWER has priority here, as PATH_PLANNER is a subset of CORE_VIEWER
#ifdef CORE_VIEWER
static void fv_aboutCoreViewerScenario (Widget w, XtPointer data, XtPointer callData)

{

        int ac = 0;
        Arg args[10];
        XmString diastring;
        char *SCD, *SD, *SN;
        char *msg = NULL;

        const char *ver = libFreeWRL_get_version();

        // find strings, or default to an info string
        if (ScenarioDescription != NULL) SD = ScenarioDescription;
        else SD = "not supplied";
        if (ScenarioCreationDate != NULL) SCD = ScenarioCreationDate;
        else SCD = "not supplied";
        if (ScenarioName != NULL) SN = ScenarioName;
        else SN = "not supplied";

        // create one text string
        msg = MALLOC(void *, strlen(ABOUT_SCENARIO) + strlen(ver)
                 + strlen(SD) + strlen(SCD) + strlen(SN));
        sprintf(msg, ABOUT_SCENARIO, ver, SN,SD,SCD);

        // set the string into the widget
        diastring = XmStringCreateLtoR(msg,XmSTRING_DEFAULT_CHARSET);
        XtSetArg(args[ac], XmNmessageAlignment, XmALIGNMENT_BEGINNING); ac++;
        XtSetArg(args[ac], XmNmessageString, diastring); ac++;
        XtSetValues(about_widget, args, ac);
        XmStringFree(diastring);
        FREE(msg);

        myXtManageChild(__LINE__,about_widget);
}

#else
#ifdef PATH_PLANNER
static void fv_aboutPathPlannerScenario (Widget w, XtPointer data, XtPointer callData)
{

        int ac = 0;
        Arg args[10];
        XmString diastring;
        char *SCD, *SD, *SN;
        char *msg = NULL;

        const char *ver = libFreeWRL_get_version();

        // find strings, or default to an info string
        if (ScenarioDescription != NULL) SD = ScenarioDescription;
        else SD = "not supplied";
        if (ScenarioCreationDate != NULL) SCD = ScenarioCreationDate;
        else SCD = "not supplied";
        if (ScenarioName != NULL) SN = ScenarioName;
        else SN = "not supplied";

        // create one text string
        msg = MALLOC(void *, strlen(ABOUT_SCENARIO) + strlen(ver)
                 + strlen(SD) + strlen(SCD) + strlen(SN));
        sprintf(msg, ABOUT_SCENARIO, ver, SN,SD,SCD);

        // set the string into the widget
        diastring = XmStringCreateLtoR(msg,XmSTRING_DEFAULT_CHARSET);
        XtSetArg(args[ac], XmNmessageAlignment, XmALIGNMENT_BEGINNING); ac++;
        XtSetArg(args[ac], XmNmessageString, diastring); ac++;
        XtSetValues(about_widget, args, ac);
        XmStringFree(diastring);
        FREE(msg);

        myXtManageChild(__LINE__,about_widget);
}

#endif //PATH_PLANNER
#endif //CORE_VIEWER

/* UI_Settings pulldown menu */
static void fv_createUI_SettingsPulldown()
{
    Widget cascade, menupane;
    Widget eleHeight_sb, brightness_sb;
    Widget eleHeight_label, brightness_label;
    int ac=0;
    Arg args[10];
           
	ac=0;
	XtSetArg (args[ac], XmNisHomogeneous, False); ac++;
	menupane = XmCreatePulldownMenu (menubar, "menupane", args, ac);
        
	/* Console Message */
	myXtManageChild(__LINE__,XmCreateSeparator (menupane, "sep1", NULL, 0));
	consolemessageButton = XtCreateManagedWidget("Console Display",
                                                 xmToggleButtonWidgetClass, menupane, buttonArgs, buttonArgc);
	XtAddCallback(consolemessageButton, XmNvalueChangedCallback, 
                  (XtCallbackProc)fv_toggleConsolebar, NULL);
	myXtManageChild (__LINE__,consolemessageButton);

	ac=0;
	XtSetArg (args[ac], XmNsubMenuId, menupane); ac++;
	//XtSetArg (args[ac], XmNisHomogeneous, False); ac++;
	cascade = XmCreateCascadeButton (menubar, "UI Settings", args, ac);
	// myXtManageChild (__LINE__,cascade);

 	// scroll bars for the elevation height, and the background brightness
	myXtManageChild(__LINE__,XmCreateSeparator (menupane, "sep1", NULL, 0));
	eleHeight_label = XmCreateLabel (menupane, "Elevation Height", NULL, 0);
	myXtManageChild(__LINE__,eleHeight_label);

	ac = 0;
	XtSetArg (args[ac], XmNorientation, XmHORIZONTAL); ac++;
	XtSetArg (args[ac], XmNmaximum, 1010); ac++;
	XtSetArg (args[ac], XmNvalue, 500); ac++;
	eleHeight_sb = XmCreateScale(menupane,"Elevation Slider", args, ac);
	XtAddCallback(eleHeight_sb, XmNvalueChangedCallback, eleHeight_scaled, NULL);
	XtAddCallback(eleHeight_sb, XmNdragCallback, eleHeight_scaled, NULL);
	myXtManageChild (__LINE__,eleHeight_sb);

	myXtManageChild(__LINE__,XmCreateSeparator (menupane, "sep1", NULL, 0));

	brightness_label = XmCreateLabel (menupane, "Elevation Brightness", NULL, 0);
	myXtManageChild(__LINE__,brightness_label);

	ac = 0;
	XtSetArg (args[ac], XmNorientation, XmHORIZONTAL); ac++;
	XtSetArg (args[ac], XmNmaximum, 1010); ac++;
	XtSetArg (args[ac], XmNvalue, 700); ac++;
	brightness_sb = XmCreateScale(menupane,"Brightness Slider", args, ac);
	XtAddCallback(brightness_sb, XmNvalueChangedCallback, brightness_scaled, NULL);
	XtAddCallback(brightness_sb, XmNdragCallback, brightness_scaled, NULL);

	myXtManageChild (__LINE__,brightness_sb);
	myXtManageChild(__LINE__,XmCreateSeparator (menupane, "sep2", NULL, 0));

    myXtManageChild (__LINE__,cascade);
}


#ifdef PATH_PLANNER
/* RF_Settings pulldown menu */
static void fv_createRF_SettingsPulldown()
{
        Widget btn, menupane;
        Arg args[10];
        int ac = 0;

        menupane = XmCreatePulldownMenu (menubar, "menupane", NULL, 0);

        /* Helpity stuff */
        ac = 0;

        //fv_removeWidgetFromSelect (frequencySettings_widget, XmDIALOG_CANCEL_BUTTON);


        btn = XmCreatePushButton (menupane, "Area Subsetting", NULL, 0);
        XtAddCallback (btn, XmNactivateCallback, (XtCallbackProc)SubsetSettings_popUp, NULL);
        myXtManageChild (__LINE__,btn);
#ifdef CORE_VIEWER
        btn = XmCreatePushButton (menupane, "Node to RF Channel Assignment", NULL, 0);
        XtAddCallback (btn, XmNactivateCallback, (XtCallbackProc)NodeToRFSettings_popUp, NULL);
        myXtManageChild (__LINE__,btn);
#endif //CORE_VIEWER
        btn = XmCreatePushButton (menupane, "WLAN Frequency", NULL, 0);
        XtAddCallback (btn, XmNactivateCallback, (XtCallbackProc)FreqSettings_popUp, NULL);
        myXtManageChild (__LINE__,btn);
        btn = XmCreatePushButton (menupane, "Heights, Combining Algorithm", NULL, 0);
        XtAddCallback (btn, XmNactivateCallback, (XtCallbackProc)TxHeightSettings_popUp, NULL);
        myXtManageChild (__LINE__,btn);
        btn = XmCreatePushButton (menupane, "Reception Statistics", NULL, 0);
        XtAddCallback (btn, XmNactivateCallback, (XtCallbackProc)StatisticsSettings_popUp, NULL);
        myXtManageChild (__LINE__,btn);

        ac = 0;
        XtSetArg (args[ac], XmNsubMenuId, menupane); ac++;
        RF_cascade = XmCreateCascadeButton (menubar, "RF Settings", args, ac);

        // make insensitive, until the info required for PathPlanner is found
        XtSetSensitive (RF_cascade, False);

        myXtManageChild (__LINE__,RF_cascade);
}
#endif  //PATH_PLANNER

#ifdef CORE_VIEWER

// the back end wants to display an error message.
static Widget prompt, info, info_dialog;
void info_activate(Widget dialog) {
    // printf("Info Ok was pressed.\n");
}

void GUI_display_error_string(char *title, char *errmsg) {
        XmString xm_string;
        Arg args[1];
        Widget remove;
/* Create InformationDialog */

    /* Label the dialog */

    xm_string = XmStringCreateLocalized(errmsg);
    XtSetArg(args[0], XmNmessageString, xm_string);

    /* Create the InformationDialog */

        info_dialog = XmCreateInformationDialog(mainw, title, args, 1);
        remove = XmMessageBoxGetChild(info_dialog, XmDIALOG_HELP_BUTTON);
        XtUnmanageChild(remove);
        remove = XmMessageBoxGetChild(info_dialog, XmDIALOG_CANCEL_BUTTON);
        XtUnmanageChild(remove);

    XmStringFree(xm_string);

    XtAddCallback(info_dialog, XmNokCallback, info_activate, NULL);

    /* Create Warning DIalog */

        XtManageChild(info_dialog);
}
static void fv_createContent_SettingsPulldown()
{
        Widget btn, menupane;
        Arg args[10];
        int ac = 0;

        menupane = XmCreatePulldownMenu (menubar, "menupane", NULL, 0);

        /* Helpity stuff */
        ac = 0;

        //fv_removeWidgetFromSelect (frequencySettings_widget, XmDIALOG_CANCEL_BUTTON);


        btn = XmCreatePushButton (menupane, "Show Node Types ", NULL, 0);
        XtAddCallback (btn, XmNactivateCallback,
                (XtCallbackProc)CoreViewer_ShowNodeType_popUp, NULL);
        myXtManageChild (__LINE__,btn);

        btn = XmCreatePushButton (menupane, "Identify Nodes ", NULL, 0);
        XtAddCallback (btn, XmNactivateCallback,
                (XtCallbackProc)CoreViewer_MakeNodeIdentifySettings_popUp, NULL);
        myXtManageChild (__LINE__,btn);

        btn = XmCreatePushButton (menupane, "Link Settings ", NULL, 0);
        XtAddCallback (btn, XmNactivateCallback,
                (XtCallbackProc)CoreViewer_LinkSettings_popUp, NULL);
        myXtManageChild (__LINE__,btn);

        btn = XmCreatePushButton (menupane, "GeoLayer Selection", NULL, 0);
        XtAddCallback (btn, XmNactivateCallback,
                (XtCallbackProc)CoreViewer_GeoLayerSelection_popUp, NULL);
        myXtManageChild (__LINE__,btn);

        ac = 0;
        XtSetArg (args[ac], XmNsubMenuId, menupane); ac++;
        Content_cascade = XmCreateCascadeButton (menubar, "Content Settings", args, ac);


        // make insensitive, until we have all nodes read in
        XtSetSensitive(Content_cascade,False);

        myXtManageChild (__LINE__,Content_cascade);
}


#endif //CORE_VIEWER


static void fv_aboutFreeWRLpopUp (Widget w, XtPointer data, XtPointer callData)
{ 

    int ac;
    Arg args[10];
    const char *ver;
    char *msg, *rdr, *vendor;
    XmString diastring;
    ac = 0;

    ver = libFreeWRL_get_version();

    rdr = (char *)glGetString(GL_RENDERER); // JAS - was gglobal()->display.rdr_caps.renderer;
    vendor = (char *)glGetString(GL_VENDOR); // JAS - was gglobal()->display.rdr_caps.vendor;

    msg = MALLOC(void *, strlen(ABOUT_FREEWRL) + strlen(ver)
		 + strlen(rdr) + strlen(vendor));
    sprintf(msg, ABOUT_FREEWRL, ver, rdr, vendor);

    diastring = xec_NewString(msg);     
    XtSetArg(args[ac], XmNmessageString, diastring); ac++;
    XtSetValues(about_widget, args, ac);
    XmStringFree(diastring);
    FREE(msg);

    myXtManageChild(2,about_widget);
}

/* quit selected */
static void fv_quitMenuBar (Widget w, XtPointer data, XtPointer callData)
{ 
    fwl_doQuit(__FILE__,__LINE__);
}

static void fv_reloadFile (Widget w, XtPointer data, XtPointer callData)
{
	ConsoleMessage ("reloading %s", BrowserFullPath);
	/* FIXME: implement reload function */
}


/* do we want a console window displaying errors, etc? */
static void fv_toggleConsolebar (Widget w, XtPointer data, XtPointer callData)
{
    consWindowOnscreen = !consWindowOnscreen; /* keep track of state */
    XmToggleButtonSetState (consolemessageButton,consWindowOnscreen,FALSE); /* display blip if on */
    if (consWindowOnscreen) myXtManageChild (__LINE__,consoleTextWidget); /* display (or not) console window */
    else XtUnmanageChild (consoleTextWidget);
}

/* file selection dialog box, ok button pressed */
static void fv_fileSelectPressed (Widget w, XtPointer data, XmFileSelectionBoxCallbackStruct *callData)
{
    char *newfile;

    /* get the filename */
    XmStringGetLtoR(callData->value, 
                    XmSTRING_DEFAULT_CHARSET, &newfile);

    if (!Anchor_ReplaceWorld(newfile)) {
	    /* error message */
    }
    XtUnmanageChild(w);
}

/* file selection dialog box cancel button - just get rid of widget */
static void fv_unManageMe (Widget widget, XtPointer client_data, 
                 XmFileSelectionBoxCallbackStruct *selection)
{
    XtUnmanageChild(widget);
}

/* new file popup - user wants to load a new file */ 
static void fv_newFilePopup(Widget cascade_button, char *text, XmPushButtonCallbackStruct *cbs)
{
    myXtManageChild(4,newFileWidget);
    XtPopup(XtParent(newFileWidget), XtGrabNone); 
}

#ifdef DOESNOTGETICONICSTATE
/* resize, configure events */
static void fv_GLAreaexpose (Widget w, XtPointer data, XtPointer callData)
{
    XmDrawingAreaCallbackStruct *cd = (XmDrawingAreaCallbackStruct *) callData;
    switch (cd->reason) {
    case XmCR_EXPOSE: printf ("got expose event \n");
    default: printf ("not known event, %d\n",cd->reason);
    }
}
#endif

/* resize, configure events */
static void fv_GLArearesize (Widget w, XtPointer data, XtPointer callData)
{
/*     XmDrawingAreaCallbackStruct *cd = (XmDrawingAreaCallbackStruct *) callData; */
    Dimension width, height;

    XtVaGetValues (w, XmNwidth, &width, XmNheight, &height, NULL);
    /* printf("%s,%d GLArearesize %d, %d\n",__FILE__,__LINE__,width,height); */
    fv_setScreenDim (width,height);
}

/* remove this button from this SelectionBox widget */
static void fv_removeWidgetFromSelect (Widget parent, 
#if NeedWidePrototypes
                             unsigned int 
#else
                             unsigned char
#endif
                             button) {

    Widget tmp;

    tmp = XmSelectionBoxGetChild(parent, button);
    if (tmp == NULL) {
        printf ("hmmm - button does not exist\n");
    } else {
        XtUnmanageChild(tmp);
    }
}


/* start up the browser, and point it to www.freewrl.org */
static void fv_pathPlannerHelpPopup (Widget w, XtPointer data, XtPointer callData)
{ 
#if DJ_KEEP_COMPILER_WARNING
	#define MAXLINE 2000
#endif
	const char *browser;
	char *sysline;
	const char pattern[] = "%s http://www.freewrl.org &";

	browser = freewrl_get_browser_program();
	if (!browser) {
		browser = BROWSER;
	}
	sysline = MALLOC(char *, strlen(browser)+strlen(pattern));
	sprintf(sysline, pattern, browser);

	freewrlSystem(sysline);

	FREE(sysline);
}

// callback for brightness scale dragged or clicked
static void brightness_scaled (Widget scrollbar, XtPointer client_data, XtPointer call_data) {
	float pc;

	// values from the slider go from 0 to 1000. Make that into a percent
	XmScaleCallbackStruct *cbs = (XmScaleCallbackStruct *) call_data;
	pc = ((float) cbs->value)/10.0;
	// printf ("elebrightness %f\n",pc);
	sliderBrightness(pc);
}

// callback for height scale dragged or clicked
static void eleHeight_scaled (Widget scrollbar, XtPointer client_data, XtPointer call_data) {
	float pc;
	XmScaleCallbackStruct *cbs = (XmScaleCallbackStruct *) call_data;
	pc = ((float) cbs->value)/10.0;
	//printf ("eleHeight %f\n",pc);
	sliderElevation(pc);
}

/* start up the browser, and point it to www.crc.ca/FreeWRL */
static void fv_freewrlHomePopup (Widget w, XtPointer data, XtPointer callData)
{ 
#if DJ_KEEP_COMPILER_WARNING
	#define MAXLINE 2000
#endif
	const char *browser;
	char *sysline;
	const char pattern[] = "%s http://freewrl.sourceforge.net &";

	browser = freewrl_get_browser_program();
	if (!browser) {
		browser = BROWSER;
	}
	sysline = MALLOC(char *, strlen(browser)+strlen(pattern));
	sprintf(sysline, pattern, browser);

	freewrlSystem(sysline);

	FREE(sysline);
}

#ifdef XTDEBUG
/* for debugging... */
printEvent (XEvent event)
{
    switch (event.type) {
    case KeyPress: printf ("KeyPress"); break;
    case KeyRelease: printf ("KeyRelease"); break;
    case ButtonPress: printf ("ButtonPress"); break;
    case ButtonRelease: printf ("ButtonRelease"); break;
    case MotionNotify: printf ("MotionNotify"); break;
    case EnterNotify: printf ("EnterNotify"); break;
    case LeaveNotify: printf ("LeaveNotify"); break;
    case FocusIn: printf ("FocusIn"); break;
    case FocusOut: printf ("FocusOut"); break;
    case KeymapNotify: printf ("KeymapNotify"); break;
    case Expose: printf ("Expose"); break;
    case GraphicsExpose: printf ("GraphicsExpose"); break;
    case NoExpose: printf ("NoExpose"); break;
    case VisibilityNotify: printf ("VisibilityNotify"); break;
    case CreateNotify: printf ("CreateNotify"); break;
    case DestroyNotify: printf ("DestroyNotify"); break;
    case UnmapNotify: printf ("UnmapNotify"); break;
    case MapNotify: printf ("MapNotify"); break;
    case MapRequest: printf ("MapRequest"); break;
    case ReparentNotify: printf ("ReparentNotify"); break;
    case ConfigureNotify: printf ("ConfigureNotify"); break;
    case ConfigureRequest: printf ("ConfigureRequest"); break;
    case GravityNotify: printf ("GravityNotify"); break;
    case ResizeRequest: printf ("ResizeRequest"); break;
    case CirculateNotify: printf ("CirculateNotify"); break;
    case CirculateRequest: printf ("CirculateRequest"); break;
    case PropertyNotify: printf ("PropertyNotify"); break;
    case SelectionClear: printf ("SelectionClear"); break;
    case SelectionRequest: printf ("SelectionRequest"); break;
    case SelectionNotify: printf ("SelectionNotify"); break;
    case ColormapNotify: printf ("ColormapNotify"); break;
    case ClientMessage: printf ("ClientMessage"); break;
    case MappingNotify: printf ("MappingNotify"); break;
    default :printf ("Event out of range - %d",event.type);
    }
    printf ("\n");
}
#endif

/* File pulldown menu */
static void fv_createFilePulldown()
{
    Widget menupane, btn, cascade;

    XmString mask;
    int ac;
    Arg args[10];
           
    /* Create the FileSelectionDialog */     
    memset(args, 0, sizeof(args));
    ac = 0;
    mask  = XmStringCreateLocalized("*.x3d");
    XtSetArg(args[ac], XmNdirMask, mask); ac++;

    /* newFileWidget = XmCreateFileSelectionDialog(menubar, "select", args, 1); */
    newFileWidget = XmCreateFileSelectionDialog(mainw, "select", args, 1);        

    XtAddCallback(newFileWidget, XmNokCallback, (XtCallbackProc)fv_fileSelectPressed, NULL);
    XtAddCallback(newFileWidget, XmNcancelCallback, (XtCallbackProc)fv_unManageMe, NULL);
    /* delete buttons not wanted */
    fv_removeWidgetFromSelect(newFileWidget,XmDIALOG_HELP_BUTTON);
    XtUnmanageChild(newFileWidget);


    menupane = XmCreatePulldownMenu (menubar, "menupane", NULL, 0);
    btn = XmCreatePushButton (menupane, "Reload", NULL, 0);
    XtAddCallback (btn, XmNactivateCallback, (XtCallbackProc)fv_reloadFile, NULL);
    myXtManageChild (5,btn);
    btn = XmCreatePushButton (menupane, "New...", NULL, 0);
    XtAddCallback (btn, XmNactivateCallback, (XtCallbackProc)fv_newFilePopup, NULL);
    myXtManageChild (6,btn);

    btn = XmCreatePushButton (menupane, "Quit", NULL, 0);
    XtAddCallback (btn, XmNactivateCallback, (XtCallbackProc)fv_quitMenuBar, NULL);
    myXtManageChild (7,btn);
    XtSetArg (args[0], XmNsubMenuId, menupane);
    cascade = XmCreateCascadeButton (menubar, "File", args, 1);
    myXtManageChild (8,cascade);
}

static void fv_createHelpPulldown()
{
    Widget btn, menupane, cascade;
    int ac;
    Arg args[10];


    menupane = XmCreatePulldownMenu (menubar, "menupane", NULL, 0);

    /* Helpity stuff */
    ac = 0;
    /*
      sprintf (ns,ABOUT_FREEWRL,getLibVersion(),"","");
      diastring = xec_NewString(ns);

      XtSetArg(args[ac], XmNmessageString, diastring); ac++;
    */
    XtSetArg(args[ac], XmNmessageAlignment,XmALIGNMENT_CENTER); ac++;
    about_widget = XmCreateInformationDialog(menubar, "about", args, ac);        
    XtAddCallback(about_widget, XmNokCallback, (XtCallbackProc)fv_unManageMe, NULL);
    fv_removeWidgetFromSelect (about_widget, XmDIALOG_CANCEL_BUTTON);
    /*
      causes segfault on Core3 fv_removeWidgetFromSelect (about_widget, XmDIALOG_HELP_BUTTON);
    */

#ifdef CORE_VIEWER
        btn = XmCreatePushButton (menupane, "About CoreViewer Scenario...", NULL, 0);
        XtAddCallback (btn, XmNactivateCallback, (XtCallbackProc)fv_aboutCoreViewerScenario, NULL);
        myXtManageChild (__LINE__,btn);
        btn = XmCreatePushButton (menupane, "CoreViewer Help Pages...", NULL, 0);
        XtAddCallback (btn, XmNactivateCallback, (XtCallbackProc)fv_pathPlannerHelpPopup, NULL);
        myXtManageChild (__LINE__,btn);
#else

#ifdef PATH_PLANNER
                btn = XmCreatePushButton (menupane, "About PathPlanner Scenario...", NULL, 0);
                XtAddCallback (btn, XmNactivateCallback, (XtCallbackProc)fv_aboutPathPlannerScenario, NULL);
                myXtManageChild (__LINE__,btn);
                btn = XmCreatePushButton (menupane, "PathPlanner Help Pages...", NULL, 0);
                XtAddCallback (btn, XmNactivateCallback, (XtCallbackProc)fv_pathPlannerHelpPopup, NULL);
                myXtManageChild (__LINE__,btn);
#else
    btn = XmCreatePushButton (menupane, "About FreeWRL...", NULL, 0);
    XtAddCallback (btn, XmNactivateCallback, (XtCallbackProc)fv_aboutFreeWRLpopUp, NULL);
    myXtManageChild (23,btn);
    btn = XmCreatePushButton (menupane, "FreeWRL Homepage...", NULL, 0);
    XtAddCallback (btn, XmNactivateCallback, (XtCallbackProc)fv_freewrlHomePopup, NULL);
    myXtManageChild (24,btn);
        #endif //PATH_PLANNER
#endif //CORE_VIEWER


    XtSetArg (args[0], XmNsubMenuId, menupane);
    cascade = XmCreateCascadeButton (menubar, "Help", args, 1);
    myXtManageChild (25,cascade);
}

/**********************************/
static void fv_createMenuBar(void)
{
    Arg menuArgs[10]; int menuArgc = 0;

    /* create the menu bar */
    memset(menuArgs, 0, sizeof(menuArgs));
    menuArgc = 0;
        
    /* the following XtSetArg is not required; it only "pretties" up the display
       in some circumstances. It came out in Motif 2.0, and is not always found */
#ifdef XmNscrolledWindowChildType
    XtSetArg(menuArgs[menuArgc], XmNscrolledWindowChildType, XmMENU_BAR); menuArgc++;
#endif

    menubar = XmCreateMenuBar (mainw, "menubar", menuArgs, menuArgc);
    myXtManageChild (26,menubar);

    /* generic toggle button resources */
    XtSetArg(buttonArgs[buttonArgc], XmCVisibleWhenOff, TRUE); buttonArgc++;
    XtSetArg(buttonArgs[buttonArgc],XmNindicatorType,XmN_OF_MANY); buttonArgc++;

    if (!RUNNINGASPLUGIN) fv_createFilePulldown();

    fv_createUI_SettingsPulldown();
    //printf ("fv_createMenuBar, line %d\n",__LINE__);

    #ifdef PATH_PLANNER
    fv_createRF_SettingsPulldown();
    //pathPlanner_createRF_SettingsPulldown(menubar);
    //printf ("fv_createMenuBar, line %d\n",__LINE__);
    #endif
    #ifdef CORE_VIEWER
    fv_createContent_SettingsPulldown();
    #endif


    fv_createHelpPulldown();

}

/**********************************************************************************/
/*
  create a frame for FreeWRL, and for messages
*/
static void fv_createDrawingFrame(void)
{
    /* frame holds everything here */
    frame = XtVaCreateManagedWidget("form", xmPanedWindowWidgetClass, mainw, NULL);

    /* create the FreeWRL OpenGL drawing area, and map it. */

#if 0 /* MB: do not create a glwDrawingArea but a simple widget
	 we have our own initialization of OpenGL ...
	 in the near future we could remove completely the GLwDrawA files...
      */
    freewrlDrawArea = XtVaCreateManagedWidget ("freewrlDrawArea", glwDrawingAreaWidgetClass,
                                               frame, "visualInfo", Xvi, 
                                               XmNtopAttachment, XmATTACH_WIDGET,
                                               XmNbottomAttachment, XmATTACH_FORM,
                                               XmNleftAttachment, XmATTACH_FORM,
                                               XmNrightAttachment, XmATTACH_FORM,
                                               NULL);
#endif

    freewrlDrawArea = XmCreateDrawingArea (frame, "drawing_a", NULL, 0);

#ifdef DOESNOTGETICONICSTATE
    XtAddCallback (freewrlDrawArea, XmNexposeCallback, fv_GLAreaexpose, NULL);
#endif

    XtAddCallback (freewrlDrawArea, XmNresizeCallback, fv_GLArearesize, NULL);

    myXtManageChild(27,freewrlDrawArea);
}

void setConsoleMessage (char *str)
{
// we do not do this anymore, Doug Sanden's HUD code does lots now
}



// this does nothing anymore, Doug Sanden's HUD code replaces this.
void frontendUpdateButtons()
{
}

#endif /* IPHONE */
#endif /* KEEP_FV_INLIB */
