
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
    void* libsound_createContext()
    {
        std::unique_ptr<lab::AudioContext> context;
        lab::AudioContext* ccontext;
        const auto defaultAudioDeviceConfigurations = GetDefaultAudioDeviceConfiguration();
        context = lab::MakeRealtimeAudioContext(defaultAudioDeviceConfigurations.second, defaultAudioDeviceConfigurations.first);

        //auto musicClip = MakeBusFromSampleFile("samples/stereo-music-clip.wav", argc, argv);
        const std::string path = "C:/Users/Public/dev/source5/audio/LabSound-master/assets/samples/stereo-music-clip.wav";
        std::shared_ptr<AudioBus> bus = MakeBusFromFile(path, false);
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
