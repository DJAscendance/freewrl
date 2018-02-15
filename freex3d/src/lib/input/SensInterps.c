/*


Do Sensors and Interpolators in C, not in perl.

Interps are the "EventsProcessed" fields of interpolators.

*/


/****************************************************************************
    This file is part of the FreeWRL/FreeX3D Distribution.

    Copyright 2009 CRC Canada. (http://www.crc.gc.ca)

    FreeWRL/FreeX3D is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FreeWRL/FreeX3D is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FreeWRL/FreeX3D.  If not, see <http://www.gnu.org/licenses/>.
****************************************************************************/



#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>
#include <list.h>

#include "../vrml_parser/Structs.h"
#include "../input/InputFunctions.h"
#include "../opengl/LoadTextures.h"        /* for finding a texture url in a multi url */


#include "../main/headers.h"
#include "../opengl/OpenGL_Utils.h"
#include "../scenegraph/RenderFuncs.h"

#include "../x3d_parser/Bindable.h"
#include "../scenegraph/LinearAlgebra.h"
#include "../scenegraph/Collision.h"
#include "../scenegraph/quaternion.h"
#include "../scenegraph/sounds.h"
#include "../vrml_parser/CRoutes.h"
#include "../opengl/OpenGL_Utils.h"
#include "../opengl/Textures.h"            /* for finding a texture url in a multi url */

#include "SensInterps.h"



/* when we get a new sound source, what is the number for this? */
//int SoundSourceNumber = 0;
typedef struct pSensInterps{
	int stub;
}* ppSensInterps;
void *SensInterps_constructor(){
	void *v = MALLOCV(sizeof(struct pSensInterps));
	memset(v,0,sizeof(struct pSensInterps));
	return v;
}
void SensInterps_init(struct tSensInterps *t)
{
	//public
	//private
	t->prv = SensInterps_constructor();
	{
		//ppSensInterps p = (ppSensInterps)t->prv;
	}
}


///* function prototypes */
//void locateAudioSource (struct X3D_AudioClip *node);


/* time dependent sensor nodes- check/change activity state */
void do_active_inactive (
	int *act, 		/* pointer to are we active or not?	*/
	double *inittime,	/* pointer to nodes inittime		*/
	double *startt,		/* pointer to nodes startTime		*/
	double *stopt,		/* pointer to nodes stop time		*/
	int loop,		/* nodes loop field			*/
	double myDuration,	/* duration of cycle			*/
	double speed,		/* speed field				*/
	double elapsedTime   /* cumulative non-paused time */
) {

	/* what we do now depends on whether we are active or not */
	/* gcc seemed to have problems mixing double* and floats in a function
	   call, so make them all doubles. Note duplicate printfs in 
	   do_active_inactive call - uncomment and make sure all are identical 
		printf ("called ");
		printf ("act %d ",*act);
		printf ("initt %lf ",*inittime);
		printf ("startt %lf ",*startt);
		printf ("stopt %lf ",*stopt);
		printf ("loop %d ",loop);
		printf ("myDuration %lf ",myDuration);
		printf ("speed %f\n",speed);
	*/
	double ticktime = TickTime(); //changes once per frame, not in here

	if (*act == 1) {   /* active - should we stop? */
		#ifdef SEVERBOSE
		printf ("is active tick %f startt %f stopt %f\n",
				TickTime(), *startt, *stopt);
		#endif

		if (ticktime > *stopt) {
			if (*startt >= *stopt) {
				/* cases 1 and 2 */
				if (!(loop)) {
					/*printf ("case 1 and 2, not loop md %f sp %f fabs %f\n",
							myDuration, speed, fabs(myDuration/speed));
					*/
					
					/* if (speed != 0) */
					if (! APPROX(speed, 0)) {
					    //if (ticktime >= (*startt + fabs(myDuration/speed))) {
					    if (elapsedTime >= fabs(myDuration/speed) ) {
							#ifdef SEVERBOSE
							printf ("stopping case x\n");
							printf ("TickTime() %f\n",ticktime);
							printf ("startt %f\n",*startt);
							printf ("myDuration %f\n",myDuration);
							printf ("speed %f\n",speed);
							#endif

							*act = 0;
							*stopt = ticktime;
					    }
					}
				}
			} else {
				#ifdef SEVERBOSE
				printf ("stopping case z\n");
				#endif

				*act = 0;
				*stopt = ticktime;
			}
		}
	}

	/* immediately process start events; as per spec.  */
	if (*act == 0) {   /* active - should we start? */
		/* printf ("is not active TickTime %f startt %f\n",TickTime(),*startt); */

		if (ticktime >= *startt) {
			/* We just might need to start running */

			if (ticktime >= *stopt) {
				/* lets look at the initial conditions; have not had a stoptime
				event (yet) */

				if (loop) {
					if (*startt >= *stopt) {
						/* VRML standards, table 4.2 case 2 */
						/* printf ("CASE 2\n"); */
						/* Umut Sezen's code: */
						if (!(*startt > 0)) *startt = ticktime;
						*act = 1;
					}
				} else if (*startt >= *stopt) {
					if (*startt > *inittime) {
						/* ie, we have an event */
						 /* printf ("case 1 here\n"); */
						/* we should be running VRML standards, table 4.2 case 1 */
						/* Umut Sezen's code: */
						if (!(*startt > 0)) 
							*startt = ticktime;
						*act = 1;
					}
				}
			} else {
				/* printf ("case 3 here\n"); */
				/* we should be running -
				VRML standards, table 4.2 cases 1 and 2 and 3 */
				/* Umut Sezen's code: */
				if (!(*startt > 0)) *startt = ticktime;
				*act = 1;
			}
		}
	}
}


/* Interpolators - local routine, look for the appropriate key */
int find_key (int kin, float frac, float *keys) {
	int counter;

	for (counter=1; counter <= kin; counter++) {
		if (frac <keys[counter]) {
			return counter;
		}
	}
	return kin;	/* huh? not found! */
}


/* ScalarInterpolators - return only one float */
void do_OintScalar (void *node) {
	/* ScalarInterpolator - store final value in px->value_changed */
	struct X3D_ScalarInterpolator *px;
	int kin, kvin;
	float *kVs;
	int counter;

	if (!node) return;
	px = (struct X3D_ScalarInterpolator *) node;
	kin = px->key.n;
	kvin = px->keyValue.n;
	kVs = px->keyValue.p;

	MARK_EVENT (node, offsetof (struct X3D_ScalarInterpolator, value_changed));

	/* make sure we have the keys and keyValues */
	if ((kvin == 0) || (kin == 0)) {
		px->value_changed = (float) 0.0;
		return;
	}
	if (kin>kvin) kin=kvin; /* means we don't use whole of keyValue, but... */

	#ifdef SEVERBOSE
		printf ("ScalarInterpolator, kin %d kvin %d, vc %f\n",kin,kvin,px->value_changed);
	#endif

	/* set_fraction less than or greater than keys */
	if (px->set_fraction <= px->key.p[0]) {
		 px->value_changed = kVs[0];
	} else if (px->set_fraction >= px->key.p[kin-1]) {
		 px->value_changed = kVs[kvin-1];
	} else {
		/* have to go through and find the key before */
		counter=find_key(kin,(float)(px->set_fraction),px->key.p);
		px->value_changed =
			(px->set_fraction - px->key.p[counter-1]) /
			(px->key.p[counter] - px->key.p[counter-1]) *
			(kVs[counter] - kVs[counter-1]) +
			kVs[counter-1];
	}
}


void do_OintNormal(void *node) {
	struct X3D_NormalInterpolator *px;
	int kin, kvin/* , counter */;
	struct SFVec3f *kVs;
	struct SFVec3f *valchanged;

	int thisone, prevone;	/* which keyValues we are interpolating between */
	int tmp;
	float interval;		/* where we are between 2 values */
	struct point_XYZ normalval;	/* different structures for normalization calls */
	int kpkv; /* keys per key value */
	int indx;
	int myKey;

	if (!node) return;
	px = (struct X3D_NormalInterpolator *) node;


	#ifdef SEVERBOSE
		printf ("debugging OintCoord keys %d kv %d vc %d\n",px->keyValue.n, px->key.n,px->value_changed.n);
	#endif

	MARK_EVENT (node, offsetof (struct X3D_NormalInterpolator, value_changed));

	kin = px->key.n;
	kvin = px->keyValue.n;
	kVs = px->keyValue.p;
	kpkv = kvin/kin;

	/* do we need to (re)allocate the value changed array? */
	if (kpkv != px->value_changed.n) {
		#ifdef SEVERBOSE
		    printf ("refactor valuechanged array. n %d sizeof p %d\n",
			kpkv,sizeof (struct SFColor) * kpkv);
		#endif
		if (px->value_changed.n != 0) {
			FREE_IF_NZ (px->value_changed.p);
		}
		px->value_changed.n = kpkv;
		px->value_changed.p = MALLOC (struct SFVec3f*, sizeof (struct SFVec3f) * kpkv);
	}

	/* shortcut valchanged; have to put it here because might be reMALLOC'd */
	valchanged = px->value_changed.p;


	/* make sure we have the keys and keyValues */
	if ((kvin == 0) || (kin == 0)) {
		#ifdef SEVERBOSE
		printf ("no keys or keyValues yet\n");
		#endif

		for (indx = 0; indx < kpkv; indx++) {
			valchanged[indx].c[0] = (float) 0.0;
			valchanged[indx].c[1] = (float) 0.0;
			valchanged[indx].c[2] = (float) 0.0;
		}
		return;
	}
	if (kin>kvin) kin=kvin; /* means we don't use whole of keyValue, but... */


	#ifdef SEVERBOSE
		printf ("debugging, kpkv %d, px->value_changed.n %d\n", kpkv, px->value_changed.n);
		printf ("NormalInterpolator, kpkv %d\n",kpkv);
	#endif
	

	/* set_fraction less than or greater than keys */
	if (px->set_fraction <= px->key.p[0]) {
		#ifdef SEVERBOSE
		printf ("COINT out1\n");
		#endif

		for (indx = 0; indx < kpkv; indx++) {
			memcpy ((void *)&valchanged[indx],
				(void *)&kVs[indx], sizeof (struct SFColor));
		}
		#ifdef SEVERBOSE
		printf ("COINT out1 copied\n");
		#endif
	} else if (px->set_fraction >= px->key.p[kin-1]) {
		#ifdef SEVERBOSE
		printf ("COINT out2\n");
		#endif

		for (indx = 0; indx < kpkv; indx++) {
			memcpy ((void *)&valchanged[indx],
				(void *)&kVs[kvin-kpkv+indx],
				sizeof (struct SFColor));
		}
		#ifdef SEVERBOSE
		printf ("COINT out2 finished\n");
		#endif
	} else {
		#ifdef SEVERBOSE
		printf ("COINT out3\n");
		#endif

		/* have to go through and find the key before */
		#ifdef SEVERBOSE
		printf ("indx=0, kin %d frac %f\n",kin,px->set_fraction);
		#endif

		myKey=find_key(kin,(float)(px->set_fraction),px->key.p);
		#ifdef SEVERBOSE
		printf ("working on key %d\n",myKey);
		#endif

		/* find the fraction between the 2 values */
		interval = (px->set_fraction - px->key.p[myKey-1]) /
				(px->key.p[myKey] - px->key.p[myKey-1]);

		for (indx = 0; indx < kpkv; indx++) {
			thisone = myKey * kpkv + indx;
			prevone = (myKey-1) * kpkv + indx;

			#ifdef SEVERBOSE
			if (thisone >= kvin) {
				printf ("CoordinateInterpolator error: thisone %d prevone %d indx %d kpkv %d kin %d kvin %d\n",thisone,prevone,
				indx,kpkv,kin,kvin);
			}
			#endif

			for (tmp=0; tmp<3; tmp++) {
				valchanged[indx].c[tmp] = kVs[prevone].c[tmp]  +
						interval * (kVs[thisone].c[tmp] -
							kVs[prevone].c[tmp]);
			}
			#ifdef SEVERBOSE
			printf ("	1 %d interval %f prev %f this %f final %f\n",1,interval,kVs[prevone].c[1],kVs[thisone].c[1],valchanged[indx].c[1]);
			#endif
		}
		#ifdef SEVERBOSE
		printf ("COINT out3 finished\n");
		#endif

	}

	/* if this is a NormalInterpolator... */
	for (indx = 0; indx < kpkv; indx++) {
		normalval.x = valchanged[indx].c[0];
		normalval.y = valchanged[indx].c[1];
		normalval.z = valchanged[indx].c[2];
		normalize_vector(&normalval);
		valchanged[indx].c[0] = (float) normalval.x;
		valchanged[indx].c[1] = (float) normalval.y;
		valchanged[indx].c[2] = (float) normalval.z;
	}
	#ifdef SEVERBOSE
	printf ("Done CoordinateInterpolator\n");
	#endif
}


