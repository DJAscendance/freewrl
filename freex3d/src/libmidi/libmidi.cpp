

/* part of libfreewrl, usual libfreewrl license
not currently building this as a lib, 
-- just putting libmidi.cpp in libfreewrl list of source files
-- so could be in /lib/scenegraph
- called by Component_MIDI.c
General design: 
- similar to web audio, midi nodes will be implicitly connected by scenegraph hierarchy
- a separate thread will be recursing from MIDIsource through list of registered listener nodes
-- to MIDI destination or MIDI utility node that converts to ROUTEs or to web audio sound node
- the rendering thread will visit once per frame, scrape the latest data
-- and send it as field updates / routes or whatever.
- relies on libremidi https://github.com/jcelerier/libremidi
-- which uses C++ 17, so needs this wrapper libmidi
- libremidi is a close derivitive of rtmidi https://www.music.mcgill.ca/~gary/rtmidi/index.html 
-- rtmidi is a close 2nd choice / fallback, and apparently has an optional C interface
*/
#ifdef HAVE_LIBREMIDI
#define __STDC_LIMIT_MACROS
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <libremidi/libremidi.hpp>
#include <libremidi/reader.hpp>
#include <thread> //https://en.cppreference.com/w/cpp/thread/thread
#include <mutex>
#include <condition_variable>
#include <map>


