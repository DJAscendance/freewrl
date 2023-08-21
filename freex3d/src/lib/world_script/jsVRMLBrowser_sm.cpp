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
#if defined(JS_SMCPP)
#ifdef JAVASCRIPT_SM
#undef DEBUG
//#define DEBUG 1 //challenge it with lots of ASSERTS, just for cleaning up code correctness, not production
# include <jsapi.h> /* JS compiler */
//# include <jsdbgapi.h> /* JS debugger */

#ifndef JS_VERSION
#define JS_VERSION 187
#endif
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
#define JSSCRIPT2 JSScript
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

#include <display.h>
#include <internal.h>

//#include <libFreeWRL.h>
//#include <list.h>
//
//#include "../vrml_parser/Structs.h"
#include "../vrml_parser/CRoutes.h"
#include "../opengl/OpenGL_Utils.h"
//#include "../main/headers.h"
#include "../main/ProdCon.h"
#include "../scenegraph/RenderFuncs.h"
//#include "../vrml_parser/CParseGeneral.h"
//#include "../scenegraph/Vector.h"
//#include "../vrml_parser/CFieldDecls.h"
//#include "../vrml_parser/CParseParser.h"
//#include "../vrml_parser/CParseLexer.h"
//#include "../vrml_parser/CParse.h"
//#include "../main/Snapshot.h"
//#include "../scenegraph/Collision.h"
//#include "../scenegraph/quaternion.h"
//#include "../scenegraph/Viewer.h"
void getCurrentSpeed();
//#include "../x3d_parser/Bindable.h"
//#include "../input/EAIHeaders.h"	/* for implicit declarations */
#include "../ui/common.h"


#include "JScript.h"
#include "CScripts.h"
#include "fieldSet.h"
#include "jsNative.h"

struct X3D_Anchor* get_EAIEventsIn_AnchorNode();

//ComonentInfo{
//String name;
//Numeric level;
//String Title;
//String providerUrl;
//}
int capabilitiesHandler_getTableLength(int* table);
int capabilitiesHandler_getComponentLevel(int *table, int comp);
int capabilitiesHandler_getProfileLevel(int prof);
const int *capabilitiesHandler_getProfileComponent(int prof);
const int *capabilitiesHandler_getCapabilitiesTable();
typedef struct intTableIndex{
	int* table;
	int index;
} *IntTableIndex;

//X3DRoute{
//SFNode sourceNode;
//String sourceField;
//SFNode destinationNode;
//String destinationField;
//}
struct CRStruct *getCRoutes();
int getCRouteCount();

struct X3D_Node* broto_search_DEFname(struct X3D_Proto* context, const char* name);
struct X3D_Node* broto_search_ALLnames(struct X3D_Proto* context, const char* name, int* source);
void remove_node_from_parents_children(struct X3D_Node* node);
int remove_broto_node(struct X3D_Proto* context, struct X3D_Node* node);
void remove_node_from_def_list(struct X3D_Proto* ec, struct X3D_Node* node, char* defname) {
	if (ec->__DEFnames) {
		struct brotoDefpair* bd;
		for (int i = 0; i < vectorSize((Vector*)ec->__DEFnames); i++) {
			bd = vector_get_ptr(struct brotoDefpair, (Vector*)ec->__DEFnames, i);
			if (!strcmp(bd->name, defname)) {
				node = bd->node;
				//remove DEF name mapping:
				vector_remove_elem(struct brotoDefpair, (Vector*)ec->__DEFnames, i);
				break;
			}
		}
	}

}
void* addDeleteRoute0(void* ec, const char* callingFunc, struct X3D_Node* fromNode, const char* sfromField, struct X3D_Node* toNode, const char* stoField);
void update_weakRoutes(struct X3D_Proto* context); 

} //extern "C"

#include "jsUtils_sm.h"
#include "jsVRMLClasses_sm.h"
#include "jsVRMLBrowser_sm.h"

// the JSVAL_IS_INT wasn't giving me the tinyid for switch-casing on property like it used to
// this function will take the string field name and get the tinyid, so old switch-case can continue
int lookup_tinyid(char* fieldname, JSPropertySpec* properties) {
	JSPropertySpec* p = &properties[0];
	int i = 0;
	int index = -1;
	while (p->name) {
		if (!strcmp(p->name, fieldname)) {
			index = p->tinyid;
			break;
		}
		i++;
		p = &properties[i];
	}
	return index;
}

#define X3DBROWSER 1



#ifndef X3DBROWSER
#define SetPropertyStub JS_StrictPropertyStub
#endif // ndef X3DBROWSER

#ifdef X3DBROWSER
JSBool
BrowserGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp);

JSBool
BrowserSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp);
#endif
int jsrrunScript(JSContext *_context, JSObject *_globalObj, char *script, jsval *rval);
/* 
//js 17 aka 186
struct JSClass {
    const char          *name;
    uint32_t            flags;

    // Mandatory non-null function pointer members. 
    JSPropertyOp        addProperty;
    JSPropertyOp        delProperty;
    JSPropertyOp        getProperty;
    JSStrictPropertyOp  setProperty;
    JSEnumerateOp       enumerate;
    JSResolveOp         resolve;
    JSConvertOp         convert;
    JSFinalizeOp        finalize;

    // Optionally non-null members start here. 
    JSCheckAccessOp     checkAccess;
    JSNative            call;
    JSHasInstanceOp     hasInstance; // +
    JSNative            construct;
    JSTraceOp           trace;

    void                *reserved[40];
};
//js 185
struct JSClass {
    const char          *name;
    uint32              flags;

    // Mandatory non-null function pointer members. 
    JSPropertyOp        addProperty;
    JSPropertyOp        delProperty;
    JSPropertyOp        getProperty;
    JSStrictPropertyOp  setProperty;
    JSEnumerateOp       enumerate;
    JSResolveOp         resolve;
    JSConvertOp         convert;
    JSFinalizeOp        finalize;

    // Optionally non-null members start here. 
    JSClassInternal     reserved0;   // -
    JSCheckAccessOp     checkAccess;
    JSNative            call;
    JSNative            construct;
    JSXDRObjectOp       xdrObject; //-
    JSHasInstanceOp     hasInstance; //changed place with construct
    JSMarkOp            mark;  //changed from JSTraceOp

    JSClassInternal     reserved1;
    void                *reserved[19]; //-
};
*/

#if JS_VERSION < 187
#define JS_DeletePropertyStub JS_PropertyStub
#endif


//X3DConstants 
struct string_int {
	char* c;
	int i;
};

struct string_int lookup_X3DConstants[] = {
	{"INITIALIZED_EVENT",1},
	{"SHUTDOWN_EVENT",1},
	{"CONNECTION_ERROR",1},
	{"INITIALIZED_ERROR",1},
	{"NOT_STARTED_STATE",1},
	{"IN_PROGRESS_STATE",1},
	{"COMPLETE_STATE",1},
	{"FAILED_STATE",0},
	{"SFBool",FIELDTYPE_SFBool},
	{"MFBool",FIELDTYPE_MFBool},
	{"MFInt32",FIELDTYPE_MFInt32},
	{"SFInt32",FIELDTYPE_SFInt32},
	{"SFFloat",FIELDTYPE_SFFloat},
	{"MFFloat",FIELDTYPE_MFFloat},
	{"SFDouble",FIELDTYPE_SFDouble},
	{"MFDouble",FIELDTYPE_MFDouble},
	{"SFTime",FIELDTYPE_SFTime},
	{"MFTime",FIELDTYPE_MFTime},
	{"SFNode",FIELDTYPE_SFNode},
	{"MFNode",FIELDTYPE_MFNode},
	{"SFVec2f",FIELDTYPE_SFVec2f},
	{"MFVec2f",FIELDTYPE_MFVec2f},
	{"SFVec3f",FIELDTYPE_SFVec3f},
	{"MFVec3f",FIELDTYPE_MFVec3f},
	{"SFVec3d",FIELDTYPE_SFVec3d},
	{"MFVec3d",FIELDTYPE_MFVec3d},
	{"SFRotation",FIELDTYPE_SFRotation},
	{"MFRotation",FIELDTYPE_MFRotation},
	{"SFColor",FIELDTYPE_SFColor},
	{"MFColor",FIELDTYPE_MFColor},
	{"SFImage",FIELDTYPE_SFImage},
	//	{"MFImage",FIELDTYPE_MFImage},
		{"SFColorRGBA",FIELDTYPE_SFColorRGBA},
		{"MFColorRGBA",FIELDTYPE_MFColorRGBA},
		{"SFString",FIELDTYPE_SFString},
		{"MFString",FIELDTYPE_MFString},
		/*
			{"X3DBoundedObject",},
			{"X3DMetadataObject",},
			{"X3DUrlObject",},
			{"X3DTriggerNode",},
			{"X3DInfoNode",},
			{"X3DAppearanceNode",},
			{"X3DAppearanceChildNode",},
			{"X3DMaterialNode",},
			{"X3DTextureNode",},
			{"X3DTexture2DNode",},
			{"X3DTexture3DNode",},
			{"X3DTextureTransformNode",},
			{"X3DGeometryNode",},
			{"X3DGeometry3DNode",},
			{"X3DCoordinateNode",},
			{"X3DParametricGeometryNode",},
			{"X3DGeometricPropertyNode",},
			{"X3DColorNode",},
			{"X3DProtoInstance",},
			{"X3DNormalNode",},
			{"X3DTextureCoordinateNode",},
			{"X3DFontStyleNode",},
			{"X3DGroupingNode ",},
			{"X3DChildNode",},
			{"X3DBindableNode",},
			{"X3DBackgroundNode",},
			{"X3DInterpolatorNode",},
			{"X3DShapeNode",},
			{"X3DScriptNode",},
			{"X3DSensorNode",},
			{"X3DEnvironmentalSensorNode",},
			{"X3DLightNode",},
			{"X3DNetworkSensorNode",},
			{"X3DPointingDeviceSensorNode",},
			{"X3DDragSensorNode",},
			{"X3DKeyDeviceSensorNode",},
			{"X3DSequencerNode",},
			{"X3DTimeDependentNode",},
			{"X3DSoundNode",},
			{"X3DSoundSourceNode",},
			{"X3DTouchSensorNode",},
		*/
			{"inputOnly",PKW_inputOnly},
			{"outputOnly",PKW_outputOnly},
			{"inputOutput",PKW_inputOutput},
			{"initializeOnly",PKW_initializeOnly},
			{NULL,0}
};

struct string_int* lookup_string_int(struct string_int* table, const char* searchkey, int* index) {
	int i;
	//struct string_int *retval = NULL;
	*index = -1;
	if (!table) return NULL;
	i = 0;
	while (table[i].c) {
		if (!strcmp(table[i].c, searchkey)) {
			//found it
			(*index) = i;
			return &table[i];
		}
		i++;
	}
	return NULL;
}
JSBool
X3DConstantsGetProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp) {
	JSObject* obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval* vp = hvp.address();

	int index;
	JSString* _str;
	char* str;
	jsval rval;
	jsval id;

	if (!JS_IdToValue(cx, iid, &id)) {
		printf("JS_IdToValue failed in X3DRouteGetProperty.\n");
		return JS_FALSE;
	}
	if (JSVAL_IS_STRING(id)) {
		str = (char*)JS_EncodeString(cx, JSVAL_TO_STRING(id));;
		string_int* iret = lookup_string_int(lookup_X3DConstants, str, &index);
		rval = INT_TO_JSVAL(iret->i);
		JS_SET_RVAL(cx, vp, rval);
		return JS_TRUE;
	}
	return JS_FALSE;
}


int len_constants() {
	int len = (sizeof(lookup_X3DConstants) / sizeof(struct string_int)) - 1;
	return len;
}

static JSClass X3DConstantsClass = {
	"X3DConstants",
	JSCLASS_HAS_PRIVATE,
	JS_PropertyStub,
	JS_DeletePropertyStub,
	X3DConstantsGetProperty,
	JS_StrictPropertyStub,
	JS_EnumerateStub,
	JS_ResolveStub,
	JS_ConvertStub,
	JS_FinalizeStub
};

//fieldDefinition

static JSPropertySpec(FieldDefinitionProperties)[] = {
	{"name", 0, JSPROP_ENUMERATE}, //string
	{"accessType", 1, JSPROP_ENUMERATE}, //numeric / enumerated inputOnly..
	{"dataType", 2, JSPROP_ENUMERATE}, //numeric / enumerated SFBool etc
	{0}
};

JSBool
FieldDefinitionGetProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp) {
	JSObject* obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval* vp = hvp.address();

	struct ProtoFieldDecl* ptr;
	int _index;
	JSString* _str;
	jsval rval;
	jsval id;

	UNUSED(rval); // compiler warning mitigation


	if (!JS_IdToValue(cx, iid, &id)) {
		printf("JS_IdToValue failed in X3DRouteGetProperty.\n");
		return JS_FALSE;
	}

	if ((ptr = (struct ProtoFieldDecl*)JS_GetPrivateFw(cx, obj)) == NULL) {
		printf("JS_GetPrivate failed in FieldDefinitionGetProperty.\n");
		return JS_FALSE;
	}
	struct X3D_Proto* ec = (struct X3D_Proto*)JS_GetContextPrivate(cx);
	

	int index = -1;
	if (JSVAL_IS_INT(id))
		index = JSVAL_TO_INT(id);
	else
		index = lookup_tinyid(JS_EncodeString(cx, JSVAL_TO_STRING(id)), FieldDefinitionProperties);
	switch (index) {
	case 0://name (string)
	{
		JSString* _str;
		_str = JS_NewStringCopyZ(cx,ptr->cname);
		rval = STRING_TO_JSVAL(_str);

		JS_SET_RVAL(cx, vp, rval);
		break;
	}
	case 1://accessType enumerant ie inputOnly
	{

		rval = INT_TO_JSVAL(ptr->mode);
		JS_SET_RVAL(cx, vp, rval);

		break;
	}
	case 2://dataType enumerant ie SFBool
	{
		rval = INT_TO_JSVAL(ptr->type);
		JS_SET_RVAL(cx, vp, rval);
		break;
	}
	}

	return JS_TRUE;
}
JSBool
FieldDefinitionSetProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp) {
	JSObject* obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval* vp = hvp.address();
	//can I, should I force it to read-only this way?
	return JS_FALSE;
}

static JSClass FieldDefinitionClass = {
	"FieldDefinition",
	JSCLASS_HAS_PRIVATE,
	JS_PropertyStub,
	JS_DeletePropertyStub,
	FieldDefinitionGetProperty,
	FieldDefinitionSetProperty,
	JS_EnumerateStub,
	JS_ResolveStub,
	JS_ConvertStub,
	JS_FinalizeStub
};

//fieldDefinitionArray

static JSPropertySpec(FieldDefinitionArrayProperties)[] = {
	{"length", -1, JSPROP_READONLY | JSPROP_SHARED | JSPROP_PERMANENT}, //JSPROP_ENUMERATE},
	{0}
};

JSBool
FieldDefinitionArrayGetProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp) {
	JSObject* obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval* vp = hvp.address();

	struct ProtoDefinition* pd;
	jsval rval;
	jsval id;

	UNUSED(rval); // compiler warning mitigation


	if (!JS_IdToValue(cx, iid, &id)) {
		printf("JS_IdToValue failed in FieldDefinitionArrayGetProperty.\n");
		return JS_FALSE;
	}

	if ((pd = (struct ProtoDefinition*)JS_GetPrivateFw(cx, obj)) == NULL) {
		printf("JS_GetPrivate failed in ProtoDeclarationArrayGetProperty.\n");
		return JS_FALSE;
	}
	struct X3D_Proto* ec = (struct X3D_Proto*)JS_GetContextPrivate(cx);

	int index = -1;
	if (JSVAL_IS_INT(id))
		index = JSVAL_TO_INT(id);
	else
		index = lookup_tinyid(JS_EncodeString(cx, JSVAL_TO_STRING(id)), FieldDefinitionArrayProperties);
	if (index == -1) {
		int _length = vectorSize(pd->iface); 
		JS_SET_RVAL(cx, vp, INT_TO_JSVAL(_length));
	}
	else if (index > -1 && index < vectorSize(pd->iface))
	{
		JSObject* _obj;
		struct ProtoFieldDecl* pfield = vector_get(struct ProtoFieldDecl*, pd->iface, index);

		_obj = JS_NewObject(cx, &FieldDefinitionClass, NULL, obj);
		if (0) if (!JS_DefineProperties(cx, _obj, FieldDefinitionProperties)) {
			printf("JS_DefineProperties failed in FieldDefinitionProperties.\n");
			return JS_FALSE;
		}

		if (!JS_SetPrivateFw(cx, _obj, (void*)pfield)) {
			printf("JS_SetPrivate failed in FieldDefinitionArray.\n");
			return JS_FALSE;
		}

		JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(_obj));

	}

	return JS_TRUE;
}
JSBool
FieldDefinitionArraySetProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp) {
	JSObject* obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval* vp = hvp.address();
	//can I, should I force it to read-only this way?
	return JS_FALSE;
}


static JSClass FieldDefinitionArrayClass = {
	"FieldDefinitionArray",
	JSCLASS_HAS_PRIVATE,
	JS_PropertyStub,
	JS_DeletePropertyStub,
	FieldDefinitionArrayGetProperty,
	FieldDefinitionArraySetProperty,
	JS_EnumerateStub,
	JS_ResolveStub,
	JS_ConvertStub,
	JS_FinalizeStub
};


