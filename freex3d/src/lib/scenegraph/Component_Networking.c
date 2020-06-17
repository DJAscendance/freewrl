/*


X3D Networking Component

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
#include <system.h>
#include <display.h>
#include <internal.h>

#include "../vrml_parser/Structs.h"
#include "../vrml_parser/CRoutes.h"
#include "../main/headers.h"

#include "../input/EAIHeaders.h"
#include "../input/EAIHelpers.h"
#include "../opengl/Frustum.h"
#include "../opengl/OpenGL_Utils.h"
#include "../opengl/Textures.h"

#include "Component_Networking.h"
#include "Children.h"
#include "../scenegraph/RenderFuncs.h"

#include <libFreeWRL.h>
#include <list.h>
#include <io_http.h>
#ifdef WANT_OSC
	#include <lo/lo.h>
	#include "ringbuf.h"
	#define USE_OSC 1
	#define TRACK_OSC_MSG 0
#else
	#define USE_OSC 0
#endif

#if USE_OSC
#include "../vrml_parser/CParseGeneral.h"
#include "../scenegraph/Vector.h"
#include "../vrml_parser/CFieldDecls.h"
#include "../world_script/JScript.h"
#include "../world_script/CScripts.h"
#include "../world_script/fieldSet.h"
#include "../vrml_parser/CParseParser.h"
#include "../vrml_parser/CParseLexer.h"
#include "../vrml_parser/CParse.h"
#endif

//OLDCODE #define BUTTON_PRESS_STRING "use_for_buttonPresses"

#if USE_OSC
/**************** START OF OSC node **************************/

void error(int num, const char *m, const char *path);
void utilOSCcounts(char *types , int *intCount, int *fltCount, int *strCount, int *blobCount, int *midiCount, int *otherCount);

/* We actually want to keep this one inline, as it offers ease of editing with minimal infrastructure */
#include "OSCcallbacks.c"

int serverCount=0;
#define MAX_OSC_SERVERS 32
int serverPort[MAX_OSC_SERVERS] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
lo_server_thread oscThread[MAX_OSC_SERVERS] ;

static uintptr_t *OSC_Nodes = NULL;
static int num_OSC_Nodes = 0;
int curr_OSC_Node = 0;
int active_OSC_Nodes = FALSE;

void utilOSCcounts(char *types , int *intCount, int *fltCount, int *strCount, int *blobCount, int *midiCount, int *otherCount) {
	*intCount	= 0;
	*fltCount	= 0;
	*strCount	= 0;
	*blobCount	= 0;
	*midiCount	= 0;
	*otherCount	= 0;

	int i,j;

	j=strlen(types) ;
	/* what amount of storage */
	for (i=0 ; i < j ; i++) {
		switch (types[i]) {
		case 'i':
			(*intCount)++;
			break;
		case 'f':
			(*fltCount)++;
			break;
		case 's':
			(*strCount)++;
			break;
		case 'b':
			(*blobCount)++;
			break;
		case 'm':
			(*midiCount)++;
			break;
		default:
			(*otherCount)++;
			break;
		}
	}
}

