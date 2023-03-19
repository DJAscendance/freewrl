/*


X3D Sound Component

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

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h"
#include "../vrml_parser/CRoutes.h"
#include "../main/headers.h"
#include "../opengl/OpenGL_Utils.h"

#include "LinearAlgebra.h"

#define BADAUDIOSOURCE -9999
//#undef HAVE_LIBSOUND
#ifdef HAVE_LIBSOUND
#undef HAVE_OPENAL
#endif //HAVE_LIBSOUND
#ifdef HAVE_OPENAL
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#ifdef HAVE_ALUT
#include <AL/alut.h>
#endif //HAVE_ALUT
/* InitAL opens the default device and sets up a context using default
 * attributes, making the program ready to call OpenAL functions. */
void* fwInitAL(void)
{
    ALCdevice *device;
    ALCcontext *ctx;

    /* Open and initialize a device with default settings */
    device = alcOpenDevice(NULL);
    if(!device)
    {
        fprintf(stderr, "Could not open a device!\n");
        return NULL;
    }

    ctx = alcCreateContext(device, NULL);
    if(ctx == NULL || alcMakeContextCurrent(ctx) == ALC_FALSE)
    {
        if(ctx != NULL)
            alcDestroyContext(ctx);
        alcCloseDevice(device);
        fprintf(stderr, "Could not set a context!\n");
        return NULL;
    }

    printf("Opened \"%s\"\n", alcGetString(device, ALC_DEVICE_SPECIFIER));
    return ctx;
}

/* CloseAL closes the device belonging to the current context, and destroys the
 * context. */
void fwCloseAL(void *alctx)
{
    ALCdevice *device;
    ALCcontext *ctx;

    //ctx = alcGetCurrentContext();
	ctx = alctx;
    if(ctx == NULL)
        return;

    device = alcGetContextsDevice(ctx);

    alcMakeContextCurrent(NULL);
    alcDestroyContext(ctx);
    alcCloseDevice(device);
}

#endif
#ifdef HAVE_LIBSOUND
#include "../../libsound/libsound.h"
//libsound is our /src/libsound C wrapper lib over 
// LabSound https://github.com/LabSound/LabSound 
#endif //HAVE_LIBSOUND
typedef struct pComponent_Sound{
#ifdef HAVE_OPENAL
	void *alContext;
#endif //HAVE_OPENAL
	Stack *audio_context_stack;
	Stack *audio_parent_stack;
	Stack* doppler_factor_stack;
	Stack* splitter_source_stack;
}* ppComponent_Sound;
void *Component_Sound_constructor(){
	void *v = MALLOCV(sizeof(struct pComponent_Sound));
	memset(v,0,sizeof(struct pComponent_Sound));
	return v;
}
void Component_Sound_init(struct tComponent_Sound *t){
	//public
	/* Sounds can come from AudioClip nodes, or from MovieTexture nodes. Different
   structures on these */
	t->sound_from_audioclip= 0;

	/* is the sound engine started yet? */
	t->SoundEngineStarted = FALSE;
	//private
	t->prv = Component_Sound_constructor();
	{
		ppComponent_Sound p = (ppComponent_Sound)t->prv;
		/* for printing warnings about Sound node problems - only print once per invocation */
		p->audio_context_stack = newStack(int);
		stack_push(int, p->audio_context_stack, 0); //a null will signal we have no audio context yet.
		p->audio_parent_stack = newStack(ivec3);
		icset aps = { 0, 0, 0, 0, 0, 0 };
		stack_push(icset, p->audio_parent_stack, aps); //a null will signal we have no audio parent yet.
		p->doppler_factor_stack = newStack(float);
		stack_push(float, p->doppler_factor_stack, 1.0f);
		p->splitter_source_stack = newStack(ivec2);
		ivec2 sss = { 0, 0 };
		stack_push(ivec2, p->splitter_source_stack, sss);
#ifdef HAVE_OPENAL
		p->alContext = NULL;
#endif //HAVE_OPENAL
	}
}
void Component_Sound_clear(struct tComponent_Sound *t){
	ppComponent_Sound p = (ppComponent_Sound)t->prv;
	deleteVector(struct X3D_Node*,p->audio_context_stack);
}
//ppComponent_Sound p = (ppComponent_Sound)gglobal()->Component_Sound.prv;

// Position of the listener.
float ListenerPos[] = { 0.0, 0.0, 0.0 };
// Velocity of the listener.
float ListenerVel[] = { 0.0, 0.0, 0.0 };
// Orientation of the listener. (first 3 elements are "at", second 3 are "up")
float ListenerOri[] = { 0.0, 0.0, -1.0, 0.0, 1.0, 0.0 };

int SoundEngineInit(void)
{
	int retval = FALSE;
#ifdef HAVE_OPENAL
	{
		void *alctx;
		ppComponent_Sound p = (ppComponent_Sound)gglobal()->Component_Sound.prv;
		retval = TRUE;
		/* Initialize OpenAL with the default device, and check for EFX support. */
		alctx = fwInitAL();
		if(!alctx ){
			ConsoleMessage("initAL failed\n");
			retval = FALSE;
		}
		p->alContext = alctx;
#ifdef HAVE_ALUT
		if(!alutInitWithoutContext(NULL,NULL)) //this does not create an AL context (simple)
		{
			ALenum error = alutGetError ();
			ConsoleMessage("%s\n", alutGetErrorString (error));
			retval = FALSE;
		}
#endif //HAVE_ALUT

		//listener is avatar,
		//we could move both listener and sources in world coordinates
		//instead we'll work in avatar/local coordinates and
		//freeze listener at 0,0,0 and update the position of the sound sources 
		//relative to listener, on each frame
		alListenerfv(AL_POSITION,    ListenerPos);
		alListenerfv(AL_VELOCITY,    ListenerVel);
		alListenerfv(AL_ORIENTATION, ListenerOri);
		if(1){
			//ALenum error;
			if(FALSE) //meters)
				alSpeedOfSound(345.0f); //alDopplerVelocity(34.0f); //m/s
			else //feet
				alSpeedOfSound(1132.0f); //alDopplerVelocity(1132.0f); // using feet/second – change propagation velocity 
			alDopplerFactor(1.0f); // exaggerate pitch shift by 20% 
			//if ((error = alGetError()) != AL_NO_ERROR) DisplayALError("alDopplerX : ", error);
		}
		alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED); //here's what I think web3d wants
		//alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED); //seems a bit faint
	}
#endif //HAVE_OPENAL
#ifdef HAVE_LIBSOUND
	// labsound does have some resources --HRDF table and a few more things I think-- which would be 'installed' 
	//  and would need to be loaded at some point from a known path. Maybe loaded here.
	retval = TRUE;
#endif //HAVE_LIBSOUND
	gglobal()->Component_Sound.SoundEngineStarted = retval;
	return retval;
}

int haveSoundEngine(){
	ttglobal tg = gglobal();

	if (!tg->Component_Sound.SoundEngineStarted) {
		#ifdef SEVERBOSE
		printf ("SetAudioActive: initializing SoundEngine\n");
		#endif
		tg->Component_Sound.SoundEngineStarted = SoundEngineInit();
	}
	return tg->Component_Sound.SoundEngineStarted;
}


#define LOAD_INITIAL_STATE 0
#define LOAD_REQUEST_RESOURCE 1
#define LOAD_FETCHING_RESOURCE 2
//#define LOAD_PARSING 3
#define LOAD_STABLE 10

