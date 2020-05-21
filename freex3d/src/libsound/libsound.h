#pragma once

#if !defined(_WIN32)
#define EXPORT_DLL
#elif !defined(EXPORT_DLL)
#if defined(_LIB)
#define EXPORT_DLL
#elif defined(_USRDLL)
#define EXPORT_DLL __declspec(dllexport)
#else
#define EXPORT_DLL __declspec(dllimport)
#endif
#endif /* _WIN32 && EXPORT_DLL */
enum {
	AN_AudioClip = 1,
	AN_AudioBuffer,
	AN_AudioBufferSourceNode,
	AN_GainNode,
	AN_OscillatorNode,
	AN_AudioDestinationNode,
};
EXPORT_DLL extern void libsound_testNoise();
EXPORT_DLL extern void* libsound_createContext();
EXPORT_DLL extern void* libsound_createNode(void *context, int type);
EXPORT_DLL extern void libsound_connect(void* context, void *destination, void *source);