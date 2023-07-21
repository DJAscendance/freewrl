/*


Complex VRML nodes as Javascript classes.

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


#ifndef __FREEWRL_JS_VRML_CLASSES_H__
#define __FREEWRL_JS_VRML_CLASSES_H__

#ifndef UNUSED
#define UNUSED(v) ((void) v)
#endif

#define INIT_ARGC_NODE 1
#define INIT_ARGC 0

/* tie a node into the root. Currently not required, as we do a better job
of garbage collection 
... NOTE! JS_AddRoot and JS_RemoveRoot is DEPRECATED as of JS_VERSION 185*/
#define ADD_ROOT(a,b) \
	/* printf ("adding root  cx %u pointer %u value %u\n",a,&b,b); \
        if (JS_AddRoot(a,&b) != JS_TRUE) { \
                printf ("JA_AddRoot failed at %s:%d\n",__FILE__,__LINE__); \
                return JS_FALSE; \
        } */

#define REMOVE_ROOT(a,b) \
	/* printf ("removing root %u\n",b); \
        JS_RemoveRoot(a,&b);  */

//#define MF_LENGTH_FIELD "mf_len"
#define MF_LENGTH_FIELD "length"

#define DEFINE_LENGTH(this_context,this_object,this_length) \
	{jsval zimbo = INT_TO_JSVAL(this_length);\
	/* printf ("defining length to %d for %d %d\n",this_length,this_context,this_object);*/ \
	if (!JS_DefineProperty(this_context, this_object, MF_LENGTH_FIELD, zimbo, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB2, JSPROP_PERMANENT)) { \
		ConsoleMessage( "JS_DefineProperty failed for \"%s\" at %s:%d.\n",MF_LENGTH_FIELD,__FILE__,__LINE__); \
		return JS_FALSE;\
	}}

#define DEFINE_LENGTH_NORV(this_context,this_object,this_length) \
	{jsval zimbo = INT_TO_JSVAL(this_length);\
	/* printf ("defining length to %d for %d %d\n",this_length,this_context,this_object);*/ \
	if (!JS_DefineProperty(this_context, this_object, MF_LENGTH_FIELD, zimbo, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB2, JSPROP_PERMANENT)) { \
		ConsoleMessage( "JS_DefineProperty failed for \"%s\" at %s:%d.\n",MF_LENGTH_FIELD,__FILE__,__LINE__); \
		return;\
	}}

#define DEFINE_MF_ECMA_HAS_CHANGED \
	{jsval zimbo = INT_TO_JSVAL(0); \
	/* printf ("defining property for MF_ECMA_HAS_CHANGED... %d %d ",cx,obj);  */ \
	if (!JS_DefineProperty(cx, obj, "MF_ECMA_has_changed", zimbo, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB2, JSPROP_PERMANENT)) { \
		printf( "JS_DefineProperty failed for \"MF_ECMA_has_changed\" at %s:%d.\n",__FILE__,__LINE__); \
		/* printf ("myThread is %u\n",pthread_self()); */ \
		return JS_FALSE; \
	}}

#define SET_MF_ECMA_HAS_CHANGED { jsval myv; \
                        myv = INT_TO_JSVAL(1); \
			 /* printf ("setting property for MF_ECMA_has_changed %d %d\n",cx,obj); */ \
                        if (!JS_SetProperty(cx, obj, "MF_ECMA_has_changed", &myv)) { \
                                printf( "JS_SetProperty failed for \"MF_ECMA_has_changed\" in doMFSetProperty.\n"); \
                                return JS_FALSE; \
                        }}


#define SET_JS_TICKTIME { jsval zimbo; \
        JS_NewNumberValue(cx, TickTime(), &zimbo);  \
        if (!JS_DefineProperty(cx,obj, "__eventInTickTime", zimbo, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB2, JSPROP_PERMANENT)) {  \
                printf( "JS_DefineProperty failed for \"__eventInTickTime\" at %s:%d.\n",__FILE__,__LINE__); \
                return; \
        }}