void do_OintCoord(void *node) {
	struct X3D_CoordinateInterpolator *px;
	int kin, kvin/* , counter */;
	struct SFVec3f *kVs;
	struct SFVec3f *valchanged;

	int thisone, prevone;	/* which keyValues we are interpolating between */
	int tmp;
	float interval;		/* where we are between 2 values */
	int kpkv; /* keys per key value */
	int indx;
	int myKey;

	if (!node) return;
	px = (struct X3D_CoordinateInterpolator *) node;

#ifdef SEVERBOSE
        printf ("do_OintCoord, frac %f toGPU %d toCPU %d\n",px->set_fraction,px->_GPU_Routes_out, px->_CPU_Routes_out);
		printf ("debugging OintCoord keys %d kv %d vc %d\n",px->keyValue.n, px->key.n,px->value_changed.n);
	#endif

	MARK_EVENT (node, offsetof (struct X3D_CoordinateInterpolator, value_changed));

    // create the VBOs if required, for running on the GPU
    if (px->_GPU_Routes_out > 0) {
        if (px->_keyVBO==0) {
            glGenBuffers(1,(GLuint *)&px->_keyValueVBO);
            glGenBuffers(1,(GLuint *)&px->_keyVBO);
            FW_GL_BINDBUFFER(GL_ARRAY_BUFFER,px->_keyValueVBO);
            printf ("genning buffer data for %d keyValues, total floats %d\n",px->keyValue.n, px->keyValue.n*3);
            glBufferData(GL_ARRAY_BUFFER,px->keyValue.n *sizeof(float)*3,px->keyValue.p, GL_STATIC_DRAW);
            
            FW_GL_BINDBUFFER(GL_ARRAY_BUFFER,px->_keyVBO);
            glBufferData(GL_ARRAY_BUFFER,px->key.n *sizeof(float),px->key.p, GL_STATIC_DRAW);
            printf ("created VBOs for the CoordinateInterpolator, they are %d and %d\n",
                    px->_keyValueVBO, px->_keyVBO);
        }
    }
    

    
    if (px->_CPU_Routes_out == 0) {
        #ifdef SEVERBOSE
        printf ("do_OintCoord, no CPU routes out, no need to do this work\n");
        #endif
        return;
    }

    //MARK_EVENT (node, offsetof (struct X3D_CoordinateInterpolator, value_changed));

    
	kin = px->key.n;
	kvin = px->keyValue.n;
	kVs = px->keyValue.p;
	kpkv = kvin/kin;

	/* do we need to (re)allocate the value changed array? */
	if (kpkv != px->value_changed.n) {
		#ifdef SEVERBOSE
		    printf ("refactor valuechanged array. n %d sizeof p %d\n",
			kpkv,sizeof (struct SFVec3f) * kpkv);
		#endif
		if (px->value_changed.n != 0) {
			FREE_IF_NZ (px->value_changed.p);
		}
		px->value_changed.n = kpkv;
		px->value_changed.p = MALLOC (struct SFVec3f*, sizeof (struct SFVec3f) * kpkv);
	}

	/* shortcut valchanged; have to put it here because might be reMALLOC'd */
	valchanged = px->value_changed.p;


	/* make sure we have the keys and keyValues */
	if ((kvin == 0) || (kin == 0)) {
		#ifdef SEVERBOSE
		printf ("no keys or keyValues yet\n");
		#endif

		for (indx = 0; indx < kpkv; indx++) {
			valchanged[indx].c[0] = (float) 0.0;
			valchanged[indx].c[1] = (float) 0.0;
			valchanged[indx].c[2] = (float) 0.0;
		}
		return;
	}
	if (kin>kvin) kin=kvin; /* means we don't use whole of keyValue, but... */


	#ifdef SEVERBOSE
		printf ("debugging, kpkv %d, px->value_changed.n %d\n", kpkv, px->value_changed.n);
		printf ("CoordinateInterpolator, kpkv %d\n",kpkv);
	#endif
	

	/* set_fraction less than or greater than keys */
	if (px->set_fraction <= px->key.p[0]) {
		#ifdef SEVERBOSE
		printf ("COINT out1\n");
		#endif

		for (indx = 0; indx < kpkv; indx++) {
			memcpy ((void *)&valchanged[indx],
				(void *)&kVs[indx], sizeof (struct SFVec3f));
			/* JAS valchanged[indx].c[0] = kVs[indx].c[0]; */
			/* JAS valchanged[indx].c[1] = kVs[indx].c[1]; */
			/* JAS valchanged[indx].c[2] = kVs[indx].c[2]; */
		}
		#ifdef SEVERBOSE
		printf ("COINT out1 copied\n");
		#endif
	} else if (px->set_fraction >= px->key.p[kin-1]) {
		#ifdef SEVERBOSE
		printf ("COINT out2\n");
		#endif

		for (indx = 0; indx < kpkv; indx++) {
			memcpy ((void *)&valchanged[indx],
				(void *)&kVs[kvin-kpkv+indx],
				sizeof (struct SFVec3f));
		}
		#ifdef SEVERBOSE
		printf ("COINT out2 finished\n");
		#endif
	} else {
		#ifdef SEVERBOSE
		printf ("COINT out3\n");
		#endif

		/* have to go through and find the key before */
		#ifdef SEVERBOSE
		printf ("indx=0, kin %d frac %f\n",kin,px->set_fraction);
		#endif

		myKey=find_key(kin,(float)(px->set_fraction),px->key.p);
		#ifdef SEVERBOSE
		printf ("working on key %d\n",myKey);
		#endif

		/* find the fraction between the 2 values */
		interval = (px->set_fraction - px->key.p[myKey-1]) /
				(px->key.p[myKey] - px->key.p[myKey-1]);

		for (indx = 0; indx < kpkv; indx++) {
			thisone = myKey * kpkv + indx;
			prevone = (myKey-1) * kpkv + indx;

			#ifdef SEVERBOSE
			if (thisone >= kvin) {
				printf ("CoordinateInterpolator error: thisone %d prevone %d indx %d kpkv %d kin %d kvin %d\n",thisone,prevone,
				indx,kpkv,kin,kvin);
			}
			#endif

			for (tmp=0; tmp<3; tmp++) {
				valchanged[indx].c[tmp] = kVs[prevone].c[tmp]  +
						interval * (kVs[thisone].c[tmp] -
							kVs[prevone].c[tmp]);
			}
			#ifdef SEVERBOSE
			printf ("	1 %d interval %f prev %f this %f final %f\n",1,interval,kVs[prevone].c[1],kVs[thisone].c[1],valchanged[indx].c[1]);
			#endif
		}
		#ifdef SEVERBOSE
		printf ("COINT out3 finished\n");
		#endif

	}

	#ifdef SEVERBOSE
	printf ("Done CoordinateInterpolator\n");
	#endif
}

