
#define FWVER "1.22.13"

#undef FW_DEBUG

#define FREEWRL_STEREO_RENDERING 1

#define FRONTEND_DOES_SNAPSHOTS 1
#define FRONTEND_HANDLES_DISPLAY_THREAD 1
//not now for desktop, using desktop.c:
//#define FRONTEND_GETS_FILES 1

#define STATUSBAR_HUD 1
#define DO_COLLISION_GPU 1

#define FONTS_DIR "/Applications/FreeWRL/fonts"

#define HAVE_DIRENT_H 1
#define HAVE_STDARG_H 1
#define HAVE_ERRNO_H 1
#define HAVE_MEMCPY 1
#define HAVE_GETTIMEOFDAY 1
#define AQUA 1
#define TARGET_AQUA 1
#define XP_UNIX 1
#define HAVE_STDINT_H 1
#define HAVE_STDBOOL_H 1
#define HAVE_UNISTD_H 1
//Apple deprecated OpenGL in 10.14 but still ships 4.1 core; FreeWRL uses it knowingly
#define GL_SILENCE_DEPRECATION 1
//standard headers: older SDKs pulled these in transitively (e.g. via AGL), modern ones do not
#define STDC_HEADERS 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_CTYPE_H 1
#define HAVE_STRING_H 1
#define HAVE_LIMITS_H 1
#define HAVE_MATH_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TIME_H 1
#define HAVE_TIME_H 1
#define HAVE_FCNTL_H 1
#define HAVE_STDLIB_H 1
#define HAVE_INTTYPES_H 1
#define HAVE_SCHED_H 1
#define HAVE_SYS_SOCKET_H 1
#define HAVE_STRNLEN 1
#define HAVE_STRNDUP 1
#define HAVE_SYS_WAIT_H 1
#define HAVE_PTHREAD 1
#define HAVE_GETOPT_H 1
#define HAVE_GLU_TESS 1
#define STATUSBAR_STD 1
#define FREEWRL_MESSAGE_WRAPPER "/usr/bin/say"
#define	BROWSER		"/usr/bin/open"
#define PACKAGE_BUGREPORT "freewrl.sf.net"
#define HAVE_GETOPT_LONG 1
//the following can be undefed when starting to build on a clean machine
//libcurl is part of osx (for pulling http files)
#define HAVE_LIBCURL 1
//duktape js engine is compiled into freewrl project, nothing to download or install
#define JAVASCRIPT_DUK 1
//macports: sudo port -t install alut (will also install openal)
//image loading (jpg/png/gif textures): the bundled stb_image (opengl/stb_image.h); no Imlib2
//#define HAVE_IMLIB2 1
#define HAVE_ALUT 1
#define HAVE_OPENAL 1
//macports: sudo port -t install ffmpeg
//#define MOVIETEXTURE_FFMPEG 1 //Homebrew ffmpeg 8 lacks the ffmpeg-4 APIs MPEG_Utils_ffmpeg.c uses
//macports: sudo port -t install ode
#define WITH_RBP 1
//nurbs is part of glu in osx opengl, just define
#define NURBS_LIB 1
//spidermonkey aka mozjs17 via macports -on commandline can switch to duk with -J duk (or to SM with -J sm2)
//#define JAVASCRIPT_SM 1 //no mozjs17 in Homebrew; use bundled duktape
#define JAVASCRIPT_ENGINE_VARIANT 1 //1 = SM1 2 = SM2 - (default in libfreewrl is 2, mozjs17 not GCing so SM1 is better on mac

