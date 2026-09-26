

// wrapper for labsound.lib
#ifdef HAVE_LABSOUND

#include "LabSound.h"

//using namespace lab;

extern "C" {
#include "fwlabsound.h"

void * flabsound_initialize(){


}

}  //extern C

#endif //HAVE_LABSOUND