void do_OintCoord2D(void *node) {
	struct X3D_CoordinateInterpolator2D *px;
	int kin, kvin/* , counter */;
	struct SFVec2f *kVs;
	struct SFVec2f *valchanged;

	int thisone, prevone;	/* which keyValues we are interpolating between */
	int tmp;
	float interval;		/* where we are between 2 values */
	int kpkv; /* keys per key value */
	int indx;
	int myKey;

	if (!node) return;
	px = (struct X3D_CoordinateInterpolator2D *) node;


	#ifdef SEVERBOSE
		printf ("debugging OintCoord keys %d kv %d vc %d\n",px->keyValue.n, px->key.n,px->value_changed.n);
	#endif

	MARK_EVENT (node, offsetof (struct X3D_CoordinateInterpolator2D, value_changed));

	kin = px->key.n;
	kvin = px->keyValue.n;
	kVs = px->keyValue.p;
	kpkv = kvin/kin;

	/* do we need to (re)allocate the value changed array? */
	if (kpkv != px->value_changed.n) {
		#ifdef SEVERBOSE
		    printf ("refactor valuechanged array. n %d sizeof p %d\n",
			kpkv,sizeof (struct SFVec2f) * kpkv);
		#endif
		if (px->value_changed.n != 0) {
			FREE_IF_NZ (px->value_changed.p);
		}
		px->value_changed.n = kpkv;
		px->value_changed.p = MALLOC (struct SFVec2f*, sizeof (struct SFVec2f) * kpkv);
	}

	/* shortcut valchanged; have to put it here because might be reMALLOC'd */
	valchanged = px->value_changed.p;


	/* make sure we have the keys and keyValues */
	if ((kvin == 0) || (kin == 0)) {
		#ifdef SEVERBOSE
		printf ("no keys or keyValues yet\n");
		#endif

		for (indx = 0; indx < kpkv; indx++) {
			valchanged[indx].c[0] = (float) 0.0;
			valchanged[indx].c[1] = (float) 0.0;
		}
		return;
	}
	if (kin>kvin) kin=kvin; /* means we don't use whole of keyValue, but... */


	#ifdef SEVERBOSE
		printf ("debugging, kpkv %d, px->value_changed.n %d\n", kpkv, px->value_changed.n);
		printf ("CoordinateInterpolator2D, kpkv %d\n",kpkv);
	#endif
	

	/* set_fraction less than or greater than keys */
	if (px->set_fraction <= px->key.p[0]) {
		#ifdef SEVERBOSE
		printf ("COINT out1\n");
		#endif

		for (indx = 0; indx < kpkv; indx++) {
			memcpy ((void *)&valchanged[indx],
				(void *)&kVs[indx], sizeof (struct SFVec2f));
			/* JAS valchanged[indx].c[0] = kVs[indx].c[0]; */
			/* JAS valchanged[indx].c[1] = kVs[indx].c[1]; */
		}
		#ifdef SEVERBOSE
		printf ("COINT out1 copied\n");
		#endif
	} else if (px->set_fraction >= px->key.p[kin-1]) {
		#ifdef SEVERBOSE
		printf ("COINT out2\n");
		#endif

		for (indx = 0; indx < kpkv; indx++) {
			memcpy ((void *)&valchanged[indx],
				(void *)&kVs[kvin-kpkv+indx],
				sizeof (struct SFVec2f));
		}
		#ifdef SEVERBOSE
		printf ("COINT out2 finished\n");
		#endif
	} else {
		#ifdef SEVERBOSE
		printf ("COINT out3\n");
		#endif

		/* have to go through and find the key before */
		#ifdef SEVERBOSE
		printf ("indx=0, kin %d frac %f\n",kin,px->set_fraction);
		#endif

		myKey=find_key(kin,(float)(px->set_fraction),px->key.p);
		#ifdef SEVERBOSE
		printf ("working on key %d\n",myKey);
		#endif

		/* find the fraction between the 2 values */
		interval = (px->set_fraction - px->key.p[myKey-1]) /
				(px->key.p[myKey] - px->key.p[myKey-1]);

		for (indx = 0; indx < kpkv; indx++) {
			thisone = myKey * kpkv + indx;
			prevone = (myKey-1) * kpkv + indx;

			#ifdef SEVERBOSE
			if (thisone >= kvin) {
				printf ("CoordinateInterpolator2D error: thisone %d prevone %d indx %d kpkv %d kin %d kvin %d\n",thisone,prevone,
				indx,kpkv,kin,kvin);
			}
			#endif

			for (tmp=0; tmp<2; tmp++) {
				valchanged[indx].c[tmp] = kVs[prevone].c[tmp]  +
						interval * (kVs[thisone].c[tmp] -
							kVs[prevone].c[tmp]);
			}
		}
		#ifdef SEVERBOSE
		printf ("COINT out3 finished\n");
		#endif

	}

	#ifdef SEVERBOSE
	printf ("Done CoordinateInterpolator2D\n");
	#endif
}

void do_OintPos2D(void *node) {
/* PositionInterpolator2D				 		*/
/* Called during the "events_processed" section of the event loop,	*/
/* so this is called ONLY when there is something required to do, thus	*/
/* there is no need to look at whether it is active or not		*/

	struct X3D_PositionInterpolator2D *px;
	int kin, kvin, counter, tmp;
	struct SFVec2f *kVs;

	if (!node) return;
	px = (struct X3D_PositionInterpolator2D *) node;

	MARK_EVENT (node, offsetof (struct X3D_PositionInterpolator2D, value_changed));

	kin = px->key.n;
	kvin = px->keyValue.n;
	kVs = px->keyValue.p;

	#ifdef SEVERBOSE
		printf("do_Oint2: Position interp2D, node %u kin %d kvin %d set_fraction %f\n",
			   node, kin, kvin, px->set_fraction);
	#endif

	/* make sure we have the keys and keyValues */
	if ((kvin == 0) || (kin == 0)) {
		px->value_changed.c[0] = (float) 0.0;
		px->value_changed.c[1] = (float) 0.0;
		return;
	}
	if (kin>kvin) kin=kvin; /* means we don't use whole of keyValue, but... */


	/* set_fraction less than or greater than keys */
	if (px->set_fraction <= ((px->key).p[0])) {
		memcpy ((void *)&px->value_changed,
				(void *)&kVs[0], sizeof (struct SFVec2f));
	} else if (px->set_fraction >= px->key.p[kin-1]) {
		memcpy ((void *)&px->value_changed,
				(void *)&kVs[kvin-1], sizeof (struct SFVec2f));
	} else {
		/* have to go through and find the key before */
		counter = find_key(kin,((float)(px->set_fraction)),px->key.p);
		for (tmp=0; tmp<2; tmp++) {
			px->value_changed.c[tmp] =
				(px->set_fraction - px->key.p[counter-1]) /
				(px->key.p[counter] - px->key.p[counter-1]) *
				(kVs[counter].c[tmp] -
					kVs[counter-1].c[tmp]) +
				kVs[counter-1].c[tmp];
		}
	}
	#ifdef SEVERBOSE
	printf ("Pos/Col, new value (%f %f)\n",
		px->value_changed.c[0],px->value_changed.c[1]);
	#endif
}

/* PositionInterpolator, ColorInterpolator, GeoPositionInterpolator	*/
/* Called during the "events_processed" section of the event loop,	*/
/* so this is called ONLY when there is something required to do, thus	*/
/* there is no need to look at whether it is active or not		*/

/* GeoPositionInterpolator in the Component_Geospatial file */

/* ColorInterpolator == PositionIterpolator */
void do_ColorInterpolator (void *node) {
	struct X3D_ColorInterpolator *px;
	int kin, kvin, counter, tmp;
	struct SFColor *kVs; 

	if (!node) return;
	px = (struct X3D_ColorInterpolator *) node;

	kvin = px->keyValue.n;
	kVs = px->keyValue.p;
	kin = px->key.n;

	MARK_EVENT (node, offsetof (struct X3D_ColorInterpolator, value_changed)); 

	#ifdef SEVERBOSE
		printf("do_ColorInt: Position/Color interp, node %u kin %d kvin %d set_fraction %f\n",
			   node, kin, kvin, px->set_fraction);
	#endif

	/* make sure we have the keys and keyValues */
	if ((kvin == 0) || (kin == 0)) {
		px->value_changed.c[0] = (float) 0.0;
		px->value_changed.c[1] = (float) 0.0;
		px->value_changed.c[2] = (float) 0.0;
		return;
	}

	if (kin>kvin) kin=kvin; /* means we don't use whole of keyValue, but... */

	/* set_fraction less than or greater than keys */
	if (px->set_fraction <= ((px->key).p[0])) {
		memcpy ((void *)&px->value_changed, (void *)&kVs[0], sizeof (struct SFColor));
	} else if (px->set_fraction >= px->key.p[kin-1]) {
		memcpy ((void *)&px->value_changed, (void *)&kVs[kvin-1], sizeof (struct SFColor));
	} else {
		/* have to go through and find the key before */
		counter = find_key(kin,((float)(px->set_fraction)),px->key.p);
		for (tmp=0; tmp<3; tmp++) {
			px->value_changed.c[tmp] =
				(px->set_fraction - px->key.p[counter-1]) /
				(px->key.p[counter] - px->key.p[counter-1]) *
				(kVs[counter].c[tmp] - kVs[counter-1].c[tmp]) + kVs[counter-1].c[tmp];
		}
	}
	#ifdef SEVERBOSE
	printf ("Pos/Col, new value (%f %f %f)\n",
		px->value_changed.c[0],px->value_changed.c[1],px->value_changed.c[2]);
	#endif
}


void do_PositionInterpolator (void *node) {
	struct X3D_PositionInterpolator *px;
	int kin, kvin, counter, tmp;
	struct SFVec3f *kVs; 

	if (!node) return;
	px = (struct X3D_PositionInterpolator *) node;

	kvin = px->keyValue.n;
	kVs = px->keyValue.p;
	kin = px->key.n;

	MARK_EVENT (node, offsetof (struct X3D_PositionInterpolator, value_changed)); 

	#ifdef SEVERBOSE
		printf("do_PositionInt: Position/Vec3f interp, node %u kin %d kvin %d set_fraction %f\n",
			   node, kin, kvin, px->set_fraction);
	#endif

	/* make sure we have the keys and keyValues */
	if ((kvin == 0) || (kin == 0)) {
		px->value_changed.c[0] = (float) 0.0;
		px->value_changed.c[1] = (float) 0.0;
		px->value_changed.c[2] = (float) 0.0;
		return;
	}

	if (kin>kvin) kin=kvin; /* means we don't use whole of keyValue, but... */

	/* set_fraction less than or greater than keys */
	if (px->set_fraction <= ((px->key).p[0])) {
		memcpy ((void *)&px->value_changed, (void *)&kVs[0], sizeof (struct SFVec3f));
	} else if (px->set_fraction >= px->key.p[kin-1]) {
		memcpy ((void *)&px->value_changed, (void *)&kVs[kvin-1], sizeof (struct SFVec3f));
	} else {
		/* have to go through and find the key before */
		counter = find_key(kin,((float)(px->set_fraction)),px->key.p);
		for (tmp=0; tmp<3; tmp++) {
			px->value_changed.c[tmp] =
				(px->set_fraction - px->key.p[counter-1]) /
				(px->key.p[counter] - px->key.p[counter-1]) *
				(kVs[counter].c[tmp] - kVs[counter-1].c[tmp]) + kVs[counter-1].c[tmp];
		}
	}
	#ifdef SEVERBOSE
	printf ("Pos/Col, new value (%f %f %f)\n",
		px->value_changed.c[0],px->value_changed.c[1],px->value_changed.c[2]);
	#endif
}

/* OrientationInterpolator				 		*/
/* Called during the "events_processed" section of the event loop,	*/
/* so this is called ONLY when there is something required to do, thus	*/
/* there is no need to look at whether it is active or not		*/

