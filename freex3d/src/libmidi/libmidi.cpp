

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
#include <queue>

static libremidi::midi_in midiin;
static libremidi::midi_out midiout;

extern "C" {
    void midiin_C_callback(const libremidi::message * msg);
}
void midiin_callback(libremidi::message& msg)
{
    midiin_C_callback(&msg);
}
void set_midiin_callback() {
    std::cout << "setting input callback" << std::endl;
    midiin.set_callback(
        [](const libremidi::message& message)
        {
            /*
            std::cout << "input callback called " << std::endl;
            std::vector<unsigned char> messout(message.size());
            auto nBytes = message.size();
            for (auto i = 0U; i < nBytes; i++)
                std::cout << "Byte " << i << " = " << (int)message[i] << ", ";
            if (nBytes > 0)
                std::cout << "stamp = " << message.timestamp << std::endl;
            messout[0] = message[0];
            messout[1] = message[1];
            messout[2] = message[2];
            if (messout[2] > 0 && messout[2] < 64)
                messout[2] = 64;
            midiout.send_message(messout);
            */
            midiin_C_callback(&message);

        });

}
// A threadsafe-queue. https://stackoverflow.com/questions/15278343/c11-thread-safe-queue 
template <class T>
class SafeQueue
{
public:
    SafeQueue(void)
        : q()
        , m()
        , c()
    {}

    ~SafeQueue(void)
    {}

    // Add an element to the queue.
    void enqueue(T t)
    {
        std::lock_guard<std::mutex> lock(m);
        q.push(t);
        c.notify_one();
    }

