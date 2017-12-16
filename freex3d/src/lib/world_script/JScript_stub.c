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
#include "../ui/common.h"
//#if defined(JAVASCRIPT_STUB)

typedef int indexT;
union anyVrml{
	int nothing;
} anyVrml;
struct X3D_Node;
struct X3D_Proto;
//#ifndef BOOL
//#define BOOL int
//#endif
//#include "JScript.h"
//.c module gglobal sub-state initializers
//void JScript_init(void *t){}
//void jsVRMLBrowser_init(void *t){}
//void jsUtils_init(void *t){}
//void jsVRMLClasses_init(void *t){}

/* stubs, when you don't have a javascript engine */
/* //switching between engines within a stub
	#ifdef JAVASCRIPT_SM
		if(getJsEngine() == JSENGINE_SM)
			sm_JScript_init(t);
	#endif
	#ifdef JAVASCRIPT_DUK
		if(getJsEngine() == JSENGINE_DUK)
			duk_JScript_init(t);
	#endif
*/
//void kill_javascript(void){return;}
//void JSInit(int num){return;}
//void SaveScriptText(int num, const char *text){return;}
void process_eventsProcessed(){
	#ifdef JAVASCRIPT_SM
		if(getJsEngine() == JSENGINE_SM)
			sm_process_eventsProcessed();
	#endif
	#ifdef JAVASCRIPT_DUK
		if(getJsEngine() == JSENGINE_DUK)
			duk_process_eventsProcessed();
	#endif
	return;
}
void js_cleanup_script_context(int counter){
	#ifdef JAVASCRIPT_SM
		if(getJsEngine() == JSENGINE_SM)
			sm_js_cleanup_script_context(counter);
	#endif
	#ifdef JAVASCRIPT_DUK
		if(getJsEngine() == JSENGINE_DUK)
			duk_js_cleanup_script_context(counter);
	#endif
	return;
}
int jsActualrunScript(int num, char *script){
	int iret = 0;
	#ifdef JAVASCRIPT_SM
		if(getJsEngine() == JSENGINE_SM)
			iret = sm_jsActualrunScript(num, script);
	#endif
	#ifdef JAVASCRIPT_DUK
		if(getJsEngine() == JSENGINE_DUK)
			iret = duk_jsActualrunScript(num, script);
	#endif
	return iret;
}
//void JSInitializeScriptAndFields (int num){return;}
void JSCreateScriptContext(int num){
	#ifdef JAVASCRIPT_SM
		if(getJsEngine() == JSENGINE_SM)
			sm_JSCreateScriptContext(num);
	#endif
	#ifdef JAVASCRIPT_DUK
		if(getJsEngine() == JSENGINE_DUK)
			duk_JSCreateScriptContext(num);
	#endif
	return;
}
void JSInitializeScriptAndFields (int num) {
	#ifdef JAVASCRIPT_SM
		if(getJsEngine() == JSENGINE_SM)
			sm_JSInitializeScriptAndFields(num);
	#endif
	#ifdef JAVASCRIPT_DUK
		if(getJsEngine() == JSENGINE_DUK)
			duk_JSInitializeScriptAndFields(num);
	#endif
	return;
}
//void SaveScriptField (int num, indexT kind, indexT type, const char* field, union anyVrml value){return;}
void js_setField_javascriptEventOut_B(union anyVrml* any, int fieldType, unsigned len, int extraData, int actualscript){
	#ifdef JAVASCRIPT_SM
		if(getJsEngine() == JSENGINE_SM)
			sm_js_setField_javascriptEventOut_B(any,fieldType,len,extraData,actualscript);
	#endif
	#ifdef JAVASCRIPT_DUK
		if(getJsEngine() == JSENGINE_DUK)
			duk_js_setField_javascriptEventOut_B(any,fieldType,len,extraData,actualscript);
	#endif
	return;
}
void js_setField_javascriptEventOut(struct X3D_Node *tn,unsigned int tptr,  int fieldType, unsigned len, int extraData, int actualscript){
	#ifdef JAVASCRIPT_SM
		if(getJsEngine() == JSENGINE_SM)
			sm_js_setField_javascriptEventOut(tn,tptr,fieldType,len,extraData,actualscript);
	#endif
	#ifdef JAVASCRIPT_DUK
		if(getJsEngine() == JSENGINE_DUK)
			duk_js_setField_javascriptEventOut(tn,tptr,fieldType,len,extraData,actualscript);
	#endif
	return;
}