void activate_OSCsensors() {
	curr_OSC_Node = 0 ;
	active_OSC_Nodes = TRUE ;
	struct X3D_OSC_Sensor *realnode ;
	char buf[32];
	int i ;
	/* what amount of storage */
	int fltCount;
	int intCount;
	int strCount;
	int blobCount;
	int midiCount;
	int otherCount;

	while (active_OSC_Nodes && curr_OSC_Node < num_OSC_Nodes) {
		realnode = (struct X3D_OSC_Sensor *) OSC_Nodes[curr_OSC_Node] ;
        if (checkNode(realnode,__FILE__,__LINE__)) {
		#if TRACK_OSC_MSG
		printf("activate_OSCsensors : %s,%d node=%p name=%s\n", __FILE__,__LINE__,realnode,realnode->description->strptr) ;
		#endif
		if (realnode->_status < 0) {
			printf("activate_OSCsensors : %s,%d Moving %s to ready.\n", __FILE__,__LINE__,realnode->description->strptr) ;
			realnode->_status = 0 ;
		} else if (realnode->_status == 0) {
			printf("activate_OSCsensors : %s,%d\n", __FILE__,__LINE__) ;
			printf("activate_OSCsensors : enabled=%d\n",	realnode->enabled) ;
			printf("activate_OSCsensors : gotEvents=%d\n",	realnode->gotEvents) ;
			printf("activate_OSCsensors : description=%s\n",realnode->description->strptr) ;
			printf("activate_OSCsensors : protocol=%s\n",	realnode->protocol->strptr) ;
			printf("activate_OSCsensors : port=%d\n",	realnode->port) ;
			printf("activate_OSCsensors : filter=%s\n",	realnode->filter->strptr) ;
			printf("activate_OSCsensors : handler=%s\n",	realnode->handler->strptr) ;
/*
11715                     if(allFields) {
11716                         spacer fprintf (fp,"\t_talkToNodes (MFNode):\n");
11717                         for (i=0; i<tmp->_talkToNodes.n; i++) { dump_scene(fp,level+1,tmp->_talkToNodes.p[i]); }
11718                     }
11719                     if(allFields) {
11720                         spacer fprintf (fp,"\t_status (SFInt32) \t%d\n",tmp->_status);
11721                     }
11722                     if(allFields) {
11723                         spacer fprintf (fp,"\t_floatInpFIFO (SFNode):\n"); dump_scene(fp,level+1,tmp->_floatInpFIFO);
11724                     }
11725                     if(allFields) {
11726                         spacer fprintf (fp,"\t_int32OutFIFO (SFNode):\n"); dump_scene(fp,level+1,tmp->_int32OutFIFO);
11727                     }
11728                         spacer fprintf (fp,"\ttalksTo (MFString): \n");
11729                         for (i=0; i<tmp->talksTo.n; i++) { spacer fprintf (fp,"                 %d: \t%s\n",i,tmp->talksTo.p[i]->strptr); }
*/
			printf("activate_OSCsensors : talksTo=[ ");
			for (i=0; i < realnode->talksTo.n; i++) {
				printf("\"%s\" ",realnode->talksTo.p[i]->strptr);
				/* This would be a good time to convert the name into an entry in _talkToNodes */
				struct X3D_Node * myNode;
				/* myNode = X3DParser_getNodeFromName(realnode->talksTo.p[i]->strptr); */
				myNode = parser_getNodeFromName(realnode->talksTo.p[i]->strptr);
				if (myNode != NULL) {
					printf("(%p) ",(void *)myNode);
				} else {
					printf("(..) ");
				}
			}
			printf("] (%d nodes) (Need to fix %s,%d)\n",realnode->talksTo.n , __FILE__,__LINE__);
			printf("activate_OSCsensors : listenfor=%s , expect %d parameters\n", realnode->listenfor->strptr , (int)strlen(realnode->listenfor->strptr)) ;
			printf("activate_OSCsensors : FIFOsize=%d\n",	realnode->FIFOsize) ;
			printf("activate_OSCsensors : _status=%d\n",	realnode->_status) ;

			if (realnode->FIFOsize > 0) {
				/* what amount of storage */
				utilOSCcounts(realnode->listenfor->strptr,&intCount,&fltCount,&strCount,&blobCount,&midiCount,&otherCount);
				intCount = realnode->FIFOsize * (intCount + midiCount); 
				fltCount = realnode->FIFOsize * fltCount;
				strCount = realnode->FIFOsize * (strCount + blobCount + otherCount); 
				printf("Allocate %d floats, %d ints for '%s'\n",fltCount,intCount,realnode->description->strptr);

				realnode->_int32InpFIFO = (void *) NewRingBuffer (intCount) ;
				realnode->_floatInpFIFO = (void *) NewRingBuffer (fltCount) ;
				realnode->_stringInpFIFO = (void *) NewRingBuffer (strCount) ;

			}

			/* start a new server on the required port */
			int foundCurrentPort = -1 ;
			for ( i=0 ; i < num_OSC_Nodes ; i++) {
				if(realnode->port == serverPort[i]) {
					foundCurrentPort=i;
					i = num_OSC_Nodes+1;
				}
			}
			if (foundCurrentPort < 0) {
				foundCurrentPort = serverCount ;
				serverPort[foundCurrentPort] = realnode->port ;
				serverCount++ ;

				sprintf (buf,"%d",realnode->port);

				if (strcmp("TCP",realnode->protocol->strptr)==0) {
					/* oscThread[foundCurrentPort] = lo_server_thread_new_with_proto(buf, LO_TCP, error); */
					oscThread[foundCurrentPort] = lo_server_thread_new(buf, error);
				} else if (strcmp("UNIX",realnode->protocol->strptr)==0) {
					/* oscThread[foundCurrentPort] = lo_server_thread_new_with_proto(buf, LO_UNIX, error); */
					oscThread[foundCurrentPort] = lo_server_thread_new(buf, error);
				} else {
					/* oscThread[foundCurrentPort] = lo_server_thread_new_with_proto(buf, LO_UDP, error); */
					oscThread[foundCurrentPort] = lo_server_thread_new(buf, error);
				}
				lo_server_thread_start(oscThread[foundCurrentPort]);
			}

			printf("%d servers; current server is running in slot %d on port %d\n",serverCount,foundCurrentPort,serverPort[foundCurrentPort]) ;

			/* add method (by looking up its name) that will in future use the required path */
			/* need to re-read the lo code to check that you can register the same callback twice (or more) with different paths) */
			/* See OSCcallbacks.c */
			int foundHandler = 0 ;
			for (i=0 ; i < OSCfuncCount ; i++) {
				printf("%d/%d : Check %s against %s\n",i,OSCfuncCount,realnode->handler->strptr,OSCfuncNames[i]);
				if (0 == strcmp(realnode->handler->strptr,OSCfuncNames[i])) {foundHandler = i;}
			}
			if (OSCcallbacks[foundHandler] != NULL) {
				printf("Going to hook '%s' to '%s' handler\n",realnode->description->strptr ,OSCfuncNames[foundHandler]) ;
				lo_server_thread_add_method(oscThread[foundCurrentPort], realnode->filter->strptr, realnode->listenfor->strptr,
					(OSCcallbacks[foundHandler]), realnode);
			}

			realnode->_status = 1 ;
			/* We only want one OSC node to become active in one slowtick */
			active_OSC_Nodes = FALSE ;
		} 
        } // end of checkNode conditional - JAS
            
		curr_OSC_Node++;
	}
}

void error(int num, const char *msg, const char *path)
{
    printf("liblo server error %d in path %s: %s\n", num, path, msg);
}

