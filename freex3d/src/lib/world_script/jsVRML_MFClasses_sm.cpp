/*


???

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
# include <jsapi.h> /* JS compiler */
# include <jsdbgapi.h> /* JS debugger */
//#if !(defined(JAVASCRIPT_STUB) || defined(JAVASCRIPT_DUK))
#define JS_VERSION 187
//#define JS_THREADSAFE 1 //by default in 186+

#define STRING_SIZE 256
#define uintN unsigned
#define intN int
#define jsint int32_t
#define jsuint uint32_t
#define int32 int32_t
#define jsdouble double

#define JS_FinalizeStub NULL
#ifndef IBOOL
typedef int IBOOL;
#endif
typedef IBOOL _Bool;



extern "C" {
#include <system.h>
//#if !(defined(JAVASCRIPT_STUB) || defined(JAVASCRIPT_DUK))

//#include <system_threads.h>
#include <display.h>
#include <internal.h>

//#include <libFreeWRL.h>
#include "../scenegraph/LinearAlgebra.h"
#include "../scenegraph/quaternion.h"

//#include "../vrml_parser/Structs.h"
//#include "../main/headers.h"
//#include "../vrml_parser/CParseGeneral.h"
//#include "../main/Snapshot.h"
//#include "../scenegraph/Collision.h"
//#include "../scenegraph/quaternion.h"
//#include "../scenegraph/Viewer.h"
//#include "../input/SensInterps.h"
//#include "../x3d_parser/Bindable.h"

#include "JScript.h"
#include "CScripts.h"
#include "jsNative.h"

#include "JScript.h"

} //extern "C"

#include "jsUtils_sm.h"
#include "jsVRMLClasses_sm.h"

/********************************************************/
/*							*/
/* Third part - MF classes				*/
/*							*/
/********************************************************/

/* remove any private data from this datatype, and let the garbage collector handle the object */

void
#if JS_VERSION < 186
JS_MY_Finalize(JSContext *cx, JSObject *obj){
#else
JS_MY_Finalize(JSFreeOp *fop, JSObject *obj){
//JSContext *cx = NULL;
#endif

	void *ptr;
	//#ifdef JSVRMLCLASSESVERBOSE
	//printf ("finalizing %p\n",obj);
	//printJSNodeType(cx,obj);
	//#endif

	//REMOVE_ROOT(cx,obj)

	//if ((ptr = (void *)JS_GetPrivateFw(cx, obj)) != NULL) {
		ptr = (void*)JS_GetPrivate(obj);
		if (ptr) {
			if (SM_method() == 0)
				FREE_IF_NZ(ptr);
			if (SM_method() == 2) {
				//AnyNativeNew mallocs ptr, v and elsewhere mf.p is malloced
				AnyNative* any = (AnyNative*)ptr;
				if (any->gc) {
					if (any->type % 2 == 1) //is it MF
						FREE_IF_NZ(any->v->mffloat.p);
					FREE_IF_NZ(any->v);
				}
				//printf("finalize anygc = %d\n", any->gc);
				FREE_IF_NZ(ptr);
			}
		}
		//JS_SetPrivateFw(cx,obj,NULL);
		//FREE_IF_NZ(ptr);
		
	//}

	#ifdef JSVRMLCLASSESVERBOSE
	} else {
		printf ("Finalize - no private data!\n");
	}
	#endif

}

// MFColor
JSBool
MFColorToString(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
	jsval rval;

	UNUSED(argc);
	UNUSED(argv);
	if (!doMFToString(cx, obj, "MFColor", &rval)) { return JS_FALSE; }
	JS_SET_RVAL(cx,vp,rval);
	return JS_TRUE;

}

JSBool
MFColorAssign(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
	jsval rval;
	if (!_standardMFAssign (cx, obj, argc, argv, &rval, &MFColorClass,FIELDTYPE_SFColor)) { return JS_FALSE; }
	JS_SET_RVAL(cx,vp,rval);
	return JS_TRUE;

}

JSBool
MFColorConstr(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_NewObject(cx,&MFColorClass,NULL,NULL);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval = OBJECT_TO_JSVAL(obj);
        if (!MFColorConstrInternals(cx,obj,argc,argv,&rval)) { return JS_FALSE; }
        JS_SET_RVAL(cx,vp,rval);
        return JS_TRUE;
}
JSBool MFColorConstrInternals(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	JSObject *_arrayObj;
	int isArray;
	JSObject *_obj;
	unsigned int i;
	union anyVrml *anyv;
	
	ADD_ROOT(cx,obj)

	isArray = FALSE;
	if(argc == 1 && argv){
		//could it be new MFxxx( [A,B] ) javscript array, as used by Carlson aka Carlson Array
		// tests/JohnCarlson/Arc1A.x3d
		if (!JS_ValueToObject(cx, argv[0], &_arrayObj)) {
			printf( "JS_ValueToObject failed in MFVec3fConstr.\n");
			return JS_FALSE;
		}

		if(JS_IsArrayObject(cx, _arrayObj)){
			jsuint lengthp;
			jsval vp;
			//printf("its an array\n");
			isArray = TRUE;
			JS_GetArrayLength(cx,_arrayObj, &lengthp);
			argc = lengthp;
		}
	}

	if(SM_method() == 2){
		AnyNative *any;
		int newsize;
		if((any = (AnyNative*)AnyNativeNew(FIELDTYPE_MFColor,NULL,NULL)) == NULL){
			printf( "AnyfNativeNew failed in MFColorConstr.\n");
			return JS_FALSE;
		}
		if (!JS_SetPrivateFw(cx, obj, any)) {
			printf( "JS_SetPrivate failed in MFColorConstr.\n");
			return JS_FALSE;
		}
		anyv = any->v;
		newsize = sizeof(struct SFColor)*upper_power_of_two(argc);
		if(argc > 0){
			anyv->mfcolor.p = MALLOC(struct SFColor*,newsize);
			memset(anyv->mfcolor.p,0,newsize);
		}

	}else{
		DEFINE_LENGTH(cx,obj,argc)
	}
	if (!argv) {
		return JS_TRUE;
	}

	#ifdef JSVRMLCLASSESVERBOSE
		printf("MFColorConstr: obj = %p, %u args\n",
			   obj, argc);
	#endif

	for (i = 0; i < argc; i++) {
		jsval vp;
		if(isArray){
			JS_GetElement(cx, _arrayObj, i, &vp);

		}else{
			vp = argv[i];
		}
		if (!JS_ValueToObject(cx, vp, &_obj)) {
			printf(
					"JS_ValueToObject failed in MFColorConstr.\n");
			return JS_FALSE;
		}

		CHECK_CLASS(cx,_obj,NULL,__FUNCTION__,SFColorClass)
		if(SM_method()==2){
			AnyNative *any2;
			if((any2 = (AnyNative *)JS_GetPrivateFw(cx,_obj)) != NULL){
				//2018 I think as long as its 3+ contiguous floats, we can use it as a color, 
				// but in future internal types might change
				if(any2->type == FIELDTYPE_SFColor || any2->type == FIELDTYPE_SFVec3f || any2->type == FIELDTYPE_SFColorRGBA){
					shallow_copy_field(FIELDTYPE_SFColor,any2->v,(union anyVrml*)&anyv->mfcolor.p[i]);
					anyv->mfcolor.n = i+1;
				}
			}
			// else for now we'll leave zeros
		}else{
			if (!JS_DefineElement(cx, obj, (jsint) i, vp, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_CHECK, JSPROP_ENUMERATE)) {
				printf( "JS_DefineElement failed for arg %u in MFColorConstr.\n", i);
				return JS_FALSE;
			}
		}
	}
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}

JSBool

MFColorAddProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid,  JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid id = *hiid.address();
	jsval *vp = hvp.address();

	return doMFAddProperty(cx, obj, id, vp,"MFColorAddProperty");
}

JSBool
MFColorGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid,  JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid id = *hiid.address();
	jsval *vp = hvp.address();

	return _standardMFGetProperty(cx, obj, id, vp,
			"_FreeWRL_Internal = new SFColor()", FIELDTYPE_MFColor);
}

JSBool
MFColorSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid id = *hiid.address();
	jsval *vp = hvp.address();

	return doMFSetProperty(cx, obj, id, vp,FIELDTYPE_MFColor);
}

// MFColorRGBA
JSBool
MFColorRGBAToString(JSContext* cx, uintN argc, jsval* vp) {
	JSObject* obj = JS_THIS_OBJECT(cx, vp);
	jsval* argv = JS_ARGV(cx, vp);
	jsval rval;

	UNUSED(argc);
	UNUSED(argv);
	if (!doMFToString(cx, obj, "MFColorRGBA", &rval)) { return JS_FALSE; }
	JS_SET_RVAL(cx, vp, rval);
	return JS_TRUE;

}

JSBool
MFColorRGBAAssign(JSContext* cx, uintN argc, jsval* vp) {
	JSObject* obj = JS_THIS_OBJECT(cx, vp);
	jsval* argv = JS_ARGV(cx, vp);
	jsval rval;
	if (!_standardMFAssign(cx, obj, argc, argv, &rval, &MFColorRGBAClass, FIELDTYPE_SFColorRGBA)) { return JS_FALSE; }
	JS_SET_RVAL(cx, vp, rval);
	return JS_TRUE;

}

JSBool
MFColorRGBAConstr(JSContext* cx, uintN argc, jsval* vp) {
	JSObject* obj = JS_NewObject(cx, &MFColorRGBAClass, NULL, NULL);
	jsval* argv = JS_ARGV(cx, vp);
	jsval rval = OBJECT_TO_JSVAL(obj);
	if (!MFColorRGBAConstrInternals(cx, obj, argc, argv, &rval)) { return JS_FALSE; }
	JS_SET_RVAL(cx, vp, rval);
	return JS_TRUE;
}
JSBool MFColorRGBAConstrInternals(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) {

	JSObject* _arrayObj;
	int isArray;
	JSObject* _obj;
	unsigned int i;
	union anyVrml* anyv;

	ADD_ROOT(cx, obj)

		isArray = FALSE;
	if (argc == 1 && argv) {
		//could it be new MFxxx( [A,B] ) javscript array, as used by Carlson aka Carlson Array
		// tests/JohnCarlson/Arc1A.x3d
		if (!JS_ValueToObject(cx, argv[0], &_arrayObj)) {
			printf("JS_ValueToObject failed in MFVec3fConstr.\n");
			return JS_FALSE;
		}

		if (JS_IsArrayObject(cx, _arrayObj)) {
			jsuint lengthp;
			jsval vp;
			//printf("its an array\n");
			isArray = TRUE;
			JS_GetArrayLength(cx, _arrayObj, &lengthp);
			argc = lengthp;
		}
	}

	if (SM_method() == 2) {
		AnyNative* any;
		int newsize;
		if ((any = (AnyNative*)AnyNativeNew(FIELDTYPE_MFColorRGBA, NULL, NULL)) == NULL) {
			printf("AnyfNativeNew failed in MFColorRGBAConstr.\n");
			return JS_FALSE;
		}
		if (!JS_SetPrivateFw(cx, obj, any)) {
			printf("JS_SetPrivate failed in MFColorRGBAConstr.\n");
			return JS_FALSE;
		}
		anyv = any->v;
		newsize = sizeof(struct SFColorRGBA) * upper_power_of_two(argc);
		if (argc > 0) {
			anyv->mfcolorrgba.p = MALLOC(struct SFColorRGBA*, newsize);
			memset(anyv->mfcolorrgba.p, 0, newsize);
		}

	}
	else {
		DEFINE_LENGTH(cx, obj, argc)
	}
	if (!argv) {
		return JS_TRUE;
	}

#ifdef JSVRMLCLASSESVERBOSE
	printf("MFColorRGBAConstr: obj = %p, %u args\n",
		obj, argc);
#endif

	for (i = 0; i < argc; i++) {
		jsval vp;
		if (isArray) {
			JS_GetElement(cx, _arrayObj, i, &vp);

		}
		else {
			vp = argv[i];
		}
		if (!JS_ValueToObject(cx, vp, &_obj)) {
			printf(
				"JS_ValueToObject failed in MFColorRGBAConstr.\n");
			return JS_FALSE;
		}

		CHECK_CLASS(cx, _obj, NULL, __FUNCTION__, SFColorRGBAClass)
			if (SM_method() == 2) {
				AnyNative* any2;
				if ((any2 = (AnyNative*)JS_GetPrivateFw(cx, _obj)) != NULL) {
					//2018 I think as long as its 4+ contiguous floats, we can use it as a colorrgba, 
					// but in future internal types might change
					if (any2->type == FIELDTYPE_SFColorRGBA || any2->type == FIELDTYPE_SFVec4f) {
						shallow_copy_field(FIELDTYPE_SFColorRGBA, any2->v, (union anyVrml*)&anyv->mfcolorrgba.p[i]);
						anyv->mfcolorrgba.n = i + 1;
					}
				}
				// else for now we'll leave zeros
			}
			else {
				if (!JS_DefineElement(cx, obj, (jsint)i, vp, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_CHECK, JSPROP_ENUMERATE)) {
					printf("JS_DefineElement failed for arg %u in MFColorRGBAConstr.\n", i);
					return JS_FALSE;
				}
			}
	}
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}

JSBool

MFColorRGBAAddProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp) {
	JSObject* obj = *hobj.address();
	jsid id = *hiid.address();
	jsval* vp = hvp.address();

	return doMFAddProperty(cx, obj, id, vp, "MFColorRGBAAddProperty");
}

JSBool
MFColorRGBAGetProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp) {
	JSObject* obj = *hobj.address();
	jsid id = *hiid.address();
	jsval* vp = hvp.address();

	return _standardMFGetProperty(cx, obj, id, vp,
		"_FreeWRL_Internal = new SFColorRGBA()", FIELDTYPE_MFColorRGBA);
}

JSBool
MFColorRGBASetProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp) {
	JSObject* obj = *hobj.address();
	jsid id = *hiid.address();
	jsval* vp = hvp.address();

	return doMFSetProperty(cx, obj, id, vp, FIELDTYPE_MFColorRGBA);
}


// MFFloat
JSBool
MFFloatToString(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;

	UNUSED(argc);
	UNUSED(argv);
	if (!doMFToString(cx, obj, "MFFloat", &rval)) { 
		return JS_FALSE; }
	JS_SET_RVAL(cx,vp,rval);
	return JS_TRUE;

}

JSBool
MFFloatAssign(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;
	SET_MF_ECMA_HAS_CHANGED

	if (!_standardMFAssign (cx, obj, argc, argv, &rval, &MFFloatClass,FIELDTYPE_SFFloat)) { return JS_FALSE; }
        JS_SET_RVAL(cx,vp,rval);
        return JS_TRUE;
}

JSBool
MFFloatConstr(JSContext *cx, uintN argc, jsval *vp) {
		JSObject* obj = JS_NewObject(cx, &MFFloatClass, NULL, NULL);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval = OBJECT_TO_JSVAL(obj);
        if (!MFFloatConstrInternals(cx,obj,argc,argv,&rval)) { return JS_FALSE; }
        JS_SET_RVAL(cx,vp,rval);
        return JS_TRUE;
}
JSBool MFFloatConstrInternals(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	JSObject *_arrayObj;
	int isArray;

	jsdouble _d;
	unsigned int i;
	union anyVrml *anyv;

//	ADD_ROOT(cx,obj) only root non-stack variables

	isArray = FALSE;
	if(argc == 1 && argv){
		//could it be new MFxxx( [A,B] ) javscript array, as used by Carlson aka Carlson Array
		// tests/JohnCarlson/Arc1A.x3d
		if (!JS_ValueToObject(cx, argv[0], &_arrayObj)) {
			printf( "JS_ValueToObject failed in MFVec3fConstr.\n");
			return JS_FALSE;
		}

		if(JS_IsArrayObject(cx, _arrayObj)){
			jsuint lengthp;
			jsval vp;
			//printf("its an array\n");
			isArray = TRUE;
			JS_GetArrayLength(cx,_arrayObj, &lengthp);
			argc = lengthp;
		}
	}

	if(SM_method() == 2){
		AnyNative *any;
		if((any = (AnyNative*)AnyNativeNew(FIELDTYPE_MFFloat,NULL,NULL)) == NULL){
			printf( "AnyfNativeNew failed in MFFloatConstr.\n");
			return JS_FALSE;
		}
		if (!JS_SetPrivateFw(cx, obj, any)) {
			printf( "JS_SetPrivate failed in MFFloatConstr.\n");
			return JS_FALSE;
		}
		anyv = any->v;
		if(argc > 0)
			anyv->mffloat.p = (float*)malloc(sizeof(float)*upper_power_of_two(argc));

	}else{
		DEFINE_LENGTH(cx,obj,argc)
		DEFINE_MF_ECMA_HAS_CHANGED
	}

	if (!argv) {
		return JS_TRUE;
	}

	#ifdef JSVRMLCLASSESVERBOSE
		printf("MFFloatConstr: obj = %p, %u args\n", obj, argc);
	#endif
	for (i = 0; i < argc; i++) {
		jsval vp;
		if(isArray){
			JS_GetElement(cx, _arrayObj, i, &vp);

		}else{
			vp = argv[i];
		}
		if (!JS_ValueToNumber(cx, vp, &_d)) {
			printf( "JS_ValueToNumber failed in MFFloatConstr.\n");
			return JS_FALSE;
		}
		if(SM_method()==2){
			anyv->mffloat.p[i] = _d;
			anyv->mffloat.n = i+1;
		}else{
			if (!JS_DefineElement(cx, obj, (jsint) i, vp, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_CHECK, JSPROP_ENUMERATE)) {
				printf( "JS_DefineElement failed for arg %u in MFFloatConstr.\n", i);
				return JS_FALSE;
			}
		}
	}
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}

JSBool
MFFloatAddProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid,  JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid id = *hiid.address();
	jsval *vp = hvp.address();
	return doMFAddProperty(cx, obj, id, vp,"MFFloatAddProperty");
}

JSBool
MFFloatGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid,  JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid id = *hiid.address();
	jsval *vp = hvp.address();

	return _standardMFGetProperty(cx, obj, id, vp,
			"_FreeWRL_Internal = 0.0", FIELDTYPE_MFFloat);
}

JSBool
MFFloatSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid id = *hiid.address();
	jsval *vp = hvp.address();

	return doMFSetProperty(cx, obj, id, vp,FIELDTYPE_MFFloat);
}

// MFInt32
JSBool
MFInt32ToString(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;

	UNUSED(argc);
	UNUSED(argv);
	#ifdef JSVRMLCLASSESVERBOSE
	printf ("start of MFInt32ToString\n");
	#endif

	if (!doMFToString(cx, obj, "MFInt32", &rval)) { return JS_FALSE; }
	JS_SET_RVAL(cx,vp,rval);
	return JS_TRUE;

}

JSBool
MFInt32Assign(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;

	#ifdef JSVRMLCLASSESVERBOSE
	printf ("start of MFInt32Assign\n");
	#endif

	SET_MF_ECMA_HAS_CHANGED

	if (!_standardMFAssign (cx, obj, argc, argv, &rval, &MFInt32Class,FIELDTYPE_SFInt32)) { return JS_FALSE; }
	JS_SET_RVAL(cx,vp,rval);
	return JS_TRUE;

}


JSBool
MFInt32Constr(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_NewObject(cx,&MFInt32Class,NULL,NULL);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval = OBJECT_TO_JSVAL(obj);
        if (!MFInt32ConstrInternals(cx,obj,argc,argv,&rval)) { return JS_FALSE; }
        JS_SET_RVAL(cx,vp,rval);
        return JS_TRUE;
}
JSBool MFInt32ConstrInternals(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	JSObject *_arrayObj;
	int isArray;
	int32 _i;
	unsigned int i;
	union anyVrml *anyv;
	#ifdef JSVRMLCLASSESVERBOSE
	printf ("start of MFInt32Constr\n");
	#endif

	ADD_ROOT(cx,obj)
	isArray = FALSE;
	if(argc == 1 && argv){
		//could it be new MFxxx( [A,B] ) javscript array, as used by Carlson aka Carlson Array
		// tests/JohnCarlson/Arc1A.x3d
		if (!JS_ValueToObject(cx, argv[0], &_arrayObj)) {
			printf( "JS_ValueToObject failed in MFVec3fConstr.\n");
			return JS_FALSE;
		}

		if(JS_IsArrayObject(cx, _arrayObj)){
			jsuint lengthp;
			jsval vp;
			//printf("its an array\n");
			isArray = TRUE;
			JS_GetArrayLength(cx,_arrayObj, &lengthp);
			argc = lengthp;
		}
	}

	if(SM_method() == 2){
		AnyNative *any;
		int newsize;
		if((any = (AnyNative*)AnyNativeNew(FIELDTYPE_MFInt32,NULL,NULL)) == NULL){
			printf( "AnyfNativeNew failed in MFInt32Constr.\n");
			return JS_FALSE;
		}
		if (!JS_SetPrivateFw(cx, obj, any)) {
			printf( "JS_SetPrivate failed in MFInt32Constr.\n");
			return JS_FALSE;
		}
		anyv = any->v;
		newsize = sizeof(int) * upper_power_of_two(argc); //newsize in bytes
		if(argc > 0){
			anyv->mfint32.p = MALLOC(int*,newsize);
			memset(anyv->mfint32.p,0,newsize);
		}

	}else{
		DEFINE_LENGTH(cx,obj,argc)
        DEFINE_MF_ECMA_HAS_CHANGED
	}
	if (!argv) {
		return JS_TRUE;
	}

	#ifdef JSVRMLCLASSESVERBOSE
		printf("MFInt32Constr: obj = %p, %u args\n", obj, argc);
	#endif

	/* any values here that we should add in? */
	for (i = 0; i < argc; i++) {
		jsval vp;
		if(isArray){
			JS_GetElement(cx, _arrayObj, i, &vp);

		}else{
			vp = argv[i];
		}
		//if (!JS::ToInt32(cx, vp, &_i)) {
		//if (!JS::ToInt32(cx, vp, &_i)) {
		if(!vp.isInt32()){
			printf( "JS_ValueToInt32 failed in MFInt32Constr.\n");
			return JS_FALSE;
		}
		_i = vp.toInt32();
		#ifdef JSVRMLCLASSESVERBOSE
		printf ("value at %d is %d\n",i,_i);
		#endif
		if(SM_method()==2){
			anyv->mfint32.p[i] = _i;
			anyv->mfint32.n = i+1;
		}else{
			if (!JS_DefineElement(cx, obj, (jsint) i, vp, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_CHECK, JSPROP_ENUMERATE)) {
				printf( "JS_DefineElement failed for arg %u in MFInt32Constr.\n", i);
				return JS_FALSE;
			}
		}
	}

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}

JSBool
MFInt32AddProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid,  JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid id = *hiid.address();
	jsval *vp = hvp.address();

	#ifdef JSVRMLCLASSESVERBOSE
	printf ("start of MFInt32AddProperty\n");
	#endif

	return doMFAddProperty(cx, obj, id, vp,"MFInt32AddProperty");
}

JSBool
MFInt32GetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid,  JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid id = *hiid.address();
	jsval *vp = hvp.address();
	
	#ifdef JSVRMLCLASSESVERBOSE
	printf ("start of MFInt32GetProperty\n");
	#endif

	return _standardMFGetProperty(cx, obj, id, vp,
			"_FreeWRL_Internal = 0", FIELDTYPE_MFInt32);
}

JSBool
MFInt32SetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid id = *hiid.address();
	jsval *vp = hvp.address();

	#ifdef JSVRMLCLASSESVERBOSE
	printf ("start of MFInt32SetProperty\n");
	#endif

	return doMFSetProperty(cx, obj, id, vp,FIELDTYPE_MFInt32);
}

// MFBool
JSBool
MFBoolToString(JSContext* cx, uintN argc, jsval* vp) {
	JSObject* obj = JS_THIS_OBJECT(cx, vp);
	jsval* argv = JS_ARGV(cx, vp);
	jsval rval;

	UNUSED(argc);
	UNUSED(argv);
#ifdef JSVRMLCLASSESVERBOSE
	printf("start of MFBoolToString\n");
#endif

	if (!doMFToString(cx, obj, "MFBool", &rval)) { return JS_FALSE; }
	JS_SET_RVAL(cx, vp, rval);
	return JS_TRUE;

}

JSBool
MFBoolAssign(JSContext* cx, uintN argc, jsval* vp) {
	JSObject* obj = JS_THIS_OBJECT(cx, vp);
	jsval* argv = JS_ARGV(cx, vp);
	jsval rval;

#ifdef JSVRMLCLASSESVERBOSE
	printf("start of MFBoolAssign\n");
#endif

	SET_MF_ECMA_HAS_CHANGED

		if (!_standardMFAssign(cx, obj, argc, argv, &rval, &MFBoolClass, FIELDTYPE_SFBool)) { return JS_FALSE; }
	JS_SET_RVAL(cx, vp, rval);
	return JS_TRUE;

}


