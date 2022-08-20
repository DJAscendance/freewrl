/*


Javascript C language binding.

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
#ifdef JAVASCRIPT_SM
#if defined(JS_SMCPP)
#undef DEBUG
//#define DEBUG 1 //challenge it with lots of ASSERTS, just for cleaning up code correctness, not production
//#include <system.h>
//# include <stdint.h>
//#include <ctype.h>
//#undef HAVE_BOOL
//#include <stdlib.h>
//#include <math.h>
//# include <stdio.h>
//# include <string.h>

# include <jsapi.h> /* JS compiler */
//# include <jsdbgapi.h> /* JS debugger */

//#if !(defined(JAVASCRIPT_STUB) || defined(JAVASCRIPT_DUK))

#define JS_VERSION 187
static int js_run_version = JS_VERSION; //may be over-ridden below when more info avail
//#define JS_THREADSAFE 1 //by default in 186+

#define STRING_SIZE 256
#define uintN unsigned
#define intN int
#define jsint int32_t
#define jsuint uint32_t
#define int32 int32_t
#define jsdouble double

#define JS_FinalizeStub NULL
#define JS_GET_CLASS JS_GetClassFw
JSBool JS_NewNumberValue(JSContext *cx, jsdouble d, jsval *rval);
//#define JSVAL_IS_OBJECT(retval) JSVAL_IS_OBJECT_OR_NULL_IMPL(retval)


//typedef _Bool bool;


#define JS_GET_PROPERTY_STUB JS_PropertyStub
/* #define JS_GET_PROPERTY_STUB js_GetPropertyDebug */

#define JS_SET_PROPERTY_STUB1 js_SetPropertyDebug1

/* #define JS_SET_PROPERTY_STUB2 js_SetPropertyDebug2  */
# define JS_SET_PROPERTY_STUB2 JS_StrictPropertyStub
#define JS_SET_PROPERTY_STUB3 js_SetPropertyDebug3 


extern "C" {
#ifndef IBOOL
typedef int IBOOL;
#endif
typedef IBOOL _Bool;
#include <system.h>
#include "scenegraph/Vector.h"
//#include <display.h>
#include <internal.h>
#include "JScript.h"


//
//#include <libFreeWRL.h>
//
#include "../vrml_parser/Structs.h"
#include <iglobal.h>
//#include "../main/headers.h"
//#include "../vrml_parser/CParseGeneral.h"
//#include "../vrml_parser/CRoutes.h"
//#include "../main/Snapshot.h"
//#include "../scenegraph/Collision.h"
//#include "../scenegraph/quaternion.h"
//#include "../scenegraph/Viewer.h"
//#include "../input/EAIHelpers.h"
//#include "../input/SensInterps.h"
//#include "../x3d_parser/Bindable.h"
#include "../vrml_parser/CRoutes.h"
#include "jsNative.h"

void Parser_scanStringValueToMem_B(union anyVrml* any, indexT ctype, const char *value, int isXML);
int getJsEngineVariant();

} //extern "C"


#include "jsVRMLBrowser_sm.h"
//#include "CScripts.h"
#include "jsUtils_sm.h"
#include "jsVRMLClasses_sm.h"


#ifndef JSCLASS_GLOBAL_FLAGS
//spidermonkey < 1.7 doesn't have so define here
#define JSCLASS_GLOBAL_FLAGS 0
#endif

#ifdef JAVASCRIPTVERBOSE
int ActualrunScript(int num, char *script, jsval *rval, char *fn, int line);
#else
int ActualrunScript(int num, char *script, jsval *rval);
#endif


static JSClass staticGlobalClass = {
	"global",		// char *name
	JSCLASS_GLOBAL_FLAGS,	// uint32 flags
	JS_PropertyStub,	// JSPropertyOp addProperty
	JS_DeletePropertyStub,	// JSDeletePropertyOp delProperty
	JS_PropertyStub,	// JSPropertyOp getProperty
	JS_StrictPropertyStub,	// JSStrictPropertyOp setProperty
	JS_EnumerateStub,	// JSEnumerateOp enumerate
	globalResolve,		// JSResolveOp resolve
	JS_ConvertStub,		// JSConvertOp convert
	// following are optional and can be NULL
	JS_FinalizeStub,	// JSFinalizeOp finalize
	NULL,			// JSClassInternal reserved
	NULL,			// JSCheckAccessOp checkAccess
	NULL,			// JSNative call
	NULL,			// JSNative construct
	NULL,			// JSXDRObjectOp xdrObject
	NULL,			// JSJasInstanceOp hasInstance
	NULL			// JSTraceOp trace
};



extern "C" {


typedef struct pJScript{
	JSRuntime *runtime;// = NULL;
	JSClass globalClass;
	jsval JSglobal_return_value;
	struct Shader_Script *current_script; //when in some js callback, you get cx, obj. But what script? Set in the call stack
	int ijunk;
}* ppJScript;

void *JScript_constructor(){
	void *v = MALLOCV(sizeof(struct pJScript));
	memset(v,0,sizeof(struct pJScript));
	return v;
}


void JScript_init(struct iiglobal::tJScript *t){
	//public
	t->JSglobal_return_val = NULL;
	//private
	t->prv = JScript_constructor();
	{
		ppJScript p = (ppJScript)t->prv;

		p->runtime = NULL;
		memcpy(&p->globalClass,&staticGlobalClass,sizeof(staticGlobalClass));
		t->JSglobal_return_val = &p->JSglobal_return_value;
		p->current_script = NULL;

	}
}
//	ppJScript p = (ppJScript)gglobal()->JScript.prv;


void sm_js_cleanup_script_context(int counter){
	ttglobal tg = gglobal();
	ppJScript p = (ppJScript)tg->JScript.prv;
	JSContext *cx;
	JSObject *global;

	cx = (JSContext *)getScriptControlIndex(counter)->cx;
	global = (JSObject *)getScriptControlIndex(counter)->glob;
	{ // Scope A  for our various stack objects (JSAutoRequest, RootedObject), so they all go
		// out of scope before we JS_DestroyContext.
		JSAutoRequest ar(cx); // In practice, you would want to exit this any
							// time you're spinning the event loop
		{ // Scope B  for JSAutoCompartment
			JSAutoCompartment ac(cx, global);
			JS_MaybeGC(cx); 
		} //Scope B
	} //Scope A

}

/********************************************************************

process_eventsProcessed()

According to the spec, all scripts can have an eventsProcessed
function - see section C.4.3 of the spec.

********************************************************************/
/* run the script from within C */
void sm_process_eventsProcessed() {

	int counter;
	jsval retval;
	struct CRscriptStruct *scriptcontrol;
	ttglobal tg = gglobal();
	//ppJScript p = (ppJScript)tg->JScript.prv;
	for (counter = 0; counter <= tg->CRoutes.max_script_found_and_initialized; counter++) {
		scriptcontrol = getScriptControlIndex(counter);
		if(scriptcontrol->thisScriptType != NOSCRIPT ){
			JSContext *cx = (JSContext *)scriptcontrol->cx;
			JSObject *obj = (JSObject *)scriptcontrol->glob;
			{ // Scope A for our various stack objects (JSAutoRequest, RootedObject), so they all go
				// out of scope before we JS_DestroyContext.
				JSAutoRequest ar(cx); // In practice, you would want to exit this any
									// time you're spinning the event loop
				{ // Scope B for JSAutoCompartment
					JSAutoCompartment ac(cx, obj);
					if (scriptcontrol->eventsProcessed == NULL) {
						scriptcontrol->eventsProcessed = (void *)JS_CompileScript(cx,obj,"eventsProcessed(__eventInTickTime)", strlen ("eventsProcessed(__eventInTickTime)"),
							"compile eventsProcessed()", 1);
						if (!JS_AddObjectRoot(cx,(JSObject**)(&scriptcontrol->eventsProcessed))) {
							printf ("can not add object root for compiled eventsProcessed() for script %d\n",counter);
						}
					}
					SET_JS_TICKTIME
					if (!JS_ExecuteScript(cx,obj,(JSScript *)scriptcontrol->eventsProcessed, &retval)) {
						printf ("can not run eventsProcessed() for script %d\n",counter);
					}
				} //Scope B
			} //Scope A
		} //if !noscript
	} //for counter
}




void sm_jsClearScriptControlEntries(int num) //struct CRscriptStruct *ScriptControl)
{
	struct CRscriptStruct *ScriptControl;
	ScriptControl = getScriptControlIndex(num);
	if (ScriptControl->eventsProcessed != NULL) {

		if (ScriptControl->cx != NULL) {
			JS_RemoveObjectRoot((JSContext *)ScriptControl->cx,(JSObject**)(&ScriptControl->eventsProcessed));
		}

		ScriptControl->eventsProcessed = NULL;
	}
}



/* MAX_RUNTIME_BYTES controls when garbage collection takes place. */
//#define MAX_RUNTIME_BYTES 0xB00000L
//#define MAX_RUNTIME_BYTES 0xC00000L
//#define MAX_RUNTIME_BYTES 0x1000000L 
#define MAX_RUNTIME_BYTES 0x4000000L
//#define MAX_RUNTIME_BYTES 0xF000000L



#define STACK_CHUNK_SIZE 8192

static int JSaddGlobalECMANativeProperty(int num, const char *name);
static int JSaddGlobalAssignProperty(int num, const char *name, const char *str);

/*
 * Global JS variables (from Brendan Eichs short embedding tutorial):
 *
 * JSRuntime       - 1 runtime per process
 * JSContext       - 1 CONTEXT per thread
 * global JSObject - 1 global object per CONTEXT
 *
 * struct JSClass {
 *     char *name;
 *     uint32 flags;
 * Mandatory non-null function pointer members:
 *     JSPropertyOp addProperty;
 *     JSPropertyOp delProperty;
 *     JSPropertyOp getProperty;
 *     JSPropertyOp setProperty;
 *     JSEnumerateOp enumerate;
 *     JSResolveOp resolve;
 *     JSConvertOp convert;
 *     JSFinalizeOp finalize;
 * Optionally non-null members start here:
 *     JSGetObjectOps getObjectOps;
 *     JSCheckAccessOp checkAccess;
 *     JSNative call;
 *     JSNative construct;
 *     JSXDRObjectOp xdrObject;
 *     JSHasInstanceOp hasInstance;
 *     prword spare[2];
 * };
 *
 * global JSClass  - populated by stubs
 *
 */

static char *DefaultScriptMethods = "function initialize() {}; " \
			" function shutdown() {}; " \
			" function eventsProcessed() {}; " \
			" TRUE=true; FALSE=false; " \
			" function print(x) {Browser.print(x)}; " \
			" function println(x) {Browser.println(x)}; " \
			" function getName() {return Browser.getName()}; "\
			" function getVersion() {return Browser.getVersion()}; "\
			" function getCurrentSpeed() {return Browser.getCurrentSpeed()}; "\
			" function getCurrentFrameRate() {return Browser.getCurrentFrameRate()}; "\
			" function getWorldURL() {return Browser.getWorldURL()}; "\
			" function replaceWorld(x) {Browser.replaceWorld(x)}; "\
			" function loadURL(x,y) {Browser.loadURL(x,y)}; "\
			" function setDescription(x) {Browser.setDescription(x)}; "\
			" function createVrmlFromString(x) {Browser.createVrmlFromString(x)}; "\
			" function createVrmlFromURL(x,y,z) {Browser.createVrmlFromURL(x,y,z)}; "\
			" function createX3DFromString(x) {Browser.createX3DFromString(x)}; "\
			" function createX3DFromURL(x,y,z) {Browser.createX3DFromURL(x,y,z)}; "\
			" function addRoute(a,b,c,d) {Browser.addRoute(a,b,c,d)}; "\
			" function deleteRoute(a,b,c,d) {Browser.deleteRoute(a,b,c,d)}; "\
			" function _rename_function(obj,oldf,newf) {if(typeof obj[oldf] === 'function') {obj[newf]=obj[oldf]; delete obj[oldf];}}; "\
			"";

			//" function _rename_function(obj,oldf,newf) {obj[newf]=obj[oldf]; delete obj[oldf]}; "

/* housekeeping routines */
int sm_jsIsRunning(){
	ppJScript p = (ppJScript)gglobal()->JScript.prv;
	return p->runtime ? 1 : 0;
}
void sm_JSDeleteScriptContext(int num){
	struct CRscriptStruct *ScriptControl;
	//ppJScript p = (ppJScript)gglobal()->JScript.prv;
	/* printf ("kill_javascript, context is %p\n",ScriptControl[i].cx); */
	ScriptControl = getScriptControlIndex(num);

	if (ScriptControl->eventsProcessed != NULL) {
		JS_RemoveObjectRoot((JSContext *)ScriptControl->cx,(JSObject **)(&ScriptControl->eventsProcessed));
	}

	//JS_DestroyContext(ScriptControl->cx);

}
void sm_jsShutdown(){
	ttglobal tg = gglobal();
	ppJScript p = (ppJScript)tg->JScript.prv;
	if(p->runtime)
		JS_DestroyRuntime(p->runtime);
	p->runtime = NULL;
}
//========================


static struct keyname {
	int key;
	const char *name;
} gcparamname [] = {
{JSGC_MAX_BYTES,"JSGC_MAX_BYTES"},
{JSGC_MAX_MALLOC_BYTES, "JSGC_MAX_MALLOC_BYTES"},
{JSGC_BYTES,"JSGC_BYTES"},
{JSGC_NUMBER,"JSGC_NUMBER"},
{JSGC_MAX_CODE_CACHE_BYTES,"JSGC_MAX_CODE_CACHE_BYTES"},
{JSGC_MODE,"JSGC_MODE"},
{JSGC_UNUSED_CHUNKS,"JSGC_UNUSED_CHUNKS"},
{JSGC_TOTAL_CHUNKS,"JSGC_TOTAL_CHUNKS"},
{JSGC_SLICE_TIME_BUDGET,"JSGC_SLICE_TIME_BUDGET"},
{JSGC_MARK_STACK_LIMIT,"JSGC_MARK_STACK_LIMIT"},
{JSGC_HIGH_FREQUENCY_TIME_LIMIT,"JSGC_HIGH_FREQUENCY_TIME_LIMIT"},
{JSGC_HIGH_FREQUENCY_LOW_LIMIT,"JSGC_HIGH_FREQUENCY_LOW_LIMIT"},
{JSGC_HIGH_FREQUENCY_HIGH_LIMIT,"JSGC_HIGH_FREQUENCY_HIGH_LIMIT"},
{JSGC_HIGH_FREQUENCY_HEAP_GROWTH_MAX,"JSGC_HIGH_FREQUENCY_HEAP_GROWTH_MAX"},
{JSGC_HIGH_FREQUENCY_HEAP_GROWTH_MIN,"JSGC_HIGH_FREQUENCY_HEAP_GROWTH_MIN"},
{JSGC_LOW_FREQUENCY_HEAP_GROWTH,"JSGC_LOW_FREQUENCY_HEAP_GROWTH"},
{JSGC_DYNAMIC_HEAP_GROWTH,"JSGC_DYNAMIC_HEAP_GROWTH"},
{JSGC_DYNAMIC_MARK_SLICE,"JSGC_DYNAMIC_MARK_SLICE"},
{JSGC_ANALYSIS_PURGE_TRIGGER,"JSGC_ANALYSIS_PURGE_TRIGGER"},
{-1,NULL},
};

const char *getgcparamname(int key){
	int i = 0;
	while(gcparamname[i].name != NULL){
		if(gcparamname[i].key == key){
			return gcparamname[i].name;
		}
		i++;
	}
	return "NULL";
}
//void reportError(JSContext *cx, const char *message, JSErrorReport *report) {
//     printf("%s:%u:%s\n",
//             report->filename ? report->filename : "[no filename]",
//             (unsigned int) report->lineno,
//             message);
//}



/* create the script context for this script. This is called from the thread
   that handles script calling in the fwl_RenderSceneUpdateScene 
   July 3, 2018
   x just changing to cpp for moz24 (from moz185/17) doesn't work. the moz dll bombs/throws.
   * but following the formula here works - it runs a little script:
   	https://developer.mozilla.org/en-US/docs/Mozilla/Projects/SpiderMonkey/How_to_embed_the_JavaScript_engine
	so a lot of our code needs grooming for these weird stack techniques.
   
   */
void sm_JSCreateScriptContext(int num) {
	jsval rval;
	JSContext *_context; 	/* these are set here */
	JSObject *_globalObj; 	/* these are set here */
	BrowserNative *br; 	/* these are set here */
	ppJScript p = (ppJScript)gglobal()->JScript.prv;
	struct CRscriptStruct *ScriptControl;

	ScriptControl = getScriptControlIndex(num);
	/* is this the first time through? */
	if (p->runtime == NULL) {
		//p->runtime = JS_NewRuntime(MAX_RUNTIME_BYTES, JSUseHelperThreads::JS_USE_HELPER_THREADS);
		p->runtime = JS_NewRuntime(MAX_RUNTIME_BYTES, JS_USE_HELPER_THREADS); //JSUseHelperThreads::JS_NO_HELPER_THREADS);
		if (!p->runtime) freewrlDie("JS_NewRuntime failed");
	}


	_context = JS_NewContext(p->runtime, STACK_CHUNK_SIZE);
	if (!_context) freewrlDie("JS_NewContext failed");
	//JS_SetErrorReporter(_context, reportError);

	JSContext *cx = _context;
	JS_SetContextPrivate(cx, ScriptControl->script->ShaderScriptNode->_executionContext); //Q. will it be helpful in any X3DScene (aka vrml context) functions?
	static int once = 0;
	if (!once) {
		js_run_version = (long)JS_GetVersion(cx);
		once = 1;
	}

	{ //scope A
		JSAutoRequest ar(cx); // In practice, you would want to exit this any
							// time you're spinning the event loop

		JS::RootedObject global(cx, JS_NewGlobalObject(cx, &p->globalClass, nullptr));
		if (!global)
			return ;

		JS::RootedValue rval(cx);

		{ // Scope B for JSAutoCompartment
			JSAutoCompartment ac(cx, global);
			JS_InitStandardClasses(cx, global);

			br = (BrowserNative *) JS_malloc(_context, sizeof(BrowserNative));
			/* for this script, here are the necessary data areas */
			_globalObj = global;
			ScriptControl->cx =  _context;
			ScriptControl->glob =  _globalObj;
			if(SM_method()==2){
				//JS_SetPrivateFw(_context,_globalObj,ScriptControl->script); //in get/setECMAtype we need our C script struct
				JS_SetPrivate((JSObject*)ScriptControl->glob, ScriptControl->script);
			}

			if (!loadVrmlClasses(_context, _globalObj)){
				freewrlDie("loadVrmlClasses failed");
			}
			if (!VrmlBrowserInit(_context, _globalObj, br)){
				freewrlDie("VrmlBrowserInit failed");
			}
			if (!ActualrunScript(num,DefaultScriptMethods,rval.address()))
				cleanupDie(num,"runScript failed in VRML::newJS DefaultScriptMethods");

			if(0) {
				//baby step test
				const char *script = "'hello'+'world, it is '+new Date()";
				const char *filename = "noname";
				int lineno = 1;
				bool ok = JS_EvaluateScript(cx, global, script, strlen(script), filename, lineno, rval.address());
				if (!ok)
					return ;
				JSString *str = rval.toString();
				printf("%s\n", JS_EncodeString(cx, str));
			}
		} //scope B

	} //scope A

	/* send this data over to the routing table functions. */
	CRoutes_js_new (num, JAVASCRIPT);

}
} //extern "C"