void locateAudioSource (struct X3D_AudioBuffer *node) {
	resource_item_t *res;
	//resource_item_t *parentPath;
	//ppComponent_Sound p = (ppComponent_Sound)gglobal()->Component_Sound.prv;
	int debug = 1;
	if(debug) printf("\nurl %s\n", node->url.p[0]->strptr);
	switch (node->__loadstatus) {
		case LOAD_INITIAL_STATE: /* nothing happened yet */

		if (node->url.n == 0) {
			node->__loadstatus = LOAD_STABLE; /* a "do-nothing" approach */
			break;
		} else {
			res = resource_create_multi(&(node->url));
			if(node->_nodeType == NODE_MovieTexture)
				res->media_type = resm_movie;
			else //if(node->_nodeType == NODE_AudioClip)
				res->media_type = resm_audio;
			node->__loadstatus = LOAD_REQUEST_RESOURCE;
			node->__loadResource = res;
		}
		if(debug) printf("1");
		break;

		case LOAD_REQUEST_RESOURCE:
		res = node->__loadResource;
		resource_identify(node->_parentResource, res);
		res->actions = resa_download | resa_load; //not resa_parse which we do below
		res->ectx = (void*)node->_executionContext;
		res->whereToPlaceData = X3D_NODE(node);
		//res->offsetFromWhereToPlaceData = offsetof (struct X3D_AudioClip, __FILEBLOB);
		resitem_enqueue(ml_new(res));
		node->__loadstatus = LOAD_FETCHING_RESOURCE;
		if(debug) printf("2");
		break;

		case LOAD_FETCHING_RESOURCE:
		res = node->__loadResource;
		/* printf ("load_Inline, we have type  %s  status %s\n",
			resourceTypeToString(res->type), resourceStatusToString(res->status)); */
		if(res->complete){
			if (res->status == ress_loaded) {
				if (res->actions != resa_process) {
					res->actions = resa_process;
					res->complete = FALSE;
					resitem_enqueue(ml_new(res));
				}
			} else if ((res->status == ress_failed) || (res->status == ress_invalid)) {
				//no hope left
				printf ("resource failed to load\n");
				for(int ii=0;ii<node->url.n;ii++)
					printf ("-- url[%d]=%s\n",ii,node->url.p[ii]->strptr);
				node->__loadstatus = LOAD_STABLE; // a "do-nothing" approach 
				node->__sourceNumber = BADAUDIOSOURCE;
			} else	if (res->status == ress_parsed) {
				node->__loadstatus = LOAD_STABLE; 
			} //if (res->status == ress_parsed)
		} //if(res->complete)
		//end case LOAD_FETCHING_RESOURCE
		if(debug) printf("3");
		break;

		case LOAD_STABLE:
		if(debug) printf("4");
		break;
	}
}
int loadstatus_AudioClip(struct X3D_AudioClip *node){
	int istate = 0;
	if(node){
		if(node->__loadstatus > LOAD_INITIAL_STATE && node->__loadstatus < LOAD_STABLE)
			istate = 1;
		if(node->__loadstatus == LOAD_STABLE)
			istate = 2;
	}
	return istate;
}
//void compile_AudioClip(struct X3D_AudioClip* node) {
//
//}




int	parse_audioclip(struct X3D_AudioClip* node, char* bbuffer, int len, char* url) {
#ifdef HAVE_OPENAL
	ALint buffer = AL_NONE;
#ifdef HAVE_ALUT
	buffer = alutCreateBufferFromFileImage(bbuffer, len);
	//#elif HAVE_SDL
#endif
	if (buffer == AL_NONE)
		buffer = BADAUDIOSOURCE;
#elif HAVE_LIBSOUND
	int buffer;
	//if(0)
		buffer = libsound_createBusFromBuffer0(bbuffer, len);
	//else
	//	buffer = libsound_createBusFromFile0(url);
#else
	int buffer = BADAUDIOSOURCE;
#endif
	//printf("parse_audioclip buffer=%d\n",buffer);
	return buffer;
}

double compute_duration(int ibuffer) {

	double retval = 1.0;
#ifdef HAVE_OPENAL
	int ibytes;
	int ibits;
	int ichannels;
	int ifreq;
	double framesizebytes, bytespersecond;
	alGetBufferi(ibuffer, AL_FREQUENCY, &ifreq);
	alGetBufferi(ibuffer, AL_BITS, &ibits);
	alGetBufferi(ibuffer, AL_CHANNELS, &ichannels);
	alGetBufferi(ibuffer, AL_SIZE, &ibytes);
	framesizebytes = (double)(ibits * ichannels) / 8.0;
	bytespersecond = framesizebytes * (double)ifreq;
	if (bytespersecond > 0.0)
		retval = (double)(ibytes) / bytespersecond;
	else
		retval = 1.0;
#endif //HAVE_OPENAL
#ifdef HAVE_LIBSOUND
	retval = libsound_computeDuration0(ibuffer);
#endif //HAVE_LIBSOUND
	return retval;
}
bool  process_res_audio(resource_item_t* res) {
	//s_list_t *l;
	openned_file_t* of;
	//struct Shader_Script* ss;
	char* buffer;
	int len;
	struct X3D_AudioClip* node;

	buffer = NULL;
	len = 0;
	switch (res->type) {
	case rest_invalid:
		return FALSE;
		break;

	case rest_string:
		buffer = res->URLrequest;
		break;
	case rest_url:
	case rest_file:
	case rest_multi:
		//l = (s_list_t *) res->openned_files;
		//if (!l) {
		//	/* error */
		//	return FALSE;
		//}

		//of = ml_elem(l);
		of = res->openned_files;
		if (!of) {
			/* error */
			return FALSE;
		}

		buffer = of->fileData;
		len = of->fileDataSize;
		break;
	}

	node = (struct X3D_AudioClip*)res->whereToPlaceData;
	//node->__FILEBLOB = buffer;
	node->__sourceNumber = parse_audioclip(node, buffer, len, res->actual_file); //__sourceNumber will be openAL buffer number
	if (node->__sourceNumber > -1 ) {
		if (node->_nodeType == NODE_AudioClip) {
			node->duration_changed = compute_duration(node->__sourceNumber);
			MARK_EVENT(X3D_NODE(node), offsetof(struct X3D_AudioClip, duration_changed));
		}
		return TRUE;
	}
	return FALSE;
}


/* returns the audio duration, unscaled by pitch */
double return_Duration(struct X3D_AudioClip* node) {
	double retval;
	int indx;
	indx = node->__sourceNumber;
	if (indx < 0)  retval = 1.0;
	else if (indx > 50) retval = 1.0;
	else
	{
#if defined(HAVE_OPENAL) || defined(HAVE_LIBSOUND)
		retval = node->duration_changed;
#endif
	}
	return retval;
}




#ifdef HAVE_OPENAL

void render_AudioClip(struct X3D_AudioClip* node) {
	/*  audio clip is a flat sound -no 3D- and a sound node (3D) refers to it
		specs: if an audioclip can't be reached in the scenegraph, then it doesn't play
	*/

	/* is this audio wavelet initialized yet? */
	if (node->__loadstatus != LOAD_STABLE) {
		locateAudioSource(node);
	}
	if (node->__loadstatus != LOAD_STABLE) return;
	/* is this audio ok? if so, the sourceNumber will range
	 * between 0 and infinity; if it is BADAUDIOSOURCE, bad source.
	 * check out locateAudioSource to find out reasons */
	if (node->__sourceNumber == BADAUDIOSOURCE) return;

}