#define COMPILE_FUNCTION_IF_NEEDED(tnfield) \
	if (JSparamnames[tnfield].eventInFunction == NULL) { \
		sprintf (scriptline,"%s%s(%s%s,__eventInTickTime)", "",JSparamnames[tnfield].name,"__eventIn_Value_",JSparamnames[tnfield].name); \
		/* printf ("compiling function %s for type %d\n",scriptline,JSparamnames[tnfield].type); */ \
		JSparamnames[tnfield].eventInFunction = (void*)JS_CompileScript( \
			cx, obj, scriptline, strlen(scriptline), "compile eventIn",1); \
		if (!JS_AddObjectRoot(cx,(JSObject**)(&JSparamnames[tnfield].eventInFunction))) { \
			printf( "JS_AddObjectRoot failed for compilation of script \"%s\" at %s:%d.\n",scriptline,__FILE__,__LINE__); \
			return; \
		} \
	}
#define COMPILE_FUNCTION_IF_NEEDED_SET(tnfield,kind) \
	if (JSparamnames[tnfield].eventInFunction == NULL) { \
		if(kind == PKW_inputOutput) \
			sprintf (scriptline,"set_%s(%s,__eventInTickTime)", JSparamnames[tnfield].name,JSparamnames[tnfield].name); \
		else /* PKW_inputOnly */ \
			sprintf (scriptline,"%s(%s%s,__eventInTickTime)", JSparamnames[tnfield].name,"__eventIn_Value_",JSparamnames[tnfield].name); \
		/* printf ("compiling function %s for type %d\n",scriptline,JSparamnames[tnfield].type); */ \
		JSparamnames[tnfield].eventInFunction = (void*)JS_CompileScript( \
			cx, obj, scriptline, strlen(scriptline), "compile eventIn",1); \
		if (!JS_AddObjectRoot(cx,(JSObject**)(&JSparamnames[tnfield].eventInFunction))) { \
			printf( "JS_AddObjectRoot failed for compilation of script \"%s\" at %s:%d.\n",scriptline,__FILE__,__LINE__); \
			return; \
		} \
	}

#define RUN_FUNCTION(tnfield) \
	{ \
		jsval zimbo; \
		if (!JS_ExecuteScript(cx, obj, (JSScript*)JSparamnames[tnfield].eventInFunction, &zimbo)) { \
			printf ("eventIn %s failed to complete successfully, in FreeWRL code %s:%d\n",JSparamnames[tnfield].name,__FILE__,__LINE__); \
			/* printf ("myThread is %u\n",pthread_self());*/ \
		} \
	} 


#define SET_LENGTH(cx,newMFObject,length) \
	{ \
		jsval lenval; \
		lenval = INT_TO_JSVAL(length); \
		if (!JS_SetProperty(cx, newMFObject,  MF_LENGTH_FIELD, &lenval)) { \
			printf( "JS_SetProperty failed for \"%s\" at %s:%d\n", MF_LENGTH_FIELD,__FILE__,__LINE__); \
			return; \
		} \
	} 

#define SET_EVENTIN_VALUE(cx,obj,nameIndex,newObj) \
	{ \
		char scriptline[100]; \
		sprintf (scriptline,"__eventIn_Value_%s", JSparamnames[nameIndex].name); \
		if (!JS_DefineProperty(cx,obj, scriptline, OBJECT_TO_JSVAL(newObj), JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB2, JSPROP_PERMANENT)) {  \
			printf( "JS_DefineProperty failed for \"ECMA in\" at %s:%d.\n",__FILE__,__LINE__);  \
			/* printf ("myThread is %u\n",pthread_self()); */ \
			return; \
		} \
	}
	

/*
 * The following VRML field types don't need JS classes:
 * (ECMAScript native datatypes, see JS.pm):
 *
 * * SFBool
 * * SFFloat
 * * SFInt32
 * * SFString
 * * SFTime
 *
 * VRML field types that are implemented here as Javascript classes
 * are:
 *
 * * SFColor, MFColor
 * * MFFloat
 * * SFImage -- not supported currently
 * * MFInt32
 * * SFNode (special case - must be supported perl (see JS.pm), MFNode
 * * SFRotation, MFRotation
 * * MFString
 * * MFTime
 * * SFVec2f, MFVec2f
 * * SFVec3f, MFVec3f
 * * SFVec3d
 *
 * These (single value) fields have struct types defined elsewhere
 * (see Structs.h) that are stored by Javascript classes as private data.
 *
 * Some of the computations for SFVec3f, SFRotation are now defined
 * elsewhere (see LinearAlgebra.h) to avoid duplication.
 */


