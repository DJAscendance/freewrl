/*

  A substantial amount of code has been adapted from js/src/js.c,
  which is the sample application included with the javascript engine.

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
#define JS_VERSION 187
//#define JS_THREADSAFE 1 //by default in 186+
int JS_SetPrivateFw(JSContext *cx, JSObject* obj, void *data);
JSObject* JS_NewGlobalObjectFw(JSContext *cx, JSClass *clasp); //, JSPrincipals *princ);
void * JS_GetPrivateFw(JSContext *cx,JSObject*_obj);
JSObject* JS_GetParentFw(JSContext *cx, JSObject *me);
JSObject * JS_ConstructObjectWithArgumentsFw(JSContext *cx, JSClass *clasp, JSObject *parent, unsigned argc, jsval *argv); 
JSObject * JS_ConstructObjectFw(JSContext *cx, JSClass *clasp, void *whatever, JSObject *parent);
JSObject * JS_GetPrototypeFw(JSContext *cx, JSObject * obj);
JSClass * JS_GetClassFw(JSContext *cx, JSObject * obj);
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

#ifndef IBOOL
typedef int IBOOL;
#endif
typedef IBOOL _Bool;
//typedef _Bool bool;



extern "C" {
#include <system.h>
//#include <system_threads.h>
//#include <display.h>
#include <internal.h>

//#include <libFreeWRL.h>
//#include <list.h>
#include "scenegraph/LinearAlgebra.h"
#include "scenegraph/quaternion.h"
//#include <io_files.h>

//#include "../vrml_parser/Structs.h"
//#include "../main/headers.h"
#include "../main/ProdCon.h"
//#include "../vrml_parser/CParseGeneral.h"
//#include "../main/Snapshot.h"
//#include "../scenegraph/LinearAlgebra.h"
//#include "../scenegraph/Collision.h"
//#include "../scenegraph/quaternion.h"
//#include "../scenegraph/Viewer.h"
//#include "../input/SensInterps.h"
//#include "../x3d_parser/Bindable.h"
//#include "../input/InputFunctions.h"

//#include <system_js.h>
#include "JScript.h"
#include "CScripts.h"
#include "jsNative.h"

void Parser_scanStringValueToMem_B(union anyVrml* any, indexT ctype, const char *value, int isXML);
} //extern "C"

#include "jsUtils_sm.h"
#include "jsVRMLClasses_sm.h"

//#include "JScript.h"


/********************************************************/
/*							*/
/* Second part - SF classes				*/
/*							*/
/********************************************************/

/* from http://www.cs.rit.edu/~ncs/color/t_convert.html */
double dMIN3(double a, double b, double c) {
	double min;
	if((a<b)&&(a<c))min=a; else if((b<a)&&(b<c))min=b; else min=c; return min;
}

double dMAX3(double a, double b, double c) {
	double max;
	if((a>b)&&(a>c))max=a; else if((b>a)&&(b>c))max=b; else max=c; return max;
}

void convertRGBtoHSV(double r, double g, double b, double *h, double *s, double *v) {
	double my_min, my_max, delta;

	my_min = dMIN3( r, g, b );
	my_max = dMAX3( r, g, b );
	*v = my_max;				/* v */
	delta = my_max - my_min;
	if( my_max != 0 )
		*s = delta / my_max;		/* s */
	else {
		/* r = g = b = 0 */		/* s = 0, v is undefined */
		*s = 0;
		*h = -1;
		return;
	}
	if( r == my_max )
		*h = ( g - b ) / delta;		/* between yellow & magenta */
	else if( g == my_max )
		*h = 2 + ( b - r ) / delta;	/* between cyan & yellow */
	else
		*h = 4 + ( r - g ) / delta;	/* between magenta & cyan */
	*h *= 60;				/* degrees */
	if( *h < 0 )
		*h += 360;
}
void convertHSVtoRGB( double h, double s, double v ,double *r, double *g, double *b)
{
	int i;
	double f, p, q, t;
	if( s == 0 ) {
		/* achromatic (grey) */
		*r = *g = *b = v;
		return;
	}
	h /= 60;			/* sector 0 to 5 */
	i = (int) floor( h );
	f = h - i;			/* factorial part of h */
	p = v * ( 1 - s );
	q = v * ( 1 - s * f );
	t = v * ( 1 - s * ( 1 - f ) );
	switch( i ) {
		case 0: *r = v; *g = t; *b = p; break;
		case 1: *r = q; *g = v; *b = p; break;
		case 2: *r = p; *g = v; *b = t; break;
		case 3: *r = p; *g = q; *b = v; break;
		case 4: *r = t; *g = p; *b = v; break;
		default: *r = v; *g = p; *b = q; break;
	}
}

