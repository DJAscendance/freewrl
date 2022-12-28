
// license: MIT or equivalent permissive
//
#include "LabSound.h"
#include <algorithm>
#include <array>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <map>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <vector>
#include <iostream>
#include <string>

// In the future, this class could do all kinds of clever things, like setting up the context,
// handling recording functionality, etc.

#include <string>
#include <vector>

using namespace lab;


// Returns input, output
inline std::pair<AudioStreamConfig, AudioStreamConfig> GetDefaultAudioDeviceConfiguration(const bool with_input = false)
{
    AudioStreamConfig inputConfig;
    AudioStreamConfig outputConfig;

    const std::vector<AudioDeviceInfo> audioDevices = lab::MakeAudioDeviceList();
    const uint32_t default_output_device = lab::GetDefaultOutputAudioDeviceIndex();
    const uint32_t default_input_device = lab::GetDefaultInputAudioDeviceIndex();

    AudioDeviceInfo defaultOutputInfo, defaultInputInfo;
    for (auto& info : audioDevices)
    {
        if (info.index == default_output_device) defaultOutputInfo = info;
        else if (info.index == default_input_device) defaultInputInfo = info;
    }

    if (defaultOutputInfo.index != -1)
    {
        outputConfig.device_index = defaultOutputInfo.index;
        outputConfig.desired_channels = std::min(uint32_t(2), defaultOutputInfo.num_output_channels);
        outputConfig.desired_samplerate = defaultOutputInfo.nominal_samplerate;
    }

    if (with_input)
    {
        if (defaultInputInfo.index != -1)
        {
            inputConfig.device_index = defaultInputInfo.index;
            inputConfig.desired_channels = std::min(uint32_t(1), defaultInputInfo.num_input_channels);
            inputConfig.desired_samplerate = defaultInputInfo.nominal_samplerate;
        }
        else
        {
            throw std::invalid_argument("the default audio input device was requested but none were found");
        }
    }

    return { inputConfig, outputConfig };
}
inline std::vector<std::string> SplitCommandLine(int argc, char** argv)
{
    // takes a string, and separates out according to embedded quoted strings
    // the quotes are preserved, and quotes are escaped.
    // examples
    // * abc > abc
    // * abc "def" > abc, "def"
    // * a "def" ghi > a, "def", ghi
    // * a\"bc > a\"bc

    auto Separate = [](const std::string& input) -> std::vector<std::string>
    {
        std::vector<std::string> output;

        size_t curr = 0;
        size_t start = 0;
        size_t end = input.length();
        bool inQuotes = false;

        while (curr < end)
        {
            if (input[curr] == '\\')
            {
                ++curr;
                if (curr != end && input[curr] == '\"')
                    ++curr;
            }
            else
            {
                if (input[curr] == '\"')
                {
                    // no empty string if not in quotes, otherwise preserve it
                    if (inQuotes || (start != curr))
                    {
                        output.push_back(input.substr(start - (inQuotes ? 1 : 0), curr - start + (inQuotes ? 2 : 0)));
                    }
                    inQuotes = !inQuotes;
                    start = curr + 1;
                }
                ++curr;
            }
        }

        // catch the case of a trailing substring that was not quoted, or a completely unquoted string
        if (curr - start > 0) output.push_back(input.substr(start, curr - start));

        return output;
    };

    // join the command line together so quoted strings can be found
    std::string cmd;
    for (int i = 1; i < argc; ++i)
    {
        if (i > 1) cmd += " ";
        cmd += std::string(argv[i]);
    }

    // separate the command line, respecting quoted strings
    std::vector<std::string> result = Separate(cmd);
    result.insert(result.begin(), std::string{ argv[0] });
    return result;
}

inline std::shared_ptr<AudioBus> MakeBusFromSampleFile(char const* const name, int argc, char** argv)
{
    std::string path_prefix;
    auto cmds = SplitCommandLine(argc, argv);

    if (cmds.size() > 1) path_prefix = cmds[1] + "/";  // cmds[0] is the path to the exe

    const std::string path = path_prefix + name;
    std::shared_ptr<AudioBus> bus = MakeBusFromFile(path, false);
    if (!bus) throw std::runtime_error("couldn't open " + path);

    return bus;
}