//ProtoDeclaration
JSBool
ProtoDeclaration_newInstance(JSContext* cx, uintN argc, jsval* vp) {
	JSObject* obj = JS_THIS_OBJECT(cx, vp);
	jsval* argv = JS_ARGV(cx, vp);
	jsval rval;

	UNUSED(argc);
	UNUSED(argv);


	struct X3D_Proto* ptr;
	char str[200];
	JSString* _str;
	if ((ptr = (struct X3D_Proto*)JS_GetPrivateFw(cx, obj)) == NULL) {
		printf("in ProtoDeclaration_newInstance() - not a Native\n");
		return JS_FALSE;
	}
	struct X3D_Proto* ec = (struct X3D_Proto*)JS_GetContextPrivate(cx);
	struct X3D_Proto* proto = ptr;

	struct ProtoDefinition* pd;
	pd = (struct ProtoDefinition*)proto->__protoDef;

	struct X3D_Node* dest = X3D_NODE(brotoInstance(X3D_PROTO(X3D_PROTO(proto)->__prototype), ciflag_get(ec->__protoFlags, 0)));

	AnyNative* nany = MALLOC(AnyNative*, sizeof(AnyNative));
	memset(nany, 0, sizeof(AnyNative));
	nany->type = FIELDTYPE_SFNode;
	nany->v = MALLOC(union anyVrml*, sizeof(union anyVrml));
	memset(nany->v, 0, sizeof(union anyVrml));

	nany->v->sfnode = dest;

	JSObject* _obj = JS_NewObject(cx, &SFNodeClass, NULL, obj);
	if (0) if (!JS_DefineProperties(cx, _obj, SFNodeProperties)) {
		printf("JS_DefineProperties failed in Route sourceNode.\n");
		return JS_FALSE;
	}
	if (0) if (!JS_DefineFunctions(cx, _obj, SFNodeFunctions)) {
		printf("JS_DefineFunctions failed in Route sourceNode.\n");
		return JS_FALSE;
	}

	if (!JS_SetPrivateFw(cx, _obj, (void*)nany)) {
		printf("JS_SetPrivate failed in Route sourceNode.\n");
		return JS_FALSE;
	}

	JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(_obj));

	return JS_TRUE;

}
JSFunctionSpec(ProtoDeclarationFunctions)[] = {
	{"newInstance", ProtoDeclaration_newInstance, 0},
	{0}
};

static JSPropertySpec(ProtoDeclarationProperties)[] = {
	{"name", 0, JSPROP_ENUMERATE}, //string
	{"fields", 1, JSPROP_ENUMERATE}, //FieldDefinitionArray
	{"isExternProto", 2, JSPROP_ENUMERATE}, //boolea
	{0}
};

JSBool
ProtoDeclarationGetProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp) {
	JSObject* obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval* vp = hvp.address();

	struct X3D_Proto* ptr;
	int _index;
	JSString* _str;
	jsval rval;
	jsval id;

	UNUSED(rval); // compiler warning mitigation


	if (!JS_IdToValue(cx, iid, &id)) {
		printf("JS_IdToValue failed in ProtoDeclarationGetProperty.\n");
		return JS_FALSE;
	}

	if ((ptr = (struct X3D_Proto*)JS_GetPrivateFw(cx, obj)) == NULL) {
		printf("JS_GetPrivate failed in ProtoDeclarationGetProperty.\n");
		return JS_FALSE;
	}
	struct X3D_Proto* ec = (struct X3D_Proto*)JS_GetContextPrivate(cx);
	struct X3D_Proto* proto = ptr;
	struct ProtoDefinition* pd;
	pd = (struct ProtoDefinition*)proto->__protoDef;

	int index = -1;
	if (JSVAL_IS_INT(id))
		index = JSVAL_TO_INT(id);
	else
		index = lookup_tinyid(JS_EncodeString(cx, JSVAL_TO_STRING(id)), ProtoDeclarationProperties);
	switch (index) {
	case 0://name (string)
	{
		//Q. where do we hide the name of the proto type?
		//see BOOL isAvailableBroto(const char *pname, struct X3D_Proto* currentContext, struct X3D_Proto **proto) 
	
		JSString* _str;
		_str = JS_NewStringCopyZ(cx, pd->protoName);
		rval = STRING_TO_JSVAL(_str);

		JS_SET_RVAL(cx, vp, rval);
		break;
	}
	case 1://fields (FieldDefinitionArray)
	{
		JSObject* _obj;
		_obj = JS_NewObject(cx, &FieldDefinitionArrayClass, NULL, obj);
		if (0) if (!JS_DefineProperties(cx, _obj, FieldDefinitionArrayProperties)) {
			printf("JS_DefineProperties failed in FieldDefinitionArrayProperties.\n");
			return JS_FALSE;
		}

		if (!JS_SetPrivateFw(cx, _obj, (void*)pd)) {
			printf("JS_SetPrivate failed in ProtoDeclarationArray.\n");
			return JS_FALSE;
		}

		JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(_obj));

		break;
	}
	case 2://isExternProto (boolean)
	{
		char flagInstance, flagExtern;
		flagInstance = ciflag_get(proto->__protoFlags, 2);
		flagExtern = ciflag_get(proto->__protoFlags, 3);

		JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(flagExtern == 0 ? false : true));
		break;
	}
	}

	return JS_TRUE;
}
JSBool
ProtoDeclarationSetProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp) {
	JSObject* obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval* vp = hvp.address();
	//can I, should I force it to read-only this way?
	return JS_FALSE;
}

static JSClass ProtoDeclarationClass = {
	"ProtoDeclaration",
	JSCLASS_HAS_PRIVATE,
	JS_PropertyStub,
	JS_DeletePropertyStub,
	ProtoDeclarationGetProperty,
	ProtoDeclarationSetProperty,
	JS_EnumerateStub,
	JS_ResolveStub,
	JS_ConvertStub,
	JS_FinalizeStub
};

//ProtoDeclarationArray{

static JSPropertySpec(ProtoDeclarationArrayProperties)[] = {
	{"length", -1, JSPROP_READONLY | JSPROP_SHARED | JSPROP_PERMANENT}, //JSPROP_ENUMERATE},
	{0}
};

JSBool
ProtoDeclarationArrayGetProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp) {
	JSObject* obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval* vp = hvp.address();

	Stack *protos;
	jsval rval;
	jsval id;

	UNUSED(rval); // compiler warning mitigation


	if (!JS_IdToValue(cx, iid, &id)) {
		printf("JS_IdToValue failed in ProtoDeclarationArrayGetProperty.\n");
		return JS_FALSE;
	}

	if ((protos = (Stack*)JS_GetPrivateFw(cx, obj)) == NULL) {
		printf("JS_GetPrivate failed in ProtoDeclarationArrayGetProperty.\n");
		return JS_FALSE;
	}
	struct X3D_Proto* ec = (struct X3D_Proto*)JS_GetContextPrivate(cx);

	int index = -1;
	if (JSVAL_IS_INT(id))
		index = JSVAL_TO_INT(id);
	else
		index = lookup_tinyid(JS_EncodeString(cx, JSVAL_TO_STRING(id)), ProtoDeclarationArrayProperties);
	if (index == -1) {
		int _length = vectorSize(protos);
		JS_SET_RVAL(cx, vp, INT_TO_JSVAL(_length));
	}
	else if (index > -1 && index < vectorSize(protos))
	{
		JSObject* _obj;
		_obj = JS_NewObject(cx, &ProtoDeclarationClass, NULL, obj);
		if (0) if (!JS_DefineProperties(cx, _obj, ProtoDeclarationProperties)) {
			printf("JS_DefineProperties failed in ProtoDeclarationProperties.\n");
			return JS_FALSE;
		}
		struct X3D_Proto* proto = vector_get(struct X3D_Proto*,protos,index);
		if (!JS_SetPrivateFw(cx, _obj, (void*)proto)) {
			printf("JS_SetPrivate failed in ProtoDeclarationArray.\n");
			return JS_FALSE;
		}

		JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(_obj));

	}

	return JS_TRUE;
}
JSBool
ProtoDeclarationArraySetProperty(JSContext* cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp) {
	JSObject* obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval* vp = hvp.address();
	//can I, should I force it to read-only this way?
	return JS_FALSE;
}


static JSClass ProtoDeclarationArrayClass = {
	"ProtoDeclarationArray",
	JSCLASS_HAS_PRIVATE,
	JS_PropertyStub,
	JS_DeletePropertyStub,
	ProtoDeclarationArrayGetProperty,
	ProtoDeclarationArraySetProperty,
	JS_EnumerateStub,
	JS_ResolveStub,
	JS_ConvertStub,
	JS_FinalizeStub
};




//Q. is this a true sharable static?
static JSClass BrowserClass = {
    "Browser",
    JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,
    JS_DeletePropertyStub,
#ifdef X3DBROWSER
    BrowserGetProperty, //JS_PropertyStub, 
	BrowserSetProperty, //JS_StrictPropertyStub, 
#else
	JS_PropertyStub,
	SetPropertyStub,
#endif

    JS_EnumerateStub,
    JS_ResolveStub,
    JS_ConvertStub,
    JS_FinalizeStub
};

static JSBool doVRMLRoute(JSContext *context, JSObject *obj, uintN argc, jsval *argv, const char *browserFunc); 

static JSFunctionSpec (BrowserFunctions)[] = {
	{"getName", VrmlBrowserGetName, 0},
	{"getVersion", VrmlBrowserGetVersion, 0},
	{"getCurrentSpeed", VrmlBrowserGetCurrentSpeed, 0},
	{"getCurrentFrameRate", VrmlBrowserGetCurrentFrameRate, 0},
	{"getWorldURL", VrmlBrowserGetWorldURL, 0},
	{"replaceWorld", VrmlBrowserReplaceWorld, 0},
	{"loadURL", VrmlBrowserLoadURL, 0},
	{"setDescription", VrmlBrowserSetDescription, 0},
	{"createVrmlFromString", VrmlBrowserCreateVrmlFromString, 0},
	{"createVrmlFromURL", VrmlBrowserCreateVrmlFromURL, 0},
	{"createX3DFromString", VrmlBrowserCreateX3DFromString, 0},
	{"createX3DFromURL", VrmlBrowserCreateVrmlFromURL, 0},
	{"addRoute", VrmlBrowserAddRoute, 0},
	{"deleteRoute", VrmlBrowserDeleteRoute, 0},
	{"print", VrmlBrowserPrint, 0},
	{"println", VrmlBrowserPrintln, 0},
#ifdef X3DBROWSER
	//{"replaceWorld", X3dBrowserReplaceWorld, 0},  //conflicts - X3DScene vs MFNode parameter - could detect?
	//{"createX3DFromString", X3dBrowserCreateX3DFromString, 0}, //conflicts but above verion shouldn't be above, or could detect?
	//{"createX3DFromURL", X3dBrowserCreateVrmlFromURL, 0}, //conflicts but above version shouldn't be above, or could detect?
	//{importDocument, X3dBrowserImportDocument, 0), //not sure we need/want this, what does it do?
	//{getRenderingProperty, X3dGetRenderingProperty, 0},
	//{addBrowserListener, X3dAddBrowserListener, 0},
	//{removeBrowserListener, X3dRemoveBrowserListener, 0},
#endif
	{0}
};
#ifdef X3DBROWSER

/* ProfileInfo, ProfileInfoArray, ComponentInfo, ComponentInfoArray
   I decided to do these as thin getter wrappers on the bits and pieces defined
   in Structs.h, GeneratedCode.c and capabilitiesHandler.c
   The Array types return the info type wrapper with private native member == index into
   the native array.
*/


JSBool
ComponentInfoGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();


	IntTableIndex ptr;
	int _index, *_table, _nameIndex;
	jsval rval;
	jsval id;

	UNUSED(rval); // compiler warning mitigation


	if (!JS_IdToValue(cx,iid,&id)) {
		printf("JS_IdToValue failed in ComponentInfoGetProperty.\n");
		return JS_FALSE;
	}

	if ((ptr = (IntTableIndex)JS_GetPrivateFw(cx, obj)) == NULL) {
		printf( "JS_GetPrivate failed in ExecutionContextGetProperty.\n");
		return JS_FALSE;
	}
	_index = ptr->index;
	_table = ptr->table;
//extern const char *COMPONENTS[];
//extern const int COMPONENTS_COUNT;

    if (JSVAL_IS_INT(id)) 
	{
		int index = JSVAL_TO_INT(id);
		switch(index){
			case 0://name
			case 1://Title
				_nameIndex = _table[2*_index];
			JS_SET_RVAL(cx,vp,STRING_TO_JSVAL(JS_NewStringCopyZ(cx,COMPONENTS[_nameIndex])));
				break;
			case 2://level
				{
				int level = capabilitiesHandler_getComponentLevel(_table,_index);
			JS_SET_RVAL(cx,vp,INT_TO_JSVAL(level));
				}
				break;
			case 3://providerUrl
				JS_SET_RVAL(cx,vp,STRING_TO_JSVAL(JS_NewStringCopyZ(cx,"freewrl.sourceforge.net")));
				break;
		}
	}
	return JS_TRUE;
}
JSBool
ComponentInfoSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();
	//can I, should I force it to read-only this way?
	return JS_FALSE;
}
void
ComponentInfoFinalize(JSContext *cx, JSObject *obj)
{
	IntTableIndex ptr;
	if ((ptr = (IntTableIndex)JS_GetPrivateFw(cx, obj)) == NULL) {
		return;
	} else {
		FREE_IF_NZ (ptr);
	}
}


static JSClass ComponentInfoClass = {
    "ComponentInfo",
    JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,
    JS_DeletePropertyStub,
    ComponentInfoGetProperty, 
	ComponentInfoSetProperty, 
    JS_EnumerateStub,
    JS_ResolveStub,
    JS_ConvertStub,
    JS_FinalizeStub, //ComponentInfoFinalize //JS_FinalizeStub
};

static JSPropertySpec (ComponentInfoProperties)[] = {
	//executionContext
	{"name", 0, JSPROP_ENUMERATE},  //"Core"
	{"title", 1, JSPROP_ENUMERATE}, //"Core"
	{"level", 2, JSPROP_ENUMERATE},  //4
	{"providerUrl", 3, JSPROP_ENUMERATE}, //"freewrl.sourceforge.net"
	{0}
};


//ComponentInfoArray{
//numeric length;
//ComponentInfo [integer index];
//}
static JSPropertySpec(ComponentInfoArrayProperties)[] = {
	{"length", -1, JSPROP_READONLY | JSPROP_SHARED | JSPROP_PERMANENT}, //JSPROP_ENUMERATE},
	{0}
};

JSBool
ComponentInfoArrayGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid,  JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();

	int *_table;
	jsval rval;
	jsval id;

	UNUSED(rval); // compiler warning mitigation


	if (!JS_IdToValue(cx,iid,&id)) {
		printf("JS_IdToValue failed in ComponentInfoArrayGetProperty.\n");
		return JS_FALSE;
	}

	if ((_table = (int *)JS_GetPrivateFw(cx, obj)) == NULL) {
		printf( "JS_GetPrivate failed in ComponentInfoGetProperty.\n");
		return JS_FALSE;
	}

	int index = -1;
	if (JSVAL_IS_INT(id))
		index = JSVAL_TO_INT(id);
	else
		index = lookup_tinyid(JS_EncodeString(cx, JSVAL_TO_STRING(id)), ComponentInfoArrayProperties);
	if(index == -1){
//extern const char *COMPONENTS[];
//extern const int COMPONENTS_COUNT;

		int _length = capabilitiesHandler_getTableLength(_table); //COMPONENTS_COUNT;
		JS_SET_RVAL(cx,vp,INT_TO_JSVAL(_length));
	}else if(index > -1 && index < COMPONENTS_COUNT )
	{
		JSObject *_obj;
		IntTableIndex tableindex = (IntTableIndex)MALLOC(void *, sizeof(struct intTableIndex));
		//int* _index = MALLOC(void *, sizeof(int));
		_obj = JS_NewObject(cx,&ComponentInfoClass,NULL,obj);
		tableindex->index = index;
		tableindex->table = _table;
		if(0) if (!JS_DefineProperties(cx, _obj, ComponentInfoProperties)) {
			printf( "JS_DefineProperties failed in ComponentInfoProperties.\n");
			return JS_FALSE;
		}

		if (!JS_SetPrivateFw(cx, _obj, (void*)tableindex)) {
			printf( "JS_SetPrivate failed in ComponentInfoArray.\n");
			return JS_FALSE;
		}

		JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(_obj));

	}

	return JS_TRUE;
}
JSBool
ComponentInfoArraySetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();
	//can I, should I force it to read-only this way?
	return JS_FALSE;
}


static JSClass ComponentInfoArrayClass = {
    "ComponentInfoArray",
    JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,
    JS_DeletePropertyStub,
    ComponentInfoArrayGetProperty, 
	ComponentInfoArraySetProperty, 
    JS_EnumerateStub,
    JS_ResolveStub,
    JS_ConvertStub,
    JS_FinalizeStub
};


//ProfileInfo{
//String name;
//Numeric level;
//String Title;
//String providerUrl;
//ComonentInfoArray components;
//}

JSBool
ProfileInfoGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid,  JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();

	int *ptr;
	int _index;
	jsval rval;
	jsval id;

	UNUSED(rval); // compiler warning mitigation

	if (!JS_IdToValue(cx,iid,&id)) {
		printf("JS_IdToValue failed in ProfileInfoGetProperty.\n");
		return JS_FALSE;
	}

	if ((ptr = (int *)JS_GetPrivateFw(cx, obj)) == NULL) {
		printf( "JS_GetPrivate failed in ProfileInfoGetProperty.\n");
		return JS_FALSE;
	}
	_index = *ptr;