void render_Sound (struct X3D_Sound *node) {
/*  updates the position and velocity vector of the sound source relative to the listener/avatar
	so 3D sound effects can be rendered: distance attenuation, stereo left/right volume balance, 
	and doppler (pitch) effect
	- refers to sound source ie audioclip or movie
	- an audioclip may be DEFed and USEd in multiple Sounds, but will be playing the same tune at the same time
*/
	int sound_from_audioclip;

	struct X3D_AudioClip *acp = NULL;
	struct X3D_MovieTexture *mcp = NULL;
	struct X3D_Node *tmpN = NULL;
	//ppComponent_Sound p = (ppComponent_Sound)gglobal()->Component_Sound.prv;

	/* why bother doing this if there is no source? */
	if (node->source == NULL) 
		return;

	/* ok, is the source a valid node?? */

	/* might be a PROTO expansion, as in what Adam Nash does... */
	POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->source,tmpN)

	/* did not find a valid source node, even after really looking at a PROTO def */
	if (tmpN == NULL) return;

	sound_from_audioclip = FALSE;
	if (tmpN->_nodeType == NODE_AudioClip) {
		acp = (struct X3D_AudioClip *) tmpN;
		sound_from_audioclip = TRUE;
	}else if (tmpN->_nodeType == NODE_MovieTexture){
		//mcp = (struct X3D_MovieTexture *) tmpN;
		//july 2016 ordered fields in MovieTexture to mach AudioClip, can up-caste
		acp = (struct X3D_AudioClip *) tmpN;
	} else {
		ConsoleMessage ("Sound node- source type of %s invalid",stringNodeType(tmpN->_nodeType));
		node->source = NULL; /* stop messages from scrolling forever */
		return;
	}


	/*  4 sources of openAL explanations and examples:
		- http://open-activewrl.sourceforge.net/data/OpenAL_PGuide.pdf  
		- http://forum.devmaster.net/t  and type 'openal' in the search box to get several lessons on openal
		- http://kcat.strangesoft.net/openal.html  example code (win32 desktop is using this openal-soft implementation of openal)
		- http://en.wikipedia.org/wiki/OpenAL links
		- <al.h> comments
	*/
	if(acp){
		if(haveSoundEngine()){
			if( acp->__sourceNumber < 0){
				render_AudioClip(acp);
			}
			if( acp->__sourceNumber > -1 ){
				//have a buffer loaded
				int i;
				GLDOUBLE modelMatrix[16];
				GLDOUBLE SourcePosd[3] = { 0.0f, 0.0f, 0.0f };
				ALfloat SourcePos[3];

				//transform source local coordinate 0,0,0 location into avatar/listener space
				FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, modelMatrix);
				transformAFFINEd(SourcePosd,SourcePosd,modelMatrix);
				for(i=0;i<3;i++) SourcePos[i] = (ALfloat)SourcePosd[i];

				if( node->__sourceNumber < 0){
					//convert buffer to openAL sound source
					ALint source;
					source = 0;
					alGenSources(1, &source);
					alSourcei(source, AL_BUFFER, acp->__sourceNumber);
					alSourcef (source, AL_PITCH,    acp->pitch);
					alSourcef (source, AL_GAIN,     node->intensity );
					alSourcei (source, AL_LOOPING,  acp->loop);
					alSourcei (source, AL_SOURCE_RELATIVE, AL_TRUE);  //we'll treat the avatar/listener as fixed, and the sources moving relative
					//openAL will automatically mix multiple sources for one listener, there's no need for .priority hint
					alSourcef (source, AL_MAX_DISTANCE, node->maxFront);
					//no attempt is made to implement minBack, maxBack ellipsoidal as in web3d specs
					//- just a spherical sound, and with spatialize attempt at a cone
					node->__lasttime = TickTime();
					veccopy3f(node->__lastlocation.c,SourcePos);

					node->__sourceNumber = source;
					//assert(alGetError()==AL_NO_ERROR && "Failed to setup sound source");
					if(alGetError()!=AL_NO_ERROR) {
						static int once = 0;
						if(!once){
							ConsoleMessage("Failed to setup sound source\n");
							once = 1;
						}
						node->__sourceNumber = BADAUDIOSOURCE;
					}
				}
				if( node->__sourceNumber > -1){
					int istate;
					ALfloat SourceVel[3] = { 0.0f, 0.0f, 0.0f };
					float travelled[3];
					double traveltime;

					//update position
					alSourcefv(node->__sourceNumber, AL_POSITION, SourcePos);

					//update velocity for doppler effect
					vecdif3f(travelled,node->__lastlocation.c,SourcePos);
					traveltime = TickTime() - node->__lasttime;
					if(traveltime > 0.0)
						vecscale3f(SourceVel,travelled,1.0f/(float)traveltime);
					alSourcefv(node->__sourceNumber, AL_VELOCITY, SourceVel);

					node->__lasttime = TickTime();
					veccopy3f(node->__lastlocation.c,SourcePos);

					//directional sound - I don't hear directional effects with openAL-Soft
					//AL_CONE_OUTER_GAIN f the gain when outside the oriented cone 
					//AL_CONE_INNER_ANGLE f, i the gain when inside the oriented cone 
					//AL_CONE_OUTER_ANGLE f, i outer angle of the sound cone, in degrees default is 360 
					if(node->spatialize){
						double dird[3];
						ALfloat dirf[3];
						//transform source direction into avatar/listener space
						for(i=0;i<3;i++) dird[i] = node->direction.c[i];
						transformAFFINEd(dird,dird,modelMatrix);
						for(i=0;i<3;i++) dirf[i] = (float)dird[i];
						if (1)
							alSourcefv(node->__sourceNumber, AL_DIRECTION, dirf);
						else
							alSource3f(node->__sourceNumber, AL_DIRECTION, dirf[0], dirf[1], dirf[2]);
						alSourcef(node->__sourceNumber, AL_CONE_OUTER_GAIN, .5f);
						alSourcef(node->__sourceNumber,AL_CONE_INNER_ANGLE,90.0f);
						alSourcef(node->__sourceNumber,AL_CONE_OUTER_ANGLE,135.0f);
					}

					// for routed values going to audioclip, update values
					alSourcef (node->__sourceNumber, AL_PITCH,    acp->pitch);
					alSourcef (node->__sourceNumber, AL_GAIN,     node->intensity );
					alSourcei (node->__sourceNumber, AL_LOOPING,  acp->loop);
					// update to audioclip start,stop,pause,resume is done in do_AudioTick()
					if(acp->isPaused) alSourcePause(node->__sourceNumber);
					//execute audioclip state
					alGetSourcei(node->__sourceNumber, AL_SOURCE_STATE,&istate);
					if(acp->isActive ){
						if(istate != AL_PLAYING && !acp->isPaused){
							alSourcePlay(node->__sourceNumber);
							//printf(".play.");
						}
					}else{
						if(istate != AL_STOPPED)
							alSourceStop(node->__sourceNumber);
					}
				}
			}
		}
	}

}
void visit_check_sound(struct X3D_Node* node, unsigned int iframe) {}
#endif //HAVE_OPENAL

