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
#ifdef HAVE_LIBREMIDI
#include "../../libmidi/libmidi.h"
#else
typedef struct icset { int p; int d; int ld; int n; int s; int ls; } icset;
#endif

typedef struct pComponent_MIDI {
	Stack* midi_context_stack;
	Stack* midi_parent_stack;
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
		p->midi_context_stack = newStack(int);
		stack_push(int, p->midi_context_stack, 0); //a null will signal we have no audio context yet.
		p->midi_parent_stack = newStack(icset);
		icset aps = { 0, 0, 0, 0, 0, 0 };
		stack_push(icset, p->midi_parent_stack, aps); //a null will signal we have no audio parent yet.

	}
}
void Component_MIDI_clear(struct tComponent_MIDI* t) {
	ppComponent_MIDI p = (ppComponent_MIDI)t->prv;
	//deleteVector(struct X3D_Node*, p->audio_context_stack);
}
//ppComponent_MIDI p = (ppComponent_MIDI)gglobal()->Component_MIDI.prv;


#ifdef HAVE_LIBREMIDI

/*
struct X3D_MidiRep {
	int itype; //==8, 0 PointRep 1 LineRep 2 PolyRep 3 MeshRep 4 TextureRep 5 LightRep 6 ProjectorRep 7 SoundRep 8 MidiRep
	int icontext; //map audio_contexts[icontext] = libmidi context
	int inode; //map nodes[inode] = libmidi node
	unsigned int iframe; //last frame visited on scenegraph traversal
	int ibuffer; //just for source nodes with a buffer, like MIDIFileSource
	void* connections;
	int last_indexSource[10];
	int last_indexDestination[10];
	int last_count;
};
*/
struct X3D_MidiRep* getMidiRep(struct X3D_Node* pnode) {
	//main benefit of _intern Rep structure: saves switch-casing on _NodeType 
	// to get specific common fields used for internal processing only
	// -- just put the common fields in the Rep
	// in our case it would be our lookup table int, maybe some AudioNode fields related to connecting, starting, stopping
	struct X3D_MidiRep* srep = NULL;
	if (pnode) {
		srep = (struct X3D_MidiRep*)pnode->_intern;
		if (!srep) {
			srep = (struct X3D_MidiRep*)malloc(sizeof(struct X3D_MidiRep));
			memset(srep, 0, sizeof(struct X3D_MidiRep));
			srep->itype = 8; //MidiRep
			pnode->_intern = (struct X3D_GeomRep*)srep;
		}
	}
	return srep;
}

void register_visit_check(struct X3D_Node* node);
void visit_check_midi(struct X3D_Node* node, unsigned int iframe) {
	struct X3D_MidiRep* srep = getMidiRep(node);
	if (srep->icontext) {
		if (iframe == srep->iframe) {
			libmidi_resumeContext0(srep->icontext);
			//libmidi_resumeNode0(node);
		}
		else {
			//not visited on last frame, perhaps in a switch deactivated branch
			//lets pause the context
			libmidi_pauseContext0(srep->icontext);
			//libmidi_pauseNode0(node);
		}
	}
}
// v4 visibility functions, push & pop (to be) called from all X3DGroupingNode child_ functions
void push_midi_context(int midi_context) {
	ppComponent_MIDI p = (ppComponent_MIDI)gglobal()->Component_MIDI.prv;
	stack_push(int, p->midi_context_stack, midi_context);
}
void create_and_push_midi_context(struct X3D_Node* node) {
	//Hypothesis: Destination / output audio nodes create a context, and child source and processing audio nodes use the context
	//Semi-disconfirmed for MIDI. Source nodes have different threads, but shouldn't affect connections.
	//H2: no need for a context - just make connections whereever.
	struct X3D_MidiRep* srep = getMidiRep(node);
	if (!srep->icontext) {
		int jcontext = peek_midi_context();
		if (!jcontext) {
			jcontext = libmidi_createContext0();
		}
		srep->icontext = jcontext;
		register_visit_check(node); //mainloop will check for you if it visits a node on a frame
	}
	push_midi_context(srep->icontext);
}
void pop_midi_context() {
	ppComponent_MIDI p = (ppComponent_MIDI)gglobal()->Component_MIDI.prv;
	stack_pop(int, p->midi_context_stack);
}
int peek_midi_context() {
	ppComponent_MIDI p = (ppComponent_MIDI)gglobal()->Component_MIDI.prv;
	return stack_top(int, p->midi_context_stack);
}
void push_midi_parent(int inode) {
	ppComponent_MIDI p = (ppComponent_MIDI)gglobal()->Component_MIDI.prv;
	icset aps = { 0, 0, 0, 0, 0, 0 };
	aps.p = inode;
	aps.d = 0;
	aps.ld = 0;
	stack_push(icset, p->midi_parent_stack, aps);
}
void pop_midi_parent() {
	ppComponent_MIDI p = (ppComponent_MIDI)gglobal()->Component_MIDI.prv;
	stack_pop(icset, p->midi_parent_stack);
}
icset peek_midi_parent() {
	ppComponent_MIDI p = (ppComponent_MIDI)gglobal()->Component_MIDI.prv;
	return stack_top(icset, p->midi_parent_stack);
}