void add_OSCsensor(struct X3D_Node * node) {
	uintptr_t *myptr;

	if (node == 0) {
		printf ("error in registerOSCNode; somehow the node datastructure is zero \n");
		return;
	}

	if (node->_nodeType != NODE_OSC_Sensor) return;

	OSC_Nodes = (uintptr_t *) REALLOC (OSC_Nodes,sizeof (uintptr_t *) * (num_OSC_Nodes+1));
	myptr = OSC_Nodes;

	/* now, put the node pointer into the structure entry */
	*myptr = (uintptr_t) node;

	num_OSC_Nodes++;
}
void remove_OSCsensor(struct X3D_Node * node) {}
/***************** END OF OSC node ***************************/
#else
void add_OSCsensor(struct X3D_Node * node) {}
void remove_OSCsensor(struct X3D_Node * node) {}
#endif

int loadstatus_AudioClip(struct X3D_AudioClip *node);
int loadstatus_Script(struct X3D_Script *script);
int getFieldFromNodeAndNameC(struct X3D_Node* node,const char *fieldname, int *type, int *kind, int *iifield, int *builtIn, union anyVrml **value, const char **cname);
void render_LoadSensor (struct X3D_LoadSensor *node) {
	int count;
	int nowLoading;
	int nowFinished;
	struct X3D_Node *cnode;
	// HAVE TO RECODE MovieTexture struct X3D_MovieTexture *mnode;
	
	/* if not enabled, do nothing */
	if (!node) return;
	if (node->__oldEnabled != node->enabled) {
		node->__oldEnabled = node->enabled;
		MARK_EVENT(X3D_NODE(node),offsetof (struct X3D_LoadSensor, enabled));
	}
	if (!node->enabled) return;

	/* we only need to look at this once per event loop */
	//if (!renderstate()->render_geom) return;
	if (!renderstate()->render_sensitive) return;

	/* do we need to re-generate our internal variables? */
	if NODE_NEEDS_COMPILING {
		MARK_NODE_COMPILED
		node->__loading = 0;
		node->__finishedloading = 0;
		node->progress = (float) 0.0;
		node->__StartLoadTime = 0.0;
	}

	/* do we actually have any nodes to watch? */
	if (node->watchList.n<=0) return;

	/* are all nodes loaded? */
	if (node->__finishedloading == node->watchList.n) return;

	/* our current status... */
	nowLoading = 0;
	nowFinished = 0;

	/* go through node list, and check to see what the status is */
	/* printf ("have %d nodes to watch\n",node->watchList.n); */
	for (count = 0; count < node->watchList.n; count ++) {

		cnode = node->watchList.p[count];

		/* printf ("node type of node %d is %d\n",count,tnode->_nodeType); */
		switch (cnode->_nodeType) {
		case NODE_ImageTexture:
			{
				/* printf ("opengl tex is %d\n",tnode->__texture); */
				/* is this texture thought of yet? */
				struct X3D_ImageTexture *tnode = (struct X3D_ImageTexture *) cnode;

				nowLoading++;
				if (fwl_isTextureLoaded(tnode->__textureTableIndex)) {
					/* is it finished loading? */
					nowFinished ++;
				}
			}
			break;
		case NODE_Inline:
			{
				struct X3D_Inline *inode;
				inode = (struct X3D_Inline *) cnode; /* change type to Inline */
				if(inode->__loadstatus > INLINE_INITIAL_STATE && inode->__loadstatus < INLINE_STABLE)
					nowLoading++;
				if(inode->__loadstatus == INLINE_STABLE)
					nowFinished ++;
				/* printf ("LoadSensor, Inline %d, type %d loadstatus %d at %d\n",inode,inode->_nodeType,inode->__loadstatus, &inode->__loadstatus); */
			}
			break;
		case NODE_Script:
			{
				if(loadstatus_Script(X3D_SCRIPT(cnode)))
					nowFinished ++;
			}
			break;
		case NODE_ShaderProgram:
			{
				struct Shader_Script *shader;
				shader=(struct Shader_Script *)(X3D_SHADERPROGRAM(cnode)->_shaderUserDefinedFields); 
				if(shader->loaded) nowFinished++;
			}
			break;
		case NODE_PackagedShader: 
			{
				struct Shader_Script *shader;
				shader=(struct Shader_Script *)(X3D_PACKAGEDSHADER(cnode)->_shaderUserDefinedFields); 
				if(shader->loaded) nowFinished++;
			}
			break;
		case NODE_ComposedShader: 
			{
				struct Shader_Script *shader;
				shader=(struct Shader_Script *)(X3D_COMPOSEDSHADER(cnode)->_shaderUserDefinedFields); 
				if(shader->loaded) nowFinished++;
			}

			break;
		case NODE_Effect: 
			{
				struct Shader_Script *shader;
				shader=(struct Shader_Script *)(X3D_EFFECT(cnode)->_shaderUserDefinedFields); 
				if(shader->loaded) nowFinished++;
			}

			break;
		case NODE_MovieTexture: //july 2016 - ordered fields in movietexture to match audioclip
		case NODE_AudioClip:
			{
				int istate;
				struct X3D_AudioClip *anode;
				anode = (struct X3D_AudioClip *) cnode; /* change type to AudioClip */
				/* AudioClip sourceNumber will be gt -1 if the clip is ok. see code for details */
				istate = loadstatus_AudioClip(anode);
				if (istate == 1) 
					nowLoading ++;
				if(istate == 2)
					nowFinished++;
			}
			break;

		default :{} /* there should never be anything here, but... */
		}
	}
		

	/* ok, are we NOW finished loading? */
	if (nowFinished == node->watchList.n) {
		node->isActive = 0;
		MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_LoadSensor, isActive));

		node->isLoaded = 1;
		MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_LoadSensor, isLoaded));

		node->progress = (float) 1.0;
		MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_LoadSensor, progress));

		node->loadTime = TickTime();
		MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_LoadSensor, loadTime));
	}	

	/* have we NOW started loading? */
	if ((nowLoading > 0) && (node->__loading == 0)) {
		/* mark event isActive TRUE */
		node->isActive = 1;
		MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_LoadSensor, isActive));

	
		node->__StartLoadTime = TickTime();
	}
	
	/* what is our progress? */
	if (node->isActive == 1) {
		node->progress = (float)(nowFinished)/(float)(node->watchList.n);
		MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_LoadSensor, progress));
	}

	/* remember our status for next time. */
	node->__loading = nowLoading;
	node->__finishedloading = nowFinished;

	/* did we run out of time? */
	if (node->timeOut > 0.0001) {			/* we have a timeOut specified */
		if (node->__StartLoadTime > 0.001) {	/* we have a start Time recorded from the isActive = TRUE */
		
			/* ok, we should look at time outs */
			if ((TickTime() - node->__StartLoadTime) > node->timeOut) {
				node->isLoaded = 0;
				MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_LoadSensor, isLoaded));

				node->isActive = 0;
				MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_LoadSensor, isActive));

				/* and, we will just assume that we have loaded everything next iteration */
				node->__finishedloading = node->watchList.n;
			}
		}
	}
}