//extern const char *PROFILES[];
//extern const int PROFILES_COUNT;

    if (JSVAL_IS_INT(id)) 
	{
		int index = JSVAL_TO_INT(id);
		switch(index){
			case 0://name
			case 1://Title
			JS_SET_RVAL(cx,vp,STRING_TO_JSVAL(JS_NewStringCopyZ(cx,PROFILES[_index])));
				break;
			case 2://level
				{
				int level = capabilitiesHandler_getProfileLevel(_index);
			JS_SET_RVAL(cx,vp,INT_TO_JSVAL(level));

				}
				break;
			case 3://providerUrl
				JS_SET_RVAL(cx,vp,STRING_TO_JSVAL(JS_NewStringCopyZ(cx,"freewrl.sourceforge.net")));
				break;
			case 4://components ComponentInfoArray
				{
					const int *_table = capabilitiesHandler_getProfileComponent(_index);
					JSObject *_obj;
					//malloc private not needed
					_obj = JS_NewObject(cx,&ComponentInfoArrayClass,NULL,obj);
					if(0)if (!JS_DefineProperties(cx, _obj, ComponentInfoArrayProperties)) {
						printf( "JS_DefineProperties failed in ComponentInfoArrayProperties.\n");
						return JS_FALSE;
					}
				
					if (!JS_SetPrivateFw(cx, _obj, (void*)_table)) {
						printf( "JS_SetPrivate failed in ComponentInfoArray.\n");
						return JS_FALSE;
					}
					JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(_obj));
				}
				break;
		}
	}
	return JS_TRUE;
}
JSBool
ProfileInfoSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();

	//can I, should I force it to read-only this way?
	return JS_FALSE;
}

static JSClass ProfileInfoClass = {
    "ProfileInfo",
    JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,
    JS_DeletePropertyStub,
    ProfileInfoGetProperty, 
	ProfileInfoSetProperty, 
    JS_EnumerateStub,
    JS_ResolveStub,
    JS_ConvertStub,
    JS_FinalizeStub
};

static JSPropertySpec (ProfileInfoProperties)[] = {
	//executionContext
	{"name", 0, JSPROP_ENUMERATE},
	{"Title", 1, JSPROP_ENUMERATE},
	{"level", 2, JSPROP_ENUMERATE},
	{"providerUrl", 3, JSPROP_ENUMERATE},
	{"components", 4, JSPROP_ENUMERATE},
	{0}
};

//ProfileInfoArray{
//numeric length;
//ProfileInfo [integer index];
//}
static JSPropertySpec(ProfileInfoArrayProperties)[] = {
	{"length", -1, JSPROP_READONLY | JSPROP_SHARED | JSPROP_PERMANENT}, //JSPROP_ENUMERATE},
	{0}
};

JSBool
ProfileInfoArrayGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid,  JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();

	jsval rval;
	jsval id;

	UNUSED(rval); // compiler warning mitigation


	if (!JS_IdToValue(cx,iid,&id)) {
		printf("JS_IdToValue failed in ProfileInfoArrayGetProperty.\n");
		return JS_FALSE;
	}

	int index = -1;
	if (JSVAL_IS_INT(id))
		index = JSVAL_TO_INT(id);
	else
		index = lookup_tinyid(JS_EncodeString(cx, JSVAL_TO_STRING(id)), ProfileInfoArrayProperties);
	if (index == -1) {
		int _length = PROFILES_COUNT;
		JS_SET_RVAL(cx,vp,INT_TO_JSVAL(_length));
	}else
	//if(index < getNumberOfProfiles() )
	{
		JSObject *_obj;
		int* _index = (int*)MALLOC(void *, sizeof(int));
		_obj = JS_NewObject(cx,&ProfileInfoClass,NULL,obj);
		*_index = index;
		if (!JS_DefineProperties(cx, _obj, ProfileInfoProperties)) {
			printf( "JS_DefineProperties failed in ProfileInfoArray.\n");
			return JS_FALSE;
		}

		if (!JS_SetPrivateFw(cx, _obj, (void*)_index)) {
			printf( "JS_SetPrivate failed in ProfileInfoArray.\n");
			return JS_FALSE;
		}

		JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(_obj));
	}
	
	return JS_TRUE;
}
JSBool
ProfileInfoArraySetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();

	//can I, should I force it to read-only this way?
	return JS_FALSE;
}


static JSClass ProfileInfoArrayClass = {
    "ProfileInfo",
    JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,
    JS_DeletePropertyStub,
    ProfileInfoArrayGetProperty, 
	ProfileInfoArraySetProperty, 
    JS_EnumerateStub,
    JS_ResolveStub,
    JS_ConvertStub,
    JS_FinalizeStub
};

extern "C" {
	char* lookup_brotoDefname(struct X3D_Proto* ec, struct X3D_Node* node) {
		int n = vectorSize(ec->__DEFnames);
		char* name = NULL;
		struct brotoDefpair def;
		for (int i = 0; i < n; i++) {
			def = vector_get(struct brotoDefpair, ec->__DEFnames, i);
			//printf("%x %x %s\n",node,def.node,def.name);
			if (def.node == node) {
				name = def.name;
				break;
			}
		}
		return name;
	}
} //extern C

char* lookup_brotoDefname(struct X3D_Proto* ec, struct X3D_Node* node);
JSBool
X3DRouteToString(JSContext* cx, uintN argc, jsval* vp) {
	JSObject* obj = JS_THIS_OBJECT(cx, vp);
	jsval* argv = JS_ARGV(cx, vp);
	jsval rval;

	UNUSED(argc);
	UNUSED(argv);


	long long ptr;
	char str[200];
	JSString* _str;
	if ((ptr = (long long)JS_GetPrivateFw(cx, obj)) == NULL) {
		printf("in route.toString() - not a Native\n");
		return JS_FALSE;
	}
	int _index = ptr - 1;
	struct X3D_Proto *ec = (struct X3D_Proto*)JS_GetContextPrivate(cx);
	struct brotoRoute* route = vector_get(struct brotoRoute*, ec->__ROUTES, _index);

	//getSpecificRoute(_index, &fromNode, &fromOffset, &toNode, &toOffset);
	char* fromName = lookup_brotoDefname(ec,route->from.node); // parser_getNameFromNode(route->from.node);
	char* toName = lookup_brotoDefname(ec,route->from.node); // parser_getNameFromNode(route->to.node);
	char *fromfield = findFIELDNAMESfromNodeOffset0(route->from.node, route->from.ifield);
	char *tofield = findFIELDNAMESfromNodeOffset0(route->to.node, route->to.ifield);

	sprintf(str,"[ROUTE %s.%s TO %s.%s]", fromName, fromfield, toName, tofield);
	_str = JS_NewStringCopyZ(cx, str);
	rval = STRING_TO_JSVAL(_str);

	JS_SET_RVAL(cx, vp, rval);
	return JS_TRUE;

}

JSFunctionSpec(X3DRouteFunctions)[] = {
	{"toString", X3DRouteToString, 0},
	{0}
};

static JSPropertySpec(X3DRouteProperties)[] = {
	//executionContext
	{"sourceNode", 0, JSPROP_ENUMERATE},
	{"sourceField", 1, JSPROP_ENUMERATE},
	{"destinationNode", 2, JSPROP_ENUMERATE},
	{"destinationField", 3, JSPROP_ENUMERATE},
	{0}
};

JSBool
X3DRouteGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid,  JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();

	long long ptr;
	int _index;
	JSString *_str;
	jsval rval;
	struct X3D_Node *fromNode, *toNode;
	int fromOffset, toOffset;
	const char *fieldname;
	jsval id;

	UNUSED(rval); // compiler warning mitigation


	if (!JS_IdToValue(cx,iid,&id)) {
		printf("JS_IdToValue failed in X3DRouteGetProperty.\n");
		return JS_FALSE;
	}

	if ((ptr = (long long)JS_GetPrivateFw(cx, obj)) == NULL) {
		printf( "JS_GetPrivate failed in X3DRouteGetProperty.\n");
		return JS_FALSE;
	}
	_index = ptr - 1;
	struct X3D_Proto* ec = (struct X3D_Proto*)JS_GetContextPrivate(cx);
	struct brotoRoute* route = vector_get(struct brotoRoute*, ec->__ROUTES, _index);

	//routes = getCRoutes();
	//route = routes[_index];
	//getSpecificRoute (_index,&fromNode, &fromOffset, &toNode, &toOffset);
	char* fromName = lookup_brotoDefname(ec,route->from.node); // parser_getNameFromNode(route->from.node);
	char* toName = lookup_brotoDefname(ec,route->to.node); // parser_getNameFromNode(route->to.node);

	int index = -1;
	if (JSVAL_IS_INT(id))
		index = JSVAL_TO_INT(id);
	else
		index = lookup_tinyid(JS_EncodeString(cx, JSVAL_TO_STRING(id)), X3DRouteProperties);
	switch(index){
		case 0://sourceNode
		case 2://destinationNode
			//route.routeFromNode
			{
				JSObject *_obj;
				//SFNodeNative *sfnn = (SFNodeNative *)MALLOC(void *, sizeof(SFNodeNative));
				//memset(sfnn,0,sizeof(SFNodeNative)); //I don't know if I'm supposed to set something else dug9 aug5,2013
				AnyNative* nany = MALLOC(AnyNative*, sizeof(AnyNative));
				memset(nany, 0, sizeof(AnyNative));
				nany->type = FIELDTYPE_SFNode;
				nany->v = MALLOC(union anyVrml*, sizeof(union anyVrml));
				memset(nany->v, 0, sizeof(union anyVrml));
				if (index == 0)
					nany->v->sfnode = route->from.node; // fromNode;
				if (index == 2)
					nany->v->sfnode = route->to.node; // toNode;
					
				_obj = JS_NewObject(cx,&SFNodeClass,NULL,obj);
				if(0) if (!JS_DefineProperties(cx, _obj, SFNodeProperties)) {
					printf( "JS_DefineProperties failed in Route sourceNode.\n");
					return JS_FALSE;
				}
				if(0) if (!JS_DefineFunctions(cx, _obj, SFNodeFunctions)) {
					printf( "JS_DefineFunctions failed in Route sourceNode.\n");
					return JS_FALSE;
				}

				if (!JS_SetPrivateFw(cx, _obj, (void*)nany)) {
					printf( "JS_SetPrivate failed in Route sourceNode.\n");
					return JS_FALSE;
				}

				JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(_obj));
			}
			break;

		case 1://sourceField
			fieldname = findFIELDNAMESfromNodeOffset0(route->from.node, route->from.ifield);
			_str = JS_NewStringCopyZ(cx,fieldname);
			JS_SET_RVAL(cx,vp,STRING_TO_JSVAL(_str));
			break;
		case 3://destinationField
			fieldname = findFIELDNAMESfromNodeOffset0(route->to.node, route->to.ifield);
			_str = JS_NewStringCopyZ(cx,fieldname);
			JS_SET_RVAL(cx,vp,STRING_TO_JSVAL(_str));

			break;
	}
	
	return JS_TRUE;
}
JSBool
X3DRouteSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();
	//can I, should I force it to read-only this way?
	return JS_FALSE;
}

static JSClass X3DRouteClass = {
    "X3DRoute",
    JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,
    JS_DeletePropertyStub,
    X3DRouteGetProperty, 
	X3DRouteSetProperty, 
    JS_EnumerateStub,
    JS_ResolveStub,
    JS_ConvertStub,
    JS_FinalizeStub
};



//ProfileInfoArray{
//numeric length;
//ProfileInfo [integer index];
//}
static JSPropertySpec(RouteArrayProperties)[] = {
	{"length", -1, JSPROP_READONLY | JSPROP_SHARED | JSPROP_PERMANENT}, //JSPROP_ENUMERATE},
	{0}
};

JSBool
RouteArrayGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid,  JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();


	jsval rval;
	jsval id;

	UNUSED(rval); //compiler warning mitigation


	if (!JS_IdToValue(cx,iid,&id)) {
		printf("JS_IdToValue failed in RouteArrayGetProperty.\n");
		return JS_FALSE;
	}
	struct X3D_Proto* ec;
	//ec = getExecutionContextFromCx(cx);
	ec = (struct X3D_Proto*)JS_GetContextPrivate(cx);

	int index = -1;
	if (JSVAL_IS_STRING(id)) {
		char* field = (char*)JS_EncodeString(cx, JSVAL_TO_STRING(id));
		index = lookup_tinyid(field, RouteArrayProperties);
		//printf("ExecutionContextGetProperty %s %d\n", field, index);
		if (index == -1) {
			int _length = vectorSize(ec->__ROUTES); //getCRouteCount();
			JS_SET_RVAL(cx, vp, INT_TO_JSVAL(_length));
		}
	}
	else if (JSVAL_IS_INT(id)) {
		index = JSVAL_TO_INT(id);
		{
			JSObject *_obj;
			//int* _index = (int*) MALLOC(void *, sizeof(int));
			_obj = JS_NewObject(cx,&X3DRouteClass,NULL,obj);
			//*_index = index;
			if(0) if (!JS_DefineProperties(cx, _obj, X3DRouteProperties)) {
				printf( "JS_DefineProperties failed in RouteArray.\n");
				return JS_FALSE;
			}
			long long iindex = index+1; //instead of malloc and free wrapper for long, just send a longlong on x64
			if (!JS_SetPrivateFw(cx, _obj, (void*)iindex)) {
				printf( "JS_SetPrivate failed in RouteArray.\n");
				return JS_FALSE;
			}

			JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(_obj));

		}
	}
	return JS_TRUE;
}
JSBool
RouteArraySetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();
	//can I, should I force it to read-only this way?
	return JS_FALSE;
}


static JSClass RouteArrayClass = {
    "RouteArray",
    JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,
    JS_DeletePropertyStub,
    RouteArrayGetProperty, 
	RouteArraySetProperty, 
    JS_EnumerateStub,
    JS_ResolveStub,
    JS_ConvertStub,
    JS_FinalizeStub
};


static JSBool
X3DExecutionContext_createNode(JSContext* context, uintN argc, jsval* vp) {
	JSObject* obj = JS_NewObject(context, &SFNodeClass, NULL, NULL);
	ADD_ROOT(cx, obj)

	jsval* argv = JS_ARGV(context, vp);
	const char* _c_format = "S";
	JSString* js_c;

	char* _c;

	/* for the return of the nodes */
	struct X3D_Group* retGroup;
	struct Multi_Node* newHandle = NULL;

	if (argc == 1 &&
		JS_ConvertArguments(context, argc, argv, _c_format, &js_c)) {
		_c = JS_EncodeString(context, js_c);
#ifdef JSVERBOSE
		printf("X3DExecutionContext_createNode: obj = %u, str = \"%s\"\n",
			obj, _c);
#endif
		{
			int ctype;
			//check builtins
			ctype = findFieldInNODES(_c);
			struct X3D_Node* node = NULL;
			struct X3D_Proto* ec;
			//ec = getExecutionContextFromCx(cx);
			ec = (struct X3D_Proto*)JS_GetContextPrivate(context);

			if (ctype > -1) {
				node = (struct X3D_Node*)createNewX3DNode(ctype);
				add_node_to_broto_context(ec, node);

				AnyNative* lhs;
				if ((lhs = (AnyNative*)AnyNativeNew(FIELDTYPE_SFNode, NULL, NULL)) == NULL) {
					printf("AnyNativeNew failed in SFNodeConstr.\n");
					return JS_FALSE;
				}
				if (!JS_SetPrivateFw(context, obj, lhs)) {
					printf("JS_SetPrivate failed in SFNodeConstr.\n");
					return JS_FALSE;
				}
				//lhs->valueChanged = NULL; 
				lhs->v->sfnode = node;
			}
			else {
				printf("\nIncorrect argument format for createNode('nodetype').\n");
				JS_free(context, _c);
				return JS_FALSE;
			}
		}
		JS_free(context, _c);
	}
	else {
		printf("\nIncorrect argument format for createNode('nodetype').\n");
		return JS_FALSE;
	}

	JS_SET_RVAL(context, vp, OBJECT_TO_JSVAL(obj));
	return JS_TRUE;
}

static JSBool
X3DExecutionContext_createProto(JSContext* context, uintN argc, jsval* vp) {
	JSObject* obj = JS_NewObject(context, &SFNodeClass, NULL, NULL);
	ADD_ROOT(cx, obj)
	jsval* argv = JS_ARGV(context, vp);
	const char* _c_format = "S";
	JSString* js_c;
	struct X3D_Node* node = NULL;
	struct X3D_Proto* ec;
	struct X3D_Proto* proto;
	proto = NULL;

	char* _c;

	if (argc == 1 &&
		JS_ConvertArguments(context, argc, argv, _c_format, &js_c)) {
		_c = JS_EncodeString(context, js_c);
#ifdef JSVERBOSE
		printf("X3DExecutionContext_createProto: obj = %u, str = \"%s\"\n",	obj, _c);
#endif
		ec = (struct X3D_Proto*)JS_GetContextPrivate(context);
		if (isAvailableBroto(_c, ec, &proto))
		{
			struct X3D_Proto* source, * dest;
			node = X3D_NODE(brotoInstance(proto, 1));
			node->_executionContext = X3D_NODE(ec); //me->ptr;
			add_node_to_broto_context(ec, node);
			//during parsing, setting of fields would occur between instance and body,
			//so field values perculate down.
			//here we elect default field values
			source = X3D_PROTO(X3D_PROTO(node)->__prototype);
			dest = X3D_PROTO(node);
			deep_copy_broto_body2(&source, &dest);

			AnyNative* lhs;
			if ((lhs = (AnyNative*)AnyNativeNew(FIELDTYPE_SFNode, NULL, NULL)) == NULL) {
				printf("AnyNativeNew failed in SFNodeConstr.\n");
				return JS_FALSE;
			}
			if (!JS_SetPrivateFw(context, obj, lhs)) {
				printf("JS_SetPrivate failed in SFNodeConstr.\n");
				return JS_FALSE;
			}
			//lhs->valueChanged = NULL; 
			lhs->v->sfnode = node;
		}
		else {
			printf("\nIncorrect argument for createProto('prototype').\n");
			JS_free(context, _c);
			return JS_FALSE;
		}
		JS_free(context, _c);
	}
	else {
		printf("\nIncorrect argument format for createNode('nodetype').\n");
		return JS_FALSE;
	}
	JS_SET_RVAL(context, vp, OBJECT_TO_JSVAL(obj));
	return JS_TRUE;
}
/*
struct X3D_Node *broto_search_DEFname(struct X3D_Proto *context, const char *name);
struct X3D_Node * broto_search_ALLnames(struct X3D_Proto *context, const char *name, int *source);
int X3DExecutionContext_getNamedNode(FWType fwtype, void *ec, void *fwn, int argc, FWval fwpars, FWval fwretval){
	int nr = 0;
	struct X3D_Node* node = NULL;
	//broto warning - DEF name list should be per-executionContext
	//struct X3D_Proto *ec = (struct X3D_Proto *)fwn; //we want the script node's parent context for imported nodes, I think
	node = broto_search_DEFname(ec, fwpars[0]._string);

	if(node){
		//fwretval->_web3dval.native = node;  //Q should this be &node? to convert it from X3D_Node to anyVrml->sfnode?
		fwretval->_web3dval.anyvrml = malloc(sizeof(union anyVrml));
		fwretval->_web3dval.anyvrml->sfnode = node;
		fwretval->_web3dval.fieldType = FIELDTYPE_SFNode;
		fwretval->_web3dval.gc = 1;
		fwretval->itype = 'W';
		nr = 1;
	}
	return nr;
}
*/

