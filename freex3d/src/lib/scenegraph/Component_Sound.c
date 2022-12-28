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

void locateAudioSource (struct X3D_AudioClip *node) {
	resource_item_t *res;
	//resource_item_t *parentPath;
	//ppComponent_Sound p = (ppComponent_Sound)gglobal()->Component_Sound.prv;

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
		printf("1");
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
		printf("2");
		break;

		case LOAD_FETCHING_RESOURCE:
		res = node->__loadResource;
		/* printf ("load_Inline, we have type  %s  status %s\n",
			resourceTypeToString(res->type), resourceStatusToString(res->status)); */
		if(res->complete){
			if (res->status == ress_loaded) {
				res->actions = resa_process;
				res->complete = FALSE;
				resitem_enqueue(ml_new(res));
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
		printf("3");
		break;

		case LOAD_STABLE:
		printf("4");
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
void compile_AudioClip(struct X3D_AudioClip* node) {

}
void render_AudioClip (struct X3D_AudioClip *node) {
/*  audio clip is a flat sound -no 3D- and a sound node (3D) refers to it
	specs: if an audioclip can't be reached in the scenegraph, then it doesn't play
*/

	/* is this audio wavelet initialized yet? */
	if (node->__loadstatus != LOAD_STABLE) {
		locateAudioSource (node);
	}
	if(node->__loadstatus != LOAD_STABLE) return;
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

#ifdef HAVE_OPENAL
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
#elif HAVE_LIBSOUND
	if (acp) {
		if (haveSoundEngine()) {
			if (acp->__sourceNumber < 0) {
				render_AudioClip(acp);
			}
			if (acp->__sourceNumber > -1) {
				//have a buffer loaded
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
					veccopy3f(node->__lastlocation.c, SourcePos);

					node->__sourceNumber = 0;
				}
				if (node->__sourceNumber > -1) {
					int istate;
					float SourceVel[3] = { 0.0f, 0.0f, 0.0f };
					float travelled[3];
					double traveltime;

					//update velocity for doppler effect
					vecdif3f(travelled, node->__lastlocation.c, SourcePos);
					traveltime = TickTime() - node->__lasttime;
					if (traveltime > 0.0)
						vecscale3f(SourceVel, travelled, 1.0f / (float)traveltime);

					node->__lasttime = TickTime();
					veccopy3f(node->__lastlocation.c, SourcePos);

					//directional sound 
					if (node->spatialize) {
						double dird[3];
						float dirf[3];
						//transform source direction into avatar/listener space
						for (i = 0; i < 3; i++) dird[i] = node->direction.c[i];
						transformAFFINEd(dird, dird, modelMatrix);
						for (i = 0; i < 3; i++) dirf[i] = (float)dird[i];
					}

					// for routed values going to audioclip, update values
					// update to audioclip start,stop,pause,resume is done in do_AudioTick()
					if (acp->isActive) {
							//printf(".play.");
					}
					else {
						//stop
					}
				}
			}
		}
	}
#endif

}



// v4 visibility functions, push & pop (to be) called from all X3DGroupingNode child_ functions
void push_audio_context(int audio_context) {
	ppComponent_Sound p = (ppComponent_Sound)gglobal()->Component_Sound.prv;
	stack_push(int, p->audio_context_stack, audio_context);
}
void super_push_audio_context(struct X3D_AudioNode* node) {
	struct X3D_SoundRep* srep = getSoundRep(node);
	if (!srep->icontext) {
		int jcontext = peek_audio_context();
		if (!jcontext) {
#ifdef HAVE_LIBSOUND
			jcontext = libsound_createContext();
#endif //HAVE_LIBSOUND
		}
		srep->icontext = jcontext;
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


void prep_Sound(struct X3D_Sound* node) {
	if (!node->_context) {
		node->_context = peek_audio_context();
		if (!node->_context) {
#ifdef HAVE_LIBSOUND
			node->_context = libsound_createContext();
#endif //HAVE_LIBSOUND
		}
	}
	push_audio_context(node->_context);
}
void child_Sound(struct X3D_Sound* node) {
	normalChildren(node->children);

}
void fin_Sound(struct X3D_Sound* node) {
	pop_audio_context();
}




int	parse_audioclip(struct X3D_AudioClip *node,char *bbuffer, int len){
#ifdef HAVE_OPENAL
	ALint buffer = AL_NONE;
#ifdef HAVE_ALUT
	buffer = alutCreateBufferFromFileImage (bbuffer, len);
//#elif HAVE_SDL
#endif
	if (buffer == AL_NONE)
		buffer = BADAUDIOSOURCE;
#elif HAVE_LIBSOUND
	int buffer = libsound_createBusFromBuffer(bbuffer, len);
#else
	int buffer = BADAUDIOSOURCE;
#endif
	//printf("parse_audioclip buffer=%d\n",buffer);
	return buffer;
}

double compute_duration(int ibuffer){

	double retval = 1.0;
#ifdef HAVE_OPENAL
	int ibytes;
	int ibits;
	int ichannels;
	int ifreq;
	double framesizebytes, bytespersecond;
	alGetBufferi(ibuffer,AL_FREQUENCY,&ifreq);
	alGetBufferi(ibuffer,AL_BITS,&ibits);
	alGetBufferi(ibuffer,AL_CHANNELS,&ichannels);
	alGetBufferi(ibuffer,AL_SIZE,&ibytes);
	framesizebytes = (double)(ibits * ichannels)/8.0;
	bytespersecond = framesizebytes * (double)ifreq;
	if(bytespersecond > 0.0)
		retval = (double)(ibytes) / bytespersecond;
	else
		retval = 1.0;
#endif
	return retval;
}
bool  process_res_audio(resource_item_t *res){
	//s_list_t *l;
	openned_file_t *of;
	//struct Shader_Script* ss;
	char *buffer;
	int len;
	struct X3D_AudioClip *node;

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

	node = (struct X3D_AudioClip *) res->whereToPlaceData;
	//node->__FILEBLOB = buffer;
	node->__sourceNumber = parse_audioclip(node,buffer,len); //__sourceNumber will be openAL buffer number
	if(node->__sourceNumber > -1) {
		node->duration_changed = compute_duration(node->__sourceNumber);
		MARK_EVENT (X3D_NODE(node), offsetof(struct X3D_AudioClip, duration_changed));
		return TRUE;
	} 
	return FALSE;
}


/* returns the audio duration, unscaled by pitch */
double return_Duration (struct X3D_AudioClip *node) {
	double retval;
	int indx;
	indx = node->__sourceNumber;
	if (indx < 0)  retval = 1.0;
	else if (indx > 50) retval = 1.0;
	else 
	{
#ifdef HAVE_OPENAL
		retval = node->duration_changed;
#endif
	}
	return retval;
}


#ifdef HAVE_LIBSOUND


void render_OscillatorSource(struct X3D_OscillatorSource *node){
	COMPILE_IF_REQUIRED
}

//void compile_BufferAudioSource(struct X3D_BufferAudioSource *node){
//	if(!node->_self && peek_audio_context()){
//		node->_self = libsound_createNode(peek_audio_context(),AN_AudioBuffer); //,node type, parameter list
//		node->_context = peek_audio_context();
//	}else if(peek_audio_context() == node->_context){
//		//which field changed
//		//libsound_updatenode(,,parameter_list);
//	}
//
//	MARK_NODE_COMPILED
//}
void render_BufferAudioSource(struct X3D_BufferAudioSource *node){
	COMPILE_IF_REQUIRED
}
void render_StreamAudioSource(struct X3D_StreamAudioSource *node){
	COMPILE_IF_REQUIRED
}

void render_WaveShaper(struct X3D_WaveShaper *node){
	COMPILE_IF_REQUIRED
}
void render_PeriodicWave(struct X3D_PeriodicWave *node){
	COMPILE_IF_REQUIRED
}

void render_AudioDestination(struct X3D_AudioDestination *node){
	COMPILE_IF_REQUIRED
}

void render_StreamAudioDestination(struct X3D_StreamAudioDestination *node){
	COMPILE_IF_REQUIRED
}

void render_Analyser(struct X3D_Analyser *node){
	COMPILE_IF_REQUIRED
}

void render_ChannelMerger(struct X3D_ChannelMerger *node){
	COMPILE_IF_REQUIRED
}

void render_ChannelSelector(struct X3D_ChannelSelector* node) {
	COMPILE_IF_REQUIRED
}
void render_ChannelSplitter(struct X3D_ChannelSplitter* node) {
	COMPILE_IF_REQUIRED
}

void render_BiquadFilter(struct X3D_BiquadFilter* node) {
	COMPILE_IF_REQUIRED
}

void render_Convolver(struct X3D_Convolver* node) {
	COMPILE_IF_REQUIRED
}

void render_Delay(struct X3D_Delay* node) {
	COMPILE_IF_REQUIRED
}

void render_DynamicsCompressor(struct X3D_DynamicsCompressor* node) {
	COMPILE_IF_REQUIRED
}

void render_Gain(struct X3D_Gain* node) {
	COMPILE_IF_REQUIRED
}

void render_ListenerPointSource(struct X3D_ListenerPointSource *node){
	COMPILE_IF_REQUIRED
}

void render_MicrophoneSource(struct X3D_MicrophoneSource* node) {
	COMPILE_IF_REQUIRED
}

void render_SpatialSound(struct X3D_SpatialSound *node){
	COMPILE_IF_REQUIRED
}

#endif //HAVE_LIBSOUND