#ifdef HAVE_LIBSOUND
void register_visit_check(struct X3D_Node* node);
void visit_check_sound(struct X3D_Node* node, unsigned int iframe) {
	struct X3D_SoundRep* srep = getSoundRep(node);
	if (srep->icontext) {
		if (iframe == srep->iframe) {
			libsound_resumeContext0(srep->icontext);
			//libsound_resumeNode0(node);
		} else {
			//not visited on last frame, perhaps in a switch deactivated branch
			//lets pause the context
			libsound_pauseContext0(srep->icontext);
			//libsound_pauseNode0(node);
		}
	}
}
// v4 visibility functions, push & pop (to be) called from all X3DGroupingNode child_ functions
void push_audio_context(int audio_context) {
	ppComponent_Sound p = (ppComponent_Sound)gglobal()->Component_Sound.prv;
	stack_push(int, p->audio_context_stack, audio_context);
}
void create_and_push_audio_context(struct X3D_Node* node) {
	//Hypothesis: Destination / output audio nodes create a context, and child source and processing audio nodes use the context
	struct X3D_SoundRep* srep = getSoundRep(node);
	if (!srep->icontext) {
		int jcontext = peek_audio_context();
		if (!jcontext) {
			jcontext = libsound_createContext0();
		}
		srep->icontext = jcontext;
		register_visit_check(node);
	}
	push_audio_context(srep->icontext);
}
void pop_audio_context() {
	ppComponent_Sound p = (ppComponent_Sound)gglobal()->Component_Sound.prv;
	stack_pop(int, p->audio_context_stack);
}
int peek_audio_context() {
	ppComponent_Sound p = (ppComponent_Sound)gglobal()->Component_Sound.prv;
	return stack_top(int, p->audio_context_stack);
}
void push_audio_parent(int inode) {
	ppComponent_Sound p = (ppComponent_Sound)gglobal()->Component_Sound.prv;
	icset aps = { 0, 0, 0, 0, 0, 0 };
	aps.p = inode;
	aps.d = 0;
	aps.ld = 0;
	stack_push(icset, p->audio_parent_stack, aps);
}
void push_audio_parent3(int inode, int dstChan, int lstDst) {
	ppComponent_Sound p = (ppComponent_Sound)gglobal()->Component_Sound.prv;
	icset aps = { 0, 0, 0, 0, 0, 0 };
	aps.p = inode;
	aps.d = dstChan;
	aps.ld = lstDst;
	stack_push(icset, p->audio_parent_stack, aps);
}
void push_audio_parentnode(struct X3D_Node* node) {
	ppComponent_Sound p = (ppComponent_Sound)gglobal()->Component_Sound.prv;
	struct X3D_SoundRep* srep = getSoundRep(node);
	push_audio_parent(srep->inode);
}
void pop_audio_parent() {
	ppComponent_Sound p = (ppComponent_Sound)gglobal()->Component_Sound.prv;
	stack_pop(icset, p->audio_parent_stack);
}
icset peek_audio_parent() {
	ppComponent_Sound p = (ppComponent_Sound)gglobal()->Component_Sound.prv;
	return stack_top(icset, p->audio_parent_stack);
}
void push_doppler_factor(float dopplerFactor) {
	ppComponent_Sound p = (ppComponent_Sound)gglobal()->Component_Sound.prv;
	stack_push(float, p->doppler_factor_stack, dopplerFactor);
}

void pop_doppler_factor() {
	ppComponent_Sound p = (ppComponent_Sound)gglobal()->Component_Sound.prv;
	stack_pop(float, p->doppler_factor_stack);
}
float peek_doppler_factor() {
	ppComponent_Sound p = (ppComponent_Sound)gglobal()->Component_Sound.prv;
	return stack_top(float, p->doppler_factor_stack);
}

void push_splitter_source_index(int source_index, int last_source_index) {
	ppComponent_Sound p = (ppComponent_Sound)gglobal()->Component_Sound.prv;
	ivec2 ssi;
	ssi.x = source_index;
	ssi.y = last_source_index;
	stack_push(ivec2, p->splitter_source_stack, ssi);
}

void pop_splitter_source_index() {
	ppComponent_Sound p = (ppComponent_Sound)gglobal()->Component_Sound.prv;
	stack_pop(ivec2, p->splitter_source_stack);
}
ivec2 peek_splitter_source_index() {
	ppComponent_Sound p = (ppComponent_Sound)gglobal()->Component_Sound.prv;
	return stack_top(ivec2, p->splitter_source_stack);
}

int newconnect(struct X3D_SoundRep* srep, icset iparent) {
	if (!srep->connections) srep->connections = newStack(ivec3);
	int duplicate = 0;
	for (int i = 0; i < vectorSize(srep->connections); i++) {
		icset conn = vector_get(icset, srep->connections, i);
		if (conn.p == iparent.p && conn.n == iparent.n && conn.d == iparent.d && conn.s == iparent.s) 
			duplicate = 1;
	}
	if (!duplicate) {
		stack_push(icset, srep->connections, iparent);
	}
	return 1 - duplicate;
}
int disconnect(struct X3D_SoundRep* srep, icset iparent) {
	if (iparent.d == iparent.ld && iparent.s == iparent.ls) return 0; //no change in destination or source
	if (!srep->connections) return 0; //no connections yet to delete
	int found_old = -1;
	for (int i = 0; i < vectorSize(srep->connections); i++) {
		icset conn = vector_get(icset, srep->connections, i);
		// LOGIC HERE IS STILL UNDER REVIEW
		// proposed merger node: ls = s, ld = d at end of each render_ChannelMerger  
		// v4 draft merger/selector nodes: ls = s at end of each render_ChannelSelector 
		// the problem is initializing on first render only, so non-zero ls = s, ld = d
		if (conn.p == iparent.p && conn.n == iparent.n) {
			if (iparent.d != iparent.ld && conn.d == iparent.ld)
				if (conn.s == iparent.s || conn.s == iparent.ls) found_old = i; //WHAT IF BOTH DESTINATION AND SOURCE CHANGE ON SAME FRAME?
			if (iparent.s != iparent.ls && conn.s == iparent.ls)
				if (conn.d == iparent.d || conn.d == iparent.ld) found_old = i; //WHAT IF BOTH DESTINATION AND SOURCE CHANGE ON SAME FRAME?
		}
	}
	if (found_old > -1) {
		vector_remove_elem(icset, srep->connections, found_old);
	}
	return found_old > -1 ? 1 : 0;
}