static JSBool
X3DExecutionContext_getNamedNode(JSContext* context, uintN argc, jsval* vp) {
	JSObject* obj = JS_NewObject(context, &SFNodeClass, NULL, NULL);
	ADD_ROOT(cx, obj)
	jsval* argv = JS_ARGV(context, vp);
	const char* _c_format = "S";
	JSString* js_c;
	char* _c;

	if (argc == 1 &&
		JS_ConvertArguments(context, argc, argv, _c_format, &js_c)) {
		_c = JS_EncodeString(context, js_c);
#ifdef JSVERBOSE
		printf("X3DExecutionContext_getNamedNode: obj = %u, str = \"%s\"\n",
			obj, _c);
#endif
		{
			struct X3D_Node* node = NULL;
			struct X3D_Proto* ec;
			//ec = getExecutionContextFromCx(cx);
			ec = (struct X3D_Proto*)JS_GetContextPrivate(context);
			node = broto_search_DEFname(ec, _c);

			if (node != NULL) {
				AnyNative* lhs;
				if ((lhs = (AnyNative*)AnyNativeNew(FIELDTYPE_SFNode, NULL, NULL)) == NULL) {
					printf("AnyNativeNew failed in SFNodeConstr.\n");
					return JS_FALSE;
				}
				if (!JS_SetPrivateFw(context, obj, lhs)) {
					printf("JS_SetPrivate failed in SFNodeConstr.\n");
					return JS_FALSE;
				}
				//lhs->valueChanged = NULL; 
				lhs->v->sfnode = node;
			}
			else {
				printf("\nIncorrect argument for getNamedNode('DEFname').\n");
				JS_free(context, _c);
				return JS_FALSE;
			}
		}
		JS_free(context, _c);
	}
	else {
		printf("\nIncorrect argument format for getNamedNode('DEFname').\n");
		return JS_FALSE;
	}

	JS_SET_RVAL(context, vp, OBJECT_TO_JSVAL(obj));
	return JS_TRUE;
}

static JSBool
X3DExecutionContext_removeNamedNode(JSContext* context, uintN argc, jsval* vp) {
	jsval* argv = JS_ARGV(context, vp);
	const char* _c_format = "S";
	JSString* js_c;
	char* _c;

	if (argc == 1 &&
		JS_ConvertArguments(context, argc, argv, _c_format, &js_c)) {
		_c = JS_EncodeString(context, js_c);
#ifdef JSVERBOSE
		printf("X3DExecutionContext_removeNamedNode: obj = %u, str = \"%s\"\n",
			obj, _c);
#endif
		{
			struct X3D_Node* node = NULL;
			struct X3D_Proto* ec;
			//ec = getExecutionContextFromCx(cx);
			ec = (struct X3D_Proto*)JS_GetContextPrivate(context);
			node = broto_search_DEFname(ec, _c);

			if (node != NULL) {
				remove_node_from_parents_children(node);
				remove_broto_node(ec, node);
				remove_node_from_def_list(ec, node, _c);
			}
			else {
				printf("\nIncorrect argument for removeNamedNode('DEFname').\n");
				JS_free(context, _c);
				return JS_FALSE;
			}
		}
		JS_free(context, _c);
	}
	else {
		printf("\nIncorrect argument format for removeNamedNode('DEFname').\n");
		return JS_FALSE;
	}

	return JS_TRUE;
}
static JSBool
X3DExecutionContext_updateNamedNode(JSContext* context, uintN argc, jsval* vp) {
	jsval* argv = JS_ARGV(context, vp);
	const char* _c_format = "S";
	JSString* js_c;
	JSObject* _ob2;
	struct X3D_Node* node = NULL;
	struct brotoDefpair* bd;
	int found = 0;
	char* defname;

	if (argc == 2 &&
		JS_ConvertArguments(context, argc, argv, _c_format, &js_c)) {
		defname = JS_EncodeString(context, js_c);
		if (argv[1].isObject()) {
			_ob2 = JSVAL_TO_OBJECT(argv[1]);
			AnyNative* _node = NULL;
			if ((_node = (AnyNative*)JS_GetPrivateFw(context, _ob2)) == NULL) {
				printf("JS_GetPrivate failed for arg format \"o d\" in SFRotationConstr.\n");
				return JS_FALSE;
			}
			node = _node->v->sfnode;

#ifdef JSVERBOSE
		printf("X3DExecutionContext_removeNamedNode: obj = %u, str = \"%s\"\n",
			obj, _c);
#endif

			struct X3D_Proto* ec;
			//ec = getExecutionContextFromCx(cx);
			ec = (struct X3D_Proto*)JS_GetContextPrivate(context);
			if (ec->__DEFnames) {
				struct Vector* defnames = (struct Vector*)ec->__DEFnames;
				for (int i = 0; i < vectorSize(defnames); i++) {
					bd = vector_get_ptr(struct brotoDefpair, defnames, i);
					//Q. is it the DEF we search for, and node we replace, OR
					//   is it the node we search for, and DEF we replace?
					if (!strcmp(bd->name, defname)) {
						bd->node = node;
						found = 1;
						break;
					}
					if (bd->node == node) {
						bd->name = strdup(defname);
						found = 2;
						break;
					}
				}
			}
			if (!found) {
				//I guess its an add
				if (!ec->__DEFnames)
					ec->__DEFnames = newVector(struct brotoDefpair, 4);
				struct brotoDefpair bd2;
				memset(&bd2, 0, sizeof(struct brotoDefpair));
				bd2.node = node;
				bd2.name = strdup(defname);
				stack_push(struct brotoDefpair, (struct Vector*)ec->__DEFnames, bd2);
			}
			else {
				printf("\nIncorrect argument for updateNamedNode('DEFname').\n");
				JS_free(context, defname);
				return JS_FALSE;
			}
		}
		JS_free(context, defname);
	}
	else {
		printf("\nIncorrect argument format for updateNamedNode('DEFname').\n");
		return JS_FALSE;
	}

	return JS_TRUE;
}

static JSBool X3DExecutionContext_addRoute(JSContext* context, uintN argc, jsval* vp) {
	JSObject* obj = JS_THIS_OBJECT(context, vp);
	jsval* argv = JS_ARGV(context, vp);

	JSObject* fromNodeObj, * toNodeObj;
	JSClass* _cls[2];
	char * fromField, * toField;
	const char* _c_args =
		"SFNode fromNode, SFString fromEventOut, SFNode toNode, SFString toEventIn",
		* _c_format = "oSoS";
	JSString* fromFieldStringJS, * toFieldStringJS;
	struct X3D_Node* fromNode;
	struct X3D_Node* toNode;
	int fromOfs, toOfs, len;
	int fromtype, totype;
	int xxx;
	int myField;

	/* first, are there 4 arguments? */
	if (argc != 4) {
		printf("Problem with script - add/delete route command needs 4 parameters\n");
		return JS_FALSE;
	}

	/* get the arguments, and ensure that they are obj, string, obj, string */
	if (JS_ConvertArguments(context, argc, argv, _c_format,
		&fromNodeObj, &fromFieldStringJS, &toNodeObj, &toFieldStringJS)) {
		fromField = JS_EncodeString(context, fromFieldStringJS);
		toField = JS_EncodeString(context, toFieldStringJS);

		if ((_cls[0] = JS_GET_CLASS(context, fromNodeObj)) == NULL) {
			printf("JS_GetClass failed for arg 0 in doVRMLRoute called from %s.\n",
				"addRoute");
			return JS_FALSE;
		}
		if ((_cls[1] = JS_GET_CLASS(context, toNodeObj)) == NULL) {
			printf("JS_GetClass failed for arg 2 in doVRMLRoute called from %s.\n",
				"addRoute");
			return JS_FALSE;
		}

		/* make sure these are both SFNodes */
		if (memcmp("SFNode", (_cls[0])->name, strlen((_cls[0])->name)) != 0 &&
			memcmp("SFNode", (_cls[1])->name, strlen((_cls[1])->name)) != 0) {
			printf("\nArguments 0 and 2 must be SFNode in doVRMLRoute called from %s(%s): %s\n",
				"addRoute", _c_args, "addRoute");
			return JS_FALSE;
		}

		/* get the "private" data for these nodes. It will consist of a SFNodeNative structure */
		AnyNative* fromNative;
		AnyNative* toNative;
		if ((fromNative = (AnyNative*)JS_GetPrivateFw(context,fromNodeObj)) == NULL) {
			printf("problem getting native props first node\n");
			return JS_FALSE;
		}
		if ((toNative = (AnyNative*)JS_GetPrivateFw(context, toNodeObj)) == NULL) {
			printf("problem getting native props second node\n");
			return JS_FALSE;
		}

		/* get the "handle" for the actual memory pointer */
		fromNode = X3D_NODE(fromNative->v->sfnode);
		toNode = X3D_NODE(toNative->v->sfnode);

#ifdef JSVERBOSE
		printf("routing from a node of type %s to a node of type %s\n",
			stringNodeType(fromNode->_nodeType),
			stringNodeType(toNode->_nodeType));
#endif	
		struct X3D_Proto* ec;
		//ec = getExecutionContextFromCx(cx);
		ec = (struct X3D_Proto*)JS_GetContextPrivate(context);

		void* xroute;
		int nr = 0;
		xroute = addDeleteRoute0(ec, "addRoute", fromNode, fromField, toNode, toField);
		//find its index
		struct Vector* routes = (struct Vector*)ec->__ROUTES;
		int index = -1;
		for (int j = 0; j < vectorSize(routes); j++) {
			struct brotoRoute* route = vector_get(struct brotoRoute*, routes, j);
			if ((void*)route == (void*)xroute) {
				index = j;
				break;
			}
		}
		if (index > -1) {
			JSObject* _obj;
			//int* _index = (int*) MALLOC(void *, sizeof(int));
			_obj = JS_NewObject(context, &X3DRouteClass, NULL, obj); //could parent be context or RouteArray?
			ADD_ROOT(context, _obj)

			long long iindex = index + 1; //instead of malloc and free wrapper for long, just send a longlong on x64
			if (!JS_SetPrivateFw(context, _obj, (void*)iindex)) {
				printf("JS_SetPrivate failed in RouteArray.\n");
				return JS_FALSE;
			}
			JS_SET_RVAL(context, vp, OBJECT_TO_JSVAL(_obj));
		}
		JS_free(context, fromField);
		JS_free(context, toField);
	}
	else {
		printf("\nIncorrect argument format for %s(%s).\n",
			"addRoute", _c_args);
		return JS_FALSE;
	}
	return JS_TRUE;
}
static JSBool X3DExecutionContext_deleteRoute(JSContext* context, uintN argc, jsval* vp) {
	JSObject* obj = JS_THIS_OBJECT(context, vp);
	jsval* argv = JS_ARGV(context, vp);
	JSObject* routeObj;
	JSClass* _cls[1];
	const char* _c_args =
		"X3DRoute route",
		* _c_format = "o";

	/* first, are there 4 arguments? */
	if (argc != 1) {
		printf("Problem with script - delete route command needs 1 parameter\n");
		return JS_FALSE;
	}

	/* get the arguments, and ensure that they are obj, string, obj, string */
	if (JS_ConvertArguments(context, argc, argv, _c_format,
		&routeObj)) {

		if ((_cls[0] = JS_GET_CLASS(context, routeObj)) == NULL) {
			printf("JS_GetClass failed for arg 0 in deleteRoute \n");
			return JS_FALSE;
		}

		/* make sure these are both SFNodes */
		if (memcmp("X3DRoute", (_cls[0])->name, strlen((_cls[0])->name)) != 0 ) {
			printf("\nArguments 0 must be X3DRoute in deleteRoute \n");
			return JS_FALSE;
		}
		long long iindex;
		if ((iindex = (long long)JS_GetPrivateFw(context, routeObj)) == 0) {
			printf("problem getting native prop for route\n");
		}
		struct X3D_Proto* ec;
		//ec = getExecutionContextFromCx(cx);
		ec = (struct X3D_Proto*)JS_GetContextPrivate(context);

		void* xroute;
		int nr = 0;
		struct Vector* routes = (struct Vector*)ec->__ROUTES;
		int index = iindex -1;
		struct brotoRoute* broute = vector_get(struct brotoRoute*, routes, index);
		CRoutes_RemoveSimpleB(broute->from.node, broute->from.ifield, broute->from.builtIn, broute->to.node, broute->to.ifield, broute->to.builtIn, broute->ft);
		vector_remove_elem(struct brotoRoute*, routes, index);
	}
	else {
		printf("\nIncorrect argument format for deleteRoute.\n");
		return JS_FALSE;
	}
	return JS_TRUE;
}
struct X3D_Node* broto_search_DEFname(struct X3D_Proto* context, const char* name);
struct IMEXPORT* broto_search_IMPORTname(struct X3D_Proto* context, const char* name);
struct IMEXPORT* broto_search_EXPORTname(struct X3D_Proto* context, const char* name);

static JSBool
X3DExecutionContext_getImportedNode(JSContext* context, uintN argc, jsval* vp) {
	//has optional 2nd parameter in specs (importname,exportname)
	JSObject* obj = JS_NewObject(context, &SFNodeClass, NULL, NULL);
	ADD_ROOT(cx, obj)
		jsval* argv = JS_ARGV(context, vp);
	const char* _c_format = "S";
	JSString* js_c;
	char* _c;

	if (argc == 1 &&
		JS_ConvertArguments(context, argc, argv, _c_format, &js_c)) {
		_c = JS_EncodeString(context, js_c);
#ifdef JSVERBOSE
		printf("X3DExecutionContext_getNamedNode: obj = %u, str = \"%s\"\n",
			obj, _c);
#endif
		{
			struct X3D_Node* node = NULL;
			struct X3D_Proto* ec;
			//ec = getExecutionContextFromCx(cx);
			ec = (struct X3D_Proto*)JS_GetContextPrivate(context);
			//struct IMEXPORT* im = broto_search_IMPORTname(ec, _c);
			//if (im) node = im->nodeptr;
			// 
			//imports show up late. The main scene parser never sees the nodeptr in the inline, leaves it null.
			//then if you patiently wait for the inline to load, something has to go hunting for 
			// the imported node in the inline. broto_search_ALLnames has some code for that.
			int source;
			node = broto_search_ALLnames(ec, _c, &source);
			if (source == 0) node = NULL; //import must be source 2
			if (node != NULL) {
				AnyNative* lhs;
				if ((lhs = (AnyNative*)AnyNativeNew(FIELDTYPE_SFNode, NULL, NULL)) == NULL) {
					printf("AnyNativeNew failed in SFNodeConstr.\n");
					return JS_FALSE;
				}
				if (!JS_SetPrivateFw(context, obj, lhs)) {
					printf("JS_SetPrivate failed in SFNodeConstr.\n");
					return JS_FALSE;
				}
				//lhs->valueChanged = NULL; 
				lhs->v->sfnode = node;
			}
			else {
				printf("Incorrect argument for getImportedNode('DEFname').\n");
			}
		}
		JS_free(context, _c);
	}
	else {
		printf("\nIncorrect argument format for getImportedNode('DEFname').\n");
		return JS_FALSE;
	}

	JS_SET_RVAL(context, vp, OBJECT_TO_JSVAL(obj));
	return JS_TRUE;
}
static JSBool
X3DExecutionContext_removeImportedNode(JSContext* context, uintN argc, jsval* vp) {
	jsval* argv = JS_ARGV(context, vp);
	const char* _c_format = "S";
	JSString* js_c;
	char* _c;

	if (argc == 1 &&
		JS_ConvertArguments(context, argc, argv, _c_format, &js_c)) {
		_c = JS_EncodeString(context, js_c);
#ifdef JSVERBOSE
		printf("X3DExecutionContext_getNamedNode: obj = %u, str = \"%s\"\n",
			obj, _c);
#endif
		{
			struct X3D_Node* node = NULL;
			struct X3D_Proto* ec;
			//ec = getExecutionContextFromCx(cx);
			ec = (struct X3D_Proto*)JS_GetContextPrivate(context);
			struct IMEXPORT* im = broto_search_IMPORTname(ec, _c);

			if (im) {
				struct IMEXPORT* def;
				struct Vector* imports = (struct Vector*)ec->__IMPORTS;
				int k = 0;
				for (int i = 0; i < vectorSize(imports); i++) {
					def = vector_get(struct IMEXPORT*, imports, i);
					vector_set(struct IMEXPORT*, imports, i, def);
					if (im != def) k++;
				}
				imports->n = k;
			}
		}
		JS_free(context, _c);
	}
	else {
		printf("\nIncorrect argument format for getImportedNode('DEFname').\n");
		return JS_FALSE;
	}

	return JS_TRUE;
}