/* helper functions */
void JS_MY_Finalize(JSFreeOp *fop, JSObject *obj);


JSBool doMFToString(JSContext *cx, JSObject *obj, const char *className, jsval *rval); 

JSBool doMFAddProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp, const char *name); 
JSBool doMFSetProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp, int type); 

JSBool getBrowser(JSContext *context, JSObject *obj, BrowserNative **brow); 
JSBool doMFStringUnquote(JSContext *cx, jsval *vp);


/* class functions */


JSBool globalResolve(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid);


JSBool
loadVrmlClasses(JSContext *context,
				JSObject *globalObj);



JSBool getECMANative(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);

JSBool setECMANative(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid>  hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp);



JSBool getAssignProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);


JSBool setAssignProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp);


JSBool SFColorGetHSV(JSContext *cx, uintN argc, jsval *vp);
JSBool SFColorSetHSV(JSContext *cx, uintN argc, jsval *vp);
JSBool SFColorToString(JSContext *cx, uintN argc, jsval *vp);
JSBool SFColorAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool SFColorConstr(JSContext *cx, uintN argc, jsval *vp);


JSBool SFColorGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool SFColorSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp);



JSBool SFColorRGBAGetHSV(JSContext *cx, uintN argc, jsval *vp);
JSBool SFColorRGBASetHSV(JSContext *cx, uintN argc, jsval *vp);
JSBool SFColorRGBAToString(JSContext *cx, uintN argc, jsval *vp);
JSBool SFColorRGBAAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool SFColorRGBAConstr(JSContext *cx, uintN argc, jsval *vp);


JSBool SFColorRGBAGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool SFColorRGBASetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp);


JSBool SFImageToString(JSContext *cx, uintN argc,jsval *vp);
JSBool SFImageAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool SFImageConstr(JSContext *cx, uintN argc, jsval *vp);




JSBool SFImageGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool SFImageSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp);



JSBool SFNodeToString(JSContext *cx, uintN argc, jsval *vp);
JSBool SFNodeValueOf(JSContext *cx, uintN argc, jsval *vp);
JSBool SFNodeAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool SFNodeEquals(JSContext *cx, uintN argc, jsval *vp);
JSBool SFNodeConstr(JSContext *cx, uintN argc, jsval *vp);



void SFNodeFinalize(JSFreeOp *fop, JSObject *obj);

JSBool SFNodeGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool SFNodeSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp);


JSBool SFRotationGetAxis(JSContext *cx, uintN argc, jsval *vp);
JSBool SFRotationInverse(JSContext *cx, uintN argc, jsval *vp); /* not implemented */
JSBool SFRotationMultiply(JSContext *cx, uintN argc, jsval *vp);
JSBool SFRotationMultVec(JSContext *cx, uintN argc, jsval *vp);
JSBool SFRotationSetAxis(JSContext *cx, uintN argc, jsval *vp);
JSBool SFRotationSlerp(JSContext *cx, uintN argc, jsval *vp);
JSBool SFRotationToString(JSContext *cx, uintN argc, jsval *vp);
JSBool SFRotationAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool SFRotationConstr(JSContext *cx, uintN argc, jsval *vp);




JSBool SFRotationGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool SFRotationSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp);