void render_AudioClip(struct X3D_AudioClip* node) {
	/*  audio clip is a flat sound -no 3D- and a sound node (3D) refers to it
		specs: if an audioclip can't be reached in the scenegraph, then it doesn't play
	*/

	/* is this audio wavelet initialized yet? */
	if (node->__loadstatus != LOAD_STABLE) {
		locateAudioSource((struct X3D_AudioBuffer*)node); //downcast to share resource loading code
	}
	if (node->__loadstatus != LOAD_STABLE) return;
	/* is this audio ok? if so, the sourceNumber will range
	 * between 0 and infinity; if it is BADAUDIOSOURCE, bad source.
	 * check out locateAudioSource to find out reasons */
	if (node->__sourceNumber == BADAUDIOSOURCE) return;
	struct X3D_SoundRep* srep = getSoundRep(X3D_NODE(node));
	srep->iframe = gglobal()->Mainloop.iframe;
	srep->ibuffer = node->__sourceNumber;
	srep->dopplerFactor = peek_doppler_factor();
	int icontext = peek_audio_context();
	icset iparent = peek_audio_parent();
	//printf("ac audio_context %d parent_node %d\n", peek_audio_context(), iparent.x);

	//node->gain = 1.0;
	libsound_updateNode3(icontext, iparent, X3D_NODE(node));
	if (newconnect(srep, iparent)) {
		iparent.s = 0;
		iparent.n = srep->inode;
		libsound_connect(srep->icontext, iparent);
	}


}
void render_AudioBuffer(struct X3D_AudioBuffer* node) {
	// two ways to load an audiobuffer:
	// 1) floats representing PCM data, in MFFloat buffer field
	//   -- this field may be populated at run time, for example a script using a math formula
	// 2) url loading of .wav. If url is empty, assume #1
	struct X3D_SoundRep* srep = getSoundRep(X3D_NODE(node));
	srep->iframe = gglobal()->Mainloop.iframe;
	if (node->__loadstatus == LOAD_STABLE) return;
	if (node->url.n == 0) {
		// 1) PCM float data
		if (node->__loadstatus != LOAD_STABLE) {
			//check for (scene or run-time) populated buffer MFFloat data
			if (node->buffer.n) {
				node->__sourceNumber = libsound_createBusFromPCM32(node->buffer.p, node->bufferChannels, node->buffer.n);
				srep->ibuffer = node->__sourceNumber;
				node->__loadstatus = LOAD_STABLE;
				node->_ichange++;
			}
		}
	}
	else 
	{
		//2) load .wav via URL (like audioclip)
		if (node->__loadstatus != LOAD_STABLE) {
			locateAudioSource(node);
		}
		if (node->__loadstatus != LOAD_STABLE) return;
		/* is this audio ok? if so, the sourceNumber will range
		 * between 0 and infinity; if it is BADAUDIOSOURCE, bad source.
		 * check out locateAudioSource to find out reasons */
		if (node->__sourceNumber == BADAUDIOSOURCE) return;
		srep->ibuffer = node->__sourceNumber;
		node->_ichange++;

		//we don't connect() to parent. 
		// Parent looks in its bufferNode field and if not null, mines this node directly to setImpluse
	}
}
void render_BufferAudioSource(struct X3D_BufferAudioSource* node) {
	struct X3D_SoundRep* srep = getSoundRep(X3D_NODE(node));
	srep->iframe = gglobal()->Mainloop.iframe;
	if (node->buffer && node->buffer->_nodeType == NODE_AudioBuffer) {
		struct X3D_AudioBuffer* AB = (struct X3D_AudioBuffer*)node->buffer;
		render_AudioBuffer(AB);
		if (AB->_ichange != AB->_change)
			node->_ichange++;
		AB->_ichange = AB->_change;
		srep->ibuffer = max(0,AB->__sourceNumber);
		node->__sourceNumber = AB->__sourceNumber;
	}
	if (srep->ibuffer) { //wait for AudioBuffer URL to load
		icset iparent = peek_audio_parent();
		if (node->_ichange != node->_change) {

			struct X3D_Node* anode = (struct X3D_Node*)node;
			int icontext = peek_audio_context();
			libsound_updateNode3(icontext, iparent, anode);
			//MARK_NODE_COMPILED
			node->_ichange = node->_change;
		}
		if (newconnect(srep, iparent)) {
			iparent.s = 0;
			iparent.n = srep->inode;
			libsound_connect(srep->icontext, iparent);
		}
	}

}

void render_AudioDestination(struct X3D_AudioDestination* node) {
	struct X3D_Node* anode = (struct X3D_Node*)node;
	icset have_parent = peek_audio_parent();
	create_and_push_audio_context(anode);
	if (!have_parent.p)
		push_audio_parent(1); //should be the audio context device node

	//push_audio_parentnode(anode);
	//libsound_updateNode0(peek_audio_context(), have_parent, anode);

	//printf("ad audio_context %d parent_node %d this node %d\n", peek_audio_context(), have_parent, peek_audio_parent());

	struct X3D_SoundRep* srep = getSoundRep(X3D_NODE(node));
	srep->iframe = gglobal()->Mainloop.iframe;
	if (node->children.n) {
		for (int i = 0; i < node->children.n; i++)
			//libsound_updateNode0(icontext,anode,(struct X3D_Node*) node->children.p[i]);
			render_node(X3D_NODE(node->children.p[i]));
	}
	//pop_audio_parent(); // sound audio destination node
	if (!have_parent.p)
		pop_audio_parent(); //audio context device node 1
	pop_audio_context();
}

void update_Sound_pose(struct X3D_Sound* node){
	// update the pose of this sound source node relative to avatar 
	// avatar listener is always at location 0,0,0 looking 0 0 -1, up 0 1 0
	// and sound source node does all the work transforming relative to moving avatar via modelMatrix on each frame
	// transformed location, direction are stored in node.__lastlocation and node.__lastdirection
	int i;
	GLDOUBLE modelMatrix[16];
	GLDOUBLE SourcePosd[3] = { 0.0f, 0.0f, 0.0f };
	float SourcePos[3];

	//transform source local coordinate 0,0,0 location into avatar/listener space
	FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, modelMatrix);
	transformAFFINEd(SourcePosd, SourcePosd, modelMatrix);
	for (i = 0; i < 3; i++) SourcePos[i] = (float)SourcePosd[i];

	if (node->__sourceNumber < 0) {
		node->__lasttime = TickTime();
		veccopy3f(node->__lastlocation.c, SourcePos); //transformed location

		node->__sourceNumber = 0;
	}
	if (node->__sourceNumber > -1) {
		int istate;
		float SourceVel[3] = { 0.0f, 0.0f, 0.0f };
		float travelled[3];
		double traveltime;

		//update velocity for doppler effect
		vecdif3f(travelled, SourcePos, node->__lastlocation.c);
		traveltime = TickTime() - node->__lasttime;
		if (traveltime > 0.0)
			vecscale3f(node->__velocity.c, travelled, 1.0f / (float)traveltime);


		node->__lasttime = TickTime();
		veccopy3f(node->__lastlocation.c, SourcePos);

		//directional sound
		//if (node->spatialize) 
		{
			double dird[3];
			double zero[3];
			float dirf[3];
			//transform source direction into avatar/listener space
			//for (i = 0; i < 3; i++) dird[i] = node->direction.c[i];
			float2double(dird, node->direction.c, 3);
			vecsetd(zero, 0.0, 0.0, 0.0);

			//zero[0] = zero[1] = zero[2] = 0.0;
			transformAFFINEd(dird, dird, modelMatrix);
			transformAFFINEd(zero, zero, modelMatrix);

			vecdifd(dird, dird, zero);
			vecnormald(dird, dird);
			//for (i = 0; i < 3; i++) dirf[i] = (float)dird[i];
			double2float(node->__lastdirection.c, dird, 3);
			//veccopy3f(node->__lastdirection.c, dirf); //transformed direction
			/*
			if (1)
				alSourcefv(node->__sourceNumber, AL_DIRECTION, dirf);
			else
				alSource3f(node->__sourceNumber, AL_DIRECTION, dirf[0], dirf[1], dirf[2]);
			alSourcef(node->__sourceNumber, AL_CONE_OUTER_GAIN, .5f);
			alSourcef(node->__sourceNumber, AL_CONE_INNER_ANGLE, 90.0f);
			alSourcef(node->__sourceNumber, AL_CONE_OUTER_ANGLE, 135.0f);
			*/
		}
	}
}

void render_Sound(struct X3D_Sound* node) {
	struct X3D_Node* anode = (struct X3D_Node*)node;
	icset have_parent = peek_audio_parent();
	create_and_push_audio_context(anode); //Sound is a destination/output audioNode
	update_Sound_pose(node);
	if (!have_parent.p)
		push_audio_parent(1); //should be the audio context device node
	int icontext = peek_audio_context();
	libsound_updateNode3(icontext,peek_audio_parent(), anode);
	push_audio_parentnode(anode);
	struct X3D_SoundRep* srep = getSoundRep(X3D_NODE(node));
	srep->iframe = gglobal()->Mainloop.iframe;

	if (node->source)
		//libsound_updateNode0(icontext,anode,node->source);
		render_node(node->source);
	if (node->children.n) {
		for (int i = 0; i < node->children.n; i++)
			//libsound_updateNode0(icontext,anode,(struct X3D_Node*) node->children.p[i]);
			render_node(X3D_NODE(node->children.p[i]));
	}
	pop_audio_parent(); // sound panner node
	if(!have_parent.p)
		pop_audio_parent(); //audio context device node 1
	pop_audio_context();
}