static JSBool
X3DExecutionContext_updateImportedNode(JSContext* context, uintN argc, jsval* vp) {
	//2023 interpretation of parameters:
	// string1 is the main scene local DEF name (the AS name)
	// string2 is the inline export name
	// no need for 3rd string for inline name:
	// - if you want to say which inline, use <inline name>.<export name> in string2
	// - otherwise if no "." it assumes that's the export name and will search for that in all inlines
	//
	jsval* argv = JS_ARGV(context, vp);
	const char* _c_format = "SS";
	JSString* js_1, * js_2, * js_3;
	JSObject* _ob2;
	struct X3D_Node* node = NULL;
	int found = 0;
	const char* as, * mxname, * impname;
	char* nline;
	struct IMEXPORT* mxp;


	if (argc == 2) {
		JS_ConvertArguments(context, argc, argv, "SS", &js_1, &js_2);

		as = JS_EncodeString(context, js_1);
		mxname = JS_EncodeString(context, js_2);
		impname = strdup(mxname);
		nline = NULL;
		const char* dot = strstr(mxname, ".");
		if (dot) {
			nline = strdup(mxname);
			nline[dot - mxname] = 0;
			impname = strdup(&dot[1]);
		}
		printf("as [%s] impname [%s] nline [%s]\n", as, impname, nline);
#ifdef JSVERBOSE
		printf("X3DExecutionContext_removeNamedNode: obj = %u, str = \"%s\"\n",
			obj, _c);
#endif

		struct X3D_Proto* ec;
		//ec = getExecutionContextFromCx(cx);
		ec = (struct X3D_Proto*)JS_GetContextPrivate(context);
		struct Vector* imports = (struct Vector*)ec->__IMPORTS;
		if (imports) {
			for (int i = 0; i < vectorSize(imports); i++) {
				mxp = vector_get(struct IMEXPORT*, imports, i);
				//Q. is it the DEF we search for, and node we replace, OR
				//   is it the node we search for, and DEF we replace?
				int inlineOK = !nline || !strcmp(nline, mxp->inlinename);
				int asOK = !strcmp(as, mxp->as);
				int impOK = !strcmp(impname, mxp->mxname);
				if(inlineOK && impOK){
					//if (!strcmp(nline, mxp->inlinename) && !strcmp(mxp->mxname, mxname)) {
					printf("updating import new as [%s] old import [%s] inline [%s]\n", as, impname, nline);
					mxp->as = strdup(as);
					found = 1;
					break;
				}
			}
		}
		if (!found) {
			//I guess its an add
			if (!ec->__IMPORTS)
				ec->__IMPORTS = newVector(struct IMEXPORT*, 4);
			mxp = (struct IMEXPORT*)malloc(sizeof(struct IMEXPORT));
			mxp->mxname = strdup(impname);
			mxp->as = strdup(as);
			mxp->inlinename = nline ? strdup(nline) : NULL;
			printf("adding import mapping as [%s] import [%s] inline [%s]\n", impname, as, nline);
			stack_push(struct IMEXPORT*, (struct Vector *)ec->__IMPORTS, mxp);
		}
		update_weakRoutes(ec);
		//JS_free(context, mxname); //? what's wrong
	}
	else {
		printf("\nIncorrect argument format for updateImportedNode('DEFname').\n");
		return JS_FALSE;
	}

	return JS_TRUE;
}


static JSBool
X3DScene_getExportedNode(JSContext* context, uintN argc, jsval* vp) {
	//has optional 2nd parameter in specs (importname,exportname)
	JSObject* obj = JS_NewObject(context, &SFNodeClass, NULL, NULL);
	ADD_ROOT(cx, obj)
		jsval* argv = JS_ARGV(context, vp);
	const char* _c_format = "S";
	JSString* js_c;
	char* _c;

	if (argc == 1 &&
		JS_ConvertArguments(context, argc, argv, _c_format, &js_c)) {
		_c = JS_EncodeString(context, js_c);
#ifdef JSVERBOSE
		printf("X3DExecutionContext_getNamedNode: obj = %u, str = \"%s\"\n",
			obj, _c);
#endif
		{
			struct X3D_Node* node = NULL;
			struct X3D_Proto* ec;
			//ec = getExecutionContextFromCx(cx);
			ec = (struct X3D_Proto*)JS_GetContextPrivate(context);
			int source = 0;
			struct IMEXPORT * ex = broto_search_EXPORTname(ec, _c);
			if (ex) node = ex->nodeptr;
			if (node != NULL) {
				AnyNative* lhs;
				if ((lhs = (AnyNative*)AnyNativeNew(FIELDTYPE_SFNode, NULL, NULL)) == NULL) {
					printf("AnyNativeNew failed in SFNodeConstr.\n");
					return JS_FALSE;
				}
				if (!JS_SetPrivateFw(context, obj, lhs)) {
					printf("JS_SetPrivate failed in SFNodeConstr.\n");
					return JS_FALSE;
				}
				//lhs->valueChanged = NULL; 
				lhs->v->sfnode = node;
			}
			else {
				printf("Incorrect argument for getExportedNode('DEFname').\n");
			}
		}
		JS_free(context, _c);
	}
	else {
		printf("\nIncorrect argument format for getExportedNode('DEFname').\n");
		return JS_FALSE;
	}

	JS_SET_RVAL(context, vp, OBJECT_TO_JSVAL(obj));
	return JS_TRUE;
}
static JSBool
X3DScene_removeExportedNode(JSContext* context, uintN argc, jsval* vp) {
	jsval* argv = JS_ARGV(context, vp);
	const char* _c_format = "S";
	JSString* js_c;
	char* _c;

	if (argc == 1 &&
		JS_ConvertArguments(context, argc, argv, _c_format, &js_c)) {
		_c = JS_EncodeString(context, js_c);
#ifdef JSVERBOSE
		printf("X3DExecutionContext_getNamedNode: obj = %u, str = \"%s\"\n",
			obj, _c);
#endif
		{
			struct X3D_Node* node = NULL;
			struct X3D_Proto* ec;
			//ec = getExecutionContextFromCx(cx);
			ec = (struct X3D_Proto*)JS_GetContextPrivate(context);
			struct IMEXPORT* ex = broto_search_EXPORTname(ec, _c);

			if (ex) {
				struct IMEXPORT* def;
				struct Vector* exports = (struct Vector*)ec->__EXPORTS;
				int k = 0;
				for (int i = 0; i < vectorSize(exports); i++) {
					def = vector_get(struct IMEXPORT*, exports, i);
					vector_set(struct IMEXPORT*, exports, i, def);
					if (ex != def) k++;
				}
				exports->n = k;
			}
		}
		JS_free(context, _c);
	}
	else {
		printf("\nIncorrect argument format for removeExportedNode('DEFname').\n");
		return JS_FALSE;
	}

	return JS_TRUE;
}


static JSBool
X3DScene_updateExportedNode(JSContext* context, uintN argc, jsval* vp) {
	jsval* argv = JS_ARGV(context, vp);
	const char* _c_format = "SS";
	JSString* js_1, * js_2, * js_3;
	JSObject* _ob2;
	struct X3D_Node* node = NULL;
	int found = 0;
	const char* as, * mxname, * nline;
	struct IMEXPORT* mxp;


	if (argc >= 2) {
		if (argc == 2)
			JS_ConvertArguments(context, argc, argv, "SS", &js_1, &js_2);
		if (argc == 3)
			JS_ConvertArguments(context, argc, argv, "SSS", &js_1, &js_2, &js_3);

		nline = JS_EncodeString(context, js_1);
		mxname = JS_EncodeString(context, js_2);
		as = mxname;
		if (argc == 3)
			as = JS_EncodeString(context, js_3);


#ifdef JSVERBOSE
		printf("X3DExecutionContext_removeNamedNode: obj = %u, str = \"%s\"\n",
			obj, _c);
#endif

		struct X3D_Proto* ec;
		//ec = getExecutionContextFromCx(cx);
		ec = (struct X3D_Proto*)JS_GetContextPrivate(context);
		struct Vector* exports = (struct Vector*)ec->__EXPORTS;
		if (exports) {
			for (int i = 0; i < vectorSize(exports); i++) {
				mxp = vector_get(struct IMEXPORT*, exports, i);
				//Q. is it the DEF we search for, and node we replace, OR
				//   is it the node we search for, and DEF we replace?
				if (!strcmp(nline, mxp->inlinename) && !strcmp(mxp->mxname, mxname)) {
					mxp->as = strdup(as);
					found = 1;
					break;
				}
			}
		}
		if (!found) {
			//I guess its an add
			if (!ec->__EXPORTS)
				ec->__EXPORTS = newVector(struct IMEXPORT*, 4);
			mxp = (struct IMEXPORT*)malloc(sizeof(struct IMEXPORT));
			mxp->mxname = strdup(mxname);
			mxp->as = strdup(as);
			mxp->inlinename = strdup(nline);
			stack_push(struct IMEXPORT*, (struct Vector*)ec->__EXPORTS, mxp);
		}
		update_weakRoutes(ec);
		//JS_free(context, mxname); //? what's wrong
	}
	else {
		printf("\nIncorrect argument format for updateImportedNode('DEFname').\n");
		return JS_FALSE;
	}

	return JS_TRUE;
}

static JSBool
X3DScene_getMetaData(JSContext* context, uintN argc, jsval* vp) {
	//has optional 2nd parameter in specs (importname,exportname)
	jsval* argv = JS_ARGV(context, vp);
	const char* _c_format = "S";
	JSString* js_c;
	char* name, *content;
	content = NULL;
	if (argc == 1 &&
		JS_ConvertArguments(context, argc, argv, _c_format, &js_c)) {
		name = JS_EncodeString(context, js_c);
#ifdef JSVERBOSE
		printf("X3DScene_getMetaData: obj = %u, str = \"%s\"\n",
			obj, _c);
#endif
		{
			struct X3D_Node* node = NULL;
			struct X3D_Proto* ec;
			//ec = getExecutionContextFromCx(cx);
			ec = (struct X3D_Proto*)JS_GetContextPrivate(context);
			if (ec->__META) {
				struct Vector* metalist = (struct Vector*)ec->__META;
				struct metarecord* mr;
				for (int i = 0; i < vectorSize(metalist); i++) {
					mr = vector_get_ptr(struct metarecord, metalist, i);
					if (!strcmp(mr->name, name)) {
						content = strdup(mr->content);
						break;
					}
				}
			}
		}
		JS_free(context, name);
	}
	else {
		printf("\nIncorrect argument format for Scene.getMetaData('name').\n");
		return JS_FALSE;
	}

	JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(JS_NewStringCopyZ(context, content)));
	return JS_TRUE;
}
static JSBool
X3DScene_setMetaData(JSContext* context, uintN argc, jsval* vp) {
	//has optional 2nd parameter in specs (importname,exportname)
	jsval* argv = JS_ARGV(context, vp);
	const char* _c_format = "SS";
	JSString* js_name, *js_content;
	char* name, * content;

	if (argc == 2 &&
		JS_ConvertArguments(context, argc, argv, _c_format, &js_name, &js_content)) {
		name = JS_EncodeString(context, js_name);
		content = JS_EncodeString(context, js_content);
#ifdef JSVERBOSE
		printf("X3DScene_getMetaData: obj = %u, str = \"%s\"\n",
			obj, _c);
#endif
		{
			struct X3D_Node* node = NULL;
			struct X3D_Proto* ec;
			//ec = getExecutionContextFromCx(cx);
			ec = (struct X3D_Proto*)JS_GetContextPrivate(context);
			if (!ec->__META)
				ec->__META = newVector(struct metarecord, 10);

			struct Vector* metalist = (struct Vector*)ec->__META;
			struct metarecord* mr;
			int done = FALSE;
			for (int i = 0; i < vectorSize(metalist); i++) {
				mr = vector_get_ptr(struct metarecord, metalist, i);
				if (!strcmp(mr->name, name)) {
					mr->content = strdup(content);
					done = TRUE;
					break;
				}
			}
			if (!done) {
				//add
				struct metarecord mr2;
				mr2.name = strdup(name);
				mr2.content = strdup(content);
				vector_pushBack(struct metarecord, metalist, mr2);
			}
		}
		JS_free(context, name);
		JS_free(context, content);
	}
	else {
		printf("\nIncorrect argument format for Scene.setMetaData('name','content').\n");
		return JS_FALSE;
	}

	return JS_TRUE;
}

static JSFunctionSpec (ExecutionContextFunctions)[] = {
	//executionContext
	{"addRoute", X3DExecutionContext_addRoute, 0},
	{"deleteRoute", X3DExecutionContext_deleteRoute, 0},
	{"createNode", X3DExecutionContext_createNode, 0},
	{"createProto", X3DExecutionContext_createProto, 0},
	{"getImportedNode", X3DExecutionContext_getImportedNode, 0},
	{"updateImportedNode", X3DExecutionContext_updateImportedNode, 0},
	{"removeImportedNode", X3DExecutionContext_removeImportedNode, 0},
	{"getNamedNode", X3DExecutionContext_getNamedNode, 0},
	{"updateNamedNode", X3DExecutionContext_updateNamedNode, 0},
	{"removeNamedNode", X3DExecutionContext_removeNamedNode, 0},
	//scene
	{"setMetaData", X3DScene_setMetaData, 0},
	{"getMetaData", X3DScene_getMetaData, 0},
	{"getExportedNode", X3DScene_getExportedNode, 0},
	{"updateExportedNode", X3DScene_updateExportedNode, 0},
	{"removeExportedNode", X3DScene_removeExportedNode, 0},
	{0}
};


static JSPropertySpec (ExecutionContextProperties)[] = {
	//executionContext
	{"specificationVersion", 0, JSPROP_ENUMERATE},
	{"encoding", 1, JSPROP_ENUMERATE},
	{"profile", 2, JSPROP_ENUMERATE},
	{"components", 3, JSPROP_ENUMERATE},
	{"worldURL", 4, JSPROP_ENUMERATE},
	{"rootNodes", 5, JSPROP_ENUMERATE},
	{"protos", 6, JSPROP_ENUMERATE},
	{"externprotos", 7, JSPROP_ENUMERATE},
	{"routes", 8, JSPROP_ENUMERATE},
	//scene
	//{"specificationVersion", 9, JSPROP_ENUMERATE}, //already done for executionContext above
	{"isScene", 9, JSPROP_ENUMERATE}, //else protoInstance. extra beyond specs - I think flux has it.
	{0,0,0}
};

//typedef struct _ExecutionContextNative {
//	struct X3D_Node *handle;
//} ExecutionContextNative;
typedef struct X3D_Node * ExecutionContextNative;