int SM_method(){
	return getJsEngineVariant() == 2? 2 : 0;
	//return 2; //new way dec 31, 2017
	//return 0; //old way before dec 31, 2017
}

/* run the script from within C */
int ActualrunScript(int num, char *script, jsval *rval) {

	int len;
	JSContext *cx;
	JSObject *global;
	struct CRscriptStruct *ScriptControl;

	ScriptControl = getScriptControlIndex(num);
	/* get context and global object for this script */
	cx = (JSContext*)ScriptControl->cx;
	global = (JSObject*)ScriptControl->glob;

	len = (int) strlen(script);

	{ // Scope A for our various stack objects (JSAutoRequest, RootedObject), so they all go
		// out of scope before we JS_DestroyContext.
		JSAutoRequest ar(cx); // In practice, you would want to exit this any
							// time you're spinning the event loop
		{ // Scope B for JSAutoCompartment
			JSAutoCompartment ac(cx, global);
			bool ok = JS_EvaluateScript(cx, global, script, len, FNAME_STUB, LINENO_STUB, rval);
			if (!ok) {
				printf ("ActualrunScript - JS_EvaluateScript failed for %s", script);
				printf ("\n");
				ConsoleMessage ("ActualrunScript - JS_EvaluateScript failed for %s", script);
				return JS_FALSE;
			}
		} //Scope B
	} //Scope A

	return JS_TRUE;
}

/* run the script from within Javascript  */
int jsrrunScript(JSContext *cx, JSObject *global, char *script, jsval *rval) {
	int len;

	#ifdef JAVASCRIPTVERBOSE
		printf("jsrrunScript script cx %p \"%s\", \n",
			   _context, script);
	#endif
	len = (int) strlen(script);
	{ // Scope A  for our various stack objects (JSAutoRequest, RootedObject), so they all go
		// out of scope before we JS_DestroyContext.
		JSAutoRequest ar(cx); // In practice, you would want to exit this any
							// time you're spinning the event loop
		{ // Scope B  for JSAutoCompartment
			JSAutoCompartment ac(cx, global);
			if (!JS_EvaluateScript(cx, global, script, len,
								   FNAME_STUB, LINENO_STUB, rval)) {
				ConsoleMessage ("jsrunScript - JS_EvaluateScript failed for %s", script);
				return JS_FALSE;
			}
		} //Scope B
	} //Scope A
	return JS_TRUE;
}


extern "C" {

int sm_jsActualrunScript(int num, char *script) {
	jsval rval; //discard
	return ACTUALRUNSCRIPT(num,script,&rval);
}




//put source = NULL if you want it to malloc
//if its a script field, send in the pointer to the script field valuechanged, or NULL if new thing()
int sizeofSForMF(int itype);
void *AnyNativeNew(int type, union anyVrml* source, int *valueChanged){
	AnyNative *ptr;
	ptr = MALLOC(AnyNative *,sizeof(AnyNative));
	memset(ptr,0,sizeof(AnyNative));
	ptr->type = type;
	ptr->valueChanged = valueChanged;
	ptr->v = source;
	ptr->gc = 0;
	if(ptr->v == NULL){
		ptr->v = (union anyVrml *)MALLOC(void *,sizeofSForMF(type));
		memset(ptr->v,0,sizeofSForMF(type));
		ptr->gc = 1;
	}
	return ptr;
}
void shallow_copy_field(int typeIndex, union anyVrml* source, union anyVrml* dest);
void AnyNativeAssign(void *top, void *fromp)
{
	if(top != fromp){
		AnyNative *to = (AnyNative *)top;
		AnyNative *from = (AnyNative *)fromp;
		if(to->type == from->type){
			if(to->valueChanged)
				(*to->valueChanged) ++;
			//shallow assumes the top has already been malloced (just base part of MF needed)
			//use this if you need to malloc anyvrml: int sizeofSForMF(int itype)
			shallow_copy_field(from->type,from->v,to->v);
		}
	}
}
/* FROM VRMLC.pm */
void *SFNodeNativeNew()
{
	SFNodeNative *ptr;
	ptr = MALLOC(SFNodeNative *, sizeof(*ptr));

	/* printf ("SFNodeNativeNew; string len %d handle_len %d\n",vrmlstring_len,handle_len);*/

	ptr->handle = 0;
	ptr->valueChanged = 0;
	if(SM_method() == 0){
		ptr->X3DString = NULL;
		ptr->fieldsExpanded = FALSE;
	}
	return ptr;
}

/* assign this internally to the Javascript engine environment */
int SFNodeNativeAssign(void *top, void *fromp)
{
	SFNodeNative *to = (SFNodeNative *)top;
	SFNodeNative *from = (SFNodeNative *)fromp;

	/* indicate that this was touched; and copy contents over */
	to->valueChanged++;

	if (from != NULL) {
		to->handle = from->handle;
		if(SM_method() == 0)
			to->X3DString = STRDUP(from->X3DString);

		#ifdef JAVASCRIPTVERBOSE
		printf ("SFNodeNativeAssign, copied %p to %p, handle %p, string %s\n", from, to, to->handle, to->X3DString);
		#endif
	} else {
		to->handle = 0;
		if(SM_method() == 0)
			to->X3DString = STRDUP("from a NULL assignment");
	}

	return JS_TRUE;
}
/* assign this internally to the Javascript engine environment */
int SFNodeNativeEquals(void *top, void *fromp)
{
	int equal;
	SFNodeNative *to = (SFNodeNative *)top;
	SFNodeNative *from = (SFNodeNative *)fromp;

	equal = 0;

	if (from != NULL) {
		if(to->handle == from->handle)
			equal=1;
	}

	return equal;// ? JS_TRUE : JS_FALSE;
}

void *SFColorRGBANativeNew()
{
	SFColorRGBANative *ptr;
	ptr = MALLOC(SFColorRGBANative *, sizeof(*ptr));
	ptr->valueChanged = 0;
	return ptr;
}

void SFColorRGBANativeAssign(void *top, void *fromp)
{
	SFColorRGBANative *to = (SFColorRGBANative *)top;
	SFColorRGBANative *from = (SFColorRGBANative *)fromp;
	to->valueChanged ++;
	(to->v) = (from->v);
}

void *SFColorNativeNew()
{
	SFColorNative *ptr;
	ptr = MALLOC(SFColorNative *, sizeof(*ptr));
	ptr->valueChanged = 0;
	return ptr;
}

void SFColorNativeAssign(void *top, void *fromp)
{
	SFColorNative *to = (SFColorNative *)top;
	SFColorNative *from = (SFColorNative *)fromp;
	to->valueChanged++;
	(to->v) = (from->v);
}

void *SFImageNativeNew()
{
	SFImageNative *ptr;
	ptr =MALLOC(SFImageNative *, sizeof(*ptr));
	ptr->valueChanged = 0;
	return ptr;
}

void SFImageNativeAssign(void *top, void *fromp)
{
	SFImageNative *to = (SFImageNative *)top;
	/* SFImageNative *from = fromp; */
	UNUSED(fromp);

	to->valueChanged++;
/* 	(to->v) = (from->v); */
}

void *SFRotationNativeNew()
{
	SFRotationNative *ptr;
	ptr = MALLOC(SFRotationNative *, sizeof(*ptr));
	ptr->valueChanged = 0;
	return ptr;
}

void SFRotationNativeAssign(void *top, void *fromp)
{
	SFRotationNative *to = (SFRotationNative *)top;
	SFRotationNative *from = (SFRotationNative *)fromp;
	to->valueChanged++;
	(to->v) = (from->v);
}

void *SFVec2fNativeNew()
{
	SFVec2fNative *ptr;
	ptr = MALLOC(SFVec2fNative *, sizeof(*ptr));
	ptr->valueChanged = 0;
	return ptr;
}

void SFVec2fNativeAssign(void *top, void *fromp)
{
	SFVec2fNative *to = (SFVec2fNative *)top;
	SFVec2fNative *from = (SFVec2fNative *)fromp;
	to->valueChanged++;
	(to->v) = (from->v);
}

void *SFVec3fNativeNew() {
	SFVec3fNative *ptr;
	ptr = MALLOC(SFVec3fNative *, sizeof(*ptr));
	ptr->valueChanged = 0;
	return ptr;
}

void SFVec3fNativeAssign(void *top, void *fromp) {
	SFVec3fNative *to = (SFVec3fNative *)top;
	SFVec3fNative *from = (SFVec3fNative *)fromp;
	to->valueChanged++;
	(to->v) = (from->v);
}

void *SFVec3dNativeNew() {
	SFVec3dNative *ptr;
	ptr = MALLOC(SFVec3dNative *, sizeof(*ptr));
	ptr->valueChanged = 0;
	return ptr;
}

void SFVec3dNativeAssign(void *top, void *fromp) {
	SFVec3dNative *to = (SFVec3dNative *)top;
	SFVec3dNative *from = (SFVec3dNative *)fromp;
	to->valueChanged++;
	(to->v) = (from->v);
}

void *SFVec4fNativeNew() {
	SFVec4fNative *ptr;
	ptr = MALLOC(SFVec4fNative *, sizeof(*ptr));
	ptr->valueChanged = 0;
	return ptr;
}

void SFVec4fNativeAssign(void *top, void *fromp) {
	SFVec4fNative *to = (SFVec4fNative *)top;
	SFVec4fNative *from = (SFVec4fNative *)fromp;
	to->valueChanged++;
	(to->v) = (from->v);
}

void *SFVec4dNativeNew() {
	SFVec4dNative *ptr;
	ptr = MALLOC(SFVec4dNative *, sizeof(*ptr));
	ptr->valueChanged = 0;
	return ptr;
}

void SFVec4dNativeAssign(void *top, void *fromp) {
	SFVec4dNative *to = (SFVec4dNative *)top;
	SFVec4dNative *from = (SFVec4dNative *)fromp;
	to->valueChanged++;
	(to->v) = (from->v);
}

} //extern "C"


static char* re_strcat(char *_Dest, const char *_Source, int *destLen, int *destDim)
{
	/* strcats, but first checks strlen on source and destination
	   and reallocs if necessary - good when you are doing a lot of strcatting of un-pre-known elements
	   (Q. is there something for this already?)
	   _Dest, _Source - as with strcat(_Dest,_Source)
	   destLen - current cumulative strlen(_Dest)
	   destDim - current malloc/realloc dimension
	   Usage example:
		dstdim = (rows+1)*(elements*15) + 100; //a guess
		dstlen = 0;
		smallfield = MALLOC (char *, dstdim+1); //rows+1)*(elements*15) + 100);
		smallfield[0] = '\0';
		...
		for(;;)
		{
			...
			smallfield = re_strcat(smallfield, "new ",&dstlen,&dstdim);
		...
		FREE_IF_NZ(smallfield)
	*/
	int srclen = (int) strlen(_Source);
	*destLen = *destLen + srclen;
	if(*destLen > *destDim -1)
	{
		*destDim = *destDim + srclen + 1 + 100;
		_Dest = (char *)REALLOC(_Dest,*destDim);
	}
	_Dest = strcat(_Dest,_Source);
	return _Dest;
}
/* the fwl_RenderSceneUpdateScene is initializing this field now */
/* A new version of InitScriptField which takes "nicer" arguments; currently a
 * simple and restricted wrapper, but it could replace it soon? */