JSBool
MFBoolConstr(JSContext* cx, uintN argc, jsval* vp) {
	JSObject* obj = JS_NewObject(cx, &MFBoolClass, NULL, NULL);
	jsval* argv = JS_ARGV(cx, vp);
	jsval rval = OBJECT_TO_JSVAL(obj);
	if (!MFBoolConstrInternals(cx, obj, argc, argv, &rval)) { return JS_FALSE; }
	JS_SET_RVAL(cx, vp, rval);
	return JS_TRUE;
}
JSBool MFBoolConstrInternals(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) {

	JSObject* _arrayObj;
	int isArray;
	int32 _i;
	unsigned int i;
	union anyVrml* anyv;
#ifdef JSVRMLCLASSESVERBOSE
	printf("start of MFBoolConstr\n");
#endif

	ADD_ROOT(cx, obj)
		isArray = FALSE;
	if (argc == 1 && argv) {
		//could it be new MFxxx( [A,B] ) javscript array, as used by Carlson aka Carlson Array
		// tests/JohnCarlson/Arc1A.x3d
		if (!JS_ValueToObject(cx, argv[0], &_arrayObj)) {
			printf("JS_ValueToObject failed in MFBoolConstr.\n");
			return JS_FALSE;
		}

		if (JS_IsArrayObject(cx, _arrayObj)) {
			jsuint lengthp;
			jsval vp;
			//printf("its an array\n");
			isArray = TRUE;
			JS_GetArrayLength(cx, _arrayObj, &lengthp);
			argc = lengthp;
		}
	}

	if (SM_method() == 2) {
		AnyNative* any;
		int newsize;
		if ((any = (AnyNative*)AnyNativeNew(FIELDTYPE_MFBool, NULL, NULL)) == NULL) {
			printf("AnyfNativeNew failed in MFBoolConstr.\n");
			return JS_FALSE;
		}
		if (!JS_SetPrivateFw(cx, obj, any)) {
			printf("JS_SetPrivate failed in MFBoolConstr.\n");
			return JS_FALSE;
		}
		anyv = any->v;
		newsize = sizeof(int) * upper_power_of_two(argc); //newsize in bytes
		if (argc > 0) {
			anyv->mfbool.p = MALLOC(int*, newsize);
			memset(anyv->mfbool.p, 0, newsize);
		}

	}
	else {
		DEFINE_LENGTH(cx, obj, argc)
			DEFINE_MF_ECMA_HAS_CHANGED
	}
	if (!argv) {
		return JS_TRUE;
	}

#ifdef JSVRMLCLASSESVERBOSE
	printf("MFBoolConstr: obj = %p, %u args\n", obj, argc);
#endif

	/* any values here that we should add in? */
	for (i = 0; i < argc; i++) {
		jsval vp;
		if (isArray) {
			JS_GetElement(cx, _arrayObj, i, &vp);

		}
		else {
			vp = argv[i];
		}
		//if (!JS::ToInt32(cx, vp, &_i)) {
		//if (!JS::ToInt32(cx, vp, &_i)) {
		if (!vp.isInt32()) {
			printf("JS_ValueToInt32 failed in MFBoolConstr.\n");
			return JS_FALSE;
		}
		_i = vp.toInt32();
#ifdef JSVRMLCLASSESVERBOSE
		printf("value at %d is %d\n", i, _i);
#endif
		if (SM_method() == 2) {
			anyv->mfbool.p[i] = _i;
			anyv->mfbool .n = i + 1;
		}
		else {
			if (!JS_DefineElement(cx, obj, (jsint)i, vp, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_CHECK, JSPROP_ENUMERATE)) {
				printf("JS_DefineElement failed for arg %u in MFBoolConstr.\n", i);
				return JS_FALSE;
			}
		}
	}

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}

JSBool
MFBoolAddProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp) {
	JSObject* obj = *hobj.address();
	jsid id = *hiid.address();
	jsval* vp = hvp.address();

#ifdef JSVRMLCLASSESVERBOSE
	printf("start of MFBoolAddProperty\n");
#endif

	return doMFAddProperty(cx, obj, id, vp, "MFBoolAddProperty");
}

JSBool
MFBoolGetProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp) {
	JSObject* obj = *hobj.address();
	jsid id = *hiid.address();
	jsval* vp = hvp.address();

#ifdef JSVRMLCLASSESVERBOSE
	printf("start of MFBoolGetProperty\n");
#endif

	return _standardMFGetProperty(cx, obj, id, vp,
		"_FreeWRL_Internal = 0", FIELDTYPE_MFBool);
}

JSBool
MFBoolSetProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp) {
	JSObject* obj = *hobj.address();
	jsid id = *hiid.address();
	jsval* vp = hvp.address();

#ifdef JSVRMLCLASSESVERBOSE
	printf("start of MFBoolSetProperty\n");
#endif

	return doMFSetProperty(cx, obj, id, vp, FIELDTYPE_MFBool);
}


// MFNode

JSBool
MFNodeToString(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;

	UNUSED(argc);
	UNUSED(argv);
	
	#ifdef JSVRMLCLASSESVERBOSE
	printf ("start of MFNODETOSTRING, obj %p\n",obj);
	#endif
	if (!doMFToString(cx, obj, "MFNode", &rval)) { return JS_FALSE; }
	JS_SET_RVAL(cx,vp,rval);
	return JS_TRUE;

}

JSBool
MFNodeAssign(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;

	#ifdef JSVRMLCLASSESVERBOSE
	printf ("start of MFNODEASSIGN, obj %p\n",obj);
	#endif

	if (!_standardMFAssign (cx, obj, argc, argv, &rval, &MFNodeClass,FIELDTYPE_SFNode)) { return JS_FALSE; }
	JS_SET_RVAL(cx,vp,rval);
	return JS_TRUE;

}

JSBool
MFNodeConstr(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_NewObject(cx,&MFNodeClass,NULL,NULL);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval = OBJECT_TO_JSVAL(obj);
        if (!MFNodeConstrInternals(cx,obj,argc,argv,&rval)) { return JS_FALSE; }
        JS_SET_RVAL(cx,vp,rval);
        return JS_TRUE;
}
JSBool MFNodeConstrInternals(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	JSObject *_arrayObj;
	int isArray;
	JSObject *_obj;
	unsigned int i;
	union anyVrml *anyv;

	ADD_ROOT(cx,obj)
	isArray = FALSE;
	if(argc == 1 && argv){
		//could it be new MFxxx( [A,B] ) javscript array, as used by Carlson aka Carlson Array
		// tests/JohnCarlson/Arc1A.x3d
		if (!JS_ValueToObject(cx, argv[0], &_arrayObj)) {
			printf( "JS_ValueToObject failed in MFVec3fConstr.\n");
			return JS_FALSE;
		}

		if(JS_IsArrayObject(cx, _arrayObj)){
			jsuint lengthp;
			jsval vp;
			//printf("its an array\n");
			isArray = TRUE;
			JS_GetArrayLength(cx,_arrayObj, &lengthp);
			argc = lengthp;
		}
	}

	if(SM_method() == 2){
		AnyNative *any;
		int newsize;
		if((any = (AnyNative*)AnyNativeNew(FIELDTYPE_MFNode,NULL,NULL)) == NULL){
			printf( "AnyfNativeNew failed in MFNodeConstr.\n");
			return JS_FALSE;
		}
		if (!JS_SetPrivateFw(cx, obj, any)) {
			printf( "JS_SetPrivate failed in MFNodeConstr.\n");
			return JS_FALSE;
		}
		anyv = any->v;
		newsize = sizeof(struct X3D_Node *) * upper_power_of_two(argc); //newsize in bytes
		if(argc > 0){
			anyv->mfnode.p = MALLOC(struct X3D_Node**,newsize);
			memset(anyv->mfnode.p,0,newsize);
		}

	}else{
		DEFINE_LENGTH(cx,obj,argc)
	}
	if (!argv) {
		return JS_TRUE;
	}

	#ifdef JSVRMLCLASSESVERBOSE
		printf("MFNodeConstr: obj = %p, %u args\n", obj, argc);
	#endif

	for (i = 0; i < argc; i++) {
		jsval vp;
		if(isArray){
			JS_GetElement(cx, _arrayObj, i, &vp);

		}else{
			vp = argv[i];
		}
		//if (JSVAL_IS_OBJECT(vp)) {
		if ((vp).isObject()) {

			if (!JS_ValueToObject(cx, vp, &_obj)) {
				printf( "JS_ValueToObject failed in MFNodeConstr.\n");
				return JS_FALSE;
			}

			CHECK_CLASS(cx,_obj,NULL,__FUNCTION__,SFNodeClass)
			if(SM_method()==2){
				AnyNative *any2;
				if((any2 = (AnyNative *)JS_GetPrivateFw(cx,_obj)) != NULL){
					if(any2->type == FIELDTYPE_SFNode){
						shallow_copy_field(FIELDTYPE_SFNode,any2->v,(union anyVrml*)&anyv->mfnode.p[i]);
						anyv->mfnode.n = i+1;
					}
				}
				// else for now we'll leave zeros
			}else{
				if (!JS_DefineElement(cx, obj, (jsint) i, vp, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_CHECK, JSPROP_ENUMERATE)) {
					printf( "JS_DefineElement failed for arg %d in MFNodeConstr.\n", i);
					return JS_FALSE;
				}
			}
		} else {
			/* if a NULL is passed in, eg, we have a script with an MFNode eventOut, and
			   nothing sets it, we have a NULL here. Lets just ignore it */
			/* hmmm - this is not an object - lets see... */
			#ifdef JSVRMLCLASSESVERBOSE
			if (JSVAL_IS_NULL(argv[i])) { printf ("MFNodeConstr - its a NULL\n");}
			if (JSVAL_IS_INT(argv[i])) { printf ("MFNodeConstr - its a INT\n");}
			if (JSVAL_IS_STRING(argv[i])) { printf ("MFNodeConstr - its a STRING\n");}
			#endif
		}
	}
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}

JSBool
MFNodeAddProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid,  JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid id = *hiid.address();
	jsval *vp = hvp.address();

	#ifdef JSVRMLCLASSESVERBOSE
	printf ("startof MFNODEADDPROPERTY\n");
	#endif
	return doMFAddProperty(cx, obj, id, vp,"MFNodeAddProperty");
}

JSBool
MFNodeGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid,  JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid id = *hiid.address();
	jsval *vp = hvp.address();

	#ifdef JSVRMLCLASSESVERBOSE
	printf ("start of MFNODEGETPROPERTY obj %p\n",obj);
	#endif
	return _standardMFGetProperty(cx, obj, id, vp,
			"_FreeWRL_Internal = 0",
			FIELDTYPE_MFNode);
}

JSBool
MFNodeSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid id = *hiid.address();
	jsval *vp = hvp.address();
	/* printf ("start of MFNODESETPROPERTY obj %d\n",obj); */
	return doMFSetProperty(cx, obj, id, vp,FIELDTYPE_MFNode);
}

// MFTime

JSBool
MFTimeAddProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid,  JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid id = *hiid.address();
	jsval *vp = hvp.address();

	return doMFAddProperty(cx, obj, id, vp,"MFTimeAddProperty");
}

JSBool
MFTimeGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid,  JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid id = *hiid.address();
	jsval *vp = hvp.address();

	return _standardMFGetProperty(cx, obj, id, vp,
			 "_FreeWRL_Internal = 0.0",
			FIELDTYPE_MFTime);
}

JSBool
MFTimeSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid id = *hiid.address();
	jsval *vp = hvp.address();

	return doMFSetProperty(cx, obj, id, vp,FIELDTYPE_MFTime);
}

JSBool
MFTimeToString(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
	jsval rval;

	UNUSED(argc);
	UNUSED(argv);
	if (!doMFToString(cx, obj, "MFTime", &rval)) { return JS_FALSE; }
	JS_SET_RVAL(cx,vp,rval);
	return JS_TRUE;

}

JSBool
MFTimeConstr(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_NewObject(cx,&MFTimeClass,NULL,NULL);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval = OBJECT_TO_JSVAL(obj);
        if (!MFTimeConstrInternals(cx,obj,argc,argv,&rval)) { return JS_FALSE; }
        JS_SET_RVAL(cx,vp,rval);
        return JS_TRUE;
}
JSBool MFTimeConstrInternals(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	JSObject *_arrayObj;
	int isArray;
	jsdouble _d;
	unsigned int i;
	union anyVrml *anyv;

	ADD_ROOT(cx,obj)
	isArray = FALSE;
	if(argc == 1 && argv){
		//could it be new MFxxx( [A,B] ) javscript array, as used by Carlson aka Carlson Array
		// tests/JohnCarlson/Arc1A.x3d
		if (!JS_ValueToObject(cx, argv[0], &_arrayObj)) {
			printf( "JS_ValueToObject failed in MFVec3fConstr.\n");
			return JS_FALSE;
		}

		if(JS_IsArrayObject(cx, _arrayObj)){
			jsuint lengthp;
			jsval vp;
			//printf("its an array\n");
			isArray = TRUE;
			JS_GetArrayLength(cx,_arrayObj, &lengthp);
			argc = lengthp;
		}
	}

	if(SM_method() == 2){
		AnyNative *any;
		int newsize;
		if((any = (AnyNative*)AnyNativeNew(FIELDTYPE_MFTime,NULL,NULL)) == NULL){
			printf( "AnyfNativeNew failed in MFTimeConstr.\n");
			return JS_FALSE;
		}
		if (!JS_SetPrivateFw(cx, obj, any)) {
			printf( "JS_SetPrivate failed in MFTimeConstr.\n");
			return JS_FALSE;
		}
		anyv = any->v;
		newsize = sizeof(double) * upper_power_of_two(argc); //newsize in bytes
		if(argc > 0){
			anyv->mftime.p = MALLOC(double*,newsize);
			memset(anyv->mftime.p,0,newsize);
		}

	}else{
		DEFINE_LENGTH(cx,obj,argc)
		DEFINE_MF_ECMA_HAS_CHANGED
	}
	if (!argv) {
		return JS_TRUE;
	}

	#ifdef JSVRMLCLASSESVERBOSE
		printf("MFTimeConstr: obj = %p, %u args\n", obj, argc);
	#endif
	for (i = 0; i < argc; i++) {
		jsval vp;
		if(isArray){
			JS_GetElement(cx, _arrayObj, i, &vp);

		}else{
			vp = argv[i];
		}
		if (!JS_ValueToNumber(cx, vp, &_d)) {
			printf(
					"JS_ValueToNumber failed in MFTimeConstr.\n");
			return JS_FALSE;
		}
		if(SM_method()==2){
			anyv->mftime.p[i] = _d;
			anyv->mftime.n = i+1;
		}else{
			if (!JS_DefineElement(cx, obj, (jsint) i, vp, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_CHECK, JSPROP_ENUMERATE)) {
				printf( "JS_DefineElement failed for arg %u in MFTimeConstr.\n", i);
				return JS_FALSE;
			}
		}
	}
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}