//make the interface flat C
#ifdef __cplusplus
extern "C" {
#endif
#define TRUE 1
#define FALSE 0

#define GLDOUBLE double
typedef struct {
    void* p;                   /* Pointer to actual object */
    unsigned int x;             /* Extra information - reuse count etc */
} ptw32_handle_t;

typedef ptw32_handle_t pthread_t;
#include "../lib/vrml_parser/Structs.h"
#include "libmidi.h"

typedef struct MidiNode {
    int itype;
    int numberOfInputs;
    int numberOfOutputs;
    //std::list<std::shared_ptr<MidiNode>> inputs;
    std::list<std::shared_ptr<MidiNode>> outputs;
} MidiNode;
struct mcstruct {
    std::thread context;
    bool running;
    int next_node;
    //int next_bus;
    std::map<int, std::shared_ptr<MidiNode>> nodes;
    std::map<int, int> nodetype;
 };
static int next_midi_context = 0;
static std::map<int, struct mcstruct*> midi_contexts;
void midi_context_function(struct mcstruct* ac) {
    while (true) {
        if (!ac->running) 
            std::this_thread::sleep_for(std::chrono::seconds(1));
        else {

        }
    }
}
int libmidi_createContext0() {
    struct mcstruct *ac = new mcstruct();

    next_midi_context++;
    midi_contexts[next_midi_context] = ac;
    ac->context = std::thread(midi_context_function, ac);
    ac->running = FALSE;
	return next_midi_context;
}
void midifilesourcefunction(libremidi::reader &r) {
    for (const auto& track : r.tracks)
    {
        std::cout << "\nNew track\n\n";
        for (const libremidi::track_event& event : track)
        {
            std::cout << "Event at " << event.tick << " : ";
            if (event.m.is_meta_event())
            {
                std::cout << "Meta event";
            }
            else
            {
                switch (event.m.get_message_type())
                {
                case libremidi::message_type::NOTE_ON:
                    std::cout << "Note ON: "
                        << "channel " << event.m.get_channel() << ' '
                        << "note " << (int)event.m.bytes[1] << ' '
                        << "velocity " << (int)event.m.bytes[2] << ' ';
                    break;
                case libremidi::message_type::NOTE_OFF:
                    std::cout << "Note OFF: "
                        << "channel " << event.m.get_channel() << ' '
                        << "note " << (int)event.m.bytes[1] << ' '
                        << "velocity " << (int)event.m.bytes[2] << ' ';
                    break;
                case libremidi::message_type::CONTROL_CHANGE:
                    std::cout << "Control: "
                        << "channel " << event.m.get_channel() << ' '
                        << "control " << (int)event.m.bytes[1] << ' '
                        << "value " << (int)event.m.bytes[2] << ' ';
                    break;
                case libremidi::message_type::PROGRAM_CHANGE:
                    std::cout << "Program: "
                        << "channel " << event.m.get_channel() << ' '
                        << "program " << (int)event.m.bytes[1] << ' ';
                    break;
                case libremidi::message_type::AFTERTOUCH:
                    std::cout << "Aftertouch: "
                        << "channel " << event.m.get_channel() << ' '
                        << "value " << (int)event.m.bytes[1] << ' ';
                    break;
                case libremidi::message_type::POLY_PRESSURE:
                    std::cout << "Poly pressure: "
                        << "channel " << event.m.get_channel() << ' '
                        << "note " << (int)event.m.bytes[1] << ' '
                        << "value " << (int)event.m.bytes[2] << ' ';
                    break;
                case libremidi::message_type::PITCH_BEND:
                    std::cout << "Poly pressure: "
                        << "channel " << event.m.get_channel() << ' '
                        << "bend " << (int)(event.m.bytes[1] << 7 + event.m.bytes[2]) << ' ';
                    break;
                default:
                    std::cout << "Unsupported.";
                    break;
                }
            }
            std::cout << '\n';
        }
    }
}
void libmidi_updateNode3(int icontext, icset connect_parent, struct X3D_Node* node) {
    struct mcstruct* ac = midi_contexts[icontext];
    //goal- switch-case on x3d nodeType and do any midinode create+connect, update input or update output
    struct X3D_MidiRep* srepn = (struct X3D_MidiRep*)node->_intern;
    switch (node->_nodeType) {
    case NODE_MIDIFileSource:
    {
        struct X3D_MIDIFileSource* pnode = (struct X3D_MIDIFileSource*)node;
        std::shared_ptr<MidiNode> input;
        MidiNode* input_ptr;
        if (!srepn->inode) {

            // Initialize our reader object
            libremidi::reader r(true); //use abolute? time I think

            // Parse
            libremidi::reader::parse_result result = r.parse((uint8_t*)pnode->__blob.p,pnode->__blob.n);

            // If parsing succeeded, use the parsed data
            if (result != libremidi::reader::invalid) {
                //thread filesource(midifilesourcefunction, &r);
                for (auto& track : r.tracks) {
                    for (const libremidi::track_event& event : track) {
                        std::cout << (int)event.m.bytes[0] << '\n';
                    }
                }
            }
            //create midi node
            //{
            //    std::lock_guard<std::mutex> lock(ac->context);
            //    ac->context., "microphone");
            //    //input = lab::MakeAudioHardwareInputNode(r);
            //    std::shared_ptr<AudioHardwareInputNode> inputNode(
            //        new AudioHardwareInputNode(*ac->context.get(), ac->context.get()->destinationNode()->device()->sourceProvider()));
            //    input = inputNode;
            //    //ac->context.get()->connect(ac->context.get()->destinationNode(), inputNode, 0, 0);
            //}

            ac->next_node++;
            ac->nodes[ac->next_node] = input;
            ac->nodetype[ac->next_node] = NODE_MicrophoneSource;
            srepn->inode = ac->next_node;
            srepn->icontext = icontext;
            //if (iparent.x)
            //    libsound_connect2(icontext, iparent.x, srepn->inode, iparent.y, iparent.z);
        }
        //copy changed values from x3d to labsound
        //input_ptr = static_cast<AudioHardwareInputNode*>(ac->nodes[srepn->inode].get());
    }
    break;

    default:
        break;
    }
}
void libmidi_pauseContext0(int icontext) {}
void libmidi_resumeContext0(int icontext) {}


static struct type_name {
    int iname;
    const char* cname;
} type_names[] = {
{NODE_MIDIPortDestination, "MPD"},
{NODE_MIDIPortSource, "MPS"},
{NODE_MIDIFileDestination, "MFD"},
{NODE_MIDIFileSource, "MFS"},
{NODE_MIDIIn, "MIn"},
{NODE_MIDIOut, "MOut"},
{0,NULL},
};
static const char* nodetype_lookup(int itype) {
    int i;
    const char* cname;
    struct type_name* tn;
    i = 0;
    cname = "";
    do {
        tn = &type_names[i];
        if (tn->iname == itype) {
            cname = tn->cname;
            break;
        }
        i++;
    } while (tn->iname != 0);
    return cname;

}
//this one uses int index lookup in a map, much like a vector except can deleted elements
struct midiconnection {
    int icontext;
    int iparent;
    int iparent_type;
    int ichild;
    int ichild_type;
    int srcindex;
    int dstindex;
};
static std::list<midiconnection> midiconnections;
void context_disconnect(std::shared_ptr<MidiNode> destination, std::shared_ptr<MidiNode> source, int indexDst, int indexSrc) {
    source->outputs.remove(destination);
    //destination->inputs.remove(source);
}
void context_connect(std::shared_ptr<MidiNode> destination, std::shared_ptr<MidiNode> source, int indexDst, int indexSrc) {
    source->outputs.push_back(destination);
    //destination->inputs.push_back(source);
}
void libmidi_connect2(int icontext, int idestination, int isource, int indexDst, int indexSrc) {
    struct mcstruct* ac = midi_contexts[icontext];
    std::shared_ptr<MidiNode> destination = ac->nodes[idestination];
    std::shared_ptr<MidiNode> source = ac->nodes[isource];
    int dstInputs = destination->numberOfInputs;
    int srcOutputs = source->numberOfOutputs;
    if (indexDst > dstInputs) {
        printf("destination number of inputs %d destination idx %d\n", destination->numberOfInputs, indexDst);
        printf("\n");
        printf("\n");
        printf("\n");
        printf("\n");
        return;
    }
    if (indexSrc > srcOutputs) {
        printf("source number of outputs %d source idx %d\n", srcOutputs, indexSrc);
        printf("\n");
        printf("\n");
        printf("\n");
        printf("\n");
        return;
    }

    //ac->context->connect(destination, source, indexDst, indexSrc);
    context_connect(destination, source, indexDst, indexSrc);
    int iparent_type = ac->nodetype[idestination];
    int ichild_type = ac->nodetype[isource];
    struct midiconnection cc; cc.icontext = icontext; cc.iparent = idestination; cc.iparent_type = iparent_type;
    cc.ichild = isource; cc.ichild_type = ichild_type; cc.srcindex = indexSrc; cc.dstindex = indexDst;
    midiconnections.push_back(cc);

}
void libmidi_disconnect2(int icontext, int idestination, int isource, int indexDst, int indexSrc) {
    struct mcstruct* ac = midi_contexts[icontext];
    std::shared_ptr<MidiNode> destination = ac->nodes[idestination];
    std::shared_ptr<MidiNode> source = ac->nodes[isource];
    int dstInputs = destination->numberOfInputs;
    int srcOutputs = source->numberOfOutputs;
    if (indexDst > dstInputs) {
        printf("destination number of inputs %d destination idx %d\n", destination->numberOfInputs, indexDst);
        printf("\n");
        printf("\n");
        printf("\n");
        printf("\n");
        return;
    }
    if (indexSrc > srcOutputs) {
        printf("source number of outputs %d source idx %d\n", srcOutputs, indexSrc);
        printf("\n");
        printf("\n");
        printf("\n");
        printf("\n");
        return;
    }

    //ac->context->disconnect(destination, source, indexDst, indexSrc);
    context_disconnect(destination, source, indexDst, indexSrc);
    //find and remove from vector
    // vec.erase(vec.begin() + index);
    int iparent_type = ac->nodetype[idestination];
    int ichild_type = ac->nodetype[isource];
    struct midiconnection cc; cc.icontext = icontext; cc.iparent = idestination; cc.iparent_type = iparent_type;
    cc.ichild = isource; cc.ichild_type = ichild_type; cc.srcindex = indexSrc; cc.dstindex = indexDst;

    std::list<midiconnection>::iterator it;
    for (it = midiconnections.begin(); it != midiconnections.end(); ++it) {
        midiconnection cn = *it;
        if (cn.icontext = cc.icontext && cn.iparent == cc.iparent && cn.ichild == cc.ichild
            && cn.srcindex == cc.srcindex && cn.dstindex == cc.dstindex) {
            midiconnections.erase(it);
            break;
        }
    }

}

void libmidi_connect(int icontext, icset iparent) {
    if (iparent.p)
        libmidi_connect2(icontext, iparent.p, iparent.n, iparent.d, iparent.s);
}
void libmidi_disconnect(int icontext, icset iparent) {
    libmidi_disconnect2(icontext, iparent.p, iparent.n, iparent.ld, iparent.ls);

}
void libmidi_print_connections() {
    printf("\n");
    printf("%2s %7s %4s %7s %7s %6s %4s\n", "ic", "iparent", "type", "dstIndx", "srcIndex", "ichild", "type");
    //for (int i = 0; i < connections.size(); i++) {
    //    struct connection cc = connections[i];
    std::list<midiconnection>::iterator it;
    for (it = midiconnections.begin(); it != midiconnections.end(); ++it) {
        midiconnection cc = *it;

        const char* ptype = nodetype_lookup(cc.iparent_type);
        const char* ctype = nodetype_lookup(cc.ichild_type);

        printf("%2d %7d %4s %7d %7d %6d %4s\n", cc.icontext, cc.iparent, ptype, cc.dstindex, cc.srcindex, cc.ichild, ctype);
    }
    printf("count %d\n", (int)midiconnections.size());
}


#ifdef __cplusplus
}
#endif

#else // HAVE_LIBREMIDI
#endif //HAVE_LIBREMIDI