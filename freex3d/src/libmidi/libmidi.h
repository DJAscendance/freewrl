
typedef struct icset { int p; int d; int ld; int n; int s; int ls; } icset;
typedef struct ivec3 { int x; int y; int z; } ivec3;
struct X3D_MidiRep {
	int itype; //==8, 0 PointRep 1 LineRep 2 PolyRep 3 MeshRep 4 TextureRep 5 LightRep 6 ProjectorRep 7 SoundRep 8 MidiRep
	int icontext; //map audio_contexts[icontext] = libmidi context
	int inode; //map nodes[inode] = libmidi node
	unsigned int iframe; //last frame visited on scenegraph traversal
	int ibuffer; //just for source nodes with a buffer, like MIDIFileSource
	void* connections;
	int last_indexSource[10];
	int last_indexDestination[10];
	int last_count;
};
/*
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
#endif // _WIN32 && EXPORT_DLL
*/
#define EXPORT_DLL   //UN-DEFINE IT, WE AREN'T USING A LIB AT ALL

EXPORT_DLL extern int libmidi_createContext0();
EXPORT_DLL extern void libmidi_updateNode3(int icontext, icset connect_parent, struct X3D_Node* node);
EXPORT_DLL extern void libmidi_pauseContext0(int icontext);
EXPORT_DLL extern void libmidi_resumeContext0(int icontext);
EXPORT_DLL extern void libmidi_print_connections();
EXPORT_DLL extern void libmidi_connect(int icontext, icset iparent);
EXPORT_DLL extern void libmidi_disconnect(int icontext, icset iparent);


 //   int libmidi_createContext0();
	//void libmidi_pauseContext0(int icontext);
	//void libmidi_resumeContext0(int icontext);
	//void libmidi_print_connections();
	//void libmidi_connect(int icontext, icset iparent);
	//void libmidi_disconnect(int icontext, icset iparent);

//extern int libmidi_createContext0();