void child_Anchor (struct X3D_Anchor *node) {
	int nc = (node->children).n;
	/* any children at all? */
	if (nc==0) return;
	/* any visible children? */
	OCCLUSIONTEST

	/* do we have a local light for a child? */
	prep_sibAffectors((struct X3D_Node*)node,&node->__sibAffectors);
	prep_BBox((struct BBoxFields*)&node->bboxCenter);
	/* now, just render the non-directionalLight children */
	normalChildren(node->children);
	fin_BBox((struct X3D_Node*)node,(struct BBoxFields*)&node->bboxCenter,FALSE);
	fin_sibAffectors((struct X3D_Node*)node,&node->__sibAffectors);
}

struct X3D_Node *broto_search_DEFname(struct X3D_Proto *context, const char *name);
struct IMEXPORT *broto_search_IMPORTname(struct X3D_Proto *context, const char *name);
struct IMEXPORT *broto_search_EXPORTname(struct X3D_Proto *context, const char *name);

struct X3D_Node * broto_search_ALLnames(struct X3D_Proto *context, const char *name, int *source){
	/*chain-of-command pattern looks in DEFnames and if not found looks in IMPORTS and if found
		checks Inline's EXPORT table if available, and if found, checks Inline's DEF table to get node*
		(name,node*) 'mapping': 
			name -> DEF-> IMPORT -> DEF -> inline -> EXPORT -> node*  
		- the Inline may be mentioned by char* name in IMPORT struct, so an exter DEFname lookup is needed to get Inline* node
		-- that may change/be optimized if stable enough
	*/
	struct X3D_Node *node;
	*source = 0; //main scene
	node = NULL;
	//check scene's DEF table to see if it has become a normal node mapping
	node = broto_search_DEFname(context,name);
	if(!node){
		//check scene's IMPORT table to see if it's listed there
		struct IMEXPORT *im;
		im = broto_search_IMPORTname(context,name);
		if(im){
			//if its listed in scene's import table, look to see if the mentioned Inline is loaded
			struct X3D_Node *nlinenode;
			*source = 1; //mentioned in IMPORTS
			nlinenode = broto_search_DEFname(context,im->inlinename);
			if(nlinenode && nlinenode->_nodeType == NODE_Inline ){
				struct X3D_Inline *nline = X3D_INLINE(nlinenode);
				if(nline->__loadstatus == INLINE_IMPORTING ||  nline->__loadstatus == INLINE_STABLE){
					//check to see if the loaded inline exports the node
					struct IMEXPORT *ex = broto_search_EXPORTname(X3D_PROTO(nline),im->mxname);
					if(ex){
						node = ex->nodeptr;
						if(node)
							*source = 2;
						if(0){
							//a script in the inline may have tinkered with the DEFnames, so re-lookup
							//can't do this: the export can't act as a char* lookup for DEF -> DEFnames -> node* 
							// because executionContext.updateExportedNode(char*,node*) doesn't have a separate DEF and AS)
							node = broto_search_DEFname(X3D_PROTO(nline),ex->mxname);
							if(node)
								*source = 2; //found via IMPORTs
						}
					}
				}
			}
		}
	}
	return node;
}