JSBool
ExecutionContextGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();

	//ExecutionContextNative *ptr;
	struct X3D_Proto *ptr, *ec;
	JSString *_str;
	jsval rval;
	jsval id;

	UNUSED(rval); //compiler warning mitigation


	if (!JS_IdToValue(cx,iid,&id)) {
		printf("JS_IdToValue failed in ExecutionContextGetProperty.\n");
		return JS_FALSE;
	}


	//if ((ptr = (ExecutionContextNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
	if ((ptr = (struct X3D_Proto*)JS_GetPrivateFw(cx, obj)) == NULL) {
			printf( "JS_GetPrivate failed in ExecutionContextGetProperty.\n");
		return JS_FALSE;
	}
	ec = ptr;

	int index = -1;
	if (JSVAL_IS_STRING(id)) {
		char* field = (char*)JS_EncodeString(cx, JSVAL_TO_STRING(id));
		index = lookup_tinyid(field, ExecutionContextProperties);
		//printf("ExecutionContextGetProperty %s %d\n", field, index);
	}
	else if (JSVAL_IS_INT(id)) {
		index = JSVAL_TO_INT(id);
	}
	switch(index){
		case 0: //specificationVersion string readonly
			{
				char cs[100];
				sprintf(cs,"{%d,%d,%d}",inputFileVersion[0],inputFileVersion[1],inputFileVersion[2]);
				_str = JS_NewStringCopyZ(cx,cs);
			}
			JS_SET_RVAL(cx,vp,STRING_TO_JSVAL(_str));
			break;
		case 1: //encoding string readonly
			//Valid values are "ASCII", "VRML", "XML", "BINARY", "SCRIPTED", "BIFS", "NONE" 
			_str = JS_NewStringCopyZ(cx, "not filled in yet sb. VRML or XML or .."); 
			JS_SET_RVAL(cx,vp,STRING_TO_JSVAL(_str));
			break;
		case 2: //profile ProfileInfo readonly
			{
				int index = gglobal()->Mainloop.scene_profile;

				JSObject *_obj;
				int* _index = MALLOC(int *, sizeof(int));
				_obj = JS_NewObject(cx,&ProfileInfoClass,NULL,obj);
				*_index = index;
				if (!JS_DefineProperties(cx, _obj, ProfileInfoProperties)) {
					printf( "JS_DefineProperties failed in ExecutionContextProfileInfoProperties.\n");
					return JS_FALSE;
				}

				if (!JS_SetPrivateFw(cx, _obj, (void*)_index)) {
					printf( "JS_SetPrivate failed in ExecutionContextProfileInfoArray.\n");
					return JS_FALSE;
				}

				JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(_obj));
			}
			break;
		case 3: //components ComponentInfoArray readonly
			{
				JSObject *_obj;
				//int ncomp;
				const int *_table = gglobal()->Mainloop.scene_components; //capabilitiesHandler_getProfileComponent(_index);
				//ncomp = capabilitiesHandler_getTableLength(_table);
				//malloc private not needed
				_obj = JS_NewObject(cx,&ComponentInfoArrayClass,NULL,obj);
				if (!JS_DefineProperties(cx, _obj, ComponentInfoArrayProperties)) {
					printf( "JS_DefineProperties failed in ExecutionContext_ComponentInfoArrayProperties.\n");
					return JS_FALSE;
				}
			
				if (!JS_SetPrivateFw(cx, _obj, (void*)_table)) {
					printf( "JS_SetPrivate failed in ExecutionContext_ComponentInfoArray.\n");
					return JS_FALSE;
				}
				JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(_obj));
			}
			break;
		case 4: //worldURL string readonly
			_str = JS_NewStringCopyZ(cx, gglobal()->Mainloop.url);
			//printf("mainloop.url= %s \n", gglobal()->Mainloop.url);
			JS_SET_RVAL(cx,vp,STRING_TO_JSVAL(_str));
			break;
		case 5: //rootNodes MFNode (readonly if !isScene, else rw)
			{
				JSObject *_obj;
				//printf("__nodes length %d", vectorSize(ec->__nodes)); //all nodes in context
				//printf(" __children.n %d\n", ec->__children.n); //rootnodes in context
				AnyNative* nany; 

				_obj = JS_NewObject(cx,&MFNodeClass,NULL,obj);
				//set private
				if ((nany = (AnyNative*)AnyNativeNew(FIELDTYPE_MFNode, (anyVrml*) & ec->__children, 0)) == NULL) {
					printf("AnyNativeNew failed in ExecutionContext..\n");
					return JS_FALSE;
				}

				if (!JS_SetPrivateFw(cx, _obj, nany)) {
					printf("JS_SetPrivate failed in ExecutionContext..\n");
					return JS_FALSE;
				}

				JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(_obj));
			}
			break;
		case 6: //protos protoDeclarationArray  rw
		{ 
			JSObject* _obj;
			//malloc private not needed
			_obj = JS_NewObject(cx, &ProtoDeclarationArrayClass, NULL, obj);
			if (0) if (!JS_DefineProperties(cx, _obj, RouteArrayProperties)) {
				printf("JS_DefineProperties failed in ExecutionContext_X3DRouteArrayProperties.\n");
				return JS_FALSE;
			}
			if (!JS_SetPrivateFw(cx, _obj, ptr->__protoDeclares)) {
				printf( "JS_SetPrivate failed in ExecutionContext_X3DRouteArray.\n");
				return JS_FALSE;
			}
			JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(_obj));
			
		}
		break;

		case 7: //externprotos externProtoDeclarationArray rw
		{
			JSObject* _obj;
			//malloc private not needed
			_obj = JS_NewObject(cx, &ProtoDeclarationArrayClass, NULL, obj);
			if (!JS_SetPrivateFw(cx, _obj, ptr->__externProtoDeclares)) {
				printf("JS_SetPrivate failed in ExecutionContext_X3DRouteArray.\n");
				return JS_FALSE;
			}
			JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(_obj));

		}
		break;
		case 8: //routes RouteArray readonly
			{
				JSObject *_obj;
				//malloc private not needed
				_obj = JS_NewObject(cx,&RouteArrayClass,NULL,obj);
				if(0) if (!JS_DefineProperties(cx, _obj, RouteArrayProperties)) {
					printf( "JS_DefineProperties failed in ExecutionContext_X3DRouteArrayProperties.\n");
					return JS_FALSE;
				}
					//if (!JS_SetPrivateFw(cx, _obj, (void*)_table)) {
				//	printf( "JS_SetPrivate failed in ExecutionContext_X3DRouteArray.\n");
				//	return JS_FALSE;
				//}
				JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(_obj));
			}
			break;
		case 9: //isScene readonly (extra to specs)
			//once brotos are working then the main scene broto will need a flag to say it's a scene
			JS_SET_RVAL(cx,vp,BOOLEAN_TO_JSVAL(JS_TRUE));
			break;
	}

	return JS_TRUE;
}


JSBool
ExecutionContextSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();
	//can I, should I force it to read-only this way?
	return JS_FALSE;
}
static JSClass ExecutionContextClass = {
    "ExecutionContext",
    JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,
    JS_DeletePropertyStub,
    ExecutionContextGetProperty, 
	ExecutionContextSetProperty,
    JS_EnumerateStub,
    JS_ResolveStub,
    JS_ConvertStub,
    JS_FinalizeStub
};


static JSPropertySpec (BrowserProperties)[] = {
	{"name", 0, JSPROP_ENUMERATE},
	{"version", 1, JSPROP_ENUMERATE},
	{"currentSpeed", 2, JSPROP_ENUMERATE},
	{"currentFrameRate", 3, JSPROP_ENUMERATE},
	{"description", 4, JSPROP_ENUMERATE},
	{"supportedComponents", 5, JSPROP_ENUMERATE},
	{"supportedProfiles", 6, JSPROP_ENUMERATE},
	{"currentScene", 7, JSPROP_ENUMERATE},
	{0}
};
struct X3D_Node* getExecutionContextFromCx(void *cx) {
	struct CRscriptStruct* sc;
	struct X3D_Node* executionContext = NULL; //its an X3D_Proto struct, which represents Scene, Proto (declare/instance), and Inline
	int nscript = getScriptControlCount();
	for (int i = 0; i <= nscript; i++) {
		sc = getScriptControlIndex(i);
		if (sc->cx == cx) {
			executionContext = sc->script->ShaderScriptNode->_executionContext;
			break;
		}
	}
	return executionContext;
}
JSBool
BrowserGetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid,  JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();

	BrowserNative *ptr;
	struct X3D_Proto* ec;

	jsdouble d;
	JSString *_str;
	jsval rval;
	jsval id;

	UNUSED(rval); // compiler warning mitigation


	if (!JS_IdToValue(cx,iid,&id)) {
		printf("JS_IdToValue failed in BrowserGetProperty.\n");
		return JS_FALSE;
	}

	//right now we don't need/use the ptr to BrowserNative which is a stub struct, 
	//because browser is conceptually a global static singleton 
	//(or more precisely 1:1 with a gglobal[i] 'browser instance' for things like framerate, 
	// and 1:1 with static for unchanging things like browser version, components and profiles supported), 
	//and in practice all the bits and pieces are scattered throughout freewrl
	//but for fun we'll get it:
	if ((ptr = (BrowserNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
		printf( "JS_GetPrivate failed in BrowserGetProperty.\n");
		return JS_FALSE;
	}
	ec = (struct X3D_Proto*)JS_GetContextPrivate(cx);

	int index = -1;
	if (JSVAL_IS_INT(id))
		index = JSVAL_TO_INT(id);
	else
		index = lookup_tinyid(JS_EncodeString(cx, JSVAL_TO_STRING(id)), BrowserProperties);

	switch (index) {
		case 0: //name
			_str = JS_NewStringCopyZ(cx,BrowserName);
			JS_SET_RVAL(cx,vp,STRING_TO_JSVAL(_str));
			break;
		case 1: //version
			_str = JS_NewStringCopyZ(cx, libFreeWRL_get_version());
			JS_SET_RVAL(context,vp,STRING_TO_JSVAL(_str));
			break;
		case 2: //currentSpeed
			/* get the variable updated */
			getCurrentSpeed();
			d = gglobal()->Mainloop.BrowserSpeed;
			if (JS_NewNumberValue(cx, d, vp) == JS_FALSE) {
				printf("JS_NewDouble failed for %f in BrowserGetProperty.\n",d);
				return JS_FALSE;
			}
			break;
		case 3: //currentFrameRate
			d = gglobal()->Mainloop.BrowserFPS;
			if (JS_NewNumberValue(cx, d, vp) == JS_FALSE) {
				printf("JS_NewDouble failed for %f in BrowserGetProperty.\n",d);
				return JS_FALSE;
			}
			break;
		case 4: //description
			_str = JS_NewStringCopyZ(cx, get_status());
			JS_SET_RVAL(context,vp,STRING_TO_JSVAL(_str));
			break;
		case 5: //supportedComponents
			{
				JSObject *_obj;
				//malloc private not needed
				_obj = JS_NewObject(cx,&ComponentInfoArrayClass,NULL,obj);
				if(0) if (!JS_DefineProperties(cx, _obj, ComponentInfoArrayProperties)) {
					printf( "JS_DefineProperties failed in ComponentInfoArrayProperties.\n");
					return JS_FALSE;
				}
				//if (!JS_DefineFunctions(cx, _obj, ProfileInfoArrayFunctions)) {
				//	printf( "JS_DefineProperties failed in ExecutionContextFunctions.\n");
				//	return JS_FALSE;
				//}
				if (!JS_SetPrivateFw(cx, _obj, (void*)capabilitiesHandler_getCapabilitiesTable())) {
					printf( "JS_SetPrivate failed in ExecutionContext.\n");
					return JS_FALSE;
				}

				JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(_obj));
			}
			break;

		case 6: //supportedProfiles
			{
				JSObject *_obj;
				//malloc private not needed
				_obj = JS_NewObject(cx,&ProfileInfoArrayClass,NULL,obj);
				if (!JS_DefineProperties(cx, _obj, ProfileInfoArrayProperties)) {
					printf( "JS_DefineProperties failed in ExecutionContextProperties.\n");
					return JS_FALSE;
				}
				//if (!JS_DefineFunctions(cx, _obj, ProfileInfoArrayFunctions)) {
				//	printf( "JS_DefineProperties failed in ExecutionContextFunctions.\n");
				//	return JS_FALSE;
				//}
				//set private not needed
				//if (!JS_SetPrivateFw(cx, _obj, ec)) {
				//	printf( "JS_SetPrivate failed in ExecutionContext.\n");
				//	return JS_FALSE;
				//}

				JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(_obj));
			}
			break;
		case 7: //currentScene
#ifdef sceneIsBroto  
			//someday soon I hope, the ScriptNode._executionContext might be working, 
			//and give you the currentScene native pointer to put as PRIVATE in BrowserNative
#else
			//in theory it's rootNode() in
			//struct X3D_Group *rootNode()
			//H: I have to return an ExecutionContextNative here with its guts set to our rootNode or ???
			{
				JSObject *_obj;
				_obj = JS_NewObject(cx,&ExecutionContextClass,NULL,obj);
				//if(!ec)
				//	ec = (struct X3D_Node*)rootNode(); //change this to (Script)._executionContext when brotos working fully
				if(0) if (!JS_DefineProperties(cx, _obj, ExecutionContextProperties)) {
					printf( "JS_DefineProperties failed in ExecutionContextProperties.\n");
					return JS_FALSE;
				}
				if(0) if (!JS_DefineFunctions(cx, _obj, ExecutionContextFunctions)) {
					printf( "JS_DefineProperties failed in ExecutionContextFunctions.\n");
					return JS_FALSE;
				}

				if (!JS_SetPrivateFw(cx, _obj, ec)) {
					printf( "JS_SetPrivate failed in ExecutionContext.\n");
					return JS_FALSE;
				}

				JS_SET_RVAL(cx,vp,OBJECT_TO_JSVAL(_obj));

			}

#endif
		}

	return JS_TRUE;
}


JSBool
BrowserSetProperty(JSContext *cx, JS::Handle<JSObject*> hobj, JS::Handle<jsid> hiid, JSBool strict, JS::MutableHandle<JS::Value> hvp){
	JSObject *obj = *hobj.address();
	jsid iid = *hiid.address();
	jsval *vp = hvp.address();

	BrowserNative *ptr;
	jsval _val;
	JSString *ss;
	char *cs;

	jsval id;
	if (!JS_IdToValue(cx,iid,&id)) {
		printf("JS_IdToValue failed in BrowserSetProperty.\n");
		return JS_FALSE;
	}


	if ((ptr = (BrowserNative *)JS_GetPrivateFw(cx, obj)) == NULL) {
		printf( "JS_GetPrivate failed in BrowserSetProperty.\n");
		return JS_FALSE;
	}
	//ptr->valueChanged++;

	if (!JS_ConvertValue(cx, *vp, JSTYPE_STRING, &_val)) {
		printf( "JS_ConvertValue failed in BrowserSetProperty.\n");
		return JS_FALSE;
	}

	if (JSVAL_IS_INT(id)) {
		switch (JSVAL_TO_INT(id)) {
		case 0:
		case 1:
		case 2:
		case 3:
			return JS_FALSE;
		case 4: //description
			ss = JS_ValueToString(cx, _val);
			cs = JS_EncodeString(cx, ss);
			update_status(cs);  //script node setting the statusbar text
			break;
		case 5:
		case 6:
		case 7:
			return JS_FALSE;

		}
	}
	return JS_TRUE;
}



#endif

extern "C" {

///* for setting field values to the output of a CreateVrml style of call */
///* it is kept at zero, unless it has been used. Then it is reset to zero */
//jsval JSCreate_global_return_val;
typedef struct pjsVRMLBrowser{
	int ijunk;

	jsval JSCreate_global_return_val;


}* ppjsVRMLBrowser;
void *jsVRMLBrowser_constructor(){
	void *v = MALLOC(void *, sizeof(struct pjsVRMLBrowser));
	memset(v,0,sizeof(struct pjsVRMLBrowser));
	return v;
}
void jsVRMLBrowser_init(struct iiglobal::tjsVRMLBrowser *t){
	//public
	//private
	t->prv = jsVRMLBrowser_constructor();
	{
		ppjsVRMLBrowser p = (ppjsVRMLBrowser)t->prv;
		/* Script name/type table */

		t->JSCreate_global_return_val = &p->JSCreate_global_return_val;

	}

}
} //extern "C"

//	ppjsVRMLBrowser p = (ppjsVRMLBrowser)gglobal()->jsVRMLBrowser.prv;
/* we add/remove routes with this call */
void jsRegisterRoute(
	struct X3D_Node* from, int fromOfs,
	struct X3D_Node* to, int toOfs,
	int len, const char *adrem) {
	int ad;

	if (strcmp("addRoute",adrem) == 0) 
		ad = 1;
	else ad = 0;

 	CRoutes_Register(ad, from, fromOfs, to, toOfs , len, 
 		 returnInterpolatorPointer(to->_nodeType), 0, 0);
}
 

/* used in loadURL*/
void conCat (char *out, char *in) {

	while (strlen (in) > 0) {
		strcat (out," :loadURLStringBreak:");
		while (*out != '\0') out++;

		if (*in == '[') in++;
		while ((*in != '\0') && (*in == ' ')) in++;
		if (*in == '"') {
			in++;
			/* printf ("have the initial quote string here is %s\n",in); */
			while (*in != '"') { *out = *in; out++; in++; }
			*out = '\0';
			/* printf ("found string is :%s:\n",tfilename); */
		}

		/* skip along to the start of the next name */
		if (*in == '"') in++;
		if (*in == ',') in++;
		if (*in == ']') in++; /* this allows us to leave */
	}
}



void createLoadUrlString(char *out, int outLen, char *url, char *param) {
	int commacount1;
	int commacount2;
	char *tptr;

	/* mimic the EAI loadURL, java code is:
        // send along sizes of the Strings
        SysString = "" + url.length + " " + parameter.length;
                
        for (count=0; count<url.length; count++) {
                SysString = SysString + " :loadURLStringBreak:" + url[count];
        }       

        for (count=0; count<parameter.length; count++) {
                SysString = SysString + " :loadURLStringBreak:" + parameter[count];
        }
	*/

	/* find out how many elements there are */

	commacount1 = 0; commacount2 = 0;
	tptr = url; while (*tptr != '\0') { if (*tptr == '"') commacount1 ++; tptr++; }
	tptr = param; while (*tptr != '\0') { if (*tptr == '"') commacount2 ++; tptr++; }
	commacount1 = commacount1 / 2;
	commacount2 = commacount2 / 2;

	if ((int)(strlen(url) +
		strlen(param) +
		(commacount1 * strlen (" :loadURLStringBreak:")) +
		(commacount2 * strlen (" :loadURLStringBreak:"))) > (outLen - 20)) {
		printf ("createLoadUrlString, string too long\n");
		return;
	}

	sprintf (out,"%d %d",commacount1,commacount2);
	
	/* go to the end of this string */
	while (*out != '\0') out++;

	/* go through the elements and find which (if any) url exists */	
	conCat (out,url);
	while (*out != '\0') out++;
	conCat (out,param);
}


JSBool
VrmlBrowserInit(JSContext *context, JSObject *globalObj, BrowserNative *brow)
{
	JSObject *obj;
	ttglobal tg = gglobal();
	*(jsval *)tg->jsVRMLBrowser.JSCreate_global_return_val = INT_TO_JSVAL(0);

	#ifdef JSVERBOSE
		printf("VrmlBrowserInit\n");
	#endif

	obj = JS_DefineObject(context, globalObj, "Browser", &BrowserClass, NULL, 
			JSPROP_ENUMERATE | JSPROP_PERMANENT);
	if (!JS_DefineFunctions(context, obj, BrowserFunctions)) {
		printf( "JS_DefineFunctions failed in VrmlBrowserInit.\n");
		return JS_FALSE;
	}
#ifdef X3DBROWSER

	if (!JS_DefineProperties(context, obj, BrowserProperties)) {
		printf( "JS_DefineProperties failed in VrmlBrowserInit.\n");
		return JS_FALSE;
	}
#endif
	if (!JS_SetPrivateFw(context, obj, brow)) {
		printf( "JS_SetPrivate failed in VrmlBrowserInit.\n");
		return JS_FALSE;
	}
	return JS_TRUE;
}


JSBool
VrmlBrowserGetName(JSContext *context, uintN argc, jsval *vp) {
	JSObject *obj = JS_THIS_OBJECT(context,vp);
	jsval *argv = JS_ARGV(context,vp);

	JSString *_str;

	UNUSED(obj);
	UNUSED(argc);
	UNUSED(argv);

	_str = JS_NewStringCopyZ(context,BrowserName);
	JS_SET_RVAL(context,vp,STRING_TO_JSVAL(_str));

	return JS_TRUE;
}


