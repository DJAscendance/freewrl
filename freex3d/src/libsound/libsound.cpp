
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
    const AudioDeviceIndex default_output_device = lab::GetDefaultOutputAudioDeviceIndex();
    const AudioDeviceIndex default_input_device = lab::GetDefaultInputAudioDeviceIndex();

    AudioDeviceInfo defaultOutputInfo, defaultInputInfo;
    for (auto& info : audioDevices)
    {
        if (info.is_default_output) defaultOutputInfo = info;
        else if (info.is_default_input) defaultInputInfo = info;
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

enum {
    DIST_LINEAR = lab::PannerNode::LINEAR_DISTANCE,
    DIST_INVERSE = lab::PannerNode::INVERSE_DISTANCE,
    DIST_EXPONENTIAL = lab::PannerNode::EXPONENTIAL_DISTANCE,
    DIST_NONE = 0,
};
static struct key_name {
    int iname;
    const char* cname;
} distance_models[] = {
{DIST_LINEAR, "LINEAR"},
{DIST_INVERSE, "INVERSE"},
{DIST_EXPONENTIAL, "EXPONENTIAL"},
{DIST_NONE, NULL},
};
unsigned int name_lookup(char* cname, struct key_name* keynames) {
    unsigned int i, iname;
    struct key_name* cn;
    i = 0;
    iname = 0;
    do {
        cn = &keynames[i];
        if (!strcmp(cn->cname, cname)) {
            iname = cn->iname;
            break;
        }
        i++;
    } while (cn->cname != NULL);
    return iname;

}

#ifndef DEGREES_PER_RADIAN
#define DEGREES_PER_RADIAN 57.2957795130823208768
#endif
#define RAD2DEGF(x) ((float)((x)*DEGREES_PER_RADIAN))

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

#include "libsound.h"
    //static void* busbuffers[30];
    //static int n_busbuffers = 0;
    static std::map<int, std::shared_ptr<lab::AudioBus>> busses;
    static int next_bus;

    void libsound_testNoise()
    {
        std::unique_ptr<lab::AudioContext> context;
         const auto defaultAudioDeviceConfigurations = GetDefaultAudioDeviceConfiguration();
        context = lab::MakeRealtimeAudioContext(defaultAudioDeviceConfigurations.second, defaultAudioDeviceConfigurations.first);
        lab::AudioContext& ac = *context.get();
        //auto musicClip = MakeBusFromSampleFile("samples/stereo-music-clip.wav", argc, argv);
        const std::string path = "C:/Users/Public/dev/source5/audio/LabSound-master/assets/samples/stereo-music-clip.wav";
        std::shared_ptr<AudioBus> bus = MakeBusFromFile(path, false);
        auto musicClip = bus;
        if (!musicClip)
            return;

        std::shared_ptr<OscillatorNode> oscillator;
        std::shared_ptr<SampledAudioNode> musicClipNode;
        std::shared_ptr<GainNode> gain;

        oscillator = std::make_shared<OscillatorNode>(ac);
        gain = std::make_shared<GainNode>(ac);
        gain->gain()->setValue(0.0625f);

        musicClipNode = std::make_shared<SampledAudioNode>(ac);
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

    struct anstruct { std::shared_ptr<lab::AudioNode> anode; };
    struct acstruct {
        std::shared_ptr<lab::AudioContext> context;
        bool running;
        int next_node;
        //int next_bus;
        std::map<int, std::shared_ptr<lab::AudioNode>> nodes;
        //std::map<int, std::shared_ptr<lab::AudioBus>> busses;
    };
    static int next_audio_context;
    static std::map<int, struct acstruct*> audio_contexts;
    int libsound_createContext0() {
        // destination, listener, sampleRate, channel_type
        struct acstruct *ac = new acstruct();
        std::shared_ptr<lab::AudioContext> context;

        //lab::AudioContext *ccontext;
        const auto defaultAudioDeviceConfigurations = GetDefaultAudioDeviceConfiguration();
        context = lab::MakeRealtimeAudioContext(defaultAudioDeviceConfigurations.second, defaultAudioDeviceConfigurations.first);
        if (1) {
            auto listener = context->listener();
            // I believe these are the defaults, and we keep our avatar at 0 and move sound sources relative to avatar
            //listener->forwardX()->setValue(0.0f);
            //listener->forwardY()->setValue(0.0f);
            //listener->forwardZ()->setValue(-1.0f);
            //listener->upX()->setValue(0.0f);
            //listener->upY()->setValue(1.0f);
            //listener->upZ()->setValue(0.0f);
            listener->setForward({ 0.0,0.0,-1.0 });
            listener->setUpVector({ 0.0,1.0,0.0 });
            listener->setPosition({ 0.0,0.0,0.0 });
            //listener->positionX()->setValue(0.0f);
            //listener->positionY()->setValue(0.0f);
            //listener->positionZ()->setValue(0.0f);
            //doppler is deprecated in web audio (web browsers)
            //listener->dopplerFactor()->setValue(1.0f);
        }
        ac->context = context; // libsound_createContext(); // static_cast<lab::AudioContext*>(libsound_createContext());
        ac->running = true; //for pause / resume
        next_audio_context++;
        audio_contexts[next_audio_context] = ac;

        ac->next_node++;
        ac->nodes[ac->next_node] = ac->context->device(); //the output device will be the parent to other source and processing nodes
        return next_audio_context;
    }
    //this one uses int index lookup in a map, much like a vector except can deleted elements
    void libsound_connect0(int icontext, int idestination, int isource) {
        struct acstruct* ac = audio_contexts[icontext];
        std::shared_ptr<AudioNode> destination = ac->nodes[idestination];
        std::shared_ptr<AudioNode> source = ac->nodes[isource];
        ac->context->connect(destination, source);
    }
    void libsound_pauseContext0(int icontext) {
        struct acstruct* ac = audio_contexts[icontext];
        if (ac->running) {
            ac->context->suspend(); //"any queued samples will (still) play" maybe not the right way to turn off.
            ac->running = false;
        }
    }
    void libsound_resumeContext0(int icontext) {
        struct acstruct* ac = audio_contexts[icontext];
        if (!ac->running) {
            ac->context->resume(); //"any queued samples will (still) play" maybe not the right way to turn off.
            ac->running = true;
        }
    }
    void libsound_pauseNode0(struct X3D_Node* node) {
        //this didn't work
        struct X3D_SoundRep* srepn = getSoundRep(X3D_NODE(node));
        int icontext = srepn->icontext;
        if(icontext){
            struct acstruct* ac = audio_contexts[icontext];
            //AudioContext& context = *ac->context.get();
            switch (node->_nodeType) {
            case NODE_Sound:
            {
                struct X3D_Sound* pnode = (struct X3D_Sound*)node;
                std::shared_ptr<PannerNode> pannerNode;
                PannerNode* pannerNode_ptr;
                if (srepn->inode) {
                    pannerNode_ptr = static_cast<PannerNode*>(ac->nodes[srepn->inode].get());
                    {
                        ContextRenderLock r(ac->context.get(), "ex_simple");
                        pannerNode_ptr->silenceOutputs(r);
                    }
                }
            }
            break;
            default:
                break;
            }
        }
    }
    void libsound_resumeNode0(struct X3D_Node* node) {
        // this didn't work
        struct X3D_SoundRep* srepn = getSoundRep(X3D_NODE(node));
        int icontext = srepn->icontext;
        if (icontext) {
            struct acstruct* ac = audio_contexts[icontext];
            //AudioContext& context = *ac->context.get();
            switch (node->_nodeType) {
            case NODE_Sound:
            {
                struct X3D_Sound* pnode = (struct X3D_Sound*)node;
                std::shared_ptr<PannerNode> pannerNode;
                PannerNode* pannerNode_ptr;
                if (srepn->inode) {
                    pannerNode_ptr = static_cast<PannerNode*>(ac->nodes[srepn->inode].get());
                    {
                        ContextRenderLock r(ac->context.get(), "ex_simple");
                        pannerNode_ptr->unsilenceOutputs(r);
                    }
                }
            }
            break;
            default:
                break;
            }
        }
    }

    int libsound_createBusFromBuffer0(char* bbuffer, int len) {
        //static list of busses, independent of audio context, so can DEF/USE?
        std::vector<uint8_t> buffer(bbuffer, bbuffer + len); // , (uint8_t)bbuffer);
        std::shared_ptr<AudioBus> Bus = MakeBusFromMemory(buffer, false);
        next_bus++;
        busses[next_bus] = Bus;
        return next_bus;
    }
    double libsound_computeDuration0(int ibuffer) {
        AudioBus *bus = static_cast<AudioBus*>(busses[ibuffer].get()); 
        double duration = bus->length()* bus->sampleRate();
        return duration;
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

    void libsound_updateNode0(int icontext, int iparent, struct X3D_Node* node) {
        struct acstruct* ac = audio_contexts[icontext];
        AudioContext& context = *ac->context.get();
        //lab::AudioContext& ac = *context.get();
        //goal- switch-case on x3d nodeType and do any labsound node create+connect, update input or update output
        struct X3D_SoundRep* srepn = getSoundRep(node);
        switch (node->_nodeType) {
        case NODE_Sound:
        {
            struct X3D_Sound* pnode = (struct X3D_Sound*)node;
            std::shared_ptr<PannerNode> pannerNode;
            PannerNode* pannerNode_ptr;
            GainNode* gain_ptr;
            if (!srepn->inode) {
                //gain node on output
                std::shared_ptr<GainNode> gain;
                //create labsound node
                gain = std::make_shared<GainNode>(context);
                gain_ptr = gain.get();
                ac->next_node++;
                ac->nodes[ac->next_node] = gain;
                srepn->igain = ac->next_node;
                //connect gain output to parent node input
                if (iparent)
                    libsound_connect0(icontext, iparent, srepn->igain);

                if (pnode->spatialize != TRUE) {
                    //I don't know how to turn off spatialization
                    //EQUALPOWER doesn't do it
                    //so I will ignore
                }

                pannerNode = std::make_shared<PannerNode>(context);
                pannerNode->setPanningModel(PanningModel::EQUALPOWER);
                //pannerNode->setPanningModel(PanningModel::HRTF);
                ac->next_node++;
                ac->nodes[ac->next_node] = pannerNode;
                srepn->inode = ac->next_node;
                srepn->icontext = icontext;
                // connect Sound output to gain input
                libsound_connect0(icontext, srepn->igain, srepn->inode);

                pannerNode_ptr = static_cast<PannerNode*>(ac->nodes[srepn->inode].get());
                //we don't have the inner/outer ellipsoid so we emulate with inner/outer sphere
                pannerNode_ptr->setConeInnerAngle( 90.0f);
                pannerNode_ptr->setConeOuterAngle(135.0f);
                pannerNode_ptr->setConeOuterGain(.07f);
                // LabSound bug - it can't switch directly to linear 
                // because it erroneously thinks its on LINEAR but its on INVERSE
                // so we to exponential first, then linear to get to linear
                pannerNode_ptr->setDistanceModel(lab::PannerNode::EXPONENTIAL_DISTANCE);
                pannerNode_ptr->setDistanceModel(lab::PannerNode::LINEAR_DISTANCE);
                //pannerNode_ptr->setDistanceModel(lab::PannerNode::INVERSE_DISTANCE);
                //pannerNode_ptr->distanceGain()->setValue(0.1f);
                pannerNode_ptr->setRolloffFactor(1.0f);
                pannerNode_ptr->setRefDistance(pnode->minFront);
                pannerNode_ptr->setMaxDistance(pnode->maxFront);

            }
            pannerNode_ptr = static_cast<PannerNode*>(ac->nodes[srepn->inode].get());
            gain_ptr = static_cast<GainNode*>(ac->nodes[srepn->igain].get());
             //std::cout << "[cg= " << pannerNode_ptr->coneGain()->value() << "]" << std::endl;
            gain_ptr->gain()->setValue(pnode->intensity);
            float *xyz = pnode->__lastlocation.c;
            pannerNode_ptr->setPosition(xyz[0], xyz[1], xyz[2]);
            //pannerNode_ptr->positionX()->setValue(pnode->__lastlocation.c[0]);
            //pannerNode_ptr->positionY()->setValue(pnode->__lastlocation.c[1]);
            //pannerNode_ptr->positionZ()->setValue(pnode->__lastlocation.c[2]);
                float* rxyz = pnode->__lastdirection.c;
                //std::cout << " rxyz " << rxyz[0] << " " << rxyz[1] << " " << rxyz[2] << std::endl;
                pannerNode_ptr->setOrientation({ rxyz[0], rxyz[1], rxyz[2] });
                //pannerNode_ptr->orientationX()->setValue(pnode->__lastdirection.c[0]);
                //pannerNode_ptr->orientationY()->setValue(pnode->__lastdirection.c[1]);
                //pannerNode_ptr->orientationZ()->setValue(pnode->__lastdirection.c[2]); //Q. should it be  -ve
        }
        break;
        case NODE_SpatialSound:
        {
            struct X3D_SpatialSound* pnode = (struct X3D_SpatialSound*)node;
            std::shared_ptr<PannerNode> pannerNode;
            PannerNode* pannerNode_ptr;
            GainNode* gain_ptr;
            if (!srepn->inode) {
                //gain node on output
                std::shared_ptr<GainNode> gain;
                //create labsound node
                gain = std::make_shared<GainNode>(context);
                gain_ptr = gain.get();
                ac->next_node++;
                ac->nodes[ac->next_node] = gain;
                srepn->igain = ac->next_node;
                //connect gain output to parent node input
                if (iparent)
                    libsound_connect0(icontext, iparent, srepn->igain);

                if (pnode->spatialize != TRUE) {
                    //I don't know how to turn off spatialization
                    //EQUALPOWER doesn't do it
                    //so I will ignore
                }

                pannerNode = std::make_shared<PannerNode>(context);
                if (pnode->enableHRTF == TRUE) {
                    pannerNode->setPanningModel(lab::PanningModel::HRTF);
                }
                else {
                    pannerNode->setPanningModel(lab::PanningModel::EQUALPOWER);
                }

                //pannerNode->setPanningModel(PanningModel::EQUALPOWER); //HRTF); //EQUALPOWER); // 
                pannerNode->setDistanceModel(lab::PannerNode::EXPONENTIAL_DISTANCE);
                unsigned int distance_enum = name_lookup(pnode->distanceModel->strptr, distance_models);
                switch (distance_enum) {
                case lab::PannerNode::LINEAR_DISTANCE:
                    pannerNode->setDistanceModel(lab::PannerNode::LINEAR_DISTANCE); break;
                case lab::PannerNode::INVERSE_DISTANCE:
                    pannerNode->setDistanceModel(lab::PannerNode::INVERSE_DISTANCE); break;
                case lab::PannerNode::EXPONENTIAL_DISTANCE:
                    pannerNode->setDistanceModel(lab::PannerNode::EXPONENTIAL_DISTANCE); break;
                default:
                    break;
                }
                
                ac->next_node++;
                ac->nodes[ac->next_node] = pannerNode;
                srepn->inode = ac->next_node;
                srepn->icontext = icontext;
                // connect Sound output to gain input
                libsound_connect0(icontext, srepn->igain, srepn->inode);
            }
            pannerNode_ptr = static_cast<PannerNode*>(ac->nodes[srepn->inode].get());
            gain_ptr = static_cast<GainNode*>(ac->nodes[srepn->igain].get());
            pannerNode_ptr->setConeInnerAngle(RAD2DEGF(pnode->coneInnerAngle));
            pannerNode_ptr->setConeOuterAngle(RAD2DEGF(pnode->coneOuterAngle));
            pannerNode_ptr->setConeOuterGain(0.1f);
            // something you would query, not set: pannerNode_ptr->distanceGain()->setValue(0.1f);
            pannerNode_ptr->setRolloffFactor(1.0f);
            pannerNode_ptr->setRefDistance(pnode->referenceDistance);
            pannerNode_ptr->setMaxDistance(pnode->maxDistance);

            if (pnode->dopplerEnabled == TRUE) {
                float* v = pnode->__velocity.c;
                pannerNode_ptr->setVelocity(v[0], v[1], v[2]);
                //std::cout << " vel= " << v[0] << " " << v[1] << " " << v[2] << std::endl;
                auto listener = context.listener();
                listener->setDopplerFactor(1.0f); //default is 1
                listener->setSpeedOfSound(343.0f); //default is 343 m/s
                {
                    ContextRenderLock r(&context, "ex_simple");
                    float dr = pannerNode_ptr->dopplerRate(r);
                    //std::cout << " doppRate " << dr << std::endl;
                    pnode->__dopplerFactor = dr;
                }
            }

            //std::cout << "[cg= " << pannerNode_ptr->coneGain()->value() << "]" << std::endl;
            gain_ptr->gain()->setValue(pnode->intensity * pnode->gain);
            float* xyz = pnode->__lastlocation.c;
            //std::cout << " xyz " << xyz[0] << " " << xyz[1] << " " << xyz[2] << std::endl;
            pannerNode_ptr->setPosition(xyz[0], xyz[1], xyz[2]);
            //pannerNode_ptr->positionX()->setValue(pnode->__lastlocation.c[0]);
            //pannerNode_ptr->positionY()->setValue(pnode->__lastlocation.c[1]);
            //pannerNode_ptr->positionZ()->setValue(pnode->__lastlocation.c[2]);
            float* dir = pnode->__lastdirection.c;
            //std::cout << " dir " << dir[0] << " " << dir[1] << " " << dir[2] << std::endl;
            pannerNode_ptr->setOrientation({ dir[0], dir[1], dir[2] });
            //pannerNode_ptr->orientationX()->setValue(pnode->__lastdirection.c[0]);
            //pannerNode_ptr->orientationY()->setValue(pnode->__lastdirection.c[1]);
            //pannerNode_ptr->orientationZ()->setValue(pnode->__lastdirection.c[2]); //Q. should it be  -ve
        }
        break;

        case NODE_AudioClip:
        {
            struct X3D_AudioClip* pnode = (struct X3D_AudioClip*)node;
            std::shared_ptr<SampledAudioNode> audioClipNode;
            SampledAudioNode* audioClipNode_ptr;
            if (!srepn->inode) {
                //create labsound node
                audioClipNode = std::make_shared<SampledAudioNode>(context);
                {
                    ContextRenderLock r(ac->context.get(), "ex_simple");
                    audioClipNode->setBus(r, busses[srepn->ibuffer]);
                }
                ac->next_node++;
                ac->nodes[ac->next_node] = audioClipNode;
                srepn->inode = ac->next_node;
                srepn->icontext = icontext;
                if (iparent)
                    libsound_connect0(icontext, iparent, srepn->inode);
                //audioClipNode->start((float)pnode->startTime); //do we need to convert to labsound absolute time from x3d absolute time?
                audioClipNode->schedule(0.0, -1); // -1 to loop forever


            }
            //copy changed values from x3d to labsound
            //audioClipNode = static_cast<std::shared_ptr<SampledAudioNode>>( ac->nodes[srepn->inode] );
            audioClipNode_ptr = static_cast<SampledAudioNode*>(ac->nodes[srepn->inode].get());
            // web3d time dependent nodes have an isActive state set elsewhere (freewrl do_AudioTick)
            // here we turn on / off the playback depending on isActive 
            SchedulingState status = audioClipNode_ptr->playbackState();
            if (status == SchedulingState::PLAYING && (pnode->isActive == FALSE || pnode->isPaused == TRUE))
                audioClipNode_ptr->stop(0.0);
            else if (status != SchedulingState::PLAYING && (pnode->isActive == TRUE && pnode->isPaused == FALSE))
                audioClipNode_ptr->start(0.0);
            
            //bool isactive = audioClipNode_ptr->loop();
            //audioClipNode_ptr->setLoop(pnode->loop ? true : false);
            //if (!isactive && pnode->loop) audioClipNode_ptr->start(0.0f);
            audioClipNode_ptr->playbackRate()->setValue(pnode->pitch*srepn->dopplerFactor);
           // audioClipNode_ptr->gain()->setValue(pnode->gain);
            //copy outputs from labsound to x3d
            //pnode->duration_changed = audioClipNode_ptr->duration();
        }
        break;
        case NODE_OscillatorSource:
        {
            struct X3D_OscillatorSource* pnode = (struct X3D_OscillatorSource*)node;
            //if (!pnode->_self) {
            std::shared_ptr<OscillatorNode> oscillator;
            if (!srepn->inode) {
                //create labsound node
                oscillator = std::make_shared<OscillatorNode>(context);
                  ac->next_node++;
                ac->nodes[ac->next_node] = oscillator;
                srepn->inode = ac->next_node;
                srepn->icontext = icontext;
                //connect source node output to parent node input
                if (iparent)
                    libsound_connect0(icontext, iparent, srepn->inode);
            }
            //copy changed values from x3d to labsound
            oscillator->detune()->setValue(pnode->detune);
            //copy outputs from labsound to x3d
        }
        break;
        case NODE_Gain:
        {
            struct X3D_Gain* pnode = (struct X3D_Gain*)node;
            std::shared_ptr<GainNode> gain;
            GainNode* gain_ptr;
            if (!srepn->inode) {
                //create labsound node
                gain = std::make_shared<GainNode>(context);
                gain_ptr = gain.get();
                ac->next_node++;
                ac->nodes[ac->next_node] = gain;
                srepn->inode = ac->next_node;
                srepn->icontext = icontext;
                //connect source node output to parent node input
                if (iparent)
                    libsound_connect0(icontext, iparent, srepn->inode);
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


#ifdef __cplusplus
}
#endif

/*
    Jan 3, 2023 version 4 have some sound_control working
    - need the spatial / panner node now
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