void do_Oint4 (void *node) {
	struct X3D_OrientationInterpolator *px;
	int kin, kvin;
	struct SFRotation *kVs;
	int counter;
	float interval;		/* where we are between 2 values */
	// UNUSED?? int stzero;
	// UNUSED?? int endzero;	/* starting and/or ending angles zero? */

	Quaternion st, fin, final;
	double x,y,z,a;

	if (!node) return;
	px = (struct X3D_OrientationInterpolator *) node;
	kin = ((px->key).n);
	kvin = ((px->keyValue).n);
	kVs = ((px->keyValue).p);

	#ifdef SEVERBOSE
	printf ("starting do_Oint4; keyValue count %d and key count %d\n",
				kvin, kin);
	#endif


	MARK_EVENT (node, offsetof (struct X3D_OrientationInterpolator, value_changed));

	/* make sure we have the keys and keyValues */
	if ((kvin == 0) || (kin == 0)) {
		px->value_changed.c[0] = (float) 0.0;
		px->value_changed.c[1] = (float) 0.0;
		px->value_changed.c[2] = (float) 0.0;
		px->value_changed.c[3] = (float) 0.0;
		return;
	}
	if (kin>kvin) kin=kvin; /* means we don't use whole of keyValue, but... */


	/* set_fraction less than or greater than keys */
	if (px->set_fraction <= ((px->key).p[0])) {
		memcpy ((void *)&px->value_changed,
				(void *)&kVs[0], sizeof (struct SFRotation));
	} else if (px->set_fraction >= ((px->key).p[kin-1])) {
		memcpy ((void *)&px->value_changed,
				(void *)&kVs[kvin-1], sizeof (struct SFRotation));
	} else {
		counter = find_key(kin,(float)(px->set_fraction),px->key.p);
		interval = (px->set_fraction - px->key.p[counter-1]) /
				(px->key.p[counter] - px->key.p[counter-1]);

		
		/* are either the starting or ending angles zero? */
		// unused? stzero = APPROX(kVs[counter-1].c[3],0.0);
		// unused? endzero = APPROX(kVs[counter].c[3],0.0);
		#ifdef SEVERBOSE
			printf ("counter %d interval %f\n",counter,interval);
			printf ("angles %f %f %f %f, %f %f %f %f\n",
				kVs[counter-1].c[0],
				kVs[counter-1].c[1],
				kVs[counter-1].c[2],
				kVs[counter-1].c[3],
				kVs[counter].c[0],
				kVs[counter].c[1],
				kVs[counter].c[2],
				kVs[counter].c[3]);
		#endif
		vrmlrot_to_quaternion (&st, kVs[counter-1].c[0],
                                kVs[counter-1].c[1], kVs[counter-1].c[2], kVs[counter-1].c[3]);
		vrmlrot_to_quaternion (&fin,kVs[counter].c[0],
                                kVs[counter].c[1], kVs[counter].c[2], kVs[counter].c[3]);

		quaternion_slerp(&final, &st, &fin, (double)interval);
		quaternion_to_vrmlrot(&final,&x, &y, &z, &a);
		px->value_changed.c[0] = (float) x;
		px->value_changed.c[1] = (float) y;
		px->value_changed.c[2] = (float) z;
		px->value_changed.c[3] = (float) a;

		#ifdef SEVERBOSE
		printf ("Oint, new angle %f %f %f %f\n",px->value_changed.c[0],
			px->value_changed.c[1],px->value_changed.c[2], px->value_changed.c[3]);
		#endif
	}
}

/* fired at start of event loop for every Collision */
/* void do_CollisionTick(struct X3D_Collision *cx) {*/
void do_CollisionTick( void *ptr) {
	struct X3D_Collision *cx = (struct X3D_Collision *)ptr;
        if (cx->__hit == 3) {
                /* printf ("COLLISION at %f\n",TickTime()); */
                cx->collideTime = TickTime();
                MARK_EVENT (ptr, offsetof(struct X3D_Collision, collideTime));
        }
}


/* Audio AudioClip sensor code */
/* void do_AudioTick(struct X3D_AudioClip *node) {*/
void do_AudioTick(void *ptr) {
	struct X3D_AudioClip *node = (struct X3D_AudioClip *)ptr;
	int 	oldstatus;
	double pitch, duration; /* gcc and params - make all doubles to do_active_inactive */

	/* can we possibly have started yet? */
	if (!node) return;

	if(node->__inittime == 0.0)
		node->__inittime = TickTime();

	if(TickTime() < node->startTime) {
		return;
	}

	oldstatus = node->isActive;
	pitch = node->pitch;

	if(node->__sourceNumber < 0) return;
	///* is this audio wavelet initialized yet? */
	//if (node->__sourceNumber == -1) {
	//	locateAudioSource (node);
	//	/* printf ("do_AudioTick, node %d sn %d\n", node, node->__sourceNumber);  */
	//}

	///* is this audio ok? if so, the sourceNumber will range
	// * between 0 and infinity; if it is BADAUDIOSOURCE, bad source.
	// * check out locateAudioSource to find out reasons */
	//if (node->__sourceNumber == BADAUDIOSOURCE) return;

	/* call common time sensor routine */
	//duration = return_Duration(node->__sourceNumber);
	duration = return_Duration(node);
	do_active_inactive (
		&node->isActive, &node->__inittime, &node->startTime,
		&node->stopTime,node->loop,duration,
		pitch,node->elapsedTime);

	if (oldstatus != node->isActive) {
		/* push @e, [$t, "isActive", node->{isActive}]; */
		if (node->isActive == 1) {
			/* force code below to generate event */
			//node->__ctflag = 10.0;
			node->__lasttime = TickTime();
			node->elapsedTime = 0.0;
		}
		MARK_EVENT (X3D_NODE(node), offsetof(struct X3D_AudioClip, isActive));
	}
	
	if(node->isActive){
		if(node->pauseTime > node->startTime){
			if( node->resumeTime < node->pauseTime && !node->isPaused){
				node->isPaused = TRUE;
				MARK_EVENT (X3D_NODE(node), offsetof(struct X3D_AudioClip, isPaused));
			}else if(node->resumeTime > node->pauseTime && node->isPaused){
				node->isPaused = FALSE;
				node->__lasttime = TickTime();
				MARK_EVENT (X3D_NODE(node), offsetof(struct X3D_AudioClip, isPaused));
			}
		}
	}
	if(node->isActive == 1 && node->isPaused == FALSE) {
		double dtime = TickTime();
		node->elapsedTime += dtime - node->__lasttime;
		node->__lasttime = dtime; 
		//double myFrac = node->elapsedTime / duration;
		MARK_EVENT (ptr, offsetof(struct X3D_AudioClip, elapsedTime));
	}
}



/* Similar to AudioClip, this is the Play, Pause, Stop, Resume code
*/
#define LOAD_STABLE 10 //from component_sound.c
unsigned char *movietexture_get_frame_by_fraction(struct X3D_Node* node, float fraction, int *width, int *height, int *nchan);
void do_MovieTextureTick( void *ptr) {
	struct X3D_MovieTexture *node = (struct X3D_MovieTexture *)ptr;
	//struct X3D_AudioClip *anode;
	int 	oldstatus;
	float 	frac;		/* which texture to display */
	//int 	highest,lowest;	/* selector variables		*/
	double myFrac;
	double 	speed;
	double	duration;
	int tmpTrunc; 		/* used for timing for textures */

	//anode = (struct X3D_AudioClip *)node;
	//do_AudioTick(ptr);  //does play, pause, active, inactive part

	/* can we possibly have started yet? */
	if (!node) return;

	if(node->__inittime == 0.0)
		node->__inittime = TickTime();

	if(TickTime() < node->startTime) {
		return;
	}

//	duration = (highest - lowest)/30.0;
	//highest = node->__highest;
	//lowest = node->__lowest;
	duration = node->duration_changed; //return_Duration(node);
	speed = node->speed;

	oldstatus = node->isActive;
	do_active_inactive (
		&node->isActive, &node->__inittime, &node->startTime,
		&node->stopTime,node->loop,duration,
		speed,node->elapsedTime);

	if (oldstatus != node->isActive) {
		if (node->isActive == 1) {
			/* force code below to generate event */
			//node->__ctflag = 10.0;
			node->__lasttime = TickTime();
			node->elapsedTime = 0.0;
		}
		MARK_EVENT (X3D_NODE(node), offsetof(struct X3D_MovieTexture, isActive));
	}

	if(node->isActive){
		if(node->pauseTime > node->startTime){
			if( node->resumeTime < node->pauseTime && !node->isPaused){
				node->isPaused = TRUE;
				MARK_EVENT (X3D_NODE(node), offsetof(struct X3D_MovieTexture, isPaused));
			}else if(node->resumeTime > node->pauseTime && node->isPaused){
				node->isPaused = FALSE;
				node->__lasttime = TickTime();
				MARK_EVENT (X3D_NODE(node), offsetof(struct X3D_MovieTexture, isPaused));
			}
		}
	}
	if(node->isActive && node->isPaused == FALSE) {
		double dtime = TickTime();
		node->elapsedTime += dtime - node->__lasttime;
		node->__lasttime = dtime; 
		
		//frac = node->__ctex;

		///* sanity check - avoids divide by zero problems below */
		//if (node->__lowest >= node->__highest) {
		//	node->__lowest = node->__highest-1;
		//}
		/* calculate what fraction we should be */
		// t = (now - startTime) modulo (duration/speed)
		myFrac = node->elapsedTime / duration;
 		//myTime = (TickTime() - node->startTime) * speed/duration;
		tmpTrunc = (int) myFrac;
		frac = (float)myFrac - (float)tmpTrunc;
		/* negative speed? */
		if (speed < 0) {
			frac = 1.0f + frac; /* frac will be *negative* */
		/* else if (speed == 0) */
		} else if (APPROX(speed, 0.0f)) {
			frac = 0.0f;
		}
		node->__frac = frac;
		//clamp to last frame when not looping, so at end of show last frame sticks as per specs
		if(node->loop == FALSE && tmpTrunc > 0)
			node->__frac = 1.0f; 
		//printf("tmptnk=%d frac=%f ",tmpTrunc,node->__frac);
		//node->elapsedTime = TickTime() - node->startTime;
		//printf("/ et %lf /",node->elapsedTime);
		MARK_EVENT (ptr, offsetof(struct X3D_MovieTexture, elapsedTime));
	}
	if(node->__loadstatus == LOAD_STABLE){
		//Nov 16, 2016 the following works with MPEG_Utils_ffmpeg.c on non-audio mpeg (vts.mpg)
		// x not tested with audio
		unsigned char* texdata;
		int width,height,nchan;
		textureTableIndexStruct_s *tti;
		texdata = movietexture_get_frame_by_fraction(X3D_NODE(node), node->__frac, &width, &height, &nchan);
		if(texdata){
			int thisTexture = node->__textureTableIndex;
			tti = getTableIndex(thisTexture);
			if(tti){
				static int once = 0;
				tti->x = width;
				tti->y = height;
				tti->z = 1;
				tti->channels = nchan;
				if(!once){
					//send it through textures.c once to get things like wrap set
					// textures.c likes to free texdata, so we'll deep copy
					tti->texdata = malloc(tti->x*tti->y*tti->channels);
					memcpy(tti->texdata,texdata,tti->x*tti->y*tti->channels);
					tti->status = TEX_NEEDSBINDING;
					once = 1;
				}else{
					tti->status = TEX_LOADED;
					glBindTexture(GL_TEXTURE_2D,tti->OpenGLTexture);
					//disable the mipmapping done on the once pass through textures.c above
					FW_GL_TEXPARAMETERI( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					FW_GL_TEXPARAMETERI( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					//replace the texture data every frame when we are isActive and not paused
					//we do this once per frame in startofloopnodeupdates call stack
					//(not per render call: we want the same texture to show in left/right or quad display viewports)
					if(nchan == 4)
						glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,texdata);
					if(nchan == 3)
						glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width,height,0,GL_RGB,GL_UNSIGNED_BYTE,texdata);
					glBindTexture(GL_TEXTURE_2D,0);
				}
			}
		}

	}
}


/****************************************************************************

	Sensitive nodes


*****************************************************************************/