/* get the string stored in FWVER into a jsObject */
JSBool
VrmlBrowserGetVersion(JSContext *context, uintN argc, jsval *vp) {
	JSObject *obj = JS_THIS_OBJECT(context,vp);
	jsval *argv = JS_ARGV(context,vp);

	JSString *_str;

	UNUSED(obj);
	UNUSED(argc);
	UNUSED(argv);

	_str = JS_NewStringCopyZ(context, libFreeWRL_get_version());
	JS_SET_RVAL(context,vp,STRING_TO_JSVAL(_str));
	return JS_TRUE;
}


JSBool
VrmlBrowserGetCurrentSpeed(JSContext *context, uintN argc, jsval *vp) {
	JSObject *obj = JS_THIS_OBJECT(context,vp);
	jsval *argv = JS_ARGV(context,vp);

	JSString *_str;
	char string[1000];

	UNUSED(obj);
	UNUSED(argc);
	UNUSED(argv);

	/* get the variable updated */
	getCurrentSpeed();
	sprintf (string,"%f",gglobal()->Mainloop.BrowserSpeed);
	_str = JS_NewStringCopyZ(context,string);
        JS_SET_RVAL(context,vp,STRING_TO_JSVAL(_str));
	return JS_TRUE;
}


JSBool
VrmlBrowserGetCurrentFrameRate(JSContext *context, uintN argc, jsval *vp) {
	JSObject *obj = JS_THIS_OBJECT(context,vp);
	jsval *argv = JS_ARGV(context,vp);

	JSString *_str;
	char FPSstring[1000];

	UNUSED(obj);
	UNUSED(argc);
	UNUSED(argv);

	sprintf (FPSstring,"%6.2f",gglobal()->Mainloop.BrowserFPS);
	_str = JS_NewStringCopyZ(context,FPSstring);
        JS_SET_RVAL(context,vp,STRING_TO_JSVAL(_str));
	return JS_TRUE;
}


JSBool
VrmlBrowserGetWorldURL(JSContext *context, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(context,vp);
        jsval *argv = JS_ARGV(context,vp);

	JSString *_str;

	UNUSED(obj);
	UNUSED(argc);
	UNUSED(argv);

	_str = JS_NewStringCopyZ(context,BrowserFullPath);
        JS_SET_RVAL(context,vp,STRING_TO_JSVAL(_str));
	return JS_TRUE;
}


JSBool
VrmlBrowserReplaceWorld(JSContext *context, uintN argc, jsval *vp) {
        jsval *argv = JS_ARGV(context,vp);
	JSObject *_obj;
	JSString *_str;
	JSClass *_cls;
	jsval _rval = INT_TO_JSVAL(0);
	const char *_c_args = "MFNode nodes",
		*_c_format = "o";
	char *_costr,*tptr;

	if (JS_ConvertArguments(context, argc, argv, _c_format, &_obj)) {
		if ((_cls = JS_GET_CLASS(context, _obj)) == NULL) {
			printf("JS_GetClass failed in VrmlBrowserReplaceWorld.\n");
			return JS_FALSE;
		}

		if (memcmp("MFNode", _cls->name, strlen(_cls->name)) != 0) {
			printf( "\nIncorrect argument in VrmlBrowserReplaceWorld.\n");
			return JS_FALSE;
		}
		_str = JS_ValueToString(context, argv[0]);
		_costr = JS_EncodeString(context,_str);

		/* sanitize string, for the EAI_RW call (see EAI_RW code) */
		tptr = _costr;
		while (*tptr != '\0') {
			if(*tptr == '[') *tptr = ' ';
			if(*tptr == ']') *tptr = ' ';
			if(*tptr == ',') *tptr = ' ';
			tptr++;
		}
		EAI_RW(_costr);

		JS_free(context,_costr);
	} else {
		printf( "\nIncorrect argument format for replaceWorld(%s).\n", _c_args);
		return JS_FALSE;
	}
	JS_SET_RVAL(context,vp,_rval);

	return JS_TRUE;
}

JSBool
VrmlBrowserLoadURL(JSContext *context, uintN argc, jsval *vp) {
        jsval *argv = JS_ARGV(context,vp);
	JSObject *_obj[2];
	JSString *_str[2];
	JSClass *_cls[2];
	const char *_c_args = "MFString url, MFString parameter",
		*_c_format = "o o";
	#define myBufSize 2000
	char *_costr[2];
	char myBuf[myBufSize];

	if (JS_ConvertArguments(context, argc, argv, _c_format, &(_obj[0]), &(_obj[1]))) {
		if ((_cls[0] = JS_GET_CLASS(context, _obj[0])) == NULL) {
			printf( "JS_GetClass failed for arg 0 in VrmlBrowserLoadURL.\n");
			return JS_FALSE;
		}
		if ((_cls[1] = JS_GET_CLASS(context, _obj[1])) == NULL) {
			printf( "JS_GetClass failed for arg 1 in VrmlBrowserLoadURL.\n");
			return JS_FALSE;
		}
		if (memcmp("MFString", (_cls[0])->name, strlen((_cls[0])->name)) != 0 &&
			memcmp("MFString", (_cls[1])->name, strlen((_cls[1])->name)) != 0) {
			printf( "\nIncorrect arguments in VrmlBrowserLoadURL.\n");
			return JS_FALSE;
		}
		_str[0] = JS_ValueToString(context, argv[0]);
		_costr[0] = JS_EncodeString(context,_str[0]);

		_str[1] = JS_ValueToString(context, argv[1]);
		_costr[1] = JS_EncodeString(context,_str[1]);

		/* we use the EAI code for this - so reformat this for the EAI format */
		{
			//extern struct X3D_Anchor EAI_AnchorNode;  /* win32 C doesnt like new declarations in the middle of executables - start a new scope {} and put dec at top */

			/* make up the URL from what we currently know */
			createLoadUrlString(myBuf,myBufSize,_costr[0], _costr[1]);
			createLoadURL(myBuf);

			/* now tell the fwl_RenderSceneUpdateScene that BrowserAction is requested... */
			setAnchorsAnchor( get_EAIEventsIn_AnchorNode()); //&gglobal()->EAIEventsIn.EAI_AnchorNode;
		}
		gglobal()->RenderFuncs.BrowserAction = TRUE;

		JS_free(context,_costr[0]);
		JS_free(context,_costr[1]);
	} else {
		printf( "\nIncorrect argument format for loadURL(%s).\n", _c_args);
		return JS_FALSE;
	}
	JS_SET_RVAL(context,vp,INT_TO_JSVAL(0)); //JSVAL_ZERO); 

	return JS_TRUE;
}


JSBool
VrmlBrowserSetDescription(JSContext *context, uintN argc, jsval *vp) {
        jsval *argv = JS_ARGV(context,vp);
	JSString *js_c;
	const char *_c_format = "S", *_c_args = "SFString description";
	char *_c;

	UNUSED(_c); // compiler warning mitigation

	if (argc == 1 &&
		JS_ConvertArguments(context, argc, argv, _c_format, &js_c)) {
			/* _c = JS_EncodeString(context,js_c);
			...why encode the string when we just have to JS_free it later? */

		/* we do not do anything with the description. If we ever wanted to, it is in _c */
		JS_SET_RVAL(context,vp,INT_TO_JSVAL(0)); //JSVAL_ZERO);
	} else {
		printf( "\nIncorrect argument format for setDescription(%s).\n", _c_args);
		return JS_FALSE;
	}
	return JS_TRUE;
}

//new May 2022 based on jsVRML_SFClasses_sm.cpp SFNodeConstr handling of js new SfNode('Shape{}');

JSBool
VrmlBrowserCreateVrmlFromString(JSContext* context, uintN argc, jsval* vp) {
	//JSObject* obj = JS_THIS_OBJECT(context, vp);
	JSObject* obj = JS_NewObject(context, &MFNodeClass, NULL, NULL);
	ADD_ROOT(cx, obj)

	jsval* argv = JS_ARGV(context, vp);
	const char* _c_format = "S", * _c_args = "SFString vrmlSyntax";
	JSString* js_c;

	char* _c;

	/* for the return of the nodes */
	struct X3D_Group* retGroup;
	struct Multi_Node* newHandle = NULL;

	if (argc == 1 &&
		JS_ConvertArguments(context, argc, argv, _c_format, &js_c)) {
		_c = JS_EncodeString(context, js_c);

#ifdef JSVERBOSE
		printf("VrmlBrowserCreateVrmlFromString: obj = %u, str = \"%s\"\n",
			obj, _c);
#endif

		{
			resource_item_t* res = resource_create_from_string(_c);
			struct X3D_Group* myGroup = (struct X3D_Group*)createNewX3DNode(NODE_Group);
			res->whereToPlaceData = myGroup;
			res->ectx = JS_GetContextPrivate(context); //executionContext the script is in, stored using JS_SetContextPrivate 
			res->offsetFromWhereToPlaceData = (int)offsetof(struct X3D_Group, children);
			res->media_type = resm_vrml;
			res->parsed_request = strdup("From the EAI bootcamp of life ");
			parser_process_res_VRML_X3D(res);
			newHandle = &(myGroup->children);

			AnyNative* lhs;
			if ((lhs = (AnyNative*)AnyNativeNew(FIELDTYPE_MFNode, NULL, NULL)) == NULL) {
				printf("AnyNativeNew failed in SFNodeConstr.\n");
				return JS_FALSE;
			}
			if (!JS_SetPrivateFw(context, obj, lhs)) {
				printf("JS_SetPrivate failed in SFNodeConstr.\n");
				return JS_FALSE;
			}
			//lhs->valueChanged = NULL; 
			lhs->v->mfnode = *newHandle;
		}

		JS_free(context, _c);
	}
	else {
		printf("\nIncorrect argument format for createVrmlFromString(%s).\n", _c_args);
		return JS_FALSE;
	}

	JS_SET_RVAL(context, vp, OBJECT_TO_JSVAL(obj));
	return JS_TRUE;
}




JSBool
VrmlBrowserCreateX3DFromString(JSContext* context, uintN argc, jsval* vp) {
	//JSObject* obj = JS_THIS_OBJECT(context, vp);
	JSObject* obj = JS_NewObject(context, &MFNodeClass, NULL, NULL);
	ADD_ROOT(cx, obj)

		jsval* argv = JS_ARGV(context, vp);
	const char* _c_format = "S", * _c_args = "SFString x3dSyntax";
	JSString* js_c;

	char* _c;

	/* for the return of the nodes */
	struct X3D_Group* retGroup;
	struct Multi_Node* newHandle = NULL;

	if (argc == 1 &&
		JS_ConvertArguments(context, argc, argv, _c_format, &js_c)) {
		_c = JS_EncodeString(context, js_c);

#ifdef JSVERBOSE
		printf("VrmlBrowserCreateVrmlFromString: obj = %u, str = \"%s\"\n",
			obj, _c);
#endif

		{
			resource_item_t* res = resource_create_from_string(_c);
			struct X3D_Group* myGroup = (struct X3D_Group*)createNewX3DNode(NODE_Group);
			res->whereToPlaceData = myGroup;
			res->ectx = JS_GetContextPrivate(context); //executionContext the script is in, stored using JS_SetContextPrivate 
			res->offsetFromWhereToPlaceData = (int)offsetof(struct X3D_Group, children);
			res->media_type = resm_x3d;
			res->parsed_request = strdup("From the EAI bootcamp of life ");
			parser_process_res_VRML_X3D(res);
			newHandle = &(myGroup->children);

			AnyNative* lhs;
			if ((lhs = (AnyNative*)AnyNativeNew(FIELDTYPE_MFNode, NULL, NULL)) == NULL) {
				printf("AnyNativeNew failed in SFNodeConstr.\n");
				return JS_FALSE;
			}
			if (!JS_SetPrivateFw(context, obj, lhs)) {
				printf("JS_SetPrivate failed in SFNodeConstr.\n");
				return JS_FALSE;
			}
			//lhs->valueChanged = NULL; 
			lhs->v->mfnode = *newHandle;
		}

		JS_free(context, _c);
	}
	else {
		printf("\nIncorrect argument format for createX3dFromString(%s).\n", _c_args);
		return JS_FALSE;
	}

	JS_SET_RVAL(context, vp, OBJECT_TO_JSVAL(obj));
	return JS_TRUE;
}

JSBool
VrmlBrowserCreateVrmlFromURL(JSContext *context, uintN argc, jsval *vp) {
        jsval *argv = JS_ARGV(context,vp);
	jsval _my_rval;
	jsval *rval = &_my_rval;

	JSString *_str[2];
	JSClass *_cls[2];
	SFNodeNative *oldPtr;
	char *fieldStr,
		*_costr0;
	struct X3D_Node *myptr;
	#define myFileSizeLimit 4000

/* DJ Tue May  4 21:25:15 BST 2010 Old stuff, no longer applicable
	int count;
	int offset;
	int fromtype;
	int xxx;
	int myField;
	char *address;
	struct X3D_Group *subtree;
*/
	resource_item_t *res = NULL;
	int fieldInt;
	int offs;
	int type;
	int accessType;
	struct Multi_String url;


	#ifdef JSVERBOSE
	printf ("JS start of createVrmlFromURL\n");
	#endif

	/* rval is always zero, so lets just set it */
	*rval = INT_TO_JSVAL(0); //JSVAL_ZERO;

	/* first parameter - expect a MFString Object here */
	//if (JSVAL_IS_OBJECT(argv[0])) {
	if (argv[0].isObject()) {
		if ((_cls[0] = JS_GET_CLASS(context, JSVAL_TO_OBJECT(argv[0]))) == NULL) {
                        printf( "JS_GetClass failed for arg 0 in VrmlBrowserLoadURL.\n");
                        return JS_FALSE;
                }
	} else {
		printf ("VrmlBrowserCreateVrmlFromURL - expect first parameter to be an object\n");
		return JS_FALSE;
	}

	/* second parameter - expect a SFNode Object here */
	//if (JSVAL_IS_OBJECT(argv[1])) {
	if (argv[1].isObject()) {
		if ((_cls[1] = JS_GET_CLASS(context, JSVAL_TO_OBJECT(argv[1]))) == NULL) {
                        printf( "JS_GetClass failed for arg 1 in VrmlBrowserLoadURL.\n");
                        return JS_FALSE;
                }
	} else {
		printf ("VrmlBrowserCreateVrmlFromURL - expect first parameter to be an object\n");
		return JS_FALSE;
	}

	#ifdef JSVERBOSE
	printf ("JS createVrml - step 2\n");
	printf ("JS create - we should havve a MFString and SFNode, have :%s: :%s:\n",(_cls[0])->name, (_cls[1])->name);
	#endif

	/* make sure these 2 objects are really MFString and SFNode */
	if (memcmp("MFString", (_cls[0])->name, strlen((_cls[0])->name)) != 0 &&
		memcmp("SFNode", (_cls[1])->name, strlen((_cls[1])->name)) != 0) {
		printf( "Incorrect arguments in VrmlBrowserLoadURL.\n");
		return JS_FALSE;
	}

	/* third parameter should be a string */
	if (JSVAL_IS_STRING(argv[2])) {
		_str[1] = JSVAL_TO_STRING(argv[2]);
		fieldStr = JS_EncodeString(context,_str[1]);

		#ifdef JSVERBOSE
		printf ("field string is :%s:\n",fieldStr); 
		#endif
	 } else {
		printf ("Expected a string in createVrmlFromURL\n");
		return JS_FALSE;
	}

	#ifdef JSVERBOSE
	printf ("passed object type tests\n");
	#endif

	/* get the URL listing as a string */
	_str[0] = JS_ValueToString(context, argv[0]);
	_costr0 = JS_EncodeString(context,_str[0]);

	#ifdef JSVERBOSE
	printf ("URL string is %s\n",_costr0);
	#endif


	/* get a pointer to the SFNode structure, in order to properly place the new string */
	if ((oldPtr = (SFNodeNative *)JS_GetPrivateFw(context, JSVAL_TO_OBJECT(argv[1]))) == NULL) {
		printf( "JS_GetPrivate failed in VrmlBrowserLoadURL for SFNode parameter.\n");

		JS_free(context,_costr0);
		JS_free(context,fieldStr);

		return JS_FALSE;
	}
	myptr = X3D_NODE(oldPtr->handle);
	if (myptr == NULL) {
		printf ("CreateVrmlFromURL, internal error - SFNodeNative memory pointer is NULL\n");

		JS_free(context,_costr0);
		JS_free(context,fieldStr);

		return JS_FALSE;
	}


	#ifdef JSVERBOSE
	printf ("SFNode handle %d, old X3DString %s\n",oldPtr->handle, oldPtr->X3DString);
	printf ("myptr %d\n",myptr);
	printf ("points to a %s\n",stringNodeType(myptr->_nodeType));
	#endif


	/* bounds checks */
	if (sizeof (_costr0) > (myFileSizeLimit-200)) {
		printf ("VrmlBrowserCreateVrmlFromURL, url too long...\n");

		JS_free(context,_costr0);
		JS_free(context,fieldStr);

		return JS_FALSE;
	}

	/* ok - here we have:
		_costr0		: the url string array; eg: [ "vrml.wrl" ]
		opldPtr		: pointer to a SFNode, with oldPtr->handle as C memory location. 
		fielsStr	: the field to send this to, eg: addChildren
	*/
	
	url.n = 0;
	url.p = NULL;
		
	/* parse the string, put it into the "url" struct defined here */
	Parser_scanStringValueToMem(X3D_NODE(&url),0,FIELDTYPE_MFString, _costr0, FALSE);

	/* find a file name that exists. If not, return JS_FALSE */
	res = resource_create_multi(&url);
	res->whereToPlaceData = myptr;


	/* lets see if this node has a routed field  fromTo  = 0 = from node, anything else = to node */
	fieldInt = findRoutedFieldInFIELDNAMES (myptr, fieldStr, TRUE);

	if (fieldInt >=0) { 
		findFieldInOFFSETS(myptr->_nodeType, fieldInt, &offs, &type, &accessType);
	} else {
		ConsoleMessage ("Can not find field :%s: in nodeType :%s:",fieldStr,stringNodeType(myptr->_nodeType));

		JS_free(context,_costr0);
		JS_free(context,fieldStr);

		return JS_FALSE;
	}

	/* printf ("type of field %s, accessType %s\n",stringFieldtypeType(type),stringKeywordType(accessType)); */
	res->offsetFromWhereToPlaceData = offs;
	parser_process_res_VRML_X3D(res);
	//send_resource_to_parser(res);
	//resource_wait(res);
	//
	//if (res->status == ress_parsed) {
	//	/* Cool :) */
	//}

	MARK_EVENT(myptr,offs);

	JS_SET_RVAL(context,vp,*rval);
	JS_free(context,fieldStr);
	JS_free(context,_costr0);

	return JS_TRUE;
}