void update_weakRoute(struct X3D_Proto *context, struct brotoRoute *route){
	/* we re-search for 'weak' (import node) route ends via (name,node*) 'mapping': 
			name -> DEF-> IMPORT -> DEF -> inline -> EXPORT -> DEF -> node*
	   so whatever parser created, whatever tinkering javascript has done to import names,
	   whatever state inline is in, we'll get the latest mapping of name to node*
	*/
	struct X3D_Node* newnodef, *newnodet; 
	int source, type, kind, ifield, builtIn;
	const char *cname;
	union anyVrml *value;
	int changed = 0;

	newnodef = route->from.node;
	newnodet = route->to.node;
	if(route->from.weak){
		int ic = 0;
		newnodef = broto_search_ALLnames(context,route->from.cnode,&source);
		ic = newnodef != route->from.node;
		changed = changed || ic;
		if(newnodef && ic) {
			route->from.weak = 3; //an extra marker indicating wether its currently 'satisified' or unknown
			getFieldFromNodeAndNameC(newnodef,route->from.cfield,&type,&kind,&ifield,&builtIn, &value, &cname);
			if(ifield < 0) ConsoleMessage("bad FROM field ROUTE %s.%s TO %s.%s\n",route->from.cnode,route->from.cfield,route->to.cnode,route->to.cfield);
			route->from.ifield = ifield;
			route->from.builtIn = builtIn;
			route->from.ftype = type;
			route->ft = type;
		}
		else route->from.weak = 1;
	}
	if(route->to.weak){
		int ic;
		newnodet = broto_search_ALLnames(context,route->to.cnode,&source);
		ic = newnodet != route->to.node;
		changed = changed || ic;
		if(newnodet && ic) {
			route->to.weak = 3; //an extra marker indicating wether its currently 'satisified' or unknown
			getFieldFromNodeAndNameC(newnodet,route->to.cfield,&type,&kind,&ifield,&builtIn,&value,&cname);
			if(ifield < 0) 
				ConsoleMessage("bad TO field ROUTE %s.%s TO %s.%s\n",route->from.cnode,route->from.cfield,route->to.cnode,route->to.cfield);
			route->to.ifield = ifield;
			route->to.builtIn = builtIn;
			route->to.ftype = type;
			route->ft = type;
		}
		else route->to.weak = 1;
	}
	if(changed){
		if(route->lastCommand){
			//its registered, so unregister
			CRoutes_RemoveSimpleB(route->from.node,route->from.ifield,route->from.builtIn,route->to.node,route->to.ifield,route->to.builtIn,route->ft);
			route->lastCommand = 0;
		}
		route->from.node = newnodef;
		route->to.node = newnodet;
		if(route->from.node && route->to.node && route->from.ifield > -1 && route->to.ifield > -1){ //both satisfied
			route->lastCommand = 1;
			CRoutes_RegisterSimpleB(route->from.node,route->from.ifield,route->from.builtIn,route->to.node,route->to.ifield,route->to.builtIn,route->ft);
		}
	}
}
void update_weakRoutes(struct X3D_Proto *context){
	/* Goal: update any routes relying on imports -registering or unregistering- that change as Inlines are loaded and unloaded,
		and/or as javascript tinkers with import names or def names
		Oct 2014 implementation: we don't have a way to recursively update all contexts once per frame.
		So we need to catch any changes caused by parsing, inline load/unload, and javascript tinkering with DEF and IMPORT names.
		This function is designed general (and wasteful) enough so that it can be called from anywhere
		in the current context: during javascript tinkering, during parsing, and (future) during recursive per-frame context updating
		PROBLEM: if an inline changes one of its exports, nothing triggers this update, because to call update_weakRoutes, it would need to know 
		the importing scene context, which it doesn't.
	*/
	if(context && context->__ROUTES){
		//in theory we could have a separate __WEAKROUTE vector with entries that point to any weak __ROUTES so it's not so wasteful,
		//  but then we need to maintain that __WEAKROUTE vector, when adding/removing routes during parsing or javascript.
		//  its handy to keep both strong and weak routes in one __ROUTES array for javascript currentScene.routes.length and routes[i].fromNode etc
		//  for now (Oct 2014) we'll do a big wasteful loop over all routes.
		int k;
		for(k=0;k<vectorSize(context->__ROUTES);k++){
			struct brotoRoute *route = vector_get(struct brotoRoute *,context->__ROUTES,k);
			if(route->from.weak || route->to.weak){
				update_weakRoute(context,route);
			}
		}
	}
}
struct X3D_Proto *hasContext(struct X3D_Node* node);