float fclamp(float fval, float fstart, float fend) { 
	float fret = fval;
	fret = fval > fend? fend : fval;		//min(fval,fend)
	fret = fret < fstart ? fstart : fret;	//max(fval,fstart)
	return fret;
}
float *vecclamp3f(float *fval, float *fstart, float *fend){
	int i;
	for(i=0;i<3;i++){
		if(fstart[i] <= fend[i])
			fval[i] = fclamp(fval[i],fstart[i],fend[i]);
	}
	return fval;  //so you can chain
}
// #define APPROX(a,b) (fabs((a)-(b))<0.00000001)
int approx3f(float *a, float *b){
	float tol = 0.00000001;
	int i, iret = TRUE;
	for(i=0;i<3;i++){
		iret = iret && (fabs(a[i] - b[i]) < tol) ? iret : FALSE;
	}
	return iret;
}
int approx4f(float *a, float *b){
	float tol = 0.00000001;
	int i, iret = TRUE;
	for(i=0;i<4;i++){
		iret = iret && (fabs(a[i] - b[i]) < tol) ? iret : FALSE;
	}
	return iret;
}
void do_TouchSensor ( void *ptr, int ev, int but1, int over) {

	struct X3D_TouchSensor *node = (struct X3D_TouchSensor *)ptr;
	float normalval[3];	
	ttglobal tg;
	#ifdef SENSVERBOSE
	printf ("%lf: TS ",TickTime());
	if (ev==ButtonPress) printf ("ButtonPress ");
	else if (ev==ButtonRelease) printf ("ButtonRelease ");
	else if (ev==KeyPress) printf ("KeyPress ");
	else if (ev==KeyRelease) printf ("KeyRelease ");
	else if (ev==MotionNotify) printf ("%lf MotionNotify ");
	else printf ("ev %d ",ev);
	
	if (but1) printf ("but1 TRUE "); else printf ("but1 FALSE ");
	if (over) printf ("over TRUE "); else printf ("over FALSE ");
	printf ("\n");
	#endif


	/* if not enabled, do nothing */
	if (!node) return;
	if (node->__oldEnabled != node->enabled) {
		node->__oldEnabled = node->enabled;
		MARK_EVENT(X3D_NODE(node),offsetof (struct X3D_TouchSensor, enabled));
	}
	if (!node->enabled) return;
	tg = gglobal();
	/* isOver state */
	if ((ev == overMark) && (over != node->isOver)) {
		#ifdef SENSVERBOSE
		printf ("TS %u, isOver changed %d\n",node, over);
		#endif
		node->isOver = over;
		MARK_EVENT (ptr, offsetof (struct X3D_TouchSensor, isOver));
	}

	/* active */
		/* button presses */
		if (ev == ButtonPress) {
			node->isActive=TRUE;
			MARK_EVENT (ptr, offsetof (struct X3D_TouchSensor, isActive));
			#ifdef SENSVERBOSE
			printf ("touchSens %u, butPress\n",node);
			#endif

			node->touchTime = TickTime();
			MARK_EVENT(ptr, offsetof (struct X3D_TouchSensor, touchTime));

		} else if (ev == ButtonRelease) {
			#ifdef SENSVERBOSE
			printf ("touchSens %u, butRelease\n",node);
			#endif
			node->isActive=FALSE;
			MARK_EVENT (ptr, offsetof (struct X3D_TouchSensor, isActive));
		}

		/* hitPoint and hitNormal */
		/* save the current hitPoint for determining if this changes between runs */
		veccopy3f(node->_oldhitPoint.c,tg->RenderFuncs.ray_save_posn);

		/* did the hitPoint change between runs? */
		if(!approx3f(node->_oldhitPoint.c,node->hitPoint_changed.c)){
			veccopy3f(node->hitPoint_changed.c,node->_oldhitPoint.c);
			MARK_EVENT(ptr, offsetof (struct X3D_TouchSensor, hitPoint_changed));
		}

		/* have to normalize normal; change it from SFColor to struct point_XYZ. */
		veccopy3f(normalval,tg->RenderFuncs.hyp_save_norm);
		vecnormalize3f(normalval,normalval);
		veccopy3f(node->_oldhitNormal.c,normalval);

		/* did the hitNormal change between runs? */
		if(!approx3f(node->_oldhitNormal.c,node->hitNormal_changed.c)) {
			//memcpy ((void *) &node->hitNormal_changed, (void *) &node->_oldhitNormal, sizeof(struct SFColor));
			veccopy3f(node->hitNormal_changed.c,node->_oldhitNormal.c);
			MARK_EVENT(ptr, offsetof (struct X3D_TouchSensor, hitNormal_changed));
		}
}
// see Mainloop.c get_hyperhit() for more explanation:
// in sensor-node-local coordinates (not quite sensor-local if sensor node has axisRotation):
// ray_save_posn - intersection point of pickray/bearing with sensitized geometry
// hyp_save_posn - point on camera/viewpoint nearplane on pickray/bearing, transformed to sensor-node-local
// hyp_save_norm - point on carmera/viewpoint farplane on pickray/bearing, transformed to sensor-node-local

void do_LineSensor(void *ptr, int ev, int but1, int over) {
	/* There is no LineSensor node in the specs in April 2014. X3Dom guru Max Limper complained
		on X3DPublic about how PlaneSensor fails as an axis mover in the degenerate case of
		looking edge-on at the planeSensor. The solution we (dug9) came up with was LineSensor, 
		which also has a degenerate case (when looking end-on at the Line), but that degerate
		case is more normal for users - more intuitive.
		LineSensor is the same as PlaneSensor, except minPosition and maxPosition are floats,
		and LineSensor uses a SFVec3f .direction field to say which way the Line is oriented
		in local-sensor coordinates. In April 2014, Paulo added LineSensor to the perl generator,
		and dug9 implemented it here.
	*/
	struct X3D_LineSensor *node;
	float trackpoint[3], translation[3], xxx;
	//struct SFColor tr;
	//int tmp;
	ttglobal tg;
	UNUSED(over);
	node = (struct X3D_LineSensor *)ptr;
#ifdef SENSVERBOSE
	printf("%lf: TS ", TickTime());
	if (ev == ButtonPress) printf("ButtonPress ");
	else if (ev == ButtonRelease) printf("ButtonRelease ");
	else if (ev == KeyPress) printf("KeyPress ");
	else if (ev == KeyRelease) printf("KeyRelease ");
	else if (ev == MotionNotify) printf("%lf MotionNotify ");
	else printf("ev %d ", ev);

	if (but1) printf("but1 TRUE "); else printf("but1 FALSE ");
	if (over) printf("over TRUE "); else printf("over FALSE ");
	printf("\n");
#endif

	/* if not enabled, do nothing */
	if (!node) return;

	if (node->__oldEnabled != node->enabled) {
		node->__oldEnabled = node->enabled;
		MARK_EVENT(X3D_NODE(node), offsetof(struct X3D_LineSensor, enabled));
	}
	if (!node->enabled) return;
	tg = gglobal();

	/* only do something when button pressed */
	/* if (!but1) return; */
	if (but1){
		//pre-calculate for Press and Move
		/* hyperhit saved in render_hypersensitive phase */
		// bearing in sensor-local coordinates: (A=posn,B=norm) 
		// B/norm is a point, so to get a direction vector: v = B - A
		float tt;
		float origin [] = { 0.0f, 0.0f, 0.0f };
		float footpoint2[3], footpoint1[3], v1[3]; //, temp[3], temp2[3];
		vecdif3f(v1, tg->RenderFuncs.hyp_save_norm, tg->RenderFuncs.hyp_save_posn);
		vecnormalize3f(v1, v1);
		if (!line_intersect_line_3f(tg->RenderFuncs.hyp_save_posn, v1,
			origin, node->direction.c, NULL, &tt, footpoint1, footpoint2)) 
			return; //no intersection, lines are parallel
		//footpoint1 - closest point of intersection on the A'B' bearing
		//footpoint2 - closest point of intersection on the Line (0,0,0)(LineSensor.direction)
		//tt is scale of unit vector from origin to footpoint2
		xxx = tt;
		veccopy3f(trackpoint,footpoint2); //unclamped intersection with Sensor geometry, for trackpoint
	}
	if ((ev == ButtonPress) && but1) {
		/* record the current position from the saved position */
#define LINESENSOR_FLOAT_OFFSET 1
#ifndef LINESENSOR_FLOAT_OFFSET
		struct SFColor op;
		veccopy3f(op.c, trackpoint);
		memcpy((void *)&node->_origPoint, (void *)&op,sizeof(struct SFColor));
		//	(void *)&tg->RenderFuncs.ray_save_posn, sizeof(struct SFColor));
#else
		node->_origPoint.c[0] = xxx;
		//in case we go from mousedown to mouseup without mousemove:
		if (node->autoOffset)
			node->_origPoint.c[1] = node->offset; 
		else
			node->_origPoint.c[1] = 0.0f;
#endif
		/* set isActive true */
		node->isActive = TRUE;
		MARK_EVENT(ptr, offsetof(struct X3D_LineSensor, isActive));

	}
	else if ((ev == MotionNotify) && (node->isActive) && but1) {
		float xxxoffset, xxxorigin;
		//float diroffset[3], nondiroffset[3];
		/* trackpoint changed */
		veccopy3f(node->_oldtrackPoint.c,trackpoint);
		
		if(!approx3f(node->_oldtrackPoint.c, node->trackPoint_changed.c)) {
			veccopy3f(node->trackPoint_changed.c, node->_oldtrackPoint.c);
			MARK_EVENT(ptr, offsetof(struct X3D_LineSensor, trackPoint_changed));

		}

		//clamp to min,max 
#ifdef LINESENSOR_FLOAT_OFFSET
		xxxoffset = node->offset;
		xxxorigin = node->_origPoint.c[0];
#else
		//in theory the user can set a non-autoOffset sfvec3f offset that's not along .direction
		//- we accomodate that below^, so here we just use the part going along .direction
		xxxoffset = vecdot3f(node->direction.c,node->offset.c); //xxxoffset - like web3d specs offset, except just along direction vector
		xxxorigin = vecdot3f(node->direction.c,node->_origPoint.c); //mouse-down origin
#endif
		//xxx before: unclamped position from line origin
		xxx -= xxxorigin; //xxx after: net drag/delta along line since mouse-down
		xxx += xxxoffset; //xxx after: cumulative position along line (from line 0) after any/all mousedown/drag sequences
		if (node->maxPosition >= node->minPosition) {
			if (xxx < node->minPosition) {
				xxx = node->minPosition;
			}
			else if (xxx > node->maxPosition) {
				xxx = node->maxPosition;
			}
		}
		//translation clamped to LineSensor.minPosition/.maxPosition
		vecscale3f(translation, node->direction.c, xxx);

#ifndef LINESENSOR_FLOAT_OFFSET		
		//^add on any non-autoOffset non-.direction offset 
		//a) part of offset going along direction
		vecscale3f(diroffset, node->direction.c, xxxoffset);
		//b) part of offset not going along direction
		vecdif3f(nondiroffset, node->offset.c, diroffset);
		//add non-direction part of offset
		vecadd3f(translation, translation, nondiroffset);
#endif

		veccopy3f(node->_oldtranslation.c,translation);

		if(!approx3f(node->_oldtranslation.c, node->translation_changed.c)) {
			veccopy3f(node->translation_changed.c, node->_oldtranslation.c);
			MARK_EVENT(ptr, offsetof(struct X3D_LineSensor, translation_changed));
		}
		//save current for use in mouse-up auto-offset
		node->_origPoint.c[1] = xxx;
	}
	else if (ev == ButtonRelease) {
		/* set isActive false */
		node->isActive = FALSE;
		MARK_EVENT(ptr, offsetof(struct X3D_LineSensor, isActive));

		/* autoOffset? */
		if (node->autoOffset) {
#ifdef LINESENSOR_FLOAT_OFFSET
			node->offset = node->_origPoint.c[1];
#else
			veccopy3f(node->offset.c,node->translation_changed.c);
#endif
			MARK_EVENT(ptr, offsetof(struct X3D_LineSensor, offset));
		}
	}

}


