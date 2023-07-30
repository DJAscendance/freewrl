

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
    bool empty() {
        if (q.empty()) return true;
        return false;
    }
    // Get the "front"-element.
    // If the queue is empty, wait till a element is avaiable.
    T dequeue(void)
    {
        std::unique_lock<std::mutex> lock(m);
        //if (q.empty()) return 0.0; // nullptr;
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
typedef unsigned short ushort;
void midiump_packet2values(double packet, ubyte* channel, ubyte* command, ubyte* note, ushort* velocity);
double TickTime();

typedef struct MidiNode {
    int itype;
    int numberOfInputs;
    int numberOfOutputs;
    //std::list<MidiNode> inputs;
    std::list<MidiNode*> outputs;
    void (*takemessage)(MidiNode*, const struct libremidi::message *);
    void (*takepacket)(MidiNode*, timedpacket packet);
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

// from midi.org MIDI 2.0 Protocol 
//  M2-104-UM_v1-1_UMP_and_MIDI_2-0_Protocol_Specification.pdf 
//  Appendix D: Translation: MIDI 1.0 and MIDI 2.0 Messages
//  July 2023 we don't have Microsoft.Devices.Midi2.Core package available
//  -- so can't build for Midi 2 services (yet, coming eventually)
// but we can translate MIDI 1 port and file messages to UMP universal midi packet format
// and translate back at port destination
// so web3d MIDI processing nodes will be in UMP / Midi2 format, and midi UMP packets will be routed
// scene designers can parse and make UMP packets in SAI javascript 
// -- they'll get a 64 bit SFDouble or SFTime field/event in UMP format
// and when Midi2 services arrive, it will be just the Source and Destination nodes that change.
// 2 possible designs: 
// a) take and make note-specific messages (command, note, velocity)
// b) general message to/from packet conversion (Appendix D)
// below we do the general conversion so we are ready for MIDI2.

// from Appendix D
// Min-Center-Max Upscaling Algorithm
// power of 2, pow(2, exp)
uint32_t power_of_2(uint8_t exp)
{
    return 1 << exp; // implement integer power of 2 using bit shift
}
// preconditions: srcBits > 1, dstBits<=32, srcBits < dstBits
uint32_t scaleUp(uint32_t srcVal, uint8_t srcBits, uint8_t dstBits)
{
    uint8_t scaleBits = (dstBits - srcBits); // number of bits to upscale
    uint32_t srcCenter = power_of_2(srcBits - 1); // center value for srcBits, e.g.
    // 0x40 (64) for 7 bits
   // 0x2000 (8192) for 14 bits 
   // simple bit shift
    uint32_t bitShiftedValue = srcVal << scaleBits;
    if (srcVal <= srcCenter) {
        return bitShiftedValue;
    }
    // expanded bit repeat scheme
    uint8_t repeatBits = srcBits - 1; // we must repeat all but the highest bit
    uint32_t repeatMask = power_of_2(repeatBits) - 1;
    uint32_t repeatValue = srcVal & repeatMask; // repeat bit sequence
    if (scaleBits > repeatBits) { // need to repeat multiple times
        repeatValue <<= (scaleBits - repeatBits);
    }
    else {
        repeatValue >>= (repeatBits - scaleBits);
    }
    while (repeatValue != 0) {
        bitShiftedValue |= repeatValue; // fill lower bits with repeatValue
        repeatValue >>= repeatBits; // move repeat bit sequence to next position
    }
    return bitShiftedValue;
}
// Code for Min - Center - Max Scaling Up from 7 - Bit to 16 - Bit
uint16_t scaleUp7to16(uint8_t value7) {
    uint16_t bitShiftedValue = (uint16_t)value7 << 9;
    if (value7 <= 64) {
        return bitShiftedValue;
    }
    // use bit repeat bits from extended value7
    uint16_t repeatValue6 = (uint16_t)value7 & 0x3F;
    return bitShiftedValue
        | (repeatValue6 << 3)
        | (repeatValue6 >> 3);
}

//typedef union {
//    double packet;
//    unsigned short u16[4];
//    unsigned char bytes[8];
//} UMP;
int msg2ump(int nbytes, const unsigned char* bytes, double* packets) {
    //a long message can produce multiple packets
    //nbytes - number of msg bytes
    //bytes - msg bytes
    //packets - array of UMP packets, please dimension double packets[100] in calling code
    //return - number of packets (usually 1, or 0 if no mapping to midi2, but might be nultple with long message)
    printf("in msg2ump\n");
    UMP ump;
    ump.packet = 0.0;
    int npacket = 0;
    ubyte channel, note, command, velocity7;
    channel = (bytes[0] & 0xF) + 1;
    command = bytes[0] - (channel - 1);
    note = bytes[1];
    velocity7 = bytes[2];
    //code to convert bytes to packets
    //D.3.1 Not On/Off
    if (command == NOTE_ON || command == NOTE_OFF){
        ump.bytes[1] = bytes[0]; //command, channel
        ump.bytes[2] = bytes[1]; //note
        ump.u16[2] = 0;         //velocity
        if (command == NOTE_ON) {
            if(velocity7 == 0)
                ump.bytes[1] = channel | NOTE_OFF;
            else
                ump.u16[2] = scaleUp7to16(bytes[2]);
        }
        packets[npacket] = ump.packet;
        npacket++;
    }
    //D.3.2 PolyPressure
    if (command == POLY_PRESSURE) {
        ump.bytes[1] = bytes[0]; //command, channel
        ump.bytes[2] = bytes[1]; //note
        ump.uint[1] = scaleUp(bytes[2], 7, 32);
        packets[npacket] = ump.packet;
        npacket++;
    }
    //D.3.3 Control Change, RPN, and NRPN
    if (command == CONTROL_CHANGE) {
        ump.bytes[1] = bytes[0];
        ump.bytes[2] = bytes[1];
        ump.uint[1] = scaleUp(bytes[2], 7, 32);
        packets[npacket] = ump.packet;
        npacket++;
        //special case with MSB, LSB values, don't know if I have the right idea
        //would we be getting a long message, or 2 messages? If 2, then buffer
        if (nbytes > 3 && bytes[2] > 0 && bytes[2] < 32 && bytes[3] > 0 && bytes[3] < 64) {
            //send 2nd packet
            ump.bytes[1] = bytes[0];
            ump.bytes[2] = bytes[1];
            ump.uint[1] = scaleUp(bytes[3], 7, 32);
            packets[npacket] = ump.packet;
            npacket++;
        }
    }
    //D.3.4 Program Change and Bank Select
    if (command == PROGRAM_CHANGE) {
        ump.bytes[1] = bytes[0];
        ump.bytes[4] = bytes[1];
        static ubyte bankselect_msb = 0, bankselect_lsb = 0;
        if (bankselect_msb && bankselect_lsb) {
            ump.bytes[6] = bankselect_msb;
            ump.bytes[7] = bankselect_lsb;
        }
        packets[npacket] = ump.packet;
        npacket++;

    }
    //D.3.5 Channel Pressure
    if (command == CHANNEL_PRESSURE) {
        ump.bytes[1] = bytes[0];
        ump.uint[1] = scaleUp(bytes[1], 7, 32);
        packets[npacket] = ump.packet;
        npacket++;
    }
    //D.3.6 Pitch Bend
    if (command == PITCH_BEND) {
        ump.bytes[1] = bytes[0];
        ushort pitchbend = bytes[1] | (bytes[2] << 7);
        ump.uint[1] = scaleUp(pitchbend, 14, 32);
        packets[npacket] = ump.packet;
        npacket++;
    }
    //D.3.7 System Messages
    if (command == SYSTEM_EXCLUSIVE) {
        int mbytes = channel; 
        ump.bytes[1] = bytes[0]; //should the top bit be zeroed? Or whole thing?
        
        int j = 0;
        if (mbytes <= 6) {
            for (int i = 0; i < mbytes; i++) {
                if (!(bytes[i + 1] == 0xF0 || bytes[i + 1] == 0xF7)) {
                    ump.bytes[2 + j] = bytes[1 + i];
                    j++;
                }
            }
        }
        else {
            //break into multiple packets

        }
        //ump.bytes[1] = command | j; //Am I supposed to change the count in the status? or do they not count sthe start and end bytes either
        //what if more bytes than 6? does that ever happen? If so what then?
        packets[npacket] = ump.packet;
        npacket++;
    }
    //memcpy(ump.bytes, bytes, nbytes); //delivers midi1 messages in first 3 bytes
    packets[0] = ump.packet;
    return 1;
}
// Code for Downscaling Algorithm
uint32_t scaleDown(uint32_t srcVal, uint8_t srcBits, uint8_t dstBits) {
    // simple bit shift
    uint8_t scaleBits = (srcBits - dstBits);
    return srcVal >> scaleBits;
}

int ump2msg(double packet, ubyte **msg, int *nbytes, ubyte* bytearray) {
    //chained packets can produce a single long message
    //ump - packet
    //bytes - msg bytes - please dimension unsigned char bytes[200] in calling code
    //return: number of msg bytes (might be 0 for start of a long message) 
    // this function will buffer packets till it gets the last of chained packets
    //static unsigned char ubytes[200];
    //static int nbytes = 0; //one-time initializatino
    printf("in ump2msg\n");
    ubyte command, channel, note, mt, group, * bytes;
    ushort velocity;
    UMP ump;
    ump.packet = packet;
    int nmsg = 0;
    bytes = bytearray;
    midiump_packet2values(packet, &channel, &command, &note, &velocity);
    channel--;
    mt = ump.bytes[0] >> 4;
    group = ump.bytes[0] & (0xF >> 4);
    //code to convert ump to bytes
    if (command >= 0x10 && command <= 0x30) {
        //D.2.6 System Messages
        //D.2.7 System Exclusive
        static ubyte cumbytes[100];
        static int ncum = 0; //initialize once
        int nb = channel;
        if (ncum == 0 || command == 0x10) {
            //start new sysex command
            cumbytes[0] = SYSTEM_EXCLUSIVE; // = 0xF0
            ncum++;
        }
        for (int i = 0; i < channel; i++) {
            cumbytes[ncum] = ump.bytes[i + 2];
            ncum++;
        }
        if (command == 0x30) {
            //end sysex message
            cumbytes[ncum] = EOX; //end of transmission byte
            ncum++;
            memcpy(bytes, cumbytes, ncum);
            msg[nmsg] = bytes;
            nbytes[nmsg] = ncum;
            bytes += ncum;
            nmsg++;
        }
    }
    else { 
        //not a system exclusive
        switch (command) {
            //D.2.1 Not On/Off, Poly Pressure, Control Change
        case NOTE_ON:
        case NOTE_OFF:
        case POLY_PRESSURE:
        case CONTROL_CHANGE:
        {
            memset(bytes, 0, 3);
            bytes[0] = command | channel | 1 << 7; //is the top bit still set?
            bytes[1] = ump.bytes[2];
            bytes[2] = (ubyte)scaleDown(ump.bytes[4], 8, 7);
            if (command == NOTE_ON && bytes[2] == 0) bytes[2] = 0x01; //minimum note on velocity
            msg[nmsg] = bytes;
            nbytes[nmsg] = 3;
            bytes += 3;
            nmsg++;
        }
        break;
        //D.2.2 Channel Pressure
        case CHANNEL_PRESSURE:
        {
            memset(bytes, 0, 2);
            bytes[0] = command | channel | 1 << 7; //is the top bit still set?
            bytes[1] = (ubyte)scaleDown(ump.bytes[4], 8, 7);
            msg[nmsg] = bytes;
            nbytes[nmsg] = 2;
            bytes += 2;
            nmsg++;
        }
        break;
        //D.2.3 Assignable Controllers (NRPN) and Registered Controllers (RPN)
        //D.2.4 Program Change and Bank Select
        case PROGRAM_CHANGE:
        {
            if (ump.bytes[3] & 1) {
                //bank select MSB
                memset(bytes, 0, 3);
                bytes[0] = command | channel | 1 << 7; //is the top bit still set?
                bytes[2] = ump.bytes[6] & (0xF >> 1); //make sure top bit clear
                msg[nmsg] = bytes;
                nbytes[nmsg] = 3;
                bytes += 3;
                nmsg++;
                //bank select LSB
                memset(bytes, 0, 3);
                bytes[0] = command | channel | 1 << 7; //is the top bit still set?
                bytes[2] = ump.bytes[7] & (0xF >> 1); //make sure top bit clear
                msg[nmsg] = bytes;
                nbytes[nmsg] = 3;
                bytes += 3;
                nmsg++;

            }
            //unconditional program change
            memset(bytes, 0, 2);
            bytes[0] = ump.bytes[1]; // command | channel | 1 << 7; //is the top bit still set?
            bytes[1] = ump.bytes[4]; // &(0xF >> 1); //make sure top bit clear
            msg[nmsg] = bytes;
            nbytes[nmsg] = 2;
            bytes += 2;
            nmsg++;
        }
        break;
        case PITCH_BEND:
        {
            //D.2.5 Pitch Bend
            memset(bytes, 0, 3);
            bytes[0] = command | channel | 1 << 7; //is the top bit still set?
            bytes[1] = (ubyte)scaleDown(ump.bytes[5], 8, 6) | ((ump.bytes[4] & 1) << 6);
            bytes[2] = (ubyte)scaleDown(ump.bytes[4], 8, 7);
            msg[nmsg] = bytes;
            nbytes[nmsg] = 3;
            bytes += 3;
            nmsg++;
        }
        break;
        //D.2.8 non-translatable to MIDI 1
        //D.2.9 non-translatable to non-UMP MIDI 1
        } //end switch
    } //end else sysex
    return nmsg;

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
                    libremidi::message msg = event.m;
                    double packets[10];
                    int npackets;
                    if(MIDITransport() == 2)
                        npackets = msg2ump(msg.bytes.size(), msg.bytes.data(), packets);

                    std::wcout << "outputs.count" << mnode->outputs.size() << std::endl;
                    for (std::list<MidiNode*>::iterator it = mnode->outputs.begin(); it != mnode->outputs.end(); ++it)
                    {
                        MidiNode* mout = *it;
                        //std::cout << "mout->takemessage=" << mout->takemessage << std::endl;
                        if (MIDITransport() == MIDI_UMP) {
                            //MIDI 2.0 UMP
                            for (int j = 0; j < npackets; j++) {
                                timedpacket tpacket;
                                tpacket.packet = packets[j];
                                tpacket.timestamp = msg.timestamp;
                                if (mout->takepacket) mout->takepacket(mout, tpacket);
                            }
                        }
                        else {
                            //MIDI 1.0 MIDI_MSG
                            if (mout->takemessage) mout->takemessage(mout, &msg);
                        }
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
void midiPortDestination_takepacket(MidiNode* midiNode, timedpacket tpacket) {
    //convert from UMP MIDI 2.0 to MIDI 1 message and send to output port
    ubyte bytearray[200];
    ubyte *msgs[50];
    int nbytes[50];
    double packet = tpacket.packet;
    int nmsg = ump2msg(packet, msgs, nbytes, bytearray); 
    for(int j=0;j<nmsg;j++) {
        ubyte* bytes = msgs[j];
        int nbyte = nbytes[j];
        std::vector<unsigned char> messout(nbyte);
        for (int i = 0; i < nbyte; i++)
            messout[i] = bytes[i];
        libremidi::message mout = libremidi::message(messout, tpacket.timestamp);
        midiout.send_message(mout);
    }
}
void midiPrintDestination_takemessage(MidiNode* midiNode, const struct libremidi::message * msg) {
    const struct libremidi::message& m = *msg;
    printf("in midiPrintDestination_takemessage\n");
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
void midiPrintDestination_takepacket(MidiNode* midiNode, timedpacket tpacket) {
    ubyte bytearray[200];
    ubyte* msgs[50];
    int nbytes[50];
    double packet = tpacket.packet;
    printf("in midiPrintDestination_takepacket\n");
    int nmsg = ump2msg(packet, msgs, nbytes, bytearray);
    for(int j=0;j<nmsg;j++) {
        ubyte *bytes = msgs[j];
        int nbyte = nbytes[j];
        std::vector<unsigned char> messout(nbyte);
        for (int i = 0; i < nbyte; i++)
            messout[i] = bytes[i];
        libremidi::message mout = libremidi::message(messout, tpacket.timestamp);
        midiPrintDestination_takemessage(midiNode, &mout);
    }
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
void midiOut_takepacket(MidiNode* midiNode, timedpacket tpacket) {
    if (!midiNode->queue)
        midiNode->queue = (void*) new SafeQueue<timedpacket>();
    SafeQueue<timedpacket>* que = (SafeQueue<timedpacket>*)midiNode->queue;
    std::cout << "enqueuing one" << std::endl;
    que->enqueue(tpacket);
    std::cout << "enqueued one" << std::endl;
}

void midimsg_uint2values(unsigned int msg, ubyte* channel, ubyte* command, ubyte* note, ubyte* velocity);
unsigned int midimsg_values2uint(ubyte channel, ubyte command, ubyte note, ubyte velocity);
void midiump_packet2values(double packet, ubyte* channel, ubyte* command, ubyte* note, ushort* velocity);
double midiump_values2packet(ubyte channel, ubyte command, ubyte note, ushort velocity);

void mark_event(struct X3D_Node* from, int totalptr);
//#define MARK_EVENT(node,offset)	mark_event(X3D_NODE(node),(int) offset)
void midiOut_message2fields(MidiNode* midiNode, struct X3D_MIDIOut* node) {
    // dequeues direct midi messages and converts to MFInt32 outputOnly midiMsg field entries
    struct X3D_Node* anode = X3D_NODE(node);
    const struct libremidi::message *msg;
    if (!midiNode->queue)
        midiNode->queue = (void*) new SafeQueue<const struct libremidi::message*>();
    SafeQueue<const struct libremidi::message*>* que = (SafeQueue<const struct libremidi::message*>*)midiNode->queue;
    Multi_Int32* last = &node->midiMsg;
    unsigned int cur[1000], n = 0;
    //int pedal = FALSE;
    int mark = FALSE;
    //std::cout << "starting dequeue loop" << std::endl;

    while (!que->empty()) { //msg = que->dequeue()
        msg = que->dequeue();
        std::cout << "dequed one" << std::endl;
        
        const struct libremidi::message& m = *msg;
        switch (m.get_message_type())
        {
        case libremidi::message_type::NOTE_ON:
            std::cout << "Note ON: "
                << "channel " << m.get_channel() << ' '
                << "note " << (int)m.bytes[1] << ' '
                << "velocity " << (int)m.bytes[2] << ' ';
            //cur[n] = (int)m.bytes[2] > 0 ? (int)m.bytes[1] : -(int)m.bytes[1];
            cur[n] = midimsg_values2uint(m.get_channel(), (ubyte)m.get_message_type(), m.bytes[1], m.bytes[2]);
            n++;
            break;
        case libremidi::message_type::NOTE_OFF:
            std::cout << "Note OFF: "
                << "channel " << m.get_channel() << ' '
                << "note " << (int)m.bytes[1] << ' '
                << "velocity " << (int)m.bytes[2] << ' ';
            //cur[n] = -(int)m.bytes[1]; //negative sign for OFF, + for ON
            cur[n] = midimsg_values2uint(m.get_channel(), (ubyte)m.get_message_type(), m.bytes[1], m.bytes[2]);
            n++;
            break;
        case libremidi::message_type::CONTROL_CHANGE:
            std::cout << "Control: "
                << "channel " << m.get_channel() << ' '
                << "control " << (int)m.bytes[1] << ' '
                << "value " << (int)m.bytes[2] << ' ';
            //pedal = node->pedal;
            //node->pedal = m.bytes[2] > 0 ? TRUE : FALSE;
            //if(pedal != node->pedal) MARK_EVENT(anode, offsetof(struct X3D_MIDIOut, pedal));
            cur[n] = midimsg_values2uint(m.get_channel(), (ubyte)m.get_message_type(), m.bytes[1], m.bytes[2]);
            n++;
            break;
        default:
            break;
        }
        
       //I don't have a destructor,memory use will escalate ~msg();
    }
    //std::cout << "ended dequeue loop" << std::endl;

    if (n) {
        node->midiMsg.p = (int *)realloc(node->midiMsg.p, n * sizeof(int));
        memcpy(node->midiMsg.p, cur, n * sizeof(int));
        node->midiMsg.n = n;
        MARK_EVENT(anode, offsetof(struct X3D_MIDIOut, midiMsg));
    }
    else {
        node->midiMsg.n = 0;
    }
    //std::cout << "finished midiOut render" << std::endl;

}

void midiOut_packet2fields(MidiNode* midiNode, struct X3D_MIDIOut* node) {
    // dequeues direct midi messages and converts to MFInt32 outputOnly midiMsg field entries
    struct X3D_Node* anode = X3D_NODE(node);
    if (!midiNode->queue)
        midiNode->queue = (void*) new SafeQueue<timedpacket>();
    SafeQueue<timedpacket>* que = (SafeQueue<timedpacket>*)midiNode->queue;
    Multi_Double* last = &node->midiUmp;
    double cur[1000];
    int n = 0;
    int mark = FALSE;
    //std::cout << "starting dequeue loop" << std::endl;
    UMP ump;
    timedpacket tpacket;
    while (!que->empty()) {
        tpacket = que->dequeue();
        ump.packet = tpacket.packet;
        std::cout << "dequed one" << std::endl;
        cur[n] = ump.packet;
        n = n >= 999 ? 999 : n + 1; //we'll drop packets if we get flooded.
    }
    if (n) {
        node->midiUmp.p = (double*)realloc(node->midiUmp.p, n * sizeof(double));
        memcpy(node->midiUmp.p, cur, n * sizeof(double));
        node->midiUmp.n = n;
        MARK_EVENT(anode, offsetof(struct X3D_MIDIOut, midiUmp));
    }
    else {
        node->midiUmp.n = 0;
    }
    //std::cout << "finished midiOut render" << std::endl;

}
double TickTime();
void midiin_midinote2messages(MidiNode* mnode, struct X3D_MIDIIn* pnode) {
    static double lasttime = 0.0;
    if (lasttime == 0.0) lasttime = TickTime();
    for (int i = 0; i < pnode->midiMsg.n; i++) {
        //libremidi::message *msg = new libremidi:message()
        ubyte channel, command, note, velocity;
        midimsg_uint2values(pnode->midiMsg.p[i], &channel, &command, &note, &velocity);
        //unsigned char inote = abs(pnode->midiMsg.p[i]);
        //unsigned char velocity = pnode->midiMsg.p[i] > 0 ? 64 : 0;
        //std::vector<unsigned char> messout(3);
        //messout[0] = 144; // 176; //its a note
        //messout[1] = inote;
        //messout[2] = on;
        double now = TickTime();
        double timestamp = now - lasttime;
        lasttime = now;
        libremidi::message msg; // = libremidi::message(messout, timestamp);
        std::vector<unsigned char> messout(3);
        messout[0] = command | channel;
        messout[1] = note;
        messout[2] = velocity;
        msg = libremidi::message(messout, timestamp);
        printf("MI %d %d %d %lf", messout[0], messout[1], messout[2], timestamp);
 /*       if (command == (ubyte)libremidi::message_type::NOTE_ON || command == (ubyte)libremidi::message_type::NOTE_OFF)
        {
            if (velocity)
                msg = libremidi::message::note_on(1, note, velocity);
            else
                msg = libremidi::message::note_off(1, note, velocity);
        }*/
        for (std::list<MidiNode*>::iterator it = mnode->outputs.begin(); it != mnode->outputs.end(); ++it)
        {
            MidiNode* mout = *it;
            //std::cout << "mout->takemessage=" << mout->takemessage << std::endl;
            if (mout->takemessage) mout->takemessage(mout, &msg);
        }
        
    }
    pnode->midiMsg.n = 0;
}
void midiin_midinote2packets(MidiNode* mnode, struct X3D_MIDIIn* pnode) {
    static double lasttime = 0.0;
    if (lasttime == 0.0) lasttime = TickTime();
    for (int i = 0; i < pnode->midiUmp.n; i++) {
        double packet = pnode->midiMsg.p[i];
        double now = TickTime();
        double timestamp = now - lasttime;
        lasttime = now;
        timedpacket tpacket;
        tpacket.packet = packet;
        tpacket.timestamp = timestamp;
        for (std::list<MidiNode*>::iterator it = mnode->outputs.begin(); it != mnode->outputs.end(); ++it)
        {
            MidiNode* mout = *it;
            if (mout->takepacket) mout->takepacket(mout, tpacket);
        }
    }
    pnode->midiMsg.n = 0;
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
    //called by libremidi when there's port input event
    const struct libremidi::message& messin = *msg;

    MidiNode* mnode = midiin_node;
    auto nBytes = messin.size();
    if (MIDITransport() == MIDI_UMP) {
        // MIDI 2.0 64 bit UMP packet transport
        double packets[100];
        const libremidi::midi_bytes bytes[200];
        double npacket = msg2ump(nBytes, messin.bytes.data(), packets); // messin.timestamp);
        if (npacket) {
            for (std::list<MidiNode*>::iterator it = mnode->outputs.begin(); it != mnode->outputs.end(); ++it)
            {
                MidiNode* mout = *it;
                if (mout->takepacket)
                    for (int i = 0; i < npacket; i++) {
                        timedpacket tpacket;
                        tpacket.packet = packets[i];
                        tpacket.timestamp = messin.timestamp;
                        mout->takepacket(mout, tpacket);
                    }
            }
        }
    }
    else if(MIDITransport() == MIDI_MSG) {
        //MIDI 1 byte stream message transport
        std::vector<unsigned char> messout(messin.size());

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
            input->takepacket = midiPortDestination_takepacket;
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
            printf("updateNod3 for FileSource");
            // If parsing succeeded, use the parsed data
            if (result != libremidi::reader::invalid) {
                input->reader = midireader;
                printf("starting filesource thread\n");
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
            input->takepacket = midiPrintDestination_takepacket;
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
            input->takepacket = midiOut_takepacket;

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
            input->takepacket = NULL;

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