int unload_broto(struct X3D_Proto* node);
/* note that we get the resources in a couple of steps; this tries to keep the scenegraph running */
void load_Inline (struct X3D_Inline *node) {
	resource_item_t *res;
	struct X3D_Proto *context;
    //printf ("load_Inline, node %p loadStatus %d url %s\n",node,node->__loadstatus,node->url.p[0]->strptr);
	/* printf ("loading Inline\n");  */

	switch (node->__loadstatus) {
		case INLINE_INITIAL_STATE: /* nothing happened yet */
		if(node->load){
			if (node->url.n == 0) {
				node->__loadstatus = INLINE_STABLE; /* a "do-nothing" approach */
			} else {
				//wrong parent resource? see Component_DIS note on parsing vs rendering _parentResource
				//parsing: comes from a stack which is pushed and popped
				//rendering creation of inlines: comes from parent context's _parentResource
				res = resource_create_multi(&(node->url));
				res->media_type = resm_unknown;
				node->__loadstatus = INLINE_REQUEST_RESOURCE;
				node->__loadResource = res;
			}
		}
		break;

		case INLINE_REQUEST_RESOURCE:
		res = node->__loadResource;
		resource_identify(node->_parentResource, res);
		/* printf ("load_Inline, we have type  %s  status %s\n",
			resourceTypeToString(res->type), resourceStatusToString(res->status)); */
		res->actions = resa_download | resa_load; //not resa_parse which we do below
		//frontenditem_enqueue(ml_new(res));
		resitem_enqueue(ml_new(res));
		//printf("fetching..");
		node->__loadstatus = INLINE_FETCHING_RESOURCE;
		break;

		case INLINE_FETCHING_RESOURCE:
		res = node->__loadResource;
		//printf ("load_Inline, we have type  %s  status %s\n",
		//	resourceTypeToString(res->type), resourceStatusToString(res->status));
		if(res->complete){
			if (res->status == ress_loaded) {
				//determined during load process by resource_identify_type(): res->media_type = resm_vrml; //resm_unknown;
				res->ectx = (void*)node;
				res->whereToPlaceData = X3D_NODE(node);
				res->offsetFromWhereToPlaceData = offsetof (struct X3D_Inline, __children);
				res->actions = resa_process;
				node->__loadstatus = INLINE_PARSING; // a "do-nothing" approach 
				//tell it to instance (vs library)
				node->__protoFlags = ciflag_set(node->__protoFlags,1,0);
				res->complete = FALSE;
				//send_resource_to_parser(res);
				//send_resource_to_parser_if_available(res);
				resitem_enqueue(ml_new(res));
				//printf("parsing..");
			} else if ((res->status == ress_failed) || (res->status == ress_invalid)) {
				//no hope left
				//printf ("resource failed to load\n");
				node->__loadstatus = INLINE_STABLE; // a "do-nothing" approach 
			}
		}
		break;

		case INLINE_PARSING:
			res = node->__loadResource;

			//printf ("inline parsing.... %s\n",resourceStatusToString(res->status));
			//printf ("res complete %d\n",res->complete);
			if(res->complete){
				if (res->status == ress_parsed) {
					/* this might be a good place to populate parent context IMPORT table with our EXPORT nodes? */
					node->__loadstatus = INLINE_IMPORTING; //INLINE_STABLE; 

				} 
			}

		break;
		case INLINE_IMPORTING:
			//printf("importing..");
			context = hasContext(node->_executionContext);
			if(context)
				update_weakRoutes(context);
			node->__loadstatus = INLINE_STABLE;
		break;
		case INLINE_STABLE:
		if(!node->load){
			//printf ("unloading Inline..\n");
			node->__loadstatus = INLINE_UN_IMPORTING; //INITIAL_STATE;
		}

		break;
		case INLINE_UN_IMPORTING:
			//printf("un-importing..");
			context = hasContext(node->_executionContext);
			if(context)
				update_weakRoutes(context); //remove any imported routes so no dangling route pointers
			node->__loadstatus = INLINE_UNLOADING;
			break;
		case INLINE_UNLOADING:
			//printf("unloading ..");
			/* missing code to unload inline 
				The same (missing) cleanup function could also be used to unload scene and protoInstances, and 
				the garbage collection part can be used on protoDeclares, externProtoDeclares,
				and extern proto library scenes. All these use X3D_Proto == X3D_Inline struct 
				with a few X3D_Proto.__protoFlags distinguishing their use at runtime.
			A. unregister items registered in global/browser structs
				a  remove registered sensors -need a __sensors array?
				b. remove registered scripts -see __scripts
				c. remove registered routes:
					c.i regular routes -from __ROUTES table
					c.ii IS construction routes - from __IStable - a function was developed but not yet tested: unregister_IStableRoutes
				d unregister nodes from table used by startofloopnodeupdates - see createNewX3DNode vs createNewX3DNode0 in generatedCode.c
			B. deallocate context-specific heap: 
				a nodes allocated -need a context-specific nodes heap
					a.0 recursively unload sub-contexts: inlines and protoInstances
					a.1 builtin nodes
				b. context vectors: __ROUTES, __IMPORTS, __EXPORTS, __DEFnames, __scripts, addChildren, removeChildren, _children
				c prototypes declared: __protoDeclares, __externProtoDeclares - use same recursive unload
				d string heap -need a string heap
				e malloc heap used for elements of __ vectors - need a context-specific malloc and heap ie fmalloc(context,sizeof)
			C. clear/reset scalar values so Inline can be re-used/re-loaded: (not sure, likely none to worry about)
			*/
			//node->__children.n = 0; //this hack will make it look like it's unloaded, but chaos results with a subsequent reload
			unload_broto(X3D_PROTO(node));
			node->__loadstatus = INLINE_INITIAL_STATE;
			//printf("unloaded..\n");
			break;
		default:
			break; //if its part way loaded, we'll wait till it finishes.
	}
}

