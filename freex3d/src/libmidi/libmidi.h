
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
typedef unsigned char ubyte;

//MIDI 2 64 bit packet, a few ways to slice it
typedef union {
	double packet;
	unsigned int uint[2];
	unsigned short u16[4];
	unsigned char bytes[8];
} UMP;
typedef struct timedpacket {
	double packet;
	double timestamp;
} timedpacket;
int MIDITransport();
#define MIDI_UMP 2
#define MIDI_MSG 1
enum message_type
{
	INVALID = 0x0,
	// Standard Message
	NOTE_OFF = 0x80,
	NOTE_ON = 0x90,
	POLY_PRESSURE = 0xA0,
	CONTROL_CHANGE = 0xB0,
	PROGRAM_CHANGE = 0xC0, 
	AFTERTOUCH = 0xD0, //aka CHANNEL_PRESSURE
	CHANNEL_PRESSURE = 0xD0, //aka AFTERTOUCH
	PITCH_BEND = 0xE0,

	// System Common Messages
	SYSTEM_EXCLUSIVE = 0xF0,
	TIME_CODE = 0xF1,
	SONG_POS_POINTER = 0xF2,
	SONG_SELECT = 0xF3,
	RESERVED1 = 0xF4,
	RESERVED2 = 0xF5,
	TUNE_REQUEST = 0xF6,
	EOX = 0xF7,

	// System Realtime Messages
	TIME_CLOCK = 0xF8,
	RESERVED3 = 0xF9,
	START = 0xFA,
	CONTINUE = 0xFB,
	STOP = 0xFC,
	RESERVED4 = 0xFD,
	ACTIVE_SENSING = 0xFE,
	SYSTEM_RESET = 0xFF
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