void do_PointSensor(void *ptr, int ev, int but1, int over) {
	/* Experimental node There is no PointSensor node in the specs in Dec 2017.
		Concept: you should be able to grab and drag something perpendicular to your ray.
		then if you move your viewpoint (ie with examine) you should be able to drag 
		perpendicular to your new ray direction 
		So the direction isn't in a field 
		- its computed internally based on pickray/bearing direction
		- (in theory it could be an outputOnly)
		Trackpoint would start at ray/bearing distance from viewpoint
	*/
	struct X3D_PointSensor *node;
	float trackpoint[3], translation[3], *posn, *rposn, *norm;
	ttglobal tg;
	UNUSED(over);
	node = (struct X3D_PointSensor *)ptr;
#ifdef SENSVERBOSE
	printf("%lf: TS ", TickTime());
	if (ev == ButtonPress) printf("ButtonPress ");
	else if (ev == ButtonRelease) printf("ButtonRelease ");
	else if (ev == KeyPress) printf("KeyPress ");
	else if (ev == KeyRelease) printf("KeyRelease ");
	else if (ev == MotionNotify) printf("%lf MotionNotify ");
	else printf("ev %d ", ev);

	if (but1) printf("but1 TRUE "); else printf("but1 FALSE ");
	if (over) printf("over TRUE "); else printf("over FALSE ");
	printf("\n");
#endif

	/* if not enabled, do nothing */
	if (!node) return;

	if (node->__oldEnabled != node->enabled) {
		node->__oldEnabled = node->enabled;
		MARK_EVENT(X3D_NODE(node), offsetof(struct X3D_PointSensor, enabled));
	}
	if (!node->enabled) return;
	tg = gglobal();

	/* only do something when button pressed */
	if (!but1) return; 
	norm = tg->RenderFuncs.hyp_save_norm;
	posn = tg->RenderFuncs.hyp_save_posn;
	rposn = tg->RenderFuncs.ray_save_posn;

	if ((ev == ButtonPress) && but1) {
		/* record the current position from the saved position */
		float tt[3];
		float distance = veclength3f(vecdif3f(tt,rposn,norm));
		//printf("dist0 = %f\n",distance);
		veccopy3f(trackpoint,rposn); 
		veccopy3f(node->_origPoint.c,trackpoint); 

		/* set isActive true */
		node->isActive = TRUE;
		MARK_EVENT(ptr, offsetof(struct X3D_PointSensor, isActive));

	}
	else if ((ev == MotionNotify) && (node->isActive) && but1) {
		/* trackpoint changed */
		float t1[3];
		
		//pre-calculate for Press and Move
		/* hyperhit saved in render_hypersensitive phase */
		// bearing in sensor-local coordinates: (A=posn,B=norm) 
		// B/norm is a point, so to get a direction vector: v = B - A
		float tt[3];
		//float N [] = { 0.0f, 0.0f, 1.0f };
		float v1[3]; 

		veccopy3f(trackpoint,rposn);

		veccopy3f(node->_oldtrackPoint.c,trackpoint);
		if(!approx3f(node->_oldtrackPoint.c, node->trackPoint_changed.c)) {
			veccopy3f(node->trackPoint_changed.c, node->_oldtrackPoint.c);
			MARK_EVENT(ptr, offsetof(struct X3D_PointSensor, trackPoint_changed));
		}

		vecdif3f(v1, norm, posn);
		vecnormalize3f(v1, v1);

		//IDEA intersect the pickray with a plane at distance to the mouse-down point
		// and drag in a plane perpendicular to the camera axis
		// ie plane = (mouse-down ray_posn, viewpoint axis)
		if (!line_intersect_plane_3f(posn,v1,tg->RenderFuncs.camera_axis,node->_origPoint.c,translation,NULL))
			return;

		if (node->autoOffset){
			vecadd3f(translation,translation,node->offset.c);
		}


		//clamp to min,max 
		vecclamp3f(translation,node->minPosition.c,node->maxPosition.c);

		veccopy3f(node->_oldtranslation.c,translation);

		if(!approx3f(node->_oldtranslation.c, node->translation_changed.c)) {
			veccopy3f(node->translation_changed.c, node->_oldtranslation.c);
			MARK_EVENT(ptr, offsetof(struct X3D_PointSensor, translation_changed));
		}
	}
	else if (ev == ButtonRelease) {
		/* set isActive false */
		node->isActive = FALSE;
		MARK_EVENT(ptr, offsetof(struct X3D_PointSensor, isActive));
		/* autoOffset? */
		if (node->autoOffset) {
			veccopy3f(node->offset.c,node->translation_changed.c);
			MARK_EVENT(ptr, offsetof(struct X3D_PointSensor, offset));
		}
	}

}

/* void do_PlaneSensor (struct X3D_PlaneSensor *node, int ev, int over) {*/
void do_PlaneSensor ( void *ptr, int ev, int but1, int over) {
	struct X3D_PlaneSensor *node;
	float mult, nx, ny, trackpoint[3], inverserotation[4], *posn;
	float tr[3];
	int tmp, imethod;

	ttglobal tg;
	UNUSED(over);
	node = (struct X3D_PlaneSensor *)ptr;
#ifdef SENSVERBOSE
	ConsoleMessage("%lf: TS ",TickTime());
	if (ev==ButtonPress) ConsoleMessage("ButtonPress ");
	else if (ev==ButtonRelease) ConsoleMessage("ButtonRelease ");
	else if (ev==KeyPress) ConsoleMessage("KeyPress ");
	else if (ev==KeyRelease) ConsoleMessage("KeyRelease ");
	else if (ev==MotionNotify) ConsoleMessage("MotionNotify ");
	else ConsoleMessage("ev %d ",ev);
	
	if (but1) ConsoleMessage("but1 TRUE "); else ConsoleMessage("but1 FALSE ");
	if (over) ConsoleMessage("over TRUE "); else ConsoleMessage("over FALSE ");
	ConsoleMessage ("\n");
#endif

	/* if not enabled, do nothing */
	if (!node) return;

	if (node->__oldEnabled != node->enabled) {
		node->__oldEnabled = node->enabled;
		MARK_EVENT(X3D_NODE(node),offsetof (struct X3D_PlaneSensor, enabled));
	}
	if (!node->enabled) return;
	tg = gglobal();

	/* only do something when button pressed */
	/* if (!but1) return; */
	if (but1){
		float v[3], t1[3];
		float N[3] = { 0.0f, 0.0f, 1.0f }; //plane normal, in plane-local
		float NS[3]; //plane normal, in sensor-local after axisRotation
		//bearing (A,B) in sensor-local
		// A=posn, B=norm - norm is a point. To get a direction vector v = (B - A)
		//ConsoleMessage("hsp = %f %f %f \n", tg->RenderFuncs.hyp_save_posn[0], tg->RenderFuncs.hyp_save_posn[1], tg->RenderFuncs.hyp_save_posn[2]);
		vecnormalize3f(v, vecdif3f(t1, tg->RenderFuncs.hyp_save_norm, tg->RenderFuncs.hyp_save_posn));
		//rotate plane normal N, in plane-local to plane normal NS in sensor-local using axisRotation
		axisangle_rotate3f(NS,N, node->axisRotation.c);
		//a plane P dot N = d = const, for any point P on plane. Our plane is in plane-local coords, 
		// so we could use P={0,0,0} and P dot N = d = 0
		posn = tg->RenderFuncs.hyp_save_posn;
		if (!line_intersect_planed_3f(posn, v, NS, 0.0f, trackpoint, NULL))
			return; //looking at plane edge-on / parallel, no intersection
		//is rotating the trackpoint/translation_changed opposite sense to rotating the virtual geometry?
		//-- we harmonize with x3dom and view3dscene 
		veccopy4f(inverserotation,node->axisRotation.c);
		inverserotation[3] = -inverserotation[3];
		//axisangle_rotate3f(trackpoint, trackpoint, inverserotation);
	}

	if ((ev==ButtonPress) && but1) {
		/* record the current position from the saved position */
		struct SFColor op;
		float *posn;
		posn = tg->RenderFuncs.hyp_save_posn;

		veccopy3f(op.c, trackpoint);
		memcpy((void *)&node->_origPoint, (void *)&op,sizeof(struct SFColor));
		veccopy3f(node->_origPoint.c,op.c);

		/* set isActive true */
		node->isActive=TRUE;
		MARK_EVENT (ptr, offsetof (struct X3D_PlaneSensor, isActive));

	} else if ((ev==MotionNotify) && (node->isActive) && but1) {
		/* hyperhit saved in render_hypersensitive phase */
		nx = trackpoint[0]; ny = trackpoint[1];
		#ifdef SEVERBOSE
		ConsoleMessage ("now, mult %f nx %f ny %f op %f %f %f\n",mult,nx,ny,
			node->_origPoint.c[0],node->_origPoint.c[1],
			node->_origPoint.c[2]);
		#endif

		/* trackpoint changed */
		if(!node->sensorLocalOutput){
			axisangle_rotate3f(trackpoint,trackpoint, inverserotation);
		}

		veccopy3f(node->_oldtrackPoint.c, trackpoint);
		/*printf(">%f %f %f\n",nx,ny,node->_oldtrackPoint.c[2]); */
		if(!approx3f(node->_oldtrackPoint.c,node->trackPoint_changed.c)) {
			veccopy3f(node->trackPoint_changed.c, node->_oldtrackPoint.c);
			MARK_EVENT(ptr, offsetof (struct X3D_PlaneSensor, trackPoint_changed));

		}

		/* clamp translation to max/min position */
		tr[0] = nx - node->_origPoint.c[0] + node->offset.c[0];
		tr[1] = ny - node->_origPoint.c[1] + node->offset.c[1];
		tr[2] = node->offset.c[2];

		vecclamp3f(tr,node->minPosition.c,node->maxPosition.c);
		if(!node->sensorLocalOutput){
			axisangle_rotate3f(tr,tr, node->axisRotation.c);
		}
		veccopy3f(node->_oldtranslation.c,tr);

		if(!approx3f(node->_oldtranslation.c,node->translation_changed.c)) {
			veccopy3f(node->translation_changed.c, (void *) node->_oldtranslation.c);
			MARK_EVENT(ptr, offsetof (struct X3D_PlaneSensor, translation_changed));
		}

	} else if (ev==ButtonRelease) {
		/* set isActive false */
		node->isActive=FALSE;
		MARK_EVENT (ptr, offsetof (struct X3D_PlaneSensor, isActive));

		/* autoOffset? */
		if (node->autoOffset) {
			veccopy3f(node->offset.c,node->translation_changed.c);

			MARK_EVENT (ptr, offsetof (struct X3D_PlaneSensor, offset));
		}
	}

}