JSBool SFVec2fAdd(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec2fDivide(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec2fDot(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec2fLength(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec2fMultiply(JSContext *cx, uintN argc, jsval *vp);
/* JSBool SFVec2fNegate(JSContext *cx, uintN argc, jsval *vp); */
JSBool SFVec2fNormalize(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec2fSubtract(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec2fToString(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec2fAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec2fConstr(JSContext *cx, uintN argc, jsval *vp);


JSBool SFVec2fGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool SFVec2fSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp);



JSBool SFVec3fAdd(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3fCross(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3fDivide(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3fDot(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3fLength(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3fMultiply(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3fNegate(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3fNormalize(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3fSubtract(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3fToString(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3fAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3fConstr(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3fGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool SFVec3fSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp);

JSBool SFVec2dAdd(JSContext* cx, uintN argc, jsval* vp);
JSBool SFVec2dDivide(JSContext* cx, uintN argc, jsval* vp);
JSBool SFVec2dDot(JSContext* cx, uintN argc, jsval* vp);
JSBool SFVec2dLength(JSContext* cx, uintN argc, jsval* vp);
JSBool SFVec2dMultiply(JSContext* cx, uintN argc, jsval* vp);
/* JSBool SFVec2fNegate(JSContext *cx, uintN argc, jsval *vp); */
JSBool SFVec2dNormalize(JSContext* cx, uintN argc, jsval* vp);
JSBool SFVec2dSubtract(JSContext* cx, uintN argc, jsval* vp);
JSBool SFVec2dToString(JSContext* cx, uintN argc, jsval* vp);
JSBool SFVec2dAssign(JSContext* cx, uintN argc, jsval* vp);
JSBool SFVec2dConstr(JSContext* cx, uintN argc, jsval* vp);


JSBool SFVec2dGetProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool SFVec2dSetProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp);




JSBool SFVec3dAdd(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3dCross(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3dDivide(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3dDot(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3dLength(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3dMultiply(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3dNegate(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3dNormalize(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3dSubtract(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3dToString(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3dAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3dConstr(JSContext *cx, uintN argc, jsval *vp);

JSBool SFVec3dGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool SFVec3dSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp);




JSBool SFVec4fToString(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec4fAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec4fConstr(JSContext *cx, uintN argc, jsval *vp);

JSBool SFVec4fGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool SFVec4fSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp);

JSBool SFVec4dToString(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec4dAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec4dConstr(JSContext *cx, uintN argc, jsval *vp);

JSBool SFVec4dGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool SFVec4dSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp);



JSBool MFColorToString(JSContext *cx, uintN argc, jsval *vp);
JSBool MFColorAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool MFColorConstr(JSContext *cx, uintN argc, jsval *vp);
JSBool MFColorConstrInternals(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

JSBool MFColorAddProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool MFColorGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool MFColorSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp);

JSBool MFColorRGBAToString(JSContext* cx, uintN argc, jsval* vp);
JSBool MFColorRGBAAssign(JSContext* cx, uintN argc, jsval* vp);
JSBool MFColorRGBAConstr(JSContext* cx, uintN argc, jsval* vp);
JSBool MFColorRGBAConstrInternals(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval);

JSBool MFColorRGBAAddProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool MFColorRGBAGetProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool MFColorRGBASetProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp);


JSBool MFFloatToString(JSContext *cx, uintN argc, jsval *vp);
JSBool MFFloatAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool MFFloatConstr(JSContext *cx, uintN argc, jsval *vp);
JSBool MFFloatConstrInternals(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);


JSBool MFFloatAddProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool MFFloatGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool MFFloatSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp);


JSBool MFInt32ToString(JSContext *cx, uintN argc, jsval *vp);
JSBool MFInt32Assign(JSContext *cx, uintN argc, jsval *vp);
JSBool MFInt32Constr(JSContext *cx, uintN argc, jsval *vp);
JSBool MFInt32ConstrInternals(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

JSBool MFInt32AddProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool MFInt32GetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool MFInt32SetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp);


JSBool MFBoolToString(JSContext* cx, uintN argc, jsval* vp);
JSBool MFBoolAssign(JSContext* cx, uintN argc, jsval* vp);
JSBool MFBoolConstr(JSContext* cx, uintN argc, jsval* vp);
JSBool MFBoolConstrInternals(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval);

JSBool MFBoolAddProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool MFBoolGetProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool MFBoolSetProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp);


JSBool MFNodeToString(JSContext *cx, uintN argc, jsval *vp);
JSBool MFNodeAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool MFNodeConstr(JSContext *cx, uintN argc, jsval *vp);
JSBool MFNodeConstrInternals(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

JSBool MFNodeAddProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool MFNodeGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool MFNodeSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp);


JSBool MFRotationToString(JSContext *cx, uintN argc, jsval *vp);
JSBool MFRotationAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool MFRotationConstr(JSContext *cx, uintN argc, jsval *vp);
JSBool MFRotationConstrInternals(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);


JSBool MFRotationAddProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool MFRotationGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool MFRotationSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp);


JSBool MFStringToString(JSContext *cx, uintN argc, jsval *vp);
JSBool MFStringAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool MFStringConstr(JSContext *cx, uintN argc, jsval *vp);
JSBool MFStringConstrInternals(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);


JSBool MFStringConvertProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JSType type, JS::MutableHandle<JS::Value> hvp);
JSBool MFStringEnumerateProperty(JSContext *cx, JS::Handle<JSObject*> hobj);
JSBool MFStringDeleteProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid,  JSBool *succeeded);

JSBool MFStringAddProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool MFStringDeleteProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool *succeeded);
JSBool MFStringGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool MFStringSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp);
JSBool MFStringResolveProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid);



JSBool MFTimeToString(JSContext *cx, uintN argc, jsval *vp);
JSBool MFTimeAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool MFTimeConstr(JSContext *cx, uintN argc, jsval *vp);
JSBool MFTimeConstrInternals(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool MFTimeAddProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool MFTimeGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool MFTimeSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp);

JSBool MFDoubleToString(JSContext* cx, uintN argc, jsval* vp);
JSBool MFDoubleAssign(JSContext* cx, uintN argc, jsval* vp);
JSBool MFDoubleConstr(JSContext* cx, uintN argc, jsval* vp);
JSBool MFDoubleConstrInternals(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval);
JSBool MFDoubleAddProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool MFDoubleGetProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool MFDoubleSetProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp);



JSBool MFVec2fToString(JSContext *cx, uintN argc, jsval *vp);
JSBool MFVec2fAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool MFVec2fConstr(JSContext *cx, uintN argc, jsval *vp);
JSBool MFVec2fConstrInternals(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);



JSBool MFVec2fAddProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool MFVec2fGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool MFVec2fSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp);


JSBool MFVec3fToString(JSContext *cx, uintN argc, jsval *vp);
JSBool MFVec3fAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool MFVec3fConstr(JSContext *cx, uintN argc, jsval *vp);
JSBool MFVec3fConstrInternals(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool MFVec3fAddProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool MFVec3fGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool MFVec3fSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp);

JSBool MFVec4fToString(JSContext* cx, uintN argc, jsval* vp);
JSBool MFVec4fAssign(JSContext* cx, uintN argc, jsval* vp);
JSBool MFVec4fConstr(JSContext* cx, uintN argc, jsval* vp);
JSBool MFVec4fConstrInternals(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval);
JSBool MFVec4fAddProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool MFVec4fGetProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool MFVec4fSetProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp);

JSBool MFVec2dToString(JSContext* cx, uintN argc, jsval* vp);
JSBool MFVec2dAssign(JSContext* cx, uintN argc, jsval* vp);
JSBool MFVec2dConstr(JSContext* cx, uintN argc, jsval* vp);
JSBool MFVec2dConstrInternals(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval);
JSBool MFVec2dAddProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool MFVec2dGetProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool MFVec2dSetProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp);

JSBool MFVec3dToString(JSContext* cx, uintN argc, jsval* vp);
JSBool MFVec3dAssign(JSContext* cx, uintN argc, jsval* vp);
JSBool MFVec3dConstr(JSContext* cx, uintN argc, jsval* vp);
JSBool MFVec3dConstrInternals(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval);
JSBool MFVec3dAddProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool MFVec3dGetProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool MFVec3dSetProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp);


JSBool VrmlMatrixToString(JSContext *cx, uintN argc, jsval *vp);
JSBool VrmlMatrixAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool VrmlMatrixsetTransform(JSContext *cx, uintN argc, jsval *vp);
JSBool VrmlMatrixgetTransform(JSContext *cx, uintN argc, jsval *vp);
JSBool VrmlMatrixinverse(JSContext *cx, uintN argc, jsval *vp);
JSBool VrmlMatrixtranspose(JSContext *cx, uintN argc, jsval *vp);
JSBool VrmlMatrixmultLeft(JSContext *cx, uintN argc, jsval *vp);
JSBool VrmlMatrixmultRight(JSContext *cx, uintN argc, jsval *vp);
JSBool VrmlMatrixmultVecMatrix(JSContext *cx, uintN argc, jsval *vp);
JSBool VrmlMatrixmultMatrixVec(JSContext *cx, uintN argc, jsval *vp);
JSBool VrmlMatrixConstr(JSContext *cx, uintN argc, jsval *vp);
JSBool VrmlMatrixConstrInternals(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);


JSBool VrmlMatrixAddProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool VrmlMatrixGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);
JSBool VrmlMatrixSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp);


JSBool _standardMFAssign(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval, JSClass *myClass, int type);
JSBool _standardMFGetProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp, const char *makeNewElement, int type);

void printJSNodeType (JSContext *context, JSObject *myobj);

extern JSClass SFColorClass;
extern JSPropertySpec (SFColorProperties)[];
extern JSFunctionSpec (SFColorFunctions)[];
extern JSClass SFColorRGBAClass;
extern JSPropertySpec (SFColorRGBAProperties)[];
extern JSFunctionSpec (SFColorRGBAFunctions)[];
extern JSClass SFImageClass;
extern JSPropertySpec (SFImageProperties)[];
extern JSFunctionSpec (SFImageFunctions)[];
extern JSClass SFNodeClass;
extern JSPropertySpec (SFNodeProperties)[];
extern JSFunctionSpec (SFNodeFunctions)[];
extern JSClass SFRotationClass;
extern JSPropertySpec (SFRotationProperties)[];
extern JSFunctionSpec (SFRotationFunctions)[];
extern JSClass SFVec2fClass;
extern JSPropertySpec (SFVec2fProperties)[];
extern JSFunctionSpec (SFVec2fFunctions)[];
extern JSClass SFVec3fClass;
extern JSPropertySpec (SFVec3fProperties)[];
extern JSFunctionSpec (SFVec3fFunctions)[];
extern JSClass SFVec2dClass;
extern JSPropertySpec(SFVecdfProperties)[];
extern JSFunctionSpec(SFVecdfFunctions)[];
extern JSClass SFVec3dClass;
extern JSPropertySpec (SFVec3dProperties)[];
extern JSFunctionSpec (SFVec3dFunctions)[];


extern JSClass SFVec4fClass;
extern JSPropertySpec (SFVec4fProperties)[];
extern JSFunctionSpec (SFVec4fFunctions)[];
extern JSClass SFVec4dClass;
extern JSPropertySpec (SFVec4dProperties)[];
extern JSFunctionSpec (SFVec4dFunctions)[];

extern JSClass MFColorClass;
extern JSFunctionSpec (MFColorFunctions)[];
extern JSClass MFColorRGBAClass;
extern JSFunctionSpec(MFColorRGBAFunctions)[];

extern JSClass MFFloatClass;
extern JSFunctionSpec (MFFloatFunctions)[];
extern JSClass MFBoolClass;
extern JSFunctionSpec (MFBoolFunctions)[];
extern JSClass MFInt32Class;
extern JSFunctionSpec (MFInt32Functions)[];
extern JSClass MFNodeClass;
extern JSFunctionSpec (MFNodeFunctions)[];
extern JSClass MFRotationClass;
extern JSFunctionSpec (MFRotationFunctions)[];
extern JSClass MFStringClass;
extern JSFunctionSpec (MFStringFunctions)[];
extern JSClass MFTimeClass;
extern JSPropertySpec (MFTimeProperties)[] ;
extern JSFunctionSpec (MFTimeFunctions)[];
extern JSClass MFDoubleClass;
extern JSPropertySpec(MFDoubleProperties)[];
extern JSFunctionSpec(MFDoubleFunctions)[];

extern JSClass MFVec2fClass;
extern JSFunctionSpec (MFVec2fFunctions)[];
extern JSClass MFVec3fClass;
extern JSFunctionSpec (MFVec3fFunctions)[];
extern JSClass MFVec4fClass;
extern JSFunctionSpec(MFVec4fFunctions)[];
extern JSClass MFVec2dClass;
extern JSFunctionSpec(MFVec2dFunctions)[];
extern JSClass MFVec3dClass;
extern JSFunctionSpec(MFVec3dFunctions)[];


extern JSClass VrmlMatrixClass;
extern JSFunctionSpec (VrmlMatrixFunctions)[];


JSBool js_SetPropertyCheck(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp);
JSBool js_SetPropertyDebug5(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp);
JSBool js_SetPropertyDebug6 (JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp);
JSBool js_SetPropertyDebug3 (JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp);
JSBool js_SetPropertyDebug8 (JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp);


#endif /*  JS_VRML_CLASSES */