void render_SpatialSound(struct X3D_SpatialSound* node) {
	struct X3D_Node* anode = (struct X3D_Node*)node;
	icset have_parent = peek_audio_parent();
	create_and_push_audio_context(anode); //Sound is a destination/output audioNode
	update_Sound_pose((struct X3D_Sound*)node); //down-cast to Sound, and assume the fields accessed are in same order as Sound
	if (!have_parent.p)
		push_audio_parent(1); //should be the audio context device node
	int icontext = peek_audio_context();
	libsound_updateNode3(icontext, peek_audio_parent(), anode);
	push_doppler_factor(node->__dopplerFactor);
	push_audio_parentnode(anode);
	//printf("ss audio_context %d parent_node %d\n", peek_audio_context(), have_parent);

	struct X3D_SoundRep* srep = getSoundRep(X3D_NODE(node));
	srep->iframe = gglobal()->Mainloop.iframe;
	if (node->children.n) {
		for (int i = 0; i < node->children.n; i++)
			//libsound_updateNode0(icontext,anode,(struct X3D_Node*) node->children.p[i]);
			render_node(X3D_NODE(node->children.p[i]));
	}
	pop_audio_parent(); // sound panner node
	pop_doppler_factor(node->__dopplerFactor);
	if(!have_parent.p)
		pop_audio_parent(); //audio context device node 1
	pop_audio_context();

}

#endif //HAVE_LIBSOUND


#ifdef HAVE_LIBSOUND

void render_PeriodicWave(struct X3D_PeriodicWave* node);

void render_OscillatorSource(struct X3D_OscillatorSource *node){
	struct X3D_SoundRep* srep = getSoundRep(X3D_NODE(node));
	srep->iframe = gglobal()->Mainloop.iframe;
	//SenseInterp.c > OscillatorSourceTick: 
	//  no exemplar in Ticks of setting _changed, just MARK_EVENT which doesnt seem to (it updates route event)
	//  could/should it set _ichange if any MARK_EVENTs? DONE, WORKS
	icset iparent = peek_audio_parent();
	if (node->_ichange != node->_change) {
		//if (node->_ichange == 0) return;

		struct X3D_Node* anode = (struct X3D_Node*)node;
		int icontext = peek_audio_context();
		libsound_updateNode3(icontext, iparent, anode);
		//MARK_NODE_COMPILED
		node->_ichange = node->_change;
	}
	iparent.s = 0;
	iparent.n = srep->inode;
	if (newconnect(srep, iparent)) {
		libsound_connect(srep->icontext, iparent);
	}

	if (node->periodicWave) {
		push_audio_parent(srep->inode);
		render_PeriodicWave((struct X3D_PeriodicWave*)node->periodicWave); //like rendering a child, check if anything changed.
		pop_audio_parent();
	}


}
void render_PeriodicWave(struct X3D_PeriodicWave* node) {
	struct X3D_SoundRep* srep = getSoundRep(X3D_NODE(node));
	srep->iframe = gglobal()->Mainloop.iframe;
	if (node->_ichange != node->_change) {
		//if (node->_ichange == 0) return;

		struct X3D_Node* anode = (struct X3D_Node*)node;
		int icontext = peek_audio_context();
		icset iparent = peek_audio_parent();
		libsound_updateNode3(icontext, iparent, anode);
		//MARK_NODE_COMPILED
	}


}

void render_ChannelMerger(struct X3D_ChannelMerger* node) {
	struct X3D_SoundRep* srep = getSoundRep(X3D_NODE(node));
	srep->iframe = gglobal()->Mainloop.iframe;
	struct X3D_Node* anode = (struct X3D_Node*)node;
	icset iparent = peek_audio_parent();
	if (node->_ichange != node->_change) {
		int icontext = peek_audio_context();
		libsound_updateNode3(icontext, iparent, anode);
		//MARK_NODE_COMPILED
		node->_ichange = node->_change;
	}
	iparent.s = 0;
	iparent.n = srep->inode;
	if (newconnect(srep, iparent)) {
		libsound_connect(srep->icontext, iparent);
	}

	//push_audio_parentnode(anode);
	//printf("ss audio_context %d parent_node %d\n", peek_audio_context(), have_parent);
	//srep->imerger = srep->inode; //libsound audio nodes check if their parent is a merger..
	int inode = srep->inode;
	int noisy = 0;
	if (node->children.n) {
		//for (int i = 0; i < node->children.n; i++) {
		if (noisy) printf("render_merger nchan %d\n", node->indexDestination.n);
		if (noisy) libsound_print_connections();
		if (node->selectors.n) {
			// proposal 3, selector is a 3-tuple (sourceChannel, destinationChannel, stream)
			// and merger.selectors mfnde holds the selectors
			// merger.children is a list of audio streams
			for (int i = 0; i < node->selectors.n; i++) {
				struct X3D_ChannelSelector* selector = (struct X3D_ChannelSelector*)node->selectors.p[i];
				if (!selector->_initialized) {
					selector->_lastDestinationChannel = selector->destinationChannel;
					selector->_lastSourceChannel = selector->sourceChannel;
					selector->_lastStream = selector->stream;
					selector->_initialized = TRUE;
				}

				int destination_index = selector->destinationChannel;
				int last_destination_index = selector->_lastDestinationChannel;
				int source_index = selector->sourceChannel;
				int last_source_index = selector->_lastSourceChannel;
				if (noisy) printf("%d indxDst %d indxSrc %d\n", i, destination_index, source_index);
				if (source_index > -1) push_splitter_source_index(source_index, last_source_index); //-1 means there's no splitter in the audio stream
				if (destination_index != last_destination_index)
					printf("changing destination\n");
				push_audio_parent3(inode, destination_index, last_destination_index); // 0 is over-ridden by source_index if child is a Splitter
				//libsound_updateNode0(icontext,anode,(struct X3D_Node*) node->children.p[i]);
				//srep->idestination = i; // .. and if so connect to their parent using the recommended destination channel
				//int ichild = min(node->children.n - 1, source_index); //RE-USE LAST CHILD IF FEWER THAN INDXDST.N
				int ichild = selector->stream; //if there aren't enough streams, re-use the last one
				render_node(X3D_NODE(node->children.p[ichild]));
				pop_audio_parent();
				if (source_index > -1) pop_splitter_source_index();
				if (noisy) libsound_print_connections();
				if (noisy) printf("\n");
				selector->_lastDestinationChannel = selector->destinationChannel;
				selector->_lastSourceChannel = selector->sourceChannel;
				selector->_lastStream = selector->stream;
			}
		} else if (node->indexDestination.n && node->indexSource.n && node->indexStream.n) {
			//Doug's proposed way with (indexStream,indexSource,indexDestination) tuples, Merger.children[i] == audio stream [i]
			for (int i = 0; i < node->indexDestination.n; i++) {
				int destination_index = node->indexDestination.p[i];
				int last_destination_index = srep->last_indexDestination[i];
				int source_index = node->indexSource.p[i];
				int last_source_index = srep->last_indexSource[i];
				if (noisy) printf("%d indxDst %d indxSrc %d\n", i, destination_index, source_index);
				if (source_index > -1) push_splitter_source_index(source_index, last_source_index); //-1 means there's no splitter in the audio stream
				push_audio_parent3(inode, destination_index, last_destination_index); // 0 is over-ridden by source_index if child is a Splitter
				//libsound_updateNode0(icontext,anode,(struct X3D_Node*) node->children.p[i]);
				//srep->idestination = i; // .. and if so connect to their parent using the recommended destination channel
				//int ichild = min(node->children.n - 1, source_index); //RE-USE LAST CHILD IF FEWER THAN INDXDST.N
				int ichild = node->indexStream.p[min(i, node->indexStream.n - 1)]; //if there aren't enough indexStream, re-use the last one
				render_node(X3D_NODE(node->children.p[ichild]));
				pop_audio_parent();
				if (source_index > -1) pop_splitter_source_index();
				if (noisy) libsound_print_connections();
				if (noisy) printf("\n");
			}
			memcpy(srep->last_indexSource, node->indexSource.p, node->indexSource.n * sizeof(int));
			memcpy(srep->last_indexDestination, node->indexDestination.p, node->indexDestination.n * sizeof(int));
			srep->last_count = node->indexDestination.n;
		} else {
			//thunk to v4 spec way, with ChannelSelector, and children[i] == mergerChannel[i]
			for (int i = 0; i < node->children.n; i++) {
				int destination_index = i;
				if (noisy) printf("%d indxDst %d \n", i, destination_index);
				push_audio_parent3(inode, destination_index, 0); // 0 is over-ridden by source_index if child is a Splitter
				//libsound_updateNode0(icontext,anode,(struct X3D_Node*) node->children.p[i]);
				//srep->idestination = i; // .. and if so connect to their parent using the recommended destination channel
				int ichild = i; 
				render_node(X3D_NODE(node->children.p[ichild]));
				pop_audio_parent();
				if (noisy) libsound_print_connections();
				if (noisy) printf("\n");
			}
		}
	}
}