void prep_Inline (struct X3D_Inline *node) {
	if(0)printf("in prep_inline\n");
	//load_externProtoInstance(node);
	COMPILE_IF_REQUIRED
	if ((node->__loadstatus != INLINE_STABLE && node->load) || (node->__loadstatus != INLINE_INITIAL_STATE && !node->load)) {
		load_Inline(node);
	}
	RECORD_DISTANCE

}
/* not sure why we would compile */
void compile_Inline(struct X3D_Inline *node) {
	if(0)printf("in compile_inline\n");
	//unsigned char pflag = ciflag_get(node->__protoFlags,2);
	//if(pflag == 2){
		//scene
		REINITIALIZE_SORTED_NODES_FIELD(node->__children,node->_sortedChildren);
	//}
	{
		int loadchanged, urlchanged;
		// something in resource fetch or startofloopnodeupdates sets the node changed flag,
		// (not sure where, but likely to indicate _children have changed and node needs compiling) 
		// and we aren't interested in that here,
		// just in the url and load fields changing, so we compare to last recorded values
		loadchanged = urlchanged = 0;
		loadchanged = node->load != node->__oldload;
		urlchanged = node->url.n != node->__oldurl.n || node->url.p != node->__oldurl.p;
		if(loadchanged || urlchanged){
			//whether we are loading a new url, or unloading, we always start with an unconditional unload
			node->__loadstatus = INLINE_UN_IMPORTING;
			if(loadchanged) node->__oldload = node->load;
			if(urlchanged) node->__oldurl = node->url;  //we don't need to strdup the url strings, assuming the old p* from a malloc doesn't get re-used/remalloced for a new url
			//MARK_NODE_COMPILED
		}
	} 
	MARK_NODE_COMPILED
}
void prep_unitscale (struct X3D_Proto *ec);
void fin_unitscale (struct X3D_Proto *ec);
void child_Inline (struct X3D_Inline *node) {

	//static int usingSortedChildren = 0;
	//struct Multi_Node * kids;
	CHILDREN_COUNT
	//int nc = node->__children.n; //_sortedChildren.n;
	//LOCAL_LIGHT_SAVE

	RETURN_FROM_CHILD_IF_NOT_FOR_ME
	prep_unitscale(X3D_PROTO(node));
	prep_sibAffectors((struct X3D_Node*)node,&node->__sibAffectors);
	//LOCAL_LIGHT_CHILDREN(node->_sortedChildren);
	prep_BBox((struct BBoxFields*)&node->bboxCenter);

	normalChildren(node->_sortedChildren);

	fin_BBox((struct X3D_Node*)node,(struct BBoxFields*)&node->bboxCenter,FALSE);
	fin_sibAffectors((struct X3D_Node*)node,&node->__sibAffectors);
	fin_unitscale(X3D_PROTO(node));
	//LOCAL_LIGHT_OFF

}

// GLTF
//https://github.com/jkuhlmann/cgltf 
//- include 100 line recursive json parser (how does data come out?) etc.
//- first 600 lines of header is API. next 4000 lines is CGLTF_IMPLEMENTATION
#define  CGLTF_IMPLEMENTATION 1
#include "cgltf.h"
struct name_node {
char *name;
struct X3D_Node * node;
};
static Stack *defs = NULL;
struct X3D_Node *USE_node(char *name){
	if(!defs) defs = newStack(struct name_node);
	struct name_node nn;
	for(int i=0;i<defs->n;i++){
		nn = vector_get(struct name_node,defs,i);
		if(!strcmp(name,nn.name)){
			return nn.node;
		}
	}
	return NULL;
 }
 struct X3D_Node *DEF_node(char *name, int nodetype){
	if(!defs) defs = newStack(struct name_node);
	struct name_node nn;
	struct X3D_Node *node = createNewX3DNode0(nodetype);
	nn.name = name;
	nn.node = node;
	stack_push(struct name_node,defs,nn);
	return node;
 }
int parse_gltf_node(struct X3D_Node *ectx, struct X3D_Node **spot, cgltf_data * data, cgltf_node *node){
	//transform part
	int m = 0;
	struct X3D_Transform *t = createNewX3DNode0(NODE_Transform);
	if(node->has_matrix){
		//parse matrix into TRS
		//
	}else{
		if(node->has_rotation){
			veccopy3f(t->rotation.c,&node->rotation[1]); 
			t->rotation.c[3] = node->rotation[0];
		}
		if(node->has_scale){
			veccopy3f(t->scale.c,node->scale);
		}
		if(node->has_translation){
			veccopy3f(t->translation.c,node->translation);
		}
	}
	//content part
	if(node->camera){
		m++;
		t->children.p = realloc(t->children.p,m*sizeof(struct X3D_Node*));
	}
	if(node->light){
		m++;
		t->children.p = realloc(t->children.p,m*sizeof(struct X3D_Node*));
	}
	if(node->mesh){
		//gltf mesh is like our shape: it refers to material and to geometry/accessor
		m++;
		t->children.p = realloc(t->children.p,m*sizeof(struct X3D_Node*));
		struct X3D_Shape *sn = (struct X3D_Shape*) USE_node(node->mesh->name);
		if(!sn){
			sn = (struct X3D_Shape*) DEF_node(node->mesh->name,NODE_Shape);
			for(int j=0;j<node->mesh->primitives_count;j++){
				cgltf_primitive prim = node->mesh->primitives[j];
				if(prim.material){
					if(prim.material->unlit){
						int mtype = NODE_UnlitMaterial;
						struct X3D_UnlitMaterial* mat = (struct X3D_UnlitMaterial*) USE_node(prim.material->name);
						if(!mat){
							mat = (struct X3D_UnlitMaterial*) DEF_node(prim.material->name,mtype);
							//fill in 
						}
					}else{
						int mtype = NODE_PhysicalMaterial;
						struct X3D_PhysicalMaterial* mat = (struct X3D_PhysicalMaterial*) USE_node(prim.material->name);
						if(!mat){
							mat = (struct X3D_PhysicalMaterial*) DEF_node(prim.material->name,mtype);
						}
					} 
				}
				// https://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/rendering.html
				// https://www.khronos.org/files/gltf20-reference-guide.pdf
				struct X3D_Node *gn = NULL;
				switch(prim.type){
					case cgltf_primitive_type_points:
					case cgltf_primitive_type_lines:
					case cgltf_primitive_type_line_loop:
					case cgltf_primitive_type_line_strip:
						break;
					case cgltf_primitive_type_triangles:
						{
						cgltf_float element_float[16];
						int acount = prim.attributes_count;
						if(1){
							printf("triangles\n");
							for(int ii=0;ii<acount;ii++){
								printf("attr %s indx %d ",prim.attributes[ii].name,prim.attributes[ii].index);
								switch(prim.attributes[ii].type){
									case cgltf_attribute_type_invalid: printf("invalid");break;
									case cgltf_attribute_type_position: printf("position");break;
									case cgltf_attribute_type_normal: printf("normal");break;
									case cgltf_attribute_type_tangent: printf("tangent");break;
									case cgltf_attribute_type_texcoord: printf("texcoord");break;
									case cgltf_attribute_type_color: printf("color");break;
									case cgltf_attribute_type_joints: printf("joints");break;
									case cgltf_attribute_type_weights: printf("weights");break;
									default: break;
								}
							
								const cgltf_accessor* blob = prim.attributes[ii].data;
								cgltf_size nfloats = cgltf_num_components(blob->type) * blob->count;
								printf(" nfloats = %d accessor type %d count %d ",nfloats,blob->type,blob->count);
								switch(blob->type){
									case cgltf_type_scalar: printf("SCALAR");break;
									case cgltf_type_vec2: printf("VEC2");break;
									case cgltf_type_vec3: printf("VEC3");break;
									default: break;
								}
								printf("\n");
								cgltf_float element_float[16];
								for (cgltf_size index = 0; index < blob->count; index++)
								{
									cgltf_accessor_read_float(blob, index, element_float, 16);
									printf("%d %f %f %f\n",index,element_float[0],element_float[1],element_float[2]);
								}

							}
						}
						gn = createNewX3DNode0(NODE_TriangleSet);
						//gn->_intern = 
						}
						break;
					case cgltf_primitive_type_triangle_strip:
						printf("triangle strip\n");
						break;
					case cgltf_primitive_type_triangle_fan:
						printf("triangle fan\n");
						break;
				}
			}
		}
		t->children.p[m-1] = X3D_NODE(sn);
	}
	if(node->skin){
		m++;
		t->children.p = realloc(t->children.p,m*sizeof(struct X3D_Node*));
	}
	if(node->weights_count){
	}
	size_t estart = node->extras.start_offset;
	size_t eend = node->extras.end_offset;
	//children part
	int mc = node->children_count;
	if(mc){
		t->children.p = realloc(t->children.p,(mc+m)*sizeof(struct X3D_Node*));
		for(int i=0;i<mc;i++){
			parse_gltf_node(ectx,&t->children.p[i+m],data,node->children[i]);
		}
		m += mc;
	}
	t->children.n = m;
	return TRUE;
}

