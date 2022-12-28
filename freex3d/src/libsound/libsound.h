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
struct X3D_SoundRep {
	int itype; //=5, 0 PointRep 1 LineRep 2 PolyRep 3 MeshRep 4 TextureRep 5 LightRep 6 ProjectorRep 7 SoundRep
	int icontext; //map audio_contexts[icontext] = lab context
	int inode; //map nodes[inode] = lab node
	//int inodetype; // x3d 1:1 labsound nodes, shouldn't need if know X3DNode->_nodeType
	//int iparent; //x3d parent.inode should become destination labnode
};

enum {
	AN_AudioClip = 1,
	AN_AudioBuffer,
	AN_AudioBufferSourceNode,
	AN_GainNode,
	AN_OscillatorNode,
	AN_AudioDestinationNode,
};
EXPORT_DLL extern struct X3D_SoundRep* getSoundRep(struct X3D_Node* pnode);
EXPORT_DLL extern void libsound_testNoise();
EXPORT_DLL extern void* libsound_createContext();
EXPORT_DLL extern int libsound_createBusFromBuffer(char* bbuffer, int len);
EXPORT_DLL extern void* libsound_createNode(void *context, int type);
EXPORT_DLL extern void libsound_connect(void* context, void *destination, void *source);