    // Get the "front"-element.
    // If the queue is empty, wait till a element is avaiable.
    T dequeue(void)
    {
        std::unique_lock<std::mutex> lock(m);
        if (q.empty()) return nullptr;
        //while (q.empty())
        //{
        //    // release lock as long as the wait and reaquire it afterwards.
        //    c.wait(lock);
        //}
        T val = q.front();
        q.pop();
        return val;
    }

private:
    std::queue<T> q;
    mutable std::mutex m;
    std::condition_variable c;
};

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
    //std::list<MidiNode> inputs;
    std::list<MidiNode*> outputs;
    void (*takemessage)(MidiNode*, const struct libremidi::message *);
    libremidi::reader* reader;
    int run;
    int loop;
    void* queue;
} MidiNode;
struct mcstruct {
    std::thread context;
    bool running;
    int next_node;
    //int next_bus;
    std::map<int, MidiNode*> nodes;
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
void midifilesourcefunction(MidiNode* mnode) {

    libremidi::reader* r = mnode->reader;

    printf("ticks per beat %f\n", r->ticksPerBeat);
    do {
        for (const auto& track : r->tracks)
        {
            int last_tick = 0;
            std::cout << "\nNew track\n\n";
            for (const libremidi::track_event& event : track)
            {
                while (mnode->run != TRUE) std::this_thread::sleep_for(std::chrono::milliseconds(50));
                std::cout << "Event at " << event.tick << " : ";
                if (event.tick) {
                    //std::this_thread::sleep_for(std::chrono::milliseconds((int)((float)(event.tick - last_tick) *1000.0f / r->ticksPerBeat / 12.0f)));
                    int elapsed_ticks = event.tick - last_tick;
                    float beats_per_second = 2.0f;
                    float elapsed_seconds = ((float)elapsed_ticks / (r->ticksPerBeat * beats_per_second));
                    int elapsed_msec = (int)(elapsed_seconds * 1000.0f);
                    std::this_thread::sleep_for(std::chrono::milliseconds(elapsed_msec));
                    last_tick = event.tick;
                }
                if (event.m.is_meta_event())
                {
                    std::cout << "Meta event";
                }
                else
                {
                    //std::wcout << "outputs.count" << mnode->outputs.size() << std::endl;
                    for (std::list<MidiNode*>::iterator it = mnode->outputs.begin(); it != mnode->outputs.end(); ++it)
                    {
                        MidiNode* mout = *it;
                        libremidi::message msg = event.m;
                        //std::cout << "mout->takemessage=" << mout->takemessage << std::endl;
                        if (mout->takemessage) mout->takemessage(mout, &msg);
                    }
                    if (0) switch (event.m.get_message_type())
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
        std::cout << "end of tracks" << std::endl;
    } while (mnode->loop == TRUE);
    std::cout << "bye bye" << std::endl;
}
void midiPortDestination_takemessage(MidiNode* midiNode, const struct libremidi::message* msg) {
    const struct libremidi::message& m = *msg;
    std::vector<unsigned char> messout(m.size());
    auto nBytes = m.size();
    std::cout << "PD ";
    for (auto i = 0U; i < nBytes; i++)
        std::cout << "Byte " << i << " = " << (int)m[i] << ", ";
    if (nBytes > 0)
        std::cout << "stamp = " << m.timestamp << std::endl;
    messout[0] = m[0];
    messout[1] = m[1];
    messout[2] = m[2];
    if (messout[2] > 0 && messout[2] < 64)
        messout[2] = 64;
    libremidi::message mout = libremidi::message(messout,m.timestamp);
    midiout.send_message(mout);

}
void midiPrintDestination_takemessage(MidiNode* midiNode, const struct libremidi::message * msg) {
    const struct libremidi::message& m = *msg;
    switch (m.get_message_type())
    {
    case libremidi::message_type::NOTE_ON:
        std::cout << "Note ON: "
            << "channel " << m.get_channel() << ' '
            << "note " << (int)m.bytes[1] << ' '
            << "velocity " << (int)m.bytes[2] << ' ';
        break;
    case libremidi::message_type::NOTE_OFF:
        std::cout << "Note OFF: "
            << "channel " << m.get_channel() << ' '
            << "note " << (int)m.bytes[1] << ' '
            << "velocity " << (int)m.bytes[2] << ' ';
        break;
    case libremidi::message_type::CONTROL_CHANGE:
        std::cout << "Control: "
            << "channel " << m.get_channel() << ' '
            << "control " << (int)m.bytes[1] << ' '
            << "value " << (int)m.bytes[2] << ' ';
        break;
    case libremidi::message_type::PROGRAM_CHANGE:
        std::cout << "Program: "
            << "channel " << m.get_channel() << ' '
            << "program " << (int)m.bytes[1] << ' ';
        break;
    case libremidi::message_type::AFTERTOUCH:
        std::cout << "Aftertouch: "
            << "channel " << m.get_channel() << ' '
            << "value " << (int)m.bytes[1] << ' ';
        break;
    case libremidi::message_type::POLY_PRESSURE:
        std::cout << "Poly pressure: "
            << "channel " << m.get_channel() << ' '
            << "note " << (int)m.bytes[1] << ' '
            << "value " << (int)m.bytes[2] << ' ';
        break;
    case libremidi::message_type::PITCH_BEND:
        std::cout << "Poly pressure: "
            << "channel " << m.get_channel() << ' '
            << "bend " << (int)(m.bytes[1] << 7 + m.bytes[2]) << ' ';
        break;
    default:
        std::cout << "Unsupported.";
        break;
    }

    std::cout << " PrintDest\n";

}
void midiOut_takemessage(MidiNode* midiNode, const struct libremidi::message* msg) {
    const struct libremidi::message& m = *msg;
    if (!midiNode->queue)
        midiNode->queue = (void*) new SafeQueue<const struct libremidi::message*>();
    SafeQueue<const struct libremidi::message*> *que = (SafeQueue<const struct libremidi::message*>*)midiNode->queue;
    std::cout << "enqueuing one" << std::endl;
    que->enqueue(new libremidi::message(*msg));
    std::cout << "enqueued one" << std::endl;

    /*
    switch (m.get_message_type())
    {
    case libremidi::message_type::NOTE_ON:
        std::cout << "Note ON: "
            << "channel " << m.get_channel() << ' '
            << "note " << (int)m.bytes[1] << ' '
            << "velocity " << (int)m.bytes[2] << ' ';
        break;
    case libremidi::message_type::NOTE_OFF:
        std::cout << "Note OFF: "
            << "channel " << m.get_channel() << ' '
            << "note " << (int)m.bytes[1] << ' '
            << "velocity " << (int)m.bytes[2] << ' ';
        break;
    case libremidi::message_type::CONTROL_CHANGE:
        std::cout << "Control: "
            << "channel " << m.get_channel() << ' '
            << "control " << (int)m.bytes[1] << ' '
            << "value " << (int)m.bytes[2] << ' ';
        break;
    }
    */
}
void mark_event(struct X3D_Node* from, int totalptr);
//#define MARK_EVENT(node,offset)	mark_event(X3D_NODE(node),(int) offset)
void midiOut_message2fields(MidiNode* midiNode, struct X3D_MIDIOut* node) {
    struct X3D_Node* anode = X3D_NODE(node);
    const struct libremidi::message *msg;
    if (!midiNode->queue)
        midiNode->queue = (void*) new SafeQueue<const struct libremidi::message*>();
    SafeQueue<const struct libremidi::message*>* que = (SafeQueue<const struct libremidi::message*>*)midiNode->queue;
    Multi_Int32* last = &node->midiNote;
    int cur[1000], n = 0;
    int pedal = FALSE;
    int mark = FALSE;
    //std::cout << "starting dequeue loop" << std::endl;

    while (msg = que->dequeue()) {
        std::cout << "dequed one" << std::endl;
        
        const struct libremidi::message& m = *msg;
        switch (m.get_message_type())
        {
        case libremidi::message_type::NOTE_ON:
            std::cout << "Note ON: "
                << "channel " << m.get_channel() << ' '
                << "note " << (int)m.bytes[1] << ' '
                << "velocity " << (int)m.bytes[2] << ' ';
            cur[n] = (int)m.bytes[2] > 0 ? (int)m.bytes[1] : -(int)m.bytes[1];
            n++;
            break;
        case libremidi::message_type::NOTE_OFF:
            std::cout << "Note OFF: "
                << "channel " << m.get_channel() << ' '
                << "note " << (int)m.bytes[1] << ' '
                << "velocity " << (int)m.bytes[2] << ' ';
            cur[n] = -(int)m.bytes[1]; //negative sign for OFF, + for ON
            n++;
            break;
        case libremidi::message_type::CONTROL_CHANGE:
            std::cout << "Control: "
                << "channel " << m.get_channel() << ' '
                << "control " << (int)m.bytes[1] << ' '
                << "value " << (int)m.bytes[2] << ' ';
            pedal = node->pedal;
            node->pedal = m.bytes[2] > 0 ? TRUE : FALSE;
            if(pedal != node->pedal) MARK_EVENT(anode, offsetof(struct X3D_MIDIOut, pedal));
            break;
        default:
            break;
        }
        
       //I don't have a destructor,memory use will escalate ~msg();
    }
    //std::cout << "ended dequeue loop" << std::endl;

    if (n) {
        node->midiNote.p = (int *)realloc(node->midiNote.p, n * sizeof(int));
        memcpy(node->midiNote.p, cur, n * sizeof(int));
        node->midiNote.n = n;
        MARK_EVENT(anode, offsetof(struct X3D_MIDIOut, midiNote));
    }
    else {
        node->midiNote.n = 0;
    }
    //std::cout << "finished midiOut render" << std::endl;

}
double TickTime();
void midiin_midinote2messages(MidiNode* mnode, struct X3D_MIDIIn* pnode) {
    static double lasttime = 0.0;
    if (lasttime == 0.0) lasttime = TickTime();
    for (int i = 0; i < pnode->midiNote.n; i++) {
        //libremidi::message *msg = new libremidi:message()
        unsigned char inote = abs(pnode->midiNote.p[i]);
        unsigned char velocity = pnode->midiNote.p[i] > 0 ? 64 : 0;
        //std::vector<unsigned char> messout(3);
        //messout[0] = 144; // 176; //its a note
        //messout[1] = inote;
        //messout[2] = on;
        double now = TickTime();
        double timestamp = now - lasttime;
        lasttime = now;
        libremidi::message msg; // = libremidi::message(messout, timestamp);
        if (velocity)
            msg = libremidi::message::note_on(1, inote, velocity);
        else
            msg = libremidi::message::note_off(1, inote, velocity);
        for (std::list<MidiNode*>::iterator it = mnode->outputs.begin(); it != mnode->outputs.end(); ++it)
        {
            MidiNode* mout = *it;
            //std::cout << "mout->takemessage=" << mout->takemessage << std::endl;
            if (mout->takemessage) mout->takemessage(mout, &msg);
        }
        
    }
    pnode->midiNote.n = 0;
}


static int ports_printed = FALSE;
void print_ports() {
    //do once per run if there are midi nodes in scene
    midiout.close_port();
    midiin.close_port();
    printf("MIDI Output ports:\n");
    std::string portName;
    unsigned int i = 0, nPorts = midiout.get_port_count();
    if (nPorts == 0)
        std::cout << "No ports available!" << std::endl;
    else
    for (i = 0; i < nPorts; i++)
    {
        portName = midiout.get_port_name(i);
        std::cout << "  port #" << i << ": " << portName << '\n';
    }
    printf("MIDI Input ports:\n");
    i = 0, nPorts = midiin.get_port_count();
    if (nPorts == 0)
        std::cout << "No ports available!" << std::endl;
    else
        for (i = 0; i < nPorts; i++)
        {
            portName = midiin.get_port_name(i);
            std::cout << "  port #" << i << ": " << portName << '\n';
        }

    ports_printed = TRUE;
}
MidiNode* midiin_node = NULL;

void midiin_C_callback(const libremidi::message* msg){
//void midiportsourcefunction(const libremidi::message * messin) {
    const struct libremidi::message& messin = *msg;

    MidiNode* mnode = midiin_node;
    std::vector<unsigned char> messout(messin.size());
    auto nBytes = messin.size();
    std::cout << "CB ";
    for (auto i = 0U; i < nBytes; i++)
        std::cout << "Byte " << i << " = " << (int)messin[i] << ", ";
    if (nBytes > 0)
        std::cout << "stamp = " << messin.timestamp << std::endl;
    messout[0] = messin[0];
    messout[1] = messin[1];
    messout[2] = messin[2];
    if (messout[2] > 0 && messout[2] < 64)
        messout[2] = 64;
    libremidi::message msgo = libremidi::message(messout, messin.timestamp);
    //midiout.send_message(messout);
    for (std::list<MidiNode*>::iterator it = mnode->outputs.begin(); it != mnode->outputs.end(); ++it)
    {
        MidiNode* mout = *it;
        //std::cout << "mout->takemessage=" << mout->takemessage << std::endl;
        if (mout->takemessage) mout->takemessage(mout, &msgo);
    }
}
//std::function<void(libremidi::message*)> standard_function(midiin_C_callback);
//libremidi::midi_in::message_callback standard_function(midiin_C_callback);
void libmidi_updateNode3(int icontext, icset connect_parent, struct X3D_Node* node) {
    // midi node type 1=PortSource 2=PortDestination 3=FileSource 4=FileDestination 5=PrintDestination 6=MIDIOut
    struct mcstruct* ac = midi_contexts[icontext];
    //goal- switch-case on x3d nodeType and do any midinode create+connect, update input or update output
    struct X3D_MidiRep* srepn = (struct X3D_MidiRep*)node->_intern;
    if (!ports_printed) print_ports();
    switch (node->_nodeType) {
    case NODE_MIDIPortSource:
    {
        struct X3D_MIDIPortSource* pnode = (struct X3D_MIDIPortSource*)node;
        MidiNode* input;
        if (!srepn->inode) {
            input = new MidiNode();
            input->itype = 1; //1=MIDIPortSource
            input->numberOfOutputs = 0;
            input->numberOfInputs = 1;
            input->takemessage = NULL;
            midiin_node = input;
            midiin.open_port(pnode->port, "libremidi Input");
            //chooseMidiPortIn(midiin);
            printf("opened source port %d\n", pnode->port);
            printf("have a MIDIPortSource node, handling it'n");
            set_midiin_callback();
            //..midiin.set_callback((void*)midiin_C_callback); // standard_function); // midiin_callback);
            /*
            midiin.set_callback(
                [](const libremidi::message& message)
                {
                    std::vector<unsigned char> messout(message.size());
                    auto nBytes = message.size();
                    for (auto i = 0U; i < nBytes; i++)
                        std::cout << "Byte " << i << " = " << (int)message[i] << ", ";
                    if (nBytes > 0)
                        std::cout << "stamp = " << message.timestamp << std::endl;
                    messout[0] = message[0];
                    messout[1] = message[1];
                    messout[2] = message[2];
                    if (messout[2] > 0 && messout[2] < 64)
                        messout[2] = 64;
                    midiout.send_message(messout);

                });
            */
            // Don't ignore sysex, timing, or active sensing messages.
            midiin.ignore_types(false, false, false);

            ac->next_node++;
            ac->nodes[ac->next_node] = input;
            ac->nodetype[ac->next_node] = NODE_MIDIPortSource;
            srepn->inode = ac->next_node;
            srepn->icontext = icontext;
        }

    }
    break;
    case NODE_MIDIPortDestination:
    {
        struct X3D_MIDIPortDestination* pnode = (struct X3D_MIDIPortDestination*)node;
        MidiNode* input;
        if (!srepn->inode) {
            input = new MidiNode();
            input->itype = 2; //MIDIPortDestination
            input->numberOfOutputs = 0;
            input->numberOfInputs = 1;
            input->takemessage = midiPortDestination_takemessage;
            midiout.open_port(pnode->port, "libremidi Output");
            printf("opened destination port %d\n", pnode->port);
            //std::thread portsource(midiportsourcefunction, input);
            //portsource.detach(); //so it doesn't try and join when done

            ac->next_node++;
            ac->nodes[ac->next_node] = input;
            ac->nodetype[ac->next_node] = NODE_MIDIPortDestination;
            srepn->inode = ac->next_node;
            srepn->icontext = icontext;
        }

    }
    break;

    case NODE_MIDIFileSource:
    {
        struct X3D_MIDIFileSource* pnode = (struct X3D_MIDIFileSource*)node;
        MidiNode* input;
        if (!srepn->inode) {
            input = new MidiNode();
            input->itype = 3; //MIDIFileSource
            input->numberOfOutputs = 1;
            input->numberOfInputs = 0;
            input->takemessage = NULL;
            input->loop = FALSE;
            input->run = FALSE;
            // Initialize our reader object
            libremidi::reader *midireader = new libremidi::reader(true); //use abolute? time I think

            // Parse
            libremidi::reader::parse_result result = midireader->parse((uint8_t*)pnode->__blob.p,pnode->__blob.n);

            // If parsing succeeded, use the parsed data
            if (result != libremidi::reader::invalid) {
                input->reader = midireader;
                std::thread filesource(midifilesourcefunction, input);
                filesource.detach(); //so it doesn't try and join when done
                //for (auto& track : r.tracks) {
                //    for (const libremidi::track_event& event : track) {
                //        std::cout << (int)event.m.bytes[0] << '\n';
                //    }
                //}
            }

            ac->next_node++;
            ac->nodes[ac->next_node] = input;
            ac->nodetype[ac->next_node] = NODE_MIDIFileSource;
            srepn->inode = ac->next_node;
            srepn->icontext = icontext;
            //if (iparent.x)
            //    libsound_connect2(icontext, iparent.x, srepn->inode, iparent.y, iparent.z);
        }
        else {
            input = ac->nodes[srepn->inode];
            //printf("setting source node to run\n");
            input->run = TRUE;
            //copy changed values from x3d to libmidi
        }
    }
    break;
    case NODE_MIDIPrintDestination:
    {
        struct X3D_MIDIPrintDestination* pnode = (struct X3D_MIDIPrintDestination*)node;
        MidiNode* input;
        if (!srepn->inode) {
            input = new MidiNode();
            input->itype = 5; //MIDIPrintDestination
            input->numberOfOutputs = 0;
            input->numberOfInputs = 1;
            input->takemessage = midiPrintDestination_takemessage;

            ac->next_node++;
            ac->nodes[ac->next_node] = input;
            ac->nodetype[ac->next_node] = NODE_MIDIPrintDestination;
            srepn->inode = ac->next_node;
            srepn->icontext = icontext;
        }

    }
    break;
    case NODE_MIDIOut:
    {
        struct X3D_MIDIOut* pnode = (struct X3D_MIDIOut*)node;
        MidiNode* input;
        if (!srepn->inode) {
            input = new MidiNode();
            input->itype = 6; //MIDIPrintDestination
            input->numberOfOutputs = 0;
            input->numberOfInputs = 1;
            input->takemessage = midiOut_takemessage;

            ac->next_node++;
            ac->nodes[ac->next_node] = input;
            ac->nodetype[ac->next_node] = NODE_MIDIPrintDestination;
            srepn->inode = ac->next_node;
            srepn->icontext = icontext;
        }
        //update x3d node fields
        input = ac->nodes[srepn->inode];
        //read input queue and convert to MFInt32 midiNote and SFInt32 pedal
        midiOut_message2fields(input, pnode);
    }
    break;
    case NODE_MIDIIn:
    {
        struct X3D_MIDIIn* pnode = (struct X3D_MIDIIn*)node;
        MidiNode* input;
        if (!srepn->inode) {
            input = new MidiNode();
            input->itype = 1; //1=MIDIPortSource
            input->numberOfOutputs = 1;
            input->numberOfInputs = 0;
            input->takemessage = NULL; //it takes a normal ROUTE, not midi messages

            ac->next_node++;
            ac->nodes[ac->next_node] = input;
            ac->nodetype[ac->next_node] = NODE_MIDIIn;
            srepn->inode = ac->next_node;
            srepn->icontext = icontext;
        }
        input = ac->nodes[srepn->inode];
        //convert MFInt32 midiNote to string of messages with this timestamp
        midiin_midinote2messages(input, pnode);

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
void context_disconnect(MidiNode* destination, MidiNode* source, int indexDst, int indexSrc) {
    source->outputs.remove(destination);
    //destination->inputs.remove(source);
}
void context_connect(MidiNode* destination, MidiNode* source, int indexDst, int indexSrc) {
    source->outputs.push_back(destination);
    //destination->inputs.push_back(source);
}
void libmidi_connect2(int icontext, int idestination, int isource, int indexDst, int indexSrc) {
    struct mcstruct* ac = midi_contexts[icontext];
    MidiNode* destination = ac->nodes[idestination];
    MidiNode* source = ac->nodes[isource];
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
    MidiNode* destination = ac->nodes[idestination];
    MidiNode* source = ac->nodes[isource];
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