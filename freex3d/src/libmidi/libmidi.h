
typedef struct icset { int p; int d; int ld; int n; int s; int ls; } icset;
typedef struct ivec3 { int x; int y; int z; } ivec3;

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