JSBool
#if JS_VERSION < 185
SFColorGetHSV(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
#else
SFColorGetHSV(JSContext *cx, uintN argc, jsval *vp) {
	JSObject *obj = JS_THIS_OBJECT(cx,vp);
	jsval *argv = JS_ARGV(cx,vp);
#endif
	JSObject *result;
	double xp[3];
	jsval _v;
	int i;
	float *cc;

	UNUSED(argv);
	if (argc != 0) {
		printf ("SFColorGetHSV; arguments found but not expected\n");
		return JS_FALSE;
	}
	
	/* get the RGB values */
	if(SM_method()==2){
		AnyNative *ptr;
        if ((ptr = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
                printf( "JS_GetPrivate failed in SFColorGetHSV.\n");
                return JS_FALSE;
        }
		cc = ptr->v->sfcolor.c;

	}else{
		SFColorNative *ptr;
        if ((ptr = (SFColorNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
                printf( "JS_GetPrivate failed in SFColorGetHSV.\n");
                return JS_FALSE;
        }
		cc = ptr->v.c;
	}
	/* convert rgb to hsv */
	convertRGBtoHSV(cc[0], cc[1], cc[2],&xp[0],&xp[1],&xp[2]);

	#ifdef JSVRMLCLASSESVERBOSE
        printf("hsv code, orig rgb is %.9g %.9g %.9g\n", (ptr->v).c[0], (ptr->v).c[1], (ptr->v).c[2]);
	printf ("hsv conversion is %lf %lf %lf\n",xp[0],xp[1],xp[2]);
	#endif
	// http://www.web3d.org/documents/specifications/19777-1/V3.3/Part1/functions.html#SFColor
	// - specs want a numeric[3] return val
	result = JS_NewArrayObject(cx, 3, NULL); 
        ADD_ROOT(cx, result); 
        for(i=0; i<3; i++) { 
		if (JS_NewNumberValue(cx, xp[i],&_v) == JS_FALSE) {
			printf( "JS_NewDouble failed for %f in SFColorGetHSV.\n", xp[i]);
			return JS_FALSE;
		}
        	JS_SetElement(cx, result, (jsint)i, &_v); 
        } 

        /* JAS - should we remove this here, or on finalize? JS_RemoveRoot(cx, &result);  */
#if JS_VERSION < 185
        *rval = OBJECT_TO_JSVAL(result); 
#else
        JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(result));
#endif
	return JS_TRUE;
}

JSBool
#if JS_VERSION < 185
SFColorSetHSV(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
#else
SFColorSetHSV(JSContext *cx, uintN argc, jsval *vp) {
	JSObject *obj = JS_THIS_OBJECT(cx,vp);
	jsval *argv = JS_ARGV(cx,vp);
#endif
	double hue, saturation, value;
	double red,green,blue;
	float *cc;

	if(SM_method() == 2){
		AnyNative *ptr;
		if ((ptr = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SSFColorSetHSV.\n");
			return JS_FALSE;
		}
		if(ptr->valueChanged)
			(*ptr->valueChanged) ++;
		cc = ptr->v->sfcolor.c;
	}else{
		SFColorNative *ptr;
		if ((ptr = (SFColorNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFColorSetHSV.\n");
			return JS_FALSE;
		}
		ptr->valueChanged ++;
		cc = ptr->v.c;
	}

	if (!JS_ConvertArguments(cx, argc, argv, "d d d", &hue, &saturation, &value)) {
		printf( "JS_ConvertArguments failed in SFColorSetHSV.\n");
		return JS_FALSE;
	}

	/* do conversion here!!! */
	#ifdef JSCLASSESVERBOSE
        printf("hsv code, orig rgb is %.9g %.9g %.9g\n", (ptr->v).c[0], (ptr->v).c[1], (ptr->v).c[2]);
	printf ("SFColorSetHSV, have %lf %lf %lf\n",hue,saturation,value);
	#endif

	convertHSVtoRGB(hue,saturation,value, &red, &green, &blue);
	cc[0] = (float) red;
	cc[1] = (float) green;
	cc[2] = (float) blue;
	#ifdef JSCLASSESVERBOSE
        printf("hsv code, now rgb is %.9g %.9g %.9g\n", (ptr->v).c[0], (ptr->v).c[1], (ptr->v).c[2]);
	#endif

#if JS_VERSION < 185
	*rval = OBJECT_TO_JSVAL(obj);
#else
	JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(obj));
#endif

    return JS_TRUE;
}

JSBool
#if JS_VERSION < 185
SFColorToString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
#else
SFColorToString(JSContext *cx, uintN argc, jsval *vp) {
	JSObject *obj = JS_THIS_OBJECT(cx,vp);
	jsval *argv = JS_ARGV(cx,vp);
#endif
    JSString *_str;
	char _buff[STRING];
	float *cc;

	UNUSED(argc);
	UNUSED(argv);
	if(SM_method()==2){
	    AnyNative *ptr;
		if ((ptr = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFColorToString.\n");
			return JS_FALSE;
		}
		cc = ptr->v->sfcolor.c;
	}else{
	    SFColorNative *ptr;
		if ((ptr = (SFColorNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFColorToString.\n");
			return JS_FALSE;
		}
		cc = ptr->v.c;
	}
	memset(_buff, 0, STRING);
	sprintf(_buff, "%.9g %.9g %.9g",
			cc[0], cc[1], cc[2]);
	_str = JS_NewStringCopyZ(cx, _buff);
#if JS_VERSION < 185
    *rval = STRING_TO_JSVAL(_str);
#else
	JS_SET_RVAL(cx,vp,STRING_TO_JSVAL(_str));
#endif
    return JS_TRUE;
}

JSBool
#if JS_VERSION < 185
SFColorAssign(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
#else
SFColorAssign(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
	JSString *_id_jsstr;
#endif
    JSObject *_from_obj;
    char *_id_str;

	UNUSED(_id_str); // compiler warning mitigation

	if(SM_method() == 2){
		AnyNative *lhs, *rhs;
		if ((lhs = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed for obj in SFColorAssign.\n");
			return JS_FALSE;
		}
		//if (!JSVAL_IS_OBJECT(*vp))
		if (!(*vp).isObject())
			return JS_FALSE;
        if ((rhs = (AnyNative *)JS_GetPrivateFw(cx, JSVAL_TO_OBJECT(*vp))) == NULL) {
			printf("JS_ConvertArguments failed in SFColorAssign. \n");
			return JS_FALSE;
        }
		AnyNativeAssign(lhs,rhs);
	}else{
		SFColorNative *ptr, *fptr;
		if ((ptr = (SFColorNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed for obj in SFColorAssign.\n");
			return JS_FALSE;
		}


		CHECK_CLASS(cx,obj,argv,__FUNCTION__,SFColorClass) 

#if JS_VERSION < 185
		if (!JS_ConvertArguments(cx, argc, argv, "o s", &_from_obj, &_id_str)) {
#else
		if (JS_ConvertArguments(cx, argc, argv, "oS", &_from_obj, &_id_jsstr) == JS_TRUE) {
			_id_str = JS_EncodeString(cx,_id_jsstr);
		} else {
#endif
			printf( "JS_ConvertArguments failed in SFColorAssign.\n");
			return JS_FALSE;
		}

		CHECK_CLASS(cx,_from_obj,argv,__FUNCTION__,SFColorClass)

		if ((fptr = (SFColorNative *)JS_GetPrivateFw(cx, _from_obj)) == NULL) {
			printf( "JS_GetPrivate failed for _from_obj in SFColorAssign.\n");
			return JS_FALSE;
		}
		#ifdef JSVRMLCLASSESVERBOSE
			printf("SFColorAssign: obj = %p, id = \"%s\", from = %p\n", obj, _id_str, _from_obj);
		#endif

		SFColorNativeAssign(ptr, fptr);
	}
#if JS_VERSION < 185
    *rval = OBJECT_TO_JSVAL(obj);
#else
    JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(obj));
#endif

    return JS_TRUE;
}

JSBool
#if JS_VERSION < 185
SFColorConstr(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
#else
SFColorConstr(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_NewObject(cx,&SFColorClass,NULL,NULL);
        jsval *argv = JS_ARGV(cx,vp);
#endif
	jsdouble pars[3];
	float *cc;

	ADD_ROOT(cx,obj)
	if(SM_method() == 2){
		AnyNative *any;
		if((any = (AnyNative*)AnyNativeNew(FIELDTYPE_SFColor,NULL,NULL)) == NULL){
			printf( "AnyfNativeNew failed in SFColorConstr.\n");
			return JS_FALSE;
		}
		if (!JS_SetPrivateFw(cx, obj, any)) {
			printf( "JS_SetPrivate failed in SFColorConstr.\n");
			return JS_FALSE;
		}
		cc = any->v->sfvec3f.c;
	}else{
		SFColorNative *ptr;

		if ((ptr = (SFColorNative *) SFColorNativeNew()) == NULL) {
			printf( "SFColorNativeNew failed in SFColorConstr.\n");
			return JS_FALSE;
		}

		//if (!JS_DefineProperties(cx, obj, SFColorProperties)) {
		//	printf( "JS_DefineProperties failed in SFColorConstr.\n");
		//	return JS_FALSE;
		//}

		if (!JS_SetPrivateFw(cx, obj, ptr)) {
			printf( "JS_SetPrivate failed in SFColorConstr.\n");
			return JS_FALSE;
		}
		cc = ptr->v.c;
		ptr->valueChanged = 1;
	}
	if (argc == 0) {
		cc[0] = (float) 0.0;
		cc[1] = (float) 0.0;
		cc[2] = (float) 0.0;
	} else if (JS_ConvertArguments(cx, argc, argv, "d d d", &(pars[0]), &(pars[1]), &(pars[2]))) {
		cc[0] = (float) pars[0];
		cc[1] = (float) pars[1];
		cc[2] = (float) pars[2];
	} else {
		printf( "Invalid arguments for SFColorConstr.\n");
		return JS_FALSE;
	}
	#ifdef JSVRMLCLASSESVERBOSE
		printf("SFColorConstr: obj = %p args = %d, %f %f %f\n",
			   obj, argc,
			   cc[0], cc[1], cc[2]);
	#endif
	

#if JS_VERSION < 185
	*rval = OBJECT_TO_JSVAL(obj);
#else
	JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(obj));
#endif

	return JS_TRUE;
}


JSBool
#if JS_VERSION < 185
SFColorGetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp){
#elif JS_VERSION == 185
SFColorGetProperty(JSContext *cx, JSObject *obj, jsid iid, jsval *vp){
#else
SFColorGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();

#endif

	jsdouble d;
	float *cc;

#if JS_VERSION >= 185
	jsval id;
	if (!JS_IdToValue(cx,iid,&id)) {
		printf("JS_IdToValue failed in SFColorGetProperty.\n");
		return JS_FALSE;
	}
#endif
	if(SM_method()==2){
		AnyNative *any;
		if ((any = (AnyNative *)JS_GetPrivateFw(cx,obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFColorGetProperty.\n");
			return JS_FALSE;
		}
		cc = any->v->sfcolor.c;
	}else{
		SFColorNative *ptr;

		if ((ptr = (SFColorNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFColorGetProperty.\n");
			return JS_FALSE;
		}
		cc = ptr->v.c;
	}
	if (JSVAL_IS_INT(id)) {
		switch (JSVAL_TO_INT(id)) {
		case 0:
			d = cc[0];
			if (JS_NewNumberValue(cx, d, vp) == JS_FALSE) {
				printf(
						"JS_NewDouble failed for %f in SFColorGetProperty.\n",
						d);
				return JS_FALSE;
			}
			break;
		case 1:
			d = cc[1];
			if (JS_NewNumberValue(cx, d, vp) == JS_FALSE) {
				printf(
						"JS_NewDouble failed for %f in SFColorGetProperty.\n",
						d);
				return JS_FALSE;
			}
			break;
		case 2:
			d = cc[2];
			if (JS_NewNumberValue(cx, d, vp) == JS_FALSE) {
				printf(
						"JS_NewDouble failed for %f in SFColorGetProperty.\n",
						d);
				return JS_FALSE;
			}
			break;
		}
	}
	return JS_TRUE;
}

JSBool
#if JS_VERSION < 185
SFColorSetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp){
#elif JS_VERSION == 185
SFColorSetProperty(JSContext *cx, JSObject *obj, jsid iid, JSBool strict, jsval *vp){
#else
SFColorSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();
#endif

	jsval _val;
	float *cc;

#if JS_VERSION >= 185
	jsval id;
	if (!JS_IdToValue(cx,iid,&id)) {
		printf("JS_IdToValue failed in SFColorSetProperty.\n");
		return JS_FALSE;
	}
#endif
	if(SM_method() == 2){
		AnyNative *any;
		if ((any = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFColorSetProperty.\n");
			return JS_FALSE;
		}
		if(any->valueChanged)
			(*any->valueChanged)++;
		cc = any->v->sfcolor.c;
	}else{
		SFColorNative *ptr;

		if ((ptr = (SFColorNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFColorSetProperty.\n");
			return JS_FALSE;
		}
		ptr->valueChanged++;
		#ifdef JSVRMLCLASSESVERBOSE
			printf("SFColorSetProperty: obj = %p, id = %d, valueChanged = %d\n",
				   obj, JSVAL_TO_INT(id), ptr->valueChanged);
		#endif
		cc = ptr->v.c;
	}
	if (!JS_ConvertValue(cx, *vp, JSTYPE_NUMBER, &_val)) {
		printf( "JS_ConvertValue failed in SFColorSetProperty.\n");
		return JS_FALSE;
	}

	if (JSVAL_IS_INT(id)) {
		switch (JSVAL_TO_INT(id)) {
		case 0:
#if JS_VERSION < 185
			cc[0] = (float) *JSVAL_TO_DOUBLE(_val);
#else
			cc[0] = (float) JSVAL_TO_DOUBLE(_val);
#endif
			break;
		case 1:
#if JS_VERSION < 185
			cc[1] = (float) *JSVAL_TO_DOUBLE(_val);
#else
			cc[1] = (float) JSVAL_TO_DOUBLE(_val);
#endif
			break;
		case 2:
#if JS_VERSION < 185
			cc[2] = (float) *JSVAL_TO_DOUBLE(_val);
#else
			cc[2] = (float) JSVAL_TO_DOUBLE(_val);
#endif
			break;

		}
	}
	return JS_TRUE;
}

/* copy code from SFColorGetHSV if the spec ever decides to implement this. */
JSBool
#if JS_VERSION < 185
SFColorRGBAGetHSV(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
#else
SFColorRGBAGetHSV(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
#endif


	JSObject *_arrayObj;
	jsdouble hue = 0, saturation = 0, value = 0;
	jsval _v;

	UNUSED(obj);
	UNUSED(argc);
	UNUSED(argv);
	/* do conversion here!!! */

	if ((_arrayObj = JS_NewArrayObject(cx, 0, NULL)) == NULL) {
		printf( "JS_NewArrayObject failed in SFColorRGBAGetHSV.\n");
		return JS_FALSE;
	}

	/* construct new double before conversion? */
	JS_NewNumberValue(cx,hue,&_v); /* was: 	_v = DOUBLE_TO_JSVAL(&hue); */
	if (!JS_SetElement(cx, _arrayObj, 0, &_v)) {
		printf( "JS_SetElement failed for hue in SFColorRGBAGetHSV.\n");
		return JS_FALSE;
	}
	JS_NewNumberValue(cx,saturation,&_v); /* was: _v = DOUBLE_TO_JSVAL(&saturation); */
	if (!JS_SetElement(cx, _arrayObj, 1, &_v)) {
		printf( "JS_SetElement failed for saturation in SFColorRGBAGetHSV.\n");
		return JS_FALSE;
	}
	JS_NewNumberValue(cx,value,&_v); /* was: _v = DOUBLE_TO_JSVAL(&value); */
	if (!JS_SetElement(cx, _arrayObj, 2, &_v)) {
		printf( "JS_SetElement failed for value in SFColorRGBAGetHSV.\n");
		return JS_FALSE;
	}
#if JS_VERSION < 185
	*rval = OBJECT_TO_JSVAL(_arrayObj);
#else
	JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(_arrayObj));
#endif

    return JS_TRUE;
}

/* implement later?? Copy most of code from SFColorSetHSV if we require this */
JSBool
#if JS_VERSION < 185
SFColorRGBASetHSV(JSContext *cx, JSObject *obj,	uintN argc, jsval *argv, jsval *rval) {
#else
SFColorRGBASetHSV(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
#endif

	jsdouble hue, saturation, value;
	float *cc;

	if(SM_method()==2){
		AnyNative *ptr;
		if ((ptr = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFColorRGBAToString.\n");
			return JS_FALSE;
		}
		cc = ptr->v->sfcolorrgba.c;
	}else{
		SFColorRGBANative *ptr;
		if ((ptr = (SFColorRGBANative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFColorRGBAToString.\n");
			return JS_FALSE;
		}
		cc = ptr->v.c;
	}
	if (!JS_ConvertArguments(cx, argc, argv, "d d d",
							 &hue, &saturation, &value)) {
		printf( "JS_ConvertArguments failed in SFColorRGBASetHSV.\n");
		return JS_FALSE;
	}

	/* do conversion here!!! NOT DOING ANYTHING - BUG */

#if JS_VERSION < 185
	*rval = OBJECT_TO_JSVAL(obj);
#else
	JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(obj));
#endif

    return JS_TRUE;
}

JSBool
#if JS_VERSION < 185
SFColorRGBAToString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
#else
SFColorRGBAToString(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
#endif

    JSString *_str;
	char _buff[STRING];
	float *cc;

	UNUSED(argc);
	UNUSED(argv);
	if(SM_method()==2){
	    AnyNative *ptr;
		if ((ptr = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFColorRGBAToString.\n");
			return JS_FALSE;
		}
		cc = ptr->v->sfcolorrgba.c;
	
	}else{
	    SFColorRGBANative *ptr;
		if ((ptr = (SFColorRGBANative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFColorRGBAToString.\n");
			return JS_FALSE;
		}
		cc = ptr->v.c;
	}
	memset(_buff, 0, STRING);
	sprintf(_buff, "%.9g %.9g %.9g %.9g",
			cc[0], cc[1], cc[2],cc[3]);
	_str = JS_NewStringCopyZ(cx, _buff);

#if JS_VERSION < 185
    *rval = STRING_TO_JSVAL(_str);
#else
	JS_SET_RVAL(cx,vp,STRING_TO_JSVAL(_str));
#endif

    return JS_TRUE;
}

JSBool
#if JS_VERSION < 185
SFColorRGBAAssign(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
#else
SFColorRGBAAssign(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
	JSString *_id_jsstr;
#endif

    JSObject *_from_obj;
    char *_id_str;

	UNUSED(_id_str); // compiler warning mitigation
	if(SM_method() == 2){
		AnyNative *lhs, *rhs;
		if ((lhs = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed for obj in SFColorRGBAAssign.\n");
			return JS_FALSE;
		}
		//if (!JSVAL_IS_OBJECT(*vp))
		if (!(*vp).isObject())
			return JS_FALSE;
        if ((rhs = (AnyNative *)JS_GetPrivateFw(cx, JSVAL_TO_OBJECT(*vp))) == NULL) {
			printf("JS_ConvertArguments failed in SFColorRGBAAssign. \n");
			return JS_FALSE;
        }
		AnyNativeAssign(lhs,rhs);
	}else{
		SFColorRGBANative *ptr, *fptr;


		if ((ptr = (SFColorRGBANative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed for obj in SFColorRGBAAssign.\n");
			return JS_FALSE;
		}

		CHECK_CLASS(cx,obj,argv,__FUNCTION__,SFColorRGBAClass)
	
#if JS_VERSION < 185
		if (!JS_ConvertArguments(cx, argc, argv, "o s", &_from_obj, &_id_str)) {
#else
		if (JS_ConvertArguments(cx, argc, argv, "oS", &_from_obj, &_id_jsstr) == JS_TRUE) {
			_id_str = JS_EncodeString(cx,_id_jsstr);
		} else {
#endif
			printf( "JS_ConvertArguments failed in SFColorRGBAAssign.\n");
			return JS_FALSE;
		}

		CHECK_CLASS(cx,_from_obj,argv,__FUNCTION__,SFColorRGBAClass)
   
		if ((fptr = (SFColorRGBANative *)JS_GetPrivateFw(cx, _from_obj)) == NULL) {
			printf( "JS_GetPrivate failed for _from_obj in SFColorRGBAAssign.\n");
			return JS_FALSE;
		}
		#ifdef JSVRMLCLASSESVERBOSE
			printf("SFColorRGBAAssign: obj = %p, id = \"%s\", from = %p\n",
				   obj, _id_str, _from_obj);
		#endif

		SFColorRGBANativeAssign(ptr, fptr);
	}
#if JS_VERSION < 185
    *rval = OBJECT_TO_JSVAL(obj);
#else
	JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(obj));
#endif

    return JS_TRUE;
}

JSBool
#if JS_VERSION < 185
SFColorRGBAConstr(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
#else
SFColorRGBAConstr(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_NewObject(cx,&SFColorRGBAClass,NULL,NULL);
        jsval *argv = JS_ARGV(cx,vp);
#endif
	float *cc;
	jsdouble pars[4];

	ADD_ROOT(cx,obj)
	if(SM_method() == 2){
		AnyNative *any;
		if((any = (AnyNative*)AnyNativeNew(FIELDTYPE_SFColorRGBA,NULL,NULL)) == NULL){
			printf( "AnyfNativeNew failed in SFColorRGBAConstr.\n");
			return JS_FALSE;
		}
		if (!JS_SetPrivateFw(cx, obj, any)) {
			printf( "JS_SetPrivate failed in SFColorRGBAConstr.\n");
			return JS_FALSE;
		}
		cc = any->v->sfvec4f.c;
	}else{
		SFColorRGBANative *ptr;

		if ((ptr = (SFColorRGBANative *) SFColorNativeNew()) == NULL) {
			printf( "SFColorRGBANativeNew failed in SFColorConstr.\n");
			return JS_FALSE;
		}

		//if (!JS_DefineProperties(cx, obj, SFColorRGBAProperties)) {
		//	printf( "JS_DefineProperties failed in SFColorRGBAConstr.\n");
		//	return JS_FALSE;
		//}

		if (!JS_SetPrivateFw(cx, obj, ptr)) {
			printf( "JS_SetPrivate failed in SFColorRGBAConstr.\n");
			return JS_FALSE;
		}
		cc = ptr->v.c;
		ptr->valueChanged = 1;

	}
	if (argc == 0) {
		cc[0] = (float) 0.0;
		cc[1] = (float) 0.0;
		cc[2] = (float) 0.0;
		cc[3] = (float) 0.0;
	} else if (JS_ConvertArguments(cx, argc, argv, "d d d d",
					&(pars[0]), &(pars[1]), &(pars[2]), &(pars[3]))) {
		cc[0] = (float) pars[0];
		cc[1] = (float) pars[1];
		cc[2] = (float) pars[2];
		cc[3] = (float) pars[3];
	} else {
		printf( "Invalid arguments for SFColorRGBAConstr.\n");
		return JS_FALSE;
	}

	

	#ifdef JSVRMLCLASSESVERBOSE
		printf("SFColorRGBAConstr: obj = %p %u args, %f %f %f %f\n",
			   obj, argc,
			  cc[0], cc[1], cc[2],cc[3]);
	#endif
#if JS_VERSION < 185
	*rval = OBJECT_TO_JSVAL(obj);
#else
	JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(obj));
#endif

	return JS_TRUE;
}

JSBool
#if JS_VERSION < 185
SFColorRGBAGetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp){
#elif JS_VERSION == 185
SFColorRGBAGetProperty(JSContext *cx, JSObject *obj, jsid iid, jsval *vp){
	//jsval id;
#else
SFColorRGBAGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();
	//jsval id = JS_NumberValue(0.0); = INT_TO_JSVAL(0)
#endif

	jsdouble d;
	float *cc;

#if JS_VERSION >= 185
	jsval id;
	if (!JS_IdToValue(cx,iid,&id)) {
		printf("JS_IdToValue failed in SFColorRGBAGetProperty.\n");
		return JS_FALSE;
	}
#endif

	if(SM_method()==2){
		AnyNative *any;
		if ((any = (AnyNative *)JS_GetPrivateFw(cx,obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFColorRGBAGetProperty.\n");
			return JS_FALSE;
		}
		cc = any->v->sfcolorrgba.c;
	}else{
		SFColorRGBANative *ptr;
		if ((ptr = (SFColorRGBANative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFColorRGBAGetProperty.\n");
			return JS_FALSE;
		}
		cc = ptr->v.c;
	}
	if (JSVAL_IS_INT(id)) {
		int kk = 1;
		switch (JSVAL_TO_INT(id)) {
		case 0:
			d = cc[0];
			if (JS_NewNumberValue(cx, d, vp) == JS_FALSE) {
				printf(
						"JS_NewDouble failed for %f in SFColorRGBAGetProperty.\n",
						d);
				return JS_FALSE;
			}
			break;
		case 1:
			d = cc[1];
			if (JS_NewNumberValue(cx, d, vp) == JS_FALSE) {
				printf(
						"JS_NewDouble failed for %f in SFColorRGBAGetProperty.\n",
						d);
				return JS_FALSE;
			}
			break;
		case 2:
			d = cc[2];
			if (JS_NewNumberValue(cx, d, vp) == JS_FALSE) {
				printf(
						"JS_NewDouble failed for %f in SFColorRGBAGetProperty.\n",
						d);
				return JS_FALSE;
			}
			break;
		case 3:
			d = cc[3];
			if (JS_NewNumberValue(cx, d, vp) == JS_FALSE) {
				printf(
						"JS_NewDouble failed for %f in SFColorRGBAGetProperty.\n",
						d);
				return JS_FALSE;
			}
			break;
		}
	}
	return JS_TRUE;
}

JSBool
#if JS_VERSION < 185
SFColorRGBASetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp){
#elif JS_VERSION == 185
SFColorRGBASetProperty(JSContext *cx, JSObject *obj, jsid iid, JSBool strict, jsval *vp){
#else
SFColorRGBASetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();
#endif

	jsval _val;
	float *cc;

#if JS_VERSION >= 185
	jsval id;
	if (!JS_IdToValue(cx,iid,&id)) {
		printf("JS_IdToValue failed in SFColorRGBASetProperty.\n");
		return JS_FALSE;
	}
#endif
	if(SM_method() == 2){
		AnyNative *any;
		if ((any = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFColorRGBASetProperty.\n");
			return JS_FALSE;
		}
		if(any->valueChanged)
			(*any->valueChanged)++;
		cc = any->v->sfcolorrgba.c;
	}else{
		SFColorRGBANative *ptr;

		if ((ptr = (SFColorRGBANative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFColorRGBASetProperty.\n");
			return JS_FALSE;
		}
		ptr->valueChanged++;
		#ifdef JSVRMLCLASSESVERBOSE
			printf("SFColorRGBASetProperty: obj = %p, id = %d, valueChanged = %d\n",
				   obj, JSVAL_TO_INT(id), ptr->valueChanged);
		#endif
		cc = ptr->v.c;
	}
	if (!JS_ConvertValue(cx, *vp, JSTYPE_NUMBER, &_val)) {
		printf( "JS_ConvertValue failed in SFColorRGBASetProperty.\n");
		return JS_FALSE;
	}

	if (JSVAL_IS_INT(id)) {
		switch (JSVAL_TO_INT(id)) {
		case 0:
#if JS_VERSION < 185
			cc[0] = (float) *JSVAL_TO_DOUBLE(_val);
#else
			cc[0] = (float) JSVAL_TO_DOUBLE(_val);
#endif
			break;
		case 1:
#if JS_VERSION < 185
			cc[1] = (float) *JSVAL_TO_DOUBLE(_val);
#else
			cc[1] = (float) JSVAL_TO_DOUBLE(_val);
#endif
			break;
		case 2:
#if JS_VERSION < 185
			cc[2] = (float) *JSVAL_TO_DOUBLE(_val);
#else
			cc[2] = (float) JSVAL_TO_DOUBLE(_val);
#endif
			break;
		case 3:
#if JS_VERSION < 185
			cc[3] = (float) *JSVAL_TO_DOUBLE(_val);
#else
			cc[3] = (float) JSVAL_TO_DOUBLE(_val);
#endif
			break;

		}
	}
	return JS_TRUE;
}

JSBool
#if JS_VERSION < 185
SFImageToString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
#else
SFImageToString(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
	jsval rval;
	JSBool retval;
#endif

	#ifdef JSVRMLCLASSESVERBOSE
		printf("SFImageToString: obj = %p, %u args\n", obj, argc);
	#endif

	UNUSED(argc);
	UNUSED(argv);
	
#if JS_VERSION < 185
	return doMFToString(cx, obj, "SFImage", rval);
#else
	retval = doMFToString(cx, obj, "SFImage", &rval);
	JS_SET_RVAL(cx,vp,rval);
	return retval;
#endif
}

JSBool
#if JS_VERSION < 185
SFImageAssign(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
#else
SFImageAssign(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
	jsval rval;
	JSBool retval;
#endif

	#ifdef JSVRMLCLASSESVERBOSE
		printf("SFImageAssign: obj = %p, %u args\n", obj, argc);
	#endif

#if JS_VERSION < 185
	return _standardMFAssign (cx, obj, argc, argv, rval, &SFImageClass,FIELDTYPE_SFImage);
#else
	retval = _standardMFAssign (cx, obj, argc, argv, &rval, &SFImageClass,FIELDTYPE_SFImage);
	JS_SET_RVAL(cx,vp,rval);
	return retval;
#endif
}

JSBool
#if JS_VERSION < 185
SFImageConstr(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
#else
SFImageConstr(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_NewObject(cx,&SFImageClass,NULL,NULL);
        jsval *argv = JS_ARGV(cx,vp);
#endif
	unsigned int i;
	jsval mv;
	int param[3];
	int expectedSize;



	ADD_ROOT(cx,obj)

	#ifdef JSVRMLCLASSESVERBOSE
		printf("SFImageConstr: obj = %p, %u args\n", obj, argc);
	#endif

	/* SFImage really only has the valueChanged flag. */
	if(SM_method()==2){
        AnyNative *ptr;
        if ((ptr = (AnyNative *) AnyNativeNew(FIELDTYPE_SFImage,NULL,NULL)) == NULL) {
                printf( "SFImageNativeNew failed in SFImageConstr.\n");
                return JS_FALSE;
        }

        if (!JS_SetPrivateFw(cx, obj, ptr)) {
                printf( "JS_SetPrivate failed in SFImageConstr.\n");
                return JS_FALSE;
        }
		//if(ptr->valueChanged)
		//	(*ptr->valueChanged) = 1;

	}else{
        SFImageNative *ptr;
        if ((ptr = (SFImageNative *) SFImageNativeNew()) == NULL) {
                printf( "SFImageNativeNew failed in SFImageConstr.\n");
                return JS_FALSE;
        }

        if (!JS_SetPrivateFw(cx, obj, ptr)) {
                printf( "JS_SetPrivate failed in SFImageConstr.\n");
                return JS_FALSE;
        }

		ptr->valueChanged = 1;
	}
	/* make this so that one can get the ".x", ".y", ".comp" and ".array" */
	//if (!JS_DefineProperties(cx, obj, SFImageProperties)) {
	//	printf( "JS_DefineProperties failed in SFImageConstr.\n");
	//	return JS_FALSE;
	//}

	/* null image. Make this [0, 0, 0] NOTE - there are only 3 elements now! */
	if (!argc) { 
		/* expect arguments to be number, number, number, mfint32 */
		mv = INT_TO_JSVAL(0);
		for (i=0; i<4; i++) {
			if (i==3) {
#if JS_VERSION < 185
				MFInt32Constr(cx, obj, 0, NULL, &mv);
#else
				/* note - old default constructor call allocates a new obj and assigns to mv,
				 * but calling fn directly may not actually do that. It seems illegal to call
				 * the constructor of another object directly on this object, and then feed it
				 * as a proprety of itself, but since that's what the old code did (and it seems
				 * to work), am keeping it as-is. 
				 *
				 * I would -expect- that 'JS_NewObject(cx,&MFInt32Class,NULL,NULL)' should be
				 * used instead of 'obj' below.... */
				MFInt32ConstrInternals(cx, obj, 0, NULL, &mv);
#endif
			}
			if (!JS_DefineElement(cx, obj, (jsuint) i, mv, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB6, JSPROP_ENUMERATE)) {
				printf( "JS_DefineElement failed for arg %d in SFImageConstr.\n", i);
				return JS_FALSE;
			}
		}
		DEFINE_LENGTH(cx,obj,4)
#if JS_VERSION >= 185
		/* returning with success here, so must set rval to return the object we just finished creating */
		JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(obj));
#endif
		return JS_TRUE; 
	}
	
	/* ok, ok. There are some parameters here. There had better be 4, or else... */
	if ((argc != 4) && (argc != 3)) {
		printf ("SFImageConstr, expect 4 parameters, got %d\n",argc);
		return JS_FALSE;
	}


	DEFINE_LENGTH(cx,obj,argc)

	/* expect arguments to be number, number, number, mfint32 */
	for (i=0; i<3; i++) {
		/* printf ("looking at parameter %d\n",i); */
        	if (JSVAL_IS_INT(argv[i])) { 
			/* printf ("parameter is a number\n"); */
                	param[i] =  JSVAL_TO_INT(argv[i]);
			/* printf ("param is %d\n",param[i]); */
        	} else {        
                	printf ("SFImageConstr: parameter %d is not a number\n",i);
                	return JS_FALSE;
		}
	}
	/* now look at the MFInt32 array, and tack it on here */
	expectedSize = param[0] * param[1];

	
	/* the third number should be in the range of 0-4 inclusive (number of components in image) */
	if ((param[2]<0) || (param[2]>4)) {
			
		printf ("SFImageConstr: with size > 0, comp must be between 1 and 4 inclusive, got %d\n",param[2]);
		return JS_FALSE;
	}

	/* case 1 of null initializer */
	if ((expectedSize == 0) && (param[2] != 0)) {
		printf ("SFImageConstr: with x and y equal to zero, comp must be zero\n");
		return JS_FALSE;
	}
	/* case 2 of null initializer */
	if ((expectedSize != 0) && (param[2] == 0)) {
		printf ("SFImageConstr: with x and y not zero, comp must be non-zero\n");
		return JS_FALSE;
	}

	/* worry about the MFInt32 array. Note that we copy the object pointer here. Should
	   we copy ALL of the elements, or just the object itself?? */

	if (argc == 4) {
		#ifdef JSVRMLCLASSESVERBOSE
		printJSNodeType(cx,JSVAL_TO_OBJECT(argv[3]));
		#endif
 
		CHECK_CLASS(cx,JSVAL_TO_OBJECT(argv[3]),NULL,__FUNCTION__,MFInt32Class)
		if (!JS_GetProperty(cx, JSVAL_TO_OBJECT(argv[3]),  MF_LENGTH_FIELD, &mv)) {
			printf( "JS_GetProperty failed for MFInt32 length in SFImageConstr\n");
	        	return JS_FALSE;
		}
	        if (expectedSize != JSVAL_TO_INT(mv)) {
			printf ("SFImageConstr: expected %d elements in image data, got %d\n",expectedSize, JSVAL_TO_INT(mv));
			return JS_FALSE;
		}
	}

	/* parameters are ok - just save them now in the new object. */
	for (i=0; i<argc; i++) {
		if (!JS_DefineElement(cx, obj, (jsint) i, argv[i], JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB6, JSPROP_ENUMERATE)) {
			printf( "JS_DefineElement failed for arg %d in SFImageConstr.\n", i);
			return JS_FALSE;
		}
	}
	
	/* if we are here, we must have had some success... */
#if JS_VERSION < 185
	*rval = OBJECT_TO_JSVAL(obj);
#else
	JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(obj));
#endif
	return JS_TRUE;
}

JSBool
SFImageAddProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp) {
	return doMFAddProperty(cx, obj, id, vp, "SFImage"); //FIXME: is this ok ??? "SFImageAddProperty");
}

#if JS_VERSION < 186
JSBool SFImageGetProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp) {
#else
JSBool SFImageGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid id = *hiid.address();
	jsval *vp = hvp.address();
#endif
	return _standardMFGetProperty(cx, obj, id, vp, "_FreeWRL_Internal = 0", FIELDTYPE_SFImage); //FIXME: is this ok ???  "SFImage");
}


JSBool
#if JS_VERSION < 185
SFImageSetProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp) {
#elif JS_VERSION == 185
SFImageSetProperty(JSContext *cx, JSObject *obj, jsid id, JSBool strict, jsval *vp){
#else
SFImageSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid id = *hiid.address();
	jsval *vp = hvp.address();
#endif	
	return doMFSetProperty(cx, obj, id, vp, FIELDTYPE_SFImage);
}

/**********************************************************************************/


/* returns a string rep of the pointer to the node in memory */
JSBool
#if JS_VERSION < 185
SFNodeValueOf(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
#else
SFNodeValueOf(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
	jsval rvalinst;
	jsval *rval = &rvalinst;
#endif
	SFNodeNative *ptr;
	void* handle;

	UNUSED(argc);
	UNUSED(argv);
	if(SM_method() == 2){
		AnyNative *nany;
		if ((nany = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFNodeToString.\n");
			return JS_FALSE;
		}
		handle = nany->v->sfnode;
	}else{
		#ifdef JSVRMLCLASSESVERBOSE
		printf ("SFNODETOSTRING\n");
		#endif
		if ((ptr = (SFNodeNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFNodeToString.\n");
			return JS_FALSE;
		}
		handle = ptr->handle;
	}
	/* get the string from creation, and return it. */

	/* used to do: 
	*rval = INT_TO_JSVAL(ptr->handle);
	
	but we have 64 bit pointers in OSX now, and ints are 32 bits. so...
	we convert to a double, and hope that it is still correct (seems to be ok
	32 and 64 bits - tests/46.wrl will use this path, btw */

	{
		jsdouble nv;
		char tmpline[100];
		sprintf (tmpline,"%p",handle);
		/* sprintf (tmpline,"%ld",ptr->handle); */

		/* printf ("pointer to long int :%s:\n",tmpline); */

		nv = strtod(tmpline,NULL);
		/* printf ("double is %lf\n",nv); */

		/* printf ("nv %lf, handle %lu and %p\n",nv,ptr->handle,ptr->handle); */
		if (!JS_NewNumberValue(cx, nv, rval)) {
			ConsoleMessage ("Conversion issue in SFNodeToString");
		}
	}
	

	#ifdef JSVRMLCLASSESVERBOSE
	printf ("SFNodeToString, handle %p ",ptr->handle);
	printf ("SFNodeToString, handle as unsignned  %u ",(unsigned int)ptr->handle);
	if (ptr->handle != NULL) {
		printf (" (%s) ", stringNodeType (((struct X3D_Node *)ptr->handle)->_nodeType));
	}
	printf ("string \"%s\"\n",ptr->X3DString);
	#endif

#if JS_VERSION >= 185
	JS_SET_RVAL(cx,vp,*rval);
#endif
	return JS_TRUE;
}

JSBool
#if JS_VERSION < 185
SFNodeToString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
#else
SFNodeToString(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
	jsval rvalinst;
	jsval *rval = &rvalinst;
#endif
    JSString *_str;
	SFNodeNative *ptr;
	void *handle;

	UNUSED(argc);
	UNUSED(argv);
	#ifdef JSVRMLCLASSESVERBOSE
	printf ("SFNODETOSTRING\n");
	#endif
	if(SM_method() == 2){
		AnyNative *nany;
		if ((nany = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFNodeToString.\n");
			return JS_FALSE;
		}
		handle = nany->v->sfnode;
	}else{

		if ((ptr = (SFNodeNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFNodeToString.\n");
			return JS_FALSE;
		}
		handle = ptr->handle;
	}

	/* get the string from creation, and return it. */

	/* used to do: 
	*rval = INT_TO_JSVAL(ptr->handle);
	
	but we have 64 bit pointers in OSX now, and ints are 32 bits. so...
	we convert to a double, and hope that it is still correct (seems to be ok
	32 and 64 bits - tests/46.wrl will use this path, btw */

	{
		jsdouble nv;
		char buff[STRING];
		memset(buff, 0, STRING);
		sprintf (buff,"_%p_",handle);
		/* sprintf (tmpline,"%ld",ptr->handle); */

		/* printf ("pointer to long int :%s:\n",tmpline); */
		ADD_ROOT(cx,_str)
		_str = JS_NewStringCopyZ(cx, buff);

#if JS_VERSION < 185
		*rval = STRING_TO_JSVAL(_str);
#else
		JS_SET_RVAL(cx,vp,STRING_TO_JSVAL(_str));
#endif
	
		REMOVE_ROOT (cx,_str)

	}
	
	return JS_TRUE;
}



JSBool
#if JS_VERSION < 185
SFNodeAssign(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
#else
SFNodeAssign(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
	JSString *_id_jsstr;
#endif

	JSObject *_from_obj;
	SFNodeNative *fptr, *ptr;
	char *_id_str;

	UNUSED(_id_str); // compiler warning mitigation
	if(SM_method() == 2){
		AnyNative *lhs, *rhs;
		//if (JSVAL_IS_OBJECT(*vp)) {
		if ((*vp).isObject()){
        	if ((rhs = (AnyNative *)JS_GetPrivateFw(cx, JSVAL_TO_OBJECT(*vp))) != NULL) {
				if ((lhs = (AnyNative *)JS_GetPrivateFw(cx, obj)) != NULL) {
					AnyNativeAssign(lhs,rhs);
				}
			}
		}
	}else{


		/* unsigned int toptr; */
		#ifdef JSVRMLCLASSESVERBOSE
		printf ("start of SFNodeAssign argc %d\n",argc);
		#endif

		/* are we saving to a SFNode? */
		CHECK_CLASS(cx,obj,argv,__FUNCTION__,SFNodeClass)

		/* get the pointer to the internal stuff */
		if ((ptr = (SFNodeNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed for obj in SFNodeAssign.\n");
			return JS_FALSE;
		}

		/* get the from, and the string */
		#ifdef JSVRMLCLASSESVERBOSE
		printf ("SFNodeAssign, we have %d and %d\n",(int)argv[0], (int)argv[1]);
		#endif

#if JS_VERSION < 185
		if (!JS_ConvertArguments(cx, argc, argv, "o s", &_from_obj, &_id_str)) {
#else
		if (JS_ConvertArguments(cx, argc, argv, "oS", &_from_obj, &_id_jsstr) == JS_TRUE) {
			_id_str = JS_EncodeString(cx,_id_jsstr);
		} else {
#endif
			printf( "JS_ConvertArguments failed in SFNodeAssign.\n");
			return JS_FALSE;
		}
		if (_from_obj != NULL) {
			CHECK_CLASS(cx,_from_obj,argv,__FUNCTION__,SFNodeClass)

			if ((fptr = (SFNodeNative *)JS_GetPrivateFw(cx, _from_obj)) == NULL) {
				printf( "JS_GetPrivate failed for _from_obj in SFNodeAssign.\n");
				return JS_FALSE;
			}
			#ifdef JSVRMLCLASSESVERBOSE
				printf("SFNodeAssign: obj = %p, id = \"%s\", from = %p\n",
					   obj, _id_str, _from_obj);
			#endif
		} else { fptr = NULL; }

		/* assign this internally */
		if (!SFNodeNativeAssign(ptr, fptr)) {
			printf( "SFNodeNativeAssign failed in SFNodeAssign.\n");
			return JS_FALSE;
		}
	} //SMmethod == 2
#if JS_VERSION < 185
	*rval = OBJECT_TO_JSVAL(obj);
#else
	JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(obj));
#endif

	#ifdef JSVRMLCLASSESVERBOSE
	printf ("end of SFNodeAssign\n");
	#endif
	return JS_TRUE;
}


// https://developer.mozilla.org/en-US/docs/Mozilla/Projects/SpiderMonkey/Releases/1.8.5
// when adding a node.function() don't forget to add a check in SFNodeGetProperty for "function"
JSBool
#if JS_VERSION < 185
SFNodeEquals(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	jsval rvalinst;
	jsval *rval = &rvalinst;
#else
SFNodeEquals(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
#endif
	int iret;
	JSObject *_from_obj;
    SFNodeNative *ptr, *fptr;

	if(SM_method() == 2){
		AnyNative *lhs, *rhs;
		iret = 0;
		//if (JSVAL_IS_OBJECT(*vp)) {
		if ((*vp).isObject()) {
        	if ((rhs = (AnyNative *)JS_GetPrivateFw(cx, JSVAL_TO_OBJECT(*vp))) != NULL) {
				if ((lhs = (AnyNative *)JS_GetPrivateFw(cx, obj)) != NULL) {
					iret = lhs->v->sfnode == rhs->v->sfnode ? 1 : 0;
				}
			}
		}
	}else{

		//JS_SET_RVAL(cx,vp,BOOLEAN_TO_JSVAL(1)); //JS_TRUE);
		//return JS_TRUE;
		if ((ptr = (SFNodeNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFNodeNative.\n");
			return JS_FALSE;
		}
		if (!JS_ConvertArguments(cx, argc, argv, "o",
								 &_from_obj)) {
			printf( "JS_ConvertArguments failed in SFNodeNative.\n");
			return JS_FALSE;
		}


		if (_from_obj != NULL) {
			CHECK_CLASS(cx,_from_obj,argv,__FUNCTION__,SFNodeClass)

			if ((fptr = (SFNodeNative *)JS_GetPrivateFw(cx, _from_obj)) == NULL) {
				printf( "JS_GetPrivate failed for _from_obj in SFNodeAssign.\n");
				return JS_FALSE;
			}
			#ifdef JSVRMLCLASSESVERBOSE
				printf("SFNodeAssign: obj = %p, id = \"%s\", from = %p\n",
					   obj, _id_str, _from_obj);
			#endif
		} else { fptr = NULL; }


		/* assign this internally */
		iret = SFNodeNativeEquals(ptr, fptr);
	} //SM_method == 2
#if JS_VERSION < 185
    *rval = BOOLEAN_TO_JSVAL(iret);
#else
	JS_SET_RVAL(cx,vp,BOOLEAN_TO_JSVAL(iret));
#endif
	
	#ifdef JSVRMLCLASSESVERBOSE
	printf ("end of SFNodeEqual\n");
	#endif

    return JS_TRUE;
}



/* define JSVRMLCLASSESVERBOSE */

JSBool
#if JS_VERSION < 185
SFNodeConstr(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
#else
SFNodeConstr(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_NewObject(cx,&SFNodeClass,NULL,NULL);
        jsval *argv = JS_ARGV(cx,vp);
#endif
	/*
	X3D ecmascript specs:
	http://www.web3d.org/files/specifications/19777-1/V3.0/Part1/functions.html#SFNodeInstanceCreationFunction
	"7.6.5.2 Instance creation function
	This object cannot be directly instantiated"
	That means there should never be myNode = new SFNode(); or SFNode('string') or SFNode('0x12345');
	X3DScene Browser.createX3DFromString(String x3dSyntax)

	VRML vrmlscript specs:
	http://www.web3d.org/x3d/specifications/vrml/ISO-IEC-14772-VRML97/part1/javascript.html#SFNode 
	sfNodeObjectName = new SFNode(String vrmlstring);
	MFNode Browser.createVrmlFromString( String vrmlSyntax )

	The support below is for VRML specs SFNode(vrmlstring), 
	plus 2 freewrl-specific techniques: 
	sfn = new SFNode(oldSFNode)
	sfn = new SFNode('0x12345'); //ptr to another node

	- a char* ptr->X3Dstring holding vrmlstring from a previous new SFNode(string)
	is dragged around with an SFNode, and when new SFNode(old) it resends the 
	string to the parser to create the new node.
	- this should fail when oldnode wasn't created with new SFNode(vrmlstring).
	(a future broto version would/could possibly deep copy the binary oldnode, 
	elliminating the need for saving/holding the X3DString, and allowing it to 
	work even when the oldNode wasn't created with a new SFNode(vrmlstring) constructor call)

	*/
	SFNodeNative *newPtr;
	SFNodeNative *oldPtr;

	struct X3D_Node *newHandle;
	JSString *myStr;
	char *cString;

	/* unused struct X3D_Group *myGroup; */

	ADD_ROOT(cx,obj)
	newHandle = NULL;
	cString = NULL;

	#ifdef JSVRMLCLASSESVERBOSE
	printf ("Start of SFNodeConstr argc %d object %p\n",argc,obj);
	#endif

	/* verify the argc */
	if (argc == 0) {
		newHandle = NULL;
		cString = STRDUP("SFNodeConstr from argc eq 0");
	} else if (argc == 1) {
		/* is this a string, or a number indicating a node? */
		myStr = JS_ValueToString(cx, argv[0]);
#if JS_VERSION < 185
		cString = JS_GetStringBytes(myStr);
#else
		cString = JS_EncodeString(cx,myStr);
#endif
		#ifdef JSVRMLCLASSESVERBOSE
		printf ("SFNodeConstr, argc =1l string %s\n",cString);
		#endif

		/* this is either a memory pointer, or it is actual X3D text, or it is junk */
		//if (JSVAL_IS_OBJECT(argv[0])) {
		if (argv[0].isObject()) {
			/* myNode2 = new SFNode(myNode1); not in specs*/
			#ifdef JSVRMLCLASSESVERBOSE
			printf ("SFNodeConstr, cstring was an object\n");
			#endif
			if(SM_method() == 2){
				AnyNative *rhs;
       			if ((rhs = (AnyNative *)JS_GetPrivateFw(cx, JSVAL_TO_OBJECT(argv[0]))) == NULL) {
				#ifdef JSVRMLCLASSESVERBOSE
                		printf( "JS_GetPrivate failed in SFNodeConstr.\n");
				#endif
                		return JS_FALSE;
        		}

				newHandle = rhs->v->sfnode;

			}else{
        		if ((oldPtr = (SFNodeNative *)JS_GetPrivateFw(cx, JSVAL_TO_OBJECT(argv[0]))) == NULL) {
				#ifdef JSVRMLCLASSESVERBOSE
                		printf( "JS_GetPrivate failed in SFNodeConstr.\n");
				#endif
                		return JS_FALSE;
        		}

				newHandle = oldPtr->handle;
			}
			if(SM_method() == 0)
				cString = STRDUP(oldPtr->X3DString);

		} else {
			#ifdef JSVRMLCLASSESVERBOSE
			printf ("SFNodeConstr, cstring was NOT an object\n");
			#endif

			/* check if the first character is numeric, if it is then assume a pointer */
			if (!((cString[0] >= 'A' && cString [0] <= 'Z')||(cString[0] >= 'a' && cString [0] <= 'z'))) {
				/* lets hope this is an initializer, like "new SFNode("0x100675790") */
				if (sscanf (cString,"%p",&newHandle) != 1) {
					ConsoleMessage ("expected pointer for Javascript SFNode constr, got :%s:",cString);
					newHandle = NULL;
				#ifdef JSVRMLCLASSESVERBOSE
					printf ("SFNodeConstr, expected pointer for Javascript SFNode constr, got :%s:\n",cString);
				} else {
					printf ("SFNodeConstr, got pointer for Javascript SFNode constr, :%p:\n",newHandle);
				#endif
				}
			} else {
				/* cannot be an initializer, must parse the string */
				/* myNode = new SFNode('Group{...}'); */
				/* try compiling this X3D code... */
				resource_item_t* res = resource_create_from_string(cString);
				//where put any script defined DEF names? 
				// 1= throw-away Proto context 2) Script node parent x3d context (inline, scene, protobody)
				// they both tested working May 13, 2022 but with unknown side effects
				int defname_context_method = 2; 
				if (defname_context_method == 1) {
					struct X3D_Proto* myProto = (struct X3D_Proto*)createNewX3DNode(NODE_Proto);
					res->whereToPlaceData = myProto;
					res->ectx = myProto;
					res->offsetFromWhereToPlaceData = (int)offsetof(struct X3D_Proto, __children);
					res->media_type = resm_vrml;
					res->parsed_request = strdup("From the EAI bootcamp of life ");
					parser_process_res_VRML_X3D(res);
					newHandle = X3D_NODE(myProto->__children.p[0]);

				}
				else if(defname_context_method == 2) {
					struct X3D_Group* myGroup = (struct X3D_Group*)createNewX3DNode(NODE_Group);
					res->whereToPlaceData = myGroup;
					res->ectx = JS_GetContextPrivate(cx); //executionContext the script is in, stored using JS_SetContextPrivate 
					res->offsetFromWhereToPlaceData = (int)offsetof(struct X3D_Group, children);
					res->media_type = resm_vrml;
					res->parsed_request = strdup("From the EAI bootcamp of life ");
					parser_process_res_VRML_X3D(res);
					newHandle = X3D_NODE(myGroup->children.p[0]);

				}
				//res->media_type = resm_vrml;
				//res->parsed_request = strdup("From the EAI bootcamp of life ");
				//#ifdef JSVRMLCLASSESVERBOSE
				//printf ("SFNodeConstr, sending resource to parser\n");
				//#endif
				////send_resource_to_parser(res);
				//#ifdef JSVRMLCLASSESVERBOSE
				//printf ("SFNodeConstr, waiting on resource\n");
				//#endif
				////resource_wait(res);

				//#ifdef JSVRMLCLASSESVERBOSE
				//printf ("SFNodeConstr we have created %d nodes\n",myGroup->children.n);
				//#endif

				///* we MUST create 1 node here; if not, there is an error */
				////if ((myGroup->children.n) != 1) {
				////	ConsoleMessage ("SFNativeNew - created %d nodes, expected 1 only\n",myGroup->children.n);
				////	return JS_FALSE;
				////}
				//parser_process_res_VRML_X3D(res);
				//if(defname_context_method == 1)
				//	newHandle = X3D_NODE(myProto->__children.p[0]);
				//else if(defname_context_method == 2)
				//	newHandle = X3D_NODE(myGroup->children.p[0]);
			}
			
			cString = STRDUP("node created in SFNodeConstr");
		}	

	} else if (argc == 2) {
		/* eg, createVrmlFromString will send a bunch of SFNodes to a MFNode with text */

		#ifdef JSVRMLCLASSESVERBOSE
		printf ("SFNodeConstr - have 2 arguments\n");
		#endif

		if ((JSVAL_IS_STRING(argv[0])) && (JSVAL_IS_STRING(argv[1]))) {
        		JSString *_idStr;
        		char *_id_c;

			_idStr = JS_ValueToString(cx, argv[0]);
#if JS_VERSION < 185
			_id_c = JS_GetStringBytes(_idStr);
#else
			_id_c = JS_EncodeString(cx,_idStr);
#endif
			/* printf ("first string :%s:\n",_id_c); */

			cString = STRDUP(_id_c);

			_idStr = JS_ValueToString(cx, argv[1]);
#if JS_VERSION < 185
			_id_c = JS_GetStringBytes(_idStr);
#else
			_id_c = JS_EncodeString(cx,_idStr);
#endif
			/* printf ("second string :%s:\n",_id_c); */

			if (sscanf (_id_c,"%p",&newHandle) != 1) {
				printf ("SFNodeConstr - can not get handle from %s\n",_id_c);
				return JS_FALSE;
			}
/* 			nope, need to do this as a pointer string.. newHandle = (struct X3D_Node *) JSVAL_TO_GCTHING(argv[1]); */

			#ifdef JSVRMLCLASSESVERBOSE
			printf ("string is :%s: new handle is %p\n",cString,newHandle);
			#endif

		} else {
			printf ("SFNodeConstr - 2 args, expected 2 strings\n");
			return JS_FALSE;
		}


	} else {
		printf( "SFNodeConstr requires at least 1 string arg.\n");
		return JS_FALSE;
	}

	if(SM_method() == 2){
		AnyNative *lhs;
		if ((lhs = (AnyNative *) AnyNativeNew(FIELDTYPE_SFNode,NULL,NULL)) == NULL) {
			printf( "AnyNativeNew failed in SFNodeConstr.\n");
			return JS_FALSE;
		}
		if (!JS_SetPrivateFw(cx, obj, lhs)) {
			printf( "JS_SetPrivate failed in SFNodeConstr.\n");
			return JS_FALSE;
		}
		//lhs->valueChanged = NULL; 
		lhs->v->sfnode = newHandle;
	}else{
		/* ok, so far so good... */
		if ((newPtr = (SFNodeNative *) SFNodeNativeNew()) == NULL) {
			printf( "SFNodeNativeNew failed in SFNodeConstr.\n");
			return JS_FALSE;
		}

		//if (!JS_DefineProperties(cx, obj, SFNodeProperties)) {
		//	printf( "JS_DefineProperties failed in SFNodeConstr.\n");
		//	return JS_FALSE;
		//}

		if (!JS_SetPrivateFw(cx, obj, newPtr)) {
			printf( "JS_SetPrivate failed in SFNodeConstr.\n");
			return JS_FALSE;
		}

		newPtr->handle = newHandle;
		if(SM_method() == 0){
			newPtr->X3DString = (char *)STRDUP(cString);

			if (!JS_DefineSFNodeSpecificProperties (cx, obj, newHandle)) {
				printf( "JS_DefineSFNodeSpecificProperties failed in SFNodeConstr.\n");
				return JS_FALSE;

			}
		}
	
		newPtr->valueChanged = 1;
	} //SM_method == 2

	#ifdef JSVRMLCLASSESVERBOSE
	{
		if (newHandle == NULL) 
			printf("end of SFNodeConstr: created obj = %p, argc: %u mem ptr: %p (null pointer) text string: %s\n",
			   obj, argc, newHandle, cString);
		else 
			printf("end of SFNodeConstr: created obj = %p, argc: %u mem ptr: %p (%s) text string: %s\n",
			   obj, argc, newHandle, stringNodeType(newHandle->_nodeType),cString);
	}
	#endif

#if JS_VERSION < 185
	*rval = OBJECT_TO_JSVAL(obj);
#else
	JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(obj));
#endif

	return JS_TRUE;
}

/* undef JSVRMLCLASSESVERBOSE */

void
#if JS_VERSION < 186
SFNodeFinalize(JSContext *cx, JSObject *obj){
#else
SFNodeFinalize(JSFreeOp *fop, JSObject *obj){
JSContext *cx = NULL;
#endif
	SFNodeNative *ptr;
	AnyNative *any;

	#ifdef JSVRMLCLASSESVERBOSE
		printf("SFNodeFinalize: obj = %p\n", obj);
	#endif

	REMOVE_ROOT(cx,obj)

	/*so, it appears that recent (2010) versions ofJavascript will give the following error when
	  the interpreter is shutdown. It appears that sending in the url will cause a SFNode to
	  be created, even though the normal constructor is not called, eg:

	DEF t Script {
		url "vrmlscript:
        		function eventsProcessed () {}
		"
	}
	
	will cause the following JS_GetPrivate to fail. */
	if ((ptr = (SFNodeNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
		/* see above printf( "JS_GetPrivate failed in SFNodeFinalize.\n"); */
		return;
	} else {
		if(SM_method() == 0)
				FREE_IF_NZ (ptr->X3DString); 
		if(SM_method() == 2){
			any = (AnyNative*)ptr;
			if(any->gc) FREE_IF_NZ(any->v);
		}

		FREE_IF_NZ (ptr);
		JS_SetPrivateFw(cx,obj,NULL);
	}
}


void X3D_ECMA_TO_JS(JSContext *cx, void *Data, int datalen, int dataType, jsval *newval);
void X3D_MF_TO_JS(JSContext *cx, JSObject *obj, void *Data, int dataType, jsval *newval, char *fieldName);
void X3D_SF_TO_JS(JSContext *cx, JSObject *obj, void *Data, unsigned datalen, int dataType, jsval *newval);
void X3D_MF_TO_JS_B(JSContext *cx, void *Data, int dataType, int *valueChanged, jsval *newval);
void X3D_SF_TO_JS_B(JSContext *cx, void *Data, unsigned datalen, int dataType, int *valueChanged, jsval *newval);
int getFieldFromNodeAndName(struct X3D_Node* node,const char *fieldname, int *type, int *kind, int *iifield, union anyVrml **value);
JSBool
#if JS_VERSION < 185
SFNodeGetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp){
#elif JS_VERSION == 185
SFNodeGetProperty(JSContext *cx, JSObject *obj, jsid iid, jsval *vp){
#else
SFNodeGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid,  JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();
#endif


	/* We can't really get a property of a SFNode. There are no sub-indexes, etc...
	   so we don't do anything. Check out SFVec3fGetProperty to see how it handles
	   properties, should we need to have properties in the future. */

	SFNodeNative *ptr;
	JSString *_idStr;
	char *_id_c;
	jsval rval;
#if JS_VERSION >= 185
	jsval id;
	if (!JS_IdToValue(cx,iid,&id)) {
		printf("JS_IdToValue failed in SFNodeGetProperty.\n");
		return JS_FALSE;
	}
#endif
	if (JSVAL_IS_INT(id)) {
		printf("SFNode has no [index] property.\n");
		/* would be nice to say node type or node name */
		/* note the setter does seem to take [0 or 1]  */
		return JS_FALSE;
	}

	_idStr = JS_ValueToString(cx, id);
#if JS_VERSION < 185
	_id_c = JS_GetStringBytes(_idStr);
#else
	_id_c = JS_EncodeString(cx,_idStr);
#endif
	#ifdef JSVRMLCLASSESVERBOSE
	printf ("start of SFNodeGetProperty... id is %s\n",_id_c);
	#endif

	/* is this the string "undefined" ? */
	if (strcmp ("undefined",_id_c) == 0) return JS_TRUE;

	/* is this one of the SFNode standard functions? see JSFunctionSpec (SFNodeFunctions)[] */
	if (strcmp ("toString",_id_c) == 0) return JS_TRUE;
	if (strcmp ("valueOf",_id_c) == 0) return JS_TRUE;
	if (strcmp ("equals",_id_c) == 0) return JS_TRUE;
	if (strcmp ("assign",_id_c) == 0) return JS_TRUE;

	/* get the private pointer for this node */
	if ((ptr = (SFNodeNative *)JS_GetPrivateFw(cx, obj)) != NULL) {
		int ifound, type, kind, iifield, *valueChanged, sfsize, sftype;
		union anyVrml *value;
		char *fieldname = _id_c;
		struct X3D_Node *node;
		if(SM_method() == 2){
			AnyNative *any = (AnyNative *)ptr;
			node = any->v->sfnode;
		}else{
			node = ptr->handle;
		}
		#ifdef JSVRMLCLASSESVERBOSE
		printf ("SFNodeGetProperty, working on node %p, field %s\n",ptr->handle,_id_c);
		#endif
		if(SM_method() == 2){
			ifound = getFieldFromNodeAndName(node,fieldname,&type,&kind,&iifield,&value);
			if(ifound){
				valueChanged = &node->_change; //if a regular node field changes, we (re-) compile_Node 
				// ... (but have no way to detect which field, so routes can't be done later in freewrl system :{
				if(node->_nodeType == NODE_Script){
					//need one more thing - valueChanged
					struct X3D_Script *scriptnode = X3D_SCRIPT(node);
					getFieldFromScript((struct Shader_Script*)scriptnode->__scriptObj,fieldname,&type,&kind,&iifield,&value,&valueChanged);
				}
				sftype = type2SF(type);
				sfsize = sizeofSForMF(sftype);

				//set up a return value
				switch (type) {
				case FIELDTYPE_SFBool:
				case FIELDTYPE_SFFloat:
				case FIELDTYPE_SFTime:
				case FIELDTYPE_SFDouble:
				case FIELDTYPE_SFInt32:
				case FIELDTYPE_SFString:
					X3D_ECMA_TO_JS(cx, value,sfsize,type,vp);
					break;
				case FIELDTYPE_SFColor:
				case FIELDTYPE_SFColorRGBA:
				case FIELDTYPE_SFNode:
				case FIELDTYPE_SFVec2f:
				case FIELDTYPE_SFVec3f:
				case FIELDTYPE_SFVec4f:
				case FIELDTYPE_SFVec2d:
				case FIELDTYPE_SFVec3d:
				case FIELDTYPE_SFVec4d:
				case FIELDTYPE_SFRotation:
					X3D_SF_TO_JS_B(cx, value,sfsize, type, valueChanged, vp);
					break;
				case FIELDTYPE_MFInt32:
				case FIELDTYPE_MFBool:
				case FIELDTYPE_MFFloat:
				case FIELDTYPE_MFDouble:
				case FIELDTYPE_MFTime:
				case FIELDTYPE_MFString:
				case FIELDTYPE_MFNode:
				case FIELDTYPE_MFColor:
				case FIELDTYPE_MFColorRGBA:
				case FIELDTYPE_MFVec2f:
				case FIELDTYPE_MFVec3f:
				case FIELDTYPE_MFVec4f:
				case FIELDTYPE_MFVec2d:
				case FIELDTYPE_MFVec3d:
				case FIELDTYPE_MFVec4d:
				case FIELDTYPE_MFRotation:
				case FIELDTYPE_SFImage:
				//static void X3D_MF_TO_JS(JSContext *cx, JSObject *obj, void *Data, int dataType, jsval *newval, char *fieldName) {
					X3D_MF_TO_JS_B(cx, value, type, valueChanged, vp);
					break;
				default: 
					printf ("unhandled type FIELDTYPE_ %d in getSFNodeProperty!\n", type) ;
				return JS_FALSE;
				}
	

				//#if JS_VERSION < 185
				//	*rval = OBJECT_TO_JSVAL(obj);
				//#else
				//	JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(obj));
				//#endif

			}
		} //SM_method > 0
		if(SM_method() == 0){
			// dug9 attempt to find read the field of another script 
			//if(!strcmp(stringNodeType(ptr->handle->_nodeType),"Script"))
			if( ptr->handle && ptr->handle->_nodeType== NODE_Script )
			{
				struct Shader_Script *myObj;
				JSContext *cx2;
				JSObject *obj2;
				struct CRscriptStruct *ScriptControl; // = getScriptControl(); 
				myObj = (struct Shader_Script*)X3D_SCRIPT(ptr->handle)->__scriptObj;
				// get context and global object for this script 
				ScriptControl = getScriptControlIndex(myObj->num);
				cx2 =  (JSContext*)ScriptControl->cx;
				obj2 = (JSObject*)ScriptControl->glob;
				if (JS_GetProperty (cx2, obj2, _id_c, &rval)) {
					if (JSVAL_IS_NULL(rval)) {
						ConsoleMessage ("Script - field :%s: does not exist",_id_c);
						return JS_FALSE;
					}else{
						*vp = rval;
						return JS_TRUE;
					}
				}
			}

			JS_DefineSFNodeSpecificProperties (cx, obj, ptr->handle);

			// does the property exist? 
			if (JS_LookupProperty (cx, obj, _id_c, &rval)) {
				if (JSVAL_IS_NULL(rval)) {
					// if you mis-spell a builtin node field 
					// like Cylinder.hight (sb height) you'll end up in here 
					ConsoleMessage ("SFNode - field :%s: does not exist",_id_c);
					return JS_FALSE;
				}
			}
			// if your SFNode is type Script you'll end up here 
			#ifdef JSVRMLCLASSESVERBOSE
			printf ("wondering about rval.. %d. it is a\n",(int)rval);
			if (JSVAL_IS_INT(rval)) printf ("IS AN INT\n");
			if (JSVAL_IS_OBJECT(rval)) printf ("IS AN OBJECT\n");
			if (JSVAL_IS_STRING(rval)) printf ("IS AN STRING\n");
			if (rval == JSVAL_FALSE) printf ("FALSE\n");
			if (rval == JSVAL_NULL) printf ("NULL\n");
			if (rval == JSVAL_ONE) printf ("ONE\n");
			if (rval == JSVAL_ZERO) printf ("ZERO\n");
			if (rval == JSVAL_VOID) printf ("VOID\n");
			if (rval == JSVAL_TRUE) printf ("TRUE\n");
			#endif


			//dug9 - I find the next line JS_GetProperty recursive
			//when the sfnode we're trying to read is a Script node
			//if (JS_GetProperty (cx, obj, _id_c, &rval)) {
			if(false){
				#ifdef JSVRMLCLASSESVERBOSE
					printf ("SFNodeGetProperty, found field \"%s\" in node, returning property\n",_id_c);
				#endif

				*vp = rval;
			} else {
				#ifdef JSVRMLCLASSESVERBOSE
				printf ("SFNodeGetProperty, did not find field \"%s\" in node.\n",_id_c);
				#endif
				return JS_FALSE;
			}
		} //SM_method() == 0
	} else {
		printf ("could not get private for SFNodeGetProperty, field :%s:\n",_id_c);
		return JS_FALSE;
	}

	return JS_TRUE;
}

void Parser_scanStringValueToMem_B(union anyVrml* any, indexT ctype, const char *value, int isXML);
void JS_MF_TO_X3D(JSContext *cx, JSObject * obj, void *Data, int dataType, jsval *newval);
JSBool
#if JS_VERSION < 185
SFNodeSetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp){
#elif JS_VERSION == 185
SFNodeSetProperty(JSContext *cx, JSObject *obj, jsid iid, JSBool strict, jsval *vp){
#else
SFNodeSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();
#endif
	JSString *_idStr, *_valStr;
	char *_id_c, *_val_c;
	SFNodeNative *ptr;
	size_t val_len;
	size_t tmp;
#if JS_VERSION >= 185
	jsval id;
	if (!JS_IdToValue(cx,iid,&id)) {
		printf("JS_IdToValue failed in SFNodeSetProperty.\n");
		return JS_FALSE;
	}
#endif
	if(SM_method() == 2){
		AnyNative *lhs;
		int ifound, type, kind, iifield, *valueChanged, sfsize, sftype;
		union anyVrml *value;
		char *fieldname;
		struct X3D_Node *node;

		if((lhs = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			return JS_FALSE;
		}
		_idStr = JS_ValueToString(cx, id);
#if JS_VERSION < 185
		_id_c = JS_GetStringBytes(_idStr);
#else
		_id_c = JS_EncodeString(cx,_idStr); /* _id_c field name as a string ie "currX" */
#endif
		fieldname = _id_c;
		node = lhs->v->sfnode;
		ifound = getFieldFromNodeAndName(node,fieldname,&type,&kind,&iifield,&value);
		if(ifound){
			valueChanged = &node->_change;
			if(node->_nodeType == NODE_Script){
				//need one more thing - valueChanged
				struct X3D_Script *scriptnode = X3D_SCRIPT(node);
				getFieldFromScript((struct Shader_Script*)scriptnode->__scriptObj,fieldname,&type,&kind,&iifield,&value,&valueChanged);
			}
			sftype = type2SF(type);
			sfsize = sizeofSForMF(sftype);
			//set up a return value
			//in js, null is an object
			if( JSVAL_IS_NULL(*vp)){
				if(type == FIELDTYPE_SFNode)
					value->sfnode = NULL;
				if(valueChanged)
					(*valueChanged) ++;
			//} else if (JSVAL_IS_OBJECT(*vp)) {
			} else if ((*vp).isObject()) {
				AnyNative *rhs;
        		if ((rhs = (AnyNative *)JS_GetPrivateFw(cx, JSVAL_TO_OBJECT(*vp))) == NULL) {
					//printf("in setECMANative, RHS was NOT native type \n");
        		}else{
					//printf("in setECMANative, RHS was native type \n");
					//can do an assign here
					if(type == rhs->type){
						if(valueChanged)
							(*valueChanged) ++;
						//shallow assumes the top has already been malloced (just base part of MF needed)
						//use this if you need to malloc anyvrml: int sizeofSForMF(int itype)
						shallow_copy_field(rhs->type,rhs->v,value);
					}
				}

			} else {

				switch (type) {
				case FIELDTYPE_SFBool:
				case FIELDTYPE_SFFloat:
				case FIELDTYPE_SFTime:
				case FIELDTYPE_SFDouble:
				case FIELDTYPE_SFInt32:
				case FIELDTYPE_SFString:
					JS_ECMA_TO_X3D(cx, value,sfsize,type,vp);

					if(valueChanged)
						(*valueChanged)++;

					break;
				//case FIELDTYPE_SFColor:
				//case FIELDTYPE_SFNode:
				//case FIELDTYPE_SFVec2f:
				//case FIELDTYPE_SFVec3f:
				//case FIELDTYPE_SFVec3d:
				//case FIELDTYPE_SFRotation:
				//	JS_SF_TO_X3D(cx, obj, value,sfsize, type, vp);
				//	break;
				//case FIELDTYPE_MFColor:
				//case FIELDTYPE_MFVec3f:
				//case FIELDTYPE_MFVec2f:
				//case FIELDTYPE_MFFloat:
				//case FIELDTYPE_MFTime:
				//case FIELDTYPE_MFInt32:
				//case FIELDTYPE_MFString:
				//case FIELDTYPE_MFNode:
				//case FIELDTYPE_MFRotation:
				//case FIELDTYPE_SFImage:
				//	JS_MF_TO_X3D(cx, obj, value, type, vp);
				//	break;
				default: 
					printf ("unhandled type FIELDTYPE_ %d in setSFNodeProperty?\n", type) ;
				return JS_FALSE;
				}
			}

			//#if JS_VERSION < 185
			//	*rval = OBJECT_TO_JSVAL(obj);
			//#else
			//	JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(obj));
			//#endif
			return JS_TRUE;

		}
		return JS_FALSE;

	}else{ //SM_method == 2
		//if (JSVAL_IS_INT(id)) {
		//	printf("SFNode has no [index] property.\n");
		//	/* would be nice to say node type or node name */
		//	return JS_FALSE;
		//}

		_idStr = JS_ValueToString(cx, id);
		_valStr = JS_ValueToString(cx, *vp);
#if JS_VERSION < 185
		_id_c = JS_GetStringBytes(_idStr);
		_val_c = JS_GetStringBytes(_valStr);
#else
		_id_c = JS_EncodeString(cx,_idStr); /* _id_c field name as a string ie "currX" */
		_val_c = JS_EncodeString(cx,_valStr); /* _val_c field value as a string ie "33" */
#endif


		#ifdef JSVRMLCLASSESVERBOSE
			printf("SFNodeSetProperty: obj = %p, id = %s, vp = %s\n",
				   obj, _id_c, _val_c);
		#endif


		if ((ptr = (SFNodeNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFNodeSetProperty.\n");
			return JS_FALSE;
		}

		if (JSVAL_IS_INT(id)) {
			/* dug9 Aug 2013: no Properties are defined on SFNode, so there shouldn't be 
			   any public SFNode[id] = ?. What are we doing here? 
			   Was it an experiment or test method? Or does some other internal 
			   function call this setter[id]? May we drop support for it?*/
			if(SM_method() == 0){
				ptr->valueChanged++;
				val_len = (int) strlen(_val_c) + 1;

				#ifdef JSVRMLCLASSESVERBOSE
				printf ("switching on %d\n",JSVAL_TO_INT(id));
				#endif

				switch (JSVAL_TO_INT(id)) {
				case 0:
					if ((strlen(ptr->X3DString) + 1) > val_len) {
						ptr->X3DString =
							(char *) REALLOC (ptr->X3DString, val_len * sizeof(char));
					}
					memset(ptr->X3DString, 0, val_len);
					memmove(ptr->X3DString, _val_c, val_len);
					break;
				case 1:
					scanUnsignedIntoValue(_val_c,&tmp);
					ptr->handle = X3D_NODE(tmp);
					break;
				}
			} //SM_method == 0
		} else {
			#ifdef JSVRMLCLASSESVERBOSE
			printf ("JS_IS_INT false\n");

			printf ("SFNodeSetProperty, setting node %p field %s to value %s\n", ptr->handle,_id_c,_val_c);

			{
				struct X3D_Node* ptx;
				ptx = X3D_NODE(ptr->handle);
				printf ("node is of type %s\n",stringNodeType(ptx->_nodeType));
			}
			#endif

			/* dug9 2012 attempt to find and write the field of another script */
			if( ptr->handle->_nodeType== NODE_Script )
			{
				/* code borrowed from fieldGet.c L.138 in set_one_ECMAtype() and reworked
				   for cx2, obj2 - writes to a script eventIn with timestamp, 
				   and runs the script function, completing the event
				   cascade (I think)
				*/
				char scriptline[100];
				JSScript *eventInFunction;
				struct ScriptFieldDecl* myfield; 
				struct CRjsnameStruct *JSparamnames; // = getJSparamnames();
				struct Shader_Script *myObj;
				JSContext *cx2;
				JSObject *obj2;
				jsval newval;
				//indexT myfieldType;
				//union anyVrml vrmlField;
				//bool deepcopy;
				struct CRscriptStruct *ScriptControl; // = getScriptControl(); 
				myObj = (struct Shader_Script*)X3D_SCRIPT(ptr->handle)->__scriptObj;
				/* is the script ok and initialized? */
				ScriptControl = getScriptControlIndex(myObj->num);
				if ((!ScriptControl->_initialized) || (!ScriptControl->scriptOK)) {
					/* printf ("waiting for initializing script %d at %s:%d\n",(uintptr_t)to_ptr->routeToNode, __FILE__,__LINE__); */
					return JS_FALSE;;
				}

				/* get context and global object for this script */
				cx2 =  (JSContext*)ScriptControl->cx;
				obj2 = (JSObject*)ScriptControl->glob;
				//it doesn't seem to matter which cx/obj we use.
				cx2 = cx;
				obj2 = obj;

				{ // Scope A  for our various stack objects (JSAutoRequest, RootedObject), so they all go
					// out of scope before we JS_DestroyContext.
					JSAutoRequest ar(cx); // In practice, you would want to exit this any
										// time you're spinning the event loop
					{ // Scope B  for JSAutoCompartment
						JSAutoCompartment ac(cx, obj2);
							/* set the time for this script */
							//SET_JS_TICKTIME()
							{ 
								jsval zimbo;
								JS_NewNumberValue(cx2, TickTime(), &zimbo);
								if (!JS_DefineProperty(cx2,obj2, "__eventInTickTime", zimbo, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB2, JSPROP_PERMANENT)) {
									printf( "JS_DefineProperty failed for __eventInTickTime at %s:%d.\n",__FILE__,__LINE__);
									return JS_FALSE;
								}
							}
							//X3D_ECMA_TO_JS(cx, Data, datalen, dataType, &newval);
							//if( getSFNodeField(cx,obj, id, &newval) == JS_FALSE) //this is for getting fields from builtin node types, not Script nodes
							//	return JS_FALSE;
							myfield = script_getField_viaCharName(myObj, _id_c);

							//Q. do I need to deepcopy the vp?
							// newval = deepcopy(vp); 
							//a slight difference: pointer copy: bool=true, deep copy: bool=1
							//I'll stick with pointer copy
							/*
							deepcopy = false; 
							if(deepcopy)
							{
								//step 1. get the target/output field's datatype
								myfieldType = myfield->fieldDecl->fieldType;
				
								//step 2. try and read the input *vp using that datatype
								// borrowed from jsUtils.c L.1247
								switch (myfieldType) {
									case FIELDTYPE_SFBool:
									case FIELDTYPE_SFFloat:
									case FIELDTYPE_SFTime:
									case FIELDTYPE_SFDouble:
									case FIELDTYPE_SFInt32:
									case FIELDTYPE_SFString:
									JS_ECMA_TO_X3D(cx2, &vrmlField,	returnElementLength(myfieldType), myfieldType, vp);
									break;
									case FIELDTYPE_SFColor:
									case FIELDTYPE_SFNode:
									case FIELDTYPE_SFVec2f:
									case FIELDTYPE_SFVec3f:
									case FIELDTYPE_SFVec3d:
									case FIELDTYPE_SFRotation:
									JS_SF_TO_X3D(cx2,&vrmlField,returnElementLength(myfieldType) * returnElementRowSize(myfieldType) , myfieldType, vp);
									break;
									case FIELDTYPE_MFColor:
									case FIELDTYPE_MFVec3f:
									case FIELDTYPE_MFVec2f:
									case FIELDTYPE_MFFloat:
									case FIELDTYPE_MFTime:
									case FIELDTYPE_MFInt32:
									case FIELDTYPE_MFString:
									case FIELDTYPE_MFNode:
									case FIELDTYPE_MFRotation:
									case FIELDTYPE_SFImage:
									JS_MF_TO_X3D(cx2, obj2, &vrmlField, myfieldType, vp);
									break;
									default: printf ("unhandled type in setSFNodeField\n");
									return JS_FALSE;
								}
								//step 3. if successful, convert back to a second copy newval
								//int setField_FromEAI_ToScript(int tonode, int toname, int datatype, void *data, unsigned rowcount) {
								switch (myfieldType) {
									case FIELDTYPE_SFBool:
									case FIELDTYPE_SFFloat:
									case FIELDTYPE_SFTime:
									case FIELDTYPE_SFDouble:
									case FIELDTYPE_SFInt32:
									case FIELDTYPE_SFString:
									X3D_ECMA_TO_JS(cx2, &vrmlField,	returnElementLength(myfieldType), myfieldType, &newval);
									break;
									case FIELDTYPE_SFColor:
									case FIELDTYPE_SFNode:
									case FIELDTYPE_SFVec2f:
									case FIELDTYPE_SFVec3f:
									case FIELDTYPE_SFVec3d:
									case FIELDTYPE_SFRotation:
									X3D_SF_TO_JS(cx2, obj2, &vrmlField,	returnElementLength(myfieldType) * returnElementRowSize(myfieldType) , myfieldType, &newval);
									break;
									case FIELDTYPE_MFColor:
									case FIELDTYPE_MFVec3f:
									case FIELDTYPE_MFVec2f:
									case FIELDTYPE_MFFloat:
									case FIELDTYPE_MFTime:
									case FIELDTYPE_MFInt32:
									case FIELDTYPE_MFString:
									case FIELDTYPE_MFNode:
									case FIELDTYPE_MFRotation:
									case FIELDTYPE_SFImage:
									X3D_MF_TO_JS(cx2, obj2, &vrmlField, myfieldType, &newval, _id_c);
									break;
									default: printf ("unhandled type FIELDTYPE_ %d in getSFNodeField\n", myfieldType) ;
									return JS_FALSE;
								}
							}else{ //deepcopy
							*/
								newval = *vp;
							//}
							/* get the variable name to hold the incoming value */
							//sprintf (scriptline,"__eventIn_Value_%s",  _id_c);
							strcpy(scriptline,_id_c);
							#ifdef JSVRMLCLASSESVERBOSE
							printf ("set_one_ECMAtype, calling JS_DefineProperty on name %s obj %u, setting setECMANative, 0 \n",scriptline,obj2);
							#endif

							if (!JS_DefineProperty(cx2,obj2, scriptline, newval, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB3, JSPROP_PERMANENT)) {  
								printf( "JS_DefineProperty failed for SFNodeSetProperty at %s:%d.\n",__FILE__,__LINE__); 
								#if defined(JS_THREADSAFE)
								JS_EndRequest(cx);
								#endif
								return JS_FALSE; 
							}
							/* is the function compiled yet? */
							//COMPILE_FUNCTION_IF_NEEDED(toname)
							JSparamnames = getJSparamnames();
							eventInFunction = (JSScript*) JSparamnames[myfield->fieldDecl->JSparamNameIndex].eventInFunction;
							if ( eventInFunction == NULL) { 
								//sprintf (scriptline,"%s(__eventIn_Value_%s,__eventInTickTime)", _id_c, _id_c); 
								sprintf (scriptline,"set_%s(%s,__eventInTickTime)", _id_c, _id_c); 
								/* printf ("compiling function %s\n",scriptline); */
								eventInFunction = JS_CompileScript(cx2, obj2, scriptline, strlen(scriptline), "compile eventIn",1);
								if(true){
									//if (!JS_AddObjectRoot(cx2,&eventInFunction)) {
									JSparamnames[myfield->fieldDecl->JSparamNameIndex].eventInFunction = eventInFunction;
									#if JS_VERSION >= 185
									if (!JS_AddObjectRoot(cx,(JSObject**)(&JSparamnames[myfield->fieldDecl->JSparamNameIndex].eventInFunction))) {
										printf( "JS_AddObjectRoot failed for compilation of script \"%s\" at %s:%d.\n",scriptline,__FILE__,__LINE__);
										return JS_FALSE;
									}
									#endif
								}
							}
							/* and run the function */
							//RUN_FUNCTION (toname)
							{
								jsval zimbo;
								if (!JS_ExecuteScript(cx2, obj2, eventInFunction, &zimbo)) 
								{
									printf ("failed to set parameter for eventIn %s in FreeWRL code %s:%d\n",_id_c,__FILE__,__LINE__); \
									/* printf ("myThread is %u\n",pthread_self());*/ \
									return JS_FALSE;
								}
								return JS_TRUE;
							}
					} //Scope B
				} //Scope A
			}
			//dug9 July 9, 2014 - failed attempt to fix, see runQueuedDirectOutputs()
			// problem: 1) it's a lot of work to drill into the JS object for the other script to set the field value and valueChanged flag
			// 2) taking a shortcut in the routing loop would require refactoring: JSGlobal_object between get_valueChanged and js_setField_javascriptEventOut
			// would need to be generalized to do anyVrml or anyVrml passed directly up and down.
			if(0) if( ptr->handle->_nodeType== NODE_Script )
			{
				int itype, kind;
				//step 1. unconditionally write the script->field->value regardless of its kind/PKW
				struct ScriptFieldDecl* myfield; 
				struct Shader_Script *script;
				struct CRscriptStruct *ScriptControl; // = getScriptControl(); 
				script = (struct Shader_Script*)X3D_SCRIPT(ptr->handle)->__scriptObj;
				/* is the script ok and initialized? */
				ScriptControl = getScriptControlIndex(script->num);
				if ((!ScriptControl->_initialized) || (!ScriptControl->scriptOK)) {
					/* printf ("waiting for initializing script %d at %s:%d\n",(uintptr_t)to_ptr->routeToNode, __FILE__,__LINE__); */
					return JS_FALSE;;
				}
				myfield = script_getField_viaCharName(script, _id_c);
				if(!myfield) return JS_FALSE;
				itype = ScriptFieldDecl_getType(myfield);
				kind =  ScriptFieldDecl_getMode(myfield);
				Parser_scanStringValueToMem_B(&myfield->value, itype, _val_c, FALSE);
				if(kind == PKW_inputOnly || kind == PKW_inputOutput)
					myfield->eventInSet = TRUE; //flag for runQueuedDirectOutputs() to run eventIn function on other script, feeding it the value we just set
				if(kind == PKW_inputOutput || kind == PKW_outputOnly)
					myfield->valueChanged = TRUE; //flag for eventOuts on other script to send what we are writing
				return JS_TRUE;
			}

			setField_fromJavascript (X3D_NODE(ptr->handle), _id_c, _val_c, FALSE);
		}
	} //SM_method == 2
	return JS_TRUE;
}


/********************************************************************/

JSBool
SFRotationGetAxis(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);

	JSObject *_retObj;
	float *cc, *cclhs;

	UNUSED(argc);
	UNUSED(argv);
	#ifdef JSVRMLCLASSESVERBOSE
	printf ("start of SFRotationGetAxis\n");
	#endif

	if ((_retObj = JS_ConstructObjectFw(cx, &SFVec3fClass, NULL, NULL)) == NULL) {
		printf( "JS_ConstructObject failed in SFRotationGetAxis.\n");
		return JS_FALSE;
	}

	JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(_retObj));

	if(SM_method()==2){
		AnyNative *_rot;
		AnyNative *_retNative;
		if ((_rot = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed for obj in SFRotationGetAxis.\n");
			return JS_FALSE;
		}

		if ((_retNative = (AnyNative *)JS_GetPrivateFw(cx, _retObj)) == NULL) {
			printf( "JS_GetPrivate failed for _retObj in SFRotationGetAxis.\n");
			return JS_FALSE;
		}
		cc = _rot->v->sfrotation.c;
		cclhs = _retNative->v->sfvec3f.c;
	}else{
		SFRotationNative *_rot;
		SFVec3fNative *_retNative;
		if ((_rot = (SFRotationNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed for obj in SFRotationGetAxis.\n");
			return JS_FALSE;
		}

		if ((_retNative = (SFVec3fNative *)JS_GetPrivateFw(cx, _retObj)) == NULL) {
			printf( "JS_GetPrivate failed for _retObj in SFRotationGetAxis.\n");
			return JS_FALSE;
		}
		cc = _rot->v.c;
		cclhs = _retNative->v.c;

	}
	veccopy3f(cclhs,cc);

	#ifdef JSVRMLCLASSESVERBOSE
		printf("SFRotationGetAxis: obj = %p, result = [%.9g, %.9g, %.9g]\n",
			   obj,
			   (_retNative->v).c[0],
			   (_retNative->v).c[1],
			   (_retNative->v).c[2]);
	#endif

	return JS_TRUE;
}

JSBool
SFRotationInverse(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);

	JSObject *_retObj, *_proto;
	Quaternion q1,qret;
	double a,b,c,d;
	float *cc, *cclhs;
	UNUSED(argc);
	UNUSED(argv);
	#ifdef JSVRMLCLASSESVERBOSE
	printf ("start of SFRotationInverse\n");
	#endif

	if ((_proto = JS_GetPrototypeFw(cx, obj)) == NULL) {
		printf( "JS_GetPrototype failed in SFRotationInverse.\n");
		return JS_FALSE;
	}
	if ((_retObj = JS_ConstructObjectFw(cx, &SFRotationClass, _proto, NULL)) == NULL) {
		printf( "JS_ConstructObject failed in SFRotationInverse.\n");
		return JS_FALSE;
	}
	JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(_retObj));


	if(SM_method()==2){
		AnyNative *_rot, *_retNative;

		if ((_rot = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed for obj in SFRotationInverse.\n");
			return JS_FALSE;
		}

		if ((_retNative = (AnyNative *)JS_GetPrivateFw(cx, _retObj)) == NULL) {
			printf( "JS_GetPrivate failed for _retObj in SFRotationInverse.\n");
			return JS_FALSE;
		}
		cc = _rot->v->sfrotation.c;
		cclhs = _retNative->v->sfrotation.c;

	}else{
		SFRotationNative *_rot, *_retNative;

		if ((_rot = (SFRotationNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed for obj in SFRotationInverse.\n");
			return JS_FALSE;
		}

		if ((_retNative = (SFRotationNative *)JS_GetPrivateFw(cx, _retObj)) == NULL) {
			printf( "JS_GetPrivate failed for _retObj in SFRotationInverse.\n");
			return JS_FALSE;
		}
		_retNative->valueChanged = 1; 
		cc = _rot->v.c;
		cclhs = _retNative->v.c;
	}
	/* convert both rotation to quaternion */
	vrmlrot_to_quaternion(&q1, (double) cc[0], 
		(double) cc[1], (double)cc[2], (double) cc[3]);

	/* invert it */
	quaternion_inverse(&qret,&q1);


	/* and return the resultant, as a vrml rotation */
	quaternion_to_vrmlrot(&qret, &a, &b, &c, &d);
	/* double to floats, can not use pointers... */
	cclhs[0] = (float) a;
	cclhs[1] = (float) b;
	cclhs[2] = (float) c;
	cclhs[3] = (float) d;

	/* and, we now have a new value */

	return JS_TRUE;
}

JSBool
SFRotationMultiply(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);

	Quaternion q1,q2,qret;
	double a,b,c,d;

	JSObject *_multObj, *_proto, *_retObj;
	float *cc, *cc1, *cclhs;
	#ifdef JSVRMLCLASSESVERBOSE
	printf ("start of SFRotationMultiply\n");
	#endif

	if (!JS_ConvertArguments(cx, argc, argv, "o", &_multObj)) {
		printf( "JS_ConvertArguments failed in SFRotationMultiply.\n");
		return JS_FALSE;
	}
	CHECK_CLASS(cx,_multObj,argv,__FUNCTION__,SFRotationClass)

	if ((_proto = JS_GetPrototypeFw(cx, _multObj)) == NULL) {
		printf( "JS_GetPrototype failed in SFRotationMultiply.\n");
		return JS_FALSE;
	}

	if ((_retObj = JS_ConstructObjectFw(cx, &SFRotationClass, _proto, NULL)) == NULL) {
		printf( "JS_ConstructObject failed in SFRotationMultiply.\n");
		return JS_FALSE;
	}


	JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(_retObj));

	if(SM_method()==2){
		AnyNative *_rot1, *_rot2, *_retNative;

		if ((_rot1 = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed for obj in SFRotationMultiply.\n");
			return JS_FALSE;
		}

		if ((_rot2 = (AnyNative *)JS_GetPrivateFw(cx, _multObj)) == NULL) {
			printf( "JS_GetPrivate failed for _multObj in SFRotationMultiply.\n");
			return JS_FALSE;
		}

		if ((_retNative = (AnyNative *)JS_GetPrivateFw(cx, _retObj)) == NULL) {
			printf( "JS_GetPrivate failed for _retObj in SFRotationMultiply.\n");
			return JS_FALSE;
		}
		cc = _rot1->v->sfrotation.c;
		cc1 = _rot2->v->sfrotation.c;
		cclhs = _retNative->v->sfrotation.c;
	}else{
		SFRotationNative *_rot1, *_rot2, *_retNative;

		if ((_rot1 = (SFRotationNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed for obj in SFRotationMultiply.\n");
			return JS_FALSE;
		}

		if ((_rot2 = (SFRotationNative *)JS_GetPrivateFw(cx, _multObj)) == NULL) {
			printf( "JS_GetPrivate failed for _multObj in SFRotationMultiply.\n");
			return JS_FALSE;
		}

		if ((_retNative = (SFRotationNative *)JS_GetPrivateFw(cx, _retObj)) == NULL) {
			printf( "JS_GetPrivate failed for _retObj in SFRotationMultiply.\n");
			return JS_FALSE;
		}
		cc = _rot1->v.c;
		cc1 = _rot2->v.c;
		cclhs = _retNative->v.c;
		_retNative->valueChanged = 1; 

	}
	/* convert both rotations into quaternions */
	vrmlrot_to_quaternion(&q1, (double) cc[0], 
		(double) cc[1], (double) cc[2], (double) cc[3]);
	vrmlrot_to_quaternion(&q2, (double) cc1[0], 
		(double) cc1[1], (double) cc1[2], (double) cc1[3]);

	/* multiply them */
	quaternion_multiply(&qret,&q1,&q2);


	/* and return the resultant, as a vrml rotation */
	quaternion_to_vrmlrot(&qret, &a, &b, &c, &d);
	/* double to floats, can not use pointers... */
	cclhs[0] = (float) a;
	cclhs[1] = (float) b;
	cclhs[2] = (float) c;
	cclhs[3] = (float) d;

	/* and, we now have a new value */

	return JS_TRUE;
}

JSBool
SFRotationMultVec(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);

	JSObject *_multObj, *_retObj, *_proto;
	float rl;
	//float vl;
	//float rlpt;
	float s, c, angle;
	//struct point_XYZ r, v, c1, c2;
	int i;
	float *rot, *vec, *cclhs;
	float c1[3], c2[3], r[3]; //temps


	#ifdef JSVRMLCLASSESVERBOSE
	printf ("start of SFRotationMultiVec\n");
	#endif

	if (!JS_ConvertArguments(cx, argc, argv, "o", &_multObj)) {
		printf( "JS_ConvertArguments failed in SFRotationMultVec.\n");
		return JS_FALSE;
	}

	CHECK_CLASS(cx,_multObj,argv,__FUNCTION__,SFVec3fClass)

	if ((_proto = JS_GetPrototypeFw(cx, _multObj)) == NULL) {
		printf( "JS_GetPrototype failed in SFRotationMultVec.\n");
		return JS_FALSE;
	}

	if ((_retObj = JS_ConstructObjectFw(cx, &SFVec3fClass, _proto, NULL)) == NULL) {
		printf( "JS_ConstructObject failed in SFRotationMultVec.\n");
		return JS_FALSE;
	}

	JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(_retObj));

	if(SM_method()==2){
		AnyNative *_rot;
		AnyNative *_vec, *_retNative;

		if ((_rot = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed for obj in SFRotationMultVec.\n");
			return JS_FALSE;
		}

		if ((_vec = (AnyNative *)JS_GetPrivateFw(cx, _multObj)) == NULL) {
			printf( "JS_GetPrivate failed for_multObjin SFRotationMultVec.\n");
			return JS_FALSE;
		}
		if ((_retNative = (AnyNative *)JS_GetPrivateFw(cx, _retObj)) == NULL) {
			printf( "JS_GetPrivate failed for _retObj in SFRotationMultVec.\n");
			return JS_FALSE;
		}
		rot = _rot->v->sfrotation.c;
		vec = _vec->v->sfvec3f.c;
		cclhs = _retNative->v->sfvec3f.c;
		
	}else{
		SFRotationNative *_rot;
		SFVec3fNative *_vec, *_retNative;

		if ((_rot = (SFRotationNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed for obj in SFRotationMultVec.\n");
			return JS_FALSE;
		}
		//COPY_SFVEC3F_TO_POINT_XYZ(r,_rot->v.c);
		//angle = _rot->v.c[3];

		if ((_vec = (SFVec3fNative *)JS_GetPrivateFw(cx, _multObj)) == NULL) {
			printf( "JS_GetPrivate failed for_multObjin SFRotationMultVec.\n");
			return JS_FALSE;
		}
		//COPY_SFVEC3F_TO_POINT_XYZ(v,_vec->v.c);
		if ((_retNative = (SFVec3fNative *)JS_GetPrivateFw(cx, _retObj)) == NULL) {
			printf( "JS_GetPrivate failed for _retObj in SFRotationMultVec.\n");
			return JS_FALSE;
		}
		rot = _rot->v.c;
		vec = _vec->v.c;
		cclhs = _retNative->v.c;
	}
	rl = veclength3f(rot);
	angle = rot[3];
	s = (float) sin(angle);
	c = (float) cos(angle);
	veccross3f(c1,rot,vec);
	rl = veclength3f(c1);
	vecscale3f(c1,c1,1.0f/rl);
	veccross3f(c2,rot,c1);
	rl = veclength3f(c2);
	vecscale3f(c2,c2,1.0f/rl);
	for(i=0;i<3;i++)
		cclhs[i] = (float) (vec[i] + s * c1[i] + (1.0-c) * c2[i]);


	return JS_TRUE;
}

JSBool
SFRotationSetAxis(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);

	JSObject *_setAxisObj;
	float *rot, *vec;

	#ifdef JSVRMLCLASSESVERBOSE
	printf ("start of SFRotationSetAxis\n");
	#endif

	if (!JS_ConvertArguments(cx, argc, argv, "o", &_setAxisObj)) {
		printf( "JS_ConvertArguments failed in SFRotationSetAxis.\n");
		return JS_FALSE;
	}

	CHECK_CLASS(cx,_setAxisObj,argv,__FUNCTION__,SFVec3fClass)

	if(SM_method() == 2){
		AnyNative *_rot;
		AnyNative *_vec;
		if ((_rot = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed for obj in SFRotationSetAxis.\n");
			return JS_FALSE;
		}

		if ((_vec = (AnyNative *)JS_GetPrivateFw(cx, _setAxisObj)) == NULL) {
			printf( "JS_GetPrivate failed for _retObj in SFRotationSetAxis.\n");
			return JS_FALSE;
		}
		rot = _rot->v->sfrotation.c;
		vec = _vec->v->sfvec3f.c;
	}else{
		SFRotationNative *_rot;
		SFVec3fNative *_vec;
		if ((_rot = (SFRotationNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed for obj in SFRotationSetAxis.\n");
			return JS_FALSE;
		}

		if ((_vec = (SFVec3fNative *)JS_GetPrivateFw(cx, _setAxisObj)) == NULL) {
			printf( "JS_GetPrivate failed for _retObj in SFRotationSetAxis.\n");
			return JS_FALSE;
		}
		rot = _rot->v.c;
		vec = _vec->v.c;
	}
	veccopy3f(rot,vec);

	JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(obj));


	#ifdef JSVRMLCLASSESVERBOSE
		printf("SFRotationSetAxis: obj = %p, result = [%.9g, %.9g, %.9g, %.9g]\n",
			   obj,
			   rot[0],
			   rot[1],
			   rot[2],
			   rot[3]);
	#endif

	return JS_TRUE;
}

JSBool
SFRotationSlerp(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
	jsval rvalinst;
	jsval *rval = &rvalinst;

	JSObject *_destObj, *_retObj, *_proto;
	Quaternion _quat, _quat_dest, _quat_ret;
	jsdouble t;
	float *rot, *dest, *ret;

	#ifdef JSVRMLCLASSESVERBOSE
	printf ("start of SFRotationSlerp\n");
	#endif
	if (!JS_ConvertArguments(cx, argc, argv, "o d", &_destObj, &t)) {
		printf( "JS_ConvertArguments failed in SFRotationSlerp.\n");
		return JS_FALSE;
	}

	CHECK_CLASS(cx,_destObj,argv,__FUNCTION__,SFRotationClass)


	/*
	 * From Annex C, C.6.7.4:
	 *
	 * For t = 0, return object's rotation.
	 * For t = 1, return 1st argument.
	 * For 0 < t < 1, compute slerp.
	 */
	if (APPROX(t, 0)) {
		*rval = OBJECT_TO_JSVAL(obj);
	} else if (APPROX(t, 1)) {
		*rval = OBJECT_TO_JSVAL(_destObj);
	} else {
		if ((_proto = JS_GetPrototypeFw(cx, _destObj)) == NULL) {
			printf( "JS_GetPrototype failed in SFRotationSlerp.\n");
			return JS_FALSE;
		}

		if ((_retObj = JS_ConstructObjectFw(cx, &SFRotationClass, _proto, NULL)) == NULL) {
			printf( "JS_ConstructObject failed in SFRotationSlerp.\n");
			return JS_FALSE;
		}
		/* root the object */
		*rval = OBJECT_TO_JSVAL(_retObj);

		if(SM_method() == 2){
			AnyNative *_rot, *_dest, *_ret;
			if ((_rot = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
				printf( "JS_GetPrivate failed for obj in SFRotationSlerp.\n");
				return JS_FALSE;
			}

			if ((_dest = (AnyNative *)JS_GetPrivateFw(cx, _destObj)) == NULL) {
				printf( "JS_GetPrivate failed for _destObj in SFRotationSlerp.\n");
				return JS_FALSE;
			}

			if ((_ret = (AnyNative *)JS_GetPrivateFw(cx, _retObj)) == NULL) {
				printf( "JS_GetPrivate failed for _retObj in SFRotationSlerp.\n");
				return JS_FALSE;
			}
			rot = _rot->v->sfrotation.c;
			dest = _dest->v->sfrotation.c;
			ret = _ret->v->sfrotation.c;
		}else{
			SFRotationNative *_rot, *_dest, *_ret;
			if ((_rot = (SFRotationNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
				printf( "JS_GetPrivate failed for obj in SFRotationSlerp.\n");
				return JS_FALSE;
			}

			if ((_dest = (SFRotationNative *)JS_GetPrivateFw(cx, _destObj)) == NULL) {
				printf( "JS_GetPrivate failed for _destObj in SFRotationSlerp.\n");
				return JS_FALSE;
			}

			if ((_ret = (SFRotationNative *)JS_GetPrivateFw(cx, _retObj)) == NULL) {
				printf( "JS_GetPrivate failed for _retObj in SFRotationSlerp.\n");
				return JS_FALSE;
			}
			rot = _rot->v.c;
			dest = _dest->v.c;
			ret = _ret->v.c;
		}
		vrmlrot_to_quaternion(&_quat,
							  rot[0],
							  rot[1],
							  rot[2],
							  rot[3]);

		vrmlrot_to_quaternion(&_quat_dest,
							  dest[0],
							  dest[1],
							  dest[2],
							  dest[3]);

		quaternion_slerp(&_quat_ret, &_quat, &_quat_dest, t);
		quaternion_to_vrmlrot4f(&_quat_ret,ret);
	}


	JS_SET_RVAL(cx,vp,*rval);

	return JS_TRUE;
}


JSBool
SFRotationToString(JSContext *cx, uintN argc, jsval *vp) {
	JSObject *obj = JS_THIS_OBJECT(cx,vp);
	jsval *argv = JS_ARGV(cx,vp);

    JSString *_str;
	char buff[STRING];
	float *cc;

	UNUSED(argc);
	#ifdef JSVRMLCLASSESVERBOSE
	printf ("start of SFRotationToString\n");
	#endif

	ADD_ROOT (cx,ptr)
	ADD_ROOT(cx,_str)
	if(SM_method() == 2){
	    AnyNative *ptr;
		if ((ptr = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFRotationToString.\n");
			return JS_FALSE;
		}
		cc = ptr->v->sfrotation.c;
	}else{
	    SFRotationNative *ptr;
		if ((ptr = (SFRotationNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFRotationToString.\n");
			return JS_FALSE;
		}
		cc = ptr->v.c;
	}
	memset(buff, 0, STRING);
	sprintf(buff, "%.9g %.9g %.9g %.9g",
			cc[0], cc[1], cc[2], cc[3]);
	_str = JS_NewStringCopyZ(cx, buff);

	JS_SET_RVAL(cx,vp,STRING_TO_JSVAL(_str));

	
	REMOVE_ROOT (cx,ptr)
	REMOVE_ROOT (cx,_str)
    return JS_TRUE;
}

JSBool
SFRotationAssign(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
	JSString *_id_jsstr;

    JSObject *_from_obj;
    char *_id_str;

	UNUSED(_id_str); // compiler warning mitigation


	#ifdef JSVRMLCLASSESVERBOSE
	printf ("start of SFRotationAssign\n");
	#endif
	if(SM_method() == 2){
		AnyNative *lhs, *rhs;
		if ((lhs = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed for obj in SFRotationfAssign.\n");
			return JS_FALSE;
		}
		if (!(*vp).isObject())
			return JS_FALSE;
        if ((rhs = (AnyNative *)JS_GetPrivateFw(cx, JSVAL_TO_OBJECT(*vp))) == NULL) {
			printf("JS_ConvertArguments failed in SFRotationAssign. \n");
			return JS_FALSE;
        }
		if(lhs->type != rhs->type) return JS_FALSE;
		AnyNativeAssign(lhs,rhs);
		JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(obj));


	}else{
	    SFRotationNative *fptr, *ptr;

		if ((ptr = (SFRotationNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed for obj in SFRotationAssign.\n");
			return JS_FALSE;
		}

		CHECK_CLASS(cx,obj,argv,__FUNCTION__,SFRotationClass)

		if (JS_ConvertArguments(cx, argc, argv, "oS", &_from_obj, &_id_jsstr) == JS_TRUE) {
			_id_str = JS_EncodeString(cx,_id_jsstr);
		} else {

			printf( "JS_ConvertArguments failed in SFRotationAssign.\n");
			return JS_FALSE;
		}

		/* is this an assignment of NULL? */
		if (_from_obj == NULL) {
			printf ("we have an assignment to null in SFRotationAssign\n");
			JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(NULL)); //JSVAL_VOID);

		} else {


			CHECK_CLASS(cx,_from_obj,argv,__FUNCTION__,SFRotationClass)

			if ((fptr = (SFRotationNative *)JS_GetPrivateFw(cx, _from_obj)) == NULL) {
				printf( "JS_GetPrivate failed for _from_obj in SFRotationAssign.\n");
        		return JS_FALSE;
			}
			#ifdef JSVRMLCLASSESVERBOSE
				printf("SFRotationAssign: obj = %p, id = \"%s\", from = %p\n",
					   obj, _id_str, _from_obj);
			#endif

			SFRotationNativeAssign(ptr, fptr);
			JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(obj));

		}
	}
	#ifdef JSVRMLCLASSESVERBOSE
	printf("SFRotationAssign: returning object as jsval\n");
	#endif
	return JS_TRUE;
}

JSBool
SFRotationConstr(JSContext *cx, uintN argc, jsval *vp) {
	JSObject *obj = JS_NewObject(cx,&SFRotationClass,NULL,NULL);
	jsval *argv = JS_ARGV(cx,vp);


	JSObject *_ob1, *_ob2;
	jsdouble pars[4];
	jsdouble doub;
	float v1len, v2len;
	double v12dp;
	struct point_XYZ v1, v2;
	int v3fv3f;
	float *cc, *vec, *vec2;

	#ifdef JSVRMLCLASSESVERBOSE
	printf ("start of SFRotationConstr\n");
	#endif

	ADD_ROOT(cx,obj)
	if(SM_method() == 2){
		AnyNative *any;
		if((any = (AnyNative*)AnyNativeNew(FIELDTYPE_SFRotation,NULL,NULL)) == NULL){
			printf( "AnyfNativeNew failed in SFRotationConstr.\n");
			return JS_FALSE;
		}
		if (!JS_SetPrivateFw(cx, obj, any)) {
			printf( "JS_SetPrivate failed in SFRotationConstr.\n");
			return JS_FALSE;
		}
		cc = any->v->sfrotation.c;
	}else{
		SFRotationNative *ptr;

		if ((ptr = (SFRotationNative *)SFRotationNativeNew()) == NULL) {
			printf( "SFRotationNativeNew failed in SFRotationConstr.\n");
			return JS_FALSE;
		}

		//if (!JS_DefineProperties(cx, obj, SFRotationProperties)) {
		//	printf( "JS_DefineProperties failed in SFRotationConstr.\n");
		//	return JS_FALSE;
		//}

		if (!JS_SetPrivateFw(cx, obj, ptr)) {
			printf( "JS_SetPrivate failed in SFRotationConstr.\n");
			return JS_FALSE;
		}
		ptr->valueChanged = 1;
		cc = ptr->v.c;

	}
	if (argc == 0) {
		cc[0] = 0.0f; cc[1] = 0.0f; cc[2] = 1.0f; cc[3] = 0.0f;

	} else if (argc == 2) {
		/* two possibilities - SFVec3f/numeric, or SFVec3f/SFVec3f */
		if (argv[0].isObject()) {
/*			_ob1 = (JSObject *)argv[0]; */
			_ob1 = JSVAL_TO_OBJECT(argv[0]);


			CHECK_CLASS(cx,_ob1,argv,__FUNCTION__,SFVec3fClass)
			if(SM_method()==2){
				AnyNative *_vec = NULL;
				if ((_vec = (AnyNative *)JS_GetPrivateFw(cx, _ob1)) == NULL) {
					printf( "JS_GetPrivate failed for arg format \"o d\" in SFRotationConstr.\n");
					return JS_FALSE;
				}
				vec = _vec->v->sfvec3f.c;
			}else{
				SFVec3fNative *_vec = NULL;
				if ((_vec = (SFVec3fNative *)JS_GetPrivateFw(cx, _ob1)) == NULL) {
					printf( "JS_GetPrivate failed for arg format \"o d\" in SFRotationConstr.\n");
					return JS_FALSE;
				}
				vec = _vec->v.c;
			}
		}
		//if (JSVAL_IS_OBJECT(argv[1])) {
		if (argv[1].isObject()) {
/*			_ob2 = (JSObject *)argv[1]; */
			_ob2 = JSVAL_TO_OBJECT(argv[1]);

			v3fv3f = TRUE;

			CHECK_CLASS(cx,_ob2,argv,__FUNCTION__,SFVec3fClass)
			if(SM_method()==2){
				AnyNative *_vec2 = NULL;
				if ((_vec2 = (AnyNative *)JS_GetPrivateFw(cx, _ob2)) == NULL) {
					printf( "JS_GetPrivate failed for _ob1 in SFRotationConstr.\n");
					return JS_FALSE;
				}
				vec2 = _vec2->v->sfvec3f.c;
			}else{
				SFVec3fNative *_vec2 = NULL;
				if ((_vec2 = (SFVec3fNative *)JS_GetPrivateFw(cx, _ob2)) == NULL) {
					printf( "JS_GetPrivate failed for _ob1 in SFRotationConstr.\n");
					return JS_FALSE;
				}
				vec2 = _vec2->v.c;
			}
		} else {
			v3fv3f = FALSE;
			if (!JSVAL_IS_NUMBER(argv[1])) {
				printf ("SFRotationConstr param error - number expected\n");
				return JS_FALSE;
			}
			if (!JS_ValueToNumber(cx, argv[1], &doub)) {
				printf("JS_ValueToNumber failed in SFRotationConstr.\n");
				return JS_FALSE;
			}
		}


		if (!v3fv3f) {
			cc[0] = vec[0];
			cc[1] = vec[1];
			cc[2] = vec[2];
			cc[3] = (float) doub;
		} else {
			v1len = veclength3f(vec);
			v2len = veclength3f(vec2);
			v12dp = vecdot3f(vec,vec2);
			veccross3f(cc,vec,vec2); //I think the following is cross product
			//(ptr->v).c[0] = (float) (v1.y * v2.z - v2.y * v1.z);
			//(ptr->v).c[1] = (float) (v1.z * v2.x - v2.z * v1.x);
			//(ptr->v).c[2] = (float) (v1.x * v2.y - v2.x * v1.y);
			v12dp /= v1len * v2len;
			cc[3] = (float) atan2(sqrt(1 - v12dp * v12dp), v12dp);
		}
	} else if (argc == 4 && JS_ConvertArguments(cx, argc, argv, "d d d d",
			&(pars[0]), &(pars[1]), &(pars[2]), &(pars[3]))) {
		cc[0] = (float) pars[0];
		cc[1] = (float) pars[1];
		cc[2] = (float) pars[2];
		cc[3] = (float) pars[3];
	} else {
		printf( "Invalid arguments for SFRotationConstr.\n");
		return JS_FALSE;
	}

	#ifdef JSVRMLCLASSESVERBOSE
		printf("SFRotationConstr: obj = %p, %u args, %f %f %f %f\n",
			   obj, argc,
			   cc[0], cc[1], cc[2], cc[3]);
	#endif
	

	JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(obj));


	return JS_TRUE;
}

JSBool
SFRotationGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid,  JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();

	jsdouble d;
	float *cc;

	jsval id;
	if (!JS_IdToValue(cx,iid,&id)) {
		printf("JS_IdToValue failed in SFRotationGetProperty.\n");
		return JS_FALSE;
	}


	#ifdef JSVRMLCLASSESVERBOSE
	printf ("start of SFRotationGetProperty\n");
	#endif
	if(SM_method()== 2){
		AnyNative *ptr;
		if ((ptr = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFRotationGetProperty.\n");
			return JS_FALSE;
		}
		cc = ptr->v->sfrotation.c;
	}else{
		SFRotationNative *ptr;
		if ((ptr = (SFRotationNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFRotationGetProperty.\n");
			return JS_FALSE;
		}
		cc = ptr->v.c;
	}
	if (JSVAL_IS_INT(id)) {
		switch (JSVAL_TO_INT(id)) {
		case 0:
			d = cc[0];
			if (JS_NewNumberValue(cx, d, vp) == JS_FALSE) {
				printf(
						"JS_NewDouble failed for %f in SFRotationGetProperty.\n",
						d);
				return JS_FALSE;
			}
			break;
		case 1:
			d = cc[1];
			if (JS_NewNumberValue(cx, d, vp) == JS_FALSE) {
				printf(
						"JS_NewDouble failed for %f in SFRotationGetProperty.\n",
						d);
				return JS_FALSE;
			}
			break;
		case 2:
			d = cc[2];
			if (JS_NewNumberValue(cx, d, vp) == JS_FALSE) {
				printf(
						"JS_NewDouble failed for %f in SFRotationGetProperty.\n",
						d);
				return JS_FALSE;
			}
			break;
		case 3:
			d = cc[3];
			if (JS_NewNumberValue(cx, d, vp) == JS_FALSE) {
				printf(
						"JS_NewDouble failed for %f in SFRotationGetProperty.\n",
						d);
				return JS_FALSE;
			}
			break;
		}
	}
	return JS_TRUE;
}

JSBool
SFRotationSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();

	jsval myv;
	float *cc;


	jsval id;
	if (!JS_IdToValue(cx,iid,&id)) {
		printf("JS_IdToValue failed in SFRotationSetProperty.\n");
		return JS_FALSE;
	}


	#ifdef JSVRMLCLASSESVERBOSE
	printf ("start of SFRotationSetProperty\n");
	#endif
	if(SM_method() == 2){
		AnyNative *any;
		if ((any = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFRotationProperty.\n");
			return JS_FALSE;
		}
		if(any->valueChanged)
			(*any->valueChanged)++;
		cc = any->v->sfrotation.c;
	}else{
		SFRotationNative *ptr;

		if ((ptr = (SFRotationNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFRotationSetProperty.\n");
			return JS_FALSE;
		}
		ptr->valueChanged++;
		#ifdef JSVRMLCLASSESVERBOSE
			printf("SFRotationSetProperty: obj = %p, id = %d, valueChanged = %d\n",
				   obj, JSVAL_TO_INT(id), ptr->valueChanged);
		#endif
		cc = ptr->v.c;
	}
	if (!JS_ConvertValue(cx, *vp, JSTYPE_NUMBER, &myv)) {
		printf( "JS_ConvertValue failed in SFRotationSetProperty.\n");
		return JS_FALSE;
	}

	if (JSVAL_IS_INT(id)) {
		switch (JSVAL_TO_INT(id)) {
		case 0:
			cc[0] = (float) JSVAL_TO_DOUBLE(myv);
			break;
		case 1:
			cc[1] = (float) JSVAL_TO_DOUBLE(myv);
			break;
		case 2:
			cc[2] = (float) JSVAL_TO_DOUBLE(myv);
			break;
		case 3:
			cc[3] = (float) JSVAL_TO_DOUBLE(myv);
			break;
		}
	}
	return JS_TRUE;
}

/********************************************************************/

/* Generic SFVec2f routines that return a SFVec2f */
#define __2FADD		1
#define __2FDIVIDE 	2
#define __2FMULT	3
#define __2FSUBT	4
#define __2FDOT		5
#define __2FLENGTH	6
#define __2FNORMALIZE	8
JSBool SFVec2fGeneric( JSContext *cx, JSObject *obj,
		   uintN argc, jsval *argv, jsval *rval, int op) {

	JSObject *_paramObj, *_proto, *_retObj;
	jsdouble d=0.0;
	jsdouble d0=0.0;
	jsdouble d1=0.0;
	struct point_XYZ v1, v2;
	float *cc, *cc2, cc3[2], cclhs[2];


	/* parameters */
	int SFParam = FALSE;
	int numParam = FALSE;

	/* return values */
	int retSFVec2f = FALSE;
	int retNumeric = FALSE;

	/* is the "argv" parameter a string? */
	int param_isString;
	char *charString;
	jsdouble pars[3];
	JSString *_str;

	/* determine what kind of parameter to get */
	if ((op==__2FADD)||(op==__2FDOT)||(op==__2FSUBT))SFParam=TRUE;
	if ((op==__2FDIVIDE)||(op==__2FMULT))numParam=TRUE;

	/* determine the return value, if it is NOT a SFVec2f */
	if ((op==__2FDOT)||(op==__2FLENGTH)) retNumeric = TRUE;
	retSFVec2f = (!retNumeric);

	/* is the parameter a string, possibly gotten from the VRML/X3d
	 * side of things? */
	param_isString = JSVAL_IS_STRING (*argv);

	/* get the parameter */
	if ((SFParam) || (numParam)) {
		if (numParam) {
			if (!JSVAL_IS_NUMBER(argv[0])) {
				printf ("SFVec2f param error - number expected\n");
				return JS_FALSE;
			}
			if (!JS_ValueToNumber(cx, argv[0], &d)) {
				printf("JS_ValueToNumber failed in SFVec2f.\n");
				return JS_FALSE;
			}
		} else {
			/* did this come in from VRML as a string, or did
			 * it get created in javascript? */
			if (param_isString) {
				_str = JS_ValueToString(cx, *argv);
				charString = JS_EncodeString(cx,_str);

				if (sscanf(charString, "%lf %lf",
							&(pars[0]), &(pars[1])) != 2) {
					printf ("conversion problem in SFVec2fGeneric\n");
					return JS_FALSE;
				}
				/* printf ("past scan, %f %f %f\n",pars[0], pars[1]);*/
				cc3[0] = pars[0];
				cc3[1] = pars[1];
				cc2 = cc3;
			} else {
				if(SM_method()==2){
					if (argv[0].isObject()) {
						AnyNative *_vec2;
        				if ((_vec2 = (AnyNative *)JS_GetPrivateFw(cx, JSVAL_TO_OBJECT(argv[0]))) == NULL) {
							printf("in SFVec2f, RHS was NOT native type \n");
							return JS_FALSE;
        				}
						//if(_vec2->type == FIELDTYPE_SFVec2f) or color. if 3d convert....
						cc2 = _vec2->v->sfvec2f.c;
					}else{
						return JS_FALSE;
					}

				}else{
					SFVec2fNative *_vec2 = NULL;

					if (!JS_ConvertArguments(cx, argc, argv, "o", &_paramObj)) {
						printf( "JS_ConvertArguments failed in SFVec2f.\n");
						return JS_FALSE;
					}

					CHECK_CLASS(cx,_paramObj,argv,__FUNCTION__,SFVec2fClass)

					if ((_vec2 = (SFVec2fNative*)JS_GetPrivateFw(cx, _paramObj)) == NULL) {
						printf( "JS_GetPrivate failed for _paramObj in SFVec2f.\n");
						return JS_FALSE;
					}
					cc2 = _vec2->v.c;
				}
			}
		}
	}

	/* get our values */
	if(SM_method() == 2){
		AnyNative *_vec1;
		if ((_vec1 = (AnyNative*)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed for obj in SFVec3fAdd.\n");
			return JS_FALSE;
		}
		cc = _vec1->v->sfvec2f.c;
	}else{
		SFVec2fNative *_vec1 = NULL;
		if ((_vec1 = (SFVec2fNative*)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed for obj in SFVec2fAdd.\n");
			return JS_FALSE;
		}
		cc = _vec1->v.c;
	}
	/* do the operation */
	switch (op) {
		/* returning a SFVec2f */
		case __2FADD:
			vecadd2f(cclhs,cc,cc2);
			break;
		case __2FDIVIDE:
			vecscale2f(cclhs,cc,1.0/d);
			break;
		case __2FMULT:
			vecscale2f(cclhs,cc,d);
			break;
		case __2FSUBT:
			vecdif2f(cclhs,cc,cc2);
			break;
		case __2FDOT:
			d = vecdot2f(cc,cc2);
			break;
		case __2FLENGTH:
			d = veclength2f(cc);
			break;
		case __2FNORMALIZE:
			vecnormal2f(cclhs,cc);
			break;
		default:
		return JS_FALSE;
	}

	/* set the return object */
	if (retSFVec2f) {
		if ((_proto = JS_GetPrototypeFw(cx, obj)) == NULL) {
			printf( "JS_GetPrototype failed in SFVec2f.\n");
			return JS_FALSE;
		}
		if ((_retObj =
			JS_ConstructObjectFw(cx, &SFVec2fClass, _proto, NULL)) == NULL) {
			printf( "JS_ConstructObject failed in SFVec2f.\n");
			return JS_FALSE;
		}
		*rval = OBJECT_TO_JSVAL(_retObj);
		if(SM_method() == 2){
			AnyNative *any;
			if ((any = (AnyNative*)JS_GetPrivateFw(cx, _retObj)) == NULL) {
				printf( "JS_GetPrivate failed for _retObj in SFVec3f.\n");
				return JS_FALSE;
			}
			memcpy(any->v->sfvec2f.c,cclhs,2*sizeof(float));
		}else{
			SFVec3fNative *_retNative;
			if ((_retNative = (SFVec3fNative*)JS_GetPrivateFw(cx, _retObj)) == NULL) {
				printf( "JS_GetPrivate failed for _retObj in SFVec3f.\n");
				return JS_FALSE;
			}
			memcpy(_retNative->v.c,cclhs,2*sizeof(float));
		}

	} else if (retNumeric) {
		if (JS_NewNumberValue(cx,d,rval) == JS_FALSE) {
			printf( "JS_NewDouble failed for %f in SFVec2f.\n",d);
			return JS_FALSE;
		}
	}

	#ifdef JSVRMLCLASSESVERBOSE
	if (retSFVec2f){
		printf("SFVec2fgeneric: obj = %p, result = [%.9g, %.9g]\n",
			   obj,
			   (_retNative->v).c[0], (_retNative->v).c[1]);
	}
	if (retNumeric){
		printf("SFVec2fgeneric: obj = %p, result = %.9g\n",
			   obj, d);
	}
	#endif

	return JS_TRUE;
}

JSBool
SFVec2fAdd(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
	jsval rval;
	JSBool retval =	SFVec2fGeneric(cx, obj, argc, argv, &rval, __2FADD);
	JS_SET_RVAL(cx,vp,rval);
	return retval;

}

JSBool
SFVec2fDivide(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;
        JSBool retval = SFVec2fGeneric(cx, obj, argc, argv, &rval, __2FDIVIDE);
        JS_SET_RVAL(cx,vp,rval);
        return retval;
}

JSBool
SFVec2fMultiply(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;
        JSBool retval = SFVec2fGeneric(cx, obj, argc, argv, &rval, __2FMULT);
        JS_SET_RVAL(cx,vp,rval);
        return retval;

}

JSBool
SFVec2fSubtract(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;
        JSBool retval = SFVec2fGeneric(cx, obj, argc, argv, &rval, __2FSUBT);
        JS_SET_RVAL(cx,vp,rval);
        return retval;

}

JSBool
SFVec2fDot(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;
        JSBool retval = SFVec2fGeneric(cx, obj, argc, argv, &rval, __2FDOT);
        JS_SET_RVAL(cx,vp,rval);
        return retval;
}

JSBool
SFVec2fLength(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;
        JSBool retval = SFVec2fGeneric(cx, obj, argc, argv, &rval, __2FLENGTH);
        JS_SET_RVAL(cx,vp,rval);
        return retval;

}

JSBool
SFVec2fNormalize(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;
        JSBool retval = SFVec2fGeneric(cx, obj, argc, argv, &rval, __2FNORMALIZE);
        JS_SET_RVAL(cx,vp,rval);
        return retval;
}

JSBool
SFVec2fToString(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
    JSString *_str;
	char buff[STRING];
	float *cc;

	UNUSED(argc);
	UNUSED(argv);
	if(SM_method()==2){
	    AnyNative *ptr;
		if ((ptr = (AnyNative*)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFVec2fToString.\n");
			return JS_FALSE;
		}
		cc = ptr->v->sfvec2f.c;
	}else{
	    SFVec2fNative *ptr;
		if ((ptr = (SFVec2fNative*)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFVec2fToString.\n");
			return JS_FALSE;
		}
		cc = ptr->v.c;
	}
	memset(buff, 0, STRING);
	sprintf(buff, "%.9g %.9g",
			cc[0], cc[1]);
	_str = JS_NewStringCopyZ(cx, buff);
    JS_SET_RVAL(cx,vp,STRING_TO_JSVAL(_str));

    return JS_TRUE;
}

JSBool
SFVec2fAssign(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
	JSString *_id_jsstr;

    JSObject *_from_obj;
    char *_id_str;

	UNUSED(_id_str); // compiler warning mitigation

	if(SM_method() == 2){
		AnyNative *lhs, *rhs;
		if ((lhs = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed for obj in SFVec3dAssign.\n");
			return JS_FALSE;
		}
		if (!(*vp).isObject())
			return JS_FALSE;
        if ((rhs = (AnyNative *)JS_GetPrivateFw(cx, JSVAL_TO_OBJECT(*vp))) == NULL) {
			printf("JS_ConvertArguments failed in SFVec2fAssign. \n");
			return JS_FALSE;
        }
		AnyNativeAssign(lhs,rhs);
	}else{
		SFVec2fNative *fptr, *ptr;

		if ((ptr = (SFVec2fNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed for obj in SFVec2fAssign.\n");
			return JS_FALSE;
		}

		CHECK_CLASS(cx,obj,argv,__FUNCTION__,SFVec2fClass)

		if (JS_ConvertArguments(cx, argc, argv, "oS", &_from_obj, &_id_jsstr) == JS_TRUE) {
			_id_str = JS_EncodeString(cx,_id_jsstr);
		} else {

			printf( "JS_ConvertArguments failed in SFVec2fAssign.\n");
			return JS_FALSE;
		}

		CHECK_CLASS(cx,_from_obj,argv,__FUNCTION__,SFVec2fClass)

		if ((fptr = (SFVec2fNative *)JS_GetPrivateFw(cx, _from_obj)) == NULL) {
			printf( "JS_GetPrivate failed for _from_obj in SFVec2fAssign.\n");
			return JS_FALSE;
		}
		#ifdef JSVRMLCLASSESVERBOSE
			printf("SFVec2fAssign: obj = %p, id = \"%s\", from = %p\n",
				   obj, _id_str, _from_obj);
		#endif

		SFVec2fNativeAssign(ptr, fptr);
	}
    JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(obj));


    return JS_TRUE;
}

JSBool
SFVec2fConstr(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_NewObject(cx,&SFVec2fClass,NULL,NULL);
        jsval *argv = JS_ARGV(cx,vp);

	jsdouble pars[2];
	float *cc;

	ADD_ROOT(cx,obj)

	if(SM_method() == 2){
		AnyNative *any;
		if((any = (AnyNative*)AnyNativeNew(FIELDTYPE_SFVec2f,NULL,NULL)) == NULL){
			printf( "AnyfNativeNew failed in SFVec3fConstr.\n");
			return JS_FALSE;
		}
		if (!JS_SetPrivateFw(cx, obj, any)) {
			printf( "JS_SetPrivate failed in SFVec2fConstr.\n");
			return JS_FALSE;
		}

		cc = any->v->sfvec2f.c;
	}else{
		SFVec2fNative *ptr;

		if ((ptr = (SFVec2fNative *) SFVec2fNativeNew()) == NULL) {
			printf( "SFVec2fNativeNew failed in SFVec2fConstr.\n");
			return JS_FALSE;
		}

		//if (!JS_DefineProperties(cx, obj, SFVec2fProperties)) {
		//	printf( "JS_DefineProperties failed in SFVec2fConstr.\n");
		//	return JS_FALSE;
		//}
		if (!JS_SetPrivateFw(cx, obj, ptr)) {
			printf( "JS_SetPrivate failed in SFVec2fConstr.\n");
			return JS_FALSE;
		}
		cc = ptr->v.c;
		ptr->valueChanged = 1;

	}
	if (argc == 0) {
		cc[0] = (float) 0.0;
		cc[1] = (float) 0.0;
	} else {
		if (!JS_ConvertArguments(cx, argc, argv, "d d", &(pars[0]), &(pars[1]))) {
			printf( "JS_ConvertArguments failed in SFVec2fConstr.\n");
			return JS_FALSE;
		}
		cc[0] = (float) pars[0];
		cc[1] = (float) pars[1];
	}
	#ifdef JSVRMLCLASSESVERBOSE
		printf("SFVec2fConstr: obj = %p, %u args, %f %f\n",
			   obj, argc,
			   (ptr->v).c[0], (ptr->v).c[1]);
	#endif
	

	JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(obj));

	return JS_TRUE;
}


JSBool
SFVec2fGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid,  JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();

	jsdouble d;
	float *cc;

	jsval id;
	if (!JS_IdToValue(cx,iid,&id)) {
		printf("JS_IdToValue failed in SFVec2fGetProperty.\n");
		return JS_FALSE;
	}


	if(SM_method()==2){
		AnyNative *any;
		if ((any = (AnyNative *)JS_GetPrivateFw(cx,obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFVec2fGetProperty.\n");
			return JS_FALSE;
		}
		cc = any->v->sfvec2f.c;
	}else{
		SFVec2fNative *ptr;

		if ((ptr = (SFVec2fNative *)JS_GetPrivateFw(cx,obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFVec2fGetProperty.\n");
			return JS_FALSE;
		}
		cc = ptr->v.c;
	}
	if (JSVAL_IS_INT(id)) {
		switch (JSVAL_TO_INT(id)) {
		case 0:
			d = cc[0];
			if (JS_NewNumberValue(cx, d, vp) == JS_FALSE) {
				printf(
						"JS_NewDouble failed for %f in SFVec2fGetProperty.\n",
						d);
				return JS_FALSE;
			}
			break;
		case 1:
			d = cc[1];
			if (JS_NewNumberValue(cx, d, vp) == JS_FALSE) {
				printf(
						"JS_NewDouble failed for %f in SFVec2fGetProperty.\n",
						d);
				return JS_FALSE;
			}
			break;
		}
	}
	return JS_TRUE;
}

JSBool
SFVec2fSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();

	jsval myv;
	float *cc;


	jsval id;
	if (!JS_IdToValue(cx,iid,&id)) {
		printf("JS_IdToValue failed in SFVec2fSetProperty.\n");
		return JS_FALSE;
	}


	if(SM_method() == 2){
		AnyNative *any;
		if ((any = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFVec2fSetProperty.\n");
			return JS_FALSE;
		}
		if(any->valueChanged)
			(*any->valueChanged)++;
		cc = any->v->sfvec2f.c;
	}else{
		SFVec2fNative *ptr;

		if ((ptr = (SFVec2fNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFVec2fSetProperty.\n");
			return JS_FALSE;
		}
		ptr->valueChanged++;
		#ifdef JSVRMLCLASSESVERBOSE
			printf("SFVec2fSetProperty: obj = %p, id = %d, valueChanged = %d\n",
				   obj, JSVAL_TO_INT(id), ptr->valueChanged);
		#endif
		cc = ptr->v.c;
	}
	if (!JS_ConvertValue(cx, *vp, JSTYPE_NUMBER, &myv)) {
		printf( "JS_ConvertValue failed in SFVec2fSetProperty.\n");
		return JS_FALSE;
	}

	if (JSVAL_IS_INT(id)) {
		switch (JSVAL_TO_INT(id)) {
		case 0:
			cc[0] = (float) JSVAL_TO_DOUBLE(myv);
			break;
		case 1:
			cc[1] = (float) JSVAL_TO_DOUBLE(myv);
			break;
			default: break;
		}
	}
	return JS_TRUE;
}


/********************************************************************/

/* Generic SFVec3f routines that return a SFVec3f */
#define __3FADD		1
#define __3FDIVIDE 	2
#define __3FMULT	3
#define __3FSUBT	4
#define __3FDOT		5
#define __3FLENGTH	6
#define __3FNORMALIZE	8
#define __3FNEGATE	7
#define __3FCROSS	9

JSBool SFVec3fGeneric( JSContext *cx, JSObject *obj,
		   uintN argc, jsval *argv, jsval *rval, int op) {
	JSObject *_paramObj, *_proto, *_retObj;
	//SFVec3fNative *_vec1, *_vec2, *_retNative;
	jsdouble d=0.0;
	jsdouble d0=0.0;
	jsdouble d1=0.0;
	jsdouble d2=0.0;
	struct point_XYZ v1, v2, ret;
	float *cc, *cc2, cc3[3], cclhs[3];


	/* parameters */
	int SFParam = FALSE;
	int numParam = FALSE;

	/* return values */
	int retSFVec3f = FALSE;
	int retNumeric = FALSE;

	/* is the "argv" parameter a string? */
	int param_isString;
	char *charString;
	jsdouble pars[3];
	JSString *_str;

	#ifdef JSVRMLCLASSESVERBOSE
	printf ("SFVec3fGeneric\n");
	#endif

	/* determine what kind of parameter to get */
	if ((op==__3FADD)||(op==__3FDOT)||(op==__3FCROSS)||(op==__3FSUBT))SFParam=TRUE;
	if ((op==__3FDIVIDE)||(op==__3FMULT))numParam=TRUE;

	/* determine the return value, if it is NOT a SFVec3f */
	if ((op==__3FDOT)||(op==__3FLENGTH)) retNumeric = TRUE;
	retSFVec3f = (!retNumeric);

	/* is the parameter a string, possibly gotten from the VRML/X3d
	 * side of things? */
	param_isString = JSVAL_IS_STRING (*argv);

	/* get the parameter */
	if ((SFParam) || (numParam)) {
		if (numParam) {
			if (!JSVAL_IS_NUMBER(argv[0])) {
				printf ("SFVec3f param error - number expected\n");
				return JS_FALSE;
			}
			if (!JS_ValueToNumber(cx, argv[0], &d)) {
				printf("JS_ValueToNumber failed in SFVec3f.\n");
				return JS_FALSE;
			}
		} else {
			/* did this come in from VRML as a string, or did
			 * it get created in javascript? */
			if (param_isString) {
				_str = JS_ValueToString(cx, *argv);
				charString = JS_EncodeString(cx,_str);
				if (sscanf(charString, "%lf %lf %lf",
							&(pars[0]), &(pars[1]), &(pars[2])) != 3) {
					printf ("conversion problem in SFVec3fGeneric\n");
					return JS_FALSE;
				}
				/* printf ("past scan, %f %f %f\n",pars[0], pars[1],pars[2]);*/
				cc3[0] = pars[0];
				cc3[1] = pars[1];
				cc3[2] = pars[2];
				cc2 = cc3;
			} else {
				if(SM_method() == 2){
					if (argv[0].isObject()) {
						AnyNative *_vec2;
        				if ((_vec2 = (AnyNative *)JS_GetPrivateFw(cx, JSVAL_TO_OBJECT(argv[0]))) == NULL) {
							printf("in SFVec3d, RHS was NOT native type \n");
							return JS_FALSE;
        				}
						//if(_vec2->type == FIELDTYPE_SFVec3f) or color. if 3d convert....
						cc2 = _vec2->v->sfvec3f.c;
					}else{
						return JS_FALSE;
					}
					
				}else{
					SFVec3fNative *_vec2;
					if (!JS_ConvertArguments(cx, argc, argv, "o", &_paramObj)) {
						printf( "JS_ConvertArguments failed in SFVec3f.\n");
						return JS_FALSE;
					}

					CHECK_CLASS(cx,_paramObj,argv,__FUNCTION__,SFVec3fClass)

					/* get the second object's data */
					if ((_vec2 = (SFVec3fNative*)JS_GetPrivateFw(cx, _paramObj)) == NULL) {
						printf( "JS_GetPrivate failed for _paramObj in SFVec3f.\n");
						return JS_FALSE;
					}
					cc2 = _vec2->v.c;
				}
			}
		}
	}

	/* get our values */
	if(SM_method() == 2){
		AnyNative *_vec1;
		if ((_vec1 = (AnyNative*)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed for obj in SFVec3fAdd.\n");
			return JS_FALSE;
		}
		cc = _vec1->v->sfvec3f.c;
	}else{
		SFVec3fNative *_vec1;
		if ((_vec1 = (SFVec3fNative*)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed for obj in SFVec3fAdd.\n");
			return JS_FALSE;
		}
		cc = _vec1->v.c;
	}
	/* do the operation */
	#ifdef JSVRMLCLASSESVERBOSE
	printf ("SFVec3f generic, vec2 %f %f %f\n",pars[0],pars[1],pars[2]);
	#endif

	switch (op) {
		/* returning a SFVec3f */
		case __3FADD:
			vecadd3f(cclhs,cc,cc2);
			break;
		case __3FDIVIDE:
			vecscale3f(cclhs,cc,1.0/d);
			break;
		case __3FMULT:
			vecscale3f(cclhs,cc,d);
			break;
		case __3FSUBT:
			vecdif3f(cclhs,cc,cc2);
			break;
		case __3FDOT:
			d = vecdot3f(cc,cc2);
			break;
		case __3FCROSS:
			veccross3f(cclhs,cc,cc2);
			break;
		case __3FLENGTH:
			d = veclength3f(cc);
			break;
		case __3FNORMALIZE:
			vecnormalize3f(cclhs,cc);
			break;
		case __3FNEGATE:
			vecnegate3f(cclhs,cc);
			break;
		default:
			printf ("woops... %d\n",op);
		return JS_FALSE;
	}

	#ifdef JSVRMLCLASSESVERBOSE
	printf ("past calcs\n");
	#endif

	/* set the return object */
	if (retSFVec3f) {
		#ifdef JSVRMLCLASSESVERBOSE
		printf ("returning SFVec3f\n");
		#endif
		if ((_proto = JS_GetPrototypeFw(cx, obj)) == NULL) {
			printf( "JS_GetPrototype failed in SFVec3f.\n");
			return JS_FALSE;
		}
		if ((_retObj =
			JS_ConstructObjectFw(cx, &SFVec3fClass, _proto, NULL)) == NULL) {
			printf( "JS_ConstructObject failed in SFVec3f.\n");
			return JS_FALSE;
		}
		*rval = OBJECT_TO_JSVAL(_retObj);
		if(SM_method() == 2){
			AnyNative *any;
			if ((any = (AnyNative*)JS_GetPrivateFw(cx, _retObj)) == NULL) {
				printf( "JS_GetPrivate failed for _retObj in SFVec3f.\n");
				return JS_FALSE;
			}
			memcpy(any->v->sfvec3f.c,cclhs,3*sizeof(float));
		}else{
			SFVec3fNative *_retNative;
			if ((_retNative = (SFVec3fNative*)JS_GetPrivateFw(cx, _retObj)) == NULL) {
				printf( "JS_GetPrivate failed for _retObj in SFVec3f.\n");
				return JS_FALSE;
			}
			memcpy(_retNative->v.c,cclhs,3*sizeof(float));
		}
	} else if (retNumeric) {
		if (JS_NewNumberValue(cx,d,rval) == JS_FALSE) {
			printf( "JS_NewDouble failed for %f in SFVec3f.\n",d);
			return JS_FALSE;
		}
	}
	#ifdef JSVRMLCLASSESVERBOSE
	if (retSFVec3f){
		printf("SFVec3fgeneric: obj = %p, result = [%.9g, %.9g, %.9g]\n",
			   obj,
			   (_retNative->v).c[0], (_retNative->v).c[1],
			   (_retNative->v).c[2]);
	}
	if (retNumeric){
		printf("SFVec2fgeneric: obj = %p, result = %.9g\n",
			   obj, d);
	}
	#endif
return JS_TRUE;
}

JSBool
SFVec3fAdd(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;
        JSBool retval = SFVec3fGeneric(cx, obj, argc, argv, &rval, __3FADD);
        JS_SET_RVAL(cx,vp,rval);
        return retval;
}

JSBool
SFVec3fCross(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;
        JSBool retval = SFVec3fGeneric(cx, obj, argc, argv, &rval, __3FCROSS);
        JS_SET_RVAL(cx,vp,rval);
        return retval;
}

JSBool
SFVec3fDivide(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;
        JSBool retval = SFVec3fGeneric(cx, obj, argc, argv, &rval, __3FDIVIDE);
        JS_SET_RVAL(cx,vp,rval);
        return retval;
}

JSBool
SFVec3fDot(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;
        JSBool retval = SFVec3fGeneric(cx, obj, argc, argv, &rval, __3FDOT);
        JS_SET_RVAL(cx,vp,rval);
        return retval;
}

JSBool
SFVec3fLength(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;
        JSBool retval = SFVec3fGeneric(cx, obj, argc, argv, &rval, __3FLENGTH);
        JS_SET_RVAL(cx,vp,rval);
        return retval;
}


JSBool
SFVec3fMultiply(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;
        JSBool retval = SFVec3fGeneric(cx, obj, argc, argv, &rval, __3FMULT);
        JS_SET_RVAL(cx,vp,rval);
        return retval;
}


JSBool
SFVec3fNegate(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;
        JSBool retval = SFVec3fGeneric(cx, obj, argc, argv, &rval, __3FNEGATE);
        JS_SET_RVAL(cx,vp,rval);
        return retval;
}

JSBool
SFVec3fNormalize(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;
        JSBool retval = SFVec3fGeneric(cx, obj, argc, argv, &rval, __3FNORMALIZE);
        JS_SET_RVAL(cx,vp,rval);
        return retval;
}

JSBool
SFVec3fSubtract(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;
        JSBool retval = SFVec3fGeneric(cx, obj, argc, argv, &rval, __3FSUBT);
        JS_SET_RVAL(cx,vp,rval);
        return retval;
}

JSBool
SFVec3fToString(JSContext *cx, uintN argc, jsval *vp) {
	JSObject *obj = JS_THIS_OBJECT(cx,vp);
	jsval *argv = JS_ARGV(cx,vp);

    JSString *_str;
	char buff[STRING];
	float *cc;

	UNUSED(argc);
	UNUSED(argv);
	if(SM_method()==2){
	    AnyNative *ptr;
		if ((ptr = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFVec3fToString.\n");
			return JS_FALSE;
		}
		cc = ptr->v->sfvec3f.c;
	}else{
	    SFVec3fNative *ptr;
		if ((ptr = (SFVec3fNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFVec3fToString.\n");
			return JS_FALSE;
		}
		cc = ptr->v.c;
	}
	memset(buff, 0, STRING);
	sprintf(buff, "%.9g %.9g %.9g",
			cc[0], cc[1], cc[2]);
	_str = JS_NewStringCopyZ(cx, buff);

	JS_SET_RVAL(cx,vp,STRING_TO_JSVAL(_str));

	#ifdef JSVRMLCLASSESVERBOSE
		printf ("SFVec3fToString, string is :%s:\n",buff);
	#endif

    return JS_TRUE;
}


JSBool
SFVec3fAssign(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
	JSString *_id_jsstr;
    JSObject *_from_obj;
    char *_id_str;


	UNUSED(_id_str); // compiler warning mitigation

	#ifdef JSVRMLCLASSESVERBOSE
		printf ("start of SFVec3fAssign\n");
	#endif

	if(SM_method() == 2){
		AnyNative *lhs, *rhs;
		if ((lhs = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed for obj in SFVec3dAssign.\n");
			return JS_FALSE;
		}
		if (!(*vp).isObject())
			return JS_FALSE;
        if ((rhs = (AnyNative *)JS_GetPrivateFw(cx, JSVAL_TO_OBJECT(*vp))) == NULL) {
			printf("JS_ConvertArguments failed in SFVec3fAssign. \n");
			return JS_FALSE;
        }
		AnyNativeAssign(lhs,rhs);
	}else{
	    SFVec3fNative *fptr, *ptr;
		if ((ptr = (SFVec3fNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed for obj in SFVec3fAssign.\n");
			return JS_FALSE;
		}

		CHECK_CLASS(cx,obj,argv,__FUNCTION__,SFVec3fClass)

		if (JS_ConvertArguments(cx, argc, argv, "oS", &_from_obj, &_id_jsstr)) {
			_id_str = JS_EncodeString(cx,_id_jsstr);
		} else {
			printf( "JS_ConvertArguments failed in SFVec3fAssign.\n");
			return JS_FALSE;
		}

		CHECK_CLASS(cx,_from_obj,argv,__FUNCTION__,SFVec3fClass) 

		if ((fptr = (SFVec3fNative *)JS_GetPrivateFw(cx, _from_obj)) == NULL) {
			printf( "JS_GetPrivate failed for _from_obj in SFVec3fAssign.\n");
			return JS_FALSE;
		}
		#ifdef JSVRMLCLASSESVERBOSE
			printf("SFVec3fAssign: obj = %p, id = \"%s\", from = %p\n",
				   obj, _id_str, _from_obj);
		#endif

		SFVec3fNativeAssign(ptr, fptr);
	}
	JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(obj));

	#ifdef JSVRMLCLASSESVERBOSE
		printf ("end of SFVec3fAssign\n");
	#endif

    return JS_TRUE;
}

JSBool
SFVec3fConstr(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_NewObject(cx,&SFVec3fClass,NULL,NULL);
        jsval *argv = JS_ARGV(cx,vp);
	jsdouble pars[3];
	float *cc;
	
	#ifdef JSVRMLCLASSESVERBOSE
		printf ("start of SFVec3fConstr\n");
	#endif

	ADD_ROOT(cx,obj)
	if(SM_method() == 2){
		AnyNative *any;
		if((any = (AnyNative*)AnyNativeNew(FIELDTYPE_SFVec3f,NULL,NULL)) == NULL){
			printf( "AnyfNativeNew failed in SFVec3fConstr.\n");
			return JS_FALSE;
		}
		if (!JS_SetPrivateFw(cx, obj, any)) {
			printf( "JS_SetPrivate failed in SFVec3fConstr.\n");
			return JS_FALSE;
		}
		cc = any->v->sfvec3f.c;
	}else{
		SFVec3fNative *ptr;
		if ((ptr = (SFVec3fNative *) SFVec3fNativeNew()) == NULL) {
			printf( "SFVec3fNativeNew failed in SFVec3fConstr.\n");
			return JS_FALSE;
		}

		//if (!JS_DefineProperties(cx, obj, SFVec3fProperties)) {
		//	printf( "JS_DefineProperties failed in SFVec3fConstr.\n");
		//	return JS_FALSE;
		//}
		if (!JS_SetPrivateFw(cx, obj, ptr)) {
			printf( "JS_SetPrivate failed in SFVec3fConstr.\n");
			return JS_FALSE;
		}
		ptr->valueChanged = 1;
		cc = ptr->v.c;
	}
	if (argc == 0) {
		cc[0] = 0.0f;
		cc[1] = 0.0f;
		cc[2] = 0.0f;
	} else if(argc == 1){
		if(SM_method() == 2){
			int found = 0;
			if (argv[0].isObject()) {
				AnyNative *rhs;
        		if ((rhs = (AnyNative *)JS_GetPrivateFw(cx, JSVAL_TO_OBJECT(argv[0]))) != NULL) {
					union anyVrml *anyv = rhs->v;
					int rhstype = rhs->type;
					found = 1;
					switch(rhstype){
						case FIELDTYPE_SFVec3f:
							veccopy3f(cc,anyv->sfvec3f.c); break;
						case FIELDTYPE_SFVec3d:
							double2float(cc,anyv->sfvec3d.c,3); break;
						case FIELDTYPE_SFRotation:
							veccopy3f(cc,anyv->sfrotation.c); break;
						default:
							vecset3f(cc,0.0f,0.0f,0.0f);
							ConsoleMessage("new SFVec3f( obj ) doesn't handle obj type %d\n",rhstype);
							found = 0;
					}
				}
			}
			if(!found)
				return JS_FALSE;
		}
	} else {
		if (!JS_ConvertArguments(cx, argc, argv, "d d d",
				 &(pars[0]), &(pars[1]), &(pars[2]))) {
			printf( "JS_ConvertArguments failed in SFVec3fConstr.\n");
			return JS_FALSE;
		}
		cc[0] = (float) pars[0];
		cc[1] = (float) pars[1];
		cc[2] = (float) pars[2];
	}
	#ifdef JSVRMLCLASSESVERBOSE
		printf("SFVec3fConstr: obj = %p, %u args, %f %f %f\n",
			   obj, argc,
			   cc[0], cc[1], cc[2]);
	#endif
	

	JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(obj));
	return JS_TRUE;
}

JSBool
SFVec3fGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid,  JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();

	jsdouble d;
	float *cc;
	#ifdef JSVRMLCLASSESVERBOSE
	JSString *_idStr;
	char *_id_c;
	#endif

	jsval id;
	if (!JS_IdToValue(cx,iid,&id)) {
		printf("JS_IdToValue failed in SFVec3fGetProperty.\n");
		return JS_FALSE;
	}

	#ifdef JSVRMLCLASSESVERBOSE

	//JSString *_idStr;
	//char *_id_c;

/* note, since same variables are used, this first bit gets overwritten -- commenting out
	_idStr = JS_ValueToString(cx, id);
	_id_c = JS_GetStringBytes(_idStr);*/
	_idStr = JS_ValueToString(cx, *vp);
#if JS_VERSION < 185
	_id_c = JS_GetStringBytes(_idStr);
#else
	_id_c = JS_EncodeString(cx,_idStr);
#endif
	#endif
	if(SM_method()==2){
		AnyNative *any;
		if ((any = (AnyNative *)JS_GetPrivateFw(cx,obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFVec3fGetProperty.\n");
			return JS_FALSE;
		}
		cc = any->v->sfvec3f.c;
	}else{
		SFVec3fNative *ptr;
		if ((ptr = (SFVec3fNative *)JS_GetPrivateFw(cx,obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFVec3fGetProperty.\n");
			return JS_FALSE;
		}
		cc = ptr->v.c;
	}
	if (JSVAL_IS_INT(id)) {
		switch (JSVAL_TO_INT(id)) {
		case 0:
			d = cc[0];
			if (JS_NewNumberValue(cx, d, vp) == JS_FALSE) {
				printf(
						"JS_NewDouble failed for %f in SFVec3fGetProperty.\n",
						d);
				return JS_FALSE;
			}
			break;
		case 1:
			d = cc[1];
			if (JS_NewNumberValue(cx, d, vp) == JS_FALSE) {
				printf(
						"JS_NewDouble failed for %f in SFVec3fGetProperty.\n",
						d);
				return JS_FALSE;
			}
			break;
		case 2:
			d = cc[2];
			if (JS_NewNumberValue(cx, d, vp) == JS_FALSE) {
				printf(
						"JS_NewDouble failed for %f in SFVec3fGetProperty.\n",
						d);
				return JS_FALSE;
			}
			break;
		}
	} else {
		#ifdef JSVRMLCLASSESVERBOSE
		printf ("SFVec3fGetProperty, id is NOT an int...\n");
		#endif
	}

	return JS_TRUE;
}

JSBool
SFVec3fSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();

	jsval myv;
	float *cc;

	jsval id;
	if (!JS_IdToValue(cx,iid,&id)) {
		printf("JS_IdToValue failed in SFVec3fSetProperty.\n");
		return JS_FALSE;
	}


	if(SM_method() == 2){
		AnyNative *any;
		if ((any = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFVec3fSetProperty.\n");
			return JS_FALSE;
		}
		if(any->valueChanged)
			(*any->valueChanged)++;
		cc = any->v->sfvec3f.c;
	}else{
		SFVec3fNative *ptr;
		if ((ptr = (SFVec3fNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFVec3fSetProperty.\n");
			return JS_FALSE;
		}
		ptr->valueChanged++;
		#ifdef JSVRMLCLASSESVERBOSE
			printf("SFVec3fSetProperty: obj = %p, id = %d, valueChanged = %d\n",
				   obj, JSVAL_TO_INT(id), ptr->valueChanged);
		#endif
		cc = ptr->v.c;
	}
	if (!JS_ConvertValue(cx, *vp, JSTYPE_NUMBER, &myv)) {
		printf( "JS_ConvertValue failed in SFVec3fSetProperty.\n");
		return JS_FALSE;
	}

	if (JSVAL_IS_INT(id)) {
		switch (JSVAL_TO_INT(id)) {
		case 0:
			cc[0] = (float) JSVAL_TO_DOUBLE(myv);
			break;
		case 1:
			cc[1] = (float) JSVAL_TO_DOUBLE(myv);
			break;
		case 2:
			cc[2] = (float) JSVAL_TO_DOUBLE(myv);
			break;
		}
	}
	return JS_TRUE;
}


/********************************************************************/
//SFVec2d
JSBool SFVec2dGeneric(JSContext* cx, JSObject* obj,
	uintN argc, jsval* argv, jsval* rval, int op) {

	JSObject* _paramObj, * _proto, * _retObj;
	jsdouble d = 0.0;
	jsdouble d0 = 0.0;
	jsdouble d1 = 0.0;
	struct point_XYZ v1, v2;
	double* cc, * cc2, cc3[2], cclhs[2];


	/* parameters */
	int SFParam = FALSE;
	int numParam = FALSE;

	/* return values */
	int retSFVec2d = FALSE;
	int retNumeric = FALSE;

	/* is the "argv" parameter a string? */
	int param_isString;
	char* charString;
	jsdouble pars[3];
	JSString* _str;

	/* determine what kind of parameter to get */
	if ((op == __2FADD) || (op == __2FDOT) || (op == __2FSUBT))SFParam = TRUE;
	if ((op == __2FDIVIDE) || (op == __2FMULT))numParam = TRUE;

	/* determine the return value, if it is NOT a SFVec2d */
	if ((op == __2FDOT) || (op == __2FLENGTH)) retNumeric = TRUE;
	retSFVec2d = (!retNumeric);

	/* is the parameter a string, possibly gotten from the VRML/X3d
	 * side of things? */
	param_isString = JSVAL_IS_STRING(*argv);

	/* get the parameter */
	if ((SFParam) || (numParam)) {
		if (numParam) {
			if (!JSVAL_IS_NUMBER(argv[0])) {
				printf("SFVec2d param error - number expected\n");
				return JS_FALSE;
			}
			if (!JS_ValueToNumber(cx, argv[0], &d)) {
				printf("JS_ValueToNumber failed in SFVec2d.\n");
				return JS_FALSE;
			}
		}
		else {
			/* did this come in from VRML as a string, or did
			 * it get created in javascript? */
			if (param_isString) {
				_str = JS_ValueToString(cx, *argv);
				charString = JS_EncodeString(cx, _str);

				if (sscanf(charString, "%lf %lf",
					&(pars[0]), &(pars[1])) != 2) {
					printf("conversion problem in SFVec2dGeneric\n");
					return JS_FALSE;
				}
				/* printf ("past scan, %f %f %f\n",pars[0], pars[1]);*/
				cc3[0] = pars[0];
				cc3[1] = pars[1];
				cc2 = cc3;
			}
			else {
				if (SM_method() == 2) {
					if (argv[0].isObject()) {
						AnyNative* _vec2;
						if ((_vec2 = (AnyNative*)JS_GetPrivateFw(cx, JSVAL_TO_OBJECT(argv[0]))) == NULL) {
							printf("in SFVec2d, RHS was NOT native type \n");
							return JS_FALSE;
						}
						//if(_vec2->type == FIELDTYPE_SFVec2d) or color. if 3d convert....
						cc2 = _vec2->v->sfvec2d.c;
					}
					else {
						return JS_FALSE;
					}

				}
				else {
					SFVec2dNative* _vec2 = NULL;

					if (!JS_ConvertArguments(cx, argc, argv, "o", &_paramObj)) {
						printf("JS_ConvertArguments failed in SFVec2d.\n");
						return JS_FALSE;
					}

					CHECK_CLASS(cx, _paramObj, argv, __FUNCTION__, SFVec2dClass)

						if ((_vec2 = (SFVec2dNative*)JS_GetPrivateFw(cx, _paramObj)) == NULL) {
							printf("JS_GetPrivate failed for _paramObj in SFVec2d.\n");
							return JS_FALSE;
						}
					cc2 = _vec2->v.c;
				}
			}
		}
	}

	/* get our values */
	if (SM_method() == 2) {
		AnyNative* _vec1;
		if ((_vec1 = (AnyNative*)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf("JS_GetPrivate failed for obj in SFVec3fAdd.\n");
			return JS_FALSE;
		}
		cc = _vec1->v->sfvec2d.c;
	}
	else {
		SFVec2dNative* _vec1 = NULL;
		if ((_vec1 = (SFVec2dNative*)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf("JS_GetPrivate failed for obj in SFVec2dAdd.\n");
			return JS_FALSE;
		}
		cc = _vec1->v.c;
	}
	/* do the operation */
	switch (op) {
		/* returning a SFVec2d */
	case __2FADD:
		vecadd2d(cclhs, cc, cc2);
		break;
	case __2FDIVIDE:
		vecscale2d(cclhs, cc, 1.0 / d);
		break;
	case __2FMULT:
		vecscale2d(cclhs, cc, d);
		break;
	case __2FSUBT:
		vecdif2d(cclhs, cc, cc2);
		break;
	case __2FDOT:
		d = vecdot2d(cc, cc2);
		break;
	case __2FLENGTH:
		d = veclength2d(cc);
		break;
	case __2FNORMALIZE:
		vecnormal2d(cclhs, cc);
		break;
	default:
		return JS_FALSE;
	}

	/* set the return object */
	if (retSFVec2d) {
		if ((_proto = JS_GetPrototypeFw(cx, obj)) == NULL) {
			printf("JS_GetPrototype failed in SFVec2d.\n");
			return JS_FALSE;
		}
		if ((_retObj =
			JS_ConstructObjectFw(cx, &SFVec2dClass, _proto, NULL)) == NULL) {
			printf("JS_ConstructObject failed in SFVec2d.\n");
			return JS_FALSE;
		}
		*rval = OBJECT_TO_JSVAL(_retObj);
		if (SM_method() == 2) {
			AnyNative* any;
			if ((any = (AnyNative*)JS_GetPrivateFw(cx, _retObj)) == NULL) {
				printf("JS_GetPrivate failed for _retObj in SFVec3f.\n");
				return JS_FALSE;
			}
			memcpy(any->v->sfvec2d.c, cclhs, 2 * sizeof(float));
		}
		else {
			SFVec3fNative* _retNative;
			if ((_retNative = (SFVec3fNative*)JS_GetPrivateFw(cx, _retObj)) == NULL) {
				printf("JS_GetPrivate failed for _retObj in SFVec3f.\n");
				return JS_FALSE;
			}
			memcpy(_retNative->v.c, cclhs, 2 * sizeof(float));
		}

	}
	else if (retNumeric) {
		if (JS_NewNumberValue(cx, d, rval) == JS_FALSE) {
			printf("JS_NewDouble failed for %f in SFVec2d.\n", d);
			return JS_FALSE;
		}
	}

#ifdef JSVRMLCLASSESVERBOSE
	if (retSFVec2d) {
		printf("SFVec2dgeneric: obj = %p, result = [%.9g, %.9g]\n",
			obj,
			(_retNative->v).c[0], (_retNative->v).c[1]);
	}
	if (retNumeric) {
		printf("SFVec2dgeneric: obj = %p, result = %.9g\n",
			obj, d);
	}
#endif

	return JS_TRUE;
}

JSBool
SFVec2dAdd(JSContext* cx, uintN argc, jsval* vp) {
	JSObject* obj = JS_THIS_OBJECT(cx, vp);
	jsval* argv = JS_ARGV(cx, vp);
	jsval rval;
	JSBool retval = SFVec2dGeneric(cx, obj, argc, argv, &rval, __2FADD);
	JS_SET_RVAL(cx, vp, rval);
	return retval;

}

JSBool
SFVec2dDivide(JSContext* cx, uintN argc, jsval* vp) {
	JSObject* obj = JS_THIS_OBJECT(cx, vp);
	jsval* argv = JS_ARGV(cx, vp);
	jsval rval;
	JSBool retval = SFVec2dGeneric(cx, obj, argc, argv, &rval, __2FDIVIDE);
	JS_SET_RVAL(cx, vp, rval);
	return retval;
}

JSBool
SFVec2dMultiply(JSContext* cx, uintN argc, jsval* vp) {
	JSObject* obj = JS_THIS_OBJECT(cx, vp);
	jsval* argv = JS_ARGV(cx, vp);
	jsval rval;
	JSBool retval = SFVec2dGeneric(cx, obj, argc, argv, &rval, __2FMULT);
	JS_SET_RVAL(cx, vp, rval);
	return retval;

}

JSBool
SFVec2dSubtract(JSContext* cx, uintN argc, jsval* vp) {
	JSObject* obj = JS_THIS_OBJECT(cx, vp);
	jsval* argv = JS_ARGV(cx, vp);
	jsval rval;
	JSBool retval = SFVec2dGeneric(cx, obj, argc, argv, &rval, __2FSUBT);
	JS_SET_RVAL(cx, vp, rval);
	return retval;

}

JSBool
SFVec2dDot(JSContext* cx, uintN argc, jsval* vp) {
	JSObject* obj = JS_THIS_OBJECT(cx, vp);
	jsval* argv = JS_ARGV(cx, vp);
	jsval rval;
	JSBool retval = SFVec2dGeneric(cx, obj, argc, argv, &rval, __2FDOT);
	JS_SET_RVAL(cx, vp, rval);
	return retval;
}

JSBool
SFVec2dLength(JSContext* cx, uintN argc, jsval* vp) {
	JSObject* obj = JS_THIS_OBJECT(cx, vp);
	jsval* argv = JS_ARGV(cx, vp);
	jsval rval;
	JSBool retval = SFVec2dGeneric(cx, obj, argc, argv, &rval, __2FLENGTH);
	JS_SET_RVAL(cx, vp, rval);
	return retval;

}

JSBool
SFVec2dNormalize(JSContext* cx, uintN argc, jsval* vp) {
	JSObject* obj = JS_THIS_OBJECT(cx, vp);
	jsval* argv = JS_ARGV(cx, vp);
	jsval rval;
	JSBool retval = SFVec2dGeneric(cx, obj, argc, argv, &rval, __2FNORMALIZE);
	JS_SET_RVAL(cx, vp, rval);
	return retval;
}

JSBool
SFVec2dToString(JSContext* cx, uintN argc, jsval* vp) {
	JSObject* obj = JS_THIS_OBJECT(cx, vp);
	jsval* argv = JS_ARGV(cx, vp);
	JSString* _str;
	char buff[STRING];
	double* cc;

	UNUSED(argc);
	UNUSED(argv);
	if (SM_method() == 2) {
		AnyNative* ptr;
		if ((ptr = (AnyNative*)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf("JS_GetPrivate failed in SFVec2dToString.\n");
			return JS_FALSE;
		}
		cc = ptr->v->sfvec2d.c;
	}
	else {
		SFVec2dNative* ptr;
		if ((ptr = (SFVec2dNative*)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf("JS_GetPrivate failed in SFVec2dToString.\n");
			return JS_FALSE;
		}
		cc = ptr->v.c;
	}
	memset(buff, 0, STRING);
	sprintf(buff, "%.9g %.9g",
		cc[0], cc[1]);
	_str = JS_NewStringCopyZ(cx, buff);
	JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(_str));

	return JS_TRUE;
}

JSBool
SFVec2dAssign(JSContext* cx, uintN argc, jsval* vp) {
	JSObject* obj = JS_THIS_OBJECT(cx, vp);
	jsval* argv = JS_ARGV(cx, vp);
	JSString* _id_jsstr;

	JSObject* _from_obj;
	char* _id_str;

	UNUSED(_id_str); // compiler warning mitigation

	if (SM_method() == 2) {
		AnyNative* lhs, * rhs;
		if ((lhs = (AnyNative*)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf("JS_GetPrivate failed for obj in SFVec3dAssign.\n");
			return JS_FALSE;
		}
		if (!(*vp).isObject())
			return JS_FALSE;
		if ((rhs = (AnyNative*)JS_GetPrivateFw(cx, JSVAL_TO_OBJECT(*vp))) == NULL) {
			printf("JS_ConvertArguments failed in SFVec2dAssign. \n");
			return JS_FALSE;
		}
		AnyNativeAssign(lhs, rhs);
	}
	else {
		SFVec2dNative* fptr, * ptr;

		if ((ptr = (SFVec2dNative*)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf("JS_GetPrivate failed for obj in SFVec2dAssign.\n");
			return JS_FALSE;
		}

		CHECK_CLASS(cx, obj, argv, __FUNCTION__, SFVec2dClass)

			if (JS_ConvertArguments(cx, argc, argv, "oS", &_from_obj, &_id_jsstr) == JS_TRUE) {
				_id_str = JS_EncodeString(cx, _id_jsstr);
			}
			else {

				printf("JS_ConvertArguments failed in SFVec2dAssign.\n");
				return JS_FALSE;
			}

		CHECK_CLASS(cx, _from_obj, argv, __FUNCTION__, SFVec2dClass)

			if ((fptr = (SFVec2dNative*)JS_GetPrivateFw(cx, _from_obj)) == NULL) {
				printf("JS_GetPrivate failed for _from_obj in SFVec2dAssign.\n");
				return JS_FALSE;
			}
#ifdef JSVRMLCLASSESVERBOSE
		printf("SFVec2dAssign: obj = %p, id = \"%s\", from = %p\n",
			obj, _id_str, _from_obj);
#endif

		SFVec2fNativeAssign(ptr, fptr);
	}
	JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(obj));


	return JS_TRUE;
}

JSBool
SFVec2dConstr(JSContext* cx, uintN argc, jsval* vp) {
	JSObject* obj = JS_NewObject(cx, &SFVec2dClass, NULL, NULL);
	jsval* argv = JS_ARGV(cx, vp);

	jsdouble pars[2];
	double* cc;

	ADD_ROOT(cx, obj)

		if (SM_method() == 2) {
			AnyNative* any;
			if ((any = (AnyNative*)AnyNativeNew(FIELDTYPE_SFVec2d, NULL, NULL)) == NULL) {
				printf("AnyfNativeNew failed in SFVec3fConstr.\n");
				return JS_FALSE;
			}
			if (!JS_SetPrivateFw(cx, obj, any)) {
				printf("JS_SetPrivate failed in SFVec2dConstr.\n");
				return JS_FALSE;
			}

			cc = any->v->sfvec2d.c;
		}
		else {
			SFVec2dNative* ptr;

			if ((ptr = (SFVec2dNative*)SFVec2dNativeNew()) == NULL) {
				printf("SFVec2dNativeNew failed in SFVec2dConstr.\n");
				return JS_FALSE;
			}

			//if (!JS_DefineProperties(cx, obj, SFVec2dProperties)) {
			//	printf( "JS_DefineProperties failed in SFVec2dConstr.\n");
			//	return JS_FALSE;
			//}
			if (!JS_SetPrivateFw(cx, obj, ptr)) {
				printf("JS_SetPrivate failed in SFVec2dConstr.\n");
				return JS_FALSE;
			}
			cc = ptr->v.c;
			ptr->valueChanged = 1;

		}
	if (argc == 0) {
		cc[0] = 0.0;
		cc[1] = 0.0;
	}
	else {
		if (!JS_ConvertArguments(cx, argc, argv, "d d", &(pars[0]), &(pars[1]))) {
			printf("JS_ConvertArguments failed in SFVec2dConstr.\n");
			return JS_FALSE;
		}
		cc[0] = pars[0];
		cc[1] = pars[1];
	}
#ifdef JSVRMLCLASSESVERBOSE
	printf("SFVec2dConstr: obj = %p, %u args, %f %f\n",
		obj, argc,
		(ptr->v).c[0], (ptr->v).c[1]);
#endif


	JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(obj));

	return JS_TRUE;
}


JSBool
SFVec2dGetProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp) {
	JSObject* obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval* vp = hvp.address();

	jsdouble d;
	double* cc;

	jsval id;
	if (!JS_IdToValue(cx, iid, &id)) {
		printf("JS_IdToValue failed in SFVec2dGetProperty.\n");
		return JS_FALSE;
	}


	if (SM_method() == 2) {
		AnyNative* any;
		if ((any = (AnyNative*)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf("JS_GetPrivate failed in SFVec2dGetProperty.\n");
			return JS_FALSE;
		}
		cc = any->v->sfvec2d.c;
	}
	else {
		SFVec2dNative* ptr;

		if ((ptr = (SFVec2dNative*)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf("JS_GetPrivate failed in SFVec2dGetProperty.\n");
			return JS_FALSE;
		}
		cc = ptr->v.c;
	}
	if (JSVAL_IS_INT(id)) {
		switch (JSVAL_TO_INT(id)) {
		case 0:
			d = cc[0];
			if (JS_NewNumberValue(cx, d, vp) == JS_FALSE) {
				printf(
					"JS_NewDouble failed for %f in SFVec2dGetProperty.\n",
					d);
				return JS_FALSE;
			}
			break;
		case 1:
			d = cc[1];
			if (JS_NewNumberValue(cx, d, vp) == JS_FALSE) {
				printf(
					"JS_NewDouble failed for %f in SFVec2dGetProperty.\n",
					d);
				return JS_FALSE;
			}
			break;
		}
	}
	return JS_TRUE;
}

JSBool
SFVec2dSetProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp) {
	JSObject* obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval* vp = hvp.address();

	jsval myv;
	double* cc;


	jsval id;
	if (!JS_IdToValue(cx, iid, &id)) {
		printf("JS_IdToValue failed in SFVec2dSetProperty.\n");
		return JS_FALSE;
	}


	if (SM_method() == 2) {
		AnyNative* any;
		if ((any = (AnyNative*)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf("JS_GetPrivate failed in SFVec2dSetProperty.\n");
			return JS_FALSE;
		}
		if (any->valueChanged)
			(*any->valueChanged)++;
		cc = any->v->sfvec2d.c;
	}
	else {
		SFVec2dNative* ptr;

		if ((ptr = (SFVec2dNative*)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf("JS_GetPrivate failed in SFVec2dSetProperty.\n");
			return JS_FALSE;
		}
		ptr->valueChanged++;
#ifdef JSVRMLCLASSESVERBOSE
		printf("SFVec2dSetProperty: obj = %p, id = %d, valueChanged = %d\n",
			obj, JSVAL_TO_INT(id), ptr->valueChanged);
#endif
		cc = ptr->v.c;
	}
	if (!JS_ConvertValue(cx, *vp, JSTYPE_NUMBER, &myv)) {
		printf("JS_ConvertValue failed in SFVec2dSetProperty.\n");
		return JS_FALSE;
	}

	if (JSVAL_IS_INT(id)) {
		switch (JSVAL_TO_INT(id)) {
		case 0:
			cc[0] = (float)JSVAL_TO_DOUBLE(myv);
			break;
		case 1:
			cc[1] = (float)JSVAL_TO_DOUBLE(myv);
			break;
		default: break;
		}
	}
	return JS_TRUE;
}


/********************************************************************/

/* Generic SFVec3d routines that return a SFVec3d -- note, already defined above
#define __3FADD		1
#define __3FDIVIDE 	2
#define __3FMULT	3
#define __3FSUBT	4
#define __3FDOT		5
#define __3FLENGTH	6
#define __3FNORMALIZE	8
#define __3FNEGATE	7
#define __3FCROSS	9
*/

JSBool SFVec3dGeneric( JSContext *cx, JSObject *obj,
		   uintN argc, jsval *argv, jsval *rval, int op) {
	JSObject *_paramObj, *_proto, *_retObj;
	//SFVec3dNative *_vec1, *_vec2, *_retNative;
	jsdouble d=0.0;
	jsdouble d0=0.0;
	jsdouble d1=0.0;
	jsdouble d2=0.0;
	double *cc, *cc2, cc3[3], cclhs[3];
	struct point_XYZ v1, v2, ret;


	/* parameters */
	int SFParam = FALSE;
	int numParam = FALSE;

	/* return values */
	int retSFVec3d = FALSE;
	int retNumeric = FALSE;

	/* is the "argv" parameter a string? */
	int param_isString;
	char *charString;
	jsdouble pars[3];
	JSString *_str;

	/* determine what kind of parameter to get */
	if ((op==__3FADD)||(op==__3FDOT)||(op==__3FCROSS)||(op==__3FSUBT))SFParam=TRUE;
	if ((op==__3FDIVIDE)||(op==__3FMULT))numParam=TRUE;

	/* determine the return value, if it is NOT a SFVec3d */
	if ((op==__3FDOT)||(op==__3FLENGTH)) retNumeric = TRUE;
	retSFVec3d = (!retNumeric);

	/* is the parameter a string, possibly gotten from the VRML/X3d
	 * side of things? */
	param_isString = JSVAL_IS_STRING (*argv);

	/* get the parameter */
	if ((SFParam) || (numParam)) {
		if (numParam) {
			if (!JSVAL_IS_NUMBER(argv[0])) {
				printf ("SFVec3d param error - number expected\n");
				return JS_FALSE;
			}
			if (!JS_ValueToNumber(cx, argv[0], &d)) {
				printf("JS_ValueToNumber failed in SFVec3d.\n");
				return JS_FALSE;
			}
		} else {
			/* did this come in from VRML as a string, or did
			 * it get created in javascript? */
			if (param_isString) {
				_str = JS_ValueToString(cx, *argv);
				charString = JS_EncodeString(cx,_str);
				if (sscanf(charString, "%lf %lf %lf",
							&(pars[0]), &(pars[1]), &(pars[2])) != 3) {
					printf ("conversion problem in SFVec3dGeneric\n");
					return JS_FALSE;
				}
				/* printf ("past scan, %f %f %f\n",pars[0], pars[1],pars[2]);*/
				cc3[0] = pars[0];
				cc3[1] = pars[1];
				cc3[2] = pars[2];
				cc2 = cc3;
			} else {
				if(SM_method() == 2){
					if (argv[0].isObject()) {
						AnyNative *_vec2;
        				if ((_vec2 = (AnyNative *)JS_GetPrivateFw(cx, JSVAL_TO_OBJECT(argv[0]))) == NULL) {
							printf("in SFVec3d, RHS was NOT native type \n");
							return JS_FALSE;
        				}
						cc2 = _vec2->v->sfvec3d.c;
					}else{
						return JS_FALSE;
					}
				}else{
					SFVec3dNative *_vec2;
					if (!JS_ConvertArguments(cx, argc, argv, "o", &_paramObj)) {
						printf( "JS_ConvertArguments failed in SFVec3d.\n");
						return JS_FALSE;
					}

					CHECK_CLASS(cx,_paramObj,argv,__FUNCTION__,SFVec3dClass)

					/* get the second object's data */
					if ((_vec2 = (SFVec3dNative*)JS_GetPrivateFw(cx, _paramObj)) == NULL) {
						printf( "JS_GetPrivate failed for _paramObj in SFVec3d.\n");
						return JS_FALSE;
					}
					cc2 = (_vec2->v).c;
				}
			}
		}
	}


	/* get our values */
	if(SM_method() == 2){
		AnyNative *_vec1;
		if ((_vec1 = (AnyNative*)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed for obj in SFVec3dAdd.\n");
			return JS_FALSE;
		}
		cc = _vec1->v->sfvec3d.c;
	}else{
		SFVec3dNative *_vec1;
		if ((_vec1 = (SFVec3dNative*)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed for obj in SFVec3dAdd.\n");
			return JS_FALSE;
		}
		cc = _vec1->v.c;
	}
	/* do the operation */
	#ifdef JSVRMLCLASSESVERBOSE
	printf ("SFVec3d generic, vec2 %f %f %f\n",pars[0],pars[1],pars[2]);
	#endif

	switch (op) {
		/* returning a SFVec3d */
		case __3FADD:
			vecaddd(cclhs,cc,cc2);
			break;
		case __3FDIVIDE:
			vecscaled(cclhs,cc,1.0/d);
			break;
		case __3FMULT:
			vecscaled(cclhs,cc,d);
			break;
		case __3FSUBT:
			vecdifd(cclhs,cc,cc2);
			break;
		case __3FDOT:
			d = vecdotd(cc,cc2);
			break;
		case __3FCROSS:
			veccrossd(cclhs,cc,cc2);
			break;
		case __3FLENGTH:
			d = veclengthd(cc);
			break;
		case __3FNORMALIZE:
			vecnormald(cclhs,cc);
			break;
		case __3FNEGATE:
			vecnegated(cclhs,cc);
			break;
		default:
			printf ("woops... %d\n",op);
		return JS_FALSE;
	}

	#ifdef JSVRMLCLASSESVERBOSE
	printf ("past calcs\n");
	#endif

	/* set the return object */
	if (retSFVec3d) {
		#ifdef JSVRMLCLASSESVERBOSE
		printf ("returning SFVec3d\n");
		#endif
		if ((_proto = JS_GetPrototypeFw(cx, obj)) == NULL) {
			printf( "JS_GetPrototype failed in SFVec3d.\n");
			return JS_FALSE;
		}
		if ((_retObj =
			JS_ConstructObjectFw(cx, &SFVec3dClass, _proto, NULL)) == NULL) {
			printf( "JS_ConstructObject failed in SFVec3d.\n");
			return JS_FALSE;
		}
		*rval = OBJECT_TO_JSVAL(_retObj);
		if(SM_method() == 2){
			AnyNative *any;
			if ((any = (AnyNative*)JS_GetPrivateFw(cx, _retObj)) == NULL) {
				printf( "JS_GetPrivate failed for _retObj in SFVec3d.\n");
				return JS_FALSE;
			}
			memcpy(any->v->sfvec3d.c,cclhs,sizeof(double)*3);
		}else{
			SFVec3dNative *_retNative;
			if ((_retNative = (SFVec3dNative*)JS_GetPrivateFw(cx, _retObj)) == NULL) {
				printf( "JS_GetPrivate failed for _retObj in SFVec3d.\n");
				return JS_FALSE;
			}
			memcpy(_retNative->v.c,cclhs,3*sizeof(double));
		}
	} else if (retNumeric) {
		if (JS_NewNumberValue(cx,d,rval) == JS_FALSE) {
			printf( "JS_NewDouble failed for %f in SFVec3d.\n",d);
			return JS_FALSE;
		}
	}
	#ifdef JSVRMLCLASSESVERBOSE
	if (retSFVec3d){
		printf("SFVec3dgeneric: obj = %p, result = [%.9g, %.9g, %.9g]\n",
			   obj,
			   (_retNative->v).c[0], (_retNative->v).c[1],
			   (_retNative->v).c[2]);
	}
	if (retNumeric){
		printf("SFVec2dgeneric: obj = %p, result = %.9g\n",
			   obj, d);
	}
	#endif

return JS_TRUE;
}

JSBool
SFVec3dAdd(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;
        JSBool retval = SFVec3dGeneric(cx, obj, argc, argv, &rval, __3FADD);
        JS_SET_RVAL(cx,vp,rval);
        return retval;
}

JSBool
SFVec3dCross(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;
        JSBool retval = SFVec3dGeneric(cx, obj, argc, argv, &rval, __3FCROSS);
        JS_SET_RVAL(cx,vp,rval);
        return retval;
}

JSBool
SFVec3dDivide(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;
        JSBool retval = SFVec3dGeneric(cx, obj, argc, argv, &rval, __3FDIVIDE);
        JS_SET_RVAL(cx,vp,rval);
        return retval;
}

JSBool
SFVec3dDot(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;
        JSBool retval = SFVec3dGeneric(cx, obj, argc, argv, &rval, __3FDOT);
        JS_SET_RVAL(cx,vp,rval);
        return retval;
}

JSBool
SFVec3dLength(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;
        JSBool retval = SFVec3dGeneric(cx, obj, argc, argv, &rval, __3FLENGTH);
        JS_SET_RVAL(cx,vp,rval);
        return retval;
}


JSBool
SFVec3dMultiply(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;
        JSBool retval = SFVec3dGeneric(cx, obj, argc, argv, &rval, __3FMULT);
        JS_SET_RVAL(cx,vp,rval);
        return retval;
}


JSBool
SFVec3dNegate(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;
        JSBool retval = SFVec3dGeneric(cx, obj, argc, argv, &rval, __3FNEGATE);
        JS_SET_RVAL(cx,vp,rval);
        return retval;
}

JSBool
SFVec3dNormalize(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;
        JSBool retval = SFVec3dGeneric(cx, obj, argc, argv, &rval, __3FNORMALIZE);
        JS_SET_RVAL(cx,vp,rval);
        return retval;
}

JSBool
SFVec3dSubtract(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
        jsval rval;
        JSBool retval = SFVec3dGeneric(cx, obj, argc, argv, &rval, __3FSUBT);
        JS_SET_RVAL(cx,vp,rval);
        return retval;
}

JSBool
SFVec3dToString(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
    JSString *_str;
	char buff[STRING];
	double *cc;

	UNUSED(argc);
	UNUSED(argv);
	if(SM_method()==2){
	    AnyNative *ptr;
		if ((ptr = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFVec3dToString.\n");
			return JS_FALSE;
		}
		cc = ptr->v->sfvec3d.c;
	}else{
	    SFVec3dNative *ptr;
		if ((ptr = (SFVec3dNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFVec3dToString.\n");
			return JS_FALSE;
		}
		cc = ptr->v.c;
	}
	memset(buff, 0, STRING);
	sprintf(buff, "%.9g %.9g %.9g",
			cc[0], cc[1], cc[2]);
	_str = JS_NewStringCopyZ(cx, buff);
    JS_SET_RVAL(cx,vp,STRING_TO_JSVAL(_str));

	#ifdef JSVRMLCLASSESVERBOSE
		printf ("SFVec3dToString, string is :%s:\n",buff);
	#endif

    return JS_TRUE;
}

JSBool
SFVec3dAssign(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
	JSString *_id_jsstr;
    JSObject *_from_obj;

    char *_id_str;

	UNUSED(_id_str); // compiler warning mitigation


	#ifdef JSVRMLCLASSESVERBOSE
		printf ("start of SFVec3dAssign\n");
	#endif
	if(SM_method() == 2){
		AnyNative *lhs, *rhs;
		if ((lhs = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed for obj in SFVec3dAssign.\n");
			return JS_FALSE;
		}
		if (!(*vp).isObject())
			return JS_FALSE;
        if ((rhs = (AnyNative *)JS_GetPrivateFw(cx, JSVAL_TO_OBJECT(*vp))) == NULL) {
			printf("in setECMANative, RHS was NOT native type \n");
			return JS_FALSE;
        }
		AnyNativeAssign(lhs,rhs);

	}else{
	    SFVec3dNative *fptr, *ptr;

		if ((ptr = (SFVec3dNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed for obj in SFVec3dAssign.\n");
			return JS_FALSE;
		}

		CHECK_CLASS(cx,obj,argv,__FUNCTION__,SFVec3dClass)

		if (JS_ConvertArguments(cx, argc, argv, "oS", &_from_obj, &_id_jsstr) == JS_TRUE) {
			_id_str = JS_EncodeString(cx,_id_jsstr);
		} else {
			printf( "JS_ConvertArguments failed in SFVec3dAssign.\n");
			return JS_FALSE;
		}

		CHECK_CLASS(cx,_from_obj,argv,__FUNCTION__,SFVec3dClass)

		if ((fptr = (SFVec3dNative *)JS_GetPrivateFw(cx, _from_obj)) == NULL) {
			printf( "JS_GetPrivate failed for _from_obj in SFVec3dAssign.\n");
			return JS_FALSE;
		}
		#ifdef JSVRMLCLASSESVERBOSE
			printf("SFVec3dAssign: obj = %p, id = \"%s\", from = %p\n",
				   obj, _id_str, _from_obj);
		#endif

		SFVec3dNativeAssign(ptr, fptr);
	}

    JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(obj));

	#ifdef JSVRMLCLASSESVERBOSE
		printf ("end of SFVec3dAssign\n");
	#endif

    return JS_TRUE;
}

JSBool
SFVec3dConstr(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_NewObject(cx,&SFVec3dClass,NULL,NULL);
        jsval *argv = JS_ARGV(cx,vp);
	jsdouble pars[3];
	double *cc;

	#ifdef JSVRMLCLASSESVERBOSE
		printf ("start of SFVec3dConstr\n");
	#endif

	ADD_ROOT(cx,obj)

	if(SM_method() == 2){
		AnyNative *any;
		if ((any = (AnyNative *) AnyNativeNew(FIELDTYPE_SFVec3d,NULL,NULL)) == NULL) {
			printf( "SFVec3dNativeNew failed in SFVec3dConstr.\n");
			return JS_FALSE;
		}

		if (!JS_SetPrivateFw(cx, obj, any)) {
			printf( "JS_SetPrivate failed in SFVec3dConstr.\n");
			return JS_FALSE;
		}
		cc = any->v->sfvec3d.c;
	}else{
		SFVec3dNative *ptr;
		if ((ptr = (SFVec3dNative *) SFVec3dNativeNew()) == NULL) {
			printf( "SFVec3dNativeNew failed in SFVec3dConstr.\n");
			return JS_FALSE;
		}

		//if (!JS_DefineProperties(cx, obj, SFVec3dProperties)) {
		//	printf( "JS_DefineProperties failed in SFVec3dConstr.\n");
		//	return JS_FALSE;
		//}
		if (!JS_SetPrivateFw(cx, obj, ptr)) {
			printf( "JS_SetPrivate failed in SFVec3dConstr.\n");
			return JS_FALSE;
		}
		cc = ptr->v.c;
		ptr->valueChanged = 1;
	}
	if (argc == 0) {
		cc[0] = (float) 0.0;
		cc[1] = (float) 0.0;
		cc[2] = (float) 0.0;
	} else {
		if (!JS_ConvertArguments(cx, argc, argv, "d d d",
				 &(pars[0]), &(pars[1]), &(pars[2]))) {
			printf( "JS_ConvertArguments failed in SFVec3dConstr.\n");
			return JS_FALSE;
		}
		cc[0] = (float) pars[0];
		cc[1] = (float) pars[1];
		cc[2] = (float) pars[2];
	}
	#ifdef JSVRMLCLASSESVERBOSE
		printf("SFVec3dConstr: obj = %p, %u args, %f %f %f\n",
			   obj, argc,
			   (ptr->v).c[0], (ptr->v).c[1], (ptr->v).c[2]);
	#endif
	

	JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(obj));
	return JS_TRUE;
}

JSBool
SFVec3dGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid,  JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();
	double *cc;
	jsdouble d;
	#ifdef JSVRMLCLASSESVERBOSE
	JSString *_idStr;
	char *_id_c;
	#endif

	jsval id;
	if (!JS_IdToValue(cx,iid,&id)) {
		printf("JS_IdToValue failed in SFVec3dGetProperty.\n");
		return JS_FALSE;
	}


	#ifdef JSVRMLCLASSESVERBOSE


/* same as earlier, these are never used
	_idStr = JS_ValueToString(cx, id);
	_id_c = JS_GetStringBytes(_idStr); */
	_idStr = JS_ValueToString(cx, *vp);
#if JS_VERSION < 185
	_id_c = JS_GetStringBytes(_idStr);
#else
	_id_c = JS_EncodeString(cx,_idStr);
#endif
	#endif

	if(SM_method()==2){
		AnyNative *ptr;
		if ((ptr = (AnyNative *)JS_GetPrivateFw(cx,obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFVec3dGetProperty.\n");
			return JS_FALSE;
		}
		cc = ptr->v->sfvec3d.c;
	}else{
		SFVec3dNative *ptr;
		if ((ptr = (SFVec3dNative *)JS_GetPrivateFw(cx,obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFVec3dGetProperty.\n");
			return JS_FALSE;
		}
		cc = ptr->v.c;
	}
	if (JSVAL_IS_INT(id)) {
		switch (JSVAL_TO_INT(id)) {
		case 0:
			d = cc[0];
			if (JS_NewNumberValue(cx, d, vp) == JS_FALSE) {
				printf(
						"JS_NewDouble failed for %f in SFVec3dGetProperty.\n",
						d);
				return JS_FALSE;
			}
			break;
		case 1:
			d = cc[1];
			if (JS_NewNumberValue(cx, d, vp) == JS_FALSE) {
				printf(
						"JS_NewDouble failed for %f in SFVec3dGetProperty.\n",
						d);
				return JS_FALSE;
			}
			break;
		case 2:
			d = cc[2];
			if (JS_NewNumberValue(cx, d, vp) == JS_FALSE) {
				printf(
						"JS_NewDouble failed for %f in SFVec3dGetProperty.\n",
						d);
				return JS_FALSE;
			}
			break;
		}
	} else {
		#ifdef JSVRMLCLASSESVERBOSE
			printf ("SFVec3dGetProperty, id is NOT an int...\n");
		#endif
	}

	return JS_TRUE;
}

JSBool
SFVec3dSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();

	double *cc;
	jsval myv;

	jsval id;
	if (!JS_IdToValue(cx,iid,&id)) {
		printf("JS_IdToValue failed in SFVec3dSetProperty.\n");
		return JS_FALSE;
	}


	if(SM_method()==2){
		AnyNative *ptr;
		if ((ptr = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFVec3dSetProperty.\n");
			return JS_FALSE;
		}
		if(ptr->valueChanged)
			(*ptr->valueChanged)++;
		#ifdef JSVRMLCLASSESVERBOSE
			printf("SFVec3dSetProperty: obj = %p, id = %d, valueChanged = %d\n",
				   obj, JSVAL_TO_INT(id), ptr->valueChanged);
		#endif
		cc = ptr->v->sfvec3d.c;
	}else{
		SFVec3dNative *ptr;
		if ((ptr = (SFVec3dNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFVec3dSetProperty.\n");
			return JS_FALSE;
		}
		ptr->valueChanged++;
		#ifdef JSVRMLCLASSESVERBOSE
			printf("SFVec3dSetProperty: obj = %p, id = %d, valueChanged = %d\n",
				   obj, JSVAL_TO_INT(id), ptr->valueChanged);
		#endif
		cc = ptr->v.c;
	}
	if (!JS_ConvertValue(cx, *vp, JSTYPE_NUMBER, &myv)) {
		printf( "JS_ConvertValue failed in SFVec3dSetProperty.\n");
		return JS_FALSE;
	}

	if (JSVAL_IS_INT(id)) {
		switch (JSVAL_TO_INT(id)) {
		case 0:
			cc[0] = JSVAL_TO_DOUBLE(myv);
			break;
		case 1:
			cc[1] = JSVAL_TO_DOUBLE(myv);
			break;
		case 2:
			cc[2] = JSVAL_TO_DOUBLE(myv);
			break;
		}
	}
	return JS_TRUE;
}


JSBool
SFVec4fToString(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
    JSString *_str;
	char buff[STRING];
	float *cc;

	UNUSED(argc);
	UNUSED(argv);
	if(SM_method()==2){
	    AnyNative *ptr;
		if ((ptr = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFVec4fToString.\n");
			return JS_FALSE;
		}
		cc = ptr->v->sfvec4f.c;

	}else{
	    SFVec4fNative *ptr;
		if ((ptr = (SFVec4fNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFVec4fToString.\n");
			return JS_FALSE;
		}
		cc = ptr->v.c;
	}
	memset(buff, 0, STRING);
	sprintf(buff, "%.9g %.9g %.9g %.9g",
			cc[0], cc[1], cc[2],cc[3]);
	_str = JS_NewStringCopyZ(cx, buff);
	JS_SET_RVAL(cx,vp,STRING_TO_JSVAL(_str));

	#ifdef JSVRMLCLASSESVERBOSE
		printf ("SFVec4fToString, string is :%s:\n",buff);
	#endif

    return JS_TRUE;
}

JSBool
SFVec4fAssign(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
	JSString *_id_jsstr;
    JSObject *_from_obj;
    char *_id_str;

	UNUSED(_id_str); // compiler warning mitigation


	#ifdef JSVRMLCLASSESVERBOSE
		printf ("start of SFVec4fAssign\n");
	#endif
	if(SM_method() == 2){
		AnyNative *lhs, *rhs;
		if ((lhs = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed for obj in SFVec4fAssign.\n");
			return JS_FALSE;
		}
		if (!(*vp).isObject())
			return JS_FALSE;
        if ((rhs = (AnyNative *)JS_GetPrivateFw(cx, JSVAL_TO_OBJECT(*vp))) == NULL) {
			printf("JS_ConvertArguments failed in SFVec4fAssign. \n");
			return JS_FALSE;
        }
		AnyNativeAssign(lhs,rhs);
	}else{
		SFVec4fNative *fptr, *ptr;

		if ((ptr = (SFVec4fNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed for obj in SFVec4fAssign.\n");
			return JS_FALSE;
		}

		CHECK_CLASS(cx,obj,argv,__FUNCTION__,SFVec4fClass)

		if (JS_ConvertArguments(cx, argc, argv, "oS", &_from_obj, &_id_jsstr) == JS_TRUE) {
			_id_str = JS_EncodeString(cx,_id_jsstr);
		} else {
			printf( "JS_ConvertArguments failed in SFVec4fAssign.\n");
			return JS_FALSE;
		}

		CHECK_CLASS(cx,_from_obj,argv,__FUNCTION__,SFVec4fClass)

		if ((fptr = (SFVec4fNative *)JS_GetPrivateFw(cx, _from_obj)) == NULL) {
			printf( "JS_GetPrivate failed for _from_obj in SFVec4fAssign.\n");
			return JS_FALSE;
		}
		#ifdef JSVRMLCLASSESVERBOSE
			printf("SFVec4fAssign: obj = %p, id = \"%s\", from = %p\n",
				   obj, _id_str, _from_obj);
		#endif

		SFVec4fNativeAssign(ptr, fptr);
	}
	JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(obj));

	#ifdef JSVRMLCLASSESVERBOSE
		printf ("end of SFVec4fAssign\n");
	#endif

    return JS_TRUE;
}

JSBool
SFVec4fConstr(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_NewObject(cx,&SFVec4fClass,NULL,NULL);
        jsval *argv = JS_ARGV(cx,vp);
	jsdouble pars[4];
	float *cc;
	
	#ifdef JSVRMLCLASSESVERBOSE
		printf ("start of SFVec4fConstr\n");
	#endif

	ADD_ROOT(cx,obj)
	if(SM_method() == 2){
		AnyNative *any;
		if((any = (AnyNative*)AnyNativeNew(FIELDTYPE_SFVec4f,NULL,NULL)) == NULL){
			printf( "AnyfNativeNew failed in SFVec4fConstr.\n");
			return JS_FALSE;
		}
		if (!JS_SetPrivateFw(cx, obj, any)) {
			printf( "JS_SetPrivate failed in SFVec4fConstr.\n");
			return JS_FALSE;
		}
		cc = any->v->sfvec4f.c;
	}else{
		SFVec4fNative *ptr;
		if ((ptr = (SFVec4fNative *) SFVec4fNativeNew()) == NULL) {
			printf( "SFVec4fNativeNew failed in SFVec4fConstr.\n");
			return JS_FALSE;
		}
		ptr->valueChanged = 1;
		cc = ptr->v.c;
		//if (!JS_DefineProperties(cx, obj, SFVec4fProperties)) {
		//	printf( "JS_DefineProperties failed in SFVec4fConstr.\n");
		//	return JS_FALSE;
		//}
		if (!JS_SetPrivateFw(cx, obj, ptr)) {
			printf( "JS_SetPrivate failed in SFVec4fConstr.\n");
			return JS_FALSE;
		}
	}

	if (argc == 0) {
		cc[0] = (float) 0.0;
		cc[1] = (float) 0.0;
		cc[2] = (float) 0.0;
		cc[3] = (float) 0.0;
	} else {
		if (!JS_ConvertArguments(cx, argc, argv, "d d d d",
				 &(pars[0]), &(pars[1]), &(pars[2]), &(pars[3]))) {
			printf( "JS_ConvertArguments failed in SFVec4fConstr.\n");
			return JS_FALSE;
		}
		cc[0] = (float) pars[0];
		cc[1] = (float) pars[1];
		cc[2] = (float) pars[2];
		cc[3] = (float) pars[3];
	}
	#ifdef JSVRMLCLASSESVERBOSE
		printf("SFVec4fConstr: obj = %p, %u args, %f %f %f %f\n",
			   obj, argc,
			   cc[0], cc[1], cc[2], cc[3]);
	#endif
	
	JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(obj));
	return JS_TRUE;
}

JSBool
SFVec4fGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid,  JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();

	jsdouble d;
	float *cc;
	#ifdef JSVRMLCLASSESVERBOSE
	JSString *_idStr;
	char *_id_c;
	#endif

	jsval id;
	if (!JS_IdToValue(cx,iid,&id)) {
		printf("JS_IdToValue failed in SFVec4fGetProperty.\n");
		return JS_FALSE;
	}


	#ifdef JSVRMLCLASSESVERBOSE

	//JSString *_idStr;
	//char *_id_c;

/* same as above
	_idStr = JS_ValueToString(cx, id);
	_id_c = JS_GetStringBytes(_idStr); */
	_idStr = JS_ValueToString(cx, *vp);
#if JS_VERSION < 185
	_id_c = JS_GetStringBytes(_idStr);
#else
	_id_c = JS_EncodeString(cx,_idStr);
#endif
	#endif

	if(SM_method()==2){
		AnyNative *any;
		if ((any = (AnyNative *)JS_GetPrivateFw(cx,obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFVec4fGetProperty.\n");
			return JS_FALSE;
		}
		cc = any->v->sfvec4f.c;
	}else{
		SFVec4fNative *ptr;
		if ((ptr = (SFVec4fNative *)JS_GetPrivateFw(cx,obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFVec4fGetProperty.\n");
			return JS_FALSE;
		}
		cc = ptr->v.c;
	}
	if (JSVAL_IS_INT(id)) {
		switch (JSVAL_TO_INT(id)) {
		case 0:
			d = cc[0];
			if (JS_NewNumberValue(cx, d, vp) == JS_FALSE) {
				printf(
						"JS_NewDouble failed for %f in SFVec4fGetProperty.\n",
						d);
				return JS_FALSE;
			}
			break;
		case 1:
			d = cc[1];
			if (JS_NewNumberValue(cx, d, vp) == JS_FALSE) {
				printf(
						"JS_NewDouble failed for %f in SFVec4fGetProperty.\n",
						d);
				return JS_FALSE;
			}
			break;
		case 2:
			d = cc[2];
			if (JS_NewNumberValue(cx, d, vp) == JS_FALSE) {
				printf(
						"JS_NewDouble failed for %f in SFVec4fGetProperty.\n",
						d);
				return JS_FALSE;
			}
			break;
		case 3:
			d = cc[3];
			if (JS_NewNumberValue(cx, d, vp) == JS_FALSE) {
				printf(
						"JS_NewDouble failed for %f in SFVec4fGetProperty.\n",
						d);
				return JS_FALSE;
			}
			break;
		}
	} else {
		#ifdef JSVRMLCLASSESVERBOSE
			printf ("SFVec4fGetProperty, id is NOT an int...\n");
		#endif
	}

	return JS_TRUE;
}

JSBool
SFVec4fSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();

	jsval myv;
	float *cc;


	jsval id;
	if (!JS_IdToValue(cx,iid,&id)) {
		printf("JS_IdToValue failed in SFVec4fSetProperty.\n");
		return JS_FALSE;
	}

	if(SM_method() == 2){
		AnyNative *any;
		if ((any = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFVec4fSetProperty.\n");
			return JS_FALSE;
		}
		if(any->valueChanged)
			(*any->valueChanged)++;
		cc = any->v->sfvec4f.c;
	}else{
		SFVec4fNative *ptr;

		if ((ptr = (SFVec4fNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFVec4fSetProperty.\n");
			return JS_FALSE;
		}
		ptr->valueChanged++;
		#ifdef JSVRMLCLASSESVERBOSE
			printf("SFVec4fSetProperty: obj = %p, id = %d, valueChanged = %d\n",
				   obj, JSVAL_TO_INT(id), ptr->valueChanged);
		#endif
		cc = ptr->v.c;
	}
	if (!JS_ConvertValue(cx, *vp, JSTYPE_NUMBER, &myv)) {
		printf( "JS_ConvertValue failed in SFVec4fSetProperty.\n");
		return JS_FALSE;
	}

	if (JSVAL_IS_INT(id)) {
		switch (JSVAL_TO_INT(id)) {
		case 0:
			cc[0] = (float) JSVAL_TO_DOUBLE(myv);
			break;
		case 1:
			cc[1] = (float) JSVAL_TO_DOUBLE(myv);
			break;
		case 2:
			cc[2] = (float) JSVAL_TO_DOUBLE(myv);
			break;
		case 3:
			cc[3] = (float) JSVAL_TO_DOUBLE(myv);
			break;
		}
	}
	return JS_TRUE;
}



JSBool
SFVec4dToString(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);

    JSString *_str;
	double *cc;
	char buff[STRING];

	UNUSED(argc);
	UNUSED(argv);
	if(SM_method()==2){
	    AnyNative *ptr;
		if ((ptr = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFVec4dToString.\n");
			return JS_FALSE;
		}
		cc = ptr->v->sfvec4d.c;
	}else{
	    SFVec4dNative *ptr;
		if ((ptr = (SFVec4dNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFVec4dToString.\n");
			return JS_FALSE;
		}
		cc = ptr->v.c;
	}
	memset(buff, 0, STRING);
	sprintf(buff, "%.9g %.9g %.9g %.9g",
			cc[0], cc[1], cc[2],cc[3]);
	_str = JS_NewStringCopyZ(cx, buff);

	JS_SET_RVAL(cx,vp,STRING_TO_JSVAL(_str));

	#ifdef JSVRMLCLASSESVERBOSE
		printf ("SFVec4dToString, string is :%s:\n",buff);
	#endif

    return JS_TRUE;
}

JSBool
SFVec4dAssign(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(cx,vp);
        jsval *argv = JS_ARGV(cx,vp);
	JSString *_id_jsstr;
    JSObject *_from_obj;
    char *_id_str;

	UNUSED(_id_str); // compiler warning mitigation

	#ifdef JSVRMLCLASSESVERBOSE
		printf ("start of SFVec4dAssign\n");
	#endif

	if(SM_method() == 2){
		AnyNative *lhs, *rhs;
		if ((lhs = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed for obj in SFVec4dAssign.\n");
			return JS_FALSE;
		}
		if (!(*vp).isObject())
			return JS_FALSE;
        if ((rhs = (AnyNative *)JS_GetPrivateFw(cx, JSVAL_TO_OBJECT(*vp))) == NULL) {
			printf("in SFVec4dAssign, RHS was NOT native type \n");
			return JS_FALSE;
        }
		AnyNativeAssign(lhs,rhs);

	}else{
		SFVec4dNative *fptr, *ptr;

		if ((ptr = (SFVec4dNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed for obj in SFVec4dAssign.\n");
			return JS_FALSE;
		}

		CHECK_CLASS(cx,obj,argv,__FUNCTION__,SFVec4dClass)

		if (JS_ConvertArguments(cx, argc, argv, "oS", &_from_obj, &_id_jsstr) == JS_TRUE) {
			_id_str = JS_EncodeString(cx,_id_jsstr);
		} else {
			printf( "JS_ConvertArguments failed in SFVec4dAssign.\n");
			return JS_FALSE;
		}

		CHECK_CLASS(cx,_from_obj,argv,__FUNCTION__,SFVec4dClass)

		if ((fptr = (SFVec4dNative *)JS_GetPrivateFw(cx, _from_obj)) == NULL) {
			printf( "JS_GetPrivate failed for _from_obj in SFVec4dAssign.\n");
			return JS_FALSE;
		}
		#ifdef JSVRMLCLASSESVERBOSE
			printf("SFVec4dAssign: obj = %p, id = \"%s\", from = %p\n",
				   obj, _id_str, _from_obj);
		#endif

		SFVec4dNativeAssign(ptr, fptr);
	}
	JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(obj));

	#ifdef JSVRMLCLASSESVERBOSE
		printf ("end of SFVec4dAssign\n");
	#endif

    return JS_TRUE;
}

JSBool
SFVec4dConstr(JSContext *cx, uintN argc, jsval *vp) {
        JSObject *obj = JS_NewObject(cx,&SFVec4dClass,NULL,NULL);
        jsval *argv = JS_ARGV(cx,vp);
	jsdouble pars[4];
	double *cc;
	
	#ifdef JSVRMLCLASSESVERBOSE
		printf ("start of SFVec4dConstr\n");
	#endif

	ADD_ROOT(cx,obj)
	if(SM_method() == 2){
		AnyNative *any;
		if ((any = (AnyNative *) AnyNativeNew(FIELDTYPE_SFVec4d,NULL,NULL)) == NULL) {
			printf( "SFVec3dNativeNew failed in SFVec3dConstr.\n");
			return JS_FALSE;
		}

		if (!JS_SetPrivateFw(cx, obj, any)) {
			printf( "JS_SetPrivate failed in SFVec3dConstr.\n");
			return JS_FALSE;
		}
		cc = any->v->sfvec4d.c;
	}else{
		SFVec4dNative *ptr;

		if ((ptr = (SFVec4dNative *) SFVec4dNativeNew()) == NULL) {
			printf( "SFVec4dNativeNew failed in SFVec4dConstr.\n");
			return JS_FALSE;
		}

		//if (!JS_DefineProperties(cx, obj, SFVec4dProperties)) {
		//	printf( "JS_DefineProperties failed in SFVec4dConstr.\n");
		//	return JS_FALSE;
		//}
		if (!JS_SetPrivateFw(cx, obj, ptr)) {
			printf( "JS_SetPrivate failed in SFVec4dConstr.\n");
			return JS_FALSE;
		}
		ptr->valueChanged = 1;
		cc = ptr->v.c;
	}
	if (argc == 0) {
		cc[0] = (float) 0.0;
		cc[1] = (float) 0.0;
		cc[2] = (float) 0.0;
		cc[3] = (float) 0.0;
	} else {
		if (!JS_ConvertArguments(cx, argc, argv, "d d d d",
				 &(pars[0]), &(pars[1]), &(pars[2]), &(pars[3]))) {
			printf( "JS_ConvertArguments failed in SFVec4dConstr.\n");
			return JS_FALSE;
		}
		cc[0] = (float) pars[0];
		cc[1] = (float) pars[1];
		cc[2] = (float) pars[2];
		cc[3] = (float) pars[3];
	}
	#ifdef JSVRMLCLASSESVERBOSE
		printf("SFVec4dConstr: obj = %p, %u args, %f %f %f %f\n",
			   obj, argc,
			   cc[0], cc[1], cc[2], cc[3]);
	#endif
	

	JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(obj));
	return JS_TRUE;
}

JSBool
SFVec4dGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid,  JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();

	jsdouble d;
	double *cc;
	#ifdef JSVRMLCLASSESVERBOSE
	JSString *_idStr;
	char *_id_c;
	#endif

	jsval id;
	if (!JS_IdToValue(cx,iid,&id)) {
		printf("JS_IdToValue failed in SFVec4dGetProperty.\n");
		return JS_FALSE;
	}


	#ifdef JSVRMLCLASSESVERBOSE

/*	_idStr = JS_ValueToString(cx, id);
	_id_c = JS_GetStringBytes(_idStr);*/
	_idStr = JS_ValueToString(cx, *vp);
#if JS_VERSION < 185
	_id_c = JS_GetStringBytes(_idStr);
#else
	_id_c = JS_EncodeString(cx,_idStr);
#endif
	#endif

	if(SM_method()==2){
		AnyNative *ptr;
		if ((ptr = (AnyNative *)JS_GetPrivateFw(cx,obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFVec4dGetProperty.\n");
			return JS_FALSE;
		}
		cc = ptr->v->sfvec4d.c;
	}else{
	SFVec4dNative *ptr;
		if ((ptr = (SFVec4dNative *)JS_GetPrivateFw(cx,obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFVec4dGetProperty.\n");
			return JS_FALSE;
		}
		cc = ptr->v.c;
	}
	if (JSVAL_IS_INT(id)) {
		switch (JSVAL_TO_INT(id)) {
		case 0:
			d = cc[0];
			if (JS_NewNumberValue(cx, d, vp) == JS_FALSE) {
				printf(
						"JS_NewDouble failed for %f in SFVec4dGetProperty.\n",
						d);
				return JS_FALSE;
			}
			break;
		case 1:
			d = cc[1];
			if (JS_NewNumberValue(cx, d, vp) == JS_FALSE) {
				printf(
						"JS_NewDouble failed for %f in SFVec4dGetProperty.\n",
						d);
				return JS_FALSE;
			}
			break;
		case 2:
			d = cc[2];
			if (JS_NewNumberValue(cx, d, vp) == JS_FALSE) {
				printf(
						"JS_NewDouble failed for %f in SFVec4dGetProperty.\n",
						d);
				return JS_FALSE;
			}
			break;
		case 3:
			d = cc[3];
			if (JS_NewNumberValue(cx, d, vp) == JS_FALSE) {
				printf(
						"JS_NewDouble failed for %f in SFVec4dGetProperty.\n",
						d);
				return JS_FALSE;
			}
			break;
		}
	} else {
		#ifdef JSVRMLCLASSESVERBOSE
			printf ("SFVec4dGetProperty, id is NOT an int...\n");
		#endif
	}

	return JS_TRUE;
}

JSBool
SFVec4dSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();

	jsval myv;
	double *cc;


	jsval id;
	if (!JS_IdToValue(cx,iid,&id)) {
		printf("JS_IdToValue failed in SFVec4dSetProperty.\n");
		return JS_FALSE;
	}

	if(SM_method()==2){
		AnyNative *ptr;
		if ((ptr = (AnyNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFVec4dSetProperty.\n");
			return JS_FALSE;
		}
		if(ptr->valueChanged)
			(*ptr->valueChanged)++;
		#ifdef JSVRMLCLASSESVERBOSE
			printf("SFVec4dSetProperty: obj = %p, id = %d, valueChanged = %d\n",
				   obj, JSVAL_TO_INT(id), ptr->valueChanged);
		#endif
		cc = ptr->v->sfvec4d.c;
	}else{
		SFVec4dNative *ptr;

		if ((ptr = (SFVec4dNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in SFVec4dSetProperty.\n");
			return JS_FALSE;
		}
		ptr->valueChanged++;
		#ifdef JSVRMLCLASSESVERBOSE
			printf("SFVec4dSetProperty: obj = %p, id = %d, valueChanged = %d\n",
				   obj, JSVAL_TO_INT(id), ptr->valueChanged);
		#endif
		cc = ptr->v.c;
	}
	if (!JS_ConvertValue(cx, *vp, JSTYPE_NUMBER, &myv)) {
		printf( "JS_ConvertValue failed in SFVec4dSetProperty.\n");
		return JS_FALSE;
	}

	if (JSVAL_IS_INT(id)) {
		switch (JSVAL_TO_INT(id)) {
		case 0:
			cc[0] = (float) JSVAL_TO_DOUBLE(myv);
			break;
		case 1:
			cc[1] = (float) JSVAL_TO_DOUBLE(myv);
			break;
		case 2:
			cc[2] = (float) JSVAL_TO_DOUBLE(myv);
			break;
		case 3:
			cc[3] = (float) JSVAL_TO_DOUBLE(myv);
			break;
		}
	}
	return JS_TRUE;
}
#endif //defined(JS_SMCPP)
#endif //JAVASCRIPT_SM