/* Parameters:
	num:		Script number. Starts at 0.
	kind:		One of PKW_initializeOnly PKW_outputOnly PKW_inputOutput PKW_inputOnly
	type:		One of the FIELDTYPE_ defines, eg, FIELDTYPE_MFFloat
	field:		the field name as found in the VRML/X3D file. eg "set_myField"

*/
void InitScriptField(int num, indexT kind, indexT type, const char* field, union anyVrml value) {
	jsval rval;
	char *smallfield = NULL;
	char mynewname[400];
	char *thisValue;
	int rows, elements;
	char *sftype = NULL;

	int haveMulti;
	int MFhasECMAtype;
	int rowCount, eleCount;

	int tlen;
	float *FloatPtr;
	struct X3D_Node **VoidPtr;
	int *IntPtr;
	double *DoublePtr;
	struct Uni_String **SVPtr;

	float defaultFloat[] = {0.0f,0.0f,0.0f,0.0f};
	int defaultInt[] = {0,0,0,0};
	double defaultDouble[] = {0.0, 0.0, 0.0, 0.0};
	struct Uni_String *sptr[1];
	struct X3D_Node *defaultVoid[] = {NULL,NULL};
	struct CRscriptStruct *ScriptControl; //= getScriptControl();

	#ifdef JAVASCRIPTVERBOSE
	printf ("calling InitScriptField from thread %u\n",pthread_self());
	printf ("\nInitScriptField, num %d, kind %s type %s field %s value %d\n", num,PROTOKEYWORDS[kind],FIELDTYPES[type],field,value);
	#endif

    if ((kind != PKW_inputOnly) && (kind != PKW_outputOnly) && (kind != PKW_initializeOnly) && (kind != PKW_inputOutput)) {
            ConsoleMessage ("InitScriptField: invalid kind for script: %d\n",kind);
            return;
    }

    if (type >= FIELDTYPES_COUNT) {
            ConsoleMessage ("InitScriptField: invalid type for script: %d\n",type);
            return;
    }
	ScriptControl = getScriptControlIndex(num);

	/* first, make a new name up */
	//if (kind == PKW_inputOnly ) {
	//	sprintf (mynewname,"__eventIn_Value_%s",field);
	//	//strcpy(mynewname,field);
	//}else 
	if (kind == PKW_inputOutput) {
		//check if user added an eventIn function with the same basename,
		// which is allowed with inputOutput fields
		JSContext *cx;
		JSObject *obj;
		jsval retval;
		cx =  (JSContext*)ScriptControl->cx;
		obj = (JSObject*)ScriptControl->glob;

		if (JS_GetProperty(cx,obj,field,&retval)){
			//if (JSVAL_IS_OBJECT(retval)){
			if(retval.isObject()){
				//I think functions are objects, doesn't seem to be a JSVAL_IS_FUNC
				char runstring[STRING_SIZE];
				// rename fieldname to set_fieldname
				sprintf(runstring,"_rename_function(this,'%s','set_%s');",field,field);
				//printf("%s\n",runstring);
				{ // Scope A  for our various stack objects (JSAutoRequest, RootedObject), so they all go
					// out of scope before we JS_DestroyContext.
					JSAutoRequest ar(cx); // In practice, you would want to exit this any
										// time you're spinning the event loop
					{ // Scope B  for JSAutoCompartment
						JSAutoCompartment ac(cx, obj);
						if(!JS_EvaluateScript(cx,obj, runstring, (int) strlen(runstring), FNAME_STUB, LINENO_STUB, &retval)){
							printf("sorry couldn't rename function: %s",runstring);
						}
					} //Scope B
				} //Scope A
			}
		}
		strcpy(mynewname,field); //now this is OK, won't overwrite function
		// and if so rename it to set_
	}else strcpy(mynewname,field);

	/* ok, lets handle the types here */
	switch (type) {
		/* ECMA types */
		case FIELDTYPE_SFBool:
		case FIELDTYPE_SFFloat:
		case FIELDTYPE_SFTime:
		case FIELDTYPE_SFDouble:
		case FIELDTYPE_SFInt32:
		case FIELDTYPE_SFString: {
			/* do not care about eventIns */
			if (kind != PKW_inputOnly)  {
				JSaddGlobalECMANativeProperty(num, field);
				if (kind == PKW_initializeOnly || kind == PKW_inputOutput) {
					if  (type == FIELDTYPE_SFString) {
						tlen = (int) strlen(value.sfstring->strptr) + strlen(field) + 20;
					} else {
						tlen = (int) strlen(field) + 400; /* long, in the case of doubles */
					}
					smallfield = MALLOC (char *, tlen);
					smallfield[0] = '\0';

					switch (type) {
						case FIELDTYPE_SFFloat: sprintf (smallfield,"%s=%f\n",field,value.sffloat);break;
						case FIELDTYPE_SFTime: sprintf (smallfield,"%s=%f\n",field,value.sftime);break;
						case FIELDTYPE_SFDouble: sprintf (smallfield,"%s=%f\n",field,value.sftime);break;
						case FIELDTYPE_SFInt32: sprintf (smallfield,"%s=%d\n",field,value.sfint32); break;
						case FIELDTYPE_SFBool:
							if (value.sfbool == 1) sprintf (smallfield,"%s=true",field);
							else sprintf (smallfield,"%s=false",field);
							break;
						case FIELDTYPE_SFString:
							sprintf (smallfield,"%s=\"%s\"\n",field,value.sfstring->strptr); break;
					}

					if (!ACTUALRUNSCRIPT(num,smallfield,&rval))
						printf ("huh??? Field initialization script failed %s\n",smallfield);
				}
			}
			break;
		}
		/* non ECMA types */
		default: {
			/* get an appropriate pointer - we either point to the initialization value
			   in the script header, or we point to some data here that are default values */

			/* does this MF type have an ECMA type as a single element? */
			switch (type) {
				case FIELDTYPE_MFString:
				case FIELDTYPE_MFTime:
				case FIELDTYPE_MFBool:
				case FIELDTYPE_MFInt32:
				case FIELDTYPE_MFFloat:
				JSaddGlobalECMANativeProperty(num, field);
					MFhasECMAtype = TRUE;
					break;
				default: {
					MFhasECMAtype = FALSE;
				}
			}

			elements=0;
			IntPtr = NULL;
			FloatPtr = NULL;
			DoublePtr = NULL;
			SVPtr = NULL;
			VoidPtr = NULL;
			if (kind == PKW_initializeOnly || kind == PKW_inputOutput) {
				switch (type) {
					//case FIELDTYPE_SFImage:
					//	VoidPtr = (struct X3D_Node **) (&(value.sfimage)); elements = 1;
					//	break;
					case FIELDTYPE_SFNode:
						VoidPtr = (struct X3D_Node **) (&(value.sfnode)); elements = 1;
						break;
					case FIELDTYPE_MFColor:
						FloatPtr = (float *) value.mfcolor.p; elements = value.mfcolor.n;
						break;
					case FIELDTYPE_MFColorRGBA:
						FloatPtr = (float *) value.mfcolorrgba.p; elements = value.mfcolorrgba.n;
						break;
					case FIELDTYPE_MFVec2f:
						FloatPtr = (float *) value.mfvec2f.p; elements = value.mfvec2f.n;
						break;
					case FIELDTYPE_MFVec3f:
						FloatPtr = (float *) value.mfvec3f.p; elements = value.mfvec3f.n;
						break;
					case FIELDTYPE_MFRotation:
						FloatPtr = (float *) value.mfrotation.p; elements = value.mfrotation.n;
						break;
					case FIELDTYPE_SFVec2f:
						FloatPtr = (float *) value.sfvec2f.c; elements = 1;
						break;
					case FIELDTYPE_SFColor:
						FloatPtr = value.sfcolor.c; elements = 1;
						break;
					case FIELDTYPE_SFColorRGBA:
						FloatPtr = value.sfcolorrgba.c; elements = 1;
						break;
					case FIELDTYPE_SFRotation:
						FloatPtr = value.sfrotation.c; elements = 1;
						break;
					case FIELDTYPE_SFVec3f:
						FloatPtr = value.sfvec3f.c; elements =1;
						break;
					case FIELDTYPE_SFVec3d:
						DoublePtr = value.sfvec3d.c; elements =1;
						break;
					case FIELDTYPE_MFString:
						SVPtr = value.mfstring.p; elements = value.mfstring.n;
						break;
					case FIELDTYPE_MFTime:
						DoublePtr = value.mftime.p; elements = value.mftime.n;
						break;
					case FIELDTYPE_MFBool:
						IntPtr = value.mfbool.p; elements = value.mfbool.n;
						break;
					case FIELDTYPE_SFImage:
					case FIELDTYPE_MFInt32:
						IntPtr = value.mfint32.p; elements = value.mfint32.n;
						break;
					case FIELDTYPE_MFNode:
						VoidPtr = (struct X3D_Node **)(value.mfnode.p); elements = value.mfnode.n;
						break;
					case FIELDTYPE_MFFloat:
						FloatPtr = value.mffloat.p; elements = value.mffloat.n;
						break;
					case FIELDTYPE_SFVec4f:
						FloatPtr = value.sfvec4f.c; elements = 1;
						break;
					case FIELDTYPE_SFVec4d:
						DoublePtr = value.sfvec4d.c; elements = 1;
						break;

					default: {
						printf ("unhandled type, in InitScriptField %d\n",type);
						return;
					}
				}

			} else {
				/* make up a default pointer */
				elements = 1;
				switch (type) {
					/* Void types */
					case FIELDTYPE_SFNode:
					case FIELDTYPE_MFNode:
						VoidPtr = (struct X3D_Node **) &defaultVoid;
						break;

					/* Float types */
					case FIELDTYPE_MFColor:
					case FIELDTYPE_MFColorRGBA:
					case FIELDTYPE_MFVec2f:
					case FIELDTYPE_MFVec3f:
					case FIELDTYPE_MFRotation:
					case FIELDTYPE_SFVec2f:
					case FIELDTYPE_SFColor:
					case FIELDTYPE_SFColorRGBA:
					case FIELDTYPE_SFRotation:
					case FIELDTYPE_SFVec3f:
					case FIELDTYPE_SFVec4f:
					case FIELDTYPE_MFFloat:
						FloatPtr = defaultFloat;
						break;

					/* Int types */
					case FIELDTYPE_MFBool:
					case FIELDTYPE_MFInt32:
						IntPtr = defaultInt;
						break;

					/* String types */
					case FIELDTYPE_SFString:
					case FIELDTYPE_MFString:
						sptr[0] = newASCIIString("");
						SVPtr = sptr;
						break;

					/* SFImage */
					case FIELDTYPE_SFImage:
						IntPtr = defaultInt;
						break;

					/* Double types */
					case FIELDTYPE_SFVec2d:
					case FIELDTYPE_SFVec3d:
					case FIELDTYPE_MFTime:
					case FIELDTYPE_SFTime:
					case FIELDTYPE_SFDouble:
					case FIELDTYPE_SFVec4d:
						DoublePtr = defaultDouble;
						break;

					default: {
						printf ("unhandled type, in InitScriptField part 2 %d\n",type);
						return;
					}
				}

			}

			rows = returnElementRowSize (type);

			#ifdef JAVASCRIPTVERBOSE
			printf ("in fieldSet, we have ElementRowSize %d and individual elements %d\n",rows,elements);
			#endif

			/* make this at least as large as required, then add some more on to the end... */
			/*
			Old Approach
					step1: compute using guestimate formulas
					step2: malloc
					step3: loop through strcat() and hope no overrun
				Problem: heap corruption from array overrun - the guestimate has been bad
				    a few times in 2010 with MFVec2fs and MFStrings with 42 and 47 elements, strings of varying length
				example for MFVec2f
				'new MFVec2f(new SFVec2f(1234.678910,1234.678910),...)'
				each SF 2 numbers each 10 digits plus new type(,), 15 chars  =35.
				3 x 15 = 45 (or (rows+1)x(elements*15)+100)
				old formula falls short:
					old formula: smallfield = MALLOC (rows*((elements*15) + 100));
					example 47 SFVec2fs
					actual bytes: 47 x 35 bytes = 1645 + 13 for the MF = 1658
					old formula  2 x ((47*15)+100) = 1610   //thats 48 bytes short and I bomb out
					better formula  3 x (47*15) + 100 = 2215
			New Approach (July 28, 2010)
					step1: compute using guestimate formulas
					step2: malloc
					step3: loop through and realloc before strcat() if short
			*/
			{
				int dstlen, dstdim, tdim;
				tdim = 200;
				thisValue = MALLOC(char *, tdim+1);
				dstdim = (rows+1)*(elements*15) + 100; /* a guess */
				dstlen = 0;
				smallfield = MALLOC (char *, dstdim+1); //rows+1)*(elements*15) + 100);
				/* what is the equivalent SF for this MF?? */
				if (type != convertToSFType(type)) haveMulti = TRUE;
				 else haveMulti = FALSE;

				/* the sftype is the SF form of either the MF or SF */
				sftype = STRDUP((char *)FIELDTYPES[convertToSFType(type)]);

				/* SFStrings are Strings */
				if (strncmp(sftype,"SFString",8)==0) strcpy (sftype,"String");


				/* start the string */
				smallfield[0] = '\0';

				/* is this an MF variable, with SFs in it? */
				if (haveMulti) {
					smallfield = re_strcat(smallfield, "new ",&dstlen,&dstdim);
					smallfield = re_strcat(smallfield, (char *)FIELDTYPES[type],&dstlen,&dstdim);
					smallfield = re_strcat(smallfield, "(",&dstlen,&dstdim);
				}

				/* loop through, and put values in */
				for (eleCount=0; eleCount<elements; eleCount++) {
					/* ECMA native types can just be passed in... */
					if (!MFhasECMAtype) {
						smallfield = re_strcat(smallfield, "new ",&dstlen,&dstdim);
						smallfield = re_strcat(smallfield, sftype,&dstlen,&dstdim);
						smallfield = re_strcat(smallfield, "(",&dstlen,&dstdim);
					}

					/* go through the SF type; SFints will have 1; SFVec3f's will have 3, etc */
					for (rowCount=0; rowCount<rows; rowCount++ ) {
						if (IntPtr != NULL) {
							sprintf (thisValue,"%d",*IntPtr); IntPtr++;
						} else if (FloatPtr != NULL) {
							sprintf (thisValue,"%f",*FloatPtr); FloatPtr++;
						} else if (DoublePtr != NULL) {
							sprintf (thisValue,"%f",*DoublePtr); DoublePtr++;
						} else if (SVPtr != NULL) {
							sptr[0] = *SVPtr; SVPtr++;
							if((int)strlen(sptr[0]->strptr)+2 > tdim-1)
							{
								tdim = (int) strlen(sptr[0]->strptr) + 1 + 100;
								thisValue = (char *)REALLOC(thisValue,tdim);
							}
							sprintf (thisValue,"\"%s\"",sptr[0]->strptr);
						} else { /* must be a Void */
							/* printf ("sending in a VoidPtr, it is %p\n",VoidPtr[0]);
							if (VoidPtr[0] != NULL) {printf ("it is a %s type\n",stringNodeType(X3D_NODE(VoidPtr[0])->_nodeType));} */
							sprintf (thisValue,"\"%p\"", VoidPtr[0]); VoidPtr++;
						}
						smallfield = re_strcat(smallfield, thisValue,&dstlen,&dstdim);
						if (rowCount < (rows-1)) smallfield = re_strcat(smallfield,",",&dstlen,&dstdim);
					}

					if (!MFhasECMAtype) smallfield = re_strcat(smallfield, ")",&dstlen,&dstdim);
					if (eleCount < (elements-1)) smallfield = re_strcat(smallfield,",",&dstlen,&dstdim);

				}


				if (haveMulti) {
					smallfield = re_strcat(smallfield,")",&dstlen,&dstdim);
				}
				/* printf("dstlen=%d dstdim=%d\n",dstlen,dstdim); */
				FREE_IF_NZ (thisValue);
			}
			/* Warp factor 5, Dr Sulu... */
			#ifdef JAVASCRIPTVERBOSE
			printf ("JScript, for non-ECMA newname %s, sending :%s:\n",mynewname,smallfield);
			#endif

			JSaddGlobalAssignProperty (num,mynewname,smallfield);
		}
	}

	/* Fields can generate an event, so we allow the touched flag to remain set. eventOuts have just
	   been initialized, and as such, should not send events, until after they really have been set.
	*/
	if (kind == PKW_outputOnly) {
		int fptr;
		int touched;

		UNUSED(touched); // compiler warning mitigation

		/* get the number representing this type */
		fptr = JSparamIndex (field, FIELDTYPES[type],kind);

		/* set up global variables so that we can reset the touched flag */
		touched = get_valueChanged_flag (fptr, num);

		/* and, reset the touched flag, knowing that we have the variables set properly */
		resetScriptTouchedFlag(num, fptr);
	}
	ScriptControl = getScriptControlIndex(num);
	js_cleanup_script_context(num);

	FREE_IF_NZ (smallfield);
	FREE_IF_NZ (sftype);

}

void InitScriptFieldB(int num, indexT kind, indexT type, const char* field, union anyVrml value) {
	//Dec 31 2017 this version of initscriptfield for SM treats fields more generically

	struct CRscriptStruct *ScriptControl; //= getScriptControl();
	//char mynewname[256];
	#ifdef JAVASCRIPTVERBOSE
	printf ("calling InitScriptField from thread %u\n",pthread_self());
	printf ("\nInitScriptField, num %d, kind %s type %s field %s value %d\n", num,PROTOKEYWORDS[kind],FIELDTYPES[type],field,value);
	#endif

	if ((kind != PKW_inputOnly) && (kind != PKW_outputOnly) && (kind != PKW_initializeOnly) && (kind != PKW_inputOutput)) {
		ConsoleMessage ("InitScriptField: invalid kind for script: %d\n",kind);
		return;
	}

	ScriptControl = getScriptControlIndex(num);


	// fix eventIn vs field name conflicts. by renameing inputOutput and inputOnly eventIn functions to set_fieldname
	//if (kind == PKW_inputOnly || 
	//strcpy(mynewname,field);
	//if (kind == PKW_inputOnly ) {
	//	sprintf (mynewname,"__eventIn_Value_%s",field);
	//	//strcpy(mynewname,field);
	//} else 
	if( kind == PKW_inputOutput) {
		JSContext *cx;
		JSObject *obj;
		jsval retval;
		cx =  (JSContext*)ScriptControl->cx;
		obj = (JSObject*)ScriptControl->glob;

		if (JS_GetProperty(cx,obj,field,&retval)){
			//if (JSVAL_IS_OBJECT(retval)){
			if(retval.isObject()){
				//I think functions are objects, doesn't seem to be a JSVAL_IS_FUNC
				char runstring[STRING_SIZE];
				// rename fieldname to set_fieldname
				sprintf(runstring,"_rename_function(this,'%s','set_%s');",field,field);
				//printf("%s\n",runstring);
				{ // Scope A  for our various stack objects (JSAutoRequest, RootedObject), so they all go
					// out of scope before we JS_DestroyContext.
					JSAutoRequest ar(cx); // In practice, you would want to exit this any
										// time you're spinning the event loop
					{ // Scope B  for JSAutoCompartment
						JSAutoCompartment ac(cx, obj);
						if(!JS_EvaluateScript(cx,obj, runstring, (int) strlen(runstring), FNAME_STUB, LINENO_STUB, &retval)){
							printf("sorry couldn't rename function: %s",runstring);
						}
					} //Scope B
				} //Scope A
			}
		}
		// and if so rename it to set_
	}

	//unconditionally add the field no matter its type
	//contains getter and setter
	JSaddGlobalECMANativeProperty(num, field);
}



static int JSaddGlobalECMANativeProperty(int num, const char *name) {
	JSContext *cx;
	JSObject *global;
	jsval rval = INT_TO_JSVAL(0);
	struct CRscriptStruct *ScriptControl; // = getScriptControl();

	ScriptControl = getScriptControlIndex(num);
	/* get context and global object for this script */
	cx =  (JSContext*)ScriptControl->cx;
	global = (JSObject*)ScriptControl->glob;


/* Note, for JS-185+, JSPROP_PERMANENT makes properties non-configurable, which can cause runtime
 * errors from the JS engine when said property gets redefined to a function by the script.  The
 * example file tests/Javascript_tests/MFFloat.wrl had this issue. */

	{ // Scope A for our various stack objects (JSAutoRequest, RootedObject), so they all go
		// out of scope before we JS_DestroyContext.

		JSAutoRequest ar(cx); // In practice, you would want to exit this any
							// time you're spinning the event loop
		{ // Scope B for JSAutoCompartment
			JSAutoCompartment ac(cx, global);
			bool ok = JS_DefineProperty(cx,global, name, rval, getECMANative, setECMANative,0);
			if (!ok){
				printf("JS_DefineProperty failed for \"%s\" in addGlobalECMANativeProperty.\n", name);
				return JS_FALSE;
			}
		} // Scope B
	} //Scope A
	return JS_TRUE;
}

static int JSaddGlobalAssignProperty(int num, const char *name, const char *str) {
	jsval _rval = INT_TO_JSVAL(0);
	JSContext *cx;
	JSObject *global;
	struct CRscriptStruct *ScriptControl;

	ScriptControl = getScriptControlIndex(num);

	/* get context and global object for this script */
	cx =  (JSContext*)ScriptControl->cx;
	global = (JSObject*)ScriptControl->glob;

	{ // Scope A  for our various stack objects (JSAutoRequest, RootedObject), so they all go
		// out of scope before we JS_DestroyContext.
		JSAutoRequest ar(cx); // In practice, you would want to exit this any
							// time you're spinning the event loop
		{ // Scope B  for JSAutoCompartment
			JSAutoCompartment ac(cx,global);
			if (!JS_EvaluateScript(cx,global, str, (int) strlen(str), FNAME_STUB, LINENO_STUB, &_rval)) {
				ConsoleMessage ("JSaddGlobalAssignProperty - JS_EvaluateScript failed for %s", str);
				return JS_FALSE;
			}
			if (!JS_DefineProperty(cx,global, name, _rval, getAssignProperty, setAssignProperty, 0 | JSPROP_PERMANENT)) {
				printf("JS_DefineProperty failed for \"%s\" in addGlobalAssignProperty.\n", name);
				return JS_FALSE;
			}
		} //Scope B
	} //Scope A
	return JS_TRUE;
}