JSBool
VrmlBrowserAddRoute(JSContext *context, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(context,vp);
        jsval *argv = JS_ARGV(context,vp);

	if (!doVRMLRoute(context, obj, argc, argv, "addRoute")) {
		printf( "doVRMLRoute failed in VrmlBrowserAddRoute.\n");
		return JS_FALSE;
	}
	JS_SET_RVAL(context,vp,INT_TO_JSVAL(0)); //JSVAL_ZERO);
	return JS_TRUE;
}

JSBool
VrmlBrowserPrint(JSContext *context, uintN argc, jsval *vp) {
    JSObject *obj = JS_THIS_OBJECT(context,vp);
    jsval *argv = JS_ARGV(context,vp);
	unsigned int count;
	JSString *_str;
	char *_id_c;

	UNUSED (context); UNUSED(obj);
	/* printf ("FreeWRL:javascript: "); */
	for (count=0; count < argc; count++) {
		if (JSVAL_IS_STRING(argv[count])) {
			_str = JSVAL_TO_STRING(argv[count]);
			_id_c = JS_EncodeString(context,_str);
			// OLD_IPHONE_AQUA #if defined(AQUA) || defined(_MSC_VER)
			#if defined(AQUA) || defined(_MSC_VER)
			ConsoleMessage(_id_c); /* statusbar hud */
			gglobal()->ConsoleMessage.consMsgCount = 0; /* reset the "Maximum" count */
			#else
				#ifdef HAVE_NOTOOLKIT 
					printf ("%s", _id_c);
				#else
					printf ("%s\n", _id_c);
					ConsoleMessage(_id_c); /* statusbar hud */
					gglobal()->ConsoleMessage.consMsgCount = 0; /* reset the "Maximum" count */
				#endif
			#endif
			JS_free(context,_id_c);
		} else {
	/*		printf ("unknown arg type %d\n",count); */
		}
	}
	/* the \n should be done with println below, or in javascript print("\n"); 
	  except web3d V3 specs don't have Browser.println so print will do \n like the old days*/
	JS_SET_RVAL(context,vp,INT_TO_JSVAL(0)); //JSVAL_ZERO);
	return JS_TRUE;
}

JSBool
VrmlBrowserPrintln(JSContext *context, uintN argc, jsval *vp) {
	/* note, vp holds rval, since it is set in here we should be good */
	//VrmlBrowserPrint(context,argc,vp); 

	//// OLD_IPHONE_AQUA  #if defined(AQUA) || defined(_MSC_VER)
	//#if defined(AQUA) ||  defined(_MSC_VER)
	//	//ConsoleMessage("\n"); /* statusbar hud */
	//	gglobal()->ConsoleMessage.consMsgCount = 0; /* reset the "Maximum" count */
	//#else
	//	#ifdef HAVE_NOTOOLKIT
	//		printf ("\n");
	//	#endif
	//#endif

	JSObject* obj = JS_THIS_OBJECT(context, vp);
	jsval* argv = JS_ARGV(context, vp);
	unsigned int count;
	JSString* _str;
	char* _id_c;

	UNUSED(context); UNUSED(obj);
	/* printf ("FreeWRL:javascript: "); */
	for (count = 0; count < argc; count++) {
		if (JSVAL_IS_STRING(argv[count])) {
			_str = JSVAL_TO_STRING(argv[count]);
			_id_c = JS_EncodeString(context, _str);
			// OLD_IPHONE_AQUA #if defined(AQUA) || defined(_MSC_VER)
#if defined(AQUA) || defined(_MSC_VER)
			ConsoleMessage(_id_c); /* statusbar hud */
			gglobal()->ConsoleMessage.consMsgCount = 0; /* reset the "Maximum" count */
#else
#ifdef HAVE_NOTOOLKIT 
			printf("%s", _id_c);
#else
			printf("%s\n", _id_c);
			ConsoleMessage(_id_c); /* statusbar hud */
			gglobal()->ConsoleMessage.consMsgCount = 0; /* reset the "Maximum" count */
#endif
#endif
			JS_free(context, _id_c);
		}
		else {
			/*		printf ("unknown arg type %d\n",count); */
		}
	}
	/* the \n should be done with println below, or in javascript print("\n");
	  except web3d V3 specs don't have Browser.println so print will do \n like the old days*/
	  // OLD_IPHONE_AQUA  #if defined(AQUA)  || defined(_MSC_VER)
#if defined(AQUA)  || defined(_MSC_VER)
	ConsoleMessage("\n"); /* statusbar hud */
	gglobal()->ConsoleMessage.consMsgCount = 0; /* reset the "Maximum" count */
#elif !defined(_MSC_VER)
#ifdef HAVE_NOTOOLKIT
	printf("\n");
#endif
#endif
	JS_SET_RVAL(context, vp, INT_TO_JSVAL(0)); //JSVAL_ZERO);
	return JS_TRUE;
}

JSBool
VrmlBrowserDeleteRoute(JSContext *context, uintN argc, jsval *vp) {
        JSObject *obj = JS_THIS_OBJECT(context,vp);
        jsval *argv = JS_ARGV(context,vp);

	if (!doVRMLRoute(context, obj, argc, argv, "deleteRoute")) {
		printf( "doVRMLRoute failed in VrmlBrowserDeleteRoute.\n");
		return JS_FALSE;
	}

	JS_SET_RVAL(context,vp,INT_TO_JSVAL(0)); //JSVAL_ZERO);
	return JS_TRUE;
}

/****************************************************************************************/


/****************************************************************************************************/

/* internal to add/remove a ROUTE */
static JSBool doVRMLRoute(JSContext *context, JSObject *obj, uintN argc, jsval *argv, const char *callingFunc) {
	JSObject *fromNodeObj, *toNodeObj;
	SFNodeNative *fromNative, *toNative;
	JSClass *_cls[2];
	char 
		*fromFieldString, *toFieldString;
	const char	*_c_args =
		"SFNode fromNode, SFString fromEventOut, SFNode toNode, SFString toEventIn",
		*_c_format = "oSoS";
	JSString *fromFieldStringJS, *toFieldStringJS;
	struct X3D_Node *fromNode;
	struct X3D_Node *toNode;
	int fromOfs, toOfs, len;
	int fromtype, totype;
	int xxx;
	int myField;

	/* first, are there 4 arguments? */
	if (argc != 4) {
		printf ("Problem with script - add/delete route command needs 4 parameters\n");
		return JS_FALSE;
	}

	/* get the arguments, and ensure that they are obj, string, obj, string */
	if (JS_ConvertArguments(context, argc, argv, _c_format,
				&fromNodeObj, &fromFieldStringJS, &toNodeObj, &toFieldStringJS)) {
		fromFieldString = JS_EncodeString(context,fromFieldStringJS);
		toFieldString = JS_EncodeString(context,toFieldStringJS);

		if ((_cls[0] = JS_GET_CLASS(context, fromNodeObj)) == NULL) {
			printf("JS_GetClass failed for arg 0 in doVRMLRoute called from %s.\n",
					callingFunc);
			return JS_FALSE;
		}
		if ((_cls[1] = JS_GET_CLASS(context, toNodeObj)) == NULL) {
			printf("JS_GetClass failed for arg 2 in doVRMLRoute called from %s.\n",
					callingFunc);
			return JS_FALSE;
		}

		/* make sure these are both SFNodes */
		if (memcmp("SFNode", (_cls[0])->name, strlen((_cls[0])->name)) != 0 &&
			memcmp("SFNode", (_cls[1])->name, strlen((_cls[1])->name)) != 0) {
			printf("\nArguments 0 and 2 must be SFNode in doVRMLRoute called from %s(%s): %s\n",
					callingFunc, _c_args, callingFunc);
			return JS_FALSE;
		}

		/* get the "private" data for these nodes. It will consist of a SFNodeNative structure */
		if ((fromNative = (SFNodeNative *)JS_GetPrivateFw(context, fromNodeObj)) == NULL) {
			printf ("problem getting native props\n");
			return JS_FALSE;
		}
		if ((toNative = (SFNodeNative *)JS_GetPrivateFw(context, toNodeObj)) == NULL) {
			printf ("problem getting native props\n");
			return JS_FALSE;
		}
		/* get the "handle" for the actual memory pointer */
		fromNode = X3D_NODE(fromNative->handle);
		toNode = X3D_NODE(toNative->handle);

		#ifdef JSVERBOSE
		printf ("routing from a node of type %s to a node of type %s\n",
			stringNodeType(fromNode->_nodeType), 
			stringNodeType(toNode->_nodeType));
		#endif	

		/* From field */
		/* try finding it, maybe with a "set_" or "changed" removed */
		myField = findRoutedFieldInFIELDNAMES(fromNode,fromFieldString,0);
		if (myField == -1) 
			myField = findRoutedFieldInFIELDNAMES(fromNode,fromFieldString,1);

		/* find offsets, etc */
       		findFieldInOFFSETS(fromNode->_nodeType, myField, &fromOfs, &fromtype, &xxx);

		/* To field */
		/* try finding it, maybe with a "set_" or "changed" removed */
		myField = findRoutedFieldInFIELDNAMES(toNode,toFieldString,0);
		if (myField == -1) 
			myField = findRoutedFieldInFIELDNAMES(toNode,toFieldString,1);

		/* find offsets, etc */
       		findFieldInOFFSETS(toNode->_nodeType, myField, &toOfs, &totype, &xxx);

		/* do we have a mismatch here? */
		if (fromtype != totype) {
			printf ("Javascript routing problem - can not route from %s to %s\n",
				stringNodeType(fromNode->_nodeType), 
				stringNodeType(toNode->_nodeType));
			return JS_FALSE;
		}

		len = returnRoutingElementLength(totype);

		jsRegisterRoute(fromNode, fromOfs, toNode, toOfs, len,callingFunc);

		JS_free(context,fromFieldString);
		JS_free(context,toFieldString);

	} else {
		printf( "\nIncorrect argument format for %s(%s).\n",
				callingFunc, _c_args);
		return JS_FALSE;
	}

	return JS_TRUE;
}

struct JSLoadPropElement {
	JSClass* fwclass;
	//void *constr;
	JSBool(*constr)(JSContext*, unsigned int, jsval*);
	void* Functions;
	void* Properties;
	const char* id;
};
struct JSLoadPropElement JSLoadPropsAux[] = {

		// done separately { &BrowserClass, NULL, &BrowserFunctions, &BrowserProperties, "BrowserClass"},
		{ &ExecutionContextClass, NULL, &ExecutionContextFunctions, &ExecutionContextProperties, "ExecutionContextClass"},
		{ &ComponentInfoClass, NULL, NULL, &ComponentInfoProperties, "ComponentInfoClass"},
		{ &ComponentInfoArrayClass, NULL, NULL, &ComponentInfoArrayProperties, "ComponentInforArrayClass"},
		{ &ProfileInfoClass, NULL, NULL, &ProfileInfoProperties, "ProfileInfoClass"},
		{ &ProfileInfoArrayClass, NULL, NULL, &ProfileInfoArrayProperties, "ProfileInfoArrayClass"},
		{ &X3DRouteClass, NULL, &X3DRouteFunctions, &X3DRouteProperties, "X3DRouteClass"},
		{ &RouteArrayClass, NULL, NULL, &RouteArrayProperties, "RouteArrayClass"},
		{ &ProtoDeclarationArrayClass, NULL, NULL, &ProtoDeclarationArrayProperties, "ProtoDeclarationArrayClass"},
		{ &ProtoDeclarationClass, NULL, &ProtoDeclarationFunctions, &ProtoDeclarationProperties, "ProtoDeclarationClass"},
		{ &FieldDefinitionArrayClass, NULL, NULL, &FieldDefinitionArrayProperties, "FieldDefinitionClass"},
		{ &FieldDefinitionClass, NULL, NULL, &FieldDefinitionProperties, "FieldDefinitionClass"},
		{ &X3DConstantsClass, NULL, NULL, NULL, "FieldDefinitionClass"},
		{ NULL, NULL, NULL, NULL, NULL }
};

/* load the FreeWRL extra classes */
JSBool loadAuxiliaryClasses(JSContext* context, JSObject* globalObj) {
	jsval v;
	int i;

	JSObject* myProto;

	i = 0;
	while (JSLoadPropsAux[i].fwclass != NULL) {
#ifdef JSVRMLCLASSESVERBOSE
		printf("loading %s\n", JSLoadProps[i].id);
#endif

		/* v = 0; */
		if ((myProto = JS_InitClass(context, globalObj, NULL, JSLoadPropsAux[i].fwclass,
			(JSNative)JSLoadPropsAux[i].constr, INIT_ARGC, (JSPropertySpec*)JSLoadPropsAux[i].Properties,
			(JSFunctionSpec*)JSLoadPropsAux[i].Functions, NULL, NULL)) == NULL) {
			printf("JS_InitClass for %s failed in loadVrmlClasses.\n", JSLoadPropsAux[i].id);
			return JS_FALSE;
		}
		//JS::RootedObject protoObj(context, myProto);
		v = OBJECT_TO_JSVAL(myProto);
		if (!JS_SetProperty(context, globalObj, JSLoadPropsAux[i].id, &v)) {
			printf("JS_SetProperty for %s failed in loadAuxiliaryClasses.\n", JSLoadPropsAux[i].id);
			return JS_FALSE;
		}

		i++;
	}
	return JS_TRUE;
}
//dug9 - first look at x3dbrowser and x3dscene/executionContext
#ifdef X3DBROWSER
/* The Browser's supportedComponents and supportedProfiles are statically defined 
   in 'bits and pieces' in generatedCode.c and capabilitiesHandler.c and Structs.h.
   The Scene/ExecutionContext Profile and Components should be created during parsing
   (as of Aug 3, 2013 the parser calls handleProfile() or handleComponent() which
    just complains with printfs if freewrl can't handle the scene, and doesn't save them)

	For the browser's supportedComponents and supportedProfiles, we'll have
	indexable arrays, and on getting an index, we'll construct a throwaway JS object.
*/




#endif
/*
ComponentInfo{
String name;
Numeric level;
String Title;
String providerUrl;
}

ComponentInfoArray{
numeric length;
ComponentInfo [integer index];
}


ProfileInfo{
String name;
Numeric level;
String Title;
String providerUrl;
ComonentInfoArray components;
}
ProfileInfoArray{
numeric length;
ProfileInfo [integer index];
}

X3DFieldDefinition{
//properties
String name;
numeric accessType;  //e.g.. inputOnly
numeric dataType; //e.g. SFBool
}

FieldDefinitionArray{
numeric length;
X3DFieldDefinition [integer index];
}


ProtoDeclaration{
//properties
String name;
FieldDefinitionArray fields;
Boolean isExternProto;
//functions
SFNode newInstance();
}

ExternProtoDeclaration : ProtoDeclaration {
//properties
MFString urls;
numeric loadState;
//functions
void loadNow();
}

ProtoDeclarationArray{
numeric length;
X3DProtoDeclaration [integer index];
}

ExternProtoDeclarationArray{
numeric length;
X3DExternProtoDeclaration [integer index];
}

Route{
}
RouteArray{
numeric length;
Route [integer index];
}


ExecutionContext{
//properties
String specificationVersion;
String encoding;
ProfileInfo profile;
ComponentInfoArray components;
String worldURL;
MFNode rootNodes; //R + writable except in protoInstance
ProtoDeclarationArray protos; //RW
ExternProtoDeclarationArray externprotos; //RW
RouteArray routes;
//functions
X3DRoute addRoute(SFNode fromNode, String fromReadableField, SFNode toNode, String toWriteableField);
void deleteRoute(X3DRoute route);
SFNode createNode(String x3dsyntax);
SFNode createProto(String x3dsyntax);
SFNode getImportedNode(String defname, String);
void updateImportedNode(String defname, String);
void removeImportedNode(String defname);
SFNode getNamedNode(String defname):
void updateNamedNode(String defname, SFNode);
void removeNamedNode(String defname);
}

Scene : ExecutionContext{
//properties
String specificationVersion;
//functions
void setMetaData(String name, String value);
String getMetaData(String name);
SFNode getExportedNode(String defname);
void updateExportedNode(String defname, SFNode node);
void removeExportedNode(String defname);
}

//just createX3DFromString, createX3DFromURL and replaceWorld differ in signature between VRML and X3D browser classes
X3DBrowser{
//properties
String name;
String version;
numeric currentSpeed;
numeric currentFrameRate;
String description; //R/W
CompnentInfoArray supportedComponents;
ProfileInfoArray supportedProfiles;
//functions
X3DScene currentScene;  //since X3DScene : X3DExecutionContext, use Scene w/flag
void replaceWorld(X3DScene);
X3DScene createX3DFromString(String x3dsyntax);
X3DScene createX3DFromURL(MFString url, String callbackFunctionName, Object cbContextObject);
void loadURL(MFString url, MFString parameter);
X3DScene importDocument(DOMNode domNodeObject);
void getRenderingProperty(String propertyName);
void print(Object or String);
void println(Object or String);
}
*/

#endif /* !(defined(JAVASCRIPT_STUB) || defined(JAVASCRIPT_DUK) */

#endif //defined(JS_SMCPP)
