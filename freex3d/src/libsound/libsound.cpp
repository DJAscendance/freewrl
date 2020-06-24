
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
#include "libsound.h"
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
    void libsound_connect(void* ccontext, void* cdestination, void* csource) {
        lab::AudioContext* context = (lab::AudioContext*)ccontext;
        // even if I pass around a shared_ptr, how will it know its type
        //    - don't the incoming paramters need to be strongly typed?
        //std::shared_ptr < lab::AudioNode> destination = dynamic_cast<std::shared_ptr < void *>>cdestination;
        //std::shared_ptr < lab::AudioNode> source = std::make_shared<lab::AudioNode>(csource);
        //std::shared_ptr < lab::AudioNode> destination = std::(destination);

        //context->connect(destination,source);

    }


#ifdef __cplusplus
}
#endif


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