JSBool
MFTimeAssign(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;

	SET_MF_ECMA_HAS_CHANGED

	if (!_standardMFAssign (cx, obj, argc, argv, &rval, &MFTimeClass,FIELDTYPE_SFTime)) { return JS_FALSE; }
	JS_SET_RVAL(cx,vp,rval);
	return JS_TRUE;

}

// MFDouble

JSBool
MFDoubleAddProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp) {
	JSObject* obj = *hobj.address();
	jsid id = *hiid.address();
	jsval* vp = hvp.address();

	return doMFAddProperty(cx, obj, id, vp, "MFDoubleAddProperty");
}

JSBool
MFDoubleGetProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp) {
	JSObject* obj = *hobj.address();
	jsid id = *hiid.address();
	jsval* vp = hvp.address();

	return _standardMFGetProperty(cx, obj, id, vp,
		"_FreeWRL_Internal = 0.0",
		FIELDTYPE_MFDouble);
}

JSBool
MFDoubleSetProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp) {
	JSObject* obj = *hobj.address();
	jsid id = *hiid.address();
	jsval* vp = hvp.address();

	return doMFSetProperty(cx, obj, id, vp, FIELDTYPE_MFDouble);
}

JSBool
MFDoubleToString(JSContext* cx, uintN argc, jsval* vp) {
	JSObject* obj = JS_THIS_OBJECT(cx, vp);
	jsval* argv = JS_ARGV(cx, vp);
	jsval rval;

	UNUSED(argc);
	UNUSED(argv);
	if (!doMFToString(cx, obj, "MFDouble", &rval)) { return JS_FALSE; }
	JS_SET_RVAL(cx, vp, rval);
	return JS_TRUE;

}

JSBool
MFDoubleConstr(JSContext* cx, uintN argc, jsval* vp) {
	JSObject* obj = JS_NewObject(cx, &MFDoubleClass, NULL, NULL);
	jsval* argv = JS_ARGV(cx, vp);
	jsval rval = OBJECT_TO_JSVAL(obj);
	if (!MFDoubleConstrInternals(cx, obj, argc, argv, &rval)) { return JS_FALSE; }
	JS_SET_RVAL(cx, vp, rval);
	return JS_TRUE;
}
JSBool MFDoubleConstrInternals(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) {

	JSObject* _arrayObj;
	int isArray;
	jsdouble _d;
	unsigned int i;
	union anyVrml* anyv;

	ADD_ROOT(cx, obj)
		isArray = FALSE;
	if (argc == 1 && argv) {
		//could it be new MFxxx( [A,B] ) javscript array, as used by Carlson aka Carlson Array
		// tests/JohnCarlson/Arc1A.x3d
		if (!JS_ValueToObject(cx, argv[0], &_arrayObj)) {
			printf("JS_ValueToObject failed in MFVec3fConstr.\n");
			return JS_FALSE;
		}

		if (JS_IsArrayObject(cx, _arrayObj)) {
			jsuint lengthp;
			jsval vp;
			//printf("its an array\n");
			isArray = TRUE;
			JS_GetArrayLength(cx, _arrayObj, &lengthp);
			argc = lengthp;
		}
	}

	if (SM_method() == 2) {
		AnyNative* any;
		int newsize;
		if ((any = (AnyNative*)AnyNativeNew(FIELDTYPE_MFDouble, NULL, NULL)) == NULL) {
			printf("AnyfNativeNew failed in MFTimeConstr.\n");
			return JS_FALSE;
		}
		if (!JS_SetPrivateFw(cx, obj, any)) {
			printf("JS_SetPrivate failed in MFDoubleConstr.\n");
			return JS_FALSE;
		}
		anyv = any->v;
		newsize = sizeof(double) * upper_power_of_two(argc); //newsize in bytes
		if (argc > 0) {
			anyv->mfdouble.p = MALLOC(double*, newsize);
			memset(anyv->mfdouble.p, 0, newsize);
		}

	}
	else {
		DEFINE_LENGTH(cx, obj, argc)
			DEFINE_MF_ECMA_HAS_CHANGED
	}
	if (!argv) {
		return JS_TRUE;
	}

#ifdef JSVRMLCLASSESVERBOSE
	printf("MFDoubleConstr: obj = %p, %u args\n", obj, argc);
#endif
	for (i = 0; i < argc; i++) {
		jsval vp;
		if (isArray) {
			JS_GetElement(cx, _arrayObj, i, &vp);

		}
		else {
			vp = argv[i];
		}
		if (!JS_ValueToNumber(cx, vp, &_d)) {
			printf(
				"JS_ValueToNumber failed in MFDoubleConstr.\n");
			return JS_FALSE;
		}
		if (SM_method() == 2) {
			anyv->mfdouble.p[i] = _d;
			anyv->mfdouble.n = i + 1;
		}
		else {
			if (!JS_DefineElement(cx, obj, (jsint)i, vp, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_CHECK, JSPROP_ENUMERATE)) {
				printf("JS_DefineElement failed for arg %u in MFTimeConstr.\n", i);
				return JS_FALSE;
			}
		}
	}
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}

JSBool
MFDoubleAssign(JSContext* cx, uintN argc, jsval* vp) {
	JSObject* obj = JS_THIS_OBJECT(cx, vp);
	jsval* argv = JS_ARGV(cx, vp);
	jsval rval;

	SET_MF_ECMA_HAS_CHANGED

		if (!_standardMFAssign(cx, obj, argc, argv, &rval, &MFDoubleClass, FIELDTYPE_SFDouble)) { return JS_FALSE; }
	JS_SET_RVAL(cx, vp, rval);
	return JS_TRUE;

}

// MFVec2f

JSBool
MFVec2fAddProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid,  JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid id = *hiid.address();
	jsval *vp = hvp.address();

	return doMFAddProperty(cx, obj, id, vp,"MFVec2fAddProperty");
}

JSBool
MFVec2fGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid,  JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid id = *hiid.address();
	jsval *vp = hvp.address();

	return _standardMFGetProperty(cx, obj, id, vp,
			 "_FreeWRL_Internal = new SFVec2f()",FIELDTYPE_MFVec2f);
}

JSBool
MFVec2fSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid id = *hiid.address();
	jsval *vp = hvp.address();

	return doMFSetProperty(cx, obj, id, vp,FIELDTYPE_MFVec2f);
}

JSBool
MFVec2fToString(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;

	UNUSED(argc);
	UNUSED(argv);
	if (!doMFToString(cx, obj, "MFVec2f", &rval)) { return JS_FALSE; }
	JS_SET_RVAL(cx,vp,rval);
	return JS_TRUE;

}

JSBool
MFVec2fConstr(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_NewObject(cx,&MFVec2fClass,NULL,NULL);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval = OBJECT_TO_JSVAL(obj);
        if (!MFVec2fConstrInternals(cx,obj,argc,argv,&rval)) { return JS_FALSE; }
        JS_SET_RVAL(cx,vp,rval);
        return JS_TRUE;
}
JSBool MFVec2fConstrInternals(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	JSObject *_arrayObj;
	int isArray;
	JSObject *_obj;
	unsigned int i;
	union anyVrml *anyv;

	ADD_ROOT(cx,obj)
	isArray = FALSE;
	if(argc == 1 && argv){
		//could it be new MFxxx( [A,B] ) javscript array, as used by Carlson aka Carlson Array
		// tests/JohnCarlson/Arc1A.x3d
		if (!JS_ValueToObject(cx, argv[0], &_arrayObj)) {
			printf( "JS_ValueToObject failed in MFVec3fConstr.\n");
			return JS_FALSE;
		}

		if(JS_IsArrayObject(cx, _arrayObj)){
			jsuint lengthp;
			jsval vp;
			//printf("its an array\n");
			isArray = TRUE;
			JS_GetArrayLength(cx,_arrayObj, &lengthp);
			argc = lengthp;
		}
	}

	if(SM_method() == 2){
		AnyNative *any;
		int newsize;
		if((any = (AnyNative*)AnyNativeNew(FIELDTYPE_MFVec2f,NULL,NULL)) == NULL){
			printf( "AnyfNativeNew failed in MFVec2fConstr.\n");
			return JS_FALSE;
		}
		if (!JS_SetPrivateFw(cx, obj, any)) {
			printf( "JS_SetPrivate failed in MFVec2fConstr.\n");
			return JS_FALSE;
		}
		anyv = any->v;
		newsize = sizeof(struct SFVec2f)*upper_power_of_two(argc);
		if(argc > 0){
			anyv->mfvec2f.p = MALLOC(struct SFVec2f*,newsize);
			memset(anyv->mfvec2f.p,0,newsize);
		}

	}else{
		DEFINE_LENGTH(cx,obj,argc)
	}

	if (!argv) {
		return JS_TRUE;
	}

	#ifdef JSVRMLCLASSESVERBOSE
		printf("MFVec2fConstr: obj = %p, %u args\n", obj, argc);
	#endif

	for (i = 0; i < argc; i++) {
		jsval vp;
		if(isArray){
			JS_GetElement(cx, _arrayObj, i, &vp);

		}else{
			vp = argv[i];
		}
		if (!JS_ValueToObject(cx, vp, &_obj)) {
			printf( "JS_ValueToObject failed in MFVec2fConstr.\n");
			return JS_FALSE;
		}

		CHECK_CLASS(cx,_obj,NULL,__FUNCTION__,SFVec2fClass)
		if(SM_method()==2){
			AnyNative *any2;
			if((any2 = (AnyNative *)JS_GetPrivateFw(cx,_obj)) != NULL){
				//2018 I think as long as its 2+ contiguous floats, we can use it as a vec2f, 
				// but in future internal types might change
				if(any2->type == FIELDTYPE_SFVec2f || any2->type == FIELDTYPE_SFColor || any2->type == FIELDTYPE_SFVec3f || any2->type == FIELDTYPE_SFColorRGBA){
					shallow_copy_field(FIELDTYPE_SFVec2f,any2->v,(union anyVrml*)&anyv->mfvec2f.p[i]);
					anyv->mfvec2f.n = i+1;
				}
			}
			// else for now we'll leave zeros
		}else{
			if (!JS_DefineElement(cx, obj, (jsint) i, vp, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_CHECK, JSPROP_ENUMERATE)) {
				printf( "JS_DefineElement failed for arg %d in MFVec2fConstr.\n", i);
				return JS_FALSE;
			}
		}
	}
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}

JSBool
MFVec2fAssign(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;
        if (!_standardMFAssign (cx, obj, argc, argv, &rval, &MFVec2fClass,FIELDTYPE_SFVec2f)) { return JS_FALSE; }
        JS_SET_RVAL(cx,vp,rval);
        return JS_TRUE;

}

// MFVec3f 
JSBool
MFVec3fAddProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid,  JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid id = *hiid.address();
	jsval *vp = hvp.address();
	return doMFAddProperty(cx, obj, id, vp,"MFVec3fAddProperty");
}

JSBool
MFVec3fGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid,  JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid id = *hiid.address();
	jsval *vp = hvp.address();

	return _standardMFGetProperty(cx, obj, id, vp,
			 "_FreeWRL_Internal = new SFVec3f()",FIELDTYPE_MFVec3f);
}

JSBool
MFVec3fSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid id = *hiid.address();
	jsval *vp = hvp.address();

	return doMFSetProperty(cx, obj, id, vp,FIELDTYPE_MFVec3f);
}

JSBool
MFVec3fToString(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;

	UNUSED(argc);
	UNUSED(argv);
	/* printf ("CALLED MFVec3fToString\n");*/
        if (!doMFToString(cx, obj, "MFVec3f", &rval)) { return JS_FALSE; }
        JS_SET_RVAL(cx,vp,rval);
        return JS_TRUE;

}

