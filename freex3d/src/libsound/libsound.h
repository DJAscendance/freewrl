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
typedef struct ivec3 { int x; int y; int z; } ivec3;
typedef struct icset { int p; int d; int ld; int n; int s; int ls; } icset;
typedef struct ivec2 { int x; int y; } ivec2;
typedef struct ivec4 { int x; int y; int z; int w; } ivec4;
struct X3D_SoundRep {
	int itype; //=5, 0 PointRep 1 LineRep 2 PolyRep 3 MeshRep 4 TextureRep 5 LightRep 6 ProjectorRep 7 SoundRep
	int icontext; //map audio_contexts[icontext] = lab context
	int inode; //map nodes[inode] = lab node
	int igain; //just for nodes that need a separate gain node (Sound)
	//int inodetype; // x3d 1:1 labsound nodes, shouldn't need if know X3DNode->_nodeType
	//int iparent; //x3d parent.inode should become destination labnode
	unsigned int iframe; //last frame visited on scenegraph traversal
	int ibuffer; //just for source nodes with a buffer, like audioclip
	float dopplerFactor; //used by AudioClip (computed by SpatialSound in libsound.cpp)
	void * connections;
	int last_indexSource[10];
	int last_indexDestination[10];
	int last_count;
};

//enum {
//	AN_AudioClip = 1,
//	AN_AudioBuffer,
//	AN_AudioBufferSourceNode,
//	AN_GainNode,
//	AN_OscillatorNode,
//	AN_AudioDestinationNode,
//};
EXPORT_DLL extern struct X3D_SoundRep* getSoundRep(struct X3D_Node* pnode);
EXPORT_DLL extern void libsound_updateNode3(int icontext, icset connect_parent, struct X3D_Node* node);
EXPORT_DLL extern void libsound_pauseContext0(int icontext);
EXPORT_DLL extern void libsound_resumeContext0(int icontext);
EXPORT_DLL extern void libsound_pauseNode0(struct X3D_Node* node);
EXPORT_DLL extern void libsound_resumeNode0(struct X3D_Node* node);
EXPORT_DLL extern void libsound_testNoise();
EXPORT_DLL extern int libsound_createContext0();
EXPORT_DLL extern int libsound_createBusFromBuffer0(char* bbuffer, int len);
EXPORT_DLL extern int libsound_createBusFromPCM32(float* bbuffer, int nchannel, int lentotal);
EXPORT_DLL extern int libsound_createBusFromFile0(char* url);
EXPORT_DLL extern double libsound_computeDuration0(int ibuffer);
EXPORT_DLL extern void libsound_print_connections();
EXPORT_DLL extern void libsound_connect(int icontext, icset iparent); 
EXPORT_DLL extern void libsound_disconnect(int icontext, icset iparent);

//EXPORT_DLL extern void* libsound_createNode(void *context, int type);
//EXPORT_DLL extern void libsound_connect(void* context, void *destination, void *source);