/* void do_Anchor (struct X3D_Anchor *node, int ev, int over) {*/
void do_Anchor ( void *ptr, int ev, int but1, int over) {
	struct X3D_Anchor *node = (struct X3D_Anchor *)ptr;
	UNUSED(over);
	UNUSED(but1);

	if (!node) return;
	/* try button release, so that we dont get worlds flashing past if 
	   the user keeps the finger down. :-) if (ev==ButtonPress) { */
	if (ev==ButtonRelease) {
		ttglobal tg = gglobal();
		/* no parameters in url field? */
		if (node->url.n < 1) return;
		setAnchorsAnchor( node );
		#ifdef OLDCODE
		OLDCODE FREE_IF_NZ(tg->RenderFuncs.OSX_replace_world_from_console);
		#endif // OLDCODE

		tg->RenderFuncs.BrowserAction = TRUE;
	}
}

//double angleAcuteDifferenced(double angle1, double angle2){
//	//sometimes we cross over the -PI or PI barrier and get a jump
//	//when really we want the small incremental difference
//	double angledif = angle2 - angle1;
//	if(angledif >  PI) angledif -= 2*PI;
//	if(angledif < -PI) angledif += 2*PI;
//	return angledif;
//}
//double angleNormalized(double angle){
//	return atan2(sin(angle),cos(angle));
//}

void do_CylinderSensor ( void *ptr, int ev, int but1, int over) {
	//troubled
	struct X3D_CylinderSensor *node = (struct X3D_CylinderSensor *)ptr;
	double rot, radius, ang, length;
	double det, pos, neg, temp;
	double acute_angle, disk_angle, height;
	float Y[3] = { 0.0f, 1.0f, 0.0f }, ZERO[3] = { 0.0f, 0.0f, 0.0f };
	float aBearing[3], bBearing[3], dirBearing[3], posn[3], axisRotation[4];
	Quaternion bv, dir1, dir2, tempV;
	GLDOUBLE modelMatrix[16];
	ttglobal tg;

	UNUSED(over);
	
	/* if not enabled, do nothing */
	if (!node) return;
	if (node->__oldEnabled != node->enabled) {
		node->__oldEnabled = node->enabled;
		MARK_EVENT(X3D_NODE(node),offsetof (struct X3D_CylinderSensor, enabled));
	}
	if (!node->enabled) return;

	/* only do something if the button is pressed */
	if (!but1) return;
	tg = gglobal();

	/*precompute some values for mouse-down, mouse-move*/
	//convert all almost-sensor-local points into sensor-local 
	//(the axisRotation never gets applied in the modelview transform stack - if that changes in the future, then don't need these)
	veccopy4f(axisRotation,node->axisRotation.c);
	axisRotation[3] = -axisRotation[3]; //harmonize rotation with view3dscene 
	//Dec 2017 view3dscene only other browser that shares our interp of specs on axisRotation for CylinderSensor
	//- x3dom and view3dscene share our interpretation of axisRotation for Planesensor
	axisangle_rotate3f(aBearing, tg->RenderFuncs.hyp_save_posn, axisRotation);
	axisangle_rotate3f(bBearing, tg->RenderFuncs.hyp_save_norm, axisRotation);
	vecnormalize3f(dirBearing, vecdif3f(dirBearing, bBearing, aBearing));
	axisangle_rotate3f(posn,tg->RenderFuncs.ray_save_posn, axisRotation);

	if (ev==ButtonPress) {
		/* record the current position from the saved position */
		/* on mouse-down we have to decide which sensor geometry to use: disk or cylinder, as per specs
			http://www.web3d.org/files/specifications/19775-1/V3.3/Part01/components/pointingsensor.html#CylinderSensor
			and that's determined by the angle between the bearing and the sensor Y axis, in sensor-local coords
			The bearing (A,B) where A=hyp_posn, B=hyp_norm and both are points in sensor-local coordinates
			To get a direction vector v = B - A
		*/
		struct SFColor origPoint;
		/*ray_save_posn is the intersection with scene geometry, in sensor-local coordinates, for cylinder*/
		float dot, rs[3];

		dot = vecdot3f(dirBearing, Y);
		dot = fclamp(dot,-1.0f,1.0f);
		acute_angle = acos(dot);
		ang = min(acute_angle,PI - acute_angle);
		//printf("ang= %f\n",(float)ang);
		veccopy3f(rs, posn); //posn: ray_posn (intersection with scene geometry) in sensor-local
		height = rs[1];
		rs[1] = 0.0f;
		//radius of ray_posn from cylinder axis, 
		//for scaling the 'feel' of the rotations to what the user clicked
		radius = veclength3f(rs); 
		vecnormalize3f(rs, rs);
		if (ang < node->diskAngle){
			//use end cap disks
			node->_usingDisk = TRUE;
			disk_angle = -atan2(rs[2], rs[0]);
			printf("using disk\n");
		}else{
			//use cylinder wall
			node->_usingDisk = FALSE;
			//printf("using cylinder\n");
			float travelled, cylpoint[3], axispoint[3], dif[3];
			line_intersect_line_3f(aBearing, dirBearing, ZERO, Y, NULL, NULL, cylpoint, axispoint);
			//travelled: closest distance of our bearing from cylinder axis
			travelled = veclength3f(vecdif3f(dif, cylpoint, axispoint)); 
			//which side of cylinder axis is our bearing on? 
			//v x dif will point a different direction (up or down) 
			//depending on which side, so dot with Y to get a sign
			if (det3f(dirBearing, dif, Y) > 0.0f) travelled = -travelled; 
			disk_angle = travelled / (2.0f * PI * radius) * (2.0f * PI); //don't need the 2PI except to show how we converted to radians: travelled is a fraction of circumference, and circumference is 2PI
		}
		node->_radius = (float)radius; //store for later use on mouse-moves
		//printf("radius= %f\n",node->_radius);
		//origPoint - we get to store whatever we need later mouse-moves. 
		//GOAL: be able to crank the disk, and keep going around in circles, accumulating angle, like s screw
		//printf("disk_angle=%f\n",(float)disk_angle);
		node->_origPoint.c[0] = disk_angle;
		node->_origPoint.c[1] = -height; //Q. why -height? don't know but it works
		//printf("rsp = %f %f %f\n",tg->RenderFuncs.ray_save_posn[0],tg->RenderFuncs.ray_save_posn[1],tg->RenderFuncs.ray_save_posn[2]);
		//printf("eqv = %f %f %f\n",cos(-disk_angle)*radius,height,sin(-disk_angle)*radius);
		/* set isActive true */
		node->isActive=TRUE;
		MARK_EVENT (ptr, offsetof (struct X3D_CylinderSensor, isActive));

	}else if ((ev == MotionNotify) && (node->isActive)) {
		float trackpoint[3], rotation4f[4];
		//specs > cylsensor: "trackPoint_changed events represent the unclamped intersection points 
		// on the surface of the invisible cylinder or disk"
		// Q. in sensor-local or sensor?
		veccopy3f(trackpoint,node->_oldtrackPoint.c); //a default, some non-junk value


		//compute delta rotation from drag
		//a plane P dot N = d = const, for any point P on plane. Our plane is in plane-local coords, 
		// so we could use P={0,0,0} and P dot N = d = 0
		float diskpoint[3], orig_diskangle, height;
		height = node->_origPoint.c[1];
		radius = node->_radius;
		orig_diskangle = node->_origPoint.c[0];
		if (node->_usingDisk == TRUE) {
			//disk
			line_intersect_planed_3f(aBearing, dirBearing, Y, height, diskpoint, NULL);
			veccopy3f(trackpoint,diskpoint);
			vecnormalize3f(diskpoint, diskpoint);
			//for cylinder compute angle from intersection on cylinder of radius
			disk_angle = -atan2(diskpoint[2], diskpoint[0]);
			//printf("D1 %lf ",disk_angle);
		}else {
			float cylpoint[3]; //pi1[3], 
			//cylinder wall
			//we want a drag off the cylinder to keep working even when mouse isn't over cylinder
			//on the cylinder
			//basically we try and do a linear drag perpendicular to both our bearing and the cylinder 
			//axis, and convert that linear distance from cylinder axis from distance into rotations
			float travelled, axispoint[3], dif[3];
			line_intersect_line_3f(aBearing, dirBearing, ZERO, Y, NULL, NULL, cylpoint, axispoint);
			//cylpoint - closest point of approach of our bearing, on the bearing
			//axispoint - ditto, on the cyl axis
			//dif = cylpoint - axispoint //vector perpendicular to axis - our perpendicular 'travel' from the axis
			travelled = veclength3f(vecdif3f(dif, cylpoint, axispoint));
			if (det3f(dirBearing, dif, Y) > 0.0f) travelled = -travelled; // v x dif will be up or down the cyl axis, depending on which side of the axis we are on
			//convert from linear travel to rotation, using travel/circumference * 2PI
			disk_angle = travelled / (2.0f * PI * radius) * (2.0f * PI); //convert from distance to radians using ratio of circumference
			if(!line_intersect_cylinder_3f(aBearing,dirBearing,node->_radius,trackpoint))
				veccopy3f(trackpoint,cylpoint);

		}
		rot = disk_angle - orig_diskangle;
		//printf(" D2 %lf ",rot);
		if (node->autoOffset) {
			//printf(" O %f ",node->offset);
			rot = node->offset + rot;
		}
		//printf(" D3 %lf ",rot);
		//printf(" N %lf ",rot);
		if (node->minAngle < node->maxAngle) {
			rot = fclamp(rot,node->minAngle,node->maxAngle);
		}
		//printf(" D4 %lf \n",rot);

		vecset4f(rotation4f,0.0f,1.0f,0.0f,(float)rot);

		if(!node->sensorLocalOutput){
			//this matches the octaga/instant/h3d technique
			axisangle_rotate3f(rotation4f,rotation4f,node->axisRotation.c); //rotate axis only
		}
		veccopy4f(node->_oldrotation.c,rotation4f);
		if(!approx4f(node->_oldrotation.c,node->rotation_changed.c)) {
			veccopy4f(node->rotation_changed.c, node->_oldrotation.c);
			MARK_EVENT(ptr, offsetof (struct X3D_CylinderSensor, rotation_changed));
		}

		//the specs don't explicitly say if the trackpoint is in sensor-local (with axisRotation applied)
		// or node-local. But it seems easier to understand if in node-local
		if(!node->sensorLocalOutput)
			axisangle_rotate3f(trackpoint, trackpoint, node->axisRotation.c);
		veccopy3f(node->_oldtrackPoint.c,trackpoint);
		if(!approx3f(node->_oldtrackPoint.c, node->trackPoint_changed.c)) {
			veccopy3f(node->trackPoint_changed.c, node->_oldtrackPoint.c);
			MARK_EVENT(ptr, offsetof(struct X3D_CylinderSensor, trackPoint_changed));
		}

	} else if (ev==ButtonRelease) {
		/* set isActive false */
		node->isActive=FALSE;
		MARK_EVENT (ptr, offsetof (struct X3D_CylinderSensor, isActive));
		/* save auto offset of rotation */
		if (node->autoOffset) {
			node->offset = node->rotation_changed.c[3];
		}
	}
}
// see Mainloop.c get_hyperhit() for more explanation:
// in sensor-node-local coordinates (not quite sensor-local if sensor node has axisRotation):
// ray_save_posn - intersection point of pickray/bearing with sensitized geometry
// hyp_save_posn - point on camera/viewpoint nearplane on pickray/bearing, transformed to sensor-node-local
// hyp_save_norm - point on carmera/viewpoint farplane on pickray/bearing, transformed to sensor-node-local