/* defines for getting touched flags and exact Javascript pointers */

/****************************** ECMA types ******************************************/
/* where we have a Native structure to go along with it */
#define GETJSPTR_TYPE_A(thistype) \
			 case FIELDTYPE_##thistype:  {  \
				thistype##Native *ptr; \
				/* printf ("getting private data in GETJSPTR for %p \n",JSglobal_return_val); */ \
				if ((ptr = (thistype##Native *)JS_GetPrivateFw(cx, JSVAL_TO_OBJECT(*(jsval *)(tg->JScript.JSglobal_return_val)))) == NULL) { \
					printf( "JS_GetPrivate failed in get_valueChanged_flag\n"); \
					return JS_FALSE; \
				} \
				/* if (ptr->valueChanged > 0) printf ("private is %d valueChanged %d\n",ptr,ptr->valueChanged); */ \
				tg->CRoutes.JSSFpointer = (void *)ptr; /* save this for quick extraction of values */ \
				touched = ptr->valueChanged; \
				break; \
			}

#define RESET_TOUCHED_TYPE_A(thistype) \
			case FIELDTYPE_##thistype: { \
					((thistype##Native *)tg->CRoutes.JSSFpointer)->valueChanged = 0; \
					break; \
			}

#define GETJSPTR_TYPE_MF_A(thisMFtype,thisSFtype) \
	case FIELDTYPE_##thisMFtype: { \
		thisSFtype##Native *ptr; \
		jsval mainElement; \
		int len; \
		int i; \
		if (!JS_GetProperty(cx, JSVAL_TO_OBJECT(*(jsval *)(tg->JScript.JSglobal_return_val)), "length", &mainElement)) { \
			printf ("JS_GetProperty failed for \"length\" in get_valueChanged_flag\n"); \
			return FALSE; \
		} \
		len = JSVAL_TO_INT(mainElement); \
		/* go through each element of the main array. */ \
		for (i = 0; i < len; i++) { \
			if (!JS_GetElement(cx, JSVAL_TO_OBJECT(*(jsval*)(tg->JScript.JSglobal_return_val)), i, &mainElement)) { \
				printf ("JS_GetElement failed for %d in get_valueChanged_flag\n",i); \
				return FALSE; \
			} \
			if ((ptr = (thisSFtype##Native *)JS_GetPrivateFw(cx, JSVAL_TO_OBJECT(mainElement))) == NULL) { \
				printf( "JS_GetPrivate failed for obj in setField_javascriptEventOut.\n"); \
				return FALSE; \
			} \
			if (ptr->valueChanged > 0) touched = TRUE; /* did this element change? */ \
			/* printf ("touched flag for element %d is %d\n",i,ptr->touched); */ \
		} \
		break; \
	}

#define RESET_TOUCHED_TYPE_MF_A(thisMFtype,thisSFtype) \
	case FIELDTYPE_##thisMFtype: { \
		thisSFtype##Native *ptr; \
		jsval mainElement; \
		int len; \
		int i; \
		JSContext *cx; \
		cx = (JSContext*)scriptcontrol->cx; \
		if (!JS_GetProperty(cx, JSVAL_TO_OBJECT(*(jsval*)(tg->JScript.JSglobal_return_val)), "length", &mainElement)) { \
			printf ("JS_GetProperty failed for \"length\" in get_valueChanged_flag\n"); \
			break; \
		} \
		len = JSVAL_TO_INT(mainElement); \
		/* go through each element of the main array. */ \
		for (i = 0; i < len; i++) { \
			if (!JS_GetElement(cx, JSVAL_TO_OBJECT(*(jsval*)(tg->JScript.JSglobal_return_val)), i, &mainElement)) { \
				printf ("JS_GetElement failed for %d in get_valueChanged_flag\n",i); \
				break; \
			} \
			if ((ptr = (thisSFtype##Native *)JS_GetPrivateFw(cx, JSVAL_TO_OBJECT(mainElement))) == NULL) { \
				printf( "JS_GetPrivate failed for obj in setField_javascriptEventOut.\n"); \
				break; \
			} \
			ptr->valueChanged = 0; \
		} \
		break; \
	}

/****************************** ECMA types ******************************************/

#define GET_ECMA_TOUCHED(thistype) \
	case FIELDTYPE_SF##thistype: {\
		touched = findNameInECMATable( (JSContext*)scriptcontrol->cx,fullname);\
		break;\
	}

#define GET_ECMA_MF_TOUCHED(thistype) \
	case FIELDTYPE_MF##thistype: {\
		jsval mainElement; \
		/* printf ("GET_ECMA_MF_TOUCHED called on %d\n",JSglobal_return_val);  */ \
		if (!JS_GetProperty(cx, JSVAL_TO_OBJECT(*(jsval*)(tg->JScript.JSglobal_return_val)), "MF_ECMA_has_changed", &mainElement)) { \
			printf ("JS_GetProperty failed for \"MF_ECMA_HAS_changed\" in get_valueChanged_flag\n"); \
		} /* else printf ("GET_ECMA_MF_TOUCHED MF_ECMA_has_changed is %d for %d %d\n",JSVAL_TO_INT(mainElement),cx,JSglobal_return_val); */  \
		touched = JSVAL_TO_INT(mainElement);\
		break; \
	}

#define RESET_ECMA_MF_TOUCHED(thistype) \
	case FIELDTYPE_##thistype: {\
		jsval myv = INT_TO_JSVAL(0); \
		/* printf ("RESET_ECMA_MF_TOUCHED called on %d ",JSglobal_return_val); */ \
		if (!JS_SetProperty(cx, JSVAL_TO_OBJECT(*(jsval*)(tg->JScript.JSglobal_return_val)), "MF_ECMA_has_changed", &myv)) { \
			printf( "JS_SetProperty failed for \"MF_ECMA_has_changed\" in RESET_ECMA_MF_TOUCHED.\n"); \
		}\
			/* if (!JS_GetProperty( p->ScriptControl[actualscript].cx, JSVAL_TO_OBJECT(JSglobal_return_val), "MF_ECMA_has_changed", &mainElement)) { \
					printf ("JS_GetProperty failed for \"MF_ECMA_HAS_changed\" in get_valueChanged_flag\n"); \
			} \
		printf ("and MF_ECMA_has_changed is %d\n",JSVAL_TO_INT(mainElement)); */\
	break; \
	}

#define RESET_TOUCHED_TYPE_ECMA(thistype) \
			case FIELDTYPE_##thistype: { \
				resetNameInECMATable( cx,JSparamnames[fptr].name); \
				break; \
			}

#ifdef OLDCODE
OLDCODE /* in case Bool was defined above, restore the value */
OLDCODE #define Bool savedBool
#endif //OLDCODE




/********************************************************************************/
/*									    	*/
/* get_valueChanged_flag - see if this variable (can be a sub-field; see tests   	*/
/* 8.wrl for the DEF PI PositionInterpolator). return true if variable is   	*/
/* touched, and pointer to touched value is in global variable              	*/
/* JSglobal_return_val, AND possibly:						*/
/*	void *JSSFpointer for SF non-ECMA nodes.				*/
/* 										*/
/* the way touched, and, the actual values work is as follows:			*/
/*										*/
/* keep track of the name in a table, and set valueChanged flag.		*/
/* look around the function setECMANative to see how this is done.		*/
/* FIELDTYPE_SFInt32								*/
/* FIELDTYPE_SFBool								*/
/* FIELDTYPE_SFFloat								*/
/* FIELDTYPE_SFTime								*/
/* FIELDTYPE_SFDouble								*/
/* FIELDTYPE_SFString								*/
/*										*/
/* check the "touched" flag for non-zero in the private area:			*/
/* FIELDTYPE_SFRotation								*/
/* FIELDTYPE_SFNode								*/
/* FIELDTYPE_SFVec2f								*/
/* FIELDTYPE_SFVec3f								*/
/* FIELDTYPE_SFImage								*/
/* FIELDTYPE_SFColor								*/
/* FIELDTYPE_SFColorRGBA							*/
/*										*/
/* go through all elements, and find if at least one SF has been touched:	*/
/* FIELDTYPE_MFRotation								*/
/* FIELDTYPE_MFNode								*/
/* FIELDTYPE_MFVec2f								*/
/* FIELDTYPE_MFVec3f								*/
/* FIELDTYPE_MFColor								*/
/* FIELDTYPE_MFColorRGBA							*/


/* has a flag called "MF_ECMA_has_changed" that is used here 			*/
/* FIELDTYPE_MFFloat	*/
/* FIELDTYPE_MFBool	*/
/* FIELDTYPE_MFInt32	*/
/* FIELDTYPE_MFTime	*/
/* FIELDTYPE_MFString	*/
/*                                                                          */
/****************************************************************************/
extern "C" {
int sm1_get_valueChanged_flag (int fptr, int actualscript) {


	struct CRscriptStruct *scriptcontrol;
	JSContext *cx;
	JSObject *interpobj;
	char *fullname;
	int touched;
	//ppJScript p;
	ttglobal tg = gglobal();
	struct CRjsnameStruct *JSparamnames = getJSparamnames();
	//p = (ppJScript)tg->JScript.prv;

	touched = FALSE;
	scriptcontrol = getScriptControlIndex(actualscript);
	interpobj = (JSObject*)scriptcontrol->glob;
	cx =  (JSContext*)scriptcontrol->cx;
	fullname = JSparamnames[fptr].name;

	{ // Scope A  for our various stack objects (JSAutoRequest, RootedObject), so they all go
		// out of scope before we JS_DestroyContext.
		JSAutoRequest ar(cx); // In practice, you would want to exit this any
							// time you're spinning the event loop
		{ // Scope B  for JSAutoCompartment
			JSAutoCompartment ac(cx, interpobj);

			if (!JS_GetProperty(cx,  interpobj ,fullname,(jsval*)tg->JScript.JSglobal_return_val)) {
				printf ("cant get property for %s\n",fullname);
				return FALSE;
			} else {

				switch (JSparamnames[fptr].type) {
					GETJSPTR_TYPE_A(SFRotation)
					GETJSPTR_TYPE_A(SFNode)
					GETJSPTR_TYPE_A(SFVec2f)
					/* GETJSPTR_TYPE_A(SFVec2d) */
					GETJSPTR_TYPE_A(SFVec3f)
					GETJSPTR_TYPE_A(SFVec3d)
					GETJSPTR_TYPE_A(SFVec4f)
					GETJSPTR_TYPE_A(SFVec4d)
					GETJSPTR_TYPE_A(SFImage)
					GETJSPTR_TYPE_A(SFColor)
					GETJSPTR_TYPE_A(SFColorRGBA)

					GETJSPTR_TYPE_MF_A(MFRotation,SFRotation)
					GETJSPTR_TYPE_MF_A(MFNode,SFNode)
					GETJSPTR_TYPE_MF_A(MFVec2f,SFVec2f)
					GETJSPTR_TYPE_MF_A(MFVec3f,SFVec3f)
					GETJSPTR_TYPE_MF_A(MFVec4f,SFVec4f)
					GETJSPTR_TYPE_MF_A(MFVec4d,SFVec4d)
					/* GETJSPTR_TYPE_MF_A(MFImage,SFImage)  */
					GETJSPTR_TYPE_MF_A(MFColor,SFColor)
					GETJSPTR_TYPE_MF_A(MFColorRGBA,SFColorRGBA)

					GET_ECMA_MF_TOUCHED(Int32)
					GET_ECMA_MF_TOUCHED(Bool)
					GET_ECMA_MF_TOUCHED(Time)
					GET_ECMA_MF_TOUCHED(Double)
					GET_ECMA_MF_TOUCHED(Float)
					GET_ECMA_MF_TOUCHED(String)

					GET_ECMA_TOUCHED(Int32)
					GET_ECMA_TOUCHED(Bool)
					GET_ECMA_TOUCHED(Float)
					GET_ECMA_TOUCHED(Time)
					GET_ECMA_TOUCHED(Double)
					GET_ECMA_TOUCHED(String)

					default: {printf ("not handled yet in get_valueChanged_flag %s\n",FIELDTYPES[JSparamnames[fptr].type]);
					}
				}
			}

		} //Scope B
	} //Scope A
	return touched;

}


/* this script value has been looked at, set the touched flag in it to FALSE. */
void sm1_resetScriptTouchedFlag(int actualscript, int fptr) {

	struct CRscriptStruct *scriptcontrol;
	ttglobal tg = gglobal();
	struct CRjsnameStruct *JSparamnames = getJSparamnames();
	JSContext *cx;
	JSObject *global;
	scriptcontrol = getScriptControlIndex(actualscript);
	cx = (JSContext*)scriptcontrol->cx;
	global = (JSObject*)scriptcontrol->glob;
	{ // Scope A  for our various stack objects (JSAutoRequest, RootedObject), so they all go
		// out of scope before we JS_DestroyContext.
		JSAutoRequest ar(cx); // In practice, you would want to exit this any
							// time you're spinning the event loop
		{ // Scope B  for JSAutoCompartment
			JSAutoCompartment ac(cx, global);

			switch (JSparamnames[fptr].type) {
				RESET_TOUCHED_TYPE_A(SFRotation)
				RESET_TOUCHED_TYPE_A(SFNode)
				RESET_TOUCHED_TYPE_A(SFVec2f)
				RESET_TOUCHED_TYPE_A(SFVec3f)
				RESET_TOUCHED_TYPE_A(SFVec4f)
				/* RESET_TOUCHED_TYPE_A(SFVec2d) */
				RESET_TOUCHED_TYPE_A(SFVec3d)
				RESET_TOUCHED_TYPE_A(SFVec4d)
				RESET_TOUCHED_TYPE_A(SFImage)
				RESET_TOUCHED_TYPE_A(SFColor)
				RESET_TOUCHED_TYPE_A(SFColorRGBA)
				RESET_TOUCHED_TYPE_MF_A(MFRotation,SFRotation)
				RESET_TOUCHED_TYPE_MF_A(MFNode,SFNode)
				RESET_TOUCHED_TYPE_MF_A(MFVec2f,SFVec2f)
				RESET_TOUCHED_TYPE_MF_A(MFVec3f,SFVec3f)
				RESET_TOUCHED_TYPE_MF_A(MFVec4f,SFVec4f)
				RESET_TOUCHED_TYPE_MF_A(MFVec4d,SFVec4d)
				/* RESET_TOUCHED_TYPE_MF_A(MFImage,SFImage) */
				RESET_TOUCHED_TYPE_MF_A(MFColor,SFColor)
				RESET_TOUCHED_TYPE_MF_A(MFColorRGBA,SFColorRGBA)

				RESET_TOUCHED_TYPE_ECMA (SFInt32)
				RESET_TOUCHED_TYPE_ECMA (SFBool)
				RESET_TOUCHED_TYPE_ECMA (SFFloat)
				RESET_TOUCHED_TYPE_ECMA (SFTime)
				RESET_TOUCHED_TYPE_ECMA (SFDouble)
				RESET_TOUCHED_TYPE_ECMA (SFString)
				RESET_ECMA_MF_TOUCHED(MFInt32)
				RESET_ECMA_MF_TOUCHED(MFBool)
				RESET_ECMA_MF_TOUCHED(MFFloat)
				RESET_ECMA_MF_TOUCHED(MFTime)
				RESET_ECMA_MF_TOUCHED(MFString)
				default: {printf ("can not reset touched_flag for %s\n",stringFieldtypeType(JSparamnames[fptr].type));}
			}
		} //Scope B
	} //Scope A
}

int sm2_get_valueChanged_flag (int fptr, int actualscript){
	char *fullname;
	union anyVrml* value;
	int type, kind, ifield, found;
	struct X3D_Node *node;
	struct Shader_Script *script;
	struct ScriptFieldDecl *field;
	struct CRscriptStruct *scriptcontrol; //, *ScriptControlArr = getScriptControl();
	struct CRjsnameStruct *JSparamnames = getJSparamnames();

	scriptcontrol = getScriptControlIndex(actualscript); //&ScriptControlArr[actualscript];
	script = scriptcontrol->script;
	node = script->ShaderScriptNode;
	fullname = JSparamnames[fptr].name;
	found = getFieldFromNodeAndName(node,fullname,&type,&kind,&ifield,&value);
	if(found){
		field = Shader_Script_getScriptField(script, ifield);
		gglobal()->JScript.JSglobal_return_val = (void *)&field->value;
		return field->valueChanged;
	}
	gglobal()->JScript.JSglobal_return_val = NULL;
	return 0;
}
void sm2_resetScriptTouchedFlag(int actualscript, int fptr){
	char *fullname;
	union anyVrml* value;
	int type, kind, ifield, found;
	struct X3D_Node *node;
	struct Shader_Script *script;
	struct ScriptFieldDecl *field;
	struct CRscriptStruct *scriptcontrol; // *ScriptControlArr = getScriptControl();
	struct CRjsnameStruct *JSparamnames = getJSparamnames();

	scriptcontrol = getScriptControlIndex(actualscript); //&ScriptControlArr[actualscript];
	script = scriptcontrol->script;
	node = script->ShaderScriptNode;
	fullname = JSparamnames[fptr].name;
	found = getFieldFromNodeAndName(node,fullname,&type,&kind,&ifield,&value);
	if(found){
		field = Shader_Script_getScriptField(script, ifield);
		field->valueChanged = 0;
	}
	//printf("in get_valueChanged_flag\n");
	return;
}
int sm_get_valueChanged_flag (int fptr, int actualscript){
	if(SM_method() == 2)
		return sm2_get_valueChanged_flag(fptr,actualscript);
	else
		return sm1_get_valueChanged_flag(fptr,actualscript);
}
void sm_resetScriptTouchedFlag(int actualscript, int fptr){
	if(SM_method() == 2)
		sm2_resetScriptTouchedFlag(actualscript,fptr);
	else
		sm1_resetScriptTouchedFlag(actualscript,fptr);
}

int jsActualrunScript(int num, char *script);
void sm_JSInitializeScriptAndFields (int num) {
	//jsval rval;
	//ppCRoutes p = (ppCRoutes)gglobal()->CRoutes.prv;
	struct CRscriptStruct *ScriptControl; // = getScriptControl();

	/* printf ("JSInitializeScriptAndFields script %d, thread %u\n",num,pthread_self());   */
	/* run through paramList, and run the script */
	/* printf ("JSInitializeScriptAndFields, running through params and main script\n");  */
	//if (num >= p->JSMaxScript)  {
	//	ConsoleMessage ("JSInitializeScriptAndFields: warning, script %d initialization out of order",num);
	//	return;
	//}
	/* run through fields in order of entry in the X3D file */

	int i,nfields,kind,itype;
	const char *fieldname;
	char longfieldname[256];
	struct Shader_Script *script;
	struct ScriptFieldDecl *field;

	ScriptControl = getScriptControlIndex(num);

	//add user code -including eventIn functions- first
	if(1) if (!jsActualrunScript(num, ScriptControl->scriptText)) {
		ConsoleMessage ("JSInitializeScriptAndFields, script failure\n");
		ScriptControl->scriptOK = FALSE;
		ScriptControl->_initialized = TRUE;
		return;
	}

	// old way: when adding inputOutput fieldnname, check first if there's a user
	// eventin function with the same name, and if so rename it to set_fieldname
	// new way dec 2017: rename inputOutput functions with a set_prefix
	// may 2018: keep inputOnly function at its normal name and
	//   prefix inputOnly fieldname with __eventIn_Value_ 
	//   (inputOutput function is still dec 2017 renamed to set_, and inputOutput field has normal nae)
	script = ScriptControl->script;
	//printf("adding fields from script %x\n",script);
	nfields = Shader_Script_getScriptFieldCount(script);
	for(i=0;i<nfields;i++){
		field = Shader_Script_getScriptField(script,i);
		fieldname = ScriptFieldDecl_getName(field);
		kind = ScriptFieldDecl_getMode(field);
		itype = ScriptFieldDecl_getType(field);
		longfieldname[0] = 0;
		if(kind == PKW_inputOnly)
			strcat(longfieldname,"__eventIn_Value_");
		strcat(longfieldname,fieldname);
		if(SM_method() == 2)
			InitScriptFieldB(num, kind, itype, longfieldname, field->value);
		else
			InitScriptField(num, kind, itype, longfieldname, field->value);
	}


	if(0) if (!jsActualrunScript(num, ScriptControl->scriptText)) {
		ConsoleMessage ("JSInitializeScriptAndFields, script failure\n");
		ScriptControl->scriptOK = FALSE;
		ScriptControl->_initialized = TRUE;
		return;
	}

	//FREE_IF_NZ(ScriptControl->scriptText);
	ScriptControl->_initialized = TRUE;
	ScriptControl->scriptOK = TRUE;

}



/* save this field from the parser; initialize it when the fwl_RenderSceneUpdateScene wants to initialize it */
void sm_SaveScriptField (int num, indexT kind, indexT type, const char* field, union anyVrml value) {

}

} //extern "C"