template <typename Duration>
void Wait(Duration duration)
{
    std::this_thread::sleep_for(duration);
}


//make the interface flat C
#ifdef __cplusplus
extern "C" {
#endif
#define GLDOUBLE double
typedef struct {
    void* p;                   /* Pointer to actual object */
    unsigned int x;             /* Extra information - reuse count etc */
} ptw32_handle_t;

typedef ptw32_handle_t pthread_t;
#include "../lib/vrml_parser/Structs.h"

#include "libsound.h"
    static void* busbuffers[30];
    static int n_busbuffers = 0;

    void libsound_testNoise()
    {
        std::unique_ptr<lab::AudioContext> context;
         const auto defaultAudioDeviceConfigurations = GetDefaultAudioDeviceConfiguration();
        context = lab::MakeRealtimeAudioContext(defaultAudioDeviceConfigurations.second, defaultAudioDeviceConfigurations.first);

        //auto musicClip = MakeBusFromSampleFile("samples/stereo-music-clip.wav", argc, argv);
        const std::string path = "C:/Users/Public/dev/source5/audio/LabSound-master/assets/samples/stereo-music-clip.wav";
        std::shared_ptr<AudioBus> bus = MakeBusFromFile(path, false);
        auto musicClip = bus;
        if (!musicClip)
            return;

        std::shared_ptr<OscillatorNode> oscillator;
        std::shared_ptr<SampledAudioNode> musicClipNode;
        std::shared_ptr<GainNode> gain;

        oscillator = std::make_shared<OscillatorNode>(context->sampleRate());
        gain = std::make_shared<GainNode>();
        gain->gain()->setValue(0.0625f);

        musicClipNode = std::make_shared<SampledAudioNode>();
        {
            ContextRenderLock r(context.get(), "ex_simple");
            musicClipNode->setBus(r, musicClip);
        }
        context->connect(gain, musicClipNode, 0, 0);
        musicClipNode->start(0.0f);

        // osc -> gain -> destination
        context->connect(gain, oscillator, 0, 0);
        context->connect(context->device(), gain, 0, 0);

        oscillator->frequency()->setValue(440.f);
        oscillator->setType(OscillatorType::SINE);
        oscillator->start(0.0f);

        Wait(std::chrono::seconds(6));
 
    }
    struct variant_record {
        int type; //see libsound.h for enum AN_AudioClip 1, AN_AudioBuffer 2, ...
        void* value;
    };
    struct per_context_stuff {
        lab::AudioContext *context;
        std::vector < variant_record > nodes;
    };
    static std::vector<per_context_stuff> active_contexts;
    void* libsound_createContext()
    {
        std::shared_ptr<lab::AudioContext> context;
        lab::AudioContext *ccontext;
        const auto defaultAudioDeviceConfigurations = GetDefaultAudioDeviceConfiguration();
        context = lab::MakeRealtimeAudioContext(defaultAudioDeviceConfigurations.second, defaultAudioDeviceConfigurations.first);
        ccontext = context.get(); //gimme the raw context pointer from smart pointer 
        //context.release(); //and dont garbage collect the context when we go out of scope, C taking ownership
        return (void*)ccontext;
    }
    void* libsound_createContext2()
    {
        std::unique_ptr<lab::AudioContext> context;
        lab::AudioContext* ccontext;
        const auto defaultAudioDeviceConfigurations = GetDefaultAudioDeviceConfiguration();
        context = lab::MakeRealtimeAudioContext(defaultAudioDeviceConfigurations.second, defaultAudioDeviceConfigurations.first);
        per_context_stuff stuff;
        stuff.context = context.get(); //do context.release() ownership below so doesn't go out of scope.

        /*
        active_contexts.push_back(stuff);
        
        //auto musicClip = MakeBusFromSampleFile("samples/stereo-music-clip.wav", argc, argv);
        {
        */
            const std::string path = "C:/Users/Public/dev/source5/audio/LabSound-master/assets/samples/stereo-music-clip.wav";
            std::shared_ptr<AudioBus> bus = MakeBusFromFile(path, false);
        /*
            variant_record vr;
            vr.type = AN_AudioClip;
            //vr.value = bus.get(); //static_cast<void*>(&bus)
            //bus.use_count++; //tell it a non-shared_ptr pointer copy wants to keep it alive
            vr.value = &bus;
            stuff.nodes.push_back(vr);
        }
        variant_record vr2 = stuff.nodes.back();
        void* val = vr2.value;
        //lab::AudioBus *bus0 = static_cast<lab::AudioBus*>(val);
        std::shared_ptr<lab::AudioBus> bus = static_cast<std::shared_ptr<lab::AudioBus>>(val);
        */
        //int index = stuff.nodes.at(.countsize() - 1;
        auto musicClip = bus;
        if (!musicClip)
            return NULL;

        std::shared_ptr<OscillatorNode> oscillator;
        std::shared_ptr<SampledAudioNode> musicClipNode;
        std::shared_ptr<GainNode> gain;

        oscillator = std::make_shared<OscillatorNode>(context->sampleRate());
        gain = std::make_shared<GainNode>();
        gain->gain()->setValue(0.0625f);

        musicClipNode = std::make_shared<SampledAudioNode>();
        {
            ContextRenderLock r(context.get(), "ex_simple");
            musicClipNode->setBus(r, musicClip);
        }
        lab::GainNode *gain2 = gain.get();
        gain = nullptr;
        gain.reset(gain2);

        context->connect(gain, musicClipNode, 0, 0);
        musicClipNode->start(0.0f);

        // osc -> gain -> destination
        context->connect(gain, oscillator, 0, 0);
        context->connect(context->device(), gain, 0, 0);

        oscillator->frequency()->setValue(440.f);
        oscillator->setType(OscillatorType::SINE);
        oscillator->start(0.0f);

        Wait(std::chrono::seconds(6));
        ccontext = context.get(); //gimme the raw context pointer from smart pointer 
        context.release(); //and dont garbage collect the context when we go out of scope, C taking ownership

        return (void*)ccontext;

    }
    void* libsound_createNode(void *ccontext, int type) {
        void* node = NULL;
        lab::AudioContext *context = (lab::AudioContext * )ccontext;
        switch (type) {
        case AN_AudioClip:
        {
            //auto musicClip = MakeBusFromSampleFile("samples/stereo-music-clip.wav", argc, argv);
            std::string path = "C:/Users/Public/dev/source5/audio/LabSound-master/assets/samples/stereo-music-clip.wav";
            AudioBus* bus = MakeBusFromFile(path, false).get();
            auto musicClip = bus;
            if (musicClip)
                node = (void*)bus;
        }
        break;
        case AN_AudioBuffer:
        break;
        case AN_AudioBufferSourceNode:
        {
            SampledAudioNode* musicClipNode;
            ContextRenderLock r(context, "ex_simple");
            //musicClipNode->setBus(r, musicClip);
        }
        break;
        case AN_GainNode:
        {
            GainNode* gain = new GainNode();
            gain->gain()->setValue(0.0625f);
            node = (void*)gain;
        }
        break;
        case AN_OscillatorNode:
        {
            OscillatorNode* oscillator;
            oscillator = new OscillatorNode(context->sampleRate());
            node = (void*)oscillator;
        }
        break;
        case AN_AudioDestinationNode:
        break;
        default:
        break;
        }
        return node;
    }

    //attempt 4
    struct anstruct { std::shared_ptr<lab::AudioNode> anode; };
    struct acstruct {
        lab::AudioContext* context;
        int next_node;
        int next_bus;
        std::map<int, std::shared_ptr<lab::AudioNode>> nodes;
        std::map<int, std::shared_ptr<lab::AudioBus>> busses;
    };
    static int next_audio_context;
    static std::map<int, struct acstruct*> audio_contexts;
    int libsound_createContextData() {
        struct acstruct *ac = new acstruct();
        ac->context = static_cast<lab::AudioContext*>(libsound_createContext());
        next_audio_context++;
        audio_contexts[next_audio_context] = ac;
        return next_audio_context;
    }
    //this one uses int index lookup in a map, much like a vector except can deleted elements
    void libsound_connect0(int icontext, int idestination, int isource) {
        struct acstruct* ac = audio_contexts[icontext];
        std::shared_ptr<AudioNode> destination = ac->nodes[idestination];
        std::shared_ptr<AudioNode> source = ac->nodes[isource];
        ac->context->connect(destination, source);
    }
    int libsound_createBusFromBuffer0(int icontext, char* bbuffer, int len) {
        struct acstruct* ac = audio_contexts[icontext];
        std::vector<uint8_t> buffer(bbuffer, bbuffer + len); // , (uint8_t)bbuffer);
        std::shared_ptr<AudioBus> Bus = MakeBusFromMemory(buffer, false);
        ac->next_bus++;
        ac->busses[ac->next_bus] = Bus;
        return ac->next_bus;
    }
    struct X3D_SoundRep* getSoundRep(struct X3D_Node* pnode) {
        //main benefit of _intern Rep structure: saves switch-casing on _NodeType 
        // to get specific common fields used for internal processing only
        // -- just put the common fields in the Rep
        // in our case it would be our lookup table int, maybe some AudioNode fields related to connecting, starting, stopping
        struct X3D_SoundRep* srep = NULL;
        if (pnode) {
            srep = (struct X3D_SoundRep*)pnode->_intern;
            if (!srep) {
                srep = (struct X3D_SoundRep*)malloc(sizeof(struct X3D_SoundRep));
                memset(srep, 0, sizeof(struct X3D_SoundRep));
                srep->itype = 7; //SoundRep
                pnode->_intern = (struct X3D_GeomRep*)srep;
            }
        }
        return srep;
    }
    void libsound_updateNode0(int icontext, struct X3D_Node* connect_parent, struct X3D_Node* node) {
        struct acstruct* ac = audio_contexts[icontext];
        //goal- switch-case on x3d nodeType and do any labsound node create+connect, update input or update output
        // - then this can be called from 
        struct X3D_SoundRep* srepp = getSoundRep(connect_parent);
        struct X3D_SoundRep* srepn = getSoundRep(node);
        switch (node->_nodeType) {
        case NODE_OscillatorSource:
        {
            struct X3D_OscillatorSource* pnode = (struct X3D_OscillatorSource*)node;
            //if (!pnode->_self) {
            std::shared_ptr<OscillatorNode> oscillator;
            OscillatorNode *oscillator_ptr;
            if(!srepn->inode){
                //create labsound node
                oscillator = std::make_shared<OscillatorNode>(ac->context->sampleRate());
                oscillator_ptr = oscillator.get();
                //oscillator->detune( pnode->detune);
                ac->next_node++;
                ac->nodes[ac->next_node] = oscillator; // std::shared_ptr<AudioNode>(oscillator);
                //pnode->_self = (void*)(uint64_t)ac->next_node;
                srepn->inode = ac->next_node;
                srepn->icontext = icontext;
                //connect source node output to parent node input
                //libsound_connect0(icontext, connect_parent->_self, pnode->_self);
                if(srepp)
                    libsound_connect0(icontext, srepp->inode, srepn->inode);
            }
            //oscillator = static_cast<std::shared_ptr<OscillatorNode>>(ac->nodes[srepn->inode]);
            oscillator_ptr = dynamic_cast<OscillatorNode*>(ac->nodes[srepn->inode].get());
            //copy changed values from x3d to labsound
            oscillator_ptr->detune()->setValue(pnode->detune);

            //copy outputs from labsound to x3d
            //pnode->elapsedTime = oscillator_ptr->param("time")->value(;
        }
        break;
        case NODE_Gain:
        {
            struct X3D_Gain* pnode = (struct X3D_Gain*)node;
            std::shared_ptr<GainNode> gain;
            GainNode* gain_ptr;
            if (!srepn->inode) {
                //create labsound node
                gain = std::make_shared<GainNode>();
                gain_ptr = gain.get();
                ac->next_node++;
                ac->nodes[ac->next_node] = gain;
                srepn->inode = ac->next_node;
                srepn->icontext = icontext;
                //connect source node output to parent node input
                if (srepp)
                    libsound_connect0(icontext, srepp->inode, srepn->inode);
            }
            gain_ptr = dynamic_cast<GainNode*>(ac->nodes[srepn->inode].get());
            //copy changed values from x3d to labsound
            gain_ptr->gain()->setValue(pnode->gain);

            //copy outputs from labsound to x3d

        }
        break;
        default:
            return;

        }
    }

    void libsound_connect(void* ccontext, void * cdestination, void * csource) {
        lab::AudioContext* context = (lab::AudioContext*)ccontext;
        struct anstruct* destination = (struct anstruct*)cdestination;
        struct anstruct* source = (struct anstruct*)csource;
        // even if I pass around a shared_ptr, how will it know its type
        //    - don't the incoming paramters need to be strongly typed?
        // - C++ needs to recover the vtable of the derived type
        // -- either drag around an int itype and switch-case to cast to appropriate derived class
        // -- or keep the vtable-aware structs/variables alive.
        //std::shared_ptr < lab::AudioNode> destination = static_cast<lab::AudioNode>(cdestination);
        //std::shared_ptr < lab::AudioNode> source = std::make_shared<lab::AudioNode>(csource);
        //std::shared_ptr < lab::AudioNode> destination = std::make_shared<lab::AudioNode>(cdestination);

       context->connect(destination->anode,source->anode);

    }
    typedef std::shared_ptr<lab::AudioNode> anode; //but how to persist a local shared_ptr so not itself garbage collected?
    void libsound_connect3(void* ccontext, void* cdestination, void* csource) {
        lab::AudioContext* context = (lab::AudioContext*)ccontext;
        anode *destination = (anode*)cdestination;
        anode* source = (anode*)csource;
        // even if I pass around a shared_ptr, how will it know its type
        //    - don't the incoming paramters need to be strongly typed?
        // - C++ needs to recover the vtable of the derived type
        // -- either drag around an int itype and switch-case to cast to appropriate derived class
        // -- or keep the vtable-aware structs/variables alive.
        //std::shared_ptr < lab::AudioNode> destination = static_cast<lab::AudioNode>(cdestination);
        //std::shared_ptr < lab::AudioNode> source = std::make_shared<lab::AudioNode>(csource);
        //std::shared_ptr < lab::AudioNode> destination = std::make_shared<lab::AudioNode>(cdestination);

        context->connect(*(destination), *source);

    }


    int libsound_createBusFromBuffer(char* bbuffer, int len) {

        int ibusbuffer = n_busbuffers;
        std::vector<uint8_t> buffer(bbuffer, bbuffer + len); // , (uint8_t)bbuffer);
        std::shared_ptr<AudioBus> Bus = MakeBusFromMemory(buffer, false);

        busbuffers[ibusbuffer] = (void*)Bus.get();
        n_busbuffers++;
        return ibusbuffer;

    }


    int libsound_createAudioClip() {
        return 0;
    }

#ifdef __cplusplus
}
#endif