JSBool
MFVec3fConstr(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_NewObject(cx,&MFVec3fClass,NULL,NULL);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval = OBJECT_TO_JSVAL(obj);
        if (!MFVec3fConstrInternals(cx,obj,argc,argv,&rval)) { return JS_FALSE; }
        JS_SET_RVAL(cx,vp,rval);
        return JS_TRUE;
}
JSBool MFVec3fConstrInternals(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	JSObject *_arrayObj;
	int isArray;
	JSObject *_obj;
	unsigned int i;
	union anyVrml *anyv;

	ADD_ROOT(cx,obj)

	isArray = FALSE;
	if(argc == 1 && argv){
		//could it be new MFxxx( [A,B] ) javscript array, as used by Carlson aka Carlson Array
		// tests/JohnCarlson/Arc1A.x3d
		if (!JS_ValueToObject(cx, argv[0], &_arrayObj)) {
			printf( "JS_ValueToObject failed in MFVec3fConstr.\n");
			return JS_FALSE;
		}

		if(JS_IsArrayObject(cx, _arrayObj)){
			jsuint lengthp;
			jsval vp;
			//printf("its an array\n");
			isArray = TRUE;
			JS_GetArrayLength(cx,_arrayObj, &lengthp);
			argc = lengthp;
		}
	}



	if(SM_method() == 2){
		AnyNative *any;
		int newsize;
		if((any = (AnyNative*)AnyNativeNew(FIELDTYPE_MFVec3f,NULL,NULL)) == NULL){
			printf( "AnyfNativeNew failed in MFVec3fConstr.\n");
			return JS_FALSE;
		}
		if (!JS_SetPrivateFw(cx, obj, any)) {
			printf( "JS_SetPrivate failed in MFVec3fConstr.\n");
			return JS_FALSE;
		}
		anyv = any->v;
		newsize = sizeof(struct SFVec3f)*upper_power_of_two(argc);
		if(argc > 0){
			anyv->mfvec3f.p = MALLOC(struct SFVec3f*,newsize);
			memset(anyv->mfvec3f.p,0,newsize);
		}

	}else{
		DEFINE_LENGTH(cx,obj,argc)
	}
	if (!argv) {
		return JS_TRUE;
	}

	#ifdef JSVRMLCLASSESVERBOSE
		printf("MFVec3fConstr: obj = %p, %u args\n", obj, argc);
	#endif	
	for (i = 0; i < argc; i++) {
		jsval vp;
		if(isArray){
			JS_GetElement(cx, _arrayObj, i, &vp);

		}else{
			vp = argv[i];
		}
		if (!JS_ValueToObject(cx, vp, &_obj)) {
			printf( "JS_ValueToObject failed in MFVec3fConstr.\n");
			return JS_FALSE;
		}

		CHECK_CLASS(cx,_obj,NULL,__FUNCTION__,SFVec3fClass)

		if(SM_method()==2){
			AnyNative *any2;
			if((any2 = (AnyNative *)JS_GetPrivateFw(cx,_obj)) != NULL){
				//2018 I think as long as its 3+ contiguous floats, we can use it as a vec2f, 
				// but in future internal types might change
				if(any2->type == FIELDTYPE_SFVec3f || any2->type == FIELDTYPE_SFColor || any2->type == FIELDTYPE_SFColorRGBA){
					shallow_copy_field(FIELDTYPE_SFVec3f,any2->v,(union anyVrml*)&anyv->mfvec3f.p[i]);
					anyv->mfvec3f.n = i+1;
				}
			}
			// else for now we'll leave zeros
		}else{

			if (!JS_DefineElement(cx, obj, (jsint) i, vp, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_CHECK, JSPROP_ENUMERATE)) {
				printf( "JS_DefineElement failed for arg %d in MFVec3fConstr.\n", i);
				return JS_FALSE;
			}
		}
	}
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}

JSBool
MFVec3fAssign(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;
        if (!_standardMFAssign (cx, obj, argc, argv, &rval, &MFVec3fClass,FIELDTYPE_SFVec3f)) { return JS_FALSE; }
        JS_SET_RVAL(cx,vp,rval);
        return JS_TRUE;
}


// MFVec4f
JSBool
MFVec4fAddProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp) {
	JSObject* obj = *hobj.address();
	jsid id = *hiid.address();
	jsval* vp = hvp.address();
	return doMFAddProperty(cx, obj, id, vp, "MFVec4fAddProperty");
}

JSBool
MFVec4fGetProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp) {
	JSObject* obj = *hobj.address();
	jsid id = *hiid.address();
	jsval* vp = hvp.address();

	return _standardMFGetProperty(cx, obj, id, vp,
		"_FreeWRL_Internal = new SFVec4f()", FIELDTYPE_MFVec4f);
}

JSBool
MFVec4fSetProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp) {
	JSObject* obj = *hobj.address();
	jsid id = *hiid.address();
	jsval* vp = hvp.address();

	return doMFSetProperty(cx, obj, id, vp, FIELDTYPE_MFVec4f);
}

JSBool
MFVec4fToString(JSContext* cx, uintN argc, jsval* vp) {
	JSObject* obj = JS_THIS_OBJECT(cx, vp);
	jsval* argv = JS_ARGV(cx, vp);
	jsval rval;

	UNUSED(argc);
	UNUSED(argv);
	/* printf ("CALLED MFVec4fToString\n");*/
	if (!doMFToString(cx, obj, "MFVec4f", &rval)) { return JS_FALSE; }
	JS_SET_RVAL(cx, vp, rval);
	return JS_TRUE;

}

JSBool
MFVec4fConstr(JSContext* cx, uintN argc, jsval* vp) {
	JSObject* obj = JS_NewObject(cx, &MFVec4fClass, NULL, NULL);
	jsval* argv = JS_ARGV(cx, vp);
	jsval rval = OBJECT_TO_JSVAL(obj);
	if (!MFVec4fConstrInternals(cx, obj, argc, argv, &rval)) { return JS_FALSE; }
	JS_SET_RVAL(cx, vp, rval);
	return JS_TRUE;
}
JSBool MFVec4fConstrInternals(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) {

	JSObject* _arrayObj;
	int isArray;
	JSObject* _obj;
	unsigned int i;
	union anyVrml* anyv;

	ADD_ROOT(cx, obj)

		isArray = FALSE;
	if (argc == 1 && argv) {
		//could it be new MFxxx( [A,B] ) javscript array, as used by Carlson aka Carlson Array
		// tests/JohnCarlson/Arc1A.x3d
		if (!JS_ValueToObject(cx, argv[0], &_arrayObj)) {
			printf("JS_ValueToObject failed in MFVec4fConstr.\n");
			return JS_FALSE;
		}

		if (JS_IsArrayObject(cx, _arrayObj)) {
			jsuint lengthp;
			jsval vp;
			//printf("its an array\n");
			isArray = TRUE;
			JS_GetArrayLength(cx, _arrayObj, &lengthp);
			argc = lengthp;
		}
	}



	if (SM_method() == 2) {
		AnyNative* any;
		int newsize;
		if ((any = (AnyNative*)AnyNativeNew(FIELDTYPE_MFVec4f, NULL, NULL)) == NULL) {
			printf("AnyfNativeNew failed in MFVec4fConstr.\n");
			return JS_FALSE;
		}
		if (!JS_SetPrivateFw(cx, obj, any)) {
			printf("JS_SetPrivate failed in MFVec4fConstr.\n");
			return JS_FALSE;
		}
		anyv = any->v;
		newsize = sizeof(struct SFVec4f) * upper_power_of_two(argc);
		if (argc > 0) {
			anyv->mfvec4f.p = MALLOC(struct SFVec4f*, newsize);
			memset(anyv->mfvec4f.p, 0, newsize);
		}

	}
	else {
		DEFINE_LENGTH(cx, obj, argc)
	}
	if (!argv) {
		return JS_TRUE;
	}

#ifdef JSVRMLCLASSESVERBOSE
	printf("MFVec4fConstr: obj = %p, %u args\n", obj, argc);
#endif	
	for (i = 0; i < argc; i++) {
		jsval vp;
		if (isArray) {
			JS_GetElement(cx, _arrayObj, i, &vp);

		}
		else {
			vp = argv[i];
		}
		if (!JS_ValueToObject(cx, vp, &_obj)) {
			printf("JS_ValueToObject failed in MFVec4fConstr.\n");
			return JS_FALSE;
		}

		CHECK_CLASS(cx, _obj, NULL, __FUNCTION__, SFVec4fClass)

			if (SM_method() == 2) {
				AnyNative* any2;
				if ((any2 = (AnyNative*)JS_GetPrivateFw(cx, _obj)) != NULL) {
					//2018 I think as long as its 3+ contiguous floats, we can use it as a vec2f, 
					// but in future internal types might change
					if (any2->type == FIELDTYPE_SFVec4f || any2->type == FIELDTYPE_SFColorRGBA) {
						shallow_copy_field(FIELDTYPE_SFVec4f, any2->v, (union anyVrml*)&anyv->mfvec4f.p[i]);
						anyv->mfvec4f.n = i + 1;
					}
				}
				// else for now we'll leave zeros
			}
			else {

				if (!JS_DefineElement(cx, obj, (jsint)i, vp, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_CHECK, JSPROP_ENUMERATE)) {
					printf("JS_DefineElement failed for arg %d in MFVec4fConstr.\n", i);
					return JS_FALSE;
				}
			}
	}
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}

JSBool
MFVec4fAssign(JSContext* cx, uintN argc, jsval* vp) {
	JSObject* obj = JS_THIS_OBJECT(cx, vp);
	jsval* argv = JS_ARGV(cx, vp);
	jsval rval;
	if (!_standardMFAssign(cx, obj, argc, argv, &rval, &MFVec4fClass, FIELDTYPE_SFVec4f)) { return JS_FALSE; }
	JS_SET_RVAL(cx, vp, rval);
	return JS_TRUE;
}


// VrmlMatrix 

static void _setmatrix (JSContext *cx, JSObject *obj, double *matrix) {
	jsval val;
	int i;
	for (i=0; i<16; i++) {

		if (JS_NewNumberValue(cx, matrix[i],&val) == JS_FALSE) {
			printf ("problem creating id matrix\n");
			return;
		}

		if (!JS_SetElement(cx, obj, (jsint) i, &val)) {
			printf( "JS_DefineElement failed for arg %u in VrmlMatrixSetTransform.\n", i);
			return;
		}
	}
}

/* get the matrix values into a double array */
static void _getmatrix (JSContext *cx, JSObject *obj, double *fl) {
	int32 _length;
	jsval _length_val;
	jsval val;
	int i;
	double d;

	if (!JS_GetProperty(cx, obj,  MF_LENGTH_FIELD, &_length_val)) {
		printf( "JS_GetProperty failed for \"%s\" in _getmatrix.\n", MF_LENGTH_FIELD);
		_length = 0;
	} else {
		_length = JSVAL_TO_INT(_length_val);
	}

	#ifdef JSVRMLCLASSESVERBOSE
	printf ("_getmatrix, length %d\n",_length);
	#endif


	if (_length>16) _length = 16;

	for (i = 0; i < _length; i++) {
		if (!JS_GetElement(cx, obj, (jsint) i, &val)) {
			printf( "failed in get of copyElements index %d.\n", i);
			fl[i] = 0.0;
		} else {
			if (!JS_ValueToNumber(cx, val, &d)) {
				printf ("this is not a mumber!\n");
				fl[i]=0.0;
			} else fl[i]=d;
		}
	}

	/* in case our matrix was short for some reason */
	for (i=_length; i < 16; i++) {
		fl[i]=0.0;
	}
}


JSBool
VrmlMatrixToString(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;
	UNUSED(argc);
	UNUSED(argv);

        if (!doMFToString(cx, obj, "MFFloat", &rval)) { return JS_FALSE; }
        JS_SET_RVAL(cx,vp,rval);
        return JS_TRUE;
}

/* get rows; used for scale and rot in getTransform */
void _get4f(double *ret, double *mat, int row) {
	if (row == 0) {ret[0]=MAT00;ret[1]=MAT01;ret[2]=MAT02;ret[3]=MAT03;}
	if (row == 1) {ret[0]=MAT10;ret[1]=MAT11;ret[2]=MAT12;ret[3]=MAT13;}
	if (row == 2) {ret[0]=MAT20;ret[1]=MAT21;ret[2]=MAT22;ret[3]=MAT23;}
}

/* set rows; used for scale and rot in getTransform */
void _set4f(double len, double *mat, int row) {
	if (row == 0) {MAT00=MAT00/len;MAT01=MAT01/len;MAT02=MAT02/len;MAT03=MAT03/len;}
	if (row == 1) {MAT10=MAT10/len;MAT11=MAT11/len;MAT12=MAT12/len;MAT13=MAT13/len;}
	if (row == 2) {MAT20=MAT20/len;MAT21=MAT21/len;MAT22=MAT22/len;MAT23=MAT23/len;}
}