void render_ChannelSelector(struct X3D_ChannelSelector* node) {
	
	// doesn't push or pop parent, so children will connect to grandparent
	// we don't come through here with proposal3
	push_splitter_source_index(node->channelSelection, node->lastChannelSelection);
	if (node->children.n) {
		for (int i = 0; i < node->children.n; i++)
			//libsound_updateNode0(icontext,anode,(struct X3D_Node*) node->children.p[i]);
			render_node(X3D_NODE(node->children.p[i]));
	}
	pop_splitter_source_index();
	node->lastChannelSelection = node->channelSelection;
}
void render_ChannelSplitter(struct X3D_ChannelSplitter* node) {
	struct X3D_SoundRep* srep = getSoundRep(X3D_NODE(node));
	srep->iframe = gglobal()->Mainloop.iframe;
	struct X3D_Node* anode = (struct X3D_Node*)node;
	ivec2 splitter_source_index = peek_splitter_source_index();
	icset iparent = peek_audio_parent(); //(inode, destinationIndex, lastDestinationIndex)
	iparent.s = splitter_source_index.x;
	iparent.ls = splitter_source_index.y;
	if (node->_ichange != node->_change) {
		int icontext = peek_audio_context();
		libsound_updateNode3(icontext, iparent, anode);
		//MARK_NODE_COMPILED
		node->_ichange = node->_change;
	}
	iparent.n = srep->inode;
	if (newconnect(srep, iparent)) {
		libsound_connect(srep->icontext,iparent);
		libsound_print_connections();
	}
	if (disconnect(srep, iparent)) {
		libsound_disconnect(srep->icontext, iparent);
		libsound_print_connections();
	}

	push_audio_parentnode(anode);
	//printf("ss audio_context %d parent_node %d\n", peek_audio_context(), have_parent);

	if (node->children.n) {
		for (int i = 0; i < node->children.n; i++)
			//libsound_updateNode0(icontext,anode,(struct X3D_Node*) node->children.p[i]);
			render_node(X3D_NODE(node->children.p[i]));
	}
	//printf("number of outputs nodes = %d\n", node->outputs.n);
	pop_audio_parent(); // sound panner node

}
void render_Gain(struct X3D_Gain* node) {
	struct X3D_SoundRep* srep = getSoundRep(X3D_NODE(node));
	srep->iframe = gglobal()->Mainloop.iframe;
	struct X3D_Node* anode = (struct X3D_Node*)node;
	icset iparent = peek_audio_parent();

	if (node->_ichange != node->_change) {
		//if (node->_ichange == 0) return;
		int icontext = peek_audio_context();
		libsound_updateNode3(icontext, iparent, anode);
		//MARK_NODE_COMPILED
		node->_ichange = node->_change;
	}
	iparent.n = srep->inode;
	iparent.s = 0;
	if (newconnect(srep, iparent)) {
		libsound_connect(srep->icontext, iparent);
	}

	push_audio_parentnode(anode);
	//printf("ss audio_context %d parent_node %d\n", peek_audio_context(), have_parent);
	if (node->children.n) {
		for (int i = 0; i < node->children.n; i++)
			//libsound_updateNode0(icontext,anode,(struct X3D_Node*) node->children.p[i]);
			render_node(X3D_NODE(node->children.p[i]));
	}
	pop_audio_parent(); // sound panner node

}

void render_Delay(struct X3D_Delay* node) {
	struct X3D_SoundRep* srep = getSoundRep(X3D_NODE(node));
	srep->iframe = gglobal()->Mainloop.iframe;
	struct X3D_Node* anode = (struct X3D_Node*)node;
	icset iparent = peek_audio_parent();

	if (node->_ichange != node->_change) {
		//if (node->_ichange == 0) return;


		int icontext = peek_audio_context();
		libsound_updateNode3(icontext, iparent, anode);
		//MARK_NODE_COMPILED
		node->_ichange = node->_change;
	}
	iparent.n = srep->inode;
	iparent.s = 0;
	if (newconnect(srep, iparent)) {
		libsound_connect(srep->icontext, iparent);
	}


	push_audio_parentnode(anode);
	//printf("ss audio_context %d parent_node %d\n", peek_audio_context(), have_parent);
	if (node->children.n) {
		for (int i = 0; i < node->children.n; i++)
			//libsound_updateNode0(icontext,anode,(struct X3D_Node*) node->children.p[i]);
			render_node(X3D_NODE(node->children.p[i]));
	}
	pop_audio_parent(); 

}

void render_Analyser(struct X3D_Analyser* node) {
	struct X3D_SoundRep* srep = getSoundRep(X3D_NODE(node));
	srep->iframe = gglobal()->Mainloop.iframe;
	struct X3D_Node* anode = (struct X3D_Node*)node;
	icset iparent = peek_audio_parent();
	if (node->_ichange != node->_change) {
		//if (node->_ichange == 0) return;


		int icontext = peek_audio_context();
		libsound_updateNode3(icontext, iparent, anode);
		//MARK_NODE_COMPILED
		node->_ichange = node->_change;
		node->_ichange++; //come in here every loop
		MARK_EVENT(X3D_NODE(node), offsetof(struct X3D_Analyser, floatFrequencyData));
		//MARK_EVENT(X3D_NODE(node), offsetof(struct X3D_Analyser, byteFrequencyData));
		MARK_EVENT(X3D_NODE(node), offsetof(struct X3D_Analyser, floatTimeDomainData));
		//MARK_EVENT(X3D_NODE(node), offsetof(struct X3D_Analyser, byteTimeDomainData));


	}
	iparent.n = srep->inode;
	iparent.s = 0;
	if (newconnect(srep, iparent)) {
		libsound_connect(srep->icontext, iparent);
	}

	push_audio_parentnode(anode);
	//printf("ss audio_context %d parent_node %d\n", peek_audio_context(), have_parent);
	if (node->children.n) {
		for (int i = 0; i < node->children.n; i++)
			//libsound_updateNode0(icontext,anode,(struct X3D_Node*) node->children.p[i]);
			render_node(X3D_NODE(node->children.p[i]));
	}
	pop_audio_parent();

}
void render_BiquadFilter(struct X3D_BiquadFilter* node) {
	struct X3D_SoundRep* srep = getSoundRep(X3D_NODE(node));
	srep->iframe = gglobal()->Mainloop.iframe;
	struct X3D_Node* anode = (struct X3D_Node*)node;
	icset iparent = peek_audio_parent();
	if (node->_ichange != node->_change) {
		//if (node->_ichange == 0) return;


		int icontext = peek_audio_context();
		libsound_updateNode3(icontext, iparent, anode);
		//MARK_NODE_COMPILED
		node->_ichange = node->_change;
	}
	iparent.n = srep->inode;
	iparent.s = 0;
	if (newconnect(srep, iparent)) {
		libsound_connect(srep->icontext, iparent);
	}

	push_audio_parentnode(anode);
	//printf("ss audio_context %d parent_node %d\n", peek_audio_context(), have_parent);
	if (node->children.n) {
		for (int i = 0; i < node->children.n; i++)
			//libsound_updateNode0(icontext,anode,(struct X3D_Node*) node->children.p[i]);
			render_node(X3D_NODE(node->children.p[i]));
	}
	pop_audio_parent();

}