/*
    Dec 25, 2022 prototype III
    0) make an ls_create_context function and cast back to void to return to C.
    1) make an ls_render_node for each of 22 x3d audio nodes, 
        and in each specific ls_render_node create the appropriate labsound node type, 
        and cast back to void* and store in the x3d node
    2) make a ls_connect(ls_context, x3dnode1, x3dnode2)
       in the function, switch-case on x3dnode type to caste void* to appropriate labsound node type
       then call the labsound context.connect(labsound_node1, labsound_node2);
 */
// June 22, 2020 - the above is a prototype I,
// proposed prototype II:
// 1) create_context launches a worker thread per context
//    the worker thread is in C++ and holds smart pointer variables 
// 2) the worker thread loops, and once per loop waits on a condition variable
// 3) once per rendering frame, the browser render_context(node context) sets the condition variable
// 4) if first time, the context worker thread creates all the labsound nodes and connects them
//    to match the x3d declared context and connected child nodes
// 5) the subsequent frames, if any field has been changed on the x3d audio nodes, 
//    the condition varible is set and the worker thread updates the changed fields on
//    the labsound nodes
// 6) at end of program run, the condition variable is set and the worker does an exit,
//    triggering garbage collection of smart pointer variables
// 
// https://en.cppreference.com/w/cpp/thread/condition_variable 
// -shows std::condition_variable and thread.
// But there's a web3d difference between SpatializedSound and SoundEffect nodes:
// SoundEffects
// - may not make the June30 cutoff for v4, but SpatializedSound will
// - have a competing method: a script method, like a script node,
// - with webAudio equivalnet api exposed to the js engine for scripting
// I have no idea how hard it would be to expose labsound api to js.
// - H0: brutal, like nothing we've ever done before
// - H1: routine copy and paste from webAudio implementations
// - H2: much like exposing to scengraph rendering - same deal
//