float fwfdsign(float x){ return x >= 0.0f ? 1.0f : -1.0f; }

void do_CylinderSensor_simple ( void *ptr, int ev, int but1, int over) {
	//simplest cylinder case, no axisRotation, no disk, derived from SphereSensor
	//not used except for understanding
	struct X3D_CylinderSensor *node = (struct X3D_CylinderSensor *)ptr;

	float *cur, *orig, onorm[3];
	ttglobal tg;
	UNUSED(over);

	/* if not enabled, do nothing */
	if (!node) 
		return;
	if (node->__oldEnabled != node->enabled) {
		node->__oldEnabled = node->enabled;
		MARK_EVENT(X3D_NODE(node),offsetof (struct X3D_CylinderSensor, enabled));
	}
	if (!node->enabled) 
		return;

	/* only do something if button1 is pressed */
	if (!but1) return;
	tg = gglobal();

	cur = tg->RenderFuncs.ray_save_posn;
	orig = node->_origPoint.c;
	veccopy3f(onorm,orig);
	onorm[1] = 0.0f;
	vecnormalize3f(onorm,onorm);
	if (ev==ButtonPress) {
		/* record the current position from the saved position */
		float pcur[3], height;
		veccopy3f(orig,cur);

		/* record the current Radius */
		veccopy3f(pcur,cur);
		height = pcur[1];
		pcur[1] = 0.0f;
		node->_radius = veclength3f(pcur);
		if (APPROX(node->_radius,0.0)) {
			printf ("warning, RADIUS %lf == 0, can not compute\n",node->_radius);
			return;
		}

		/* save the initial norm here */
		//vecscale3f(onorm,cur,1.0f / node->_radius);

		/* set isActive true */
		node->isActive=TRUE;
		MARK_EVENT (ptr, offsetof (struct X3D_CylinderSensor, isActive));
	} else if (ev==ButtonRelease) {
		/* set isActive false */
		node->isActive=FALSE;
		MARK_EVENT (ptr, offsetof (struct X3D_CylinderSensor, isActive));

		if (node->autoOffset) {
			node->offset = node->rotation_changed.c[3];
		}
	} else if ((ev==MotionNotify) && (node->isActive)) {
		
		float dotProd, sine, angle;
		float newRad;
		float cnorm[3];
		float newAxis[3];
		float pcur[3], height;

		/* record the current Radius */
		height = cur[1];
		veccopy3f(pcur,cur);
		pcur[1] = 0.0f;
		newRad = veclength3f(pcur);
		/* bounds check... */
		if (APPROX(newRad,0.0)) {
			printf ("warning, newRad %lf == 0, can not compute\n",newRad);
			return;
		}

		/* save the current norm here */
		//vecscale3f(cnorm,cur,1.0f/newRad);
		vecnormalize3f(cnorm,pcur);

		/* find the cross-product between the initial and current points */
		veccross3f(newAxis,onorm,cnorm);
		sine = veclength3f(newAxis);

		/* clamp the angle to |a| < 1.0 */
		/* remember A dot B = |A|*|B|*cos(theta_between) or theta_between = acos(A dot B/|A|*|B| ) */
		//dotProd = NORM_ORIG_X * NORM_CUR_X + NORM_ORIG_Y * NORM_CUR_Y + NORM_ORIG_Z * NORM_CUR_Z;
		dotProd = vecdot3f(onorm,cnorm);
		dotProd = fclamp(dotProd,-1.0f,1.0f);
		angle = acos(dotProd) * fwfdsign(newAxis[1]);

		/* have axis-angle now */
		/*
		printf ("newRotation  a %lf - rot -- %lf %lf %lf %lf\n",
			dotProd, newA.x,newA.y,newA.z,dotProd);
		*/
		if(node->autoOffset)
		{
			angle += node->offset;
			angle = atan2(sin(angle),cos(angle));
		}


		//veccopy3f(node->rotation_changed.c,newAxis);
		vecset3f(node->rotation_changed.c,0.0f,1.0f,0.0f);
		node->rotation_changed.c[3] = angle;
		MARK_EVENT (ptr, offsetof (struct X3D_CylinderSensor, rotation_changed));

		vecscale3f(node->trackPoint_changed.c,cnorm, node->_radius);
		node->trackPoint_changed.c[1] = height;
		MARK_EVENT (ptr, offsetof (struct X3D_CylinderSensor, trackPoint_changed));
	}
}


/********************************************************************************/
/*										*/
/* do the guts of a SphereSensor.... this has been changed considerably in Apr	*/
/* 2009 because the original, fast methods created by Tuomas Lukka failed in 	*/
/* a boundary area (HUD, small transform scale, close to viewer) and I could 	*/
/* not understand what *exactly* Tuomas' code did - I guess I don't have a 	*/
/* doctorate in math like he does! I went to the old linear algebra text and	*/
/* created a simple but inelegant solution from that. J.A. Stewart.		*/
/*										*/
/********************************************************************************/

void do_SphereSensor ( void *ptr, int ev, int but1, int over) {
	struct X3D_SphereSensor *node = (struct X3D_SphereSensor *)ptr;

	float *cur, *orig, *onorm;
	ttglobal tg;
	UNUSED(over);

	/* if not enabled, do nothing */
	if (!node) 
		return;
	if (node->__oldEnabled != node->enabled) {
		node->__oldEnabled = node->enabled;
		MARK_EVENT(X3D_NODE(node),offsetof (struct X3D_SphereSensor, enabled));
	}
	if (!node->enabled) 
		return;

	/* only do something if button1 is pressed */
	if (!but1) return;
	tg = gglobal();

	cur = tg->RenderFuncs.ray_save_posn;
	orig = node->_origPoint.c;
	onorm = node->_origNormalizedPoint.c;
	if (ev==ButtonPress) {
		/* record the current position from the saved position */
		veccopy3f(orig,cur);

		/* record the current Radius */
		//RADIUS = (float) sqrt(CUR_X * CUR_X + CUR_Y * CUR_Y + CUR_Z * CUR_Z);
		node->_radius = veclength3f(cur);
		if (APPROX(node->_radius,0.0)) {
			printf ("warning, RADIUS %lf == 0, can not compute\n",node->_radius);
			return;
		}

		/* save the initial norm here */
		vecscale3f(onorm,cur,1.0f / node->_radius);

		/* norm(offset) ideally this would be done once during parsing 
		  of crazy SFRotation ie '1 1 -5 .6' in 10.wrl/10.x3d 
		  I might be getting rounding errors from repeated normalization */
		vrmlrot_normalize(node->offset.c); 

		/* set isActive true */
		node->isActive=TRUE;
		MARK_EVENT (ptr, offsetof (struct X3D_SphereSensor, isActive));
	} else if (ev==ButtonRelease) {
		/* set isActive false */
		node->isActive=FALSE;
		MARK_EVENT (ptr, offsetof (struct X3D_SphereSensor, isActive));

		if (node->autoOffset) {
			veccopy4f(node->offset.c,node->rotation_changed.c);
		}
	} else if ((ev==MotionNotify) && (node->isActive)) {
		
		float dotProd;
		float newRad;
		float cnorm[3];
		float newA[4];

		/* record the current Radius */
		newRad = veclength3f(cur);
		/* bounds check... */
		if (APPROX(newRad,0.0)) {
			printf ("warning, newRad %lf == 0, can not compute\n",newRad);
			return;
		}

		/* save the current norm here */
		vecscale3f(cnorm,cur,1.0f/newRad);

		/* find the cross-product between the initial and current points */
		veccross3f(newA,orig,cur);
		vecnormalize3f(newA,newA);

		/* clamp the angle to |a| < 1.0 */
		/* remember A dot B = |A|*|B|*cos(theta_between) or theta_between = acos(A dot B/|A|*|B| ) */
		//dotProd = NORM_ORIG_X * NORM_CUR_X + NORM_ORIG_Y * NORM_CUR_Y + NORM_ORIG_Z * NORM_CUR_Z;
		dotProd = vecdot3f(onorm,cnorm);
		if (dotProd > 1.0) 
			dotProd = 1.0;
		if (dotProd < -1.0) 
			dotProd = -1.0;
		dotProd = acos(dotProd);
		newA[3] = dotProd;

		/* have axis-angle now */
		/*
		printf ("newRotation  a %lf - rot -- %lf %lf %lf %lf\n",
			dotProd, newA.x,newA.y,newA.z,dotProd);
		*/
		if(node->autoOffset)
		{

				/* copied from the javascript SFRotationMultiply */
				Quaternion q1, q2, qret;
				double newD[4];
				/* convert both rotations into quaternions */
				vrmlrot_to_quaternion(&q1, (double) newA[0], 
					(double) newA[1], (double) newA[2], (double) dotProd);
				vrmlrot_to_quaternion(&q2, (double) node->offset.c[0], 
					(double) node->offset.c[1], (double) node->offset.c[2], (double) node->offset.c[3]);
				/* multiply them */
				quaternion_multiply(&qret,&q1,&q2);
				/* and return the resultant, as a vrml rotation */
				quaternion_to_vrmlrot(&qret, &newD[0], &newD[1], &newD[2], &newD[3]);
				newA[0] = newD[0]; newA[1] = newD[1], newA[2] = newD[2], newA[3] = newD[3];
				dotProd = newD[3];
			/*}*/
		}


		veccopy4f(node->rotation_changed.c,newA);
		MARK_EVENT (ptr, offsetof (struct X3D_SphereSensor, rotation_changed));

		vecscale3f(node->trackPoint_changed.c,cnorm, node->_radius);
		MARK_EVENT (ptr, offsetof (struct X3D_SphereSensor, trackPoint_changed));
	}
}