JSBool
VrmlMatrixgetTransform(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
	int i;
    	JSObject *transObj = NULL;
	JSObject *rotObj = NULL;
	JSObject *scaleObj = NULL;
	SFRotationNative *Rptr;
	SFVec3fNative *Vptr;

    	Quaternion quat;
    	double matrix[16];
    	double qu[4];
	double r0[4], r1[4], r2[4];
	double l0,l1,l2;

	/* some intermediate calculations */
	_getmatrix(cx,obj,matrix);
	/* get each row */
	_get4f(r0,matrix,0);
	_get4f(r1,matrix,1);
	_get4f(r2,matrix,2);
	/* get the length of each row */
	l0 = sqrt(r0[0]*r0[0] + r0[1]*r0[1] + r0[2]*r0[2] +r0[3]*r0[3]);
	l1 = sqrt(r1[0]*r1[0] + r1[1]*r1[1] + r1[2]*r1[2] +r1[3]*r1[3]);
	l2 = sqrt(r2[0]*r2[0] + r2[1]*r2[1] + r2[2]*r2[2] +r2[3]*r2[3]);

	if (argc == 1) {
		if (!JS_ConvertArguments(cx, argc, argv, "o", &transObj)) {
			printf ("getTransform, invalid parameters\n");
			return JS_FALSE;
		}
	}
	if (argc == 2) {
		if (!JS_ConvertArguments(cx, argc, argv, "o o", &transObj, &rotObj)) {
			printf ("getTransform, invalid parameters\n");
			return JS_FALSE;
		}
	}
	if (argc == 3) {
		if (!JS_ConvertArguments(cx, argc, argv, "o o o",
					&transObj,&rotObj,&scaleObj)) {
			printf ("getTransform, invalid parameters\n");
			return JS_FALSE;
		}
	}

	/* translation */
	if (transObj!=NULL) {
		CHECK_CLASS(cx,transObj,NULL,__FUNCTION__,SFVec3fClass)

		if ((Vptr = (SFVec3fNative *)JS_GetPrivateFw(cx, transObj)) == NULL) {
			printf( "JS_GetPrivate failed.\n");
			return JS_FALSE;
		}
		(Vptr->v).c[0] = (float) matrix[12];
		(Vptr->v).c[1] = (float) matrix[13];
		(Vptr->v).c[2] = (float) matrix[14];
		Vptr->valueChanged++;
	}

	/* rotation */
	if (rotObj!=NULL) {

		CHECK_CLASS(cx,rotObj,NULL,__FUNCTION__,SFRotationClass)

		if ((Rptr = (SFRotationNative*)JS_GetPrivateFw(cx, rotObj)) == NULL) {
			printf( "JS_GetPrivate failed.\n");
			return JS_FALSE;
		}

		/* apply length to each row */
		_set4f(l0, matrix, 0);
		_set4f(l1, matrix, 1);
		_set4f(l2, matrix, 2);

		/* convert the matrix to a quaternion */
		matrix_to_quaternion (&quat, matrix);
		#ifdef JSVRMLCLASSESVERBOSE
		printf ("quaternion %f %f %f %f\n",quat.x,quat.y,quat.z,quat.w);
		#endif

		/* convert the quaternion to a VRML rotation */
		quaternion_to_vrmlrot(&quat, &qu[0],&qu[1],&qu[2],&qu[3]);

		/* now copy the values over */
		for (i=0; i<4; i++) (Rptr->v).c[i] = (float) qu[i];
		Rptr->valueChanged = 1;
	}

	/* scale */
	if (scaleObj != NULL) {
		CHECK_CLASS(cx,scaleObj,NULL,__FUNCTION__,SFVec3fClass)

		if ((Vptr = (SFVec3fNative*)JS_GetPrivateFw(cx, scaleObj)) == NULL) {
			printf( "JS_GetPrivate failed.\n");
			return JS_FALSE;
		}
		(Vptr->v).c[0] = (float) l0;
		(Vptr->v).c[1] = (float) l1;
		(Vptr->v).c[2] = (float) l2;
		Vptr->valueChanged = 1;
	}

	JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(NULL)); //JSVAL_VOID);

	return JS_TRUE;
}


/* Sets the VrmlMatrix to the passed values. Any of the rightmost parameters may be omitted. 
   The method has 0 to 5 parameters. For example, specifying 0 parameters results in an 
   identity matrix while specifying 1 parameter results in a translation and specifying 2 
   parameters results in a translation and a rotation. Any unspecified parameter is set to 
   its default as specified for the Transform node. */

JSBool
VrmlMatrixsetTransform(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
    	JSObject *transObj = NULL;
	JSObject *rotObj = NULL;
	JSObject *scaleObj = NULL;
	JSObject *scaleOObj = NULL;
	JSObject *centerObj = NULL;

    	double matrix[16];

	int error = FALSE;

#undef TESTING
#ifdef TESTING
	GLDOUBLE xxmat[16];
	FW_GL_MATRIX_MODE(GL_MODELVIEW);
	FW_GL_PUSH_MATRIX();
	FW_GL_LOAD_IDENTITY();
#endif


	/* set the identity for this matrix. We work on this matrix, then assign it to the variable */
	loadIdentityMatrix(matrix);

	/* first, is this a VrmlMatrix object? The chances of this failing are slim to none... */
	if (!JS_InstanceOf(cx, obj, &VrmlMatrixClass, NULL)) {
		error = TRUE;
	} else {
		if (argc == 1) {
			error = !JS_ConvertArguments(cx, argc, argv, "o", &transObj); 
		}
		if (argc == 2) {
			error = !JS_ConvertArguments(cx, argc, argv, "o o", &transObj,
				&rotObj);
		}
		if (argc == 3) {
			error = !JS_ConvertArguments(cx, argc, argv, "o o o",
				&transObj,&rotObj,&scaleObj);
		}
		if (argc == 4) {
			error = !JS_ConvertArguments(cx, argc, argv, "o o o o",
				&transObj,&rotObj,&scaleObj,&scaleOObj);
		}
		if (argc == 5) {
			error = !JS_ConvertArguments(cx, argc, argv, "o o o o o",
				&transObj,&rotObj,&scaleObj,&scaleOObj,&centerObj);
		}
		if (argc > 5) { error = TRUE; }
	}

	if (error) {
		ConsoleMessage ("setTransform: error in parameters");
		return JS_FALSE;
	}

	/* verify that we have the correct objects here */
	if (transObj != NULL) 
		error = !JS_InstanceOf(cx, transObj, &SFVec3fClass, NULL);
	if (!error && (rotObj != NULL)) 
		error = !JS_InstanceOf(cx, rotObj, &SFRotationClass, NULL);
	if (!error && (scaleObj != NULL)) 
		error = !JS_InstanceOf(cx, scaleObj, &SFVec3fClass, NULL);
	if (!error && (scaleOObj != NULL)) 
		error = !JS_InstanceOf(cx, scaleOObj, &SFRotationClass, NULL);
	if (!error && centerObj != NULL) 
		error = !JS_InstanceOf(cx, centerObj, &SFVec3fClass, NULL);

	if (error) {
		ConsoleMessage ("setTransform: at least one parameter incorrect type");
		return JS_FALSE;
	}

	/* apply Transform, if requested */
	if (transObj) {
		SFVec3fNative * Vptr;
		Vptr = (SFVec3fNative *)JS_GetPrivateFw(cx, transObj);
		error = (Vptr == NULL);
	
		if (!error) {
                	matrix[12]=Vptr->v.c[0];
                	matrix[13]=Vptr->v.c[1];
                	matrix[14]=Vptr->v.c[2];
		}
	}

	if (!error && (rotObj != NULL)) {
		SFRotationNative * Rptr;
                Rptr = (SFRotationNative *)JS_GetPrivateFw(cx, rotObj);
		error = (Rptr == NULL);
	
		if (!error) {
			Quaternion quat;
			vrmlrot_to_quaternion(&quat, Rptr->v.c[0], Rptr->v.c[1], Rptr->v.c[2], Rptr->v.c[3]);
			/* printf ("from rotation %f %f %f %f\n",Rptr->v.c[0], Rptr->v.c[1], Rptr->v.c[2], Rptr->v.c[3]);
			printf ("quaternion is %f %f %f %f\n",quat.x,quat.y,quat.x, quat.w); */
			quaternion_to_matrix (matrix, &quat);
		}
	}

	if (!error && (scaleObj != NULL)) {
		SFVec3fNative * Vptr;
                Vptr = (SFVec3fNative *)JS_GetPrivateFw(cx, scaleObj);
		error = (Vptr == NULL);

		if (!error) {
			struct point_XYZ myScale;

			COPY_SFVEC3F_TO_POINT_XYZ (myScale,Vptr->v.c);
			scale_to_matrix(matrix, &myScale);
		}

	}

	/* place the new values into the vrmlMatrix array */
	_setmatrix (cx, obj, matrix);

#ifdef TESTING
       printf ("calculated Matrix: \n\t%5.2f %5.2f %5.2f %5.2f\n\t%5.2f %5.2f %5.2f %5.2f\n\t%5.2f %5.2f %5.2f %5.2f\n\t%5.2f %5.2f %5.2f %5.2f\n",
                matrix[0],  matrix[4],  matrix[ 8],  matrix[12],
                matrix[1],  matrix[5],  matrix[ 9],  matrix[13],
                matrix[2],  matrix[6],  matrix[10],  matrix[14],
                matrix[3],  matrix[7],  matrix[11],  matrix[15]);
	glGetDoublev(GL_MODELVIEW,xxmat);
       printf ("modelview Matrix: \n\t%5.2f %5.2f %5.2f %5.2f\n\t%5.2f %5.2f %5.2f %5.2f\n\t%5.2f %5.2f %5.2f %5.2f\n\t%5.2f %5.2f %5.2f %5.2f\n",
                xxmat[0],  xxmat[4],  xxmat[ 8],  xxmat[12],
                xxmat[1],  xxmat[5],  xxmat[ 9],  xxmat[13],
                xxmat[2],  xxmat[6],  xxmat[10],  xxmat[14],
                xxmat[3],  xxmat[7],  xxmat[11],  xxmat[15]);
	FW_GL_POP_MATRIX();
#endif

/* JS 185+ -requires- rval to be set on true return; assume we will return the 'this' object */
	JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(NULL)); //JSVAL_VOID);

	return JS_TRUE;
}


JSBool
VrmlMatrixinverse(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
	double src[16];
	double dest[16];
	JSObject *retObj;
	UNUSED (argv);

	if (argc != 0) {
		printf ("VrmlMatrix, expect 0 parameters\n");
		return JS_FALSE;
	}
	_getmatrix (cx, obj,src);
	matinverseFULL (dest,src);

        retObj = JS_ConstructObjectFw(cx,&VrmlMatrixClass,NULL, NULL);

        _setmatrix(cx,retObj,dest);
	JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(retObj));
	return JS_TRUE;
}


JSBool
VrmlMatrixtranspose(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
	double src[16];
	double dest[16];
	JSObject *retObj;
	UNUSED (argv);

	if (argc != 0) {
		printf ("VrmlMatrix, expect 0 parameters\n");
		return JS_FALSE;
	}
	_getmatrix (cx, obj,src);
	mattranspose (dest,src);

        retObj = JS_ConstructObjectFw(cx,&VrmlMatrixClass,NULL, NULL);

        _setmatrix(cx,retObj,dest);
	JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(retObj));
	return JS_TRUE;
}



JSBool
VrmlMatrixmultLeft(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);

        JSObject *transObj = NULL;
	JSObject *retObj = NULL;

        double matrix1[16];
        double matrix2[16];
        int error = FALSE;

	if (argc == 1) {
		error = !JS_ConvertArguments(cx, argc, argv, "o", &transObj);
	} else error = TRUE;

	if (!error) if (!JS_InstanceOf(cx, transObj, &VrmlMatrixClass, NULL)) { error = TRUE;}	

	if (error) {
		ConsoleMessage ("VrmlMatrixMultLeft, error in params");
		return JS_FALSE;
	}

	/* fill in the 2 matricies, multiply them, then return it */
	_getmatrix(cx,obj,matrix1);
	_getmatrix(cx,transObj,matrix2);
	matmultiplyFULL(matrix1,matrix1,matrix2);

	retObj = JS_ConstructObjectFw(cx,&VrmlMatrixClass,NULL, NULL);

	/*
       printf ("multLeft calculated Matrix: \n\t%5.2f %5.2f %5.2f %5.2f\n\t%5.2f %5.2f %5.2f %5.2f\n\t%5.2f %5.2f %5.2f %5.2f\n\t%5.2f %5.2f %5.2f %5.2f\n",
                matrix1[0],  matrix1[4],  matrix1[ 8],  matrix1[12],
                matrix1[1],  matrix1[5],  matrix1[ 9],  matrix1[13],
                matrix1[2],  matrix1[6],  matrix1[10],  matrix1[14],
                matrix1[3],  matrix1[7],  matrix1[11],  matrix1[15]);
	*/
	_setmatrix(cx,retObj,matrix1);
	JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(retObj));

	return JS_TRUE;
}

JSBool
VrmlMatrixmultRight(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        JSObject *transObj = NULL;
	JSObject *retObj = NULL;

        double matrix1[16];
        double matrix2[16];
        int error = FALSE;

	if (argc == 1) {
		error = !JS_ConvertArguments(cx, argc, argv, "o", &transObj);
	} else error = TRUE;

	if (!error) if (!JS_InstanceOf(cx, transObj, &VrmlMatrixClass, NULL)) { error = TRUE;}	

	if (error) {
		ConsoleMessage ("VrmlMatrixMultRight, error in params");
		return JS_FALSE;
	}

	/* fill in the 2 matricies, multiply them, then return it */
	_getmatrix(cx,obj,matrix1);
	_getmatrix(cx,transObj,matrix2);
	matmultiplyFULL(matrix1,matrix2,matrix1);

	retObj = JS_ConstructObjectFw(cx,&VrmlMatrixClass,NULL, NULL);

	/*
       printf ("multRight calculated Matrix: \n\t%5.2f %5.2f %5.2f %5.2f\n\t%5.2f %5.2f %5.2f %5.2f\n\t%5.2f %5.2f %5.2f %5.2f\n\t%5.2f %5.2f %5.2f %5.2f\n",
                matrix1[0],  matrix1[4],  matrix1[ 8],  matrix1[12],
                matrix1[1],  matrix1[5],  matrix1[ 9],  matrix1[13],
                matrix1[2],  matrix1[6],  matrix1[10],  matrix1[14],
                matrix1[3],  matrix1[7],  matrix1[11],  matrix1[15]);
	*/
	_setmatrix(cx,retObj,matrix1);
	JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(retObj));

	return JS_TRUE;
}


