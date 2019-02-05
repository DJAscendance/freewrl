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

#define DJ_KEEP_COMPILER_WARNING 0

/* static String defaultResources[200]; */
static int MainWidgetRealized = FALSE;

XtAppContext Xtcx;

static Widget freewrlTopWidget, mainw, menubar;
static Widget frame, freewrlDrawArea;
static Widget about_widget;
static Widget newFileWidget;

static Arg buttonArgs[10]; static int buttonArgc = 0;

extern char myMenuStatus[];

static void fv_createMenuBar(void);
static void fv_createDrawingFrame(void);


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
}

static void fv_DrawArea_events (Widget w, XtPointer unused, XEvent *event, Boolean *cont)
{
#ifdef XEVENT_VERBOSE 
    // Used to track down TouchSensor loosing event with Motif (direct X11 is ok)

    XWindowAttributes attr;
    XSetWindowAttributes set_attr;

    TRACE_MSG("fv_DrawArea event went through (xm callback): widget %p event %p\n", (void*)w, (void*)event);

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

    /* This event should be passed to FreeWRL (MainLoop) control */
    DEBUG_XEV("EVENT through MOTIF\n");
    handle_Xevents(*event);
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

	XtDisplayInitialize(Xtcx, Xdpy, "FreeWRL", "FreeWRL_class", NULL, 0, &argc_out, argv_out);

	freewrlTopWidget = XtAppCreateShell("FreeWRL", "FreeWRL_class", applicationShellWidgetClass, Xdpy, initArgs, initArgc);

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
	XtAddEventHandler(freewrlTopWidget, StructureNotifyMask, FALSE, StateWatcher, NULL);
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


    btn = XmCreatePushButton (menupane, "About FreeWRL...", NULL, 0);
    XtAddCallback (btn, XmNactivateCallback, (XtCallbackProc)fv_aboutFreeWRLpopUp, NULL);
    myXtManageChild (23,btn);
    btn = XmCreatePushButton (menupane, "FreeWRL Homepage...", NULL, 0);
    XtAddCallback (btn, XmNactivateCallback, (XtCallbackProc)fv_freewrlHomePopup, NULL);
    myXtManageChild (24,btn);

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
