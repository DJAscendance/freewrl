/*


FreeWRL support library.
Internal header: Javascript engine dependencies.

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


#ifndef __LIBFREEWRL_SYSTEM_JS_H__
#define __LIBFREEWRL_SYSTEM_JS_H__


/* 
   spidermonkey is built with the following flags on Mac:

-Wall -Wno-format -no-cpp-precomp -fno-common -DJS_THREADSAFE -DXP_UNIX -DSVR4 -DSYSV -D_BSD_SOURCE -DPOSIX_SOURCE -DDARWIN  -UDEBUG -DNDEBUG -UDEBUG_root -DJS_THREADSAFE -DEDITLINE

*/

#define JS_HAS_FILE_OBJECT 1 /* workaround warning=>error */

#if defined(IPHONE) || defined(_ANDROID) || defined(NO_JAVASCRIPT)
typedef int JSContext;
typedef int JSObject;
typedef int JSScript;
typedef int jsval;
typedef int jsid;
typedef int JSType;
typedef int JSClass;
typedef int JSFunctionSpec;
typedef int  JSPropertySpec;
typedef int JSBool;
typedef int uintN;
typedef int JSErrorReport;



/* int jsrrunScript(JSContext *_context, JSObject *_globalObj, char *script, jsval *rval); */
#else

#ifdef MOZILLA_JS_UNSTABLE_INCLUDES
# include "../unstable/jsapi.h" /* JS compiler */
# include "../unstable/jsdbgapi.h" /* JS debugger */
#else
//#define DEBUG 1 //challenge it with lots of ASSERTS, just for cleaning up code correctness, not production
#undef DEBUG
# include <jsapi.h> /* JS compiler */
# include <jsdbgapi.h> /* JS debugger */
#endif
int JS_SetPrivateFw(JSContext *cx, JSObject* obj, void *data);
JSObject* JS_NewGlobalObjectFw(JSContext *cx, JSClass *clasp); //, JSPrincipals *princ);
void * JS_GetPrivateFw(JSContext *cx,JSObject*_obj);
JSObject* JS_GetParentFw(JSContext *cx, JSObject *me);
JSObject * JS_ConstructObjectWithArgumentsFw(JSContext *cx, JSClass *clasp, JSObject *parent, unsigned argc, jsval *argv); 
JSObject * JS_ConstructObjectFw(JSContext *cx, JSClass *clasp, void *whatever, JSObject *parent);
JSObject * JS_GetPrototypeFw(JSContext *cx, JSObject * obj);
JSClass * JS_GetClassFw(JSContext *cx, JSObject * obj);
#if JS_VERSION >= 185
#define JSSCRIPT JSObject
#if JS_VERSION == 185
#define JSSCRIPT2 JSObject
#endif
#if JS_VERSION == 186 
// spidermonkey 17 -last C interface version- is 186
// https://developer.mozilla.org/en-US/docs/Mozilla/Projects/SpiderMonkey/Releases/17
// could alternatively define in preprocessor parameters MOZ_CUSTOM_STDINT_H and jsapi will define, tried but didn't build
// instead for debugging / tinkering we will define here
#define uintN unsigned
#define intN int
#define jsint int32_t
#define jsuint uint32_t
#define int32 int32_t
#define jsdouble double

#define JS_FinalizeStub NULL
#define JSSCRIPT2 JSScript
#define JS_GET_CLASS JS_GetClassFw
JSBool JS_NewNumberValue(JSContext *cx, jsdouble d, jsval *rval);
#define JSVAL_IS_OBJECT(retval) JSVAL_IS_OBJECT_OR_NULL_IMPL(retval)

#endif
#else
#define JSSCRIPT JSScript
#define JSSCRIPT2 JSObject
#endif


#define JS_GET_PROPERTY_STUB JS_PropertyStub
/* #define JS_GET_PROPERTY_STUB js_GetPropertyDebug */

#define JS_SET_PROPERTY_STUB1 js_SetPropertyDebug1

/* #define JS_SET_PROPERTY_STUB2 js_SetPropertyDebug2  */
#if JS_VERSION < 185
# define JS_SET_PROPERTY_STUB2 JS_PropertyStub
#else
# define JS_SET_PROPERTY_STUB2 JS_StrictPropertyStub
#endif

#define JS_SET_PROPERTY_STUB3 js_SetPropertyDebug3 
#define JS_SET_PROPERTY_STUB4 js_SetPropertyDebug4 
#define JS_SET_PROPERTY_STUB5 js_SetPropertyDebug5 
#define JS_SET_PROPERTY_STUB6 js_SetPropertyDebug6 
#define JS_SET_PROPERTY_STUB7 js_SetPropertyDebug7 
#define JS_SET_PROPERTY_STUB8 js_SetPropertyDebug8 
#define JS_SET_PROPERTY_CHECK js_SetPropertyCheck


#endif /* IPHONE */

#endif /* __LIBFREEWRL_SYSTEM_JS_H__ */