JSBool
VrmlMatrixmultVecMatrix(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        JSObject *transObj = NULL;
	JSObject *retObj = NULL;
	SFVec3fNative *Vptr;

        double matrix1[16];
        int error = FALSE;
	struct point_XYZ inp, outp;
	outp.x = outp.y = outp.z = 0.0;

	if (argc == 1) {
		error = !JS_ConvertArguments(cx, argc, argv, "o", &transObj);
	} else error = TRUE;

	if (!error) if (!JS_InstanceOf(cx, transObj, &SFVec3fClass, NULL)) { error = TRUE;}	

	if ((Vptr = (SFVec3fNative *)JS_GetPrivateFw(cx, transObj)) == NULL) {
		error = TRUE;
	}

	if (error) {
		ConsoleMessage ("VrmlMatrixMultVec, error in params");
		return JS_FALSE;
	}

	COPY_SFVEC3F_TO_POINT_XYZ(inp,Vptr->v.c);

	/* fill in the 2 matricies, multiply them, then return it */
	_getmatrix(cx,obj,matrix1);

	/* is this the one we have to transpose? */
	/* mattranspose (matrix1, matrix1); */
	
	matrotate2v(matrix1, inp, outp);

	retObj = JS_ConstructObjectFw(cx,&SFVec3fClass,NULL, NULL);
	if ((Vptr = (SFVec3fNative *)JS_GetPrivateFw(cx, retObj)) == NULL) {
		printf ("error in new VrmlMatrix\n");
		return JS_FALSE;
	}

	COPY_POINT_XYZ_TO_SFVEC3F(Vptr->v.c,outp);
	JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(retObj));

	return JS_TRUE;
}


JSBool
VrmlMatrixmultMatrixVec(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);

        JSObject *transObj = NULL;
	JSObject *retObj = NULL;
	SFVec3fNative *Vptr;

        double matrix1[16];
        int error = FALSE;
	struct point_XYZ inp, outp;
	outp.x = outp.y = outp.z = 0.0;

	if (argc == 1) {
		error = !JS_ConvertArguments(cx, argc, argv, "o", &transObj);
	} else error = TRUE;

	if (!error) if (!JS_InstanceOf(cx, transObj, &SFVec3fClass, NULL)) { error = TRUE;}	

	if ((Vptr = (SFVec3fNative *)JS_GetPrivateFw(cx, transObj)) == NULL) {
		error = TRUE;
	}

	if (error) {
		ConsoleMessage ("VrmlMatrixMultVec, error in params");
		return JS_FALSE;
	}

	COPY_SFVEC3F_TO_POINT_XYZ(inp,Vptr->v.c);

	/* fill in the 2 matricies, multiply them, then return it */
	_getmatrix(cx,obj,matrix1);

	/* is this the one we have to transpose? */
	mattranspose (matrix1, matrix1);
	
	matrotate2v(matrix1, inp, outp);

	retObj = JS_ConstructObjectFw(cx,&SFVec3fClass,NULL, NULL);
	if ((Vptr = (SFVec3fNative *)JS_GetPrivateFw(cx, retObj)) == NULL) {
		printf ("error in new VrmlMatrix\n");
		return JS_FALSE;
	}

	COPY_POINT_XYZ_TO_SFVEC3F(Vptr->v.c,outp);
	JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(retObj));

	return JS_TRUE;
}


JSBool
VrmlMatrixAssign(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;
        if (!_standardMFAssign (cx, obj, argc, argv, &rval, &VrmlMatrixClass,FIELDTYPE_FreeWRLPTR/*does not matter*/)) { return JS_FALSE; }
        JS_SET_RVAL(cx,vp,rval);
        return JS_TRUE;
}

JSBool
VrmlMatrixConstr(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_NewObject(cx,&VrmlMatrixClass,NULL,NULL);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval = OBJECT_TO_JSVAL(obj);
        if (!VrmlMatrixConstrInternals(cx,obj,argc,argv,&rval)) { return JS_FALSE; }
        JS_SET_RVAL(cx,vp,rval);
        return JS_TRUE;
}
JSBool VrmlMatrixConstrInternals(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	JSObject *_arrayObj;
	int isArray;
	jsdouble _d;
	unsigned int i;

	ADD_ROOT(cx,obj)

	isArray = FALSE;
	if(argc == 1 && argv){
		//could it be new MFxxx( [A,B] ) javscript array, as used by Carlson aka Carlson Array
		// tests/JohnCarlson/Arc1A.x3d
		if (!JS_ValueToObject(cx, argv[0], &_arrayObj)) {
			printf( "JS_ValueToObject failed in VrmlMatrixConstr.\n");
			return JS_FALSE;
		}

		if(JS_IsArrayObject(cx, _arrayObj)){
			jsuint lengthp;
			jsval vp;
			//printf("its an array\n");
			isArray = TRUE;
			JS_GetArrayLength(cx,_arrayObj, &lengthp);
			argc = lengthp;
		}
	}

	if ((argc != 16) && (argc != 0)) {
		printf ("VrmlMatrixConstr - require either 16 or no values\n");
		return JS_FALSE;
	}

	DEFINE_LENGTH(cx,obj,16)

	if (argc == 16) {
		for (i = 0; i < 16; i++) {
			jsval vp;
			if(isArray){
				JS_GetElement(cx, _arrayObj, i, &vp);

			}else{
				vp = argv[i];
			}
			if (!JS_ValueToNumber(cx, vp, &_d)) {
				printf(
					"JS_ValueToNumber failed in VrmlMatrixConstr.\n");
				return JS_FALSE;
			}

			if (!JS_DefineElement(cx, obj, (jsint) i, vp, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_CHECK, JSPROP_ENUMERATE)) {
				printf( "JS_DefineElement failed for arg %u in VrmlMatrixConstr.\n", i);
				return JS_FALSE;
			}
		}
	} else {
		/* make the identity matrix */
		double matrix[16];
		loadIdentityMatrix(matrix);
		_setmatrix (cx, obj, matrix);
	}
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}

JSBool
VrmlMatrixAddProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid,  JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid id = *hiid.address();
	jsval *vp = hvp.address();
	return doMFAddProperty(cx, obj, id, vp,"VrmlMatrixAddProperty");
}

JSBool
VrmlMatrixGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid,  JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();

	int32 _length, _index;
	jsval _length_val;


	jsval id;
	if (!JS_IdToValue(cx,iid,&id)) {
		printf("JS_IdToValue failed in VrmlMatrixGetproperty.\n");
		return JS_FALSE;
	}


    if (!JS_GetProperty(cx, obj,  MF_LENGTH_FIELD, &_length_val)) {
		printf( "JS_GetProperty failed for \"%s\" in VrmlMatrixGetProperty.\n", MF_LENGTH_FIELD);
        return JS_FALSE;
	}
	_length = JSVAL_TO_INT(_length_val);

/* -- note, code in here is not compliant to xulrunner-2
                if (JSVAL_IS_STRING(id)==TRUE) {
                printf("        is a common string :%s:\n",
                        JS_GetStringBytes(JS_ValueToString(cx, id)));
                }
                if (JSVAL_IS_OBJECT(id)==TRUE) {
                        printf ("       parameter is an object\n");
                }
                if (JSVAL_IS_PRIMITIVE(id)==TRUE) {
                        printf ("       parameter is a primitive\n");
                }
                if (JSVAL_IS_NULL(id)) { printf ("      - its a NULL\n");}
                if (JSVAL_IS_INT(id)) { printf ("       - its a INT %d\n",JSVAL_TO_INT(id));}
*/




	if (JSVAL_IS_INT(id)) {
		_index = JSVAL_TO_INT(id);

		if (_index >= _length) {
			JS_NewNumberValue(cx,0.0,vp);
			if (!JS_DefineElement(cx, obj, (jsint) _index, *vp, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_CHECK, JSPROP_ENUMERATE)) {
				printf( "JS_DefineElement failed in VrmlMatrixGetProperty.\n");
				return JS_FALSE;
			}
		} else {
			if (!JS_LookupElement(cx, obj, _index, vp)) {
				printf(
						"JS_LookupElement failed in VrmlMatrixGetProperty.\n");
				return JS_FALSE;
			}
			if (JSVAL_IS_NULL(*vp)) {
				printf( "VrmlMatrixGetProperty: obj = %p, jsval = %d does not exist!\n",
					   obj, (int) _index);
				return JS_FALSE;
			}
		}
	} else if (id.isObject()) {
	}

	return JS_TRUE;
}

JSBool
VrmlMatrixSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid id = *hiid.address();
	jsval *vp = hvp.address();

	return doMFSetProperty(cx, obj, id, vp,1000); /* do not have a FIELDTYPE for this */
}

// MFRotation
JSBool
MFRotationAddProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid,  JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid id = *hiid.address();
	jsval *vp = hvp.address();

	return doMFAddProperty(cx, obj, id, vp,"MFRotationAddProperty");
}

JSBool
MFRotationGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid,  JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid id = *hiid.address();
	jsval *vp = hvp.address();

	return _standardMFGetProperty(cx, obj, id, vp,
			 "_FreeWRL_Internal = new SFRotation()",FIELDTYPE_MFRotation);
}

JSBool
MFRotationSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid id = *hiid.address();
	jsval *vp = hvp.address();

	return doMFSetProperty(cx, obj, id, vp,FIELDTYPE_MFRotation);
}

JSBool
MFRotationToString(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;

	UNUSED(argc);
	UNUSED(argv);
	if (!doMFToString(cx, obj, "MFRotation", &rval)) { return JS_FALSE; }
	JS_SET_RVAL(cx,vp,rval);
	return JS_TRUE;

}

JSBool
MFRotationConstr(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_NewObject(cx,&MFRotationClass,NULL,NULL);
        jsval *argv = JS_ARGV(cx,vp);
	jsval rval = OBJECT_TO_JSVAL(obj);
	if (!MFRotationConstrInternals(cx,obj,argc,argv,&rval)) { return JS_FALSE; }
	JS_SET_RVAL(cx,vp,rval);
	return JS_TRUE;
}
JSBool MFRotationConstrInternals(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	JSObject *_arrayObj;
	int isArray;
	JSObject *_obj;
	unsigned int i;
	union anyVrml *anyv;

	ADD_ROOT(cx,obj)

	isArray = FALSE;
	if(argc == 1 && argv){
		//could it be new MFxxx( [A,B] ) javscript array, as used by Carlson aka Carlson Array
		// tests/JohnCarlson/Arc1A.x3d
		if (!JS_ValueToObject(cx, argv[0], &_arrayObj)) {
			printf( "JS_ValueToObject failed in MFVec3fConstr.\n");
			return JS_FALSE;
		}

		if(JS_IsArrayObject(cx, _arrayObj)){
			jsuint lengthp;
			jsval vp;
			//printf("its an array\n");
			isArray = TRUE;
			JS_GetArrayLength(cx,_arrayObj, &lengthp);
			argc = lengthp;
		}
	}

	if(SM_method() == 2){
		AnyNative *any;
		int newsize;
		if((any = (AnyNative*)AnyNativeNew(FIELDTYPE_MFRotation,NULL,NULL)) == NULL){
			printf( "AnyfNativeNew failed in MFRotationConstr.\n");
			return JS_FALSE;
		}
		if (!JS_SetPrivateFw(cx, obj, any)) {
			printf( "JS_SetPrivate failed in MFRotationConstr.\n");
			return JS_FALSE;
		}
		anyv = any->v;
		newsize = sizeof(struct SFRotation)*upper_power_of_two(argc);
		if(argc > 0){
			anyv->mfrotation.p = MALLOC(struct SFRotation*,newsize);
			memset(anyv->mfrotation.p,0,newsize);
		}

	}else{
		DEFINE_LENGTH(cx,obj,argc)
	}

	if (!argv) {
		return JS_TRUE;
	}

	#ifdef JSVRMLCLASSESVERBOSE
		printf("MFRotationConstr: obj = %p, %u args\n", obj, argc);
	#endif
	for (i = 0; i < argc; i++) {
		jsval vp;
		if(isArray){
			JS_GetElement(cx, _arrayObj, i, &vp);

		}else{
			vp = argv[i];
		}
		if (!JS_ValueToObject(cx, vp, &_obj)) {
			printf(
					"JS_ValueToObject failed in MFRotationConstr.\n");
			return JS_FALSE;
		}

		CHECK_CLASS(cx,_obj,NULL,__FUNCTION__,SFRotationClass)
		if(SM_method()==2){
			AnyNative *any2;
			if((any2 = (AnyNative *)JS_GetPrivateFw(cx,_obj)) != NULL){
				if(any2->type == FIELDTYPE_SFRotation ){
					shallow_copy_field(FIELDTYPE_SFRotation,any2->v,(union anyVrml*)&anyv->mfrotation.p[i]);
					anyv->mfrotation.n = i+1;
				}
			}
			// else for now we'll leave zeros
		}else{
			if (!JS_DefineElement(cx, obj, (jsint) i, vp, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_CHECK, JSPROP_ENUMERATE)) {
				printf( "JS_DefineElement failed for arg %d in MFRotationConstr.\n", i);
				return JS_FALSE;
			}
		}
	}
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}

JSBool
MFRotationAssign(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;

	if (!_standardMFAssign (cx, obj, argc, argv, &rval, &MFRotationClass,FIELDTYPE_SFRotation)) { return JS_FALSE; }
	JS_SET_RVAL(cx,vp,rval);
	return JS_TRUE;

}