int midinewconnect(struct X3D_MidiRep* srep, icset iparent) {
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
int mididisconnect(struct X3D_MidiRep* srep, icset iparent) {
	if (iparent.d == iparent.ld && iparent.s == iparent.ls) return 0; //no change in destination or source
	if (!srep->connections) return 0; //no connections yet to delete
	int found_old = -1;
	for (int i = 0; i < vectorSize(srep->connections); i++) {
		icset conn = vector_get(icset, srep->connections, i);
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
void update_midi_connections(struct X3D_MidiRep* srep, icset iparent)
{
	if (midinewconnect(srep, iparent)) {
		libmidi_connect(srep->icontext, iparent);
		libmidi_print_connections();
	}
	if (mididisconnect(srep, iparent)) {
		libmidi_disconnect(srep->icontext, iparent);
		libmidi_print_connections();
	}
}

void render_MIDIPortSource(struct X3D_MIDIPortSource* node) {
	struct X3D_MidiRep* srep = getMidiRep(X3D_NODE(node));
	srep->iframe = gglobal()->Mainloop.iframe;
	struct X3D_Node* anode = (struct X3D_Node*)node;
	icset iparent = peek_midi_parent();

	//if (node->_ichange != node->_change) {
	if (TRUE) {
		//if (node->_ichange == 0) return;
		int icontext = peek_midi_context();
		libmidi_updateNode3(icontext, iparent, anode);
		//MARK_NODE_COMPILED
		node->_ichange = node->_change;
		node->_ichange++; //come in here every loop
		//could MARK_EVENT outputs
	}
	iparent.n = srep->inode;
	iparent.s = 0;
	update_midi_connections(srep, iparent);

}
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
				node->__blob.p = (int*)of->fileData;
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
		printf("res complete %d\n", res->complete);
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

void render_MIDIFileSource(struct X3D_MIDIFileSource* node) {
	COMPILE_IF_REQUIRED;
	//if (node->__loadstatus == LOADER_LOADED) printf("loaded ");
	if (node->__loadstatus != LOADER_LOADED) return;

	struct X3D_MidiRep* srep = getMidiRep(X3D_NODE(node));
	srep->iframe = gglobal()->Mainloop.iframe;
	struct X3D_Node* anode = (struct X3D_Node*)node;
	icset iparent = peek_midi_parent();
	if (!srep->ibuffer) {
		//start a thread to parse the blob
		printf("loaded .mid file size = %d\n", node->__blob.n);
		srep->ibuffer = node->__blob.n;
		int icontext = peek_midi_context();
		libmidi_updateNode3(icontext, iparent, anode);
	}

	//if (node->_ichange != node->_change) {
	if(TRUE){
		//if (node->_ichange == 0) return;
		int icontext = peek_midi_context();
		libmidi_updateNode3(icontext, iparent, anode);
		//MARK_NODE_COMPILED
		node->_ichange = node->_change;
		node->_ichange++; //come in here every loop
		//could MARK_EVENT outputs
	}
	iparent.n = srep->inode;
	iparent.s = 0;
	update_midi_connections(srep, iparent);

}
void render_MIDIPortDestination(struct X3D_MIDIPortDestination* node) {
	struct X3D_Node* anode = (struct X3D_Node*)node;
	icset have_parent = peek_midi_parent();
	create_and_push_midi_context(anode);
	struct X3D_MidiRep* srep = getMidiRep(X3D_NODE(node));
	libmidi_updateNode3(peek_midi_context(), have_parent, anode);
	if (!have_parent.p) {
		push_midi_parent(srep->inode); // 1); //should be the audio context device node
		icset junk = peek_midi_parent();
	}
	srep->iframe = gglobal()->Mainloop.iframe;
	if (node->children.n) {
		for (int i = 0; i < node->children.n; i++)
			render_node(X3D_NODE(node->children.p[i]));
	}
	if (!have_parent.p)
		pop_midi_parent(); //audio context device node 1
	//libmidi_print_connections();
	pop_midi_context();

}
void render_MIDIPrintDestination(struct X3D_MIDIPrintDestination* node) {
	struct X3D_Node* anode = (struct X3D_Node*)node;
	icset have_parent = peek_midi_parent();
	create_and_push_midi_context(anode);
	struct X3D_MidiRep* srep = getMidiRep(X3D_NODE(node));
	libmidi_updateNode3(peek_midi_context(), have_parent, anode);
	if (!have_parent.p) {
		push_midi_parent(srep->inode); // 1); //should be the audio context device node
	}
	srep->iframe = gglobal()->Mainloop.iframe;
	if (node->children.n) {
		for (int i = 0; i < node->children.n; i++)
			render_node(X3D_NODE(node->children.p[i]));
	}
	if (!have_parent.p)
		pop_midi_parent(); //audio context device node 1
	//libmidi_print_connections();
	pop_midi_context();

}
void render_MIDIFileDestination(struct X3D_MIDIFileDestination* node) {
	if (node->children.n) {
		for (int i = 0; i < node->children.n; i++)
			render_node(X3D_NODE(node->children.p[i]));
	}

}
void render_MIDIOut(struct X3D_MIDIOut* node) {
	struct X3D_Node* anode = (struct X3D_Node*)node;
	icset have_parent = peek_midi_parent();
	create_and_push_midi_context(anode);
	struct X3D_MidiRep* srep = getMidiRep(X3D_NODE(node));
	libmidi_updateNode3(peek_midi_context(), have_parent, anode);
	if (!have_parent.p) {
		push_midi_parent(srep->inode); // 1); //should be the audio context device node
	}
	srep->iframe = gglobal()->Mainloop.iframe;
	if (node->children.n) {
		for (int i = 0; i < node->children.n; i++)
			render_node(X3D_NODE(node->children.p[i]));
	}
	if (!have_parent.p)
		pop_midi_parent(); //audio context device node 1
	pop_midi_context();
}
void render_MIDIIn(struct X3D_MIDIIn* node) {
	struct X3D_MidiRep* srep = getMidiRep(X3D_NODE(node));
	srep->iframe = gglobal()->Mainloop.iframe;
	struct X3D_Node* anode = (struct X3D_Node*)node;
	icset iparent = peek_midi_parent();

	if (node->_ichange != node->_change) {
	//if (TRUE) {
		int icontext = peek_midi_context();
		libmidi_updateNode3(icontext, iparent, anode);
		MARK_NODE_COMPILED
	}
	iparent.n = srep->inode;
	iparent.s = 0;
	update_midi_connections(srep, iparent);
}
/*
enum message_type 
{
	INVALID = 0x0,
	// Standard Message
	NOTE_OFF = 0x80,
	NOTE_ON = 0x90,
	POLY_PRESSURE = 0xA0,
	CONTROL_CHANGE = 0xB0,
	PROGRAM_CHANGE = 0xC0,
	AFTERTOUCH = 0xD0,
	PITCH_BEND = 0xE0,

	// System Common Messages
	SYSTEM_EXCLUSIVE = 0xF0,
	TIME_CODE = 0xF1,
	SONG_POS_POINTER = 0xF2,
	SONG_SELECT = 0xF3,
	RESERVED1 = 0xF4,
	RESERVED2 = 0xF5,
	TUNE_REQUEST = 0xF6,
	EOX = 0xF7,

	// System Realtime Messages
	TIME_CLOCK = 0xF8,
	RESERVED3 = 0xF9,
	START = 0xFA,
	CONTINUE = 0xFB,
	STOP = 0xFC,
	RESERVED4 = 0xFD,
	ACTIVE_SENSING = 0xFE,
	SYSTEM_RESET = 0xFF
};
*/
//typedef unsigned char ubyte;
// MIDI 1 MESSAGES (packed in SFInt32)
void midimsg_uint2values(unsigned int msg, ubyte* channel, ubyte* command, ubyte* note, ubyte* velocity) {
	//we don't care about big endian etc just unpack opposite of packing
	//we are assuming the msg is a 3 byte note on/off, may need if/else on command for others
	ubyte* bytes = (ubyte*)&msg;
	*channel = (bytes[0] & 0xF) + 1;
	*command = bytes[0] - (*channel - 1);
	*note = bytes[1];
	*velocity = bytes[2];

}
unsigned int midimsg_values2uint(ubyte channel, ubyte command, ubyte note, ubyte velocity) {
	unsigned int msg;
	ubyte* bytes = (ubyte*)&msg;
	bytes[0] = (channel - 1) | command;
	bytes[1] = note;
	bytes[2] = velocity;
	return msg;
}

// MIDI 2.0 UMP PACKETS (packed in SFDouble)
void midiump_packet2values(double packet, ubyte* channel, ubyte* command, ubyte* note, ushort* velocity) {
	//we don't care about big endian etc just unpack opposite of packing
	UMP ump;
	ump.packet = packet;
	*channel = (ump.bytes[1] & 0xF) + 1;
	*command = ump.bytes[1] - (*channel - 1);
	*note    = ump.bytes[2];
	*velocity= ump.u16[2];
}
double midiump_values2packet(ubyte channel, ubyte command, ubyte note, ushort velocity) {
	UMP ump;
	ump.bytes[1] = (channel - 1) | command;
	ump.bytes[2] = note;
	ump.u16[2] = velocity;
	return ump.packet;
}

void render_MIDIConverterOut(struct X3D_MIDIConverterOut* node) {}
void render_MIDIConverterIn(struct MIDIConverterIn* node) {}
void offset_notefields_MidiToneSplitter(size_t* cfield) {
	cfield[0] = (offsetof(struct X3D_MIDIToneSplitter, C));
	cfield[1] = (offsetof(struct X3D_MIDIToneSplitter, Cs));
	cfield[2] = (offsetof(struct X3D_MIDIToneSplitter, D));
	cfield[3] = (offsetof(struct X3D_MIDIToneSplitter, Ds));
	cfield[4] = (offsetof(struct X3D_MIDIToneSplitter, E));
	cfield[5] = (offsetof(struct X3D_MIDIToneSplitter, F));
	cfield[6] = (offsetof(struct X3D_MIDIToneSplitter, Fs));
	cfield[7] = (offsetof(struct X3D_MIDIToneSplitter, G));
	cfield[8] = (offsetof(struct X3D_MIDIToneSplitter, Gs));
	cfield[9] = (offsetof(struct X3D_MIDIToneSplitter, A));
	cfield[10] = (offsetof(struct X3D_MIDIToneSplitter, As));
	cfield[11] = (offsetof(struct X3D_MIDIToneSplitter, B));
	cfield[12] = (offsetof(struct X3D_MIDIToneSplitter, pedal));

}
void render_MIDIToneSplitter(struct X3D_MIDIToneSplitter* node) {
	static size_t cfields[13] = { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0 };
	if (cfields[0] == 0) offset_notefields_MidiToneSplitter(cfields);

	struct X3D_Node* anode = X3D_NODE(node);
	if (node->_ichange != node->_change) {
		if (MIDITransport() == MIDI_MSG) {
			for (int i = 0; i < node->midiMsg.n; i++)
			{
				ubyte note, channel, command, velocity;
				unsigned int msg = (unsigned int)node->midiMsg.p[i];
				midimsg_uint2values(msg, &channel, &command, &note, &velocity);
				int status = command == NOTE_ON && velocity > 0 ? TRUE : FALSE;
				int octave = note / 12;
				int inote = note % 12;
				if (channel == node->channelFilter || node->channelFilter == -1) {
					if (command == NOTE_ON || command == NOTE_OFF) {
						if (octave == node->octaveFilter || node->octaveFilter == -1)
						{
							//printf("inote = %d cfields %zu\n", inote,cfields[inote]);
							int* field = (int*)(cfields[inote] + (unsigned char*)node); //fancy offsetof to eliminate switch-case
							*field = status;
							MARK_EVENT(anode, cfields[inote]);
						}
					}
					else if (command == CONTROL_CHANGE && note == 64) {
						node->pedal = velocity == 0 ? FALSE : TRUE;
						MARK_EVENT(anode, offsetof(struct X3D_MIDIToneSplitter, pedal));
					}
				}
			}
		}
		else if (MIDITransport() == MIDI_UMP) {
			for (int i = 0; i < node->midiUmp.n; i++)
			{
				ubyte note, channel, command;
				ushort velocity;
				UMP ump;
				ump.packet = node->midiUmp.p[i];
				midiump_packet2values(ump.packet, &channel, &command, &note, &velocity);
				int status = command == NOTE_ON && velocity > 0 ? TRUE : FALSE;
				int octave = note / 12;
				int inote = note % 12;
				if (channel == node->channelFilter || node->channelFilter == -1) {
					if (command == NOTE_ON || command == NOTE_OFF) {
						if (octave == node->octaveFilter || node->octaveFilter == -1)
						{
							//printf("inote = %d cfields %zu\n", inote,cfields[inote]);
							int* field = (int*)(cfields[inote] + (unsigned char*)node); //fancy offsetof to eliminate switch-case
							*field = status;
							MARK_EVENT(anode, cfields[inote]);
						}
					}
					else if (command == CONTROL_CHANGE && note == 64) {
						node->pedal = velocity == 0 ? FALSE : TRUE;
						MARK_EVENT(anode, offsetof(struct X3D_MIDIToneSplitter, pedal));
					}
				}
			}
		}

				/*
				switch (inote) {
				case 0: node->C   = status; MARK_EVENT(anode, offsetof(struct X3D_MIDIToneSplitter, C)); break;
				case 1: node->Cs  = status; MARK_EVENT(anode, offsetof(struct X3D_MIDIToneSplitter, Cs)); break;
				case 2: node->D   = status; MARK_EVENT(anode, offsetof(struct X3D_MIDIToneSplitter, D)); break;
				case 3: node->Ds  = status; MARK_EVENT(anode, offsetof(struct X3D_MIDIToneSplitter, Ds)); break;
				case 4: node->E   = status; MARK_EVENT(anode, offsetof(struct X3D_MIDIToneSplitter, E)); break;
				case 5: node->F   = status; MARK_EVENT(anode, offsetof(struct X3D_MIDIToneSplitter, F)); break;
				case 6: node->Fs  = status; MARK_EVENT(anode, offsetof(struct X3D_MIDIToneSplitter, Fs)); break;
				case 7: node->G   = status; MARK_EVENT(anode, offsetof(struct X3D_MIDIToneSplitter, G)); break;
				case 8: node->Gs  = status; MARK_EVENT(anode, offsetof(struct X3D_MIDIToneSplitter, Gs)); break;
				case 9: node->A   = status; MARK_EVENT(anode, offsetof(struct X3D_MIDIToneSplitter, A)); break;
				case 10: node->As = status; MARK_EVENT(anode, offsetof(struct X3D_MIDIToneSplitter, As)); break;
				case 11: node->B  = status; MARK_EVENT(anode, offsetof(struct X3D_MIDIToneSplitter, B)); break;
				default: break;
				}
				*/
		MARK_NODE_COMPILED

	}
}
void offset_notefields_MidiToneMerger(size_t * cfield) {
	cfield[0] = (offsetof(struct X3D_MIDIToneMerger, C));
	cfield[1] = (offsetof(struct X3D_MIDIToneMerger, Cs));
	cfield[2] = (offsetof(struct X3D_MIDIToneMerger, D));
	cfield[3] = (offsetof(struct X3D_MIDIToneMerger, Ds));
	cfield[4] = (offsetof(struct X3D_MIDIToneMerger, E));
	cfield[5] = (offsetof(struct X3D_MIDIToneMerger, F));
	cfield[6] = (offsetof(struct X3D_MIDIToneMerger, Fs));
	cfield[7] = (offsetof(struct X3D_MIDIToneMerger, G));
	cfield[8] = (offsetof(struct X3D_MIDIToneMerger, Gs));
	cfield[9] = (offsetof(struct X3D_MIDIToneMerger, A));
	cfield[10] = (offsetof(struct X3D_MIDIToneMerger, As));
	cfield[11] = (offsetof(struct X3D_MIDIToneMerger, B));
	cfield[12] = (offsetof(struct X3D_MIDIToneMerger, pedal));
}
void render_MIDIToneMerger(struct X3D_MIDIToneMerger* node) {
	struct X3D_Node* anode = X3D_NODE(node);
	//static struct Multi_Bool lastnote;
	static size_t cfields[13]; //reserve last one for pedal
	if (node->_lastnote.n == 0)
	{
		//we store last frame's note on/off values, so we can detect if something changed
		node->_lastnote.p = malloc(13 * sizeof(int));
		node->_lastnote.n = 13;
		for (int i = 0; i < 13; i++) node->_lastnote.p[i] = FALSE;
		offset_notefields_MidiToneMerger(cfields);
	}
	if (node->_ichange != node->_change) {
		int n, mark, mnote[12]; //max polyphony 12,assume fixed octave and max 12 changed notes in octave per frame
		double dnote[12];
		n = 0;
		mark = FALSE;
		for(int i=0;i<node->_lastnote.n;i++)
		{
			int lastval = node->_lastnote.p[i];
			int octave = node->octave;
			int channel = node->channel;
			int inote = i;
			int note = i + 12 * octave;
			int *field = (int*)(cfields[i] + (unsigned char*)node); //fancy offsetof to eliminate switch-case
			int curval = *field;
			if(curval != lastval) { 
				ubyte command;
				if (i < 12) {
					command = curval ? NOTE_ON : NOTE_OFF;
				}
				else {
					command = CONTROL_CHANGE;
					note = 64; //pedal?
				}
				if (MIDITransport() == MIDI_MSG){
					ubyte velocity = curval ? 127 : 0;
					mnote[n] = midimsg_values2uint(channel, command, note, velocity);
				}
				if (MIDITransport() == MIDI_UMP) {
					ushort velocity = curval ? 65535 : 0;
					dnote[n] = midiump_values2packet(channel, command, note, velocity);
				}
				n++;
				//mnote[n] = curval ? note : -note; n++; 
				mark = TRUE;
			}
			node->_lastnote.p[i] = curval;
			/*
			if (octave == node->octave )
			{
				switch (inote) {
				case 0:  if (node->C != status) { mnote[n] = node->C ? note : -note; n++; mark = TRUE; } break;
				case 1:  if (node->Cs != status) { mnote[n] = node->C ? note : -note; n++; mark = TRUE; } break;
				case 2:  if (node->D != status) { mnote[n] = node->C ? note : -note; n++; mark = TRUE; } break;
				case 3:  if (node->Ds != status) { mnote[n] = node->C ? note : -note; n++; mark = TRUE; }break;
				case 4:  if (node->E != status) { mnote[n] = node->C ? note : -note; n++; mark = TRUE; }break;
				case 5:  if (node->F != status) { mnote[n] = node->C ? note : -note; n++; mark = TRUE; }break;
				case 6:  if (node->Fs != status) { mnote[n] = node->C ? note : -note; n++; mark = TRUE; }break;
				case 7:  if (node->G != status) { mnote[n] = node->C ? note : -note; n++; mark = TRUE; }break;
				case 8:  if (node->Gs != status) { mnote[n] = node->C ? note : -note; n++; mark = TRUE; }break;
				case 9: if (node->A != status) { mnote[n] = node->C ? note : -note; n++; mark = TRUE; }break;
				case 10: if (node->As != status) { mnote[n] = node->C ? note : -note; n++; mark = TRUE; } break;
				case 11: if (node->B != status) { mnote[n] = node->C ? note : -note; n++; mark = TRUE; }break;
				default: break;
				}
			}
			*/
		}
		if (mark) {
			if (MIDITransport() == MIDI_MSG) {
				// MIDI 1 messages packed into MFInt32
				node->midiMsg.p = realloc(node->midiMsg.p, n * sizeof(int));
				memcpy(node->midiMsg.p, mnote, n * sizeof(int));
				node->midiMsg.n = n;
				MARK_EVENT(anode, offsetof(struct X3D_MIDIToneMerger, midiMsg));
			}
			if (MIDITransport() == MIDI_UMP) {
				// MIDI 2 packets packed into MFDouble
				node->midiUmp.p = realloc(node->midiUmp.p, n * sizeof(double));
				memcpy(node->midiUmp.p, dnote, n * sizeof(double));
				node->midiUmp.n = n;
				MARK_EVENT(anode, offsetof(struct X3D_MIDIToneMerger, midiUmp));
			}
		}
		MARK_NODE_COMPILED

	}

}
void render_MIDIAudioSynth(struct MIDIAudioSynth* node) {}

static midi_transport_method = MIDI_UMP;
void set_MIDITransport(int method) {
	midi_transport_method = method == 1 || method == 2 ? method : midi_transport_method;
}
int MIDITransport() {
	return midi_transport_method; // MIDI_UMP;
	//return MIDI_MSG;
}
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