
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
struct key_name {
    int iname;
    const char* cname;
};
static struct key_name distance_models[] = {
{DIST_LINEAR, "LINEAR"},
{DIST_INVERSE, "INVERSE"},
{DIST_EXPONENTIAL, "EXPONENTIAL"},
{DIST_NONE, NULL},
};


static struct key_name periodicWave_types[] = {
{OscillatorType::SINE, "SINE"},
{OscillatorType::SQUARE, "SQUARE"},
{OscillatorType::SAWTOOTH, "SAWTOOTH"},
{OscillatorType::TRIANGLE, "TRIANGLE"},
{OscillatorType::CUSTOM, "CUSTOM"},
{OscillatorType::OSCILLATOR_NONE, NULL},
};


#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif //_MSC_VER

unsigned int name_lookup(char* cname, struct key_name* keynames) {
    unsigned int i, iname;
    struct key_name* cn;
    i = 0;
    iname = 0;
    do {
        cn = &keynames[i];
        if (cn->cname != NULL) {
            //if (!strcmp(cn->cname, cname)) {
            if (!strcasecmp(cn->cname, cname)) {
                iname = cn->iname;
                break;
            }
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
        std::map<int, int> nodetype;
        //std::map<int, std::shared_ptr<lab::AudioBus>> busses;
        std::shared_ptr<std::vector<std::uint8_t>> bytearray; //analyser 
        std::shared_ptr<std::vector<std::float_t>> floatarray; //analyser
    };
    static int next_audio_context;
    static std::map<int, struct acstruct*> audio_contexts;
    int libsound_createContext0() {
        // destination, listener, sampleRate, channel_type
        struct acstruct *ac = new acstruct();
        std::shared_ptr<lab::AudioContext> context;

        //lab::AudioContext *ccontext;
        const auto defaultAudioDeviceConfigurations = GetDefaultAudioDeviceConfiguration(true);
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
       // ac->running = true; //for pause / resume
        next_audio_context++;
        audio_contexts[next_audio_context] = ac;

        ac->next_node++;
        ac->nodes[ac->next_node] = ac->context->device(); //the output device will be the parent to other source and processing nodes
        ac->nodetype[ac->next_node] = NODE_AudioDestination;
        return next_audio_context;
    }
    static struct type_name {
        int iname;
        const char* cname;
    } type_names[] = {
    {NODE_AudioDestination, "AD"},
    {NODE_Analyser, "Anly"},
    {NODE_Sound, "Snd"},
    {NODE_SpatialSound, "SS"},
    {NODE_AudioClip, "AC"},
    {NODE_Gain, "Gain"},
    {NODE_Convolver, "Conv"},
    {NODE_WaveShaper, "WShp"},
    {NODE_BiquadFilter, "BiQ"},
    {NODE_DynamicsCompressor, "DCmp"},
    {NODE_ChannelSplitter, "Splt"},
    {NODE_ChannelMerger, "Merg"},
    {NODE_Delay, "Dlay"},
    {NODE_BufferAudioSource, "BAS"},
    {NODE_AudioBuffer, "ABuf"},
    {NODE_OscillatorSource, "Osc"},
    {NODE_ListenerPointSource, "LPS"},
    {NODE_StreamAudioDestination, "SAD"},
    {NODE_StreamAudioSource, "SAS"},
    {NODE_MicrophoneSource, "MicS"},
    {0,NULL},
    };
    const char* nodetype_lookup(int itype) {
        int i;
        const char *cname;
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
    struct connection {
        int icontext; 
        int iparent;
        int iparent_type;
        int ichild;
        int ichild_type;
        int srcindex;
        int dstindex;
    };
    
    static std::vector<connection> connections;
    void libsound_connect0(int icontext, int idestination, int isource) {
        struct acstruct* ac = audio_contexts[icontext];
        std::shared_ptr<AudioNode> destination = ac->nodes[idestination];
        std::shared_ptr<AudioNode> source = ac->nodes[isource];
        ac->context->connect(destination, source);
        int iparent_type = ac->nodetype[idestination];
        int ichild_type = ac->nodetype[isource];
        struct connection cc; cc.icontext = icontext; cc.iparent = idestination; cc.iparent_type = iparent_type; 
           cc.ichild = isource; cc.ichild_type = ichild_type; cc.srcindex = 0; cc.dstindex = 0;
        connections.push_back(cc);
    }
    void libsound_connect1(int icontext, int idestination, int isource, int indexSrc) {
        struct acstruct* ac = audio_contexts[icontext];
        std::shared_ptr<AudioNode> destination = ac->nodes[idestination];
        std::shared_ptr<AudioNode> source = ac->nodes[isource];
        ac->context->connect(destination, source,0,indexSrc);
        int iparent_type = ac->nodetype[idestination];
        int ichild_type = ac->nodetype[isource];
        struct connection cc; cc.icontext = icontext; cc.iparent = idestination; cc.iparent_type = iparent_type;
        cc.ichild = isource; cc.ichild_type = ichild_type; cc.srcindex = indexSrc; cc.dstindex = 0;
        connections.push_back(cc);

    }
    void libsound_connect2(int icontext, int idestination, int isource, int indexDst, int indexSrc) {
        struct acstruct* ac = audio_contexts[icontext];
        std::shared_ptr<AudioNode> destination = ac->nodes[idestination];
        std::shared_ptr<AudioNode> source = ac->nodes[isource];
        int dstInputs = destination->numberOfInputs();
        int srcOutputs = source->numberOfInputs();
        if (indexDst > dstInputs) {
            printf("destination number of inputs %d destination idx %d\n", destination->numberOfInputs(), indexDst);
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

        ac->context->connect(destination, source, indexDst, indexSrc);
        int iparent_type = ac->nodetype[idestination];
        int ichild_type = ac->nodetype[isource];
        struct connection cc; cc.icontext = icontext; cc.iparent = idestination; cc.iparent_type = iparent_type;
        cc.ichild = isource; cc.ichild_type = ichild_type; cc.srcindex = indexSrc; cc.dstindex = indexDst;
        connections.push_back(cc);

    }
    void libsound_print_connections() {
        printf("\n");
        printf("%2s %7s %4s %7s %7s %6s %4s\n","ic","iparent", "type", "dstIndx", "srcIndex", "ichild", "type");
        for (int i = 0; i < connections.size(); i++) {
            struct connection cc = connections[i];
            const char* ptype = nodetype_lookup(cc.iparent_type);
            const char* ctype = nodetype_lookup(cc.ichild_type);

            printf("%2d %7d %4s %7d %7d %6d %4s\n", cc.icontext, cc.iparent, ptype, cc.dstindex, cc.srcindex, cc.ichild, ctype);
        }
        printf("count %d\n", (int)connections.size());
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
        std::shared_ptr<AudioBus> Bus;
        if (0) {
            FILE * fp = fopen("tmp_buf_wav", "w");
            int nchunks = len / 1024;
            int leftover = len % 1024;
            for(int i=0;i<nchunks;i++)
                fwrite(&bbuffer[i*1024], 1024, 1, fp);
            fwrite(&bbuffer[nchunks * 1024], leftover, 1, fp);
            fclose(fp);
            // x this doesn't work. I get junk temp file, and Bus null/empty.
            Bus = MakeBusFromFile("tmp_buf_wav", false);
            remove("tmp_buf.wav");
        }
        else {
            std::vector<uint8_t> buffer(bbuffer, bbuffer + len); // , (uint8_t)bbuffer);
            Bus = MakeBusFromMemory(buffer, false);
            printf(".");
        }
        next_bus++;
        busses[next_bus] = Bus;
        return next_bus;
    }
    int libsound_createBusFromPCM32(float* buffer, int nchannel, int lentotal) {
        //static list of busses, independent of audio context, so can DEF/USE?
        int length = lentotal / nchannel;
        std::shared_ptr<lab::AudioBus> audioBus(new lab::AudioBus(nchannel, length));
        audioBus->setSampleRate(44100.0);
        //audioBus->setSampleRate((float)audioData->sampleRate);
        for (int i = 0; i < nchannel; ++i)
        {
            std::memcpy(audioBus->channel(i)->mutableData(), buffer + (i * length), length * sizeof(float));
        }

        next_bus++;
        busses[next_bus] = audioBus;
        return next_bus;

    }

    int libsound_createBusFromFile0(char* url) {
        //static list of busses, independent of audio context, so can DEF/USE?
        std::shared_ptr<AudioBus> Bus;
        Bus = MakeBusFromFile(url, false);
        next_bus++;
        busses[next_bus] = Bus;
        return next_bus;
    }
    double libsound_computeDuration0(int ibuffer) {
        AudioBus *bus = static_cast<AudioBus*>(busses[ibuffer].get()); 
        double duration = bus->length()* bus->sampleRate();
        return duration;
    }
    void getChannelInterpretation(char *interpretation, char *mode, ChannelInterpretation *interp, ChannelCountMode *cmode) {
        *interp = lab::ChannelInterpretation::Speakers;
        if (!_stricmp(interpretation, "DISCRETE"))
            *interp = lab::ChannelInterpretation::Discrete;
        // ["max", "clamped-max", "explicit"]
        *cmode = lab::ChannelCountMode::Max;
        if (!_stricmp(mode, "CLAMPED-MAX")) *cmode = lab::ChannelCountMode::ClampedMax;
        else if (!_stricmp(mode, "EXPLICIT")) *cmode = lab::ChannelCountMode::Explicit;

    }
    void libsound_connect(int icontext, int inode, ivec3 iparent) {
        if (iparent.x)
            libsound_connect2(icontext, iparent.x, inode, iparent.y, iparent.z);
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
    static int nondefault_channelinterp = 0;
    void libsound_updateNode3(int icontext, ivec3 iparent, struct X3D_Node* node) {
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
                ac->nodetype[ac->next_node] = NODE_Gain;
                srepn->igain = ac->next_node;
                //connect gain output to parent node input
                if (iparent.x)
                    libsound_connect2(icontext, iparent.x, srepn->igain, iparent.y, iparent.z);
  

                if (pnode->spatialize != TRUE) {
                    //I don't know how to turn off spatialization
                    //EQUALPOWER doesn't do it
                    //so I will ignore
                }

                pannerNode = std::make_shared<PannerNode>(context);
                pannerNode->setPanningModel(PanningModel::EQUALPOWER); //PanningModel:: in later LabSound releases
                //pannerNode->setPanningModel(PanningModel::HRTF);
                ac->next_node++;
                ac->nodes[ac->next_node] = pannerNode;
                ac->nodetype[ac->next_node] = NODE_Sound;
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
                ac->nodetype[ac->next_node] = NODE_Gain;
                srepn->igain = ac->next_node;
                //connect gain output to parent node input
                if (iparent.x)
                    libsound_connect2(icontext, iparent.x, srepn->igain, iparent.y, iparent.z);

                if (pnode->spatialize != TRUE) {
                    //I don't know how to turn off spatialization
                    //EQUALPOWER doesn't do it
                    //so I will ignore
                }
                //if (!context.loadHrtfDatabase("hrtf")) {  //always returns true, so go directly for the good path
                    
                    std::string path = std::string("../../../../lib_windows_vc12/LabSound/share") + "/hrtf";
                    if (!context.loadHrtfDatabase(path)) {
                        printf("Could not load spatialization database");
                        return;
                    }
                //}

                pannerNode = std::make_shared<PannerNode>(context);
               
                if (pnode->enableHRTF == TRUE) {
                    pannerNode->setPanningModel(lab::PanningModel::HRTF);
                    printf("SpatialSound HRTF enabled\n");
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
                ac->nodetype[ac->next_node] = NODE_SpatialSound;
                srepn->inode = ac->next_node;
                srepn->icontext = icontext;
                // connect Sound output to gain input
                libsound_connect0(icontext, srepn->igain, srepn->inode);
            }
            pannerNode_ptr = static_cast<PannerNode*>(ac->nodes[srepn->inode].get());
            gain_ptr = static_cast<GainNode*>(ac->nodes[srepn->igain].get());
            gain_ptr->gain()->setValue(pnode->intensity* pnode->gain);
            pannerNode_ptr->setConeInnerAngle(RAD2DEGF(pnode->coneInnerAngle));
            pannerNode_ptr->setConeOuterAngle(RAD2DEGF(pnode->coneOuterAngle));
            pannerNode_ptr->setConeOuterGain(pnode->coneOuterGain);
            // something you would query, not set: pannerNode_ptr->distanceGain()->setValue(0.1f);
            pannerNode_ptr->setRolloffFactor(pnode->rolloffFactor);
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
            // done above gain_ptr->gain()->setValue(pnode->intensity * pnode->gain);

            float* dir0 = pnode->__lastdirection.c;
            float dir[3];
            memcpy(dir, dir0,3*sizeof(float));
           // dir[0] = fabs(dir[0]) < .001f ? 0.0f : dir[0];
            //deep mystery: Spatial.x3d when navigating in Fly: sound will cut out
            // but doesn't cut out if dir.y is either 0 or .001 or bigger.
            // same { dir, xyz } in LabSound Examples.hpp doesn't have a problem
            // bizarre because dir/orientation shouldn't be needed when inner and outer cone angles are both 2 PI
            dir[1] = fabs(dir[1]) < .001f ? copysign(.001f,dir[1]) : dir[1];
            //std::cout << " { " << dir[0] << ", " << dir[1] << ", " << dir[2] << ", ";

            pannerNode_ptr->setOrientation({ dir[0], dir[1] , dir[2] });
            //pannerNode_ptr->orientationX()->setValue(dir[0]);
            //pannerNode_ptr->orientationY()->setValue(dir[1]);
            //pannerNode_ptr->orientationZ()->setValue(dir[2]); //Q. should it be  -ve


            float* xyz0 = pnode->__lastlocation.c;
            float xyz[3];
            memcpy(xyz, xyz0, 3 * sizeof(float));
            //xyz[0] = fabs(xyz[0]) < .001f ? 0.0f : xyz[0];
            //xyz[1] = fabs(xyz[1]) < .001f ? .001f : xyz[1];
            //std::cout << xyz[0] << ", " << xyz[1] << ", " << xyz[2] << "}," << std::endl;
            //static int once = 0;
            //if(!once)
            pannerNode_ptr->setPosition(xyz[0], xyz[1], xyz[2]);
           // once++;
            //pannerNode_ptr->positionX()->setValue(xyz[0]);
            //pannerNode_ptr->positionY()->setValue(xyz[1]);
            //pannerNode_ptr->positionZ()->setValue(xyz[2]);
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
                ac->nodetype[ac->next_node] = NODE_AudioClip;
                srepn->inode = ac->next_node;
                srepn->icontext = icontext;
                //if (iparent.x)
                //    libsound_connect2(icontext, iparent.x, srepn->inode, iparent.y, iparent.z);


                //audioClipNode->start((float)pnode->startTime); //do we need to convert to labsound absolute time from x3d absolute time?
                audioClipNode->schedule(0.0, -1); // -1 to loop forever


            }
            //copy changed values from x3d to labsound
            //audioClipNode = static_cast<std::shared_ptr<SampledAudioNode>>( ac->nodes[srepn->inode] );
            audioClipNode_ptr = static_cast<SampledAudioNode*>(ac->nodes[srepn->inode].get());
            // web3d time dependent nodes have an isActive state set elsewhere (freewrl do_AudioTick)
            // here we turn on / off the playback depending on isActive 
            if (1) {
                SchedulingState status = audioClipNode_ptr->playbackState();
                if (status == SchedulingState::PLAYING && (pnode->isPaused == TRUE))
                    audioClipNode_ptr->stop(0.0);
                else if (status != SchedulingState::PLAYING && (pnode->isPaused == FALSE))
                    audioClipNode_ptr->start(0.0);

                bool isactive = audioClipNode_ptr->isPlayingOrScheduled();
                //audioClipNode_ptr->setLoop(pnode->loop ? true : false);
                if (!isactive && pnode->loop) 
                    audioClipNode_ptr->start(0.0f);
                audioClipNode_ptr->playbackRate()->setValue(pnode->pitch * srepn->dopplerFactor);
            }
           // audioClipNode_ptr->gain()->setValue(pnode->gain);
            //copy outputs from labsound to x3d
            //pnode->duration_changed = audioClipNode_ptr->duration();
        }
        break;
        case NODE_BufferAudioSource:
        {
            struct X3D_BufferAudioSource* pnode = (struct X3D_BufferAudioSource*)node;
            std::shared_ptr<SampledAudioNode> audioSource;
            SampledAudioNode* audioSource_ptr;
            if (!srepn->ibuffer) break; //wait for url to load
            if (!srepn->inode) {
                //create labsound node
                audioSource = std::make_shared<SampledAudioNode>(context);
                {
                    ContextRenderLock r(ac->context.get(), "ex_simple");
                    audioSource->setBus(r, busses[srepn->ibuffer]);
                }
                ac->next_node++;
                ac->nodes[ac->next_node] = audioSource;
                ac->nodetype[ac->next_node] = NODE_BufferAudioSource;
                srepn->inode = ac->next_node;
                srepn->icontext = icontext;
                //if (iparent.x)
                //    libsound_connect2(icontext, iparent.x, srepn->inode, iparent.y, iparent.z);


                //audioClipNode->start((float)pnode->startTime); //do we need to convert to labsound absolute time from x3d absolute time?
                audioSource->schedule(0.0, -1); // -1 to loop forever


            }
            //copy changed values from x3d to labsound
            //audioClipNode = static_cast<std::shared_ptr<SampledAudioNode>>( ac->nodes[srepn->inode] );
            audioSource_ptr = static_cast<SampledAudioNode*>(ac->nodes[srepn->inode].get());
            // web3d time dependent nodes have an isActive state set elsewhere (freewrl do_AudioTick)
            // here we turn on / off the playback depending on isActive 
            if (1) {
                SchedulingState status = audioSource_ptr->playbackState();
                if (status == SchedulingState::PLAYING && pnode->isPaused)
                    audioSource_ptr->stop(0.0);
                else if (status != SchedulingState::PLAYING && pnode->isPaused == FALSE)
                    audioSource_ptr->start(0.0);

                bool isactive = audioSource_ptr->isPlayingOrScheduled();
                pnode->isActive = isactive ? 1 : 0;
                if (!isactive && pnode->loop) audioSource_ptr->start(0.0f);

                audioSource_ptr->playbackRate()->setValue(pnode->playbackRate);
                audioSource_ptr->detune()->setValue(pnode->detune);

            }
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
            OscillatorNode* oscillator_ptr;
           // GainNode* gain_ptr;
            if (!srepn->inode) {
                //gain node on output
                //std::shared_ptr<GainNode> gain;
                ////create labsound node
                //gain = std::make_shared<GainNode>(context);
                //gain_ptr = gain.get();
                //ac->next_node++;
                //ac->nodes[ac->next_node] = gain;
                //ac->nodetype[ac->next_node] = NODE_Gain;
                //srepn->igain = ac->next_node;
                ////connect gain output to parent node input
                //if (iparent.x)
                //    libsound_connect2(icontext, iparent.x, srepn->igain, iparent.y, iparent.z);

                //create labsound node
                oscillator = std::make_shared<OscillatorNode>(context);
                  ac->next_node++;
                ac->nodes[ac->next_node] = oscillator;
                ac->nodetype[ac->next_node] = NODE_OscillatorSource;
                srepn->inode = ac->next_node;
                srepn->icontext = icontext;

                //connect source node output to parent node input
                //libsound_connect0(icontext, srepn->igain, srepn->inode);
                //if (iparent.x)
                //    libsound_connect2(icontext, iparent.x, srepn->inode, iparent.y, iparent.z);

                oscillator->start(0.0f);
            }
            //copy changed values from x3d to labsound
            
            //gain_ptr = static_cast<GainNode*>(ac->nodes[srepn->igain].get());
            //gain_ptr->gain()->setValue(pnode->gain);
            oscillator_ptr = static_cast<OscillatorNode*>(ac->nodes[srepn->inode].get());

            oscillator_ptr->frequency()->setValue(pnode->frequency);
            //printf("from libsound updateNode0 OscillatorSource > detun %f\n", pnode->detune);
            oscillator_ptr->detune()->setValue(pnode->detune);

            SchedulingState status = oscillator_ptr->playbackState();
            // printf("isActive %d isPaused %d status %d\n", pnode->isActive, pnode->isPaused, status);
            if (status == SchedulingState::PLAYING && (pnode->isActive == FALSE || pnode->isPaused == TRUE)) {
                oscillator_ptr->stop(0.0);
                //printf("called stop \n");
                //getchar();
            }
            else if (status != SchedulingState::PLAYING && (pnode->isActive == TRUE && pnode->isPaused == FALSE)) {
                oscillator_ptr->start(0.0);
                //printf("called start\n");
                //getchar();
            }
            
        }
        break;
        case NODE_PeriodicWave:
        {
            struct X3D_PeriodicWave* pnode = (struct X3D_PeriodicWave*)node;
            std::shared_ptr<PeriodicWave> pwave; //using an older term but equivalent WaveTable == PeriodicWave
            if (iparent.x){
                std::shared_ptr<AudioNode> oscillator = ac->nodes[iparent.x];
                OscillatorNode* oscillator_ptr =
                    static_cast<OscillatorNode*>(oscillator.get());
               //periodicWave_types
                unsigned int wave_type = name_lookup(pnode->type->strptr, periodicWave_types);
                switch (wave_type) {
                case OscillatorType::SINE:
                    oscillator_ptr->setType(OscillatorType::SINE); break;
                case OscillatorType::SQUARE:
                    oscillator_ptr->setType(OscillatorType::SQUARE); break;
                case OscillatorType::SAWTOOTH:
                    oscillator_ptr->setType(OscillatorType::SAWTOOTH); break;
                case OscillatorType::TRIANGLE:
                    oscillator_ptr->setType(OscillatorType::TRIANGLE); break;
                case OscillatorType::CUSTOM:
                    oscillator_ptr->setType(OscillatorType::CUSTOM); break;
                default:
                    oscillator_ptr->setType(OscillatorType::OSCILLATOR_NONE);
                    break;
                }
                if (pnode->optionsReal.n != 0) {
                    ///oscillator_ptr-> where put wavetable?
                    //context.createPeriodicWave
                }
                //oscillator_ptr->setType(static_cast<OscillatorType>(wave_type));

            }
            //copy changed values from x3d to labsound
            //copy outputs from labsound to x3d

        }

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
                ac->nodetype[ac->next_node] = NODE_Gain;
                srepn->inode = ac->next_node;
                srepn->icontext = icontext;
                if (nondefault_channelinterp) {
                    ChannelInterpretation interp;
                    ChannelCountMode cmode;
                    getChannelInterpretation(pnode->channelInterpretation->strptr, pnode->channelCountMode->strptr, &interp, &cmode);
                    gain_ptr->setChannelInterpretation(interp);
                    {
                        ContextGraphLock g(ac->context.get(), "ex_simple");
                        gain_ptr->setChannelCountMode(g, cmode);
                    }
                }
                //connect source node output to parent node input
                //if (iparent.x)
                //    libsound_connect2(icontext, iparent.x, srepn->inode, iparent.y, iparent.z);

            }
            gain_ptr = dynamic_cast<GainNode*>(ac->nodes[srepn->inode].get());
            //copy changed values from x3d to labsound
            gain_ptr->gain()->setValue(pnode->gain);
            //gain_ptr->silenceOutputs()
            //gain_ptr->unsilenceOutputs()

            //copy outputs from labsound to x3d

        }
        break;
        case NODE_ChannelSplitter:
        {
            struct X3D_ChannelSplitter* pnode = (struct X3D_ChannelSplitter*)node;
            std::shared_ptr<ChannelSplitterNode> splitter;
            ChannelSplitterNode* splitter_ptr;
            //GainNode* gain_ptr;
            if (!srepn->inode) {
                ////gain node on output
                //std::shared_ptr<GainNode> gain;
                ////create labsound node
                //gain = std::make_shared<GainNode>(context);
                //gain_ptr = gain.get();
                //ac->next_node++;
                //ac->nodes[ac->next_node] = gain;
                //ac->nodetype[ac->next_node] = NODE_Gain;
                //srepn->igain = ac->next_node;
                ////connect gain output to parent node input
                //if (iparent.x)
                //    libsound_connect2(icontext, iparent.x, srepn->igain, iparent.y, iparent.z);

                splitter = std::make_shared<ChannelSplitterNode>(context,pnode->channelCount);
                splitter_ptr = splitter.get();
                //splitter_ptr->output()
                ac->next_node++;
                ac->nodes[ac->next_node] = splitter;
                ac->nodetype[ac->next_node] = NODE_ChannelSplitter;
                srepn->inode = ac->next_node;
                srepn->icontext = icontext;
                if (nondefault_channelinterp) {

                    ChannelInterpretation interp;
                    ChannelCountMode cmode;
                    getChannelInterpretation(pnode->channelInterpretation->strptr, pnode->channelCountMode->strptr, &interp, &cmode);
                    splitter_ptr->setChannelInterpretation(interp);
                    {
                        ContextGraphLock g(ac->context.get(), "ex_simple");
                        splitter_ptr->setChannelCountMode(g, cmode);
                    }
                }
                //connect source node output to parent node input
                //libsound_connect0(icontext, srepn->igain, srepn->inode);
                //if (iparent.x)
                //    libsound_connect2(icontext, iparent.x, srepn->inode, iparent.y, iparent.z);

                //splitter is the one sound node that doesn't follow single-parent-multiple-children paradigm:
                // - it has multiple parents, meaning the output can go to multiple nodes,
 /*               for (int i = 0; i < std::min(pnode->channelCount,pnode->outputs.n); i++)
                {
                    struct X3D_Node* onode = (struct X3D_Node*)pnode->outputs.p[i];
                    struct X3D_SoundRep* srepo = getSoundRep(onode);
                    if (srepo && srepo->inode)
                        libsound_connect1(icontext, srepo->inode, srepn->inode,i);
                }*/

            }
            //Q. I want to add more connections on-the-fly, but how to prevent double connections on routine node recompile
            // and how to avoid missing a new connection when otherwise node doesn't need recompile?
            // Options:
            // add __field to ChannelSplitter, and iterate over first, before adding connection
            // add field to X3DSoundRep just for splitter connection tracking, (int,int) (parent,srcIndex)
            //if (iparent.x)
            //    libsound_connect2(icontext, iparent.x, srepn->inode, iparent.y, iparent.z);

            //gain_ptr = dynamic_cast<GainNode*>(ac->nodes[srepn->igain].get());
            ////copy changed values from x3d to labsound
            //gain_ptr->gain()->setValue(pnode->gain);
            splitter_ptr = dynamic_cast<ChannelSplitterNode*>(ac->nodes[srepn->inode].get());
            //copy outputs from labsound to x3d

        }
        break;
        case NODE_ChannelSelector:
        {
            //doesn't do anything except push a channel ID onto a stack in Component_Sound
        }
        break;
        case NODE_ChannelMerger:
        {
            struct X3D_ChannelMerger* pnode = (struct X3D_ChannelMerger*)node;
            std::shared_ptr<ChannelMergerNode> merger;
            ChannelMergerNode* merger_ptr;
            //GainNode* gain_ptr;
            if (!srepn->inode) {
                ////gain node on output
                //std::shared_ptr<GainNode> gain;
                ////create labsound node
                //gain = std::make_shared<GainNode>(context);
                //gain_ptr = gain.get();
                //ac->next_node++;
                //ac->nodes[ac->next_node] = gain;
                //ac->nodetype[ac->next_node] = NODE_Gain;
                //srepn->igain = ac->next_node;
                ////connect gain output to parent node input
                //if (iparent.x)
                //    libsound_connect2(icontext, iparent.x, srepn->igain, iparent.y, iparent.z);

                merger = std::make_shared<ChannelMergerNode>(context, pnode->channelCount);
                merger_ptr = merger.get();
                if (nondefault_channelinterp) {

                    ChannelInterpretation interp;
                    ChannelCountMode cmode;
                    getChannelInterpretation(pnode->channelInterpretation->strptr, pnode->channelCountMode->strptr, &interp, &cmode);
                    merger_ptr->setChannelInterpretation(interp);
                    {
                        ContextGraphLock g(ac->context.get(), "ex_simple");
                        merger_ptr->setChannelCountMode(g, cmode);
                    }
                }
                ac->next_node++;
                ac->nodes[ac->next_node] = merger;
                ac->nodetype[ac->next_node] = NODE_ChannelMerger;
                srepn->inode = ac->next_node;
                srepn->icontext = icontext;
                //connect source node output to parent node input
                //libsound_connect0(icontext, srepn->igain, srepn->inode);
                //if (iparent.x)
                //   libsound_connect2(icontext, iparent.x, srepn->inode, iparent.y, iparent.z);

            }
            //gain_ptr = dynamic_cast<GainNode*>(ac->nodes[srepn->igain].get());
            //copy changed values from x3d to labsound
            //gain_ptr->gain()->setValue(pnode->gain);
            merger_ptr = dynamic_cast<ChannelMergerNode*>(ac->nodes[srepn->inode].get());
            //copy outputs from labsound to x3d

        }
        break;
        case NODE_Delay:
        {
            struct X3D_Delay* pnode = (struct X3D_Delay*)node;
            std::shared_ptr<DelayNode> delay;
            DelayNode* delay_ptr;
            std::shared_ptr<GainNode> gain;
            //GainNode* gain_ptr;
            if (!srepn->inode) {
                ////create labsound node
                //gain = std::make_shared<GainNode>(context);
                //gain_ptr = gain.get();
                //ac->next_node++;
                //ac->nodes[ac->next_node] = gain;
                //ac->nodetype[ac->next_node] = NODE_Gain;
                //srepn->igain = ac->next_node;
                //srepn->icontext = icontext;
                ////connect source node output to parent node input
                //if (iparent.x)
                //    libsound_connect2(icontext, iparent.x, srepn->igain, iparent.y, iparent.z);

                delay = std::make_shared<DelayNode>(context,pnode->maxDelayTime);
                delay_ptr = delay.get();
                ac->next_node++;
                ac->nodes[ac->next_node] = delay;
                ac->nodetype[ac->next_node] = NODE_Delay;
                srepn->inode = ac->next_node;
                srepn->icontext = icontext;
                if (nondefault_channelinterp) {

                    ChannelInterpretation interp;
                    ChannelCountMode cmode;
                    getChannelInterpretation(pnode->channelInterpretation->strptr, pnode->channelCountMode->strptr, &interp, &cmode);
                    delay_ptr->setChannelInterpretation(interp);
                    {
                        ContextGraphLock g(ac->context.get(), "ex_simple");
                        delay_ptr->setChannelCountMode(g, cmode);
                    }
                }
                //connect source node output to parent node input
                //libsound_connect0(icontext, srepn->igain, srepn->inode);
                //if (iparent.x)
                //    libsound_connect2(icontext, iparent.x, srepn->inode, iparent.y, iparent.z);

            }
            //gain_ptr = dynamic_cast<GainNode*>(ac->nodes[srepn->igain].get());
            //copy changed values from x3d to labsound
            //gain_ptr->gain()->setValue(pnode->gain);
            delay_ptr = dynamic_cast<DelayNode*>(ac->nodes[srepn->inode].get());
            delay_ptr->delayTime()->setFloat((float)pnode->delayTime,false);

 
        }
        break;
        case NODE_Analyser:
        {
            struct X3D_Analyser* pnode = (struct X3D_Analyser*)node;
            std::shared_ptr<AnalyserNode> analyser;
            AnalyserNode* analyser_ptr;
            //std::shared_ptr<GainNode> gain;
            //GainNode* gain_ptr;
            if (!srepn->inode) {
                ////create labsound node
                //gain = std::make_shared<GainNode>(context);
                //gain_ptr = gain.get();
                //ac->next_node++;
                //ac->nodes[ac->next_node] = gain;
                //ac->nodetype[ac->next_node] = NODE_Gain;
                //srepn->igain = ac->next_node;
                //srepn->icontext = icontext;
                ////connect source node output to parent node input
                //if (iparent.x)
                //    libsound_connect2(icontext, iparent.x, srepn->igain, iparent.y, iparent.z);

                analyser = std::make_shared<AnalyserNode>(context,pnode->fftSize);
                analyser_ptr = analyser.get();
                ac->next_node++;
                ac->nodes[ac->next_node] = analyser;
                ac->nodetype[ac->next_node] = NODE_Analyser;
                srepn->inode = ac->next_node;
                srepn->icontext = icontext;
                if (nondefault_channelinterp) {

                    ChannelInterpretation interp;
                    ChannelCountMode cmode;
                    getChannelInterpretation(pnode->channelInterpretation->strptr, pnode->channelCountMode->strptr, &interp, &cmode);
                    analyser_ptr->setChannelInterpretation(interp);
                    {
                        ContextGraphLock g(ac->context.get(), "ex_simple");
                        analyser_ptr->setChannelCountMode(g, cmode);
                    }
                }
                //connect source node output to parent node input
                //libsound_connect0(icontext, srepn->igain, srepn->inode);
                //if (iparent.x)
                //    libsound_connect2(icontext, iparent.x, srepn->inode, iparent.y, iparent.z);


            }
            //gain_ptr = dynamic_cast<GainNode*>(ac->nodes[srepn->igain].get());
            ////copy changed values from x3d to labsound
            //gain_ptr->gain()->setValue(pnode->gain);
            analyser_ptr = dynamic_cast<AnalyserNode*>(ac->nodes[srepn->inode].get());
            pnode->frequencyBinCount = (int)analyser_ptr->frequencyBinCount();
            //printf("start analyser bins=%d\n", pnode->frequencyBinCount);

            analyser_ptr->setMaxDecibels(pnode->maxDecibels);
            analyser_ptr->setMinDecibels(pnode->minDecibels);
            analyser_ptr->setSmoothingTimeConstant(pnode->smoothingTimeConstant);
            {
                if (ac->bytearray == nullptr)
                    ac->bytearray = std::make_shared<std::vector<std::uint8_t>>(4096);
                if (ac->floatarray == nullptr)
                    ac->floatarray = std::make_shared<std::vector<std::float_t>>(4096);

                std::vector<std::float_t>* floatarray = ac->floatarray.get();
                std::vector<std::uint8_t>* bytearray = ac->bytearray.get();
                //analyser_ptr->getByteFrequencyData(*bytearray);
                analyser_ptr->getFloatFrequencyData(*floatarray);
                //if (pnode->byteFrequencyData.n == 0){
                //    pnode->byteFrequencyData.p = (int*)malloc(pnode->frequencyBinCount/4);
                //    pnode->byteFrequencyData.n = pnode->frequencyBinCount / 4;
                //}
                if (pnode->floatFrequencyData.n == 0) {
                    pnode->floatFrequencyData.p = (float*)malloc(pnode->frequencyBinCount * sizeof(std::float_t));
                    pnode->floatFrequencyData.n = pnode->frequencyBinCount;
                }

                std::uint8_t* p8 = (std::uint8_t*)pnode->byteFrequencyData.p;
                std::float_t* ff = (std::float_t*)pnode->floatFrequencyData.p;
                for (int i = 0; i < pnode->frequencyBinCount; i++) {
                    //brute force. Is there a memcpy path?
                    //p8[i] = (*bytearray)[i];
                    ff[i] = (*floatarray)[i];
                }
                //analyser_ptr->getByteTimeDomainData(*bytearray);
                analyser_ptr->getFloatTimeDomainData(*floatarray);
                //if (pnode->byteTimeDomainData.n == 0) {
                //    pnode->byteTimeDomainData.p = (int*)malloc(pnode->frequencyBinCount / 4);
                //    pnode->byteTimeDomainData.n = pnode->frequencyBinCount / 4;
                //}
                if (pnode->floatTimeDomainData.n == 0) {
                    pnode->floatTimeDomainData.p = (float*)malloc(pnode->frequencyBinCount * sizeof(std::float_t));
                    pnode->floatTimeDomainData.n = pnode->frequencyBinCount;
                }

                //std::uint8_t* p8t = (std::uint8_t*)pnode->byteTimeDomainData.p;
                std::float_t* fft = (std::float_t*)pnode->floatTimeDomainData.p;
                for (int i = 0; i < pnode->frequencyBinCount; i++) {
                    //brute force. Is there a memcpy path?
                    //p8t[i] = (*bytearray)[i];
                    fft[i] = (*floatarray)[i];
                }
                
            }
            //printf("end analyser\n");
        }
        break;
        case NODE_BiquadFilter:
        {
            static struct key_name biquad_types[] = {
            {FilterType::ALLPASS, "ALLPASS"},
            {FilterType::BANDPASS, "BANDPASS"},
            {FilterType::FILTER_NONE, "NONE"},
            {FilterType::HIGHPASS, "HIGHPASS"},
            {FilterType::HIGHSHELF, "HIGHSHELF"},
            {FilterType::LOWPASS, "LOWPASS"},
            {FilterType::LOWSHELF, "LOWSHELF"},
            {FilterType::NOTCH, "NOTCH"},
            {FilterType::NOTCH, "PEAKING"},
            };

            struct X3D_BiquadFilter* pnode = (struct X3D_BiquadFilter*)node;
            std::shared_ptr<BiquadFilterNode> biquad;
            BiquadFilterNode* biquad_ptr;
            //std::shared_ptr<GainNode> gain;
            //GainNode* gain_ptr;
            if (!srepn->inode) {
                //create labsound node
                //gain = std::make_shared<GainNode>(context);
                //gain_ptr = gain.get();
                //ac->next_node++;
                //ac->nodes[ac->next_node] = gain;
                //ac->nodetype[ac->next_node] = NODE_Gain;
                //srepn->igain = ac->next_node;
                //srepn->icontext = icontext;
                ////connect source node output to parent node input
                //if (iparent.x)
                //    libsound_connect2(icontext, iparent.x, srepn->igain, iparent.y, iparent.z);

                biquad = std::make_shared<BiquadFilterNode>(context);
                biquad_ptr = biquad.get();
                if (nondefault_channelinterp) {

                    ChannelInterpretation interp;
                    ChannelCountMode cmode;
                    getChannelInterpretation(pnode->channelInterpretation->strptr, pnode->channelCountMode->strptr, &interp, &cmode);
                    biquad_ptr->setChannelInterpretation(interp);
                    {
                        ContextGraphLock g(ac->context.get(), "ex_simple");
                        biquad_ptr->setChannelCountMode(g, cmode);
                    }
                }
                ac->next_node++;
                ac->nodes[ac->next_node] = biquad;
                ac->nodetype[ac->next_node] = NODE_BiquadFilter;
                srepn->inode = ac->next_node;
                srepn->icontext = icontext;
                //connect source node output to parent node input
                //libsound_connect0(icontext, srepn->igain, srepn->inode);
                //if (iparent.x)
                //    libsound_connect2(icontext, iparent.x, srepn->inode, iparent.y, iparent.z);


            }
            //gain_ptr = dynamic_cast<GainNode*>(ac->nodes[srepn->igain].get());
            ////copy changed values from x3d to labsound
            //gain_ptr->gain()->setValue(pnode->gain);
            biquad_ptr = dynamic_cast<BiquadFilterNode*>(ac->nodes[srepn->inode].get());
            FilterType biquad_type = (FilterType)name_lookup(pnode->type->strptr, biquad_types);
            biquad_ptr->setType(biquad_type);
            biquad_ptr->detune()->setValue(pnode->detune);
            biquad_ptr->frequency()->setValue(pnode->frequency);
            biquad_ptr->q()->setValue(pnode->qualityFactor);  //.Q is quality factor
        }
        break;
        case NODE_DynamicsCompressor:
        {
            struct X3D_DynamicsCompressor* pnode = (struct X3D_DynamicsCompressor*)node;
            std::shared_ptr<DynamicsCompressorNode> dynamics;
            DynamicsCompressorNode* dynamics_ptr;
            //std::shared_ptr<GainNode> gain;
            //GainNode* gain_ptr;
            if (!srepn->inode) {
                ////create labsound node
                //gain = std::make_shared<GainNode>(context);
                //gain_ptr = gain.get();
                //ac->next_node++;
                //ac->nodes[ac->next_node] = gain;
                //ac->nodetype[ac->next_node] = NODE_Gain;
                //srepn->igain = ac->next_node;
                //srepn->icontext = icontext;
                ////connect source node output to parent node input
                //if (iparent.x)
                //    libsound_connect2(icontext, iparent.x, srepn->igain, iparent.y, iparent.z);

                dynamics = std::make_shared<DynamicsCompressorNode>(context);
                dynamics_ptr = dynamics.get();
                if (nondefault_channelinterp) {

                    ChannelInterpretation interp;
                    ChannelCountMode cmode;
                    getChannelInterpretation(pnode->channelInterpretation->strptr, pnode->channelCountMode->strptr, &interp, &cmode);
                    dynamics_ptr->setChannelInterpretation(interp);
                    {
                        ContextGraphLock g(ac->context.get(), "ex_simple");
                        dynamics_ptr->setChannelCountMode(g, cmode);
                    }
                }
                ac->next_node++;
                ac->nodes[ac->next_node] = dynamics;
                ac->nodetype[ac->next_node] = NODE_DynamicsCompressor; 
                srepn->inode = ac->next_node;
                srepn->icontext = icontext;
                //connect source node output to parent node input
                //libsound_connect0(icontext, srepn->igain, srepn->inode);
                //if (iparent.x)
                //    libsound_connect2(icontext, iparent.x, srepn->inode, iparent.y, iparent.z);


            }
            //gain_ptr = dynamic_cast<GainNode*>(ac->nodes[srepn->igain].get());
            ////copy changed values from x3d to labsound
            //gain_ptr->gain()->setValue(pnode->gain);
            dynamics_ptr = dynamic_cast<DynamicsCompressorNode*>(ac->nodes[srepn->inode].get());
            dynamics_ptr->attack()->setValue((float)pnode->attack);
            dynamics_ptr->knee()->setValue(pnode->knee);
            dynamics_ptr->ratio()->setValue(pnode->ratio);
            dynamics_ptr->release()->setValue((float)pnode->release);
            dynamics_ptr->threshold()->setValue(pnode->threshold);
            pnode->reduction = dynamics_ptr->reduction()->value();

        }
        break;
        case NODE_WaveShaper:
        {
            struct X3D_WaveShaper* pnode = (struct X3D_WaveShaper*)node;
            std::shared_ptr<WaveShaperNode> wave;
            WaveShaperNode* wave_ptr;
            //std::shared_ptr<GainNode> gain;
            //GainNode* gain_ptr;
            if (!srepn->inode) {
                ////create labsound node
                //gain = std::make_shared<GainNode>(context);
                //gain_ptr = gain.get();
                //ac->next_node++;
                //ac->nodes[ac->next_node] = gain;
                //ac->nodetype[ac->next_node] = NODE_Gain;
                //srepn->igain = ac->next_node;
                //srepn->icontext = icontext;
                ////connect source node output to parent node input
                //if (iparent.x)
                //    libsound_connect2(icontext, iparent.x, srepn->igain, iparent.y, iparent.z);

                wave = std::make_shared<WaveShaperNode>(context);
                wave_ptr = wave.get();
                if (nondefault_channelinterp) {

                    ChannelInterpretation interp;
                    ChannelCountMode cmode;
                    getChannelInterpretation(pnode->channelInterpretation->strptr, pnode->channelCountMode->strptr, &interp, &cmode);
                    wave_ptr->setChannelInterpretation(interp);
                    {
                        ContextGraphLock g(ac->context.get(), "ex_simple");
                        wave_ptr->setChannelCountMode(g, cmode);
                    }
                }
                ac->next_node++;
                ac->nodes[ac->next_node] = wave;
                ac->nodetype[ac->next_node] = NODE_WaveShaper;
                srepn->inode = ac->next_node;
                srepn->icontext = icontext;
                //connect source node output to parent node input
                //libsound_connect0(icontext, srepn->igain, srepn->inode);
                //if (iparent.x)
                //    libsound_connect2(icontext, iparent.x, srepn->inode, iparent.y, iparent.z);

            }
            //gain_ptr = dynamic_cast<GainNode*>(ac->nodes[srepn->igain].get());
            ////copy changed values from x3d to labsound
            //gain_ptr->gain()->setValue(pnode->gain);
            wave_ptr = dynamic_cast<WaveShaperNode*>(ac->nodes[srepn->inode].get());
            std::vector<float> curve(pnode->curve.n);
            printf("pnode->curve.n %d p[0] %f p[44100-1] %f", pnode->curve.n, pnode->curve.p[0], pnode->curve.p[pnode->curve.n-1]);
            for (int i = 0; i < pnode->curve.n; i++)
                curve[i] = pnode->curve.p[i];
            printf("curve[0] %f curve[-1] %f", curve[0], curve[curve.size() - 1]);
            wave_ptr->setCurve(curve);
//UNFINISHED     //wave_ptr->oversample()-> there doesn't seem to be an oversample in Labsound, posted an issue Feb 6,2023

        }
        break;
        case NODE_Convolver:
        {
            struct X3D_Convolver * pnode = (struct X3D_Convolver*)node;
            std::shared_ptr<ConvolverNode> convolver;
            ConvolverNode* convolver_ptr;
            //std::shared_ptr<GainNode> gain;
            //GainNode* gain_ptr;
            if (!srepn->inode) {
                ////create labsound node
                //gain = std::make_shared<GainNode>(context);
                //gain_ptr = gain.get();
                //ac->next_node++;
                //ac->nodes[ac->next_node] = gain;
                //ac->nodetype[ac->next_node] = NODE_Gain;
                //srepn->igain = ac->next_node;
                //srepn->icontext = icontext;
                ////connect source node output to parent node input
                //if (iparent.x)
                //    libsound_connect2(icontext, iparent.x, srepn->igain, iparent.y, iparent.z);

                convolver = std::make_shared<ConvolverNode>(context);
                convolver_ptr = convolver.get();
                if (nondefault_channelinterp) {

                    ChannelInterpretation interp;
                    ChannelCountMode cmode;
                    getChannelInterpretation(pnode->channelInterpretation->strptr, pnode->channelCountMode->strptr, &interp, &cmode);
                    convolver_ptr->setChannelInterpretation(interp);
                    {
                        ContextGraphLock g(ac->context.get(), "ex_simple");
                        convolver_ptr->setChannelCountMode(g, cmode);
                    }
                }
                ac->next_node++;
                ac->nodes[ac->next_node] = convolver;
                ac->nodetype[ac->next_node] = NODE_Convolver;
                srepn->inode = ac->next_node;
                srepn->icontext = icontext;
                //connect source node output to parent node input
                //libsound_connect0(icontext, srepn->igain, srepn->inode);
                //if (iparent.x)
                //    libsound_connect2(icontext, iparent.x, srepn->inode, iparent.y, iparent.z);

            }
            //gain_ptr = dynamic_cast<GainNode*>(ac->nodes[srepn->igain].get());
            ////copy changed values from x3d to labsound
            //gain_ptr->gain()->setValue(pnode->gain);
            convolver_ptr = dynamic_cast<ConvolverNode*>(ac->nodes[srepn->inode].get());
            convolver_ptr->setNormalize(pnode->normalize);
            if (pnode->buffer) {
                struct X3D_AudioBuffer* abuf = (struct X3D_AudioBuffer*)pnode->buffer;
                if (abuf->__sourceNumber > 0)
                    convolver_ptr->setImpulse(busses[abuf->__sourceNumber]); // or srep->ibuffer
            }
        }
        break;
        case NODE_MicrophoneSource:
        {
            struct X3D_MicrophoneSource* pnode = (struct X3D_MicrophoneSource*)node;
            std::shared_ptr<AudioHardwareInputNode> input;
            AudioHardwareInputNode* input_ptr;
            if (!srepn->inode) {
                //create labsound node
                {
                    ContextRenderLock r(ac->context.get(), "microphone");
                    input = lab::MakeAudioHardwareInputNode(r);
                    ac->context.get()->connect(ac->context.get()->device(), input, 0, 0);
                }

                ac->next_node++;
                ac->nodes[ac->next_node] = input;
                ac->nodetype[ac->next_node] = NODE_MicrophoneSource;
                srepn->inode = ac->next_node;
                srepn->icontext = icontext;
                //if (iparent.x)
                //    libsound_connect2(icontext, iparent.x, srepn->inode, iparent.y, iparent.z);
            }
            //copy changed values from x3d to labsound
            input_ptr = static_cast<AudioHardwareInputNode*>(ac->nodes[srepn->inode].get());
        }
        break;

        //case NODE_AudioDestination:
        //{
        //    struct X3D_AudioDestinationn* pnode = (struct X3D_AudioDestination*)node;
        //    if (!srepn->inode) {
        //        //create labsound node
        //    }

        //}
        //break;

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