int parse_gltf(struct X3D_Node *ectx, struct Multi_Node *spot, cgltf_data * data){
	int n = data->scene[0].nodes_count;
	spot->p = realloc(spot->p, n * sizeof(struct X3D_Node *));
	for(int i=0;i<data->scene[0].nodes_count;i++){
		parse_gltf_node(ectx,&spot->p[i],data,data->scene[0].nodes[i]);
	}
	spot->n = n;
	int ret = TRUE;
	return ret;
}


//ret = X3DParse(ectx, X3D_NODE(nRn), (const char*)input);
int parser_do_parse_gltf(const char *input, const int len, struct X3D_Node *ectx, struct X3D_Node *myParent)
{
	// ectx - the context node - either Inline or Scene
	// rNr temporary group container node where we'll put the new nodes as children (should have been struct MFNode * field of container)
	int ret = FALSE;
	cgltf_options options;
	memset(&options, 0, sizeof(cgltf_options));
	cgltf_data* data = NULL;

	cgltf_result result = cgltf_parse(	&options, (void*) input, len, &data);
	if (result == cgltf_result_success)
	{
		printf("gltf parsed into cgltf scene struct\n");
		/* TODO make awesome stuff */
		result = cgltf_load_buffers(&options, data, "./");
		if(result == cgltf_result_success){
			// 1. go over struct, creating x3d nodes and nesting them
			struct Multi_Node *spot;
			if(myParent->_nodeType == NODE_Proto )
				spot = &((struct X3D_Proto*)(myParent))->__children;
			else
				spot = &((struct X3D_Group*)(myParent))->children;
			spot->p = NULL; spot->n = 0;
			parse_gltf(ectx,spot,data);
			// 2. for exta files send url request and have a place to put it in the x3d node created for it
			// documentation: """Note that cgltf does not load the contents of extra files such as buffers or images into memory by default. 
			//	You'll need to read these files yourself using URIs from data.buffers[] or data.images[] respectively. """
			cgltf_free(data);
			ret = TRUE;
		}
	}

	return ret;
}

int parser_process_res_gltf(resource_item_t *res){
	int parsedOk = FALSE;
	switch(res->media_type){
		case resm_gltf: 
		case resm_glb:
			//these media types generate x3d scene nodes and can be a scene unto themselves, or inline body
			//they can also request more resources which are placed in their node fields.
			parsedOk = parser_process_res_VRML_X3D(res);
			break;
		case resm_bin:
			//gltf can be exported with separate binary buffer file
			// the buffer needs to catch up to / join into the parsed gltf
			// before we parse/convert gltf into our freewrl-type geometry nodes
			break;
		//cesium related
		case resm_json:
			break;
		case resm_b3dm:
			break;
		case resm_i3dm:
			break;
		case resm_pnts:
			break;
		case resm_cmpt:
			break;
	
	}
	return parsedOk;
}