void render_DynamicsCompressor(struct X3D_DynamicsCompressor* node) {
	struct X3D_SoundRep* srep = getSoundRep(X3D_NODE(node));
	srep->iframe = gglobal()->Mainloop.iframe;
	struct X3D_Node* anode = (struct X3D_Node*)node;
	icset iparent = peek_audio_parent();

	if (node->_ichange != node->_change) {
		//if (node->_ichange == 0) return;


		int icontext = peek_audio_context();
		libsound_updateNode3(icontext, iparent, anode);
		//MARK_NODE_COMPILED
		node->_ichange = node->_change;
		MARK_EVENT(X3D_NODE(node), offsetof(struct X3D_DynamicsCompressor, reduction));

	}
	iparent.n = srep->inode;
	iparent.s = 0;
	if (newconnect(srep, iparent)) {
		libsound_connect(srep->icontext, iparent);
	}

	push_audio_parentnode(anode);
	//printf("ss audio_context %d parent_node %d\n", peek_audio_context(), have_parent);
	if (node->children.n) {
		for (int i = 0; i < node->children.n; i++)
			//libsound_updateNode0(icontext,anode,(struct X3D_Node*) node->children.p[i]);
			render_node(X3D_NODE(node->children.p[i]));
	}
	pop_audio_parent();
}

void render_WaveShaper(struct X3D_WaveShaper* node) {
	struct X3D_SoundRep* srep = getSoundRep(X3D_NODE(node));
	srep->iframe = gglobal()->Mainloop.iframe;
	struct X3D_Node* anode = (struct X3D_Node*)node;
	icset iparent = peek_audio_parent();

	if (node->_ichange != node->_change) {
		//if (node->_ichange == 0) return;

		int icontext = peek_audio_context();
		libsound_updateNode3(icontext, iparent, anode);
		//MARK_NODE_COMPILED
		node->_ichange = node->_change;

	}
	iparent.n = srep->inode;
	iparent.s = 0;
	if (newconnect(srep, iparent)) {
		libsound_connect(srep->icontext, iparent);
	}

	push_audio_parentnode(anode);
	//printf("ss audio_context %d parent_node %d\n", peek_audio_context(), have_parent);
	if (node->children.n) {
		for (int i = 0; i < node->children.n; i++)
			//libsound_updateNode0(icontext,anode,(struct X3D_Node*) node->children.p[i]);
			render_node(X3D_NODE(node->children.p[i]));
	}
	pop_audio_parent();

}



void render_Convolver(struct X3D_Convolver* node) {
	struct X3D_SoundRep* srep = getSoundRep(X3D_NODE(node));
	srep->iframe = gglobal()->Mainloop.iframe;
	struct X3D_Node* anode = (struct X3D_Node*)node;
	icset iparent = peek_audio_parent();
	if (node->buffer && node->buffer->_nodeType == NODE_AudioBuffer) {
		render_AudioBuffer((struct X3D_AudioBuffer*)node->buffer);
		if (node->buffer->_ichange != node->buffer->_change)
			node->_ichange++;
		node->buffer->_ichange = node->buffer->_change;
	}
	if (node->_ichange != node->_change) {
		//if (node->_ichange == 0) return;

		int icontext = peek_audio_context();
		libsound_updateNode3(icontext, iparent, anode);
		//MARK_NODE_COMPILED
		node->_ichange = node->_change;

	}
	iparent.n = srep->inode;
	iparent.s = 0;
	if (newconnect(srep, iparent)) {
		libsound_connect(srep->icontext, iparent);
	}
	push_audio_parentnode(anode);
	//printf("ss audio_context %d parent_node %d\n", peek_audio_context(), have_parent);
	if (node->children.n) {
		for (int i = 0; i < node->children.n; i++)
			//libsound_updateNode0(icontext,anode,(struct X3D_Node*) node->children.p[i]);
			render_node(X3D_NODE(node->children.p[i]));
	}
	pop_audio_parent();

}

void render_MicrophoneSource(struct X3D_MicrophoneSource* node) {
	// labsound has a few Example.hpp examples on using 'devices' as microphones
	struct X3D_SoundRep* srep = getSoundRep(X3D_NODE(node));
	srep->iframe = gglobal()->Mainloop.iframe;
	struct X3D_Node* anode = (struct X3D_Node*)node;
	icset iparent = peek_audio_parent();

	if (node->_ichange != node->_change) {
		//if (node->_ichange == 0) return;

		int icontext = peek_audio_context();
		libsound_updateNode3(icontext, iparent, anode);
		//MARK_NODE_COMPILED
		node->_ichange = node->_change;

	}
	iparent.n = srep->inode;
	iparent.s = 0;
	if (newconnect(srep, iparent)) {
		libsound_connect(srep->icontext, iparent);
	}

}



void render_StreamAudioSource(struct X3D_StreamAudioSource *node){
	COMPILE_IF_REQUIRED
	//don't know if / how to do this one in native code
	// web audio API has a MediaStreamSource that pulls from html elements
	// Labsound doesn't have MediaStreamSource
}

void render_StreamAudioDestination(struct X3D_StreamAudioDestination *node){
	COMPILE_IF_REQUIRED
}





void render_ListenerPointSource(struct X3D_ListenerPointSource *node){
	COMPILE_IF_REQUIRED
}

#else //HAVE_LIBSOUND
//HAVE_OPENAL
void render_OscillatorSource(struct X3D_OscillatorSource* node) {}
void render_BufferAudioSource(struct X3D_BufferAudioSource* node) {}
void render_StreamAudioSource(struct X3D_StreamAudioSource* node) {}
void render_WaveShaper(struct X3D_WaveShaper* node) {}
void render_PeriodicWave(struct X3D_PeriodicWave* node) {}
void render_AudioDestination(struct X3D_AudioDestination* node) {}
void render_StreamAudioDestination(struct X3D_StreamAudioDestination* node) {}
void render_Analyser(struct X3D_Analyser* node) {}
void render_ChannelMerger(struct X3D_ChannelMerger* node) {}
void render_ChannelSelector(struct X3D_ChannelSelector* node) {}
void render_ChannelSplitter(struct X3D_ChannelSplitter* node) {}
void render_BiquadFilter(struct X3D_BiquadFilter* node) {}
void render_Convolver(struct X3D_Convolver* node) {}
void render_Delay(struct X3D_Delay* node) {}
void render_DynamicsCompressor(struct X3D_DynamicsCompressor* node) {}
void render_Gain(struct X3D_Gain* node) {}
void render_ListenerPointSource(struct X3D_ListenerPointSource* node) {}
void render_MicrophoneSource(struct X3D_MicrophoneSource* node) {}
void render_SpatialSound(struct X3D_SpatialSound* node) {}
#endif //HAVE_LIBSOUND