void setScriptECMAtype(int num){
	#ifdef JAVASCRIPT_SM
		if(getJsEngine() == JSENGINE_SM)
			sm_setScriptECMAtype(num);
	#endif
	#ifdef JAVASCRIPT_DUK
		if(getJsEngine() == JSENGINE_DUK)
			duk_setScriptECMAtype(num);
	#endif
	return;
}
int get_valueChanged_flag (int fptr, int actualscript){
	int iret = 0;
	#ifdef JAVASCRIPT_SM
		if(getJsEngine() == JSENGINE_SM)
			iret = sm_get_valueChanged_flag(fptr, actualscript);
	#endif
	#ifdef JAVASCRIPT_DUK
		if(getJsEngine() == JSENGINE_DUK)
			iret = duk_get_valueChanged_flag(fptr, actualscript);
	#endif
	return iret;
}
void resetScriptTouchedFlag(int actualscript, int fptr){
	#ifdef JAVASCRIPT_SM
		if(getJsEngine() == JSENGINE_SM)
			sm_resetScriptTouchedFlag(actualscript, fptr);
	#endif
	#ifdef JAVASCRIPT_DUK
		if(getJsEngine() == JSENGINE_DUK)
			duk_resetScriptTouchedFlag(actualscript, fptr);
	#endif
	return;
}
void set_one_ECMAtype (int tonode, int toname, int dataType, void *Data, int datalen){
	#ifdef JAVASCRIPT_SM
		if(getJsEngine() == JSENGINE_SM)
			sm_set_one_ECMAtype (tonode, toname, dataType, Data, datalen);
	#endif
	#ifdef JAVASCRIPT_DUK
		if(getJsEngine() == JSENGINE_DUK)
			duk_set_one_ECMAtype (tonode, toname, dataType, Data, datalen);
	#endif
	return;
}
void set_one_MultiElementType (int tonode, int tnfield, void *Data, int dataLen){
	#ifdef JAVASCRIPT_SM
		if(getJsEngine() == JSENGINE_SM)
			sm_set_one_MultiElementType (tonode, tnfield,Data, dataLen);
	#endif
	#ifdef JAVASCRIPT_DUK
		if(getJsEngine() == JSENGINE_DUK)
			duk_set_one_MultiElementType (tonode, tnfield,Data, dataLen);
	#endif
	return;
}
void set_one_MFElementType(int tonode, int toname, int dataType, void *Data, int datalen){
	#ifdef JAVASCRIPT_SM
		if(getJsEngine() == JSENGINE_SM)
			sm_set_one_MFElementType(tonode, toname, dataType, Data, datalen);
	#endif
	#ifdef JAVASCRIPT_DUK
		if(getJsEngine() == JSENGINE_DUK)
			duk_set_one_MFElementType(tonode, toname, dataType, Data, datalen);
	#endif
	return;
}
int jsIsRunning(){
	int iret = 1;
	#ifdef JAVASCRIPT_SM
		if(getJsEngine() == JSENGINE_SM)
			iret = sm_jsIsRunning();
	#endif
	#ifdef JAVASCRIPT_DUK
		if(getJsEngine() == JSENGINE_DUK)
			iret = duk_jsIsRunning();
	#endif
	return iret;
}
void JSDeleteScriptContext(int num){
	#ifdef JAVASCRIPT_SM
		if(getJsEngine() == JSENGINE_SM)
			sm_JSDeleteScriptContext(num);
	#endif
	#ifdef JAVASCRIPT_DUK
		if(getJsEngine() == JSENGINE_DUK)
			duk_JSDeleteScriptContext(num);
	#endif
	return;
}
void jsShutdown(){
	#ifdef JAVASCRIPT_SM
		if(getJsEngine() == JSENGINE_SM)
			sm_jsShutdown();
	#endif
	#ifdef JAVASCRIPT_DUK
		if(getJsEngine() == JSENGINE_DUK)
			duk_jsShutdown();
	#endif
	return;
}

void jsClearScriptControlEntries(int num){
	#ifdef JAVASCRIPT_SM
		if(getJsEngine() == JSENGINE_SM)
			sm_jsClearScriptControlEntries (num);
	#endif
	#ifdef JAVASCRIPT_DUK
		if(getJsEngine() == JSENGINE_DUK)
			duk_jsClearScriptControlEntries (num);
	#endif
	return;
}
void SaveScriptField (int num, indexT kind, indexT type, const char* field, union anyVrml value){
	#ifdef JAVASCRIPT_SM
		if(getJsEngine() == JSENGINE_SM)
			sm_SaveScriptField (num, kind, type, field, value);
	#endif
	#ifdef JAVASCRIPT_DUK
		if(getJsEngine() == JSENGINE_DUK)
			duk_SaveScriptField (num, kind, type, field, value);
	#endif
	return;
}
int runQueuedDirectOutputs(){
	//stub for SM and STUBS (DUK has it)
	int iret = 0;
	#ifdef JAVASCRIPT_SM
		if(getJsEngine() == JSENGINE_SM)
			iret = sm_runQueuedDirectOutputs();
	#endif
	#ifdef JAVASCRIPT_DUK
		if(getJsEngine() == JSENGINE_DUK)
			iret = duk_runQueuedDirectOutputs();
	#endif
	return iret;
}
//#endif /* defined(JAVASCRIPT_STUB) */