/****************************************************************/
/* a Jscript is returning a Multi-number type; copy this from 	*/
/* the Jscript return string to the data structure within the	*/
/* freewrl C side of things.					*/
/*								*/
/* note - this cheats in that the code assumes that it is 	*/
/* a series of Multi_Vec3f's while in reality the structure	*/
/* of the multi structures is the same - so we "fudge" things	*/
/* to make this multi-purpose.					*/
/****************************************************************/
void getJSMultiNumType (JSContext *cx, struct Multi_Vec3f *tn, int eletype) {
	float *fl;
	long *il;
	double *dl;
	struct X3D_Node * *nl;

	double dtmp;
	jsval mainElement;
	int len;
	int i;
	char *strp;

	char *strpp; /* we need this to reliably free the results of JS_EncodeString() */

	int elesize;
	SFVec2fNative *sfvec2f;
	SFVec3fNative *sfvec3f;
	SFRotationNative *sfrotation;
	struct Uni_String * *ms;
	jsval *myJSVal;
	ttglobal tg = gglobal();

	/* get size of each element, used for MALLOCing memory  - eg, this will
	   be sizeof(float) * 3 for a SFColor */
	elesize = returnElementLength(eletype) * returnElementRowSize(eletype);

	/* rough check of return value */
	/* where did this come from? Was it from a script execution, or from an assignment from within a script?? */
	#ifdef SETFIELDVERBOSE
	printf ("getJSMultiNumType, JSCreate_global_return_val %u, JSglobal_return_val %u\n",
		(unsigned int) JSVAL_TO_INT(*(jsval *)tg->jsVRMLBrowser.JSCreate_global_return_val),
		(unsigned int) JSVAL_TO_INT(*(jsval *)tg->JScript.JSglobal_return_val));
	#endif

	if (JSVAL_TO_INT(*(jsval*)(tg->jsVRMLBrowser.JSCreate_global_return_val)) != 0) {
		myJSVal = (jsval *)tg->jsVRMLBrowser.JSCreate_global_return_val;
		*(jsval *)(tg->jsVRMLBrowser.JSCreate_global_return_val) = INT_TO_JSVAL(0);

		#ifdef SETFIELDVERBOSE
		printf ("getJSMultiNumType: using JSCreate_global_return_val\n");
		#endif
	} else {
		#ifdef SETFIELDVERBOSE
		printf ("getJSMultiNumType: using JSglobal_return_val\n");
		#endif

		myJSVal = (jsval *)tg->JScript.JSglobal_return_val;
	}

	///if (!JSVAL_IS_OBJECT(*myJSVal)) {
	if(!(*myJSVal).isObject()){
		printf ("getJSMultiNumType - did not get an object\n");
		return;
	}

	#ifdef SETFIELDVERBOSE
	printf ("getJSMultiNumType, tn %p dest has  %s size %d\n",tn,stringFieldtypeType(eletype), elesize);

	printf("getJSMulitNumType, node type of myJSVal is :");
	printJSNodeType (cx,myJSVal);
	#endif

	if (!JS_GetProperty(cx, JSVAL_TO_OBJECT(*myJSVal),  MF_LENGTH_FIELD, &mainElement)) {
		printf ("JS_GetProperty failed for \"%s\" in getJSMultiNumType\n", MF_LENGTH_FIELD);
		return;
	}
	len = JSVAL_TO_INT(mainElement);
	#ifdef SETFIELDVERBOSE
	printf ("getmuiltie length of grv is %d old len is %d\n",len,tn->n);
	#endif

	/* do we have to realloc memory? */
	if (len != tn->n) {

		tn->n = 0;
		/* yep... */
			/* printf ("old pointer %d\n",tn->p); */
		FREE_IF_NZ (tn->p);
		tn->p = MALLOC (struct SFVec3f *, (unsigned)(elesize*len));

		#ifdef SETFIELDVERBOSE
		printf ("MALLOCing memory for elesize %d len %d new pointer now is %p\n",elesize,len,tn->p);
		#endif

		/* if this is an MFString, we should set each element to a null string */
		if (eletype == FIELDTYPE_SFString) {
			#ifdef SETFIELDVERBOSE
			printf ("getJSMultiNumType, this is a MFString, so making tiny strings for now\n");
			#endif

			ms = (struct Uni_String * *) tn->p;
			for (i=0; i<len; i++) {
				*ms = newASCIIString ("(getJSMultiNumType null)");
				ms ++;
			}
		}
	}

	/* set these three up, but we only use one of them */
	fl = (float *) tn->p;
	il = (long *) tn->p;
	dl = (double *) tn->p;
	nl = (struct X3D_Node * *) tn->p;
	ms = (struct Uni_String * *) tn->p;

	/* go through each element of the main array. */
	for (i = 0; i < len; i++) {
		if (!JS_GetElement(cx, JSVAL_TO_OBJECT(*myJSVal), i, &mainElement)) {
			printf ("WARNING: JS_GetElement failed for %d in getJSMultiNumType\n",i);
			switch (eletype) {
			case FIELDTYPE_SFNode:
				*nl = 0; nl++; break;
			case FIELDTYPE_SFInt32:
				*il=0; il++; break;
			case FIELDTYPE_SFDouble:
			case FIELDTYPE_SFTime:
				*dl=0.0; dl++; break;
			case FIELDTYPE_SFFloat:
				*fl = (float) 0.0;  fl++; break;
			case FIELDTYPE_SFVec2f:
				*fl = (float) 0.0;  fl++; *fl = (float) 0.0;  fl++; break;
			case FIELDTYPE_SFVec3f:
			case FIELDTYPE_SFColor:
				*fl = (float) 0.0;  fl++; *fl = (float) 0.0;  fl++; *fl = (float) 0.0;  fl++; break;
			case FIELDTYPE_SFRotation:
				*fl = (float) 0.0;  fl++; *fl = (float) 0.0;  fl++; *fl = (float) 0.0;  fl++; *fl = (float) 0.0;  fl++; break;
			case FIELDTYPE_SFString:
	                        verify_Uni_String (*ms,"(empty value)"); ms++; break;

			default : {printf ("getJSMultiNumType unhandled eletype: %d\n",
					eletype);
					return;
				}
			}
		} else {
			#ifdef SETFIELDVERBOSE
			JSString *_tmpStr;

			_tmpStr = JS_ValueToString(cx, mainElement);
#if JS_VERSION < 185
			strp = JS_GetStringBytes(_tmpStr);
#else
			strp = strpp = JS_EncodeString(cx,_tmpStr);
#endif
			printf ("sub element %d is \"%s\" \n",i,strp);
#if JS_VERSION >= 185
			JS_free(cx,strpp);
#endif

			if (JSVAL_IS_OBJECT(mainElement)) printf ("sub element %d is an OBJECT\n",i);
			if (JSVAL_IS_PRIMITIVE(mainElement)) printf ("sub element %d is an PRIMITIVE\n",i);
			#endif

			/* code is pretty much same as SF* values in setField_javascriptEventOut */
			switch (eletype) {
			case FIELDTYPE_SFNode: {

				if (JS_InstanceOf (cx, JSVAL_TO_OBJECT(mainElement), &SFNodeClass, NULL)) {
					SFNodeNative *_vec;

					/* printf ("yep, this is an SFNode class\n");  */
					if ((_vec = (SFNodeNative *)JS_GetPrivateFw(cx, JSVAL_TO_OBJECT(mainElement))) == NULL) {
						printf ("error getting native\n");
						*nl = NULL;
					} else {
						/* printf ("have native, handle %p\n",_vec->handle);
						printf ("and it is a :%s:\n",stringNodeType(_vec->handle->_nodeType)); */
						*nl = _vec->handle;
					}
				} else {
					printf ("hmm - not an SFNode class\n");
					*nl = NULL;
				}

				nl++;
				break;
			}
			case FIELDTYPE_SFInt32: {
				//if (!JS::ToInt32(cx, mainElement ,il)) {
				//	printf ("error\n");
				//	*il=0;
				//}
				*il = 0;
				if(mainElement.isInt32()) *il = mainElement.toInt32();
				il++;
				break;
			}
			case FIELDTYPE_SFDouble:
			case FIELDTYPE_SFTime: {
				if (!JS_ValueToNumber(cx, mainElement ,dl)) *dl=0.0;
				dl++;
				break;
			}
			case FIELDTYPE_SFFloat: {
				if (!JS_ValueToNumber(cx, mainElement, &dtmp)) dtmp=0.0;
				/* convert double precision to single, for X3D */
				*fl = (float) dtmp;
				fl++;
				break;
			}
			case FIELDTYPE_SFVec2f: {
				//if (JSVAL_IS_OBJECT(mainElement)) {
				if(mainElement.isObject()){
					if ((sfvec2f = (SFVec2fNative *)JS_GetPrivateFw(cx, JSVAL_TO_OBJECT(mainElement))) == NULL) {
							printf( "JS_GetPrivate failed for obj in setField_javascriptEventOut.\n");
							return;
					}
					memcpy ((void *)fl, (void *)&(sfvec2f->v),2*sizeof(float));
					fl += 2;
				} else {
					/* we are working in a value that kind of exists, but is undefined */
					*fl = (float) 0.0; fl++; *fl = (float) 0.0; fl++;
				}
				break;
			}
			case FIELDTYPE_SFVec3f:
			case FIELDTYPE_SFColor: {       /* SFColor */
					//if (JSVAL_IS_OBJECT(mainElement)) {
					if(mainElement.isObject()){
					if ((sfvec3f = (SFVec3fNative *)JS_GetPrivateFw(cx, JSVAL_TO_OBJECT(mainElement))) == NULL) {
							printf( "JS_GetPrivate failed for obj in setField_javascriptEventOut.\n");
							return;
					}
					memcpy ((void *)fl, (void *)&(sfvec3f->v),3*sizeof(float));
					fl += 3;
				} else {
					/* we are working in a value that kind of exists, but is undefined */
					*fl = (float) 0.0; fl++;
					*fl = (float) 0.0; fl++;
					*fl = (float) 0.0; fl++;
				}
				break;
			}
			case FIELDTYPE_SFRotation: {
				//if (JSVAL_IS_OBJECT(mainElement)) {
				if(mainElement.isObject()){
					if ((sfrotation = (SFRotationNative *)JS_GetPrivateFw(cx, JSVAL_TO_OBJECT(mainElement))) == NULL) {
							printf( "JS_GetPrivate failed for obj in setField_javascriptEventOut.\n");
							return;
					}
					memcpy ((void *)fl, (void *)&(sfrotation->v),4*sizeof(float));
					fl += 4;
				} else {
					/* we are working in a value that kind of exists, but is undefined */
					*fl = (float) 0.0; fl++;	*fl = (float) 0.0; fl++;
					*fl = (float) 0.0; fl++;	*fl = (float) 0.0; fl++;
				}
				break;
			}

			case FIELDTYPE_SFString: {
				JSString *strval;

				strval = JS_ValueToString(cx, mainElement);
				strp = strpp = JS_EncodeString(cx,strval);



				#ifdef SETFIELDVERBOSE
				printf ("getJSMultiNumType, got string %s\n",strp);
				#endif

				/* copy the string over, delete the old one, if need be */
				verify_Uni_String (*ms,strp);
				ms++;

				JS_free(cx,strpp);

				break;
			}

			default : {printf ("getJSMultiNumType unhandled eletype: %d\n",
					eletype);
				   return;
				}
			}
		}

	}
	#ifdef SETFIELDVERBOSE
	printf ("getJSMultiNumType, setting old length %d to length %d\n",tn->n, len);
	#endif

	tn->n = len;
}

/****************************************************************/
/* a script is returning a MFString type; add this to the C	*/
/* children field						*/
/****************************************************************/
void getMFStringtype (JSContext *cx, jsval *from, struct Multi_String *to) {
	int oldlen, newlen;
	jsval _v;
	JSObject *obj;
	int i;
	char *valStr, *OldvalStr;
	struct Uni_String **svptr;
	struct Uni_String **newp, **oldp;
	int count;

	JSString *strval; /* strings */

	oldlen = to->n;
	svptr = to->p;
	newlen=0;

	if (!JS_ValueToObject(cx, *from, &obj))
		printf ("JS_ValueToObject failed in getMFStringtype\n");

	if (!JS_GetProperty(cx, obj,  MF_LENGTH_FIELD, &_v)) {
		printf ("JS_GetProperty failed for \"%s\" in getMFStringtype.\n", MF_LENGTH_FIELD);
        }

	newlen = JSVAL_TO_INT(_v);

	/* printf ("getMFStringType, newlen %d oldlen %d\n",newlen,oldlen); */


	/*  if we have to expand size of SV... */
	if (newlen > oldlen) {
		oldp = to->p; /* same as svptr, assigned above */
		to->n = newlen;
		to->p = MALLOC(struct Uni_String **, newlen * sizeof(to->p));
		newp = to->p;

		/* copy old values over */
		for (count = 0; count <oldlen; count ++) {
			/*printf ("copying over element %d\n",count); */
			*newp = *oldp;
			newp++;
			oldp++;
		}

		/* zero new entries */
		for (count = oldlen; count < newlen; count ++) {
			/* make the new SV */
			*newp = MALLOC (struct Uni_String *, sizeof (struct Uni_String));


			/* now, make it point to a blank string */
			*newp = newASCIIString("");
			newp ++;
		}
		FREE_IF_NZ (svptr);
		svptr = to->p;
	} else {
		/* possibly truncate this, but leave the memory alone. */
		to->n = newlen;
	}

	/* printf ("verifying structure here\n");
	for (i=0; i<(to->n); i++) {
		printf ("indx %d flag %x string :%s: len1 %d len2 %d\n",i,
				(svptr[i])->sv_flags,
	}
	printf ("done\n");
	*/


	for (i = 0; i < newlen; i++) {
		/* get the old string pointer */
		OldvalStr = svptr[i]->strptr;
		/* printf ("old string at %d is %s len %d\n",i,OldvalStr,strlen(OldvalStr)); */

		/* get the new string pointer */
		if (!JS_GetElement(cx, obj, i, &_v)) {
			fprintf(stderr,
				"JS_GetElement failed for %d in getMFStringtype\n",i);
			return;
		}
		strval = JS_ValueToString(cx, _v);

		valStr = JS_EncodeString(cx,strval);


		/* printf ("new string %d is %s\n",i,valStr); */

		/*  if the strings are different... */
		if (strcmp(valStr,OldvalStr) != 0) {
			/* MALLOC a new string, of correct len for terminator */
			svptr[i] =  newASCIIString(valStr);
		}

		JS_free(cx,valStr);

	}
	/*
	printf ("\n new structure: %d %d\n",svptr,newlen);
	for (i=0; i<newlen; i++) {
		printf ("indx %d string :%s: len1 %d len2 %d\n",i,
				mypv->xpv_pv, mypv->xpv_cur,mypv->xpv_len);
	}
	*/

}




