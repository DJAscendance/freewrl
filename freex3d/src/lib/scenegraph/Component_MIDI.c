/*


X3D MIDI Experimental Component 2023
- relies on libremidi https://github.com/jcelerier/libremidi 
-- which uses C++ 17, so may need a wrapper lib_midi
- which is a close derivitive of https://www.music.mcgill.ca/~gary/rtmidi/index.html rtmidi
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
#include "../main/headers.h"

#include "LinearAlgebra.h"
#include "../../libmidi/libmidi.h"

typedef struct pComponent_MIDI {
	void *nothing;
}*ppComponent_MIDI;
void* Component_MIDI_constructor() {
	void* v = MALLOCV(sizeof(struct pComponent_MIDI));
	memset(v, 0, sizeof(struct pComponent_MIDI));
	return v;
}
void Component_MIDI_init(struct tComponent_MIDI* t) {
	//public
	//t->sound_from_audioclip = 0;

	//private
	t->prv = Component_MIDI_constructor();
	{
		ppComponent_MIDI p = (ppComponent_MIDI)t->prv;
		//p->audio_context_stack = newStack(int);
	}
}
void Component_MIDI_clear(struct tComponent_MIDI* t) {
	ppComponent_MIDI p = (ppComponent_MIDI)t->prv;
	//deleteVector(struct X3D_Node*, p->audio_context_stack);
}
//ppComponent_MIDI p = (ppComponent_MIDI)gglobal()->Component_MIDI.prv;

enum {
	LOADER_INITIAL_STATE = 0,
	LOADER_REQUEST_RESOURCE,
	LOADER_FETCHING_RESOURCE,
	LOADER_PROCESSING,
	LOADER_LOADED,
	LOADER_COMPILED,
	LOADER_STABLE,
};
void compile_MIDIFileSource(struct X3D_MIDIFileSource* node) {
	resource_item_t* res;
	int retval = FALSE;

	switch (node->__loadstatus) {
	case LOADER_INITIAL_STATE: /* nothing happened yet */

		if (node->url.n == 0) {
			node->__loadstatus = LOADER_STABLE; /* a "do-nothing" approach */
		}
		else {
			res = resource_create_multi(&(node->url));
			res->media_type = resm_midi; //resm_fshader;
			node->__loadstatus = LOADER_REQUEST_RESOURCE;
			node->__loadResource = res;
		}
		break;

	case LOADER_REQUEST_RESOURCE:
		res = node->__loadResource;
		resource_identify(node->_parentResource, res);
		/* printf ("load_Inline, we have type  %s  status %s\n",
			resourceTypeToString(res->type), resourceStatusToString(res->status)); */
		res->actions = resa_download | resa_load; //not resa_parse which we do below
		resitem_enqueue(ml_new(res));
		//frontenditem_enqueue(ml_new(res));
		node->__loadstatus = LOADER_FETCHING_RESOURCE;
		break;

	case LOADER_FETCHING_RESOURCE:
		res = node->__loadResource;
		/* printf ("load_Inline, we have type  %s  status %s\n",
			resourceTypeToString(res->type), resourceStatusToString(res->status)); */
			// do we try the next url in the multi-url? 
		if (res->complete) {
			if (res->status == ress_loaded) {
				node->__loadstatus = LOADER_STABLE;
				openned_file_t* of;
				of = res->openned_files;
				if (!of) {
					/* error */
					return;
				}
				node->__loadstatus = LOADER_LOADED;
				node->__blob.p = of->fileData;
				node->__blob.n = of->fileDataSize;
			}
			else if ((res->status == ress_failed) || (res->status == ress_invalid)) {
				//no hope left
				printf("resource failed to load\n");
				node->__loadstatus = LOADER_STABLE; // a "do-nothing" approach 
			}
		}
		break;

	case LOADER_PROCESSING:
		res = node->__loadResource;

		//printf ("inline parsing.... %s\n",resourceStatusToString(res->status));
		printf ("res complete %d\n",res->complete);
		if (res->complete) {
			if (res->status == ress_parsed) {
				node->__loadstatus = LOADER_LOADED;
			}
			else {
				node->__loadstatus = LOADER_STABLE;
			}
		}

		break;
	case LOADER_STABLE:
		break;
	case LOADER_LOADED:
	case LOADER_COMPILED:
		retval = TRUE;
	}
	if (node->__loadstatus == LOADER_STABLE || node->__loadstatus == LOADER_LOADED)
	MARK_NODE_COMPILED
}


#ifdef HAVE_LIBREMIDI
void render_MIDIPortSource(struct X3D_MIDIPortSource* node) {}
void render_MIDIFileSource(struct X3D_MIDIFileSource* node) {
	COMPILE_IF_REQUIRED;
	if (node->__loadstatus == LOADER_LOADED) printf("loaded ");
	if (node->__loadstatus == LOADER_LOADED) {
		//start a thread to parse the blob
		printf("loaded .mid file size = %d\n",node->__blob.n);
	}

}
void render_MIDIPortDestination(struct X3D_MIDIPortDestination* node) {
	if (node->children.n) {
		for (int i = 0; i < node->children.n; i++)
			render_node(X3D_NODE(node->children.p[i]));
	}

}
void render_MIDIFileDestination(struct X3D_MIDIFileDestination* node) {
	if (node->children.n) {
		for (int i = 0; i < node->children.n; i++)
			render_node(X3D_NODE(node->children.p[i]));
	}

}
void render_MIDIOut(struct X3D_MIDIOut* node) {
	if (node->children.n) {
		for (int i = 0; i < node->children.n; i++)
			//libsound_updateNode0(icontext,anode,(struct X3D_Node*) node->children.p[i]);
			render_node(X3D_NODE(node->children.p[i]));
	}
}
void render_MIDIIn(struct X3D_MIDIIn* node) {}
void render_MIDIConverterOut(struct X3D_MIDIConverterOut* node) {}
void render_MIDIConverterIn(struct MIDIConverterIn* node) {}
void render_MIDIToneSplitter(struct MIDIToneSplitter* node) {}
void render_MIDIToneMerger(struct MIDIToneMerger* node) {}
void render_MIDIAudioSynth(struct MIDIAudioSynth* node) {}

#else //HAVE_LIBREMIDI
//stubs
void render_MIDIPortSource(struct X3D_MIDIPortSource* node) {}
void render_MIDIFileSource(struct X3D_MIDIFileSource* node) {}
void render_MIDIPortDestination(struct X3D_MIDIPortDestination* node) {}
void render_MIDIFileDestination(struct X3D_MIDIFileDestination* node) {}
void render_MIDIOut(struct X3D_MIDIOut* node) {}
void render_MIDIIn(struct X3D_MIDIIn* node) {}
void render_MIDIConverterOut(struct X3D_MIDIConverterOut* node) {}
void render_MIDIConverterIn(struct MIDIConverterIn* node) {}
void render_MIDIToneSplitter(struct MIDIToneSplitter* node) {}
void render_MIDIToneMerger(struct MIDIToneMerger* node) {}
void render_MIDIAudioSynth(struct MIDIAudioSynth* node) {}

#endif //HAVE_LIBREMIDI