// MFStrings 
JSBool
MFStringAddProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid,  JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();
	jsval id;
	if (!JS_IdToValue(cx,iid,&id)) {
		printf("JS_IdToValue failed in MFStringAddProperty\n");
		return JS_FALSE;
	}


	#ifdef JSVRMLCLASSESVERBOSE
	printf("MFStringAddProperty: vp = %p\n", obj);
                if (JSVAL_IS_STRING(*vp)==TRUE) {
		printf("	is a common string :%s:\n",
#if JS_VERSION < 185
                        JS_GetStringBytes(JS_ValueToString(cx, *vp)));
#else
                        JS_EncodeString(cx,JS_ValueToString(cx, *vp)));
#endif
                }
                if (JSVAL_IS_OBJECT(*vp)==TRUE) {
                        printf ("       parameter is an object\n");
                }
                if (JSVAL_IS_PRIMITIVE(*vp)==TRUE) {
                        printf ("       parameter is a primitive\n");
                }
		if (JSVAL_IS_NULL(*vp)) { printf ("	- its a NULL\n");}
		if (JSVAL_IS_INT(*vp)) { printf ("	- its a INT %d\n",JSVAL_TO_INT(*vp));}

		printf("MFStringAddProperty: id = %p\n", obj);
                if (JSVAL_IS_STRING(id)==TRUE) {
		printf("	is a common string :%s:\n",
#if JS_VERSION < 185
                        JS_GetStringBytes(JS_ValueToString(cx, id)));
#else
                        JS_EncodeString(cx,JS_ValueToString(cx, id)));
#endif
                }
                if (JSVAL_IS_OBJECT(id)==TRUE) {
                        printf ("       parameter is an object\n");
                }
                if (JSVAL_IS_PRIMITIVE(id)==TRUE) {
                        printf ("       parameter is a primitive\n");
                }
		if (JSVAL_IS_NULL(id)) { printf ("	- its a NULL\n");}
		if (JSVAL_IS_INT(id)) { printf ("	- its a INT %d\n",JSVAL_TO_INT(id));}

	#endif


	/* unquote parts of vp string if necessary */
	if (JSVAL_IS_STRING(*vp)) {
		if (!doMFStringUnquote(cx, vp)) {
			printf(
				"doMFStringUnquote failed in MFStringAddProperty.\n");
			return JS_FALSE;
		}
	}
	return doMFAddProperty(cx, obj, iid, vp,"MFStringAddProperty");

}


JSBool
MFStringGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid,  JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();


	JSString *_str;
	int32 _length, _index;
    jsval _length_val;

	jsval id;
	if (!JS_IdToValue(cx,iid,&id)) {
		printf("JS_IdToValue failed in MFStringGetProperty\n");
		return JS_FALSE;
	}

	if(SM_method()==2){
		return _standardMFGetProperty(cx, obj, iid, vp,
			 "_FreeWRL_Internal = new SFString()",FIELDTYPE_MFString);

	}

	#ifdef JSVRMLCLASSESVERBOSE
	printf("MFStringGetProperty: obj = %p\n", obj);
	#endif

    if (!JS_GetProperty(cx, obj,  MF_LENGTH_FIELD, &_length_val)) {
		printf( "JS_GetProperty failed for \"%s\" in MFStringGetProperty.\n", MF_LENGTH_FIELD);
        return JS_FALSE;
	}
	_length = JSVAL_TO_INT(_length_val);

	if (JSVAL_IS_INT(id)) {
		_index = JSVAL_TO_INT(id);

		if (_index >= _length) {
			_str = JS_NewStringCopyZ(cx, "");
			*vp = STRING_TO_JSVAL(_str);
			if (!JS_DefineElement(cx, obj, (jsint) _index, *vp, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_CHECK, JSPROP_ENUMERATE)) {
				printf( "JS_DefineElement failed in MFStringGetProperty.\n");
				return JS_FALSE;
			}
		} else {
			if (!JS_LookupElement(cx, obj, _index, vp)) {
				printf( "JS_LookupElement failed in MFStringGetProperty.\n");
				return JS_FALSE;
			}
			if (JSVAL_IS_NULL(*vp)) {
				/* jut make up new strings, as above */
				/* printf ("MFStringGetProperty, element %d is JSVAL_VOID, making up string for it\n",_index); */
				_str = JS_NewStringCopyZ(cx, "NULL");
				*vp = STRING_TO_JSVAL(_str);
				if (!JS_DefineElement(cx, obj, (jsint) _index, *vp, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_CHECK, JSPROP_ENUMERATE)) {
					printf( "JS_DefineElement failed in MFStringGetProperty.\n");
					return JS_FALSE;
				}
			}
		}
	}

	return JS_TRUE;
}

JSBool
MFStringSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid id = *hiid.address();
	jsval *vp = hvp.address();

	JSBool rv;

	#ifdef JSVRMLCLASSESVERBOSE
	printf("MFStringSetProperty: obj = %p id %d jsval %u\n", obj, id, (unsigned int)*vp);

printf ("MFStringSetProperty, setting vp of type...\n");
		if (JSVAL_IS_OBJECT(*vp)) { printf ("	- MFStringSetProperty, vp is a OBJECT\n");}
		if (JSVAL_IS_PRIMITIVE(*vp)) { printf ("	- MFStringSetProperty, vp is a PRIMITIVE\n");}
		if (JSVAL_IS_NULL(*vp)) { printf ("	- MFStringSetProperty, vp is a NULL\n");}
		if (JSVAL_IS_STRING(*vp)) { printf ("	- MFStringSetProperty, vp is a STRING\n");}
		if (JSVAL_IS_INT(*vp)) { printf ("	- MFStringSetProperty, vp is a INT %d\n",JSVAL_TO_INT(*vp));}

	#endif


	/* unquote parts of vp string if necessary */
	if (JSVAL_IS_STRING(*vp)) {
		if (!doMFStringUnquote(cx, vp)) {
			printf(
				"doMFStringUnquote failed in MFStringSetProperty.\n");
			return JS_FALSE;
		}
	}
	rv = doMFSetProperty(cx, obj, id, vp,FIELDTYPE_MFString);
	#ifdef JSVRMLCLASSESVERBOSE
	printf ("returning from MFStringSetProperty\n");
	#endif

	return rv;

}

JSBool
MFStringToString(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;

	UNUSED(argc);
	UNUSED(argv);
	#ifdef JSVRMLCLASSESVERBOSE
	printf("MFStringToString: obj = %p, %u args\n", obj, argc);
	#endif


	if (!doMFToString(cx, obj, "MFString", &rval)) { return JS_FALSE; }
	JS_SET_RVAL(cx,vp,rval);
	return JS_TRUE;

}


JSBool
MFStringConstr(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_NewObject(cx,&MFStringClass,NULL,NULL);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval = OBJECT_TO_JSVAL(obj);
        if (!MFStringConstrInternals(cx,obj,argc,argv,&rval)) { return JS_FALSE; }
        JS_SET_RVAL(cx,vp,rval);
        return JS_TRUE;
}
JSBool MFStringConstrInternals(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	JSObject *_arrayObj;
	int isArray;

	unsigned int i;
	union anyVrml *anyv;

	#ifdef JSVRMLCLASSESVERBOSE
	JSString *_str;
	printf("MFStringConstr: cx %p, obj %p args %d rval %p parent %p... ", cx, obj, argc, rval, JS_GetParent(cx, obj));
	#endif

	ADD_ROOT(cx,obj)

	isArray = FALSE;
	if(argc == 1 && argv){
		//could it be new MFxxx( [A,B] ) javscript array, as used by Carlson aka Carlson Array
		// tests/JohnCarlson/Arc1A.x3d
		if (!JS_ValueToObject(cx, argv[0], &_arrayObj)) {
			printf( "JS_ValueToObject failed in MFVec3fConstr.\n");
			return JS_FALSE;
		}

		if(JS_IsArrayObject(cx, _arrayObj)){
			jsuint lengthp;
			jsval vp;
			//printf("its an array\n");
			isArray = TRUE;
			JS_GetArrayLength(cx,_arrayObj, &lengthp);
			argc = lengthp;
		}
	}

	if(SM_method() == 2){
		AnyNative *any;
		int newsize;
		if((any = (AnyNative*)AnyNativeNew(FIELDTYPE_MFString,NULL,NULL)) == NULL){
			printf( "AnyfNativeNew failed in MFStringConstr.\n");
			return JS_FALSE;
		}
		if (!JS_SetPrivateFw(cx, obj, any)) {
			printf( "JS_SetPrivate failed in MFStringConstr.\n");
			return JS_FALSE;
		}
		anyv = any->v;
		newsize = sizeof(struct Uni_String*)*upper_power_of_two(argc);
		if(argc > 0){
			anyv->mfstring.p = MALLOC(struct Uni_String**,newsize);
			memset(anyv->mfstring.p,0,newsize);
		}

	}else{
		DEFINE_LENGTH(cx,obj,argc)
		DEFINE_MF_ECMA_HAS_CHANGED
	}

	if (!argv) {
		return JS_TRUE;
	}

	for (i = 0; i < argc; i++) {
		jsval vp;
		if(isArray){
			JS_GetElement(cx, _arrayObj, i, &vp);

		}else{
			vp = argv[i];
		}
		#ifdef JSVRMLCLASSESVERBOSE
	  	printf ("argv %d is a ...",i);

		if (JSVAL_IS_STRING(argv[i])==TRUE) {
        	        printf (" Common String, is");
			_str = JS_ValueToString(cx, argv[i]);
#if JS_VERSION < 185
			printf (" %s",JS_GetStringBytes(_str));
#else
			printf (" %s",JS_EncodeString(cx,_str));
#endif
			printf ("..");
		
	        }                                          
		if (JSVAL_IS_OBJECT(argv[i])==TRUE) {   
	                printf (" is an object");
	        }                       
		if (JSVAL_IS_PRIMITIVE(argv[i])==TRUE) {
        	        printf (" is a primitive");
        	}

		if ((_str = JS_ValueToString(cx, argv[i])) == NULL) {
			printf( "JS_ValueToString failed in MFStringConstr.");
			return JS_FALSE;
		}
		printf ("\n");
		#endif

		if(SM_method()==2){
			char *cstring = NULL;
			if (JSVAL_IS_STRING(vp)==TRUE) {
				// https://developer.mozilla.org/en-US/docs/Mozilla/Projects/SpiderMonkey/JSAPI_reference/JS_EncodeString
				// cstring: we own it
				JSString *_str;
				_str = JS_ValueToString(cx, vp);
				cstring = JS_EncodeString(cx,_str); //if utf16: lossy - will drop first byte, garbage
				//JS_free(cx,_str); bombs if I do this

			}else{
				//could try and convert object or ecma primitive to string via toString()
			}
			if(cstring){
				//newASCIIString does an extra malloc
				struct Uni_String *us;
				us = MALLOC(struct Uni_String*,sizeof(struct Uni_String));
				us->strptr = cstring;
				us->len = strlen(cstring);
				us->touched = 0;
				anyv->mfstring.p[i] = us;
				anyv->mfstring.n = i+1;
			}
			// else for now we'll leave zeros
		}else{
			if (!JS_DefineElement(cx, obj, (jsint) i, vp, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_CHECK, JSPROP_ENUMERATE)) {
				printf( "JS_DefineElement failed for arg %d in MFStringConstr.\n", i);
				return JS_FALSE;
			}
		}
	}
	*rval = OBJECT_TO_JSVAL(obj);

	#ifdef JSVRMLCLASSESVERBOSE
	printf ("finished MFStringConstr\n");
	#endif

	return JS_TRUE;
}

JSBool
MFStringAssign(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;


	#ifdef JSVRMLCLASSESVERBOSE
	printf("MFStringAssign: obj = %p args %d... ", obj, argc);
	#endif
	if(SM_method() != 2){
		SET_MF_ECMA_HAS_CHANGED
	}

	if (!_standardMFAssign (cx, obj, argc, argv, &rval, &MFStringClass,FIELDTYPE_SFString)) { return JS_FALSE; }
	JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(obj));
	return JS_TRUE;

}

/* testing.. */
JSBool MFStringDeleteProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid,  JSBool *succeeded){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();

	#ifdef JSVRMLCLASSESVERBOSE
	printf ("MFStringDeleteProperty\n"); 
	#endif
	return JS_TRUE;
}
JSBool
MFStringEnumerateProperty(JSContext *cx, JS::Handle<JSObject*> hobj) {
	JSObject *obj = *hobj.address();

	#ifdef JSVRMLCLASSESVERBOSE
	printf ("MFStringEnumerateProperty\n"); 
	#endif
	return JS_TRUE;
}

JSBool MFStringResolveProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid){
	JSObject *obj = *hobj.address();
	jsid id = *hiid.address();

	#ifdef JSVRMLCLASSESVERBOSE
	printf ("MFStringResolveProperty\n"); 
	#endif
	return JS_TRUE;
}
JSBool MFStringConvertProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JSType type, JS::MutableHandle<JS::Value> hvp) {
	JSObject *obj = *hobj.address();
	jsval *vp = hvp.address();


	#ifdef JSVRMLCLASSESVERBOSE
	printf ("MFStringConvertProperty\n"); 
	#endif
	return JS_TRUE;
}

#endif //defined(JS_SMCPP)

#endif //JAVASCRIPT_SM