void setField_javascriptEventOut(struct X3D_Node *tn,unsigned int tptr,  int fieldType, unsigned len, int extraData, JSContext *scriptContext) {
	long ival;
	double tval;
	float fl[4];
	char *memptr;
	JSString *strval; /* strings */
	char *strp;
	char *strpp;  /* strp is modified, so we cannot use it to free JS_EncodeString results */
	ttglobal tg = gglobal();


	/* set up a pointer to where to put this stuff */
	memptr = offsetPointer_deref(char *, tn, tptr);

#define GETJSVAL_TYPE_A(thistype,field) \
		case FIELDTYPE_##thistype: { \
			/* printf ("doing TYPEA memcpy to %u, from %u, len %d\n",(void *)memptr, (void *) &(((thistype##Native *)JSSFpointer)->field),len); */ \
			memcpy ((void *)memptr, (void *) &(((thistype##Native *)tg->CRoutes.JSSFpointer)->field),len); \
			break; \
		}

#define GETJSVAL_TYPE_MF_A(MFtype,SFtype) \
		case FIELDTYPE_##MFtype: {getJSMultiNumType (scriptContext, (struct Multi_Vec3f *)memptr,FIELDTYPE_##SFtype); break;}

	switch (fieldType) {
		GETJSVAL_TYPE_A(SFRotation,v)
		/* GETJSVAL_TYPE_A(SFNode,handle) */
		/* not implemented yet? GETJSVAL_TYPE_A(SFVec2d,v) */
		GETJSVAL_TYPE_A(SFVec3d,v)
		GETJSVAL_TYPE_A(SFVec4d,v)
		GETJSVAL_TYPE_A(SFVec2f,v)
		GETJSVAL_TYPE_A(SFVec3f,v)
		GETJSVAL_TYPE_A(SFVec4f,v)
		GETJSVAL_TYPE_A(SFColor,v)
		GETJSVAL_TYPE_A(SFColorRGBA,v)

		GETJSVAL_TYPE_MF_A(MFRotation,SFRotation)
		GETJSVAL_TYPE_MF_A(MFVec2d,SFVec2d)
		GETJSVAL_TYPE_MF_A(MFVec3d,SFVec3d)
		GETJSVAL_TYPE_MF_A(MFVec4d,SFVec4d)
		GETJSVAL_TYPE_MF_A(MFVec2f,SFVec2f)
		GETJSVAL_TYPE_MF_A(MFVec3f,SFVec3f)
		GETJSVAL_TYPE_MF_A(MFVec4f,SFVec4f)
		GETJSVAL_TYPE_MF_A(MFColor,SFColor)
		GETJSVAL_TYPE_MF_A(MFColorRGBA,SFColorRGBA)


		case FIELDTYPE_SFInt32:
		case FIELDTYPE_SFBool:	{	/* SFBool */
			jsval val;
			val = *(jsval*)(tg->JScript.JSglobal_return_val);
			//if (!JS::ToInt32(scriptContext, *(jsval*)(tg->JScript.JSglobal_return_val),&ival)) {
			ival=0;
			if(! (val.isBoolean() || val.isInt32())){
				printf ("error\n");
			}
			if(val.isBoolean()) ival = val.isTrue() ? TRUE : FALSE;
			if(val.isInt32()) ival = val.toInt32() == 0 ? FALSE : TRUE;
			memcpy ((void *)memptr, (void *)&ival,len);
			break;
		}

		case FIELDTYPE_SFDouble:
		case FIELDTYPE_SFTime: {
			if (!JS_ValueToNumber(scriptContext, *(jsval*)(tg->JScript.JSglobal_return_val),&tval)) tval=0.0;
			memcpy ((void *)memptr, (void *)&tval,len);
			break;
		}

		case FIELDTYPE_SFFloat: {
			if (!JS_ValueToNumber(scriptContext, *(jsval*)(tg->JScript.JSglobal_return_val),&tval)) tval=0.0;
			/* convert double precision to single, for X3D */
			fl[0] = (float) tval;
			memcpy ((void *)memptr, (void *)fl,len);
			break;
		}

		case FIELDTYPE_SFImage: {
			/* the string should be saved as an SFImage */
			strval = JS_ValueToString(scriptContext, *(jsval*)(tg->JScript.JSglobal_return_val));
			strp = strpp = JS_EncodeString(scriptContext,strval);
			Parser_scanStringValueToMem(tn, tptr, FIELDTYPE_SFImage, strp, FALSE);
			JS_free(scriptContext,strpp);
			break;
		}

		case FIELDTYPE_SFString: {
			struct Uni_String *ms;
			intptr_t *newptr;

			strval = JS_ValueToString(scriptContext, *(jsval*)(tg->JScript.JSglobal_return_val));
			strp = strpp = JS_EncodeString(scriptContext,strval);
			/* copy the string over, delete the old one, if need be */
			/* printf ("fieldSet SFString, tn %d tptr %d offset from struct %d\n",
				tn, tptr, offsetof (struct X3D_TextureCoordinateGenerator, mode)); */
			newptr = (intptr_t *)memptr;
			ms = (struct Uni_String*) *newptr;
			verify_Uni_String (ms,strp);
			JS_free(scriptContext,strpp);
			break;
		}

			/* a series of Floats... */
		case FIELDTYPE_MFFloat: {getJSMultiNumType (scriptContext, (struct Multi_Vec3f *)memptr,FIELDTYPE_SFFloat); break;}
		case FIELDTYPE_MFInt32: {getJSMultiNumType (scriptContext, (struct Multi_Vec3f *)memptr,FIELDTYPE_SFInt32); break;}
		case FIELDTYPE_MFTime: {getJSMultiNumType (scriptContext, (struct Multi_Vec3f *)memptr,FIELDTYPE_SFTime); break;}
		case FIELDTYPE_MFDouble: {getJSMultiNumType (scriptContext, (struct Multi_Vec3f *)memptr,FIELDTYPE_SFDouble); break;}
		case FIELDTYPE_MFNode: {
			struct X3D_Node *mynode;

			strval = JS_ValueToString(scriptContext, *(jsval*)(tg->JScript.JSglobal_return_val));
			strp = strpp = JS_EncodeString(scriptContext,strval);

			/* we will have at least one node here, in an ascii string */
			while ((*strp > '\0') && (*strp <= ' ')) strp ++;
			/* are we at a bracket? */
			if (*strp == '[') strp ++;
			while ((*strp > '\0') && (*strp <= ' ')) strp ++;

			/* printf ("convertingthe following string to a pointer :%s:\n",strp); */
#ifndef _x64
			mynode = X3D_NODE(atol(strp));
#else
			mynode = X3D_NODE(atoll(strp));
#endif
			JS_free(scriptContext,strpp);

			/* printf ("mynode is %p %d, \n",mynode,mynode);
			printf ("mynode is %p %d, type %d\n",mynode,mynode,mynode->_nodeType);
			printf ("calling getMFNodeType now\n"); */


			getMFNodetype (mynode,(struct Multi_Node *)memptr,X3D_NODE(tn),extraData); break;
		}
		case FIELDTYPE_MFString: {
			getMFStringtype (
				scriptContext,
				(jsval *)tg->JScript.JSglobal_return_val,
				(struct Multi_String *)memptr);
			break;
		}

		case FIELDTYPE_SFNode: {
			//unsigned int valuecopied;
			//unsigned int *ptr2value;
			/* printf ("doing TYPEA memcpy to %u, from %u, len %d\n",(void *)memptr, (void *) &(((SFNodeNative *)JSSFpointer)->handle),returnElementLength(FIELDTYPE_SFNode));*/
			memcpy ((void *)memptr, (void *) &(((SFNodeNative *)tg->CRoutes.JSSFpointer)->handle),returnElementLength(FIELDTYPE_SFNode));
			//ptr2value = (unsigned int*)memptr;
			//valuecopied = *ptr2value;
			//printf("value of memptr %u after memcpy in script route= %u\n",(void*)memptr,valuecopied);
			break;
		}
		default: {	printf("WARNING: unhandled from type %s\n", stringFieldtypeType(fieldType));
		}
	}
}



void setField_javascriptEventOut_B(union anyVrml* any,
			int fieldType, unsigned len, int extraData, JSContext *scriptContext)
{
	//dug9 Feb 2013 for new propagate_events - like setField_javascriptEventout except:
	// writes to *anyVrml instead of (toNode,toOffset) combo (which doesn't work for Proto fields)
	// and doesn't update parents for SFNode,MFNode - that's done later.
	long ival;
	double tval;
	float fl[4];
	char *memptr;
	JSString *strval; /* strings */
	char *strp;
	char *strpp;  /* strp is modified, so we cannot use it to free JS_EncodeString results */
	ttglobal tg = gglobal();
	/* set up a pointer to where to put this stuff */
	memptr = (char *)any; //offsetPointer_deref(char *, tn, tptr);

#define GETJSVAL_TYPE_A(thistype,field) \
		case FIELDTYPE_##thistype: { \
			/* printf ("doing TYPEA memcpy to %u, from %u, len %d\n",(void *)memptr, (void *) &(((thistype##Native *)JSSFpointer)->field),len); */ \
			memcpy ((void *)memptr, (void *) &(((thistype##Native *)tg->CRoutes.JSSFpointer)->field),len); \
			break; \
		}

#define GETJSVAL_TYPE_MF_A(MFtype,SFtype) \
		case FIELDTYPE_##MFtype: {getJSMultiNumType (scriptContext, (struct Multi_Vec3f *)memptr,FIELDTYPE_##SFtype); break;}

	switch (fieldType) {
		GETJSVAL_TYPE_A(SFRotation,v)
		/* GETJSVAL_TYPE_A(SFNode,handle) */
		/* not implemented yet? GETJSVAL_TYPE_A(SFVec2d,v) */
		GETJSVAL_TYPE_A(SFVec3d,v)
		GETJSVAL_TYPE_A(SFVec4d,v)
		GETJSVAL_TYPE_A(SFVec2f,v)
		GETJSVAL_TYPE_A(SFVec3f,v)
		GETJSVAL_TYPE_A(SFVec4f,v)
		GETJSVAL_TYPE_A(SFColor,v)
		GETJSVAL_TYPE_A(SFColorRGBA,v)

		GETJSVAL_TYPE_MF_A(MFRotation,SFRotation)
		GETJSVAL_TYPE_MF_A(MFVec2d,SFVec2d)
		GETJSVAL_TYPE_MF_A(MFVec3d,SFVec3d)
		GETJSVAL_TYPE_MF_A(MFVec4d,SFVec4d)
		GETJSVAL_TYPE_MF_A(MFVec2f,SFVec2f)
		GETJSVAL_TYPE_MF_A(MFVec3f,SFVec3f)
		GETJSVAL_TYPE_MF_A(MFVec4f,SFVec4f)
		GETJSVAL_TYPE_MF_A(MFColor,SFColor)
		GETJSVAL_TYPE_MF_A(MFColorRGBA,SFColorRGBA)


		case FIELDTYPE_SFInt32:
		case FIELDTYPE_SFBool:	{	/* SFBool */
			
			jsval val = *(jsval*)(tg->JScript.JSglobal_return_val);
			if(!val.isBoolean() || val.isInt32()){
			//if (!JS::ToInt32(scriptContext, ,&ival)) {
				printf ("error\n");
				ival=0;
			}
			if(val.isBoolean()) ival = val.isTrue() ? TRUE : FALSE;
			if(val.isInt32()) ival = val.toInt32() == 0 ? FALSE : TRUE;
			memcpy ((void *)memptr, (void *)&ival,len);
			break;
		}

		case FIELDTYPE_SFDouble:
		case FIELDTYPE_SFTime: {
			if (!JS_ValueToNumber(scriptContext, *(jsval*)(tg->JScript.JSglobal_return_val),&tval)) tval=0.0;
			memcpy ((void *)memptr, (void *)&tval,len);
			break;
		}

		case FIELDTYPE_SFFloat: {
			if (!JS_ValueToNumber(scriptContext, *(jsval*)(tg->JScript.JSglobal_return_val),&tval)) tval=0.0;
			/* convert double precision to single, for X3D */
			fl[0] = (float) tval;
			memcpy ((void *)memptr, (void *)fl,len);
			break;
		}

		case FIELDTYPE_SFImage: {
			/* the string should be saved as an SFImage */
			strval = JS_ValueToString(scriptContext, *(jsval*)(tg->JScript.JSglobal_return_val));
			strp = strpp = JS_EncodeString(scriptContext,strval);
			Parser_scanStringValueToMem_B(any, FIELDTYPE_SFImage, strp, FALSE);
			JS_free(scriptContext,strpp);
			break;
		}

		case FIELDTYPE_SFString: {
			struct Uni_String *ms;
			intptr_t *newptr;

			strval = JS_ValueToString(scriptContext, *(jsval*)(tg->JScript.JSglobal_return_val));
			strp = strpp = JS_EncodeString(scriptContext,strval);

			/* copy the string over, delete the old one, if need be */
			/* printf ("fieldSet SFString, tn %d tptr %d offset from struct %d\n",
				tn, tptr, offsetof (struct X3D_TextureCoordinateGenerator, mode)); */
			newptr = (intptr_t *)memptr;
			ms = (struct Uni_String*) *newptr;
			if(!ms) {
				ms = (struct Uni_String*)malloc(sizeof(struct Uni_String));
				*newptr = (intptr_t) ms;
				ms->len = 0;
				ms->strptr = NULL;
				ms->touched = FALSE;
			}
			verify_Uni_String (ms,strp);
			JS_free(scriptContext,strpp);
			break;
		}


			/* a series of Floats... */
		case FIELDTYPE_MFFloat: {getJSMultiNumType (scriptContext, (struct Multi_Vec3f *)memptr,FIELDTYPE_SFFloat); break;}
		case FIELDTYPE_MFInt32: {getJSMultiNumType (scriptContext, (struct Multi_Vec3f *)memptr,FIELDTYPE_SFInt32); break;}
		case FIELDTYPE_MFTime: {getJSMultiNumType (scriptContext, (struct Multi_Vec3f *)memptr,FIELDTYPE_SFTime); break;}
		case FIELDTYPE_MFDouble: {getJSMultiNumType (scriptContext, (struct Multi_Vec3f *)memptr,FIELDTYPE_SFDouble); break;}
		case FIELDTYPE_MFNode: {
			struct X3D_Node *mynode;

			strval = JS_ValueToString(scriptContext, *(jsval*)(tg->JScript.JSglobal_return_val));
			strp = strpp = JS_EncodeString(scriptContext,strval);
			/* we will have at least one node here, in an ascii string */
			while ((*strp > '\0') && (*strp <= ' ')) strp ++;
			/* are we at a bracket? */
			if (*strp == '[') strp ++;
			while ((*strp > '\0') && (*strp <= ' ')) strp ++;

			/* printf ("convertingthe following string to a pointer :%s:\n",strp); */

#ifndef _x64
			mynode = X3D_NODE(atol(strp));
#else
			mynode = X3D_NODE(atoll(strp));
#endif
			JS_free(scriptContext,strpp);

			/* printf ("mynode is %p %d, \n",mynode,mynode);
			printf ("mynode is %p %d, type %d\n",mynode,mynode,mynode->_nodeType);
			printf ("calling getMFNodeType now\n"); */


			//getMFNodetype (mynode,(struct Multi_Node *)memptr,X3D_NODE(tn),extraData);
			any->mfnode.n = 1;
			any->mfnode.p = MALLOC(struct X3D_Node **, sizeof(struct X3D_Node *));
			any->mfnode.p[0] = mynode;
			//Q. can we do add/remove children outside?
			break;
		}
		case FIELDTYPE_MFString: {
			getMFStringtype (
				scriptContext,
				(jsval *)tg->JScript.JSglobal_return_val,
				(struct Multi_String *)memptr);
			break;
		}

		case FIELDTYPE_SFNode: {
			//unsigned int valuecopied;
			//unsigned int *ptr2value;
				/* printf ("doing TYPEA memcpy to %u, from %u, len %d\n",(void *)memptr, (void *) &(((SFNodeNative *)JSSFpointer)->handle),returnElementLength(FIELDTYPE_SFNode));*/
			memcpy ((void *)memptr, (void *) &(((SFNodeNative *)tg->CRoutes.JSSFpointer)->handle),returnElementLength(FIELDTYPE_SFNode));
			//ptr2value = (unsigned int*)memptr;
			//valuecopied = *ptr2value;
			//printf("value of memptr %u after memcpy in script route= %u\n",(void*)memptr,valuecopied);
			break;
		}
		default: {	printf("WARNING: unhandled from type %s\n", stringFieldtypeType(fieldType));
		}
	}

}

extern "C" {
void sm_js_setField_javascriptEventOut(struct X3D_Node *tn,unsigned int tptr,  int fieldType, unsigned len, int extraData, int actualscript) {
	struct CRscriptStruct *scriptcontrol;
	JSContext *cx;
	JSObject *global;
	scriptcontrol = getScriptControlIndex(actualscript);
	cx = (JSContext*)scriptcontrol->cx;
	global = (JSObject*)scriptcontrol->glob;
	{ // Scope A  for our various stack objects (JSAutoRequest, RootedObject), so they all go
		// out of scope before we JS_DestroyContext.
		JSAutoRequest ar(cx); // In practice, you would want to exit this any
							// time you're spinning the event loop
		{ // Scope B  for JSAutoCompartment
			JSAutoCompartment ac(cx, global);
			setField_javascriptEventOut(tn,tptr,fieldType, len, extraData, (JSContext*)scriptcontrol->cx);
		} //Scope B
	} //Scope A
}

void sm_js_setField_javascriptEventOut_B(union anyVrml* any, int fieldType, unsigned len, int extraData, int actualscript){
	struct CRscriptStruct *scriptcontrol;
	JSContext *cx;
	JSObject *global;
	scriptcontrol = getScriptControlIndex(actualscript);
	cx = (JSContext*)scriptcontrol->cx;
	global = (JSObject*)scriptcontrol->glob;
	{ // Scope A  for our various stack objects (JSAutoRequest, RootedObject), so they all go
		// out of scope before we JS_DestroyContext.
		JSAutoRequest ar(cx); // In practice, you would want to exit this any
							// time you're spinning the event loop
		{ // Scope B  for JSAutoCompartment
			JSAutoCompartment ac(cx, global);
			setField_javascriptEventOut_B(any,fieldType, len, extraData, (JSContext*)scriptcontrol->cx);
		} //Scope B
	} //Scope A
}


/******************************************************************************/

void sm_set_one_ECMAtype (int tonode, int toname, int dataType, void *Data, int datalen) {
	char scriptline[256];
	jsval newval;
	JSContext *cx;
	JSObject *obj;
	int kind;
	struct CRscriptStruct *ScriptControl; // = getScriptControl();
	struct CRjsnameStruct *JSparamnames = getJSparamnames();

	#ifdef SETFIELDVERBOSE
	printf ("set_one_ECMAtype, to %d namepointer %d, fieldname %s, datatype %d length %d\n",
		tonode,toname,JSparamnames[toname].name,dataType,datalen);
	#endif
	ScriptControl = getScriptControlIndex(tonode);
	/* get context and global object for this script */
	cx =  (JSContext*)ScriptControl->cx;
	obj = (JSObject*)ScriptControl->glob;

	{ // Scope for our various stack objects (JSAutoRequest, RootedObject), so they all go
		// out of scope before we JS_DestroyContext.

		JSAutoRequest ar(cx); // In practice, you would want to exit this any
							// time you're spinning the event loop
		{ // Scope for JSAutoCompartment
			JSAutoCompartment ac(cx, obj);

			/* set the time for this script */
			//SET_JS_TICKTIME
			{ 
				jsval zimbo; 
				JS_NewNumberValue(cx, TickTime(), &zimbo);
				if (!JS_DefineProperty(cx,obj, "__eventInTickTime", zimbo, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB2, JSPROP_PERMANENT)) {
						printf( "JS_DefineProperty failed for \"__eventInTickTime\" at %s:%d.\n",__FILE__,__LINE__);
						return;
				}
			}

			//step 1 set the field value
			kind = JSparamnames[toname].kind; //PKW_inputOnly;
			if(SM_method() == 2){
				int type, iifield, *valueChanged, ifound;
				union anyVrml *value;
				char *fieldname;
				struct Shader_Script *script = ScriptControl->script;
				fieldname = JSparamnames[toname].name;
				//step 1 update the fieldvalue
				ifound = getFieldFromScript(script,fieldname,&type,&kind,&iifield,&value,&valueChanged);
				if(ifound && type == dataType && isSFType(type)){
					//we have an MF field, and mf coming in, we'll call our field LHS and incoming RHS
					union anyVrml *any = (union anyVrml*)Data;
					//printf("any float=%f",any->sffloat);
					shallow_copy_field(type,any,value);
					//if we have an inputOutput field with no eventIn function, we may still be routing
					//from the out side
					(*valueChanged) = 1;
				}else{
					ConsoleMessage("sm_set_one_ECMAtype did not find field %s type %d\n",fieldname, dataType);
					return;
				}

			}else{ //SM_method == 2
				X3D_ECMA_TO_JS(cx, Data, datalen, dataType, &newval);

				/* get the variable name to hold the incoming value */
				scriptline[0] = 0;
				if(kind == PKW_inputOnly)
					strcat(scriptline,"__eventIn_Value_");
				//sprintf (scriptline,"__eventIn_Value_%s", JSparamnames[toname].name);
				strcat(scriptline,JSparamnames[toname].name);
				#ifdef SETFIELDVERBOSE
				printf ("set_one_ECMAtype, calling JS_DefineProperty on name %s obj %u, setting setECMANative, 0 \n",scriptline,obj);
				#endif

				if (!JS_DefineProperty(cx,obj, scriptline, newval, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB3, JSPROP_PERMANENT)) {
					printf( "JS_DefineProperty failed for \"ECMA in\" at %s:%d.\n",__FILE__,__LINE__);
					return;
				}

			} //SM_method == 2

			//step 2 run eventin if it exists
			/* is the function compiled yet? */
			COMPILE_FUNCTION_IF_NEEDED_SET(toname,kind)

			/* and run the function */
			RUN_FUNCTION (toname)

		} //Scope B

	} //Scope A


}

/*  setScriptECMAtype called by getField_ToJavascript for
        case FIELDTYPE_SFBool:
        case FIELDTYPE_SFFloat:
        case FIELDTYPE_SFTime:
        case FIELDTYPE_SFDouble:
        case FIELDTYPE_SFInt32:
        case FIELDTYPE_SFString:
*/

void sm_setScriptECMAtype (int num) {
	void *fn;
	int tptr;
	int len;
	int to_counter;
	CRnodeStruct *to_ptr = NULL;
	struct CRStruct *CRoutes = getCRoutes();
	struct CRjsnameStruct *JSparamnames = getJSparamnames();

	fn = offsetPointer_deref(void *, CRoutes[num].routeFromNode, CRoutes[num].fnptr);
	len = CRoutes[num].len;

	for (to_counter = 0; to_counter < CRoutes[num].tonode_count; to_counter++) {
		struct Shader_Script *myObj;

		to_ptr = &(CRoutes[num].tonodes[to_counter]);
		myObj = (struct Shader_Script*)X3D_SCRIPT(to_ptr->routeToNode)->__scriptObj;
		/* printf ("setScriptECMAtype, myScriptNumber is %d\n",myObj->num); */
		tptr = to_ptr->foffset;
		set_one_ECMAtype (myObj->num, tptr, JSparamnames[tptr].type, fn,len);
	}
}


/* use Javascript to send in one element of an MF. datalen is in number of elements in type. */
void sm_set_one_MFElementType(int tonode, int toname, int dataType, void *Data, int datalen) {
	JSContext *cx;
	JSObject *obj;
	int elementlen;
	int x;
	char scriptline[20000];

	/* for PixelTextures we have: */
	//struct X3D_PixelTexture *mePix;
	//struct Multi_Int32 image;

	/* for MFStrings we have: */
	char *chptr;
	struct Uni_String  **uniptr;
	int kind;
	struct CRscriptStruct *ScriptControl; // = getScriptControl();
	struct CRjsnameStruct *JSparamnames = getJSparamnames();

	/* get context and global object for this script */
	ScriptControl = getScriptControlIndex(tonode);
	cx =  (JSContext*)ScriptControl->cx;
	obj = (JSObject*)ScriptControl->glob;

	{ // Scope A  for our various stack objects (JSAutoRequest, RootedObject), so they all go
		// out of scope before we JS_DestroyContext.
		JSAutoRequest ar(cx); // In practice, you would want to exit this any
							// time you're spinning the event loop
		{ // Scope B  for JSAutoCompartment
			JSAutoCompartment ac(cx, obj);

			/* set the TickTime (possibly again) for this context */
			kind = JSparamnames[toname].kind; //PKW_inputOnly;
			if(SM_method() == 2){
				int type, iifield, *valueChanged, ifound;
				union anyVrml *value;
				char *fieldname;
				struct Shader_Script *script = ScriptControl->script;
				fieldname = JSparamnames[toname].name;
				//step 1 update the fieldvalue
				ifound = getFieldFromScript(script,fieldname,&type,&kind,&iifield,&value,&valueChanged);
				if(ifound && type == dataType && !isSFType(type)){
					//we have an MF field, and mf coming in, we'll call our field LHS and incoming RHS
					union anyVrml any;
					any.mfbool.n = datalen;
					any.mfbool.p = (int *)Data;
					//printf("address of any.p %x value.p %x",any.mfbool.p,value->mfbool.p);
					//printf("any.n=%d \n",any.mffloat.n);
					//printf("mfany= %f %f %f",any.mffloat.p[0],any.mffloat.p[1],any.mffloat.p[2]);
					//printf("target value.n= %d\n",value->mfbool.n);
					shallow_copy_field(type,&any,value);
					//printf("after shallow_copy_field:\n");
					//printf("target value.n= %d\n",value->mfbool.n);
					//printf("mfvalue= %f %f %f",value->mffloat.p[0],value->mffloat.p[1],value->mffloat.p[2]);

					//if we have an inputOutput field with no eventIn function, we may still be routing
					//from the out side
					(*valueChanged) = 1;
				}else{
					ConsoleMessage("sm_set_one_MFElementType did not find field %s type %d\n",fieldname, dataType);
					return;
				}
				//step 2 run the eventIn if it exists
				SET_JS_TICKTIME
				//compile also pushes the field val onto call stack
				COMPILE_FUNCTION_IF_NEEDED_SET(toname,kind)
				RUN_FUNCTION(toname)
				return;
			}
			SET_JS_TICKTIME

			/* make up the name */
			switch (dataType) {
				case FIELDTYPE_MFRotation: {
					JSObject *newMFObject;
					JSObject *newSFObject;
					SFRotationNative 	*SFRPptr;
					float *fp, *fp_in=(float *)Data;

					/* create a new MFRotation object... */
					newMFObject = JS_ConstructObjectFw(cx, &MFRotationClass, NULL ,JS_GetParentFw(cx, obj));
					ADD_ROOT (cx, newMFObject)

					/* define the "length" property for this object */
					DEFINE_LENGTH_NORV(cx,newMFObject,datalen)

					/* fill in private pointer area */
					elementlen = (int) sizeof (float);
					for (x=0; x<datalen; x++) {
						/* create a new SFRotation object */
						newSFObject = JS_ConstructObjectFw(cx,&SFRotationClass,NULL, newMFObject);
						if ((SFRPptr = (SFRotationNative *)JS_GetPrivateFw(cx, newSFObject)) == NULL) {
							ConsoleMessage ("failure in getting SF class at %s:%d\n",__FILE__,__LINE__);
							return;
						}

						/* fill the private pointer area */
						fp = (float *)fp_in; SFRPptr->v.c[0] = *fp; fp_in = offsetPointer_deref(float *,fp_in,elementlen);
						fp = (float *)fp_in; SFRPptr->v.c[1] = *fp; fp_in = offsetPointer_deref(float *,fp_in,elementlen);
						fp = (float *)fp_in; SFRPptr->v.c[2] = *fp; fp_in = offsetPointer_deref(float *,fp_in,elementlen);
						fp = (float *)fp_in; SFRPptr->v.c[3] = *fp; fp_in = offsetPointer_deref(float *,fp_in,elementlen);

						/* put this object into the MF class */
						if (!JS_DefineElement(cx, newMFObject, (jsint) x, OBJECT_TO_JSVAL(newSFObject),
							JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB3, JSPROP_ENUMERATE)) {
								printf("failure in inserting SF class at %s:%d\n",__FILE__,__LINE__);
						}
					}

					/* set the length of this MF */
					SET_LENGTH (cx,newMFObject,datalen)

					/* set the obj variable with this new MF object */
					SET_EVENTIN_VALUE (cx,obj,toname,newMFObject)

					/* run the function */
					COMPILE_FUNCTION_IF_NEEDED_SET(toname,kind)
					RUN_FUNCTION(toname)
					break;
				}

				case FIELDTYPE_MFVec3f: {
					JSObject *newMFObject;
					JSObject *newSFObject;
					SFVec3fNative 	*SFRPptr;
					float *fp, *fp_in=(float *)Data;

					/* create a new MFVec3f object... */
					newMFObject = JS_ConstructObjectFw(cx, &MFVec3fClass, NULL ,JS_GetParentFw(cx, obj));
					ADD_ROOT (cx, newMFObject)

					/* define the "length" property for this object */
					DEFINE_LENGTH_NORV(cx,newMFObject,datalen)

					/* fill in private pointer area */
					elementlen = (int) sizeof (float);
					for (x=0; x<datalen; x++) {
						/* create a new SFVec3f object */
						newSFObject = JS_ConstructObjectFw(cx,&SFVec3fClass,NULL, newMFObject);
						if ((SFRPptr = (SFVec3fNative *)JS_GetPrivateFw(cx, newSFObject)) == NULL) {
							 ConsoleMessage ("failure in getting SF class at %s:%d\n",__FILE__,__LINE__);
							return;
						}

						/* fill the private pointer area */
						fp = (float *)fp_in; SFRPptr->v.c[0] = *fp; fp_in = offsetPointer_deref(float *,fp_in,elementlen);
						fp = (float *)fp_in; SFRPptr->v.c[1] = *fp; fp_in = offsetPointer_deref(float *,fp_in,elementlen);
						fp = (float *)fp_in; SFRPptr->v.c[2] = *fp; fp_in = offsetPointer_deref(float *,fp_in,elementlen);

						/* put this object into the MF class */
						if (!JS_DefineElement(cx, newMFObject, (jsint) x, OBJECT_TO_JSVAL(newSFObject),
							JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB3, JSPROP_ENUMERATE)) {
								printf("failure in inserting SF class at %s:%d\n",__FILE__,__LINE__);
						}
					}

					/* set the length of this MF */
					SET_LENGTH (cx,newMFObject,datalen)

					/* set the global variable with this new MF object */
					SET_EVENTIN_VALUE (cx,obj,toname,newMFObject)

					/* run the function */
					COMPILE_FUNCTION_IF_NEEDED_SET(toname,kind)
					RUN_FUNCTION(toname)
					break;
				}

				case FIELDTYPE_MFColor: {
					JSObject *newMFObject;
					JSObject *newSFObject;
					SFColorNative 	*SFRPptr;
					float *fp, *fp_in=(float *)Data;

					/* create a new MFColor object... */
					newMFObject = JS_ConstructObjectFw(cx, &MFColorClass, NULL ,JS_GetParentFw(cx, obj));
					ADD_ROOT (cx, newMFObject)

					/* define the "length" property for this object */
					DEFINE_LENGTH_NORV(cx,newMFObject,datalen)

					/* fill in private pointer area */
					elementlen = (int) sizeof (float);
					for (x=0; x<datalen; x++) {
						/* create a new SFColor object */
						newSFObject = JS_ConstructObjectFw(cx,&SFColorClass,NULL, newMFObject);
						if ((SFRPptr = (SFColorNative *)JS_GetPrivateFw(cx, newSFObject)) == NULL) {
							ConsoleMessage ("failure in getting SF class at %s:%d\n",__FILE__,__LINE__);
							return;
						}

						/* fill the private pointer area */
						fp = (float *)fp_in; SFRPptr->v.c[0] = *fp; fp_in = offsetPointer_deref(float *,fp_in,elementlen);
						fp = (float *)fp_in; SFRPptr->v.c[1] = *fp; fp_in = offsetPointer_deref(float *,fp_in,elementlen);
						fp = (float *)fp_in; SFRPptr->v.c[2] = *fp; fp_in = offsetPointer_deref(float *,fp_in,elementlen);

						/* put this object into the MF class */
						if (!JS_DefineElement(cx, newMFObject, (jsint) x, OBJECT_TO_JSVAL(newSFObject),
							JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB3, JSPROP_ENUMERATE)) {
								printf("failure in inserting SF class at %s:%d\n",__FILE__,__LINE__);
						}
					}

					/* set the length of this MF */
					SET_LENGTH (cx,newMFObject,datalen)

					/* set the global variable with this new MF object */
					SET_EVENTIN_VALUE (cx,obj,toname,newMFObject)

					/* run the function */
					COMPILE_FUNCTION_IF_NEEDED_SET(toname,kind)
					RUN_FUNCTION(toname)
					break;
				}

				case FIELDTYPE_MFVec2f: {
					JSObject *newMFObject;
					JSObject *newSFObject;
					SFVec2fNative 	*SFRPptr;
					float *fp, *fp_in=(float *)Data;

					/* create a new MFVec2f object... */
					newMFObject = JS_ConstructObjectFw(cx, &MFVec2fClass, NULL ,JS_GetParentFw(cx, obj));
					ADD_ROOT (cx, newMFObject)

					/* define the "length" property for this object */
					DEFINE_LENGTH_NORV(cx,newMFObject,datalen)

					/* fill in private pointer area */
					elementlen = (int) sizeof (float);
					for (x=0; x<datalen; x++) {
						/* create a new SFVec2f object */
						newSFObject = JS_ConstructObjectFw(cx,&SFVec2fClass,NULL, newMFObject);
						if ((SFRPptr = (SFVec2fNative *)JS_GetPrivateFw(cx, newSFObject)) == NULL) {
							ConsoleMessage ("failure in getting SF class at %s:%d\n",__FILE__,__LINE__);
							return;
						}

						/* fill the private pointer area */
						fp = (float *)fp_in; SFRPptr->v.c[0] = *fp; fp_in = offsetPointer_deref(float *,fp_in,elementlen);
						fp = (float *)fp_in; SFRPptr->v.c[1] = *fp; fp_in = offsetPointer_deref(float *,fp_in,elementlen);

						/* put this object into the MF class */
						if (!JS_DefineElement(cx, newMFObject, (jsint) x, OBJECT_TO_JSVAL(newSFObject),
							JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB3, JSPROP_ENUMERATE)) {
								ConsoleMessage("failure in inserting SF class at %s:%d\n",__FILE__,__LINE__);
						}
					}

					/* set the length of this MF */
					SET_LENGTH (cx,newMFObject,datalen)

					/* set the global variable with this new MF object */
					SET_EVENTIN_VALUE (cx,obj,toname,newMFObject)

					/* run the function */
					COMPILE_FUNCTION_IF_NEEDED_SET(toname,kind)
					RUN_FUNCTION(toname)
					break;
				}


				case FIELDTYPE_MFFloat: {
					JSObject *newMFObject;
					jsval newjsval;
					float *fp, *fp_in=(float *)Data;
					/* create a new MFFloat object... */
					newMFObject = JS_ConstructObjectFw(cx, &MFFloatClass, NULL ,JS_GetParentFw(cx, obj));
					ADD_ROOT (cx, newMFObject)

					/* define the "length" property for this object */
					DEFINE_LENGTH_NORV(cx,newMFObject,datalen)

					/* fill in private pointer area */
					elementlen = (int) sizeof (float);
					for (x=0; x<datalen; x++) {
						/* create a new SFFloat object */

						fp = (float *)fp_in;
						JS_NewNumberValue(cx,(double)*fp,&newjsval);
						fp_in = offsetPointer_deref(float *,fp_in,elementlen);

						/* put this object into the MF class */
						if (!JS_DefineElement(cx, newMFObject, (jsint) x, newjsval,
							JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB3, JSPROP_ENUMERATE)) {
								printf("failure in inserting SF class at %s:%d\n",__FILE__,__LINE__);
						}
					}

					/* set the length of this MF */
					SET_LENGTH (cx,newMFObject,datalen)

					/* set the global variable with this new MF object */
					SET_EVENTIN_VALUE (cx,obj,toname,newMFObject)

					/* run the function */
					COMPILE_FUNCTION_IF_NEEDED_SET(toname,kind)
					RUN_FUNCTION(toname)
					break;
				}
				case FIELDTYPE_MFTime: {
					JSObject *newMFObject;
					jsval newjsval;
					double *dp, *dp_in=(double *)Data;

					/* create a new MFTime object... */
					newMFObject = JS_ConstructObjectFw(cx, &MFTimeClass, NULL ,JS_GetParentFw(cx, obj));
					ADD_ROOT (cx, newMFObject)

					/* define the "length" property for this object */
					DEFINE_LENGTH_NORV(cx,newMFObject,datalen)

					/* fill in private pointer area */
					elementlen = (int) sizeof (double);
					for (x=0; x<datalen; x++) {
						/* create a new SFTime object */

						dp = (double *)dp_in;
						JS_NewNumberValue(cx,(double)*dp,&newjsval);
						dp_in = offsetPointer_deref(double *,dp_in,elementlen);

						/* put this object into the MF class */
						if (!JS_DefineElement(cx, newMFObject, (jsint) x, newjsval,
							JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB3, JSPROP_ENUMERATE)) {
								printf("failure in inserting SF class at %s:%d\n",__FILE__,__LINE__);
						}
					}

					/* set the length of this MF */
					SET_LENGTH (cx,newMFObject,datalen)

					/* set the global variable with this new MF object */
					SET_EVENTIN_VALUE (cx,obj,toname,newMFObject)

					/* run the function */
					COMPILE_FUNCTION_IF_NEEDED_SET(toname,kind)
					RUN_FUNCTION(toname)
					break;
				}
				case FIELDTYPE_MFInt32: {
					JSObject *newMFObject;
					jsval newjsval;
					int *ip, *ip_in=(int *)Data;

					/* create a new MFInt32 object... */
					newMFObject = JS_ConstructObjectFw(cx, &MFInt32Class, NULL ,JS_GetParentFw(cx, obj));
					ADD_ROOT (cx, newMFObject)

					/* define the "length" property for this object */
					DEFINE_LENGTH_NORV(cx,newMFObject,datalen)

					/* fill in private pointer area */
					elementlen = (int) sizeof (float);
					for (x=0; x<datalen; x++) {
						/* create a new SFInt32 object */

						ip = (int *)ip_in;
						newjsval = INT_TO_JSVAL((int)*ip);
						ip_in = offsetPointer_deref(int *,ip_in,elementlen);

						/* put this object into the MF class */
						if (!JS_DefineElement(cx, newMFObject, (jsint) x, newjsval,
							JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB3, JSPROP_ENUMERATE)) {
								printf("failure in inserting SF class at %s:%d\n",__FILE__,__LINE__);
						}
					}

					/* set the length of this MF */
					SET_LENGTH (cx,newMFObject,datalen)

					/* set the global variable with this new MF object */
					SET_EVENTIN_VALUE (cx,obj,toname,newMFObject)

					/* run the function */
					COMPILE_FUNCTION_IF_NEEDED_SET(toname,kind)
					RUN_FUNCTION(toname)
					break;
				}
				case FIELDTYPE_MFString: {
					JSObject *newMFObject;
					jsval newjsval;
					struct Uni_String * *ip_in=(struct Uni_String **)Data;

					/* create a new MFString object... */
					newMFObject = JS_ConstructObjectFw(cx, &MFStringClass, NULL ,JS_GetParentFw(cx, obj));
					ADD_ROOT (cx, newMFObject)

					/* Data points to a Uni_String */
					uniptr = (struct Uni_String **) ip_in;

					/* define the "length" property for this object */
					DEFINE_LENGTH_NORV(cx,newMFObject,datalen)

					/* fill in private pointer area */
					for (x=0; x<datalen; x++) {
						/* create a new SFString object */

						chptr = uniptr[x]->strptr;
						newjsval = STRING_TO_JSVAL( JS_NewStringCopyZ(cx,chptr));

						/* put this object into the MF class */
						if (!JS_DefineElement(cx, newMFObject, (jsint) x, newjsval,
							JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB3, JSPROP_ENUMERATE)) {
								printf("failure in inserting SF class at %s:%d\n",__FILE__,__LINE__);
						}
					}

					/* set the length of this MF */
					SET_LENGTH (cx,newMFObject,datalen)

					/* set the global variable with this new MF object */
					SET_EVENTIN_VALUE (cx,obj,toname,newMFObject)

					/* run the function */
					COMPILE_FUNCTION_IF_NEEDED_SET(toname,kind)
					RUN_FUNCTION(toname)
					break;
				}
				case FIELDTYPE_MFNode: {
					JSObject *newMFObject;
					jsval newjsval;
					void **ip, **ip_in=(void **)Data;
					/* create a new MFNode object... */
					newMFObject = JS_ConstructObjectFw(cx, &MFNodeClass, NULL ,JS_GetParentFw(cx, obj));
					ADD_ROOT (cx, newMFObject)

					/* define the "length" property for this object */
					DEFINE_LENGTH_NORV(cx,newMFObject,datalen)

					/* fill in private pointer area */
					elementlen = (int) sizeof (void *);
					for (x=0; x<datalen; x++) {
						ip = ip_in;
						newjsval = PRIVATE_TO_JSVAL((void*)*ip); //July 4, 2018 SM1 routing MFNode doesn't work
						ip_in = offsetPointer_deref(void **,ip_in,elementlen);

						/* put this object into the MF class */
						if (!JS_DefineElement(cx, newMFObject, (jsint) x, newjsval,
							JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB3, JSPROP_ENUMERATE)) {
								printf("failure in inserting SF class at %s:%d\n",__FILE__,__LINE__);
						}
					}

					/* set the length of this MF */
					SET_LENGTH (cx,newMFObject,datalen)

					/* set the global variable with this new MF object */
					SET_EVENTIN_VALUE (cx,obj,toname,newMFObject)

					/* run the function */
					COMPILE_FUNCTION_IF_NEEDED_SET(toname,kind)
					RUN_FUNCTION(toname)
					break;
				}

				case FIELDTYPE_SFImage:	{
					JSObject *newMFObject;
					jsval newjsval;
					int *ip_in=(int *)Data;

					/* create a new MFNode object... */
					newMFObject = JS_ConstructObjectFw(cx, &SFImageClass, NULL ,JS_GetParentFw(cx, obj));
					ADD_ROOT (cx, newMFObject)

					/* define the "length" property for this object */
					DEFINE_LENGTH_NORV(cx,newMFObject,datalen)

					/* fill in private pointer area */
					for (x=0; x<datalen; x++) {
						newjsval = INT_TO_JSVAL(ip_in[x]);
						/* put this object into the MF class */
						if (!JS_DefineElement(cx, newMFObject, (jsint) x, newjsval,
											  JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB3, JSPROP_ENUMERATE)) {
							printf("failure in inserting SF class at %s:%d\n",__FILE__,__LINE__);
						}
					}

					/* set the length of this MF */
					SET_LENGTH (cx,newMFObject,datalen)

					/* set the global variable with this new MF object */
					SET_EVENTIN_VALUE (cx,obj,toname,newMFObject)

					/* run the function */
					COMPILE_FUNCTION_IF_NEEDED_SET(toname,kind)
					RUN_FUNCTION(toname)

					break;
					}

				default: {
						printf ("setMFElement, SHOULD NOT DISPLAY THIS\n");
						strcat (scriptline,"(");
					}
			}
		} //Scope B
	} //Scope A

}

} //extern "C"

/****************************************************************/
/* sets a SFVec3f and SFColor and SFVec3d 			*/
/* and SFRotation and SFVec2fin a script 			*/
/*								*/
/* all *Native types have the same structure of the struct -	*/
/* we are just looking for the pointer, thus we can handle	*/
/* multi types here 						*/
/* sets a SFVec3f and SFColor in a script 			*/
/****************************************************************/

/* get a pointer to the internal data for this object, or return NULL on error */
void **getInternalDataPointerForJavascriptObject(JSContext *cx, JSObject *obj, int tnfield, int *iflag) {
	char scriptline[256];
	void *_privPtr;
	JSObject *sfObj;
	jsval retval;
	struct CRjsnameStruct *JSparamnames = getJSparamnames();

	/* get the variable name to hold the incoming value */
	//sprintf (scriptline,"__eventIn_Value_%s", JSparamnames[tnfield].name);
	scriptline[0] = 0;
	if(JSparamnames[tnfield].kind == PKW_inputOnly)
		strcat(scriptline,"__eventIn_Value_");
	strcat(scriptline,JSparamnames[tnfield].name);
	#ifdef SETFIELDVERBOSE
	printf ("getInternalDataPointerForJavascriptObject: line %s\n",scriptline);
	#endif

	if (!JS_GetProperty(cx,obj,scriptline,&retval)){
		//if you forgot to put both an inputOnly or inputOutput field AND
		// you forgot to define a function() with the same name
		// then you won't have any objects
		// if you didn't have a field then ROUTE would complain, unless directAccess
		printf ("JS_GetProperty failed in set_one_MultiElementType.\n");
		//return NULL;
	}
	*iflag = 1;
	//if (!JSVAL_IS_OBJECT(retval)){
	if(!retval.isObject()){
		// you don't have an inputOnly static object with this name (but might have a field static object,
		//return NULL;
		//could be inputOutput which has a norma field name
		*iflag = 0;
		if (!JS_GetProperty(cx,obj,JSparamnames[tnfield].name,&retval)){
			//you may have an inputOutput field, with the plane name
			printf ("no field for eventIn function: %s\n",JSparamnames[tnfield].name);
			return NULL;
		}
		//if (!JSVAL_IS_OBJECT(retval)){
		if(!retval.isObject()){
			printf ("no field for eventIn function: %s\n",JSparamnames[tnfield].name);
			return NULL;
		}
		*iflag = 2;
	}

	sfObj = JSVAL_TO_OBJECT(retval);

	if ((_privPtr = JS_GetPrivateFw(cx, sfObj)) == NULL)
		printf("JS_GetPrivate failed set_one_MultiElementType.\n");

	if (_privPtr == NULL) return NULL;

	/* what kind of class of object is this? */

	/* we look at EVERY kind of native class found in "jsNative.h" even
	   if it may not be ever used here */

        if (JS_InstanceOf(cx, sfObj, &SFVec3fClass, NULL)) {
		SFVec3fNative *me = (SFVec3fNative *)_privPtr;
		return (void **) &me->v;

        } else if (JS_InstanceOf(cx, sfObj, &SFVec3dClass, NULL)) {
		SFVec3dNative *me = (SFVec3dNative *)_privPtr;
		return (void **) &me->v;

        } else if (JS_InstanceOf(cx, sfObj, &SFRotationClass, NULL)) {
		SFRotationNative *me = (SFRotationNative *)_privPtr;
		return (void **) &me->v;

        } else if (JS_InstanceOf(cx, sfObj, &SFVec2fClass, NULL)) {
		SFVec2fNative *me = (SFVec2fNative *)_privPtr;
		return (void **) &me->v;

        } else if (JS_InstanceOf(cx, sfObj, &SFColorClass, NULL)) {
		SFColorNative *me = (SFColorNative *)_privPtr;
		return (void **) &me->v;

        } else if (JS_InstanceOf(cx, sfObj, &SFColorRGBAClass, NULL)) {
		SFColorRGBANative *me = (SFColorRGBANative *)_privPtr;
		return (void **) &me->v;

        } else if (JS_InstanceOf(cx, sfObj, &SFVec4fClass, NULL)) {
		SFVec4fNative *me = (SFVec4fNative *)_privPtr;
		return (void **) &me->v;

        } else if (JS_InstanceOf(cx, sfObj, &SFVec4dClass, NULL)) {
		SFVec4dNative *me = (SFVec4dNative *)_privPtr;
		return (void **) &me->v;

        } else if (JS_InstanceOf(cx, sfObj, &SFNodeClass, NULL)) {
		SFNodeNative *me = (SFNodeNative *)_privPtr;
		return (void **) &me->handle;;
		//JAS return (void **) &me->v;

        } else if (JS_InstanceOf(cx, sfObj, &SFImageClass, NULL)) {
		//SFImageNative *me = (SFImageNative *)_privPtr;
		//JAS return (void **) &me->v;

        }

	ConsoleMessage ("getInternalDataPointerForJavascriptObject malfunction");

	return NULL;
}



/* really do the individual set; used by script routing and EAI sending to a script
	Dec 2017 - You may have a inpoutOutput field you want to route values to
		and not have any inputOnly function() associated with the field
		for this scenario you want to check first if there's a function,
		and if so do some extra work. If not so be it.
	set_one_multielementtype is for SFVecxx, SFColorxxxx, SFNode, SFRotation
*/
extern "C" {
void sm_set_one_MultiElementType (int tonode, int tnfield, void *Data, int dataLen ) {
	char scriptline[256];
	JSContext *cx;
	JSObject *obj;
	void **pp;
	int iflag, kind, toname;
	struct CRscriptStruct *ScriptControl; // = getScriptControl();
	struct CRjsnameStruct *JSparamnames = getJSparamnames();

	/* get context and global object for this script */
	ScriptControl = getScriptControlIndex(tonode);
	cx =  (JSContext*)ScriptControl->cx;
	obj = (JSObject*)ScriptControl->glob;

	{ // Scope A for our various stack objects (JSAutoRequest, RootedObject), so they all go
		// out of scope before we JS_DestroyContext.
		JSAutoRequest ar(cx); // In practice, you would want to exit this any
							// time you're spinning the event loop
		{ // Scope B for JSAutoCompartment
			JSAutoCompartment ac(cx, obj);
			toname = tnfield;
			kind =  JSparamnames[toname].kind; // PKW_inputOnly;
			if(SM_method() == 2){
				int type, iifield, *valueChanged, ifound, datatype;
				union anyVrml *value;
				char *fieldname;
				struct Shader_Script *script = ScriptControl->script;

				fieldname = JSparamnames[toname].name;
				datatype = JSparamnames[toname].type;

				//step 1 update the fieldvalue
				ifound = getFieldFromScript(script,fieldname,&type,&kind,&iifield,&value,&valueChanged);
				if(ifound && type == datatype && isSFType(type)){
					//we have an SF field, and sf coming in, we'll call our field LHS and incoming RHS
					shallow_copy_field(type,(union anyVrml*)Data,value);
					(*valueChanged) = 1;
				}else{
					ConsoleMessage("sm_set_one_MultiElementType did not find field %s type %d\n",fieldname, datatype);
					return;
				}
				//step 2 run the eventIn if it exists
				SET_JS_TICKTIME
				//compile also pushes the field val onto call stack
				COMPILE_FUNCTION_IF_NEEDED_SET(toname,kind)
				RUN_FUNCTION(toname)
				return;
			}
			/* copy over the data from the VRML side into the script variable. */
			iflag = 0;
			pp = getInternalDataPointerForJavascriptObject(cx,obj,tnfield,&iflag);
			if(pp == NULL){
				//no script function with this name - you might be routing to an inputOutput field
				printf("function not found\n");
				return;
			}
			memcpy (pp,Data, dataLen);
			/* printf ("set_one_MultiElementType, dataLen %d, sizeof(double) %d\n",dataLen, sizeof(double));
			printf ("and, sending the data to pointer %p\n",pp); */

			//if we added a __eventIn_Value_<fieldname> for inputOnly field
			/* set the time for this script */
			SET_JS_TICKTIME
			/* is the function compiled yet? */
			COMPILE_FUNCTION_IF_NEEDED_SET(tnfield,kind)

			RUN_FUNCTION (tnfield)
		} // Scope B
	} // Scope A

}

int sm_runQueuedDirectOutputs(){
	//stub for SM and STUBS (DUK has it)
	static int doneOnce = 0;
	if(!doneOnce){
		//	printf("in runQueuedDirectOutputs\n");
		printf("javascript engine spidermonkey version %ld %s\n", (long)js_run_version, SM_method() == 2? "SM2" : "SM1");
		doneOnce++;
	}

	return FALSE;
}
} //extern "C"

#endif //defined(JS_SMCPP)
#endif //JAVASCRIPT_SM


