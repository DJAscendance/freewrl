/*


X3D Geospatial Component

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

#include "../vrml_parser/Structs.h"
#include "../vrml_parser/CRoutes.h"
#include "../main/headers.h"

#include "../world_script/fieldSet.h"
#include "../x3d_parser/Bindable.h"
#include "Collision.h"
#include "quaternion.h"
#include "Viewer.h"
#include "../opengl/Frustum.h"
#include "../opengl/Material.h"
#include "../opengl/OpenGL_Utils.h"
#include "../input/EAIHelpers.h"	/* for newASCIIString() */

#include "Polyrep.h"
#include "LinearAlgebra.h"
#include "Component_Shape.h" /* for appearance properties */
#include "Component_Geospatial.h"
#include "Children.h"
#include "../scenegraph/RenderFuncs.h"
#include "../ui/common.h"
#ifdef HAVE_GEOLIB
#include "fwgeolib.h"
#define GEOLIB
#endif

int method_geolib(){
#ifdef GEOLIB
	return 0; //freewrl hand coded way, was working fine for more than decade
	//return 1; //geographicLib / Karney, for testing - a way to independently verify transforms when hacking/refactoring code
#else
	return 0; //freewrl hand coded way
#endif
}
#define MAR12 1

void push_planetId(int planetId);
int current_planetId();
void pop_planetId();

/*
Jan 2018 dug9 understanding of ellipsoids, units, geoid, origins
* acronyms: nodes
	GVP	- geoViewpoint
	GEG	- GeoElevationGrid
	GL	- geoLocation
	GT	- geoTransform
	GPS	- geoProximitySensor
	GTS	- geoTouchSensor
	GPI	- geoPositionInterpolator
	GP	- geoPlanet (non-spec - (meaning not in web3d.org specs for their v3.3 geoSpatial component, invented here))
	GCV	- geoConverter (non-spec)

* acronyms: other
	LCS		- Local Coordinate System - shared cartesian coordinate system near nodes
			http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/geodata.html#high-precisioncoords
	TCS		- topocentric coordinate system at specific geo location, with -Z north, Y up as per GL
			http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/geodata.html#GeoLocation
	FCFS	- First Come First Served - how AutoOrigin is generated automatically now 2018: 
			- first geoNode's TCS -> shared LCS
	XTM		- general acronym for transverse mercator map projections: either UTM or 3TM
	GC		- geoCentric coordinates
	GD		- geodetic coordinates (latitude, longitude aka lat,lon)
	user	- coordinate system entered by the scene author that are in the geoSystem entered by the scene author, for the same node
				- one of GC, GD, XTM(UTM,3TM)
				- if GD, then lat,lon in degrees or radians as per geoSystem, lat or lon first
				- if XTM, then easting or northing first as per geoSystem



* XTM: {UTM,3TM} - 3TM is UTM with no false easting or northing, scale factor .9999, and 3 degree zones
  Feb 2018 we added 3TM capability
* geosystem preservation, aka User Coordinates
  we keep coordinates as they are given to us, and only convert degrees (if neceessary)
  or swizzle (exchange x, y to y, x) NE/latlon if/when needed for internal calculation purposes,
  and for shape mesh/polyrep.
  That way fields are ready for SAI and routing in users units. 
  It doesn't make sense to route directly  between different geosystems. 
  In theory there could/should be converter node types for that, 
  where you can set both input geosystem, and output geosystem, 
  or a flag on each node, saying to route in/out in (common) GC.
* concentric ellipsoids:
	we don't have a list of ellipsoid offsets (3DOF or 6DOF datum shift (and rotate)) to go between 'datums'.
   so we treat different ellipsoids as being otherwise axes-aligned and co-centric 
	with each other when transforming
	for insight into datum transforms see: http://www.dtic.mil/dtic/tr/fulltext/u2/a307127.pdf Appendix B
* v3.3 compiled-in vs freewrl compiled + supplyable ellipsoids
	The specs give a table of ellipsoids to compile in, and user can choose by name
	Feb 2018 we added  ('A#' 'F#', 'R#') to geoSystem so the user can supply other ellipsoids or spheres if needed
* ellipsoid neutrality:
	we don't try and force any one particular ellipsoid standard. The geoViewpoint's ellipsoid is
	the one we use for SPEED, LEVEL calculations
-- most coords are GC at some point, in preparation for 3D viewing, 
	and whatever ellipsoid they came from, they can mix as GC XYZ
* single planet v3.3 vs multiple planets via <GeoPlanet/>
	following specs 3.3, we can only do one world/planet in a scene. We can't do a planet and several moons 
	in geocoords in the same scene. Thats because we need to subtract the geoviewpoint
	location -or geoOrigin / autoOrigin- from geoshapes, to get coordinates into single precision float range
	for display. And to do that, we assume the geoviewpoint and geoShapes are on the
	same planet. 
	+ Feb 2018 we did 'depth slices' in rendering - up to 3 slices - to improve rendering
	with large coordinates at the single planet and multi-planet scale
	+ Mar 2018 we added <GeoPlanet planetId='0'> <GeoNode1/><GeoNode2.../> </GeoPlanet>
	  that converts LCS to GC for orbital mechanics, and pushes and pops a planetId from a stack, 
	  for use in node-node interactions. So we can now do multiple planets
* UNITS as per specs
	while we are interpreting radians as default for web3d specs v3.3, and degrees < 3.3
	we haven't tested linear UNITS conversion on parsing for geocoordinates. 
	Internally we are assuming meters. (all ellipsoid constants are in meters, 
	as are falseEasting and falseNorthing constants for UTM, and geoid height correction)
* geoid correction: needs a review to ensure 2-way, round-trip symmetry
	if set, it means the scenefile height data is with repect to mean sea level MSL
	and when converting to GC, where MSL is higher than the ellipsoid the correction is down
	and vice versa when converting back from GC to GD or XTM. 
	A test: using lat,lon of mount everest -which would have a mass that pulls sea level up-
	the correction should be GC = gdtogc(lat,lon, gdheight -abs(geoid_correction(lat,lon)) )
* geoOrigin / autoOrigin - the specs changed in v3.3 to deprecate geoOrigin
	we are supposed to automatically compute an origin to use
	Feb 2018 we are using FCFS First Come First Served - the first geoNode to compile_ we use
	its geoOrigin / geoPoint / geo something as an arbitrary origin for a LCS local coordinate
	system.
* LCS local coordinate system - a shared cartesion coordinate system for all geoNodes on one planet.
	the specs use this name for geoOrigins:
	LCSxyz = originRotation x (GCxyz - originXYZ)
	http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/geodata.html#high-precisioncoords
	All regular nodes not wrapped in GeoLocation use the LCS by default, or should.
	Viewer: a few nav modes use LCS (examine, turntable)
	GeoPlanet converts children's LCS back into GC coordinates for orbital mechanics, regular nodes working in GC,
	and inter-planet transform stacks
* TCS topocentric coordinate system aka NodeLocalSystem NLS
	somewhat related, for any giving GeoLocationNode, the topocentric coordinate system TCS with X east, -Z north, Y up
	http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/geodata.html#GeoLocation
	in freewrl we use the topocentric alignment at Origin for our originRotation aka AutoOrient
	Viewer: most nav modes use TopoCentric TCS (walk, fly, spherical, keyboard)
* relative heights - not in the specs but we use it with GVP in WALK mode and GL (geoLocation) 
	2 methods of RELATIVE:
	1. maintainRelative: (implicitly enforced in GVP WALK + COLLIDE)
		you give an absolute height, and relativeHeight0 = absolute - terrainHeight
		From then on absolute = terrainHeight + relativeHeight0
		initial position is the relative height from then on -as you animate the xy / latlong position.
	2. relativeHeight: (explicit field in GVP and GL)
		- the .position or .geoCoords you give it initially, via route or direct access is 
		  relative to terrain height, so absolute = gdCoords.c[2] + terrainHeight(gdCoords.c[0],.c[1])
		if no terrain underneath GL, then terrainHeight = ellipsoid height == 0 
* routing geo coordinates, GeoConvert
	the v3.3 specs don't mention explicitly what system the geoCoordinate
	events are in, but they must be in the geoSystem of the source node.
	It doesn't make sense to route GD output to GC or XTM input on another node.
	And v3.3 gives no way to convert.
	+ Mar 2018 we added new nodetype GeoConvert. You use 2 of them, like so:
	source.geoCoord -> user1 -> .set_geoCoord (geoConvert1) .gcCoord_Changed -> GC 
	GC -> .set_gcCoord (geoConvert2) .geoCoord_changed -> user2 -> destination.position
	where geoConvert1.geoSystem == source.geoSystem and geoConvert2.geoSystem == destination.geoSystem


* Transform stack versus GeoCoordinates
	There are 2 ways to get the relative pose of two nodes:
	a) via transform stack
	b) via GeoCoordinates
	Our current theory of operation:
	1) when doing geoNode-geoNode interactions, you should use geoCoords for both  via GC intermediary
	- that means ignoring transforms in between, and ignoring the modelview matrix
	- GC intermediary allows you to use different geoSystem for each geoNode
	2) when doing geo-regular, regular-geo interactions, you should use the transform stack
	- push/pop transforms like regular nodes do
	- transform stack is in/wrt LCS (the shared autoOrigin-related cartesian system)
	- rendering and geometry vertices are wrt stacks and LCS
	- and so get the effect of transforms inbetween
	- or better/more consistent across browsers: use helper nodes geoLocation and geoTransform
	2a) geoLocation can wrap regular nodes to convert to geo
	2b) geoTransform can wrap geoNodes to convert to regular stack
	3) planets can be separated a few different ways:
		a) Layers - using Layer_Component, with one planet per layer 
			x problem: each layer has an active viewpoint, leading to confused rendering
		b) use transform stack somehow to tell if 2 nodes are close enough to be on the same planet
		c) use a geoTransform to wrap each planet 
			(this implies you can only use one geoTransform per planet
			which may not be what specs meant, or how other browsers are using it)
		d) wrap geonodes in Planet nodes to keep them separate (but not web3d specs node)
		We use d) Planet
			- but geoTransform might be more specs-aligned by adding a planetId field to it
			- so a planet can have multiple geoTransforms if they share the same planetId

			
*/


/*
Coordinate Conversion algorithms were taken from 2 locations after
reading and comprehending the references. The code selected was
taken and modified, because the original coders "knew their stuff";
any problems with the modified code should be sent to John Stewart.

------
References:

Jean Meeus "Astronomical Algorithms", 2nd Edition, Chapter 11, page 82

"World Geodetic System"
http://en.wikipedia.org/wiki/WGS84
http://en.wikipedia.org/wiki/Geodetic_system#Conversion

"Mathworks Aerospace Blockset"
http://www.mathworks.com/access/helpdesk/help/toolbox/aeroblks/index.html?/access/helpdesk/help/toolbox/aeroblks/geocentrictogeodeticlatitude.html&http://www.google.ca/search?hl=en&q=Geodetic+to+Geocentric+conversion+equation&btnG=Google+Search&meta=

"TRANSFORMATION OF GEOCENTRIC TO GEODETIC COORDINATES WITHOUT APPROXIMATIONS"
http://www.astro.uni.torun.pl/~kb/Papers/ASS/Geod-ASS.htm

"Geodetic Coordinate Conversions"
http://www.gmat.unsw.edu.au/snap/gps/clynch_pdfs/coordcvt.pdf

"TerrestrialCoordinates.c"
http://www.lsc-group.phys.uwm.edu/lal/slug/nightly/doxygen/html/TerrestrialCoordinates_8c.html

------
Code Conversions:

Geodetic to UTM:
UTM to Geodetic:
	Geo::Coordinates::UTM - Perl extension for Latitiude Longitude conversions.
	Copyright (c) 2000,2002,2004,2007 by Graham Crookham. All rights reserved.

Geocentric to Geodetic:
Geodetic to Geocentric:
	Filename: Gdc_To_Gcc_Converter.java
	Author: Dan Toms, SRI International
	Package: GeoTransform <http://www.ai.sri.com/geotransform/>
	Acknowledgements:
	  The algorithms used in the package were created by Ralph Toms and
	  first appeared as part of the SEDRIS Coordinate Transformation API.
	  These were subsequently modified for this package. This package is
	  not part of the SEDRIS project, and the Java code written for this
	  package has not been certified or tested for correctness by NIMA.


*********************************************************************/
/* compileGeosystem - encode the return value such that srf->p[x] is... 
	0:	spatial reference frame (GEOSP_UTM, GEOSP_GC, GEOSP_GD); 
	1:	ellipsoid index (defaults to GEOSP_WE) 
	2:	UTM zone number, 1..60. INT_ID_UNDEFINED = not specified 
	3:	UTM:    if "northing_first" TRUE, if "easting_first", FALSE 
	4:	UTM:    if "S" - value is FALSE, not S, value is TRUE
	5:	GD:     if "latitude_first" TRUE, if "longitude_first", FALSE 
	6:	GD: true if geoid height
	7:	GD: TRUE: decimal degrees, FALSE radians
*/

typedef struct _geosys {
	int spatial_system;				//0
	int ellipsoid;					//1
	int xtm_zone;					//2
	int xtm_northing_first;			//3
	int utm_northern_hemisphere;	//4
	int gd_latitude_first;			//5
	int geoid_height;			//6
	int gd_degrees;					//7
	int relativeHeight;				//8
} Geosys;
#define GEOSYS( geosystem ) ((Geosys *)geosystem)
/*
geosys *mfi2geosys(struct Multi_Int32 *__geoSystem){
	return (Geosys *)__geoSystem->p;
}
*/
int isNodetypeGeospatial(int nodetype, int specversion){
	//its geospatial if it has a geoSystem field (GeoMetadata doesn't, a few DIS v3.3 do)
	int iret = 
			nodetype == NODE_GeoCoordinate || nodetype == NODE_GeoElevationGrid || nodetype == NODE_GeoLOD 
		|| nodetype == NODE_GeoLocation || nodetype == NODE_GeoPositionInterpolator 
		|| nodetype == NODE_GeoProximitySensor || nodetype == NODE_GeoTouchSensor 
		|| nodetype == NODE_GeoTransform || nodetype == NODE_GeoViewpoint || nodetype == NODE_GeoOrigin ? TRUE : FALSE;
	if(!iret && specversion > 320) {
		iret =
			nodetype == NODE_EspduTransform || nodetype == NODE_ReceiverPdu || nodetype == NODE_SignalPdu 
		|| nodetype == NODE_TransmitterPdu ? TRUE : FALSE;
	}
	return iret;
}
int isNodeGeospatial(struct X3D_Node* node){
	return isNodetypeGeospatial(node->_nodeType, X3D_PROTO(node->_executionContext)->__specversion);
}

#define STRICT33 TRUE //TRUE: if v3.3 assume incoming GD in base angle units (radians) FALSE: assume like v3.2-- degrees

/* defines used to get a SFVec3d into/outof a function that expects a MFVec3d */
#define MF_SF_TEMPS	struct Multi_Vec3d mIN; struct Multi_Vec3d  mOUT; struct Multi_Vec3d gdCoords;
#define FREE_MF_SF_TEMPS FREE_IF_NZ(gdCoords.p); FREE_IF_NZ(mOUT.p); FREE_IF_NZ(mIN.p);


#define INIT_MF_FROM_SF(myNode, myField) \
	mIN.n = 1; \
	mIN.p = MALLOC(struct SFVec3d *, sizeof (struct SFVec3d)); \
	mIN.p[0].c[0] = myNode-> myField .c[0];\
	mIN.p[0].c[1] = myNode-> myField .c[1];\
	mIN.p[0].c[2] = myNode-> myField .c[2];\
	mOUT.n=0; mOUT.p = NULL; \
	gdCoords.n=0; gdCoords.p = NULL;

//#define MF_FIELD_IN_OUT &mIN, &mOUT, &gdCoords
#define COPY_MF_TO_SF(myNode, myField) \
	myNode-> myField .c[0] = mOUT.p[0].c[0]; \
	myNode-> myField .c[1] = mOUT.p[0].c[1]; \
	myNode-> myField .c[2] = mOUT.p[0].c[2]; \
	FREE_IF_NZ(mIN.p); FREE_IF_NZ(mOUT.p);


#define MOVE_TO_ORIGIN(me)	GeoMove(X3D_NODE(node),X3D_GEOORIGIN(me->geoOrigin), GEOSYS(me->__geoSystem), &mIN, &mOUT, &gdCoords);
#define COMPILE_GEOSYSTEM(me) compile_geoSystem (X3D_NODE(me), me->_nodeType, &me->geoSystem, &me->__geoSystem);

#define RADIANS_PER_DEGREE (double)0.0174532925199432957692
#define DEGREES_PER_RADIAN (double)57.2957795130823208768

#define ENSURE_SPACE(variableInQuestion) \
	/* enough room for output? */ \
	if (variableInQuestion ->n < inCoords->n) { \
		if (variableInQuestion ->p != NULL) { \
			FREE_IF_NZ(variableInQuestion->p); \
		} \
		variableInQuestion ->p = MALLOC(struct SFVec3d *, sizeof (struct SFVec3d) * inCoords->n); \
		variableInQuestion ->n = inCoords->n; \
	} 

/* for UTM, GC, GD conversions */
#define UTM_SCALE 	(double)0.9996
#define UTM_FALSE_EASTING 500000.0 //500k m
#define UTM_FALSE_NORTHING 10000000.0 //10M m
#define UTM_ZONE_SIZE 6.0

#define U3TM_SCALE 	(double)0.9999
#define U3TM_FALSE_EASTING 0.0 
#define U3TM_FALSE_NORTHING 0.0 
#define U3TM_ZONE_SIZE 3.0


/* for Gd_Gc conversions */
#define GEOEL_AA_A	(double)6377563.396
#define GEOEL_AA_F	(double)299.3249646
#define GEOEL_AM_A	(double)6377340.189
#define GEOEL_AM_F	(double)299.3249646
#define GEOEL_AN_A	(double)6378160
#define GEOEL_AN_F	(double)298.25
#define GEOEL_BN_A	(double)6377483.865
#define GEOEL_BN_F	(double)299.1528128
#define GEOEL_BR_A	(double)6377397.155
#define GEOEL_BR_F	(double)299.1528128
#define GEOEL_CC_A	(double)6378206.4
#define GEOEL_CC_F	(double)294.9786982
#define GEOEL_CD_A	(double)6378249.145
#define GEOEL_CD_F	(double)293.465
#define GEOEL_EA_A	(double)6377276.345
#define GEOEL_EA_F	(double)300.8017
#define GEOEL_EB_A	(double)6377298.556
#define GEOEL_EB_F	(double)300.8017
#define GEOEL_EC_A	(double)6377301.243
#define GEOEL_EC_F	(double)300.8017
#define GEOEL_ED_A	(double)6377295.664
#define GEOEL_ED_F	(double)300.8017
#define GEOEL_EE_A	(double)6377304.063
#define GEOEL_EE_F	(double)300.8017
#define GEOEL_EF_A	(double)6377309.613
#define GEOEL_EF_F	(double)300.8017
#define GEOEL_FA_A	(double)6378155
#define GEOEL_FA_F	(double)298.3
#define GEOEL_HE_A	(double)6378200
#define GEOEL_HE_F	(double)298.3
#define GEOEL_HO_A	(double)6378270
#define GEOEL_HO_F	(double)297
#define GEOEL_ID_A	(double)6378160
#define GEOEL_ID_F	(double)298.247
#define GEOEL_IN_A	(double)6378388
#define GEOEL_IN_F	(double)297
#define GEOEL_KA_A	(double)6378245
#define GEOEL_KA_F	(double)298.3
#define GEOEL_RF_A	(double)6378137
#define GEOEL_RF_F	(double)298.257222101
#define GEOEL_SA_A	(double)6378160
#define GEOEL_SA_F	(double)298.25
#define GEOEL_WD_A	(double)6378135
#define GEOEL_WD_F	(double)298.26
//#define SMALLWORLDTESTING 1
#ifdef SMALLWORLDTESTING
#define GEOEL_WE_A	(double)637813.7
#define GEOEL_WE_F	(double)29.8257223563
#else
#define GEOEL_WE_A	(double)6378137
#define GEOEL_WE_F	(double)298.257223563
#endif


#define ELLIPSOIDB(typ) \
	case typ: *semimajor = typ##_A; *flattening = 1.0/typ##_F; break;

struct ellipsoid { double a, b, f;} extra_ellipsoid[10];
int nextra_ellipsoid = 1; //we start at 1 so we can use negative numbers as sentinal values to get here

int getEllipsoidParams(int etype, double *semimajor, double *flattening){
	//returns a, f where f is flattening (not inverse flattening) ie flattening = 1/298
	int iret = 0;
	*semimajor = *flattening = 0.0;
	if(etype < 0){
		//sentinal value etype is negative
		*semimajor = extra_ellipsoid[-etype].a;
		*flattening = extra_ellipsoid[-etype].f;
	}else{
		switch (etype) {
			ELLIPSOIDB(GEOEL_AA)
			ELLIPSOIDB(GEOEL_AM)
			ELLIPSOIDB(GEOEL_AN)
			ELLIPSOIDB(GEOEL_BN)
			ELLIPSOIDB(GEOEL_BR)
			ELLIPSOIDB(GEOEL_CC)
			ELLIPSOIDB(GEOEL_CD)
			ELLIPSOIDB(GEOEL_EA)
			ELLIPSOIDB(GEOEL_EB)
			ELLIPSOIDB(GEOEL_EC)
			ELLIPSOIDB(GEOEL_ED)
			ELLIPSOIDB(GEOEL_EE)
			ELLIPSOIDB(GEOEL_EF)
			ELLIPSOIDB(GEOEL_FA)
			ELLIPSOIDB(GEOEL_HE)
			ELLIPSOIDB(GEOEL_HO)
			ELLIPSOIDB(GEOEL_ID)
			ELLIPSOIDB(GEOEL_IN)
			ELLIPSOIDB(GEOEL_KA)
			ELLIPSOIDB(GEOEL_RF)
			ELLIPSOIDB(GEOEL_SA)
			ELLIPSOIDB(GEOEL_WD)
			ELLIPSOIDB(GEOEL_WE)
			default: printf ("unknown ellipsoid type: %s\n", stringGEOSPATIALType(etype));
		}
	}
	if(*semimajor > 0.0) iret = 1;
	//printf("ellipsoid etype %d semi-major %lf flattening %lf\n",etype,*semimajor,*flattening);
	return iret;
}


#define INITIALIZE_GEOSPATIAL(me) \
	initializeGeospatial((struct X3D_GeoOrigin **) &me->geoOrigin); 


void CONVERT_BACK_TO_GD_OR_UTMB(Geosys *targetGeoSystem, struct X3D_Node *GeoOrigin, 
		struct SFVec3d *thisField);

static void compile_geoSystem (struct X3D_Node *, int nodeType, struct Multi_String *args, struct X3D_Node **srf);
//static void Gd_Gc (Geosys *geoSystem, struct Multi_Vec3d *, struct Multi_Vec3d *, double, double);
static void gccToGdc (Geosys *geoSystem, struct SFVec3d *gcc, struct SFVec3d *gdc);
void calculateViewingSpeed(void);

struct Planet {
	int ID;
	Stack *gegs;
	struct SFVec4d autoOrient;
	struct SFVec3d autoOrigin;
	int autoOriginSet;
};

void clear_planets(Stack *planet_stack){
	// call from Component_Geospatial_clear() at end of run
	int i;
	struct Planet *planet;
	if(planet_stack)
	for(i=0;i<vectorSize(planet_stack);i++){
		planet = vector_get_ptr(struct Planet,planet_stack,i);
		if(planet && planet->gegs) 	deleteVector(struct X3D_Node *, planet->gegs);
	}
	deleteVector(struct Planet,planet_stack);
}

typedef struct pComponent_Geospatial{
	//struct SFVec4d autoOrient;
	//struct SFVec3d autoOrigin;
	//int autoOriginSet;
	//struct X3D_GeoOrigin *go;
	int geoLodLevel;// = 0;
	void * gcgdpars[50];
	Stack *planet_stack;
	Stack *current_planet_stack;
#ifdef GEOLIB
	void * fgeopars[50];
#endif //GEOLIB

}* ppComponent_Geospatial;
void *Component_Geospatial_constructor(){
	void *v = MALLOCV(sizeof(struct pComponent_Geospatial));
	memset(v,0,sizeof(struct pComponent_Geospatial));
	return v;
}
void Component_Geospatial_init(struct tComponent_Geospatial *t){
	//public
	//private
	t->prv = Component_Geospatial_constructor();
	{
		
		ppComponent_Geospatial p = (ppComponent_Geospatial)t->prv;
		//p->go = createNewX3DNode0(NODE_GeoOrigin);
		p->geoLodLevel = 0;
		{
			struct Planet planet;
			memset(&planet,0,sizeof(struct Planet));
			vecset4d(planet.autoOrient.c,0.0,0.0,1.0,0.0);
			vecsetd(planet.autoOrigin.c,0.0,0.0,0.0);
			planet.autoOriginSet = FALSE; //FALSE;
			
			p->planet_stack = newStack(struct Planet);
			stack_push(struct Planet,p->planet_stack,planet); //default planet

		}
		p->current_planet_stack = newStack(int);
		stack_push(int,p->current_planet_stack,0); //default planet
		memset(p->gcgdpars,0,50*sizeof(void*));
		#ifdef GEOLIB
		memset(p->fgeopars,0,50*sizeof(void*));
		#endif //GEOLIB
	}
}

void Component_Geospatial_clear(struct tComponent_Geospatial *t){
	if(t->prv )
	{
		ppComponent_Geospatial p = (ppComponent_Geospatial)t->prv;
		if(p->planet_stack){
			clear_planets(p->planet_stack);
			p->planet_stack = NULL;
		}	
		if(p->current_planet_stack){
			FREE_IF_NZ(p->current_planet_stack->data);
			FREE_IF_NZ(p->current_planet_stack);
			p->current_planet_stack = NULL;
		}

	}
}
//ppComponent_Geospatial p = (ppComponent_Geospatial)gglobal()->Component_Geospatial.prv;
void push_planetId(int planetId){
	ppComponent_Geospatial p = (ppComponent_Geospatial)gglobal()->Component_Geospatial.prv;
	Stack *current_planet_stack = (Stack*)p->current_planet_stack;
	stack_push(int,current_planet_stack,planetId);
}
int current_planetId(){
	ppComponent_Geospatial p = (ppComponent_Geospatial)gglobal()->Component_Geospatial.prv;
	Stack *current_planet_stack = (Stack*)p->current_planet_stack;
	return stack_top(int,current_planet_stack);
}
void pop_planetId(){
	ppComponent_Geospatial p = (ppComponent_Geospatial)gglobal()->Component_Geospatial.prv;
	Stack *current_planet_stack = (Stack*)p->current_planet_stack;
	stack_pop(int,current_planet_stack);
}
struct Planet *add_planet(int planetId){
	struct Planet planet, *ppointer;
	ppComponent_Geospatial p = (ppComponent_Geospatial)gglobal()->Component_Geospatial.prv;
	memset(&planet,0,sizeof(struct Planet));
	vecset4d(planet.autoOrient.c,0.0,0.0,1.0,0.0);
	vecsetd(planet.autoOrigin.c,0.0,0.0,0.0);
	planet.ID = planetId;
	planet.autoOriginSet = FALSE; //FALSE;
	stack_push(struct Planet,p->planet_stack,planet); //default planet
	ppointer = vector_get_ptr(struct Planet,p->planet_stack,p->planet_stack->n-1);
	return ppointer;
}
struct Planet *current_planet(){
	int planetId, i;
	struct Planet *planet;
	ppComponent_Geospatial p = (ppComponent_Geospatial)gglobal()->Component_Geospatial.prv;
	planetId = stack_top(int,p->current_planet_stack);
	//we should sort planets by planetId or use a hash, or compile planetId to index
	planet = NULL;
	for(i=0;i<vectorSize(p->planet_stack);i++){
		planet = vector_get_ptr(struct Planet,p->planet_stack,i);
		if(planetId == planet->ID){
			return planet;
		}
	}
	return NULL;
}


// http://www.colorado.edu/geography/gcraft/notes/datum/geoid84.html
char geoid[][36] = {
//-180 longitude  ---------------------------------------------------------- + 170 longitude
{13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13}, //+90 north pole
{3,1,-2,-3,-3,-3,-1,3,1,5,9,11,19,27,31,34,33,34,33,34,28,23,17,13,9,4,4,1,-2,-2,0,2,3,2,1,1}, //+80
{2,2,1,-1,-3,-7,-14,-24,-27,-25,-19,3,24,37,47,60,61,58,51,43,29,20,12,5,-2,-10,-14,-12,-10,-14,-12,-6,-2,3,6,4}, //+70
{2,9,17,10,13,1,-14,-30,-39,-46,-42,-21,6,29,49,65,60,57,47,41,21,18,14,7,-3,-22,-29,-32,-32,-26,-15,-2,13,17,19,6}, //+60
{-8,8,8,1,-11,-19,-16,-18,-22,-35,-40,-26,-12,24,45,63,62,59,47,48,42,28,12,-10,-19,-33,-43,-42,-43,-29,-2,17,23,22,6,2}, //+50
{-12,-10,-13,-20,-31,-34,-21,-16,-26,-34,-33,-35,-26,2,33,59,52,51,52,48,35,40,33,-9,-28,-39,-48,-59,-50,-28,3,23,37,18,-1,-11}, //+40
{-7,-5,-8,-15,-28,-40,-42,-29,-22,-26,-32,-51,-40,-17,17,31,34,44,36,28,29,17,12,-20,-15,-40,-33,-34,-34,-28,7,29,43,20,4,-6}, //+30
{5,10,7,-7,-23,-39,-47,-34,-9,-10,-20,-45,-48,-32,-9,17,25,31,31,26,15,6,1,-29,-44,-61,-67,-59,-36,-11,21,39,49,39,22,10}, //+20
{13,12,11,2,-11,-28,-38,-29,-10,3,1,-11,-41,-42,-16,3,17,33,22,23,2,-3,-7,-36,-59,-90,-95,-63,-24,12,53,60,58,46,36,26}, //+10
{22,16,17,13,1,-12,-23,-20,-14,-3,14,10,-15,-27,-18,3,12,20,18,12,-13,-9,-28,-49,-62,-89,-102,-63,-9,33,58,73,74,63,50,32}, //equator
{36,22,11,6,-1,-8,-10,-8,-11,-9,1,32,4,-18,-13,-9,4,14,12,13,-2,-14,-25,-32,-38,-60,-75,-63,-26,0,35,52,68,76,64,52}, //-10
{51,27,10,0,-9,-11,-5,-2,-3,-1,9,35,20,-5,-6,-5,0,13,17,23,21,8,-9,-10,-11,-20,-40,-47,-45,-25,5,23,45,58,57,63}, //-20
{46,22,5,-2,-8,-13,-10,-7,-4,1,9,32,16,4,-8,4,12,15,22,27,34,29,14,15,15,7,-9,-25,-37,-39,-23,-14,15,33,34,45}, //-30
{21,6,1,-7,-12,-12,-12,-10,-7,-1,8,23,15,-2,-6,6,21,24,18,26,31,33,39,41,30,24,13,-2,-20,-32,-33,-27,-14,-2,5,20}, //-40
{-15,-18,-18,-16,-17,-15,-10,-10,-8,-2,6,14,13,3,3,10,20,27,25,26,34,39,45,45,38,39,28,13,-1,-15,-22,-22,-18,-15,-14,-10}, //-50
{-45,-43,-37,-32,-30,-26,-23,-22,-16,-10,-2,10,20,20,21,24,22,17,16,19,25,30,35,35,33,30,27,10,-2,-14,-23,-30,-33,-29,-35,-43}, //-60
{-61,-60,-61,-55,-49,-44,-38,-31,-25,-16,-6,1,4,5,4,2,6,12,16,16,17,21,20,26,26,22,16,10,-1,-16,-29,-36,-46,-55,-54,-59}, //-70
{-53,-54,-55,-52,-48,-42,-38,-38,-29,-26,-26,-24,-23,-21,-19,-16,-12,-8,-4,-1,1,4,4,6,5,4,2,-6,-15,-24,-33,-40,-48,-50,-53,-52}, //-80
{-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30} //-90 south pole
};

static double geoidCorrection(double latitudeDeg, double longitudeDeg)
{
	//assumes: GD coords in degrees, -180 to +180 range for longitude, -90 to +90 latitude range
	//returns: what to add to sea level height to get an ellipsoid height
	//  - does not add it - you do that in the caller.
	//  - http://en.wikipedia.org/wiki/Geoid  has a global diagram to check the sign/sense of the correction
	//benefits: allows you to mix GPS and topographic data in the same scene and have good alignment
	//usage: put 2 GeoOrigins one for GPS and one for local Topographic in your scene, but set them (initially)
	//  both the same, and set them both without the 'WGS84' (geoid) geosystem option.
	//  Apply option 'WGS84' (geoid) option to your topographic data GeoCoordinate GeoSystem (except GeoOrigin).
	//  Then touch up when viewing them in the same scene by adjusting your local topographic geoOrigin height.
	int il, ip, il1, ip1;
	double dl, dp, d00, d01, d10, d11, d;
	//step 1: find the cell indexes
	il = (int)(longitudeDeg/10.0) + 18 -1; //longitude cell
	ip = 18 - ((int)(latitudeDeg/10.0) + 9);  //latitude cell
	il1 = il + 1;
	ip1 = ip - 1;
	if(il1 > 35) il1 = 0;
	if(ip1 < 0) ip1 = 0;
	//step 2: compute the corners of the cell 
	d00 = (float)geoid[ip][il]; //lower left
	d01 = (float)geoid[ip1][il]; //upper left
	d10 = (float)geoid[ip][il1]; //lower right
	d11 = (float)geoid[ip1][il1]; //upper right
	//step 3: find the position in the cell
	dl = longitudeDeg - (-180.0 + (float)(il)*10.0);
	dp = latitudeDeg - (90.0 - (float)(ip)*10.0);
	//step 4: find the normalized position within the cell range 0-1
	dl /= 10.0;
	dp /= 10.0;
	//step 5: bilinear interpolate from 4 corners to position in cell
	d = (1.0 - dl)*(1.0 - dp)*d00 + (dl)*(1.0 - dp)*d10 + (1.0-dl)*(dp)*d01 + (dl)*(dp)*d11;
	// d is how much higher the geoid is from the ellipsoid. 
	d = -d; // to correct a geoid map to ellpsoid, subtract this amount
	return (double)d;
}
/* move ourselves BACK to the from the GeoOrigin */
//static void retractOrigin(struct X3D_GeoOrigin *myGeoOrigin, struct SFVec3d *gcCoords) {
//	if (myGeoOrigin != NULL) {
//		if(myGeoOrigin->rotateYUp == TRUE)
//		{
//			int i;
//			Quaternion rq;
//			struct SFVec3d temp;
//			vrmlrot_to_quaternion(&rq,myGeoOrigin->__rotyup.c[0], myGeoOrigin->__rotyup.c[1], myGeoOrigin->__rotyup.c[2], myGeoOrigin->__rotyup.c[3]); 
//			//quaternion_multi_rotation(outxyz,&rq,inxyz,8);
//			quaternion_rotation((struct point_XYZ *)temp.c, &rq, (const struct point_XYZ *)gcCoords->c);
//			for(i=0;i<3;i++)
//				gcCoords->c[i] = temp.c[i];
//		}
//		gcCoords->c[0] += myGeoOrigin->__movedCoords.c[0];
//		gcCoords->c[1] += myGeoOrigin->__movedCoords.c[1];
//		gcCoords->c[2] += myGeoOrigin->__movedCoords.c[2];
//	}
//}


/* convert GD ellipsiod to GC coordinates. swizzles and converts degrad as needed. */

// swizzles and converts degrad as needed
static void Gd_Gc3d_fw(Geosys *geoSystem, struct SFVec3d *inc, int n, struct SFVec3d *outc) {
	int geotype, lat_first, geoid;
	double radius, flattening;
	geotype = geoSystem->ellipsoid;
	lat_first = geoSystem->gd_latitude_first;
	geoid = geoSystem->geoid_height;
	if(getEllipsoidParams(geotype,&radius,&flattening))
	{
		int i;
		double A = radius;
		double A2 = radius*radius;
		double F = flattening;
		double C = A*((double)1.0 - F);
		double C2 = C*C;
		double Eps2 = F*((double)2.0 - F);
		double Eps25 = (double) 0.25 * Eps2;

		int latitude = 0;
		int longitude = 1;
		int elevation = 2;

		double source_lat;
		double source_lon;
		double slat;
		double slat2;
		double clat;
		double Rn;
		double RnPh;

		if (!lat_first) {
			printf ("Gd_Gc, NOT lat first\n");
			latitude = 1; longitude = 0;
		}

		///* enough room for output? */
		//if (outc->n < inc->n) {
		//	FREE_IF_NZ(outc->p);
		//	outc->p = MALLOC(struct SFVec3d *, sizeof (struct SFVec3d) * inc->n);
		//	outc->n = inc->n;
		//}
		#ifdef VERBOSE
		printf ("Gd_Gc, have n of %d\n",inc->n);
		#endif

		for (i=0; i<n; i++) {
			#ifdef VERBOSE
			printf ("Gd_Gc, ining lat %lf long %lf ele %lf   ",LATITUDE_IN, LONGITUDE_IN, ELEVATION_IN);
			#endif

			if(geoSystem->gd_degrees == FALSE){
				//version 3.3+ by default in 'angle base units' which are radians
				source_lat = inc[i].c[latitude]; //LATITUDE_IN;
				source_lon = inc[i].c[longitude]; //LONGITUDE_IN;
			}else{
				//version 3.2- by default in degrees
				source_lat = RADIANS_PER_DEGREE * inc[i].c[latitude];// LATITUDE_IN;
				source_lon = RADIANS_PER_DEGREE * inc[i].c[longitude]; //LONGITUDE_IN;
			}
	
			#ifdef VERBOSE
			printf ("Source Latitude  %lf Source Longitude %lf\n",source_lat, source_lon);
			#endif

			slat = sin(source_lat);
			slat2 = slat*slat;
			clat = cos(source_lat);
	
			#ifdef VERBOSE
			printf ("slat %lf slat2 %lf clat %lf\n",slat, slat2, clat);
			#endif


			/* square root approximation for Rn */
			Rn = A / ( (.25 - Eps25 * slat2 + .9999944354799/4) + (.25-Eps25 * slat2)/(.25 - Eps25 * slat2 + .9999944354799/4));
	
			RnPh = Rn + inc[i].c[elevation]; //ELEVATION_IN;
			
			//MAR 2018 we are doing geoid at the user2something, something2user level now, higher in the call stack
			if(geoid && FALSE){
				double dlatin, dlongin;
				dlatin = inc[i].c[latitude]; //LATITUDE_IN;
				dlongin = inc[i].c[longitude]; //LONGITUDE_IN;
				if(geoSystem->gd_degrees == FALSE){
					dlatin *= DEGREES_PER_RADIAN;
					dlongin *= DEGREES_PER_RADIAN;
				}
				RnPh += geoidCorrection(dlatin,dlongin); //LATITUDE_IN,LONGITUDE_IN);
			}
			#ifdef VERBOSE
			printf ("Rn %lf RnPh %lf\n",Rn, RnPh);
			#endif

			outc[i].c[0] = RnPh * clat * cos(source_lon); //GC_X_OUT 
			outc[i].c[1] = RnPh * clat * sin(source_lon);
			outc[i].c[2]  = ((C2 / A2) * Rn + inc[i].c[elevation]) * slat; //ELEVATION_IN

			#ifdef VERBOSE
			printf ("Gd_Gc, outing x %lf y %lf z %lf\n", GC_X_OUT, GC_Y_OUT, GC_Z_OUT);
			#endif
		}
	}
}
#ifdef GEOLIB
double dclamp(double fval, double fstart, double fend);
static void* fwgeo_gc[50];
static void Gd_Gc3d_geolib(Geosys *geoSystem, struct SFVec3d *inc, int n, struct SFVec3d *outc){
	int i,geotype;
	double semimajor, flattening, gd[3], gc[3];
	//printf("hi from Gd_Gc3d_geolib\n");
	getEllipsoidParams(geoSystem->ellipsoid,&semimajor,&flattening);
	if(FALSE && flattening == 0.0){
		//easy spherical coords, but geolib OK without this, just for testing here
		double radius;
		for(i=0;i<n;i++){
			veccopyd(gd,inc[i].c);
			if(!geoSystem->gd_latitude_first) vecswizzle2d(gd);
			if(geoSystem->gd_degrees) vecscale2d(gd,gd,RADIANS_PER_DEGREE);
			radius = semimajor + gd[2];
			gc[0] = cos(gd[0])*cos(gd[1])*radius;
			gc[1] = cos(gd[0])*sin(gd[1])*radius;
			gc[2] = sin(gd[0])*radius;
			veccopyd(outc[i].c,gc);
			//if(i<10) printf("gd2gc sphere %lf %lf %lf\n",gc[0],gc[1],gc[2]);
		}
	}
	else
	{
		geotype = geoSystem->ellipsoid;
		if(geotype < 0) geotype = -geotype + GEOELLIPSOID_COUNT;
		if(!fwgeo_gc[geotype]){
			fwgeo_gc[geotype] = fgeo_initializeGC(semimajor,flattening);
		}
		for(i=0;i<n;i++){
			veccopyd(gd,inc[i].c);
			if(!geoSystem->gd_latitude_first) vecswizzle2d(gd);
			if(!geoSystem->gd_degrees) vecscale2d(gd,gd,DEGREES_PER_RADIAN); //geolib uses degrees
			// if you get 1.#QNAN00000000000 coming out here, its because geolib insists on -90,90 for lat
			// and GEG has a bit of rounding error noise that exceeds slightly .0000001
			//if(n>1 && i < 200) printf("before clamp %ld %lf %lf %lf\n",i,gd[0],gd[1],gd[2]);
			gd[0] = dclamp(gd[0],-90.0,90.0);
			fgeo_gd2gc(fwgeo_gc[geotype], gd[0], gd[1], gd[2], &gc[0], &gc[1], &gc[2]);
			veccopyd(outc[i].c,gc);
			//if(i<10) printf("gd2gc geolb %lf %lf %lf\n",gc[0],gc[1],gc[2]);
		}
	}
	if(n>1) 
		getchar();
}
#endif //GEOLIB
static void Gd_Gc3d(Geosys *geoSystem, struct SFVec3d *inc, int n, struct SFVec3d *outc){
	int i;
#ifdef GEOLIB
	if(method_geolib()){
		Gd_Gc3d_geolib(geoSystem,inc,n,outc);
		//printf("geolib gd:\n");
		//for(i=0;i<min(200,n);i++){
		//	printf("%d %lf %lf %lf\n",i,outc[i].c[0],outc[i].c[1],outc[i].c[2]);
		//}
	}else
#endif //GEOLIB
	{
		double semimajor, flattening;
		getEllipsoidParams(geoSystem->ellipsoid,&semimajor,&flattening);
		if(flattening == 0.0){
			//easy spherical coords
			//Gd_Gc3d_fw and/or its gc2gd complement has a problem with moon geoSystem 'R173...' 'F0.0'
			double radius, gd[3], gc[3];
			for(i=0;i<n;i++){
				veccopyd(gd,inc[i].c);
				if(!geoSystem->gd_latitude_first) vecswizzle2d(gd);
				if(geoSystem->gd_degrees) vecscale2d(gd,gd,RADIANS_PER_DEGREE);
				radius = semimajor + gd[2];
				gc[0] = cos(gd[0])*cos(gd[1])*radius;
				gc[1] = cos(gd[0])*sin(gd[1])*radius;
				gc[2] = sin(gd[0])*radius;
				veccopyd(outc[i].c,gc);
				//if(i<10) printf("gd2gc sphere %lf %lf %lf\n",gc[0],gc[1],gc[2]);
			}
		}
		else
		{
			Gd_Gc3d_fw(geoSystem,inc,n,outc);
			//printf("fw gd:\n");
			//for(i=0;i<min(5,n);i++){
			//	printf("%d %lf %lf %lf\n",i,outc[i].c[0],outc[i].c[1],outc[i].c[2]);
			//}
			//printf("\n");
		}
	}
}
/* convert UTM to GC coordinates by converting to GD as an intermediary step 
   we swizzle both lat,long and east,north
   we convert to radians if necessary
*/
static void Xtm_Gd3d(Geosys *geoSystem, struct SFVec3d *inc, int n, struct SFVec3d *outc, double radius, double flatten, 
	double scaleFactor, double falseEasting, double falseNorthing, double zoneSize) {

	int hemisphere_north, zone, northing_first;
	int i;
	int northing = 0;	/* for determining which input value is northing */
	int easting = 1;	/* for determining which input value is easting */
	int elevation = 2;	/* elevation is always third value, input AND output */
	int latitude = 0;	/* always return latitude as first value */
	int longitude = 1;	/* always return longtitude as second value */

	/* create the ERM constants. */
	double F = flatten;
	double Eccentricity   = (F) * (2.0-F);

	double myEasting;
	double myphi1rad;
	double myN1;
	double myT1;
	double myC1;
	double myR1;
	double myD;
	double Latitude;
	double Longitude;
	double longitudeOriginDeg;
	double myeccPrimeSquared;
	double myNorthing;
	double northingDRCT1;
	double eccRoot;
	double calcConstantTerm1;
	double calcConstantTerm2;
	double calcConstantTerm3;
	double calcConstantTerm4;

	if(geoSystem->gd_latitude_first == FALSE){
		latitude = 1;
		longitude = 0;
	}

	hemisphere_north = geoSystem->utm_northern_hemisphere;
	zone = geoSystem->xtm_zone;
	northing_first = geoSystem->xtm_northing_first;


	/* is the values specified with an "easting_first?" */
	if (!northing_first) { northing = 1; easting = 0; }
	//printf("Xtm_Gd hemisphere-north=%d zone=%d northing_first=%d\n",hemisphere_north,zone,northing_first);
	//printf("Xtm_Gd scalefactor %lf falseEasting %lf falseNorthing %lf zoneSize %lf\n",scaleFactor, falseEasting, falseNorthing, zoneSize);

	#ifdef VERBOSE
	if (!northing_first) printf ("UTM to GD, not northing first, flipping norhting and easting\n");
	#endif
		
	#ifdef VERBOSE
	if (northing_first) printf ("Utm_Gd: northing first\n"); else printf ("Utm_Gd: NOT northing_first\n");
	if (!hemisphere_north) printf ("Utm_Gd: NOT hemisphere_north\n"); else printf ("Utm_Gd: hemisphere_north\n"); 
	#endif


	/* enough room for output? */
	//if (outc->n < inc->n) {
	//	FREE_IF_NZ(outc->p);
	//	outc->p = MALLOC(struct SFVec3d *, sizeof (struct SFVec3d) * inc->n);
	//	outc->n = inc->n;
	//}

	/* constants for all UTM vertices */
	longitudeOriginDeg = (zone -1) * zoneSize - 180. + zoneSize*.5;
	myeccPrimeSquared = Eccentricity/(((double) 1.0) - Eccentricity);
	eccRoot = (((double)1.0) - sqrt (((double)1.0) - Eccentricity))/
	       (((double)1.0) + sqrt (((double)1.0) - Eccentricity));

	calcConstantTerm1 = ((double)1.0) -Eccentricity/
		((double)4.0) - ((double)3.0) *Eccentricity*Eccentricity/
		((double)64.0) -((double)5.0) *Eccentricity*Eccentricity*Eccentricity/((double)256.0);

	calcConstantTerm2 = ((double)3.0) * eccRoot/((double)2.0) - ((double)27.0) *eccRoot*eccRoot*eccRoot/((double)32.0);
	calcConstantTerm3 = ((double)21.0) * eccRoot*eccRoot/ ((double)16.0) - ((double)55.0) *eccRoot*eccRoot*eccRoot*eccRoot/ ((double)32.0);
	calcConstantTerm4 = ((double)151.0) *eccRoot*eccRoot*eccRoot/ ((double)96.0);

	#ifdef VERBOSE
	printf ("zone %d\n",zone);
	printf ("longitudeOriginDeg %lf\n",longitudeOriginDeg);
	printf ("myeccPrimeSquared %lf\n",myeccPrimeSquared);
	printf ("eccRoot %lf\n",eccRoot);
	#endif

	/* go through each vertex specified */
    for(i=0;i<n;i++) {
		/* get the values for THIS UTM vertex */
		outc[i].c[elevation] = inc[i].c[elevation]; //ELEVATION_OUT = ELEVATION_IN;
				
		myEasting = inc[i].c[easting] - falseEasting; //UTM_FALSE_EASTING; //500000; //EASTING_IN 
		if (hemisphere_north) myNorthing = inc[i].c[northing]; //NORTHING_IN;
		else myNorthing = inc[i].c[northing] - falseNorthing; //(double)UTM_FALSE_NORTHING; //10000000.0; //NORTHING_IN

		#ifdef VERBOSE
		printf ("myEasting %lf\n",myEasting);
		printf ("myNorthing %lf\n",myNorthing);
		#endif


		/* scale the northing */
		myNorthing= myNorthing / scaleFactor;
		northingDRCT1 = myNorthing /(radius * calcConstantTerm1);

		myphi1rad = northingDRCT1 + 
			calcConstantTerm2 * sin(((double)2.0) *northingDRCT1)+
			calcConstantTerm3 * sin(((double)4.0) *northingDRCT1)+
			calcConstantTerm4 * sin(((double)6.0) *northingDRCT1);

		myN1 = radius/sqrt(((double)1.0) - Eccentricity * sin(myphi1rad) * sin (myphi1rad));
		myT1 = tan(myphi1rad) * tan(myphi1rad); 
		myC1 = Eccentricity * cos(myphi1rad) * cos (myphi1rad);
		myR1 = radius * (((double)1.0) - Eccentricity) / pow(((double)1.0) - Eccentricity * sin(myphi1rad) * sin (myphi1rad), 1.5);
		myD = myEasting/(myN1*scaleFactor);

		Latitude = myphi1rad-(myN1*tan(myphi1rad)/myR1)*
				(myD*myD/((double)2.0) -
			(((double)5.0) + ((double)3.0) *myT1+ ((double)10.0) *myC1-
			((double)4.0) *myC1*myC1- ((double)9.0) *myeccPrimeSquared)*
			myD*myD*myD*myD/((double)24.0) +(((double)61.0) +((double)90.0) *
			myT1+((double)298.0) *myC1+ ((double)45.0) *myT1*myT1-
			((double)252.0) * myeccPrimeSquared- ((double)3.0) *myC1*myC1)*myD*myD*myD*myD*myD*myD/((double)720.0));

		Longitude = (myD-(((double)1.0)+((double)2.0)*myT1+myC1)*myD*myD*myD/((double)6.0)+(((double)5.0) - ((double)2.0) *myC1+
			((double)28.0) *myT1-((double)3.0) *myC1*myC1+
			((double)8.0) *myeccPrimeSquared+((double)24.0) *myT1*myT1)*myD*myD*myD*myD*myD/120)/cos(myphi1rad);

		if(geoSystem->gd_degrees == FALSE){
			//version 3.3+ works in angle base units (radians) by default
			outc[i].c[latitude] = Latitude ; //LATITUDE_OUT
			outc[i].c[longitude] = longitudeOriginDeg*RADIANS_PER_DEGREE + Longitude; //LONGITUDE_OUT
		}else{
			//version 3.2- works in degrees by default
			outc[i].c[latitude] = Latitude * DEGREES_PER_RADIAN;
			outc[i].c[longitude] = longitudeOriginDeg + (Longitude * DEGREES_PER_RADIAN);
		}


		#ifdef VERBOSE
		/* printf ("myNorthing scaled %lf\n",myNorthing);
		printf ("northingDRCT1 %lf\n",northingDRCT1);
		printf ("myphi1rad %lf\n",myphi1rad);
		printf ("myN1 %lf\n",myN1);
		printf ("myT1 %lf\n",myT1);
		printf ("myC1 %lf\n",myC1);
		printf ("myR1 %lf\n",myR1);
		printf ("myD %lf\n",myD);
		printf ("latitude %lf\n",Latitude);
		printf ("longitude %lf\n",Longitude);
		*/
		printf ("utmtogd\tnorthing %lf easting %lf ele %lf\n\tlat %lf long %lf ele %lf\n", NORTHING_IN, EASTING_IN, ELEVATION_IN, LATITUDE_OUT, LONGITUDE_OUT, ELEVATION_IN);
		#endif
        } 
}

#ifdef GEOLIB
static void Xtm_Gd3d_geolib(Geosys *geoSystem, struct SFVec3d *inc, int n, struct SFVec3d *outc, 
	double radius, double flatten, 	double scaleFactor, double falseEasting, double falseNorthing, 
	double zoneSize) {

	int hemisphere_north, zone, northing_first, geotype;
	int i;
	int northing = 0;	/* for determining which input value is northing */
	int easting = 1;	/* for determining which input value is easting */
	int elevation = 2;	/* elevation is always third value, input AND output */
	int latitude = 0;	/* always return latitude as first value */
	int longitude = 1;	/* always return longtitude as second value */

	/* create the ERM constants. */
	double F = flatten;
	double dlon0;
	double dLatitude;
	double dLongitude;
	double dlongitudeOrigin;
	double myEasting;
	double myNorthing;
	void *fgeo;
	ppComponent_Geospatial p = (ppComponent_Geospatial)gglobal()->Component_Geospatial.prv;

	if(geoSystem->gd_latitude_first == FALSE){
		latitude = 1;
		longitude = 0;
	}

	hemisphere_north = geoSystem->utm_northern_hemisphere;
	zone = geoSystem->xtm_zone;
	northing_first = geoSystem->xtm_northing_first;
	geotype = geoSystem->ellipsoid;
	if(geotype < 0) geotype = -geotype + GEOELLIPSOID_COUNT;
	if(!p->fgeopars[geotype])
		p->fgeopars[geotype] = fgeo_initializeTM(radius, F, 1.0);
	fgeo = p->fgeopars[geotype];
	/* is the values specified with an "easting_first?" */
	if (!northing_first) { northing = 1; easting = 0; }
	//printf("Xtm_Gd hemisphere-north=%d zone=%d northing_first=%d\n",hemisphere_north,zone,northing_first);
	//printf("Xtm_Gd scalefactor %lf falseEasting %lf falseNorthing %lf zoneSize %lf\n",scaleFactor, falseEasting, falseNorthing, zoneSize);

	#ifdef VERBOSE
	if (!northing_first) printf ("UTM to GD, not northing first, flipping norhting and easting\n");
	#endif
		
	#ifdef VERBOSE
	if (northing_first) printf ("Utm_Gd: northing first\n"); else printf ("Utm_Gd: NOT northing_first\n");
	if (!hemisphere_north) printf ("Utm_Gd: NOT hemisphere_north\n"); else printf ("Utm_Gd: hemisphere_north\n"); 
	#endif

	/* constants for all UTM vertices */
	dlongitudeOrigin = (zone -1) * zoneSize - 180. + zoneSize*.5;

	/* go through each vertex specified */
    for(i=0;i<n;i++) {
		/* get the values for THIS UTM vertex */
		outc[i].c[elevation] = inc[i].c[elevation]; //ELEVATION_OUT = ELEVATION_IN;
				
		myEasting = inc[i].c[easting] - falseEasting; //UTM_FALSE_EASTING; //500000; //EASTING_IN 
		if (hemisphere_north) myNorthing = inc[i].c[northing]; //NORTHING_IN;
		else myNorthing = inc[i].c[northing] - falseNorthing; //(double)UTM_FALSE_NORTHING; //10000000.0; //NORTHING_IN

		myEasting /= scaleFactor;
		myNorthing /= scaleFactor;

		#ifdef VERBOSE
		printf ("myEasting %lf\n",myEasting);
		printf ("myNorthing %lf\n",myNorthing);
		#endif

		//this adds CM longitude on, no need to add it later.
		// works in decimal degrees
		fgeo_tm2gd(fgeo,myEasting, myNorthing, dlongitudeOrigin, &dLatitude, &dLongitude);

		if(geoSystem->gd_degrees == FALSE){
			//version 3.3+ works in angle base units (radians) by default
			outc[i].c[latitude] = dLatitude * RADIANS_PER_DEGREE ; //LATITUDE_OUT
			outc[i].c[longitude] = dLongitude*RADIANS_PER_DEGREE ; //LONGITUDE_OUT
		}else{
			//version 3.2- works in degrees by default
			outc[i].c[latitude] = dLatitude;
			outc[i].c[longitude] = dLongitude;
		}
	} 
}
#endif //GEOLIB
/* compileGeosystem - encode the return value such that srf->p[x] is... 
	0:	spatial reference frame (GEOSP_UTM, GEOSP_GC, GEOSP_GD); 
	1:	ellipsoid index (defaults to GEOSP_WE) 
	2:	UTM zone number, 1..60. INT_ID_UNDEFINED = not specified 
	3:	UTM:    if "northing_first" TRUE, if "easting_first", FALSE 
	4:	UTM:    if "S" - value is FALSE, not S, value is TRUE
	5:	GD:     if "latitude_first" TRUE, if "longitude_first", FALSE 
	6:	GD: true if geoid height
	7:	GD: TRUE: decimal degrees, FALSE radians
*/
static void gdToUtm3d(Geosys *geoSystem, double *gdcoords, double *xtmcoords);
static void gdTo3tm3d(Geosys *geoSystem, double *gdcoords, double *xtmcoords);
static void Utm_Gd3d(Geosys *geoSystem, struct SFVec3d *inc, int n, struct SFVec3d *outc) {
	double semimajor, flattening;
	getEllipsoidParams(geoSystem->ellipsoid,&semimajor,&flattening);
	#ifdef GEOLIB
	if(method_geolib())
		Xtm_Gd3d_geolib(geoSystem, inc, n, outc, semimajor, flattening, UTM_SCALE, UTM_FALSE_EASTING, UTM_FALSE_NORTHING, UTM_ZONE_SIZE);
	else
	#endif GEOLIB
		Xtm_Gd3d(geoSystem, inc, n, outc, semimajor, flattening, UTM_SCALE, UTM_FALSE_EASTING, UTM_FALSE_NORTHING, UTM_ZONE_SIZE);
	if(0){
		//round trip verification, want to convert a UTM -> GD -> (UTM, 3TM)
		double xtm[3], neh[3];
		int izone = -1;
		printf("in UTM_gd3d\n");
		printf("UTM y %lf x %lf h %lf zone %d\n",inc->c[0],inc->c[1],inc->c[2],geoSystem->xtm_zone);
		gdToUtm3d(geoSystem,outc->c, xtm); 
		veccopyd(neh,xtm);
		if(!geoSystem->xtm_northing_first) vecswizzle2d(neh);
		printf("UTM y %lf x %lf h %lf zone %d\n",neh[0],neh[1],neh[2],geoSystem->xtm_zone);
		izone = -1;
		gdTo3tm3d(geoSystem,outc->c, xtm); 
		veccopyd(neh,xtm);
		if(!geoSystem->xtm_northing_first) vecswizzle2d(neh);
		printf("3TM y %lf x %lf h %lf zone %d\n",neh[0],neh[1],neh[2],geoSystem->xtm_zone);
	}
}
static void U3tm_Gd3d(Geosys *geoSystem, struct SFVec3d *inc, int n, struct SFVec3d *outc) {
	double semimajor, flattening;
	getEllipsoidParams(geoSystem->ellipsoid,&semimajor,&flattening);
	#ifdef GEOLIB
	if(method_geolib())
		Xtm_Gd3d_geolib(geoSystem, inc, n, outc, semimajor, flattening, U3TM_SCALE, U3TM_FALSE_EASTING, U3TM_FALSE_NORTHING, U3TM_ZONE_SIZE);
	else
	#endif //GEOLIB
		Xtm_Gd3d(geoSystem, inc, n, outc, semimajor, flattening, U3TM_SCALE, U3TM_FALSE_EASTING, U3TM_FALSE_NORTHING, U3TM_ZONE_SIZE);

}
double getTerrainHeight(int planetID, Geosys *geoSystem, struct SFVec3d *gdCoord);
double userHeight2ellipsoidHeight(Geosys *geoSystem, struct SFVec3d *gdCoord){
	double additionalHeight = 0.0;
	if(geoSystem->geoid_height == TRUE){
		//MSL (mean sea level) to ellipsoid height
		// ellipsoidHeight = MSL + geoid(location)
		//  so gd are in ellipsoid heights like GPS? (vs sea level heights)
		double gddegrees[3];
		if(geoSystem->gd_degrees) veccopyd(gddegrees,gdCoord->c);
		else vecscaled(gddegrees,gdCoord->c,DEGREES_PER_RADIAN);
		if(!geoSystem->gd_latitude_first) vecswizzle2d(gddegrees);
		additionalHeight += geoidCorrection(gddegrees[0],gddegrees[1]);
	}
	if(geoSystem->relativeHeight == TRUE){
		// ellipsoidHeight = height + TerrainHeight(planet,location)
		struct Planet *planet = current_planet();
		additionalHeight += getTerrainHeight( planet->ID, geoSystem, gdCoord);

	}
	return additionalHeight;
}

/* take a set of coords, and a geoSystem, and create a set of moved coords */
/* we keep around the GD coords because we need them for rotation calculations */
/* parameters: 
	geoSystem:	compiled geoSystem integer array pointer
	inCoords:	coordinate structure for input coordinates, ANY coordinate type
	outCoords:	area for GC coordinates. Will MALLOC size if required 
	gdCoords:	GD coordinates, used for rotation calculations in later stages. WILL MALLOC THIS */


static void moveCoords3d (Geosys * geoSystem, struct SFVec3d *offset, struct SFVec4d *yup,
	struct SFVec3d *inCoords, int n, struct SFVec3d *outCoords, struct SFVec3d *gdCoords) {
	// offset is in GC
	int i;

	/* GD Geosystem - copy coordinates, and convert them to GC */
	switch (geoSystem->spatial_system) {
		case  GEOSP_GD:
			{
				/* GD_Gd_Gc_convert (inCoords, outCoords); */
				Gd_Gc3d(geoSystem,inCoords,n,outCoords);

				/* just copy the coordinates for the GD temporary return  */
				memcpy (gdCoords, inCoords, sizeof (struct SFVec3d) * n);
				if(geoSystem->geoid_height || geoSystem->relativeHeight)
					for(i=0; i < n; i++)
						gdCoords[i].c[2] += userHeight2ellipsoidHeight(geoSystem,&gdCoords[i]);
			}
			break;
		case GEOSP_GC:
			/* an earth-fixed geocentric coord; no conversion required for gc value returns */
			for (i=0; i< n; i++) {
				veccopyd(outCoords[i].c,inCoords[i].c);
				/* convert this coord from GC to GD, using ellipsoid in geoSystem[1] */
				gccToGdc (geoSystem, &inCoords[i], &gdCoords[i]);
			}

			break;
		case GEOSP_UTM:
			{
				/* GD coords will be returned from the conversion process....*/
				/* first, convert UTM to GC, then GD, then GD to GC */
				/* see the compileGeosystem function for geoSystem fields */
				Utm_Gd3d(geoSystem,inCoords,n, gdCoords);
				if(geoSystem->geoid_height || geoSystem->relativeHeight)
					for(i=0; i < n; i++)
						gdCoords[i].c[2] += userHeight2ellipsoidHeight(geoSystem,&gdCoords[i]);
				Gd_Gc3d(geoSystem,gdCoords,n,outCoords);
			}
			break;
		case GEOSP_3TM:
			{
				/* GD coords will be returned from the conversion process....*/
				/* first, convert UTM to GC, then GD, then GD to GC */
				/* see the compileGeosystem function for geoSystem fields */
				U3tm_Gd3d(geoSystem,inCoords,n, gdCoords);
				if(geoSystem->geoid_height || geoSystem->relativeHeight)
					for(i=0; i < n; i++)
						gdCoords[i].c[2] += userHeight2ellipsoidHeight(geoSystem,&gdCoords[i]);
				Gd_Gc3d(geoSystem,gdCoords,n,outCoords); 
			}
			break;

		default :
			printf ("incorrect geoSystem field, %s\n",stringGEOSPATIALType(geoSystem->spatial_system));
			return;

	}
	if(offset || yup){
		if(offset)
		for(i=0;i<n;i++){
			//take offset off GC coords
			vecdifd(outCoords[i].c,outCoords[i].c,offset->c); 
		}
		if(yup){
			Quaternion qup;
			vrmlrot_to_quaternion(&qup,yup->c[0],yup->c[1],yup->c[2],-yup->c[3]);
			for(i=0;i<n;i++){
				//take offset off GC coords
				quaternion_rotationd(outCoords[i].c,&qup,outCoords[i].c);
			}
		}
	}
}


static void initializeGeospatial (struct X3D_GeoOrigin **nodeptr)  {
	//MF_SF_TEMPS
	int specversion;
	struct X3D_GeoOrigin *node = NULL;

	#ifdef VERBOSE
	printf ("\ninitializing GeoSpatial code nodeptr %u\n",*nodeptr); 
	#endif

	if (*nodeptr != NULL) {
		if (X3D_GEOORIGIN(*nodeptr)->_nodeType != NODE_GeoOrigin) {
			printf ("expected a GeoOrigin node, but got a node of type %s\n",
				stringNodeType(X3D_GEOORIGIN(*nodeptr)->_nodeType));
			*nodeptr = NULL;
			return;
		} else {
			/* printf ("um, just setting geoorign to %u\n",(*nodeptr)); */
			node = X3D_GEOORIGIN(*nodeptr);
		}
		specversion = X3D_PROTO(node->_executionContext)->__specversion;
		/* printf ("initGeoSpatial ich %d ch %d\n",node->_ichange, node->_change); */

		if NODE_NEEDS_COMPILING {
			//struct SFVec3d gdCoords;
			//struct SFVec3d offset;
			compile_geoSystem (X3D_NODE(node),node->_nodeType, &node->geoSystem, &node->__geoSystem);
			//INIT_MF_FROM_SF(node,geoCoords)
			moveCoords3d(GEOSYS(node->__geoSystem), NULL, NULL,
					&node->geoCoords,1, &node->__movedCoords, &node->__movedgd);
			//COPY_MF_TO_SF(node, __movedCoords)
			node->rotateYUp = FALSE; //Mar2018 rotateYup isn't working properly for us, get a wild angle
			//... H: we already do it with autoOrient H: we do it better with autoOrient H: we did it wrong all along
			if(node->rotateYUp == TRUE)
			{
				struct SFVec4d orient;
				int i;
				Quaternion qz,qx,qr;
				double dangle;
				Geosys *gs;
					 
				dangle = node->__movedgd.c[1];
				gs = GEOSYS(node->__geoSystem);
				//if(node->__geoSystem->.p[7] == TRUE)
				if(gs->gd_degrees)
					dangle *= RADIANS_PER_DEGREE;
				dangle += RADIANS_PER_DEGREE*90.0;
				vrmlrot_to_quaternion (&qz,0.0, 0.0, 1.0, dangle);

				#ifdef VERBOSE 
				printf ("GeoOrient qz angle (deg) %lf angle (rad) %lf quat: %lf %lf %lf %lf\n",dangle*DEGREES_PER_RADIAN, 
					dangle,qz.x, qz.y, qz.z,qz.w);
				#endif

				dangle =  node->__movedgd.c[0];
				if(gs->gd_degrees == TRUE)
					dangle *= RADIANS_PER_DEGREE;
				dangle = RADIANS_PER_DEGREE*180.0 - dangle;
				vrmlrot_to_quaternion (&qx,1.0, 0.0, 0.0,dangle);

				#ifdef VERBOSE 
				printf ("GeoOrient qx angle (deg) %lf angle (rad) %lf quat: %lf %lf %lf %lf\n",
					dangle*DEGREES_PER_RADIAN, dangle, qx.x, qx.y, qx.z,qx.w);
				#endif

				//quaternion_add (&qr, &qx, &qz);
				quaternion_multiply(&qr,&qz,&qx);

				#ifdef VERBOSE
				printf ("GeoOrient qr %lf %lf %lf %lf\n",qr.x, qr.y, qr.z,qr.w);
				#endif

				quaternion_to_vrmlrot(&qr, &orient.c[0], &orient.c[1], &orient.c[2], &orient.c[3]);
				for(i=0;i<4;i++)
					node->__rotyup.c[i] = orient.c[i];
			}else{
				int i;
				for(i=0;i<4;i++)
					node->__rotyup.c[i] = 0.0;
				node->__rotyup.c[1] = 1.0;
			}

			#ifdef VERBOSE
			printf ("initializeGeospatial, __movedCoords %lf %lf %lf, ryup %d, geoSystem %d %d %d %d\n",
				node->__movedCoords.c[0],
				node->__movedCoords.c[1],
				node->__movedCoords.c[2],
				node->rotateYUp,
				node->__geoSystem.p[0],
				node->__geoSystem.p[1],
				node->__geoSystem.p[2],
				node->__geoSystem.p[3]);
				node->__geoSystem.p[4]);
				node->__geoSystem.p[5]);
				node->__geoSystem.p[6]);
				node->__geoSystem.p[7]);
			printf ("initializeGeospatial, done\n\n");
			#endif

			//FREE_MF_SF_TEMPS
			MARK_NODE_COMPILED
		}
	}
}

/* calculate a translation that moves a Geo node to local space */
static void GeoMove(struct X3D_Node *node, struct X3D_GeoOrigin *geoOrigin, Geosys * geoSystem, struct Multi_Vec3d *inCoords, struct Multi_Vec3d *outCoords,
		struct Multi_Vec3d *gdCoords) {
	int i;
	struct X3D_GeoOrigin * myOrigin;
	Quaternion rq;

	#ifdef VERBOSE
	printf ("\nstart of GeoMove... %d coords\n",inCoords->n);
	#endif

	/* enough room for output? */
	if (inCoords->n==0) {return;}
	if (outCoords->n < inCoords->n) {
		if (outCoords->n!=0) {
			FREE_IF_NZ(outCoords->p);
		}
		outCoords->p = MALLOC(struct SFVec3d *, sizeof (struct SFVec3d) * inCoords->n);
		outCoords->n = inCoords->n;
	}
	if (gdCoords->n < inCoords->n) {
		if (gdCoords->n!=0) {
			FREE_IF_NZ(gdCoords->p);
		}
		gdCoords->p = MALLOC(struct SFVec3d *, sizeof (struct SFVec3d) * inCoords->n);
		gdCoords->n = inCoords->n;
	}


	/* set out values to 0.0 for now */
	for (i=0; i<outCoords->n; i++) {
		outCoords->p[i].c[0] = (double) 0.0; outCoords->p[i].c[1] = (double) 0.0; outCoords->p[i].c[2] = (double) 0.0;
	}

	#ifdef VERBOSE
	for (i=0; i<outCoords->n; i++) {
		printf ("start of GeoMove, inCoords %d: %lf %lf %lf\n",i, inCoords->p[i].c[0], inCoords->p[i].c[1], inCoords->p[i].c[2]);
	}
	#endif



	/* check the GeoOrigin attached node */
	myOrigin = NULL;
	if (geoOrigin != NULL) {
		if (X3D_GEOORIGIN(geoOrigin)->_nodeType != NODE_GeoOrigin) {
			ConsoleMessage ("GeoMove, expected a GeoOrigin, found a %s",stringNodeType(X3D_GEOORIGIN(geoOrigin)->_nodeType));
			printf ("GeoMove, expected a GeoOrigin, found a %s\n",stringNodeType(X3D_GEOORIGIN(geoOrigin)->_nodeType));
			return;
		}

		myOrigin = geoOrigin; /* local one */
	}
	/* printf ("GeoMove, using myOrigin %u, passed in geoOrigin %u with vals %lf %lf %lf\n",myOrigin, myOrigin,
		myOrigin->geoCoords.c[0], myOrigin->geoCoords.c[1], myOrigin->geoCoords.c[2] ); */ 
		
	//moveCoords(X3D_PROTO(node->_executionContext)->__specversion,geoSystem, inCoords, outCoords, gdCoords);
	//struct SFVec3d offset;
	//vecsetd(offset.c,0.0,0.0,0.0);
	//if(myOrigin)
	//	veccopyd(offset.c,myOrigin->__movedCoords.c); //is this right?
	moveCoords3d(geoSystem, NULL, NULL, 
		inCoords->p, inCoords->n, outCoords->p, gdCoords->p);

	for (i=0; i<outCoords->n; i++) {

	#ifdef VERBOSE
	printf ("GeoMove, before subtracting origin %lf %lf %lf\n", outCoords->p[i].c[0], outCoords->p[i].c[1], outCoords->p[i].c[2]);
	if (myOrigin != NULL) printf ("	... origin %lf %lf %lf\n",myOrigin->__movedCoords.c[0], myOrigin->__movedCoords.c[1], myOrigin->__movedCoords.c[2]);
	#endif

	if (myOrigin != NULL) {
		struct SFVec3d temp;

		outCoords->p[i].c[0] -= myOrigin->__movedCoords.c[0];
		outCoords->p[i].c[1] -= myOrigin->__movedCoords.c[1];
		outCoords->p[i].c[2] -= myOrigin->__movedCoords.c[2];
		if(myOrigin->rotateYUp == TRUE)
		{
			if(i==0)
			{
			vrmlrot_to_quaternion(&rq,myOrigin->__rotyup.c[0], myOrigin->__rotyup.c[1], myOrigin->__rotyup.c[2], -myOrigin->__rotyup.c[3]); 
			}
			//quaternion_multi_rotation(outxyz,&rq,inxyz,8);
			quaternion_rotation((struct point_XYZ *)temp.c, &rq, (const struct point_XYZ *)outCoords->p[i].c);
			outCoords->p[i].c[0] = temp.c[0];
			outCoords->p[i].c[1] = temp.c[1];
			outCoords->p[i].c[2] = temp.c[2];
		}
	}

	#ifdef VERBOSE
	printf ("GeoMove, after subtracting origin %lf %lf %lf\n", outCoords->p[i].c[0], outCoords->p[i].c[1], outCoords->p[i].c[2]);
	#endif
	}
}

/* for converting from GC to GD */
struct gcgd {
double A, F, C, A2, C2, Eps2, Eps21, Eps25, C254, C2DA, CEE,
                 CE2, CEEps2, TwoCEE, tem, ARat1, ARat2, BRat1, BRat2, B1,B2,B3,B4,B5;
};

/* for converting BACK to GD from GC */
struct gcgd* initializeGcToGdParams(int type, double A, double F) {
	struct gcgd *g;
	ppComponent_Geospatial p = (ppComponent_Geospatial)gglobal()->Component_Geospatial.prv;
	if(type < 0) type = -type + GEOELLIPSOID_COUNT;
	if(p->gcgdpars[type]) return p->gcgdpars[type];
	g = malloc(sizeof(struct gcgd));
	p->gcgdpars[type] = g;
    /*  Create the ERM constants. */
	g->A = A;
    g->A2     = A * A;
    g->F      =F;
    g->C      =(A) * (1-g->F);
    g->C2     = g->C * g->C;
    g->Eps2   =(g->F) * (2.0-g->F);
    g->Eps21  =g->Eps2 - 1.0;
    g->Eps25  =.25 * (g->Eps2);
    g->C254   =54.0 * g->C2;        
        
    g->C2DA   = g->C2 / A;
    g->CE2    = g->A2 - g->C2;
    g->tem    = g->CE2 / g->C2;
    g->CEE    = g->Eps2 * g->Eps2;        
    g->TwoCEE =2.0 * g->CEE;
    g->CEEps2 =g->Eps2 * g->CE2;
         
    /* UPPER BOUNDS ON POINT */
     

    g->ARat1  =pow((A + 50005.0),2);
    g->ARat2  =(g->ARat1) / pow((g->C+50005.0),2);
    
    /* LOWER BOUNDS ON POINT */
        
    g->BRat1  =pow((A-10005.0),2);
    g->BRat2  =(g->BRat1) / pow((g->C-10005.0),2);
          
	/* use WE ellipsoid */
	g->B1=0.100225438677758E+01;
	g->B2=-0.393246903633930E-04;
	g->B3=0.241216653453483E+12;
	g->B4=0.133733602228679E+14;
	g->B5=0.984537701867943E+00;
	return g;
}

/* convert BACK to a GD coordinate, from GC coordinates using ellipsoid */
static void gccToGdc_fw (Geosys *geoSystem, struct SFVec3d *gcc, struct SFVec3d *gdc) {
	int latitude = 0;
	int longitude = 1;
	int elevation = 2;
	double GCC_X, GCC_Y, GCC_Z;
	double A,F;
	double w2,w,z2,testu,testb,top,top2,rr,q,s12,rnn,s1,zp2,wp,wp2,cf,gee,alpha,cl,arg2,p,xarg,r2,r1,ro,
		s,roe,arg,v,zo;
	struct gcgd *g;
	#ifdef VERBOSE
	printf ("gccToGdc input %lf %lf %lf\n",GCC_X, GCC_Y, GCC_Z);
	#endif
	GCC_X = gcc->c[0];
	GCC_Y = gcc->c[1];
	GCC_Z = gcc->c[2];
	if(geoSystem->gd_latitude_first == FALSE){
		latitude = 1; longitude = 0;
	}
	getEllipsoidParams(geoSystem->ellipsoid,&A,&F);
	g = initializeGcToGdParams(geoSystem->ellipsoid,A,F);

        w2=GCC_X * GCC_X + GCC_Y * GCC_Y;
        w=sqrt(w2);
        z2=GCC_Z * GCC_Z;

        testu=w2 + g->ARat2 * z2;
        testb=w2 + g->BRat2 * z2;

        if ((testb > g->BRat1) && (testu < g->ARat1)) 
        {    

            /*POINT IS BETWEEN-10 KIL AND 50 KIL, SO COMPUTE TANGENT LATITUDE */
    
            top= GCC_Z * (g->B1 + (g->B2 * w2 + g->B3) /
                 (g->B4 + w2 * g->B5 + z2));

            top2=top*top;

            rr=top2+w2;
                  
            q=sqrt(rr);
                  
            /* ****************************************************************
                  
               COMPUTE H IN LINE SQUARE ROOT OF 1-EPS2*SIN*SIN.  USE SHORT BINOMIAL
               EXPANSION PLUS ONE ITERATION OF NEWTON'S METHOD FOR SQUARE ROOTS.
            */

            s12=top2/rr;

            rnn = g->A / ( (.25 - g->Eps25*s12 + .9999944354799/4) + (.25-g->Eps25*s12)/(.25 - g->Eps25*s12 + .9999944354799/4));
            s1=top/q;
        
            /******************************************************************/

            /* TEST FOR H NEAR POLE.  if SIN(¯)**2 <= SIN(45.)**2 THEN NOT NEAR A POLE.*/  
    
            if (s12 < .50)
                gdc->c[elevation] = q-rnn;
            else
                gdc->c[elevation] = GCC_Z / s1 + (g->Eps21 * rnn);
                gdc->c[latitude] = atan(top / w);
                gdc->c[longitude] = atan2(GCC_Y,GCC_X);
        }
              /* POINT ABOVE 50 KILOMETERS OR BELOW -10 KILOMETERS  */
        else /* Do Exact Solution  ************ */
        { 
            wp2=GCC_X * GCC_X + GCC_Y * GCC_Y;
            zp2=GCC_Z * GCC_Z;
            wp=sqrt(wp2);
            cf=g->C254 * zp2;
            gee=wp2 - (g->Eps21 * zp2) - g->CEEps2;
            alpha=cf / (gee*gee);
            cl=g->CEE * wp2 * alpha / gee;
            arg2=cl * (cl + 2.0);
            s1=1.0 + cl + sqrt(arg2);
            s=pow(s1,(1.0/3.0));
            p=alpha / (3.0 * pow(( s + (1.0/s) + 1.0),2));
            xarg= 1.0 + (g->TwoCEE * p);
            q=sqrt(xarg);
            r2= -p * (2.0 * (1.0 - g->Eps2) * zp2 / ( q * ( 1.0 + q) ) + wp2);
            r1=(1.0 + (1.0 / q));
            r2 /=g->A2;

            /*    DUE TO PRECISION ERRORS THE ARGUMENT MAY BECOME NEGATIVE IF SO SET THE ARGUMENT TO ZERO.*/

            if (r1+r2 > 0.0)
                ro = g->A * sqrt( .50 * (r1+r2));
            else
                ro=0.0;

            ro=ro - p * g->Eps2 * wp / ( 1.0 + q);
            //arg0 = pow(( wp - Eps2 * ro),2) + zp2;
            roe = g->Eps2 * ro;
            arg = pow(( wp - roe),2) + zp2;
            v=sqrt(arg - g->Eps2 * zp2);
            zo=g->C2DA * GCC_Z / v;
            gdc->c[elevation] = sqrt(arg) * (1.0 - g->C2DA / v);
            top=GCC_Z+ g->tem*zo;
            gdc->c[latitude] = atan( top / wp );
            gdc->c[longitude] =atan2(GCC_Y,GCC_X);
        }  /* end of Exact solution */

		if(geoSystem->gd_degrees == TRUE){
			//v3.2- works in degrees by default, v3.3+ works in 'angle base units' (radians) by default
			gdc->c[latitude] *= DEGREES_PER_RADIAN;
			gdc->c[longitude] *= DEGREES_PER_RADIAN;
		}
#undef VERBOSE

}
#ifdef GEOLIB
static void gccToGdc_geolib (Geosys *geoSystem, struct SFVec3d *gcc, struct SFVec3d *gdc){
	int geotype;
	double gd[3],gc[3], semimajor,flattening;
	//printf("hi from gccToGdc_geolib\n");
	getEllipsoidParams(geoSystem->ellipsoid,&semimajor,&flattening);
	if(FALSE && flattening == 0.0){
		//easy spherical coords, although geolib doesn't need help, just for testing here
		double radius, horizontal_radius;
		veccopyd(gc,gcc->c);
		//printf("gc2gd gc %lf %lf %lf\n",gc[0],gc[1],gc[2]);
		radius = veclengthd(gc);
		horizontal_radius = veclength2d(gc);
		gd[0] = atan2(gc[2],horizontal_radius);
		gd[1] = atan2(gc[1],gc[0]);
		gd[2] = radius - semimajor;
		//printf("radius %lf semimajor %lf\n",radius,semimajor);
		if(!geoSystem->gd_latitude_first) vecswizzle2d(gd);
		if(geoSystem->gd_degrees) vecscale2d(gd,gd,DEGREES_PER_RADIAN);
		//printf("gc2gd sphere gd %lf %lf %lf\n",gd[0],gd[1],gd[2]);
		veccopyd(gdc->c,gd);
	}
	else
	{
		geotype = geoSystem->ellipsoid;
		if(geotype < 0) geotype = -geotype + GEOELLIPSOID_COUNT;
		if(!fwgeo_gc[geotype]){
			fwgeo_gc[geotype] = fgeo_initializeGC(semimajor,flattening);
		}
		veccopyd(gc,gcc->c);
		// function(semimajor,flattening,gc[0],gc[1],gc[2],&gd[0],&gd[1],&gd[2]);
		fgeo_gc2gd(fwgeo_gc[geotype],gc[0],gc[1],gc[2], &gd[0],&gd[1],&gd[2]);
		if(!geoSystem->gd_latitude_first) vecswizzle2d(gd);
		if(!geoSystem->gd_degrees) vecscale2d(gd,gd,RADIANS_PER_DEGREE);
		veccopyd(gdc->c,gd);
		//printf("gc2gd geolb gd %lf %lf %lf\n",gd[0],gd[1],gd[2]);
	}

}
#endif //GEOLIB
static void gccToGdc (Geosys *geoSystem, struct SFVec3d *gcc, struct SFVec3d *gdc){
#ifdef GEOLIB
	if(method_geolib()){
		gccToGdc_geolib(geoSystem,gcc,gdc);
		//vecprint3db("gl gdc ",gdc->c,"\n");
	}else
#endif //GEOLIB
	{
		double semimajor, flattening;
		getEllipsoidParams(geoSystem->ellipsoid,&semimajor,&flattening);
		if(flattening == 0.0){
			//easy spherical coords
			//gccToGdc_fw and/or its gd2gc complement has a problem with moon geoSystem 'R173...' 'F0.0'
			double radius, horizontal_radius, gd[3], gc[3];
			veccopyd(gc,gcc->c);
			//printf("gc2gd gc %lf %lf %lf\n",gc[0],gc[1],gc[2]);
			radius = veclengthd(gc);
			horizontal_radius = veclength2d(gc);
			gd[0] = atan2(gc[2],horizontal_radius);
			gd[1] = atan2(gc[1],gc[0]);
			gd[2] = radius - semimajor;
			//printf("radius %lf semimajor %lf\n",radius,semimajor);
			if(!geoSystem->gd_latitude_first) vecswizzle2d(gd);
			if(geoSystem->gd_degrees) vecscale2d(gd,gd,DEGREES_PER_RADIAN);
			//printf("gc2gd sphere gd %lf %lf %lf\n",gd[0],gd[1],gd[2]);
			veccopyd(gdc->c,gd);
		}
		else
		{
			gccToGdc_fw(geoSystem,gcc,gdc);
			//vecprint3db("fw gdc ",gdc->c,"\n");
		}
	}
}
/* convert a GDC BACK to a UTM coordinate ASSUMES LAT LON RADIANS*/
static void gdToXtm(double radius, double flattening, double latitude, double longitude, double scaleFactor, 
	double falseEasting, double falseNorthing, double zoneSize, int *zone, double *easting, double *northing) 
{
#define DEG2RAD (PI/180.00)
//#define GEOSP_WE_INV 0.00669438
	double lat_radian;
	double long_radian;
	double myScale;
	double longOrigin;
	double longOriginradian, dlon;
	double eccentprime, e2, A, F;
	double NNN;
	double TTT;
	double CCC;
	double AAA;
	double MMM;

	A = radius;
	F = flattening;
	//e2 = 2.0*F - F*F;
	e2 = F*(2. - F);

	/* calculate the zone number if it is less than zero. If greater than zero, leave alone! */
	dlon = longitude * DEGREES_PER_RADIAN;
	if (*zone < 0) 
		*zone = (int) (((dlon + 180.0)/zoneSize) + 1);

	lat_radian = latitude;
	long_radian = longitude;
	myScale = scaleFactor; //0.9996;
	longOrigin = (*zone - 1)*zoneSize - 180.0 + zoneSize/2.0; //3;
	longOriginradian = longOrigin * DEG2RAD;
	eccentprime = e2/(1.0-e2);

	/* 
	printf ("lat_radian %lf long_radian %lf myScale %lf longOrigin %d longOriginradian %lf eccentprime %lf\n",
	   lat_radian, long_radian, myScale, longOrigin, longOriginradian, eccentprime);
	*/
	//http://www.engr.usask.ca/classes/CE/316/notes/CE%20316%20CH%204C%2031-1-12%20-INSTRUCTOR.pdf

	NNN = A / sqrt(1.0-e2 * sin(lat_radian)*sin(lat_radian));
	TTT = tan(lat_radian) * tan(lat_radian);
	CCC = eccentprime * cos(lat_radian)*cos(lat_radian);
	AAA = cos(lat_radian) * (long_radian - longOriginradian);
	MMM = A
            * ( ( 1. - e2/4 - 3. * e2 * e2/64
                  - 5.0 * e2 * e2 * e2/256.0
                ) * lat_radian
              - ( 3. * e2/8 + 3. * e2 * e2/32.
                  + 45. * e2 * e2 * e2/1024.
                ) * sin(2. * lat_radian)
              + ( 15. * e2 * e2/256. +
                  45. * e2 * e2 * e2/1024.
                ) * sin(4. * lat_radian)
              - ( 35. * e2 * e2 * e2/3072.
                ) * sin(6. * lat_radian)
              );

	/* printf ("N %lf T %lf C %lf A %lf M %lf\n",NNN,TTT,CCC,AAA,MMM); */

	*easting = myScale*NNN*(AAA+(1-TTT+CCC)*AAA*AAA*AAA/6
                    + (5.-18.*TTT+TTT*TTT+72.*CCC-58.*eccentprime)*AAA*AAA*AAA*AAA*AAA/120.)
                    + falseEasting; //500000.0;

	*northing= myScale * ( MMM + NNN*tan(lat_radian) * 
		( AAA*AAA/2.+(5.-TTT+9.*CCC+4.*CCC*CCC)*AAA*AAA*AAA*AAA/24 + (61.-58.*TTT+TTT*TTT+600.*CCC-330.*eccentprime) * AAA*AAA*AAA*AAA*AAA*AAA/720.));

	if (latitude < 0) *northing += falseNorthing; //10000000.0;*/

	#ifdef VERBOSE
	printf ("gdToUtm: lat %lf long %lf zone %d -> easting %lf northing %lf\n",latitude, longitude, *zone,*easting, *northing);
	#endif
}

#ifdef GEOLIB
//assumes LAT, LON in radians
static void gdToXtm_geolib(int geotype, double radius, double flattening, double latitude, double longitude, double scaleFactor, 
	double falseEasting, double falseNorthing, double zoneSize, int *zone, double *easting, double *northing) 
{
	double F, dlon0, dlat, dlon;
	void *fgeo;
	ppComponent_Geospatial p = (ppComponent_Geospatial)gglobal()->Component_Geospatial.prv;

	F = flattening;
	if(!p->fgeopars[geotype])
		p->fgeopars[geotype] = fgeo_initializeTM(radius, F, 1.0);
	fgeo = p->fgeopars[geotype];

	/* calculate the zone number if it is less than zero. If greater than zero, leave alone! */
	//Q. is longitude in degrees, or does that depend on UNITS, specversion and strict33?
	dlon = longitude * DEGREES_PER_RADIAN;
	if (*zone < 0) 
		*zone = (int) (((dlon + 180.0)/zoneSize) + 1);

	dlon0 = (*zone -1) * zoneSize - 180. + zoneSize*.5;
	dlat = latitude * DEGREES_PER_RADIAN;
	
	fgeo_gd2tm(fgeo,dlat,dlon,dlon0,easting, northing);
	*easting *= scaleFactor;
	*northing *= scaleFactor;

	if (latitude < 0.0) *northing += falseNorthing; //10000000.0;
	*easting += falseEasting;

	#ifdef VERBOSE
	printf ("gdToUtm: lat %lf long %lf zone %d -> easting %lf northing %lf\n",latitude, longitude, *zone,*easting, *northing);
	#endif
}
#endif //GEOLIB

/* compileGeosystem - encode the return value such that srf->p[x] is... 
	0:	spatial reference frame (GEOSP_UTM, GEOSP_GC, GEOSP_GD); 
	1:	ellipsoid index (defaults to GEOSP_WE) 
	2:	UTM zone number, 1..60. INT_ID_UNDEFINED = not specified 
	3:	UTM:    if "northing_first" TRUE, if "easting_first", FALSE 
	4:	UTM:    if "S" - value is FALSE, not S, value is TRUE
	5:	GD:     if "latitude_first" TRUE, if "longitude_first", FALSE 
	6:	GD: true if geoid height
	7:	GD: TRUE: decimal degrees, FALSE radians
*/

static void gdToUtm3d(Geosys *geoSystem, double *gdcoords, double *xtmcoords) {
	double semimajor, flattening;
	double gdradians[3];
	int geotype, northing_first, latitude_first, is_degrees, *zone;
	
	geotype = geoSystem->ellipsoid; //ellipsoid index
	northing_first = geoSystem->xtm_northing_first;
	latitude_first = geoSystem->gd_latitude_first;
	is_degrees = geoSystem->gd_degrees;
	
	if(is_degrees) vecscaled(gdradians,gdcoords,RADIANS_PER_DEGREE);
	else veccopyd(gdradians,gdcoords);
	if(!latitude_first) vecswizzle2d(gdradians); //unswizzle if swizzled
	
	getEllipsoidParams(geotype,&semimajor,&flattening);
	zone = &geoSystem->xtm_zone;
#ifdef GEOLIB
	if(method_geolib())
		gdToXtm_geolib(geotype,semimajor,flattening,gdradians[0],gdradians[1], UTM_SCALE, UTM_FALSE_EASTING, UTM_FALSE_NORTHING, UTM_ZONE_SIZE, zone, &xtmcoords[1], &xtmcoords[0]);
	else
#endif //GEOLIB
		gdToXtm(semimajor,flattening, gdradians[0],gdradians[1], UTM_SCALE, UTM_FALSE_EASTING, UTM_FALSE_NORTHING, UTM_ZONE_SIZE, zone, &xtmcoords[1], &xtmcoords[0]);
	
	if(!northing_first) vecswizzle2d(xtmcoords);
	xtmcoords[2] = gdcoords[2];
}
static void gdTo3tm3d(Geosys *geoSystem, double *gdcoords, double *xtmcoords) {
	double semimajor, flattening;
	double gdradians[3];
	int geotype, northing_first, latitude_first, is_degrees, *zone;
	
	geotype = geoSystem->ellipsoid; //ellipsoid index
	northing_first = geoSystem->xtm_northing_first;
	latitude_first = geoSystem->gd_latitude_first;
	is_degrees = geoSystem->gd_degrees;
	
	if(is_degrees) vecscaled(gdradians,gdcoords,RADIANS_PER_DEGREE);
	else veccopyd(gdradians,gdcoords);
	if(!latitude_first) vecswizzle2d(gdradians);
	
	getEllipsoidParams(geotype,&semimajor,&flattening);
	zone = &geoSystem->xtm_zone;
#ifdef GEOLIB
	if(method_geolib())
		gdToXtm_geolib(geotype,semimajor,flattening,gdradians[0],gdradians[1], U3TM_SCALE, U3TM_FALSE_EASTING, U3TM_FALSE_NORTHING, U3TM_ZONE_SIZE, zone, &xtmcoords[1], &xtmcoords[0]);
	else
#endif //GEOLIB
		gdToXtm(semimajor, flattening, gdradians[0],gdradians[1], U3TM_SCALE, U3TM_FALSE_EASTING, U3TM_FALSE_NORTHING, U3TM_ZONE_SIZE, zone,  &xtmcoords[1], &xtmcoords[0]);
	
	if(!northing_first) vecswizzle2d(xtmcoords);
	xtmcoords[2] = gdcoords[2];
}

/* calculate the rotation needed to apply to this position on the GC coordinate location */
static void GeoOrient (struct X3D_Node *geoOrigin, Geosys *geoSystem, struct SFVec3d *gdCoords, struct SFVec4d *orient) {
	Quaternion qx;
	Quaternion qz;
	Quaternion qr;
	double dangle, gdcoords[3];

	orient->c[0] = 0.0; 
	orient->c[1] = 1.0; 
	orient->c[2] = 0.0; 
	orient->c[3] = 0.0; 
	/* is this a straight GC geoSystem? If so, we do not do any orientation */
	if (geoSystem != NULL) {
		if (geoSystem->spatial_system == GEOSP_GC) {
			#ifdef VERBOSE
			printf ("GeoOrient - simple GC, so no orient\n");
			#endif
			return;
		}
	}
	if(geoOrigin)
	{
		if(((struct X3D_GeoOrigin*)geoOrigin)->rotateYUp == TRUE) return;
	}

	#ifdef VERBOSE
	printf ("GeoOrient - gdCoords->c[0,1] is %f %f\n",gdCoords->c[0],gdCoords->c[1]);
	#endif

	/* initialize qx and qz */
	veccopyd(gdcoords,gdCoords->c);
	if(!geoSystem->gd_latitude_first) vecswizzle2d(gdcoords);
	dangle = gdcoords[1]; //longitude
	if(geoSystem->gd_degrees == TRUE)
		dangle *= RADIANS_PER_DEGREE;
	dangle += RADIANS_PER_DEGREE*90.0;
	vrmlrot_to_quaternion (&qz,0.0, 0.0, 1.0, dangle);

	#ifdef VERBOSE 
	printf ("GeoOrient qz angle (deg) %lf angle (rad) %lf quat: %lf %lf %lf %lf\n",((double)90.0 + gdCoords->c[1]), 
		RADIANS_PER_DEGREE*((double)90.0 + gdCoords->c[1]),qz.x, qz.y, qz.z,qz.w);
	#endif

	dangle = gdcoords[0]; //latitude
	if(geoSystem->gd_degrees == TRUE)
		dangle *= RADIANS_PER_DEGREE;
	dangle = RADIANS_PER_DEGREE*180.0 - dangle;
	vrmlrot_to_quaternion (&qx,1.0, 0.0, 0.0, dangle);

	#ifdef VERBOSE 
	printf ("GeoOrient qx angle (deg) %lf angle (rad) %lf quat: %lf %lf %lf %lf\n",
		((double)180.0 - gdCoords->c[0]), RADIANS_PER_DEGREE*((double)180.0 - gdCoords->c[0]), qx.x, qx.y, qx.z,qx.w);
	#endif

	//quaternion_add (&qr, &qx, &qz);
	//quaternion_print(&qr,"added\n");
	quaternion_multiply(&qr, &qz, &qx);
	//quaternion_print(&qr,"multiplied\n");

	#ifdef VERBOSE
	printf ("GeoOrient qr %lf %lf %lf %lf\n",qr.x, qr.y, qr.z,qr.w);
	#endif

        quaternion_to_vrmlrot(&qr, &orient->c[0], &orient->c[1], &orient->c[2], &orient->c[3]);

	#ifdef VERBOSE
	printf ("GeoOrient rotation %lf %lf %lf %lf\n",orient->c[0], orient->c[1], orient->c[2], orient->c[3]);
	#endif
}

/* compileGeosystem - encode the return value such that srf->p[x] is...
	0:	spatial reference frame (GEOSP_UTM, GEOSP_GC, GEOSP_GD); 
	1:	ellipsoid index (defaults to GEOSP_WE) 
	2:	UTM zone number, 1..60. INT_ID_UNDEFINED = not specified 
	3:	UTM:    if "northing_first" TRUE, if "easting_first", FALSE 
	4:	UTM:    if "S" - value is FALSE, not S, value is TRUE
	5:	GD:     if "latitude_first" TRUE, if "longitude_first", FALSE 
	6:	GD: true if geoid height
	7:	GD: TRUE: decimal degrees, FALSE radians
*/
struct stringint{
	char *c;
	int i;
};
char * stringint_int2string(struct stringint *table, int itype){
	int i = 0;
	while(table[i].c){
		if(table[i].i == itype) return table[i].c;
		i++;
	}
	return NULL;
}
int stringint_string2int(struct stringint *table, const char *ctype){
	int i = 0;
	while(table[i].c){
		if(!strcmp(table[i].c,ctype)) return table[i].i;
		i++;
	}
	return -1;
}
struct stringint lookup_ellipsoids [] = {
	{"AA",GEOEL_AA},
	{"AM",GEOEL_AM},
	{"AN",GEOEL_AN},
	{"BN",GEOEL_BN},
	{"BR",GEOEL_BR},
	{"CC",GEOEL_CC},
	{"CD",GEOEL_CD},
	{"EA",GEOEL_EA},
	{"EB",GEOEL_EB},
	{"EC",GEOEL_EC},
	{"ED",GEOEL_ED},
	{"EE",GEOEL_EE},
	{"EF",GEOEL_EF},
	{"FA",GEOEL_FA},
	{"HE",GEOEL_HE},
	{"HO",GEOEL_HO},
	{"ID",GEOEL_ID},
	{"IN",GEOEL_IN},
	{"KA",GEOEL_KA},
	{"RF",GEOEL_RF},
	{"SA",GEOEL_SA},
	{"WD",GEOEL_WD},
	{"WE",GEOEL_WE},
	{NULL,-1},
};
struct stringint lookup_spatialreferencesys [] = {
	{"GC",GEOSP_GC},
	{"GD",GEOSP_GD},
	{"UTM",GEOSP_UTM},
	{"3TM",GEOSP_3TM},
	{NULL,-1},
};


static void compile_geoSystem (struct X3D_Node *node, int nodeType, struct Multi_String *args, struct X3D_Node **nodegeosys) {
	int i, specversion, nextra;
	indexT this_srf = INT_ID_UNDEFINED;
	indexT this_srf_ind = INT_ID_UNDEFINED;
	struct ellipsoid ee;
	Geosys *srf = GEOSYS(*nodegeosys);

	#ifdef VERBOSE
	printf ("start of compile_geoSystem\n");
	#endif

	/* malloc the area required for internal settings, if required */
	if (srf==NULL) {
		srf = malloc(sizeof(Geosys));
		register_node_gc(node,(void*)srf);
		*nodegeosys = X3D_NODE(srf);
	}

	/* set these as defaults */
	srf->spatial_system = GEOSP_GD; 
	srf->ellipsoid = GEOEL_WE;
	srf->xtm_zone = INT_ID_UNDEFINED;
	srf->xtm_northing_first = TRUE; //XTM: northing first
	srf->utm_northern_hemisphere = TRUE; //northern hemisphere for UTM
	srf->gd_latitude_first = TRUE; //GD: lat first
	srf->geoid_height = FALSE; //geoid - not GC, just GD/UTM
	specversion = X3D_PROTO(node->_executionContext)->__specversion;
	if(specversion > 320 && STRICT33){
		//version 3.3+ by default in 'angle base units' which are radians
		srf->gd_degrees = FALSE; //GD: TRUE decimal degrees, FALSE: radians
	}else{
		//version 3.2- by default in degrees
		srf->gd_degrees = TRUE; //GD: TRUE decimal degrees, FALSE: radians
	}
	srf->relativeHeight = FALSE; //relative height flag, not set below, its set during specific node compile
	/* if nothing specified, we just use these defaults */
	if (args->n==0) return;

	//2018 we allow the user to specify ellipsoid (A and (B or IF (inverse flattening) or F (flattening)) or R radius
	nextra = FALSE;
	ee.a = ee.b = ee.f = 0.0;

	/* first go through, and find the Spatial Reference Frame, GD, UTM, or GC */
	for (i=0; i<args->n; i++) {
		int itype = stringint_string2int(lookup_spatialreferencesys,args->p[i]->strptr);
		if(itype > -1){
			this_srf = itype;
			this_srf_ind = i;
		}
	}

	/* did we find a GC, GD, or UTM? */
	if (this_srf == INT_ID_UNDEFINED) {
		ConsoleMessage ("geoSystem in node %s,  must have GC, GD or UTM",stringNodeType(nodeType));
		return;
	}

	srf->spatial_system = (int) this_srf;
	/* go through and ensure that we have the correct parameters for this spatial reference frame */
	if (this_srf == GEOSP_GC) {
		//srf->p[1] = INT_ID_UNDEFINED;
		//nothing to do 
	} else if (this_srf == GEOSP_GD || this_srf == GEOSP_3TM || this_srf == GEOSP_UTM) {
		for (i=0; i<args->n; i++) {
			if (i != this_srf_ind) {
				int iellipse;
				char *str = args->p[i]->strptr;
				/* printf ("geosp_gd, ind %d i am %d string %s\n",i, this_srf_ind,args->p[i]->strptr); */
				iellipse = stringint_string2int(lookup_ellipsoids,str);
				if(iellipse > -1){
					srf->ellipsoid = iellipse;
				}else{
					//GD specifics
					if (strcmp("latitude_first", str) == 0) {
						srf->gd_latitude_first = TRUE;
					} else if (strcmp("longitude_first", str) == 0) {
						srf->gd_latitude_first = FALSE;
					} else if(strcmp ("WGS84",str) == 0){
						srf->geoid_height = TRUE; //geoid
					} else 
					//ellipsoid parameters specified
					if(str[0] == 'R') {
						//radius
						double radius;
						sscanf(args->p[i]->strptr,"R%lf",&radius);
						nextra = TRUE;
						ee.a = radius;
					} else if (str[0] == 'A') {
						//radius
						double a;
						sscanf(str,"A%lf",&a);
						nextra = TRUE;
						ee.a = a;
					} else if (str[0] == 'B') {
						//radius
						double b;
						sscanf(str,"B%lf",&b);
						nextra = TRUE;
						ee.b = b;
					} else if (!strncmp(str,"IF",2)) {
						//radius
						double invf;
						sscanf(str,"IF%lf",&invf);
						nextra = TRUE;
						ee.f = 1.0/invf;
					} else if (str[0] == 'F') {
						//radius
						double f;
						sscanf(str,"F%lf",&f);
						nextra = TRUE;
						ee.f = f;
					} else 
					//XTM
					if (strcmp ("S",str) == 0) {
						srf->utm_northern_hemisphere = FALSE;
					} else if (strcmp ("N",str) == 0) {
						srf->utm_northern_hemisphere = TRUE; // default
					} else if (str[0] == 'Z') {
						int zone = -1;
						sscanf(str,"Z%d",&zone);
						/* printf ("zone found as %d\n",zone); */
						srf->xtm_zone = zone;
					} else if (strcmp("northing_first",str) == 0) { 
						srf->xtm_northing_first = TRUE;
					} else if (strcmp("easting_first",str) == 0) { 
						srf->xtm_northing_first = FALSE;
					} else 
					//UNHANDLED
					{
						ConsoleMessage("geoSystem parameter %s not handled, in node %s",str,stringNodeType(nodeType));
					}
				}
			}
		}
	} 

	if(nextra){
		int ifound;
		if(ee.f == 0.0){
			//compute ellipsoid inverse flattening if not given
			double a,b,invf;
			a = extra_ellipsoid[nextra_ellipsoid].a;
			b = extra_ellipsoid[nextra_ellipsoid].b;
			ee.f = 1.0;
			if(ee.a != 0.0 && ee.b != 0.0){
				ee.f = (ee.a-ee.b)/ee.a;
			}else if(ee.a != 0.0){
				//likely radius, in which case (a-b) == 0
				ee.f  = 0.0;
			}

		}
		//see if we already have this ellipsoid, if not add it, else use it
		// (in case we have hundreds of nodes with the same user-defined ellipsoid)
		ifound = nextra_ellipsoid;
		for(i=1;i<nextra_ellipsoid;i++){
			if(extra_ellipsoid[i].a == ee.a && extra_ellipsoid[i].f == ee.f){
				ifound = i;
				break;
			}
		}
		extra_ellipsoid[ifound] = ee;
		srf->ellipsoid = -ifound; //negative sentinal value for extra ellipsoids
		if(ifound == nextra_ellipsoid)
			nextra_ellipsoid++;
	}

	#ifdef VERBOSE
	printf ("printf done compileGeoSystem\n");
	#endif

}
//ever get tired of those long parameter lists?
//how about wrapping up the call parameters in a struct
// and passing (a pointer to) the struct?
//especially to 'demacroize' while keeping generallized across related nodes
//we don't have the concept of an 'interface' -cluster of related fields-
//and in general we can't rely on fields being in a consistent order or offset from node start.

//TRANSFORMING FROM GEOSPATIAL  TO SHARED LOCAL aka LCS LOCAL COORDINATE SYSTEM
// terminology:
// geocentric GC - center of molten core of eath is 0,0,0
// geospatial aligned GCA - X through Grenwich, Z through north pole
// node local (NL): relative to node's own 'origin' ie position, geoGridOrigin, etc
// node local aligned (NLA): Y 'up', -Z toward north pole
// shared local (SL): relative to a single shared origin for all geo nodes for a planet
// shared local aligned (SLA): relative to 'up' and 'north' at the shared origin
// root node, root node aligned RNRNA - the regular scene 0,0,0 at the root level, and alignemnt
// SLSLA aka LCS could be designed to be co-incident with and aligned with RNRNA
// procedure:
// A. convert to GC
//  1. convert node 'origin' from XTM -> GD -> GC
//  2. convert any geometry in the node from XTM -> GD -> GC
// B. convert to NLNLA
//  3. subtract node origin GC from geometry GC to get node-local geocentric-aligned NLGCA
//  4. compute LocalOrientation - the rotation to apply to NLGCA to get NLNLA node local aligned = LocalOrient
// C. convert to SLSLA
//  5. compute tilt to get node geometry from NLNLA to NLSLA shared local aligned = offsetOrient
//  6. compute offset to get NLSLA to shared local SLSLA 
// summary order of transforms:
//  GC2NL
//  GCA2NLA - H: this depends how the node is defined.
//  NLA2SLA
//  NL2SL

void user2gd(Geosys * geoSystem, struct SFVec3d *geo, int n, struct SFVec3d *gd);
void gd2user(Geosys * geoSystem, struct SFVec3d *gd,  int n, struct SFVec3d *geo);
void user2gc(Geosys * geoSystem, struct SFVec3d *geo, int n, struct SFVec3d *gc);
void gc2user(Geosys * geoSystem, struct SFVec3d *gc,  int n, struct SFVec3d *geo);
void  gc2lcs(Geosys * geoSystem, struct SFVec3d *gc,  int n, struct SFVec3d *lcs);
void  lcs2gc(Geosys * geoSystem, struct SFVec3d *lcs, int n, struct SFVec3d *gc);
void   gd2gc(Geosys * geoSystem, struct SFVec3d *gd,  int n, struct SFVec3d *gc);
void   gc2gd(Geosys * geoSystem, struct SFVec3d *gc,  int n, struct SFVec3d *gd);

void gc2lcs(Geosys * geoSystem, struct SFVec3d *gc, int n, struct SFVec3d *lcs){
	//UNTESTED
	//converts from GC geocentric, to LCS local coordinate system
	//LCS = GC - origin
	int i;
	struct Planet *planet;
	planet = current_planet();
	for(i=0;i<n;i++){
		//take offset off GC coords
		vecdifd(lcs[i].c,gc[i].c,planet->autoOrigin.c); 
	}
	if(1){
		Quaternion qup;
		double aoo[4];
		veccopy4d(aoo,planet->autoOrient.c);
		vrmlrot_to_quaternion(&qup,aoo[0],aoo[1],aoo[2],-aoo[3]);
		for(i=0;i<n;i++){
			quaternion_rotationd(lcs[i].c,&qup,lcs[i].c);
		}
	}
}
void lcs2gc(Geosys * geoSystem, struct SFVec3d *lcs, int n, struct SFVec3d *gc){
	//UNTESTED
	//converts from local coorinate system to GC geocentric
	//GC = LCS + origin
	int i;
	struct Planet *planet;
	planet = current_planet();
	{
		Quaternion qup;
		double aoo[4];
		veccopy4d(aoo,planet->autoOrient.c);
		vrmlrot_to_quaternion(&qup,aoo[0],aoo[1],aoo[2],aoo[3]);
		for(i=0;i<n;i++){
			if(1) quaternion_rotationd(gc[i].c,&qup,lcs[i].c);
			else veccopyd(gc[i].c,lcs[i].c);
		}
	}
	for(i=0;i<n;i++){
		//add offset to get GC coords
		vecaddd(gc[i].c,gc[i].c,planet->autoOrigin.c); 
	}
}





typedef struct _geoOffsetInfo {
	struct X3D_Node *node;
	Geosys *geoSystem;
	struct X3D_GeoOrigin *geoOrigin;
	struct SFVec3d *position;
	//struct SFRotation *orientation;
	struct SFVec3d *gdCoord;
	struct SFVec3d *gcCoord;      //-GC2NL
	struct SFVec3d *offsetCoord;  //-NL2SL
	struct SFVec4d *localOrient;  //-GCA2NLA
	struct SFVec4d *offsetOrient; //-NLA2SLA
} geoOffsetInfo;
//void origin_offsets(struct X3D_Node *node, Geosys *geoSystem, struct X3D_GeoOrigin *geoOrigin, 
//	struct SFVec3d *position, struct SFRotation *orientation, struct SFVec3d *localCoord, struct SFVec4d *localOrient,
//	struct SFVec3d *gdCoord)
void origin_offsets(geoOffsetInfo *gi)
{
	// assumes __geoSystem is already compiled.
	//
	//
	//v3.3 way - autoOrigin - B. capture as the self-origin
	struct Planet *planet;
	int specversion;
	struct SFVec3d slnla, *pslnla, slsla, *pslsla;
	ppComponent_Geospatial p = (ppComponent_Geospatial)gglobal()->Component_Geospatial.prv;
	specversion = X3D_PROTO(gi->node->_executionContext)->__specversion;
	pslnla = &slnla;
	pslsla = &slsla;

	planet = current_planet();
	if(gi->geoOrigin && specversion < 330 && !planet->autoOriginSet ){
		//geoOrgin is deprecated and tolerated in 3.0 - 3.2, but not tolerated in 3.3+
		//to simplify, we are using FCFS on a single geoOrigin.
		struct SFVec3d offset, *poffset;
		struct SFVec4d yup, *pyup;
		pyup = NULL;
		poffset = NULL;
		double *cc;
		initializeGeospatial(&gi->geoOrigin); 
		veccopyd(planet->autoOrigin.c,gi->geoOrigin->__movedCoords.c);
		GeoOrient(X3D_NODE(gi->geoOrigin), GEOSYS(gi->geoOrigin->__geoSystem), &gi->geoOrigin->__movedgd, &planet->autoOrient);
		planet->autoOriginSet = TRUE;
	}
	{
		//H: doesn't matter what the spec version is, we can do FCFS origin with any version
		//because we have the v3.3 fields 
		moveCoords3d(gi->geoSystem, NULL, NULL, 
			gi->position, 1, gi->gcCoord, gi->gdCoord);
		GeoOrient(X3D_NODE(gi->geoOrigin), gi->geoSystem, gi->gdCoord, gi->localOrient);

		if(!planet->autoOriginSet){
			//first come first serve FCFS autoOrigin
			veccopyd(planet->autoOrigin.c,gi->gcCoord->c);
			veccopy4d(planet->autoOrient.c,gi->localOrient->c);
			planet->autoOriginSet = TRUE;
		}
		//redo the transform, with origin offsets and rotations applied
		//moveCoords3d(gi->geoSystem, &p->autoOrigin, &p->autoOrient, 
		//	gi->position, 1, gi->localCoord, gi->gdCoord);
		vecdifd(gi->offsetCoord->c,gi->gcCoord->c,planet->autoOrigin.c);
		//NLGCA == offsetCoord
		{
			//rotation difference - change the sign on one rotation, and multiply
			Quaternion localQuat, relQuat, combQuat;
			vrmlrot_to_quaternion (&localQuat,gi->localOrient->c[0], gi->localOrient->c[1], gi->localOrient->c[2], -gi->localOrient->c[3]);
			vrmlrot_to_quaternion (&relQuat, planet->autoOrient.c[0], planet->autoOrient.c[1], planet->autoOrient.c[2], planet->autoOrient.c[3]);

			/* add these together */
			//quaternion_add (&combQuat, &relQuat, &localQuat);
			quaternion_multiply(&combQuat, &localQuat, &relQuat);
			//quaternion_multiply(&combQuat,&relQuat,&localQuat);
			quaternion_rotationd(pslnla->c,&localQuat,gi->offsetCoord->c);
			/* get the rotation; 2 steps to convert doubles to floats;
				   should be quaternion_to_vrmlrot(&combQuat, &node->__movedOrientation.c[0]... */
			quaternion_to_vrmlrot(&combQuat, &gi->offsetOrient->c[0], &gi->offsetOrient->c[1], &gi->offsetOrient->c[2], &gi->offsetOrient->c[3]);
			gi->offsetOrient->c[3] = - gi->offsetOrient->c[3];
			quaternion_rotationd(pslsla->c,&combQuat,pslnla->c);
			//in theory you can do SLSLA = autoOrient x NLGCA

		}
	}
	//vecdifd(gi->localCoord->c,gi->gcCoord->c,p->autoOrigin);
	//veccopy4d(gi->localOrient->c,p->autoOrient.c);

	if(1) {
		vecprint3db("\ttp-tpa",gi->position->c,"\n");
		vecprint3db("\tgd-gda",gi->gdCoord->c,"\n");
		vecprint3db("\tgc-gca",gi->gcCoord->c,"\n");
		vecprint3db("\tsn-gca",gi->offsetCoord->c,"\n");
		vecprint3db("\tsn-lna",pslnla->c,"\n");
		vecprint3db("\tsn-sna",pslsla->c,"\n");
		vecprint4db("\tlo",gi->localOrient->c,"\n");
		vecprint4db("\too",gi->offsetOrient->c,"\n");
	}

}
void update_origin(Geosys *geoSystem, struct X3D_Node *node, struct SFVec3d *userCoord, struct X3D_GeoOrigin *geoOrigin)
{
	// assumes __geoSystem is already compiled.
	// version < 3.3 - will try and use geoOrigin
	// version 3.3+ - ignors geoOrigin and uses FCFS (first (node) come first served) shared origin for a planet
	struct Planet *planet;
	int specversion;
	specversion = X3D_PROTO(node->_executionContext)->__specversion;

	planet = current_planet();
	if(!planet->autoOriginSet){
		if(geoOrigin && specversion < 330 ){
			//geoOrgin is deprecated and tolerated in 3.0 - 3.2, but not tolerated in 3.3+
			//to simplify, we are using FCFS on a single geoOrigin.
			struct SFVec3d offset, *poffset;
			struct SFVec4d yup, *pyup;
			pyup = NULL;
			poffset = NULL;
			double *cc;
			initializeGeospatial(&geoOrigin); 
			veccopyd(planet->autoOrigin.c,geoOrigin->__movedCoords.c);
			GeoOrient(X3D_NODE(geoOrigin), GEOSYS(geoOrigin->__geoSystem), &geoOrigin->__movedgd, &planet->autoOrient);
			planet->autoOriginSet = TRUE;
		}else{
			struct SFVec3d gdCoord;
			user2gc(geoSystem,userCoord,1,&planet->autoOrigin);
			gc2gd(geoSystem,&planet->autoOrigin,1,&gdCoord);
			GeoOrient(X3D_NODE(geoOrigin), geoSystem, &gdCoord, &planet->autoOrient);
			planet->autoOriginSet = TRUE;
		}
	}
}
void node2lcsRotation(Geosys *geoSystem, struct X3D_GeoOrigin *geoOrigin, struct SFVec3d *gdCoord, struct SFVec4d *rotation){
	struct SFVec4d localOrient;
	struct Planet *planet = current_planet();

	GeoOrient(X3D_NODE(geoOrigin), geoSystem, gdCoord, &localOrient);

	//rotation difference - change the sign on one rotation, and multiply
	Quaternion localQuat, relQuat, combQuat;
	vrmlrot_to_quaternion (&localQuat,localOrient.c[0], localOrient.c[1], localOrient.c[2], -localOrient.c[3]);
	vrmlrot_to_quaternion (&relQuat, planet->autoOrient.c[0], planet->autoOrient.c[1], planet->autoOrient.c[2], planet->autoOrient.c[3]);

	quaternion_multiply(&combQuat, &localQuat, &relQuat);
	quaternion_to_vrmlrot(&combQuat, &rotation->c[0], &rotation->c[1], &rotation->c[2], &rotation->c[3]);
	rotation->c[3] = - rotation->c[3];

}
/************************************************************************/
void compile_GeoCoordinate (struct X3D_GeoCoordinate * node) {
	MF_SF_TEMPS
	int i;

	#ifdef VERBOSE
	printf ("compiling GeoCoordinate\n");
	#endif

	/* standard MACROS expect specific field names */
	mIN = node->point;
	mOUT.p = NULL; mOUT.n = 0;


	INITIALIZE_GEOSPATIAL(node)
	COMPILE_GEOSYSTEM(node)
	MOVE_TO_ORIGIN(node)

	/* convert the doubles down to floats, because coords are used as floats in FreeWRL. */
	FREE_IF_NZ(node->__movedCoords.p);
	node->__movedCoords.p = MALLOC (struct SFVec3f *, sizeof (struct SFVec3f)  * mOUT.n);
	for (i=0; i<mOUT.n; i++) {
		node->__movedCoords.p[i].c[0] = (float) mOUT.p[i].c[0];
		node->__movedCoords.p[i].c[1] = (float) mOUT.p[i].c[1];
		node->__movedCoords.p[i].c[2] = (float) mOUT.p[i].c[2];
		#ifdef VERBOSE
		printf ("coord %d now is %f %f %f\n", i, node->__movedCoords.p[i].c[0],node->__movedCoords.p[i].c[1],node->__movedCoords.p[i].c[2]);
		#endif
	}
	node->__movedCoords.n = mOUT.n;

	FREE_IF_NZ(gdCoords.p);
	FREE_IF_NZ(mOUT.p);
	MARK_NODE_COMPILED
	
	/* events */
	/* MARK_SFNODE_INOUT_EVENT(node->metadata, node->__oldmetadata, offsetof (struct X3D_GeoCoordinate, metadata)) */
}


/************************************************************************/
/* GeoElevationGrid							*/
/************************************************************************/

/* check validity of ElevationGrid fields */
int checkX3DGeoElevationGridFields (struct X3D_GeoElevationGrid *node, float **points, int *npoints) {
	MF_SF_TEMPS
	int i,j;
	int nx;
	double xSp;
	int nz;
	double zSp;
	double *height;
	int ntri;
	int nh;
	struct X3D_PolyRep *rep;
	float *newpoints;
	int nquads;
	int *cindexptr;
	float *texcoord = NULL;
	//double myHeightAboveEllip = 0.0;
	int mySRF = 0;
	Geosys *gs;
	
	nx = node->xDimension;
	xSp = node->xSpacing;
	nz = node->zDimension;
	zSp = node->zSpacing;
	height = node->height.p;
	nh = node->height.n;

	COMPILE_GEOSYSTEM(node)
	/* various values for converting to GD/UTM, etc */
	if (node->__geoSystem != NULL)  {
		mySRF = GEOSYS(node->__geoSystem)->spatial_system;
		/* NOTE - DO NOT DO THIS CALCULATION - it is added in later 
		myHeightAboveEllip = getEllipsoidRadius(node->__geoSystem.p[1]);
		*/
	}

	rep = node->_intern;

	/* work out how many triangles/quads we will have */
	ntri = (nx && nz ? 2 * (nx-1) * (nz-1) : 0);
	nquads = ntri/2;
	//printf("nx %d nz %d nquads %d ntri %d\n",nx,nz,nquads,ntri);
	/* check validity of input fields */
	if(nh != nx * nz) {
		if (nh > nx * nz) {
			printf ("GeoElevationgrid: warning: x,y vs. height: %d * %d ne %d:\n", nx,nz,nh);
		} else {
			printf ("GeoElevationgrid: error: x,y vs. height: %d * %d ne %d:\n", nx,nz,nh);
			return FALSE;
		}
	}

	/* do we have any triangles? */
	if ((nx < 2) || (nz < 2)) {
		printf ("GeoElevationGrid: xDimension and zDimension less than 2 %d %d\n", nx,nz);
		return FALSE;
	}

	//printf ("checkX3DGeoElevationGrid - node->texCoord %p\n",node->texCoord);


	/* any texture coordinates passed in? if so, DO NOT generate any texture coords here. */
	if (!(node->texCoord)) {
		/* allocate memory for texture coords */
		FREE_IF_NZ(rep->GeneratedTexCoords[0]);

		/* 6 vertices per quad each vertex has a 2-float tex coord mapping */
		texcoord = rep->GeneratedTexCoords[0] = MALLOC (float *, sizeof (float) * nquads * 12); 

		rep->tcindex=0; /* we will generate our own mapping */
	}

	/* make up points array */
	/* a point is a vertex and consists of 3 floats (x,y,z) */
	newpoints = MALLOC (float *, sizeof (float) * nz * nx * 3);
	 
	FREE_IF_NZ(rep->actualCoord);
	rep->actualCoord = (float *)newpoints;

	/* make up coord index */
	if (node->_coordIndex.n > 0) {FREE_IF_NZ(node->_coordIndex.p);}
	node->_coordIndex.p = MALLOC (int *, sizeof(int) * nquads * 5);
	cindexptr = node->_coordIndex.p;

	node->_coordIndex.n = nquads * 5; //H: 4 points and -1 to end the face
	/* return the newpoints array to the caller */
	*points = newpoints;
	*npoints = node->_coordIndex.n;

	#ifdef VERBOSE
	printf ("coordindex:\n");
	#endif

	/* ElevationGrids go 1 - 2 - 3 - 4 we go 1 - 4 - 3 - 2 */
	//printf ("GeoElevationGrids, nz %d, nx %d\n",nz,nx);

	for (j = 0; j < (nz -1); j++) {
		for (i=0; i < (nx-1) ; i++) {
			#ifdef VERBOSE
			printf ("	%d %d %d %d %d\n", j*nx+i, j*nx+i+nx, j*nx+i+nx+1, j*nx+i+1, -1);
			#endif

#ifdef WINDING_ELEVATIONGRID
			*cindexptr = j*nx+i; cindexptr++; 	/* 1 */
			*cindexptr = j*nx+i+nx; cindexptr++; 	/* 2 */
			*cindexptr = j*nx+i+nx+1; cindexptr++;  /* 3 */
			*cindexptr = j*nx+i+1; cindexptr++; 	/* 4 */
			*cindexptr = -1; cindexptr++;
#else
			*cindexptr = j*nx+i; cindexptr++; 	/* 1 */
			*cindexptr = j*nx+i+1; cindexptr++; 	/* 4 */
			*cindexptr = j*nx+i+nx+1; cindexptr++;  /* 3 */
			*cindexptr = j*nx+i+nx; cindexptr++; 	/* 2 */
			*cindexptr = -1; cindexptr++;
#endif

		}
	}

	/* tex coords These need to be streamed now; that means for each quad, each vertex needs its tex coords. */
	/* if the texCoord node exists, let render_TextureCoordinate (or whatever the node is) do our work for us */
	if (!(node->texCoord)) {
        //printf ("geoelevationgrid, doing %d x %d texture coords; tcoord %p\n",nz-1,nx-1,texcoord);
		for (j = 0; j < (nz -1); j++) {
			for (i=0; i < (nx-1) ; i++) {
				/* first triangle, 3 vertexes */
#ifdef WINDING_ELEVATIONGRID
				/* first tri */
/* 1 */				*texcoord = ((float) (i+0)/(nx-1)); texcoord++;
				*texcoord = ((float)(j+0)/(nz-1)); texcoord ++; 
			
/* 2 */				*texcoord = ((float) (i+0)/(nx-1)); texcoord++;
				*texcoord = ((float)(j+1)/(nz-1)); texcoord ++; 
	
/* 3 */				*texcoord = ((float) (i+1)/(nx-1)); texcoord++;
				*texcoord = ((float)(j+1)/(nz-1)); texcoord ++; 
	
				/* second tri */
/* 1 */				*texcoord = ((float) (i+0)/(nx-1)); texcoord++;
				*texcoord = ((float)(j+0)/(nz-1)); texcoord ++; 
	
/* 3 */				*texcoord = ((float) (i+1)/(nx-1)); texcoord++;
				*texcoord = ((float)(j+1)/(nz-1)); texcoord ++; 
	
/* 4 */				*texcoord = ((float) (i+1)/(nx-1)); texcoord++;
				*texcoord = ((float)(j+0)/(nz-1)); texcoord ++; 
#else
				/* first tri */
				*texcoord = ((float) (i+0)/(nx-1)); texcoord++; /* 1 */
				*texcoord = ((float) (j+0)/(nz-1)); texcoord++; 

				*texcoord = ((float) (i+1)/(nx-1)); texcoord++; /* 4 */
				*texcoord = ((float) (j+0)/(nz-1)); texcoord++; 

				*texcoord = ((float) (i+1)/(nx-1)); texcoord++; /* 3 */
				*texcoord = ((float) (j+1)/(nz-1)); texcoord++; 

				/* second tri */
				*texcoord = ((float) (i+0)/(nx-1)); texcoord++; /* 1 */
				*texcoord = ((float) (j+0)/(nz-1)); texcoord++; 

				*texcoord = ((float) (i+1)/(nx-1)); texcoord++; /* 3 */
				*texcoord = ((float) (j+1)/(nz-1)); texcoord++; 

				*texcoord = ((float) (i+0)/(nx-1)); texcoord++; /* 2 */
				*texcoord = ((float) (j+1)/(nz-1)); texcoord++; 
			
#endif
			}
		}
		//for (i=0; i<10; i++) printf ("geoele tc %d is %f\n",i,rep->GeneratedTexCoords[i]);
	}
			
	/* Render_Polyrep will use this number of triangles */
	rep->ntri = ntri;

	/* initialize arrays used for passing values into/out of the MOVE_TO_ORIGIN(node) values */
	mIN.n = nx * nz; 
	mIN.p = MALLOC (struct SFVec3d *, sizeof (struct SFVec3d) * mIN.n);

	mOUT.n=0; mOUT.p = NULL;
	gdCoords.n=0; gdCoords.p = NULL;
	struct SFVec3d lastpoint, firstpoint;
	/* make up a series of points, then go and convert them to local coords */
	for (j=0; j<nz; j++) {
		for (i=0; i < nx; i++) {
			int k = i+(j*nx);
			#ifdef VERBOSE
		 	printf ("		%lf %lf %lf # (hei ind %d) point [%d, %d]\n",
				xSp * i,
				height[i+(j*nx)] * ((double)node->yScale),
				zSp * j,
				i+(j*nx), i,j);
			#endif
		
		
			/* Make up a new vertex. Add the geoGridOrigin to every point */

			if ((mySRF == GEOSP_GD) || (mySRF == GEOSP_UTM) || (mySRF == GEOSP_3TM)) {
				/* GD - give it to em in Latitude/Longitude/Elevation order */
				/* UTM- or give it to em in Northing/Easting/Elevation order */
				/* latitude - range of -90 to +90 */
				mIN.p[k].c[0] = zSp * j + node->geoGridOrigin.c[0]; 
	
				/* longitude - range -180 to +180, or 0 to 360 */
				mIN.p[k].c[1] =xSp * i + node->geoGridOrigin.c[1];
	
				/* elevation, above geoid */
				mIN.p[k].c[2] = (height[k] *(node->yScale)) + node->geoGridOrigin.c[2];
				veccopyd(lastpoint.c,mIN.p[k].c);
				if(i==0 && j==0) veccopyd(firstpoint.c,mIN.p[k].c);
					//+ myHeightAboveEllip; 
			} else {
				/* nothing quite specified here - what do we really do??? */
				mIN.p[k].c[0] = zSp * j + node->geoGridOrigin.c[0]; 
	
				mIN.p[k].c[1] =xSp * i + node->geoGridOrigin.c[1];
	
				mIN.p[k].c[2] = (height[k] *(node->yScale)) + node->geoGridOrigin.c[2];
					//+ myHeightAboveEllip; 

			}
			/* printf ("height made up of %lf, geoGridOrigin %lf, myHeightAboveEllip %lf\n",(height[i+(j*nx)] *(node->yScale)),node->geoGridOrigin.c[2], myHeightAboveEllip); */
		}
	}
	#ifdef VERBOSE
	vecprint3db("firstpoint",firstpoint.c,"\n");
	vecprint3db(" lastpoint",lastpoint.c,"\n");
	vecprint3db("nx*nz",mIN.p[nx*nz -1].c,"\n");
	printf ("points before moving origin, lat, lon, height, index:\n");
	for (j=0; j<nz; j++) {
		for (i=0; i < nx; i++) {
			int k = i+(j*nx);
			printf ("	%lf %lf %lf %d\n",mIN.p[k].c[0],
				mIN.p[k].c[1],mIN.p[k].c[2],k);

		}
		printf("\n");
	}
	#endif

	/* convert this point to a local coordinate */
	if(MAR12){
		Geosys *gs;
		compile_geoSystem(X3D_NODE(node),node->_nodeType,&node->geoSystem,&node->__geoSystem);
		gs = GEOSYS(node->__geoSystem);

		update_origin(gs, X3D_NODE(node), &node->geoGridOrigin, X3D_GEOORIGIN(node->geoOrigin));
		mOUT.p = MALLOC(struct SFVec3d*,sizeof(struct SFVec3d)*mIN.n);
		mOUT.n = mIN.n;
		user2gc(gs,mIN.p,mIN.n,mOUT.p);
		#ifdef VERBOSE
		printf ("points in gc XYZ, index:\n");
		for (j=0; j<nz; j++) {
			for (i=0; i < nx; i++) {
				double ci[3], co[3];
				int k = i+(j*nx);
				veccopyd(co,mOUT.p[k].c);
				printf ("	%lf %lf %lf %d\n",co[0],co[1],co[2],k);
				veccopyd(ci,mIN.p[k].c);
				printf ("	%lf %lf %lf %d\n",ci[0],ci[1],ci[2],k);

			}
			printf("\n");
		}
		#endif

		gc2lcs(gs,mOUT.p,mOUT.n,mOUT.p);

	}else{
		//struct SFVec3d *gcCoord;      //-GC2NL
		//struct SFVec3d *offsetCoord;  //-NL2SL
		//struct SFVec4d *localOrient;  //-GCA2NLA
		//struct SFVec4d *offsetOrient; //-NLA2SLA

		//step 1 compute origin
		geoOffsetInfo ggi, *gi;
		struct SFVec3d gdCoord, gcCoord;
		struct SFVec4d locOrient;
		struct SFVec4d *yup;
		Quaternion qup;
		ppComponent_Geospatial p = (ppComponent_Geospatial)gglobal()->Component_Geospatial.prv;

		compile_geoSystem(X3D_NODE(node),node->_nodeType,&node->geoSystem,&node->__geoSystem);
		gi = &ggi;
		gi->node = X3D_NODE(node);
		gi->geoOrigin = X3D_GEOORIGIN(node->geoOrigin);
		gi->geoSystem = GEOSYS(node->__geoSystem);
		gi->position = &node->geoGridOrigin;
		gi->offsetCoord = &node->__autoOffset;
		gi->localOrient = &locOrient;
		gi->offsetOrient = &node->__localOrient;
		gi->gdCoord = &gdCoord;
		gi->gcCoord = &gcCoord;
		printf("GEG:\n");
		origin_offsets(gi);
		//step 2 apply autoOrigin to GC coords
		mOUT.p = MALLOC(struct SFVec3d*,sizeof(struct SFVec3d)*mIN.n);
		gdCoords.p = MALLOC(struct SFVec3d*,sizeof(struct SFVec3d)*mIN.n);

		//A. GD TO GCGCA 
		moveCoords3d(GEOSYS(node->__geoSystem),NULL,NULL, //&node->__localOrient,
		mIN.p,mIN.n,mOUT.p,gdCoords.p);

		//B. GCGCA 2 NLNLA
			
		for(i=0;i<mIN.n;i++){
			//take offset off GC coords
			vecdifd(mOUT.p[i].c,mOUT.p[i].c,gcCoord.c); 
		}
		if(1)for(i=0;i<mIN.n;i++){
			//take offset off GC coords
			vecaddd(mOUT.p[i].c,mOUT.p[i].c,node->__autoOffset.c); 
		}

		yup = &locOrient;
		vrmlrot_to_quaternion(&qup,yup->c[0],yup->c[1],yup->c[2],-yup->c[3]);
		for(i=0;i<mIN.n;i++){
			//take offset off GC coords
			quaternion_rotationd(mOUT.p[i].c,&qup,mOUT.p[i].c);
		}


		//C. NLNLA to SLSLA
		yup = &node->__localOrient;
		vrmlrot_to_quaternion(&qup,yup->c[0],yup->c[1],yup->c[2],yup->c[3]);
		for(i=0;i<mIN.n;i++){
			//take offset off GC coords
			quaternion_rotationd(mOUT.p[i].c,&qup,mOUT.p[i].c);
		}
			

	}


	/* copy the resulting array back to the ElevationGrid */

	for (j=0; j<nz; j++) {
		for (i=0; i < nx; i++) {
			/* copy this coordinate into our ElevationGrid array */
			int k = i+(j*nx);
			double2float(newpoints,mOUT.p[k].c,3);
			newpoints += 3;
		}
	}
	#ifdef VERBOSE
	printf ("points converted to mesh coords, xyz index:\n");
	newpoints = rep->actualCoord;
	for (j=0; j<nz; j++) {
		for (i=0; i < nx; i++) {
			/* copy this coordinate into our ElevationGrid array */
			int k = i+(j*nx);
			printf ("	%f %f %f %d\n",newpoints[0],newpoints[1],newpoints[2],k);
			newpoints += 3;
		}
		printf("\n");
	}
	#endif //VERBOSE
	FREE_MF_SF_TEMPS;
	return TRUE;
}


/* a GeoElevationGrid creates a "real" elevationGrid node as a child for rendering. */
void compile_GeoElevationGrid (struct X3D_GeoElevationGrid * node) {
// 2018 not called, see compile stack in render_
//	#ifdef VERBOSE
//	printf ("compiling GeoElevationGrid\n");
//	#endif
//	printf ("compiling GeoElevationGrid\n");
//
//	INITIALIZE_GEOSPATIAL(node)
//	COMPILE_GEOSYSTEM(node)
//	MARK_NODE_COMPILED
//	
//	/* events */
//	/* MARK_SFNODE_INOUT_EVENT(node->metadata, node->__oldmetadata, offsetof (struct X3D_GeoElevationGrid, metadata)) */
//
}
int planetInPlanets(int planet, struct Multi_Int32 *planets){
	int i,ifound = -1;
	for(i=0;i<planets->n;i++)
		if(planets->p[i] == planet) ifound = i;
	return ifound > -1;
}
void RegisterGeoElevationGrid(struct X3D_Node *node, int planetID);
void render_GeoElevationGrid (struct X3D_GeoElevationGrid *node) {
	/*compile stack for geoElevationGrid:
	checkX3DGeoElelvationGridFields *see function above
	make_genericfaceset
	compile_polyrep
	compileNode
	render_GeoElevationGrid *you are here
	*/
	//INITIALIZE_GEOSPATIAL(node)
	int planetID = 0; 
	initializeGeospatial((struct X3D_GeoOrigin **) &node->geoOrigin); 
	planetID = current_planetId();
	COMPILE_POLY_IF_REQUIRED (NULL, NULL, node->color, node->normal, node->texCoord) 
	CULL_FACE(node->solid)
	render_polyrep(node);
	if(!planetInPlanets(planetID,&node->__planets)){
		//planetID default 0 for now
		// will be "P#" in geosystem, or <GeoPlanet ID="#"><GeoElevationGrid/></GeoPlanet>
		RegisterGeoElevationGrid(X3D_NODE(node), planetID);
		node->__planets.p = realloc(node->__planets.p,(node->__planets.n+1)*sizeof(int));
		node->__planets.p[node->__planets.n] = planetID;
		node->__planets.n++;
	}
}

/************************************************************************/
/* GeoLocation								*/
/************************************************************************/
//double adjust_geoLocationRelativeHeight(struct X3D_GeoLocation *node,int planetID);
void compile_GeoLocation (struct X3D_GeoLocation * node) {
	// JAS int i;
	int specversion;
	geoOffsetInfo ggi, *gi;
	struct SFVec3d gdCoord, gcCoord;
	struct SFVec4d locOrient;
	struct Planet *planet;
	Geosys *gs;
	//ppComponent_Geospatial p = (ppComponent_Geospatial)gglobal()->Component_Geospatial.prv;

	planet = current_planet();
	#ifdef VERBOSE
	printf ("compiling GeoLocation\n");
	#endif
		//step 1 compute origin
	compile_geoSystem(X3D_NODE(node),node->_nodeType,&node->geoSystem,&node->__geoSystem);
	gs = GEOSYS(node->__geoSystem);
	if(node->relativeHeight) gs->relativeHeight = TRUE; //handy for user2anything conversion function: don't need to pass node

	if(MAR12){
		update_origin(gs, X3D_NODE(node), &node->geoCoords, X3D_GEOORIGIN(node->geoOrigin));
	}
	else
	{
		gi = &ggi;
		gi->node = X3D_NODE(node);
		gi->geoOrigin = X3D_GEOORIGIN(node->geoOrigin);
		gi->geoSystem = gs;
		gi->position = &node->geoCoords;  //it claims this gets routed to, need dynamic offset
		gi->offsetCoord = &node->__movedCoords; //__localCoords; //__autoOffset;
		gi->localOrient = &node->__localOrient; //&locOrient;
		gi->offsetOrient = &node->__offsetOrient;
		gi->gdCoord = &gdCoord;
		gi->gcCoord = &gcCoord;
		printf("GL:\n");
		origin_offsets(gi);
		//vecscaled(node->__movedCoords.c,node->__movedCoords.c,-1.0);
		veccopy4d(node->__localOrient.c,planet->autoOrient.c);
		veccopyd(node->__movedgd.c,gdCoord.c);
		if(veclengthd(node->__position.c) == 0.0)
			veccopyd(node->__position.c,gdCoord.c);

		//#ifdef VERBOSE
		printf ("compile_GeoLocation,\n\t orig coords %lf %lf %lf, \n\t moved %lf %lf %lf\n", 
		node->geoCoords.c[0], node->geoCoords.c[1], node->geoCoords.c[2], 
		node->__movedCoords.c[0], node->__movedCoords.c[1], node->__movedCoords.c[2]);
		printf ("	rotation is %lf %lf %lf %lf\n",
				node->__localOrient.c[0],
				node->__localOrient.c[1],
				node->__localOrient.c[2],
				node->__localOrient.c[3]);
		//#endif
	}
	if(0)  //don't need this in compile_ because prep_ is doing it too
	if(MAR12){
		//cylce test - should be able to transform elsewhere and back
		// with only numerical noise difference.
		struct SFVec3d gcCoords, gdCoords, userCoords, lcsCoords;
		user2gc(gs,&node->geoCoords,1,&gcCoords);
		gc2lcs(gs,&gcCoords,1,&lcsCoords);
		vecprint3db("   gc0",gcCoords.c,"\n");
		vecprint3db("   lcs",lcsCoords.c,"\n");
		vecprint3db("_movlc",node->__movedCoords.c,"\n");
		lcs2gc(gs,&lcsCoords,1,&gcCoords);
		vecprint3db("   gc1",gcCoords.c,"\n");
		gc2gd(gs,&gcCoords,1,&gdCoords);

		vecprint3db("_movgd",node->__movedgd.c,"\n");
		vecprint3db(" gc2gd",gdCoords.c,"\n");
		gd2gc(gs,&gdCoords,1,&gcCoords);
		gc2user(gs,&gcCoords,1,&userCoords);
		vecprint3db("geoCrd",node->geoCoords.c,"\n");
		vecprint3db("gc2usr",userCoords.c,"\n");
		if(MAR12){
			//beyond cycle testing, how does it look when used
			veccopyd(node->__movedgd.c,gdCoords.c);
			veccopyd(node->__movedCoords.c,lcsCoords.c);
			if(veclengthd(node->__position.c) == 0.0)
				veccopyd(node->__position.c,gdCoord.c);


		}
		node2lcsRotation(gs, X3D_GEOORIGIN(node->geoOrigin), &node->__movedgd, &node->__offsetOrient);
	}


	/* did the geoCoords change?? */
	MARK_SFVEC3D_INOUT_EVENT(node->geoCoords, node->__oldgeoCoords, offsetof (struct X3D_GeoLocation, geoCoords))

	/* how about the children field ?? */
	MARK_MFNODE_INOUT_EVENT(node->children, node->__oldChildren, offsetof (struct X3D_GeoLocation, children))

	REINITIALIZE_SORTED_NODES_FIELD(node->children,node->_sortedChildren);
	MARK_NODE_COMPILED
	
	/* events */
	/* MARK_SFNODE_INOUT_EVENT(node->metadata, node->__oldmetadata, offsetof (struct X3D_GeoLocation, metadata)) */

	INITIALIZE_EXTENT;

	#ifdef VERBOSE
	printf ("compiled GeoLocation\n\n");
	#endif
}

void child_GeoLocation (struct X3D_GeoLocation *node) {
	CHILDREN_COUNT
	//LOCAL_LIGHT_SAVE
	//INITIALIZE_GEOSPATIAL(node)
	COMPILE_IF_REQUIRED

	OCCLUSIONTEST


	/* {
		int x;
		struct X3D_Node *xx;

		printf ("child_GeoLocation, this %d \n",node);
		for (x=0; x<nc; x++) {
			xx = X3D_NODE(node->children.p[x]);
			printf ("	ch %d type %s dist %f\n",node->children.p[x],stringNodeType(xx->_nodeType),xx->_dist);
		}
	} */

	/* Check to see if we have to check for collisions for this transform. */

	RETURN_FROM_CHILD_IF_NOT_FOR_ME

	/* do we have a local for a child? */
	//LOCAL_LIGHT_CHILDREN(node->children);
	prep_sibAffectors((struct X3D_Node*)node,&node->__sibAffectors);

	/* now, just render the non-directionalLight children */

	/* printf ("GeoLocation %d, flags %d, render_sensitive %d\n",
			node,node->_renderFlags,render_sensitive); */

	#ifdef CHILDVERBOSE
		printf ("GeoLocation - doing normalChildren\n");
	#endif

	normalChildren(node->children);

	#ifdef CHILDVERBOSE
		printf ("GeoLocation - done normalChildren\n");
	#endif

	//LOCAL_LIGHT_OFF
	fin_sibAffectors((struct X3D_Node*)node,&node->__sibAffectors);

}

/* do transforms, calculate the distance */
void prep_GeoLocation (struct X3D_GeoLocation *node) {
	//INITIALIZE_GEOSPATIAL(node)
	COMPILE_IF_REQUIRED

	/* rendering the viewpoint means doing the inverse transformations in reverse order (while poping stack),
	* so we do nothing here in that case -ncoder */

	/* printf ("prep_GeoLocation, render_hier vp %d geom %d light %d sens %d blend %d prox %d col %d\n",
	render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision); */

	/* do we have any geometry visible, and are we doing anything with geometry? */
	OCCLUSIONTEST

	if(!renderstate()->render_vp) {
		if(MAR12){
			//retransform on every frame? why not in compile_?
			//1. user2gc does relativeHeight against GeoElevationGrid GEG nodes registered for the planet
			//     - and GEGs aren't registered till they are compiled, which may be after GL is compiled
			//2. the .geoCoords field is for routing to, according to specs, and may change often
			//		- is there a way to avoid compile_ completely? Maybe if we do the full trans here.
			//		- would need to do the orientation too.
			Geosys *gs;
			struct SFVec3d gcCoords, gdCoords, userCoords, lcsCoords;
			gs = GEOSYS(node->__geoSystem);
			user2gc(gs,&node->geoCoords,1,&gcCoords);
			gc2lcs(gs,&gcCoords,1,&lcsCoords);
			gc2gd(gs,&gcCoords,1,&gdCoords);

			veccopyd(node->__movedgd.c,gdCoords.c);
			veccopyd(node->__movedCoords.c,lcsCoords.c);
			node2lcsRotation(gs, X3D_GEOORIGIN(node->geoOrigin), &node->__movedgd, &node->__offsetOrient);
		}


		FW_GL_PUSH_MATRIX();

		if(!MAR12)	FW_GL_ROTATE_RADIANS(-node->__localOrient.c[3], node->__localOrient.c[0],node->__localOrient.c[1],node->__localOrient.c[2]);
		/* TRANSLATION */
		FW_GL_TRANSLATE_D(node->__movedCoords.c[0], node->__movedCoords.c[1], node->__movedCoords.c[2]);

		//printf ("prep_GeoLoc trans to %lf %lf %lf\n",node->__movedCoords.c[0],node->__movedCoords.c[1],node->__movedCoords.c[2]);

		if(!MAR12)	FW_GL_ROTATE_RADIANS(node->__localOrient.c[3], node->__localOrient.c[0],node->__localOrient.c[1],node->__localOrient.c[2]);
		FW_GL_ROTATE_RADIANS(node->__offsetOrient.c[3], node->__offsetOrient.c[0],node->__offsetOrient.c[1],node->__offsetOrient.c[2]);

		/*
		printf ("geoLocation trans %7.4f %7.4f %7.4f\n",node->__movedCoords.c[0], node->__movedCoords.c[1], node->__movedCoords.c[2]);
		printf ("geoLocation rotat %7.4f %7.4f %7.4f %7.4f\n",my_rotation, node->__localOrient.c[0],node->__localOrient.c[1],node->__localOrient.c[2]);
		*/

		/* did either we or the Viewpoint move since last time? */
		RECORD_DISTANCE
		if(renderstate()->render_boxes) extent6f_draw(node->_extent);
	}
}
void fin_GeoLocation (struct X3D_GeoLocation *node) {
	//INITIALIZE_GEOSPATIAL(node)
	COMPILE_IF_REQUIRED
	OCCLUSIONTEST

	if(!renderstate()->render_vp) {
		FW_GL_POP_MATRIX();
	} else {
		if ((node->_renderFlags & VF_Viewpoint) == VF_Viewpoint) {
			if(!MAR12) FW_GL_ROTATE_RADIANS(-node->__localOrient.c[3], node->__localOrient.c[0],node->__localOrient.c[1],node->__localOrient.c[2]);
			if(MAR12) FW_GL_ROTATE_RADIANS(-node->__offsetOrient.c[3], node->__offsetOrient.c[0],node->__offsetOrient.c[1],node->__offsetOrient.c[2]);

			FW_GL_TRANSLATE_D(-node->__movedCoords.c[0], -node->__movedCoords.c[1], -node->__movedCoords.c[2]);
		}
	}
}

/************************************************************************/
/* GeoLOD								*/
/************************************************************************/
void add_node_to_broto_context(struct X3D_Proto *currentContext,struct X3D_Node *node);

void deleteMallocedFieldValue(int type,union anyVrml *fieldPtr);
void LOAD_CHILD(struct X3D_GeoLOD *node, struct X3D_Node **childNode, struct Multi_String *childUrl) {
	/* printf ("start of LOAD_CHILD, url has %d strings\n",node->childUrl.n); */
	int i;
	if (childUrl->n > 0) {
		/* create new inline node, link it in */
		if (*childNode == NULL) {
			*childNode = createNewX3DNode(NODE_Inline);
			if(node->_executionContext)
				add_node_to_broto_context(X3D_PROTO(node->_executionContext),X3D_NODE(*childNode));
			ADD_PARENT(X3D_NODE(*childNode), X3D_NODE(node));
 		}
		/* copy over the URL from parent */
		deleteMallocedFieldValue(FIELDTYPE_MFString,(union anyVrml*)&X3D_INLINE(*childNode)->url);
		X3D_INLINE(*childNode)->url.p = MALLOC(struct Uni_String **, sizeof(struct Uni_String)*childUrl->n);
		for (i=0; i<childUrl->n; i++) {
			/* printf ("copying over url %s\n",node->childUrl.p[i]->strptr); */
			X3D_INLINE(*childNode)->url.p[i] = newASCIIString(childUrl->p[i]->strptr);
		}
		/* printf ("loading, and urlCount is %d\n",node->childUrl.n); */
		X3D_INLINE(*childNode)->url.n = childUrl->n;
		X3D_INLINE(*childNode)->load = TRUE;
	}  
}

#define UNLOAD_CHILD(childNode) \
	if (node->childNode != NULL) \
			X3D_INLINE(node->childNode)->load = FALSE;


static void GeoLODchildren (struct X3D_GeoLOD *node) {
	int load = node->__inRange;

	/* lets see if we still have to load this one... */
	if (((node->__childloadstatus)==0) && (load)) {
		#ifdef VERBOSE
		ppComponent_Geospatial p = (ppComponent_Geospatial)gglobal()->Component_Geospatial.prv;

		printf ("GeoLODchildren - have to LOAD_CHILD for node %u (level %d)\n",node,p->geoLodLevel); 
		#endif

		LOAD_CHILD(node,&node->__child1Node,&node->child1Url);
		LOAD_CHILD(node,&node->__child2Node,&node->child2Url);
		LOAD_CHILD(node,&node->__child3Node,&node->child3Url);
		LOAD_CHILD(node,&node->__child4Node,&node->child4Url);

		//LOAD_CHILD(__child1Node,child1Url)
		//LOAD_CHILD(__child2Node,child2Url)
		//LOAD_CHILD(__child3Node,child3Url)
		//LOAD_CHILD(__child4Node,child4Url)
		node->__childloadstatus = 1;
	}
}
//void GeoLODchildren1 (struct X3D_GeoLOD *node) {
//	GeoLODchildren(node);
//}
static void GeoUnLODchildren (struct X3D_GeoLOD *node) {
	int load = node->__inRange;

	if (!(load) && ((node->__childloadstatus) != 0)) {
		#ifdef VERBOSE
			ppComponent_Geospatial p = (ppComponent_Geospatial)gglobal()->Component_Geospatial.prv;
			printf ("GeoLODloadChildren, removing children from node %u level %d\n",node,p->geoLodLevel);
		#endif
		UNLOAD_CHILD(__child1Node)
		UNLOAD_CHILD(__child2Node)
		UNLOAD_CHILD(__child3Node)
		UNLOAD_CHILD(__child4Node)

		node->__childloadstatus = 0;
	}
}


static void GeoLODrootUrl (struct X3D_GeoLOD *node) {
	int load = node->__inRange == 0; //dug9 it's when you are out of range that you should get the rootnode

	/* lets see if we still have to load this one... */
	if (((node->__rooturlloadstatus)==0) && (load)) {
		#ifdef VERBOSE
		printf ("GeoLODrootUrl - have to LOAD_CHILD for node %u\n",node); 
		#endif

		LOAD_CHILD(node,&node->__rootUrl, &node->rootUrl);
		//LOAD_CHILD(__rootUrl, rootUrl)

		node->__rooturlloadstatus = 1;
	}
}


static void GeoUnLODrootUrl (struct X3D_GeoLOD *node) {
	int load = node->__inRange;

	if (!(load) && ((node->__rooturlloadstatus) != 0)) {
		#ifdef VERBOSE
		printf ("GeoLODloadChildren, removing rootUrl\n");
		#endif
		node->__childloadstatus = 0;
	}
}



void compile_GeoLOD (struct X3D_GeoLOD * node) {
	if(MAR12){
		Geosys *gs;
		struct SFVec3d gcCoord;
		compile_geoSystem(X3D_NODE(node),node->_nodeType,&node->geoSystem,&node->__geoSystem);
		gs = GEOSYS(node->__geoSystem);

		update_origin(gs, X3D_NODE(node), &node->center, X3D_GEOORIGIN(node->geoOrigin));
		user2gc(gs,&node->center,1,&gcCoord);
		gc2lcs(gs,&gcCoord,1,&node->__movedCoords);
		MARK_NODE_COMPILED

	}else{
		MF_SF_TEMPS

		#ifdef VERBOSE
		printf ("compiling GeoLOD %u\n",node);
		#endif

		/* work out the position */
		INITIALIZE_GEOSPATIAL(node)
		COMPILE_GEOSYSTEM(node)
		INIT_MF_FROM_SF(node, center)
		MOVE_TO_ORIGIN(node)
		COPY_MF_TO_SF(node, __movedCoords)

		#ifdef VERBOSE
		printf ("compile_GeoLOD %u, orig coords %lf %lf %lf, moved %lf %lf %lf\n", node, node->center.c[0], node->center.c[1], node->center.c[2], node->__movedCoords.c[0], node->__movedCoords.c[1], node->__movedCoords.c[2]);

		printf ("children.n %d childurl 1: %u 2: %u 3: %u 4: %u rootUrl: %u rootNode: %d\n",
		node->children,
		node->child1Url,
		node->child2Url,
		node->child3Url,
		node->child4Url,
		node->rootUrl,
		node->rootNode.n);
		#endif

		MARK_NODE_COMPILED
		FREE_MF_SF_TEMPS
	
		/* events */
		/* MARK_SFNODE_INOUT_EVENT(node->metadata, node->__oldmetadata, offsetof (struct X3D_GeoLOD, metadata)) */


		#ifdef VERBOSE
		printf ("compiled GeoLOD\n\n");
		#endif
	}
}
#undef VERBOSE


void child_GeoLOD (struct X3D_GeoLOD *node) {
	int i;
	ppComponent_Geospatial p = (ppComponent_Geospatial)gglobal()->Component_Geospatial.prv;

	if(!MAR12) INITIALIZE_GEOSPATIAL(node)
	COMPILE_IF_REQUIRED

	#ifdef VERBOSE
	 printf ("child_GeoLOD %u (level %d), renderFlags %x render_hier vp %d geom %d light %d sens %d blend %d prox %d col %d\n",
	node,
	p->geoLodLevel, 
	node->_renderFlags,
	 render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision); 
	#endif
	//ConsoleMessage("glod kids=%d\r",node->children.n);
	/* for debugging purposes... */
	if (node->__level == -1) node->__level = p->geoLodLevel;
	else if (node->__level != p->geoLodLevel) {
		printf ("hmmm - GeoLOD %p was level %d, now %d\n",node,node->__level, p->geoLodLevel);
	}

	#ifdef VERBOSE
	if ( node->__inRange) {
		printf ("GeoLOD %u (level %d) closer\n",node,p->geoLodLevel);
	} else {
		printf ("GeoLOD %u (level %d) farther away\n",node,p->geoLodLevel);
	}
	#endif

	/* if we are out of range, use the rootNode or rootUrl field 	*/
	/* else, use the child1Url through the child4Url fields 	*/
	if (!(node->__inRange)) {
		/* printf ("GeoLOD, node %u, doing rootNode, rootNode.n = %d\n",node,node->rootNode.n); */
		/* do we need to unload children that are no longer needed? */
		GeoUnLODchildren (node);

		if (node->rootNode.n != 0)  {
			for (i=0; i<node->rootNode.n; i++) {
				#ifdef VERBOSE
				printf ("GeoLOD %u is rendering rootNode %u",node,node->rootNode.p[i]);
				if (node->rootNode.p[i]!=NULL) printf (" (%s) ",stringNodeType(X3D_NODE(node->rootNode.p[i])->_nodeType));
				printf("\n");
				#endif

				render_node (node->rootNode.p[i]);
			}	
		} else if (node->rootUrl.n != 0) {

			/* try and load the root from the rootUrl */
			GeoLODrootUrl (node);

			/* render this rootUrl */
			if (node->__rootUrl != NULL) {
				#ifdef VERBOSE
				printf ("GeoLOD %u is rendering rootUrl %u",node,node->__rootUrl);
				if (node->__rootUrl != NULL) printf (" (%s) ", stringNodeType(X3D_NODE(node->__rootUrl)->_nodeType));
				printf ("\n");
				#endif

				render_node (node->__rootUrl);
			}	
			
			
		}
	} else {
		p->geoLodLevel++;

		/* go through 4 kids */
		GeoLODchildren (node);

		/* get rid of the rootUrl node, if it is loaded */
		GeoUnLODrootUrl (node);

		#ifdef VERBOSE
		printf ("rendering children at %d, they are: ",p->geoLodLevel);
		if (node->child1Url.n>0) printf (" :%s: ",node->child1Url.p[0]->strptr);
		if (node->child2Url.n>0) printf (" :%s: ",node->child2Url.p[0]->strptr);
		if (node->child3Url.n>0) printf (" :%s: ",node->child3Url.p[0]->strptr);
		if (node->child4Url.n>0) printf (" :%s: ",node->child4Url.p[0]->strptr);
		printf ("\n");
		#endif

		/* render these children */
		#ifdef VERBOSE
		printf ("GeoLOD %u is rendering children %u ", node, node->__child1Node);
		if (node->__child1Node != NULL) printf (" (%s) ",stringNodeType(X3D_NODE(node->__child1Node)->_nodeType));
		printf (" %u ", node->__child2Node);
		if (node->__child2Node != NULL) printf (" (%s) ",stringNodeType(X3D_NODE(node->__child2Node)->_nodeType));
		printf (" %u ", node->__child3Node);
		if (node->__child3Node != NULL) printf (" (%s) ",stringNodeType(X3D_NODE(node->__child3Node)->_nodeType));
		printf (" %u ", node->__child4Node);
		if (node->__child4Node != NULL) printf (" (%s) ",stringNodeType(X3D_NODE(node->__child4Node)->_nodeType));
		printf ("\n");
		#endif

		if (node->__child1Node != NULL) render_node (node->__child1Node);
		if (node->__child2Node != NULL) render_node (node->__child2Node);
		if (node->__child3Node != NULL) render_node (node->__child3Node);
		if (node->__child4Node != NULL) render_node (node->__child4Node);
		p->geoLodLevel--;

	}
}

/************************************************************************/
/* GeoMetaData								*/
/************************************************************************/

void compile_GeoMetadata (struct X3D_GeoMetadata * node) {
	#ifdef VERBOSE
	printf ("compiling GeoMetadata\n");

	#endif

	MARK_NODE_COMPILED
}

/************************************************************************/
/* GeoOrigin								*/
/************************************************************************/

void compile_GeoOrigin (struct X3D_GeoOrigin * node) {
	#ifdef VERBOSE
	printf ("compiling GeoOrigin\n");
	#endif

	ConsoleMessage ("compiling GeoOrigin\n"); //this doesn't get called - see line 654 in initializeGeospatial()
	/* INITIALIZE_GEOSPATIAL */
	COMPILE_GEOSYSTEM(node)
	{
		int i;
		for(i=0;i<4;i++)
			node->__rotyup.c[i] = 0.0;
		node->__rotyup.c[1] = 1.0;
	}
	MARK_NODE_COMPILED

	/* events */
	/* MARK_SFNODE_INOUT_EVENT(node->metadata, node->__oldmetadata, offsetof (struct X3D_GeoOrigin, metadata)) */
	MARK_SFVEC3D_INOUT_EVENT(node->geoCoords,node->__oldgeoCoords,offsetof (struct X3D_GeoOrigin, geoCoords))
	//dug9 may 2015 commented out __old.. see also geoViewpoint
	//MARK_MFSTRING_INOUT_EVENT(node->geoSystem,node->__oldMFString,offsetof (struct X3D_GeoOrigin, geoSystem))
}

/************************************************************************/
/* GeoPositionInterpolator						*/
/************************************************************************/

void compile_GeoPositionInterpolator (struct X3D_GeoPositionInterpolator * node) {

	#ifdef VERBOSE
	printf ("compiling GeoPositionInterpolator\n");
	#endif

	if(MAR12){
		int i;
		Geosys *gs;
		struct SFVec3d gcCoord, lcsCoord;
		compile_geoSystem(X3D_NODE(node),node->_nodeType,&node->geoSystem,&node->__geoSystem);
		gs = GEOSYS(node->__geoSystem);
		FREE_IF_NZ(node->__movedValue.p);
		node->__movedValue.p = MALLOC(struct SFVec3f*,node->keyValue.n * sizeof(struct SFVec3f));
		node->__movedValue.n = node->keyValue.n;
		for(i=0;i<node->keyValue.n;i++){
			user2gc(gs,&node->keyValue.p[i],1,&gcCoord);
			gc2lcs(gs,&gcCoord,1,&lcsCoord);
			double2float(node->__movedValue.p[i].c,lcsCoord.c,3);
		}
		MARK_NODE_COMPILED
	}else{
		MF_SF_TEMPS
		/* standard MACROS expect specific field names */

		mIN = node->keyValue;
		mOUT.p = NULL; mOUT.n = 0;

		INITIALIZE_GEOSPATIAL(node)
		COMPILE_GEOSYSTEM(node)
		MOVE_TO_ORIGIN(node)
		FREE_IF_NZ(node->__movedValue.p);
		double2float(node->__movedValue.p[0].c,mOUT.p[0].c,3*node->__movedValue.n);
		node->__movedValue.n = mOUT.n;

		FREE_IF_NZ(gdCoords.p);
		MARK_NODE_COMPILED
	}
	

	/* events */
	/* MARK_SFNODE_INOUT_EVENT(node->metadata, node->__oldmetadata, offsetof (struct X3D_GeoPositionInterpolator, metadata)) */
}

/* PositionInterpolator, ColorInterpolator, GeoPositionInterpolator	*/
/* Called during the "events_processed" section of the event loop,	*/
/* so this is called ONLY when there is something required to do, thus	*/
/* there is no need to look at whether it is active or not		*/

/* GeoPositionInterpolator == PositionIterpolator but with geovalue_changed and coordinate conversions */
// http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/geodata.html#GeoPositionInterpolator
// Mar 2018 interpretation: value_changed in LCS, geovalue_changed in geo coords
// I think we should either do 2 separate interpolations: 1) LCS 2) user geocoords
// or interpolate in geocoords and convert the resulting geovalue_changed to LCS for value_changed
void do_GeoPositionInterpolator (void *innode) {
	int specversion;
	struct X3D_GeoPositionInterpolator *node;
	int kin, kvin, counter, tmp;
	struct SFVec3d *kVs_user; //geo
	struct SFVec3f *kVs_lcs; //LCS
	/* struct SFColor *kVs */

	if (!innode) return;
	node = (struct X3D_GeoPositionInterpolator *) innode;

	if (NODE_NEEDS_COMPILING) compile_GeoPositionInterpolator(node);
	kvin = node->__movedValue.n;
	kVs_lcs = node->__movedValue.p;
	kVs_user = node->keyValue.p;
	kin = node->key.n;
	MARK_EVENT (innode, offsetof (struct X3D_GeoPositionInterpolator, value_changed)); 
	MARK_EVENT (innode, offsetof (struct X3D_GeoPositionInterpolator, geovalue_changed)); 

	/* did the key or keyValue change? */
	if (node->__oldKeyValuePtr.p != node->keyValue.p) {
		MARK_EVENT (innode, offsetof (struct X3D_GeoPositionInterpolator, keyValue)); 
		node->__oldKeyValuePtr.p= node->keyValue.p;
	}
	if (node->__oldKeyPtr.p != node->key.p) {
		MARK_EVENT (innode, offsetof (struct X3D_GeoPositionInterpolator, key)); 
		node->__oldKeyPtr.p = node->key.p;
	}


	#ifdef SEVERBOSE
		printf("do_GeoPos: Position/Color interp, node %u kin %d kvin %d set_fraction %f\n",
			   node, kin, kvin, node->set_fraction);
	#endif

	/* make sure we have the keys and keyValues */
	if ((kvin == 0) || (kin == 0)) {
		node->value_changed.c[0] = (float) 0.0;
		node->value_changed.c[1] = (float) 0.0;
		node->value_changed.c[2] = (float) 0.0;
		node->geovalue_changed.c[0] = 0.0;
		node->geovalue_changed.c[1] = 0.0;
		node->geovalue_changed.c[2] = 0.0;
		return;
	}

	if (kin>kvin) kin=kvin; /* means we don't use whole of keyValue, but... */

	/* set_fraction less than or greater than keys */
	if (node->set_fraction <= ((node->key).p[0])) {
		veccopyd(node->geovalue_changed.c,kVs_user[0].c);
		veccopy3f(node->value_changed.c,kVs_lcs[0].c);
	} else if (node->set_fraction >= node->key.p[kin-1]) {
		memcpy ((void *)&node->geovalue_changed, (void *)&kVs_user[kvin-1], sizeof (struct SFVec3d));
		veccopyd(node->geovalue_changed.c,kVs_user[kvin-1].c);
		veccopy3f(node->value_changed.c,kVs_lcs[kvin-1].c);
	} else {
		/* have to go through and find the key before */
		float fpart, fdif[3];
		double dpart, ddif[3];
		counter = find_key(kin,((float)(node->set_fraction)),node->key.p);

		//LCS to value_changed
		fpart = (node->set_fraction - node->key.p[counter-1]) /
				(node->key.p[counter] - node->key.p[counter-1]);
		veclerp3f(node->value_changed.c,kVs_lcs[counter-1].c,kVs_lcs[counter].c,fpart);

		//geo to geovalue_changed
		dpart = fpart;
		veclerpd(node->geovalue_changed.c,kVs_user[counter-1].c,kVs_user[counter].c,dpart);

		//for (tmp=0; tmp<3; tmp++) {
		//	node->geovalue_changed.c[tmp] =
		//		(node->set_fraction - node->key.p[counter-1]) /
		//		(node->key.p[counter] - node->key.p[counter-1]) *
		//		(kVs[counter].c[tmp] - kVs[counter-1].c[tmp]) + kVs[counter-1].c[tmp];
		//}

	}

	/* convert this back into the requested spatial format */
	//CONVERT_BACK_TO_GD_OR_UTM(node->geovalue_changed)
	//CONVERT_BACK_TO_GD_OR_UTMB(GEOSYS(node->__geoSystem), node->geoOrigin, &node->geovalue_changed);
	///* set the (float) value_changed, as well */
	//for (tmp=0;tmp<3;tmp++) node->value_changed.c[tmp] = (float)node->geovalue_changed.c[tmp];

	#ifdef SEVERBOSE
	printf ("Pos/Col, new value (%f %f %f)\n",
		node->value_changed.c[0],node->value_changed.c[1],node->value_changed.c[2]);
	printf ("geovalue_changed %lf %lf %lf\n",node->geovalue_changed.c[0], node->geovalue_changed.c[1], node->geovalue_changed.c[2]);
	#endif
}

/************************************************************************/
/* GeoProximitySensor							*/
/************************************************************************/

void compile_GeoProximitySensor (struct X3D_GeoProximitySensor * node) {
	int specversion;
	MF_SF_TEMPS

	#ifdef VERBOSE
	printf ("compiling GeoProximitySensor\n");
	#endif
	if(MAR12){
		int i;
		Geosys *gs;
		struct SFVec3d gcCoord, lcsCoord, gdCoord;
		compile_geoSystem(X3D_NODE(node),node->_nodeType,&node->geoSystem,&node->__geoSystem);
		gs = GEOSYS(node->__geoSystem);
		user2gc(gs,&node->geoCenter,1,&gcCoord);
		gc2lcs(gs,&gcCoord,1,&node->__movedCoords);
		gc2gd(gs,&gcCoord,1,&gdCoord);
		GeoOrient(node->geoOrigin, GEOSYS(node->__geoSystem), &gdCoord, &node->__localOrient);
		MARK_NODE_COMPILED
	}else{
		/* work out the position */
		INITIALIZE_GEOSPATIAL(node)
		COMPILE_GEOSYSTEM(node)
		INIT_MF_FROM_SF(node, geoCenter)
		MOVE_TO_ORIGIN(node)
		COPY_MF_TO_SF(node, __movedCoords)

		/* work out the local orientation */
		specversion = X3D_PROTO(node->_executionContext)->__specversion;
		GeoOrient(node->geoOrigin, GEOSYS(node->__geoSystem), &gdCoords.p[0], &node->__localOrient);
		#ifdef VERBOSE
		printf ("compile_GeoProximitySensor, orig coords %lf %lf %lf, moved %lf %lf %lf\n", node->geoCenter.c[0], node->geoCenter.c[1], node->geoCenter.c[2], node->__movedCoords.c[0], node->__movedCoords.c[1], node->__movedCoords.c[2]);
		printf ("	rotation is %lf %lf %lf %lf\n",
				node->__localOrient.c[0],
				node->__localOrient.c[1],
				node->__localOrient.c[2],
				node->__localOrient.c[3]);
		#endif

		MARK_NODE_COMPILED
		FREE_MF_SF_TEMPS
	}
	MARK_SFVEC3D_INOUT_EVENT(node->geoCenter, node->__oldGeoCenter,offsetof (struct X3D_GeoProximitySensor, geoCenter))
	MARK_SFVEC3F_INOUT_EVENT(node->size, node->__oldSize,offsetof (struct X3D_GeoProximitySensor, size))
	
	/* events */
	/* MARK_SFNODE_INOUT_EVENT(node->metadata, node->__oldmetadata, offsetof (struct X3D_GeoProximitySensor, metadata)) */


	#ifdef VERBOSE
	printf ("compiled GeoProximitySensor\n\n");
	#endif
}

	//PROXIMITYSENSOR(GeoProximitySensor,__movedCoords,INITIALIZE_GEOSPATIAL(node),COMPILE_IF_REQUIRED)
//#define PROXIMITYSENSOR(type,center,initializer1,initializer2) 
void render_GeoProximitySensor(struct X3D_GeoProximitySensor *node){
	//just for rendering the extent/bounding box
	if(renderstate()->render_boxes) 
		extent6f_draw(node->_extent);
}
void proximity_GeoProximitySensor (struct X3D_GeoProximitySensor *node) { 
	/* Viewer pos = t_r2 */ 
	double cx,cy,cz; 
	double len; 
	struct point_XYZ dr1r2; 
	struct point_XYZ dr2r3; 
	struct point_XYZ nor1,nor2; 
	struct point_XYZ ins; 
	static struct point_XYZ yvec = {0,0.05,0}; 
	static struct point_XYZ zvec = {0,0,-0.05}; 
	static struct point_XYZ zpvec = {0,0,0.05}; 
	static struct point_XYZ orig = {0,0,0};
	struct point_XYZ t_zvec, t_yvec, t_orig, t_center; 
	GLDOUBLE modelMatrix[16]; 
	GLDOUBLE projMatrix[16]; 
	GLDOUBLE view2prox[16]; 
 
	if(!((node->enabled))) return; 
	//INITIALIZE_GEOSPATIAL(node) 
	COMPILE_IF_REQUIRED 
 
	/* printf (" vp %d geom %d light %d sens %d blend %d prox %d col %d\n",*/ 
	/* render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision);*/ 
 
	/* transforms viewers coordinate space into sensors coordinate space. 
	 * this gives the orientation of the viewer relative to the sensor. 
	 */ 
	FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, modelMatrix); 
	if(0){
		FW_GL_GETDOUBLEV(GL_PROJECTION_MATRIX, projMatrix); 
		FW_GLU_UNPROJECT(orig.x,orig.y,orig.z,modelMatrix,projMatrix,viewport, 
			&t_orig.x,&t_orig.y,&t_orig.z); 
		FW_GLU_UNPROJECT(zvec.x,zvec.y,zvec.z,modelMatrix,projMatrix,viewport, 
			&t_zvec.x,&t_zvec.y,&t_zvec.z); 
		FW_GLU_UNPROJECT(yvec.x,yvec.y,yvec.z,modelMatrix,projMatrix,viewport, 
			&t_yvec.x,&t_yvec.y,&t_yvec.z); 
		VECDIFF(t_zvec, t_orig, dr1r2);  /* Z axis */
		VECDIFF(t_yvec, t_orig, dr2r3);  /* Y axis */

	}
	matinverseAFFINE(view2prox,modelMatrix); 
	if(1){
		//feature-AFFINE_GLU_UNPROJECT
		transform(&t_orig,&orig,view2prox);
		transform(&zvec,&zvec,view2prox);
		transform(&yvec,&yvec,view2prox);
		VECDIFF(zvec, t_orig, dr1r2);
		VECDIFF(yvec, t_orig, dr2r3);
	}
    transform(&t_center,&orig, view2prox); 
 
 
	/*printf ("\n"); 
	printf ("unprojected, t_orig (0,0,0) %lf %lf %lf\n",t_orig.x, t_orig.y, t_orig.z); 
	printf ("unprojected, t_yvec (0,0.05,0) %lf %lf %lf\n",t_yvec.x, t_yvec.y, t_yvec.z); 
	printf ("unprojected, t_zvec (0,0,-0.05) %lf %lf %lf\n",t_zvec.x, t_zvec.y, t_zvec.z); 
	*/ 
	cx = t_center.x - ((node->__movedCoords ).c[0]); 
	cy = t_center.y - ((node->__movedCoords ).c[1]); 
	cz = t_center.z - ((node->__movedCoords ).c[2]); 
 
	{
		float cc[3];
		//how draw bounding box? doesn't seem to draw on proximity pass
		// H: you need a render_proximity
		vecscale3f(cc,node->size.c,.5);
		extent6f_constructor(node->_extent,-cc[0],cc[0],-cc[1],cc[1],-cc[2],cc[2]);
		//if(renderstate()->render_boxes) extent6f_draw(node->_extent);
	}
	if(((node->size).c[0]) == 0 || ((node->size).c[1]) == 0 || ((node->size).c[2]) == 0) return; 
 
	if(fabs(cx) > ((node->size).c[0])/2 || 
	   fabs(cy) > ((node->size).c[1])/2 || 
	   fabs(cz) > ((node->size).c[2])/2) return; 
	/* printf ("within (Geo)ProximitySensor\n"); */ 
 
	/* Ok, we now have to compute... */ 
	(node->__hit) /*cget*/ = 1; 
 
	/* Position */ 
	((node->__t1).c[0]) = (float)t_center.x; 
	((node->__t1).c[1]) = (float)t_center.y; 
	((node->__t1).c[2]) = (float)t_center.z; 
 
 
	if(MAR12){
		Quaternion quat;
		double oo[4];
		matrix_to_quaternion(&quat,modelMatrix);
		quaternion_normalize(&quat);
		quaternion_to_vrmlrot(&quat,&oo[0],&oo[1],&oo[2],&oo[3]);
		vecnormald(oo,oo);
		double2float(node->__t2.c,oo,4);
	}else{
		/* printf ("      dr1r2 %lf %lf %lf\n",dr1r2.x, dr1r2.y, dr1r2.z); 
		printf ("      dr2r3 %lf %lf %lf\n",dr2r3.x, dr2r3.y, dr2r3.z); 
		*/ 
 
		len = sqrt(VECSQ(dr1r2)); VECSCALE(dr1r2,1/len); 
		len = sqrt(VECSQ(dr2r3)); VECSCALE(dr2r3,1/len); 
 
		/* printf ("scaled dr1r2 %lf %lf %lf\n",dr1r2.x, dr1r2.y, dr1r2.z); 
		printf ("scaled dr2r3 %lf %lf %lf\n",dr2r3.x, dr2r3.y, dr2r3.z); 
		*/ 
 
		/* 
		printf("PROX_INT: (%f %f %f) (%f %f %f) (%f %f %f)\n (%f %f %f) (%f %f %f)\n", 
			t_orig.x, t_orig.y, t_orig.z, 
			t_zvec.x, t_zvec.y, t_zvec.z, 
			t_yvec.x, t_yvec.y, t_yvec.z, 
			dr1r2.x, dr1r2.y, dr1r2.z, 
			dr2r3.x, dr2r3.y, dr2r3.z 
			); 
		*/ 
 
		if(fabs(VECPT(dr1r2, dr2r3)) > 0.001) { 
			printf ("Sorry, can't handle unevenly scaled GeoProximitySensors yet :(" 
			  "dp: %f v: (%f %f %f) (%f %f %f)\n", VECPT(dr1r2, dr2r3), 
		  		dr1r2.x,dr1r2.y,dr1r2.z, 
		  		dr2r3.x,dr2r3.y,dr2r3.z 
				); 
			return; 
		} 
 
 
		if(APPROX(dr1r2.z,1.0)) { 
			/* rotation */ 
			((node->__t2).c[0]) = (float) 0; 
			((node->__t2).c[1]) = (float) 0; 
			((node->__t2).c[2]) = (float) 1; 
			((node->__t2).c[3]) = (float) atan2(-dr2r3.x,dr2r3.y); 
		} else if(APPROX(dr2r3.y,1.0)) { 
			/* rotation */ 
			((node->__t2).c[0]) = (float) 0; 
			((node->__t2).c[1]) = (float) 1; 
			((node->__t2).c[2]) = (float) 0; 
			((node->__t2).c[3]) = (float) atan2(dr1r2.x,dr1r2.z); 
		} else { 
			/* Get the normal vectors of the possible rotation planes */ 
			nor1 = dr1r2; 
			nor1.z -= 1.0; 
			nor2 = dr2r3; 
			nor2.y -= 1.0; 
 
			/* Now, the intersection of the planes, obviously cp */ 
			VECCP(nor1,nor2,ins); 
 
			len = sqrt(VECSQ(ins)); VECSCALE(ins,1/len); 
 
			/* the angle */ 
			VECCP(dr1r2,ins, nor1);
			VECCP(zpvec, ins, nor2); 
			len = sqrt(VECSQ(nor1)); VECSCALE(nor1,1/len); 
			len = sqrt(VECSQ(nor2)); VECSCALE(nor2,1/len); 
			VECCP(nor1,nor2,ins); 
 
			((node->__t2).c[3]) = (float) -atan2(sqrt(VECSQ(ins)), VECPT(nor1,nor2)); 
 
			/* rotation  - should normalize sometime... */ 
			((node->__t2).c[0]) = (float) ins.x; 
			((node->__t2).c[1]) = (float) ins.y; 
			((node->__t2).c[2]) = (float) ins.z; 
		} 
		/* 
		printf("NORS: (%f %f %f) (%f %f %f) (%f %f %f)\n", 
			nor1.x, nor1.y, nor1.z, 
			nor2.x, nor2.y, nor2.z, 
			ins.x, ins.y, ins.z 
		); 
		*/ 
	}
} 


/* GeoProximitySensor code for ClockTick */
void do_GeoProximitySensorTick( void *ptr) {
	int specversion;
	struct X3D_GeoProximitySensor *node = (struct X3D_GeoProximitySensor *)ptr;

	/* if not enabled, do nothing */
	if (!node) return;
	if (node->__oldEnabled != node->enabled) {
		node->__oldEnabled = node->enabled;
		MARK_EVENT(X3D_NODE(node),offsetof (struct X3D_GeoProximitySensor, enabled));
	}
	if (!node->enabled) return;

	/* isOver state */
	/* did we get a signal? */
	if (node->__hit) {
		if (!node->isActive) {
			#ifdef SEVERBOSE
			printf ("PROX - initial defaults\n");
			#endif

			node->isActive = 1;
			node->enterTime = TickTime();
			MARK_EVENT (ptr, offsetof(struct X3D_GeoProximitySensor, isActive));
			MARK_EVENT (ptr, offsetof(struct X3D_GeoProximitySensor, enterTime));

		}

		/* now, has anything changed? */
		if (memcmp ((void *) &node->position_changed,(void *) &node->__t1,sizeof(struct SFColor))) {
			#ifdef SEVERBOSE
			printf ("PROX - position changed!!! \n");
			#endif

			memcpy ((void *) &node->position_changed,
				(void *) &node->__t1,sizeof(struct SFColor));
			MARK_EVENT (ptr, offsetof(struct X3D_GeoProximitySensor, position_changed));
		
			#ifdef VERBOSE
			printf ("do_GeoProximitySensorTick, position changed; it now is %lf %lf %lf\n",node->position_changed.c[0],
				node->position_changed.c[1], node->position_changed.c[2]);
			printf ("nearPlane is %lf\n",Viewer.nearPlane);

			#endif

			if(MAR12){
				ttglobal tg = gglobal();
				struct X3D_Node *boundvp = vector_back(struct X3D_Node*,getActiveBindableStacks(tg)->viewpoint);
		
				if(boundvp && boundvp->_nodeType == NODE_GeoViewpoint){
					struct SFVec3d gcCoord, geoCoord;
					struct X3D_GeoViewpoint *gvp = (struct X3D_GeoViewpoint *)boundvp;
					gd2gc(GEOSYS(gvp->__geoSystem),&gvp->__movedgd,1,&gcCoord);
					gc2user(GEOSYS(node->__geoSystem),&gcCoord,1,&geoCoord);
					veccopyd(node->geoCoord_changed.c,geoCoord.c);
				}else{
					//lcs2gc()
				}

				MARK_EVENT (ptr, offsetof(struct X3D_GeoProximitySensor, geoCoord_changed));
				
			}else{
				/* possibly we have to convert this from GCC to GDC, and maybe even then to UTM */
		
				/* prep the geoCoord changed; first, get the position. Right now, we use the
				  Viewer position, as it is more accurate (not clipped by the nearPlane) than
				  the position_changed field  */

				node->geoCoord_changed.c[0] = (double) node->position_changed.c[0];
				node->geoCoord_changed.c[1] = (double) node->position_changed.c[1];
				node->geoCoord_changed.c[2] = (double) node->position_changed.c[2];
				vecprint3fb("pos",node->position_changed.c,"\n");
				vecprint3db("geo",node->geoCoord_changed.c,"\n");
				/* then add in the nearPlane, as the way we get the position is via a clipped frustum */
				/* if we get this via the position_changed field, we have to:
					node->geoCoord_changed.c[2] += Viewer.nearPlane;
				*/
				//node->geoCoord_changed.c[2] += Viewer()->nearPlane;
				MARK_EVENT (ptr, offsetof(struct X3D_GeoProximitySensor, geoCoord_changed));

				#ifdef VERBOSE
				printf ("\ngeoCoord_changed as a GCC, %lf %lf %lf\n",
					node->geoCoord_changed.c[0],
					node->geoCoord_changed.c[1],
					node->geoCoord_changed.c[2]);
				#endif

				//CONVERT_BACK_TO_GD_OR_UTM(node->geoCoord_changed)
				CONVERT_BACK_TO_GD_OR_UTMB(GEOSYS(node->__geoSystem), node->geoOrigin, &node->geoCoord_changed);
		}	}
		if (memcmp ((void *) &node->orientation_changed, (void *) &node->__t2,sizeof(struct SFRotation))) {
			#ifdef SEVERBOSE
			printf  ("PROX - orientation changed!!!\n ");
			#endif

			memcpy ((void *) &node->orientation_changed,
				(void *) &node->__t2,sizeof(struct SFRotation));
			MARK_EVENT (ptr, offsetof(struct X3D_GeoProximitySensor, orientation_changed));
		}
	} else {
		if (node->isActive) {
			#ifdef SEVERBOSE
			printf ("PROX - stopping\n");
			#endif

			node->isActive = 0;
			node->exitTime = TickTime();
			MARK_EVENT (ptr, offsetof(struct X3D_GeoProximitySensor, isActive));

			MARK_EVENT (ptr, offsetof(struct X3D_GeoProximitySensor, exitTime));
		}
	}
	node->__hit=FALSE;
}


/************************************************************************/
/* GeoTouchSensor							*/
/************************************************************************/

void compile_GeoTouchSensor (struct X3D_GeoTouchSensor * node) {
	#ifdef VERBOSE
	printf ("compiling GeoTouchSensor\n");
	#endif
	if(MAR12){
		compile_geoSystem(X3D_NODE(node),node->_nodeType,&node->geoSystem,&node->__geoSystem);
		MARK_NODE_COMPILED
	}else{
		INITIALIZE_GEOSPATIAL(node)
		COMPILE_GEOSYSTEM(node)
		MARK_NODE_COMPILED
	}

	/* events */
	/* MARK_SFNODE_INOUT_EVENT(node->metadata, node->__oldmetadata, offsetof (struct X3D_GeoTouchSensor, metadata)) */

}

void do_GeoTouchSensor ( void *ptr, int ev, int but1, int over) {

	int specversion;
	struct X3D_GeoTouchSensor *node = (struct X3D_GeoTouchSensor *)ptr;
	struct point_XYZ normalval;	/* different structures for normalization calls */
	ttglobal tg;
	COMPILE_IF_REQUIRED

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
		MARK_EVENT(X3D_NODE(node),offsetof (struct X3D_GeoTouchSensor, enabled));
	}
	if (!node->enabled) return;
	tg = gglobal();
	/* isOver state */
	if ((ev == overMark) && (over != node->isOver)) {
		#ifdef SENSVERBOSE
		printf ("TS %u, isOver changed %d\n",node, over);
		#endif
		node->isOver = over;
		MARK_EVENT (ptr, offsetof (struct X3D_GeoTouchSensor, isOver));
	}

	/* active */
	/* button presses */
	if (ev == ButtonPress) {
		node->isActive=1;
		MARK_EVENT (ptr, offsetof (struct X3D_GeoTouchSensor, isActive));
		#ifdef SENSVERBOSE
		printf ("touchSens %u, butPress\n",node);
		#endif

		node->touchTime = TickTime();
		MARK_EVENT(ptr, offsetof (struct X3D_GeoTouchSensor, touchTime));

	} else if (ev == ButtonRelease) {
		#ifdef SENSVERBOSE
		printf ("touchSens %u, butRelease\n",node);
		#endif
		node->isActive=0;
		MARK_EVENT (ptr, offsetof (struct X3D_GeoTouchSensor, isActive));
	}

	/* hitPoint and hitNormal */
	/* save the current hitPoint for determining if this changes between runs */
	memcpy ((void *) &node->_oldhitPoint, (void *) &tg->RenderFuncs.ray_save_posn,sizeof(struct SFColor));

	/* did the hitPoint change between runs? */
	if ((APPROX(node->_oldhitPoint.c[0],node->hitPoint_changed.c[0])!= TRUE) ||
		(APPROX(node->_oldhitPoint.c[1],node->hitPoint_changed.c[1])!= TRUE) ||
		(APPROX(node->_oldhitPoint.c[2],node->hitPoint_changed.c[2])!= TRUE)) {

		#ifdef SENSVERBOSE
		printf ("GeoTouchSens, hitPoint changed: %f %f %f\n",node->hitPoint_changed.c[0],
			node->hitPoint_changed.c[1], node->hitPoint_changed.c[2]);
		#endif

		memcpy ((void *) &node->hitPoint_changed, (void *) &node->_oldhitPoint, sizeof(struct SFColor));
		vecprint3fb("hitpoint",node->hitPoint_changed.c,"\n");
		MARK_EVENT(ptr, offsetof (struct X3D_GeoTouchSensor, hitPoint_changed));

		/* convert this back into the requested GeoSpatial format... */
		node->hitGeoCoord_changed.c[0] = (double) node->hitPoint_changed.c[0];
		node->hitGeoCoord_changed.c[1] = (double) node->hitPoint_changed.c[1];
		node->hitGeoCoord_changed.c[2] = (double) node->hitPoint_changed.c[2];

		/* then add in the nearPlane, as the way we get the position is via a clipped frustum */
		/* if we get this via the position_changed field, we have to:
			node->hitGeoCoord_changed.c[2] += nearPlane;
		*/
		if(!MAR12){
			node->hitGeoCoord_changed.c[2] += Viewer()->nearPlane;
		}
		MARK_EVENT (ptr, offsetof(struct X3D_GeoTouchSensor, hitGeoCoord_changed));

		#ifdef SENSVERBOSE
		printf ("\nhitGeoCoord_changed as a GCC, %lf %lf %lf\n",
			node->hitGeoCoord_changed.c[0],
			node->hitGeoCoord_changed.c[1],
			node->hitGeoCoord_changed.c[2]);
		#endif
		if(MAR12){
			struct SFVec3d gcCoord;
			Geosys *gs = GEOSYS(node->__geoSystem);
			lcs2gc(gs,&node->hitGeoCoord_changed,1,&gcCoord);
			gc2user(gs,&gcCoord,1,&node->hitGeoCoord_changed);
			//vecprint3db("user",node->hitGeoCoord_changed.c,"\n");
		}else{
			//CONVERT_BACK_TO_GD_OR_UTM(node->hitGeoCoord_changed)
			CONVERT_BACK_TO_GD_OR_UTMB(GEOSYS(node->__geoSystem), node->geoOrigin, &node->hitGeoCoord_changed);
		}
	}

	/* have to normalize normal; change it from SFColor to struct point_XYZ. */
	normalval.x = tg->RenderFuncs.hyp_save_norm[0];
	normalval.y = tg->RenderFuncs.hyp_save_norm[1];
	normalval.z = tg->RenderFuncs.hyp_save_norm[2];
	normalize_vector(&normalval);
	node->_oldhitNormal.c[0] = (float) normalval.x;
	node->_oldhitNormal.c[1] = (float) normalval.y;
	node->_oldhitNormal.c[2] = (float) normalval.z;

	/* did the hitNormal change between runs? */
	MARK_SFVEC3F_INOUT_EVENT(node->hitNormal_changed,node->_oldhitNormal,offsetof (struct X3D_GeoTouchSensor, hitNormal_changed))
} 



/************************************************************************/
/* GeoViewpoint								*/
/************************************************************************/
void calculateViewingSpeedB();

void compile_GeoViewpoint (struct X3D_GeoViewpoint * node) {
	int specversion;
	struct SFVec4d localOrient, offsetOrient;
	struct SFVec4d orient;
	int i;
	Quaternion localQuat;
	Quaternion relQuat;
	Quaternion combQuat;
	struct SFVec3d gdCoord, gcCoord;
	struct SFVec3d offset, *poffset;
	struct SFVec4d yup, *pyup;

	#ifdef VERBOSE
	printf ("compileViewpoint is %u, its geoOrigin is %u \n",node, node->geoOrigin);
	if (node->geoOrigin!=NULL) printf ("type %s\n",stringNodeType(X3D_GEOORIGIN(node->geoOrigin)->_nodeType));
	#endif

	specversion = X3D_PROTO(node->_executionContext)->__specversion;

	// v3.3 regular fields are [inout] now /* did any of the "set_" inputOnly fields get set?  if not, just use the non-set fields */
	//USE_SET_SFVEC3D_IF_CHANGED(set_position,position)
	//USE_SET_SFROTATION_IF_CHANGED(set_orientation,orientation)  

	compile_geoSystem (X3D_NODE(node),node->_nodeType, &node->geoSystem, &node->__geoSystem);


	//struct SFVec3d *gcCoord;      //-GC2NL
	//struct SFVec3d *offsetCoord;  //-NL2SL
	//struct SFVec4d *localOrient;  //-GCA2NLA
	//struct SFVec4d *offsetOrient; //-NLA2SLA


	geoOffsetInfo ggi, *gi;
	struct Planet *planet;
	//ppComponent_Geospatial p = (ppComponent_Geospatial)gglobal()->Component_Geospatial.prv;

	planet = current_planet();
	gi = &ggi;
	gi->node = X3D_NODE(node);
	gi->geoOrigin = X3D_GEOORIGIN(node->geoOrigin);
	gi->geoSystem = GEOSYS(node->__geoSystem);
	gi->position = &node->position;
	gi->offsetCoord = &node->__movedPosition;
	gi->localOrient = &localOrient;
	gi->offsetOrient = &offsetOrient;
	gi->gdCoord = &gdCoord;
	gi->gcCoord = &gcCoord;
	printf("GVP:\n");
	origin_offsets(gi);
	veccopy4d(localOrient.c,planet->autoOrient.c);

	/* work out the local orientation and copy doubles to floats */
	veccopyd(node->__movedgd.c,gdCoord.c);

	double2float(node->__movedOrientation.c,offsetOrient.c,4);
	double2float(node->__movedOrientationB.c,localOrient.c,4);

	//we need to initialize __movedgd (lat, lon, height) early for things like speed
	moveCoords3d(GEOSYS(node->__geoSystem),NULL,NULL,&node->position,1,&gcCoord,&node->__movedgd);

        #ifdef VERBOSE
	printf ("compile_GeoViewpoint, final position %lf %lf %lf\n",node->__movedPosition.c[0],
		node->__movedPosition.c[1], node->__movedPosition.c[2]);

	printf ("compile_GeoViewpoint, getLocalOrientation %lf %lf %lf %lf\n",localOrient.c[0],
		localOrient.c[1], localOrient.c[2], localOrient.c[3]);
	printf ("compile_GeoViewpoint, initial orientation: %lf %lf %lf %lf\n",node->orientation.c[0],
		node->orientation.c[1], node->orientation.c[2], node->orientation.c[3]);
	printf ("compile_GeoViewpoint, final rotation %lf %lf %lf %lf\n",node->__movedOrientation.c[0], 
		node->__movedOrientation.c[1], node->__movedOrientation.c[2], node->__movedOrientation.c[3]);
	printf ("compile_GeoViewpoint, elevation from the WGS84 ellipsoid is %lf\n",gdCoords.p[0].c[2]);
        #endif

	MARK_NODE_COMPILED
	
	/* events */
	/* MARK_SFNODE_INOUT_EVENT(node->metadata, node->__oldmetadata, offsetof (struct X3D_GeoViewpoint, metadata)) */
	MARK_SFFLOAT_INOUT_EVENT(node->fieldOfView, node->__oldFieldOfView, offsetof (struct X3D_GeoViewpoint, fieldOfView))
	MARK_SFBOOL_INOUT_EVENT(node->headlight, node->__oldHeadlight, offsetof (struct X3D_GeoViewpoint, headlight))
	MARK_SFBOOL_INOUT_EVENT(node->jump, node->__oldJump, offsetof (struct X3D_GeoViewpoint, jump))
	/* 
	//dug9 may 2015 I'm not sure what the __old stuff was for (H: debugging) but 
	//shallow copying a string pointer -or MFString or SFString- makes it hard to generically free during exit
	//see opengl_utils.c in cbFreeMallocedBuiltinField 
	MARK_SFSTRING_INOUT_EVENT(node->description,node->__oldSFString, offsetof(struct X3D_GeoViewpoint, description))
	MARK_MFSTRING_INOUT_EVENT(node->navType,node->__oldMFString, offsetof(struct X3D_GeoViewpoint, navType))
	*/
	#ifdef VERBOSE
	printf ("compiled GeoViewpoint\n\n");
	#endif
}
struct X3D_Node *getActiveLayerBoundViewpoint();
void CONVERT_BACK_TO_GD_OR_UTMC(Geosys *targetGeoSystem, struct X3D_Node *geoorigin, 
		struct SFVec3d *LCSpos, struct SFVec3d *gdCoords, struct SFVec3d *thisField);
void geoviewpoint_update_user_offsets(struct X3D_GeoViewpoint *node, Quaternion *Quat, struct point_XYZ *Pos){
	//Theory of operation:
	// NLA - node local alignment
	// we use viewer as a 3D pointing device relative to our GVP node's local coordinate system
	//  (-Z to north pole, X east, Y up) at GVP

	struct SFVec3d GCpos, gdCoord;
	Quaternion qlc2gc;
	double oo[4], pp[3];
	//ppComponent_Geospatial p = (ppComponent_Geospatial)gglobal()->Component_Geospatial.prv;



	//1. update geo position
	//1.a recall GC at last fetch
	moveCoords3d(GEOSYS(node->__geoSystem),NULL,NULL,&node->position,1,&GCpos,&node->__movedgd);
	//1.a.0. save last gdCoord for azimuth correction
	gdCoord = node->__movedgd;

	Quaternion qlo, q2;
	struct SFVec4d lo;

	//0. skip if its rounding noise
	pointxyz2double(pp,Pos);

	//1.b GC += inverse(localOrient) x Pos
	GeoOrient(X3D_NODE(node->geoOrigin), GEOSYS(node->__geoSystem), &node->__movedgd, &lo);
	vrmlrot_to_quaternion(&qlo,lo.c[0],lo.c[1],lo.c[2], lo.c[3]);
	//if(0) vecscaled(pp,pp,node->speedFactor); //SPEED scale here? no done in calculateViewingSpeedB
	quaternion_rotationd(pp,&qlo,pp);
	vecaddd(GCpos.c,GCpos.c,pp);
	//1.c .position = GC_to_user_geo(GC)
	CONVERT_BACK_TO_GD_OR_UTMC(GEOSYS(node->__geoSystem), node->geoOrigin, &GCpos, &node->__movedgd, &node->position);
	MARK_EVENT(X3D_NODE(node),offsetof(struct X3D_GeoViewpoint,position));
	//2. update .orientation that's also in GVP NLA
	//2.a comput aziumth correction dAzimuth = sin(latitude) x (Longitude2 - Longitude1)
	//     or dA = sin(phi)*dlambda
	double deltagd[3], gd[3];
	vecdifd(deltagd,node->__movedgd.c,gdCoord.c);
	veccopyd(gd,node->__movedgd.c);
	//5:	GD:     if "latitude_first" TRUE, if "longitude_first", FALSE 
	//7:	GD: TRUE: decimal degrees, FALSE radians
	if(!GEOSYS(node->__geoSystem)->gd_latitude_first){
		//get latitude first
		vecswizzle2d(deltagd); 
		vecswizzle2d(gd);
	}
	if(GEOSYS(node->__geoSystem)->gd_degrees) {
		//get radians
		vecscale2d(deltagd,deltagd,RADIANS_PER_DEGREE);
		vecscale2d(gd,gd,RADIANS_PER_DEGREE);
	}
	double dazimuth, dlambda;
	Quaternion qaz, qq;
	//as we cross the mid-pacific time zone (PI from grenwich)
	// our longitude goes from -PI to +PI. 
	// For azimuth correction we want the incremental/acute longitude difference
	dlambda = angleNormalized(deltagd[1]); 
	//if(fabs(gd[0]) > 30.0*RADIANS_PER_DEGREE){
		dazimuth = sin(gd[0])*dlambda;
		vrmlrot_to_quaternion(&qaz,0.0,1.0,0.0,dazimuth);
		quaternion_multiply(&qq,Quat,&qaz);
	//}else{
	//	qq = *Quat;
	//}
	quaternion_to_vrmlrot(&qq,&oo[0],&oo[1],&oo[2],&oo[3]);
	oo[3] = -oo[3];
	double2float(node->orientation.c,oo,4);
	MARK_EVENT(X3D_NODE(node),offsetof(struct X3D_GeoViewpoint,orientation));

}
void geoviewpoint_fetch_user_offsets(struct X3D_GeoViewpoint *node, Quaternion *Quat, struct point_XYZ *Pos){
	//Theory of operation:
	// NLA - node local alignment
	// we use viewer as a 3D pointing device relative to our GVP node's local coordinate system
	//  (-Z to north pole, X east, Y up) at GVP
	double oo[4];
	float2double(oo,node->orientation.c,4);
	vrmlrot_to_quaternion(Quat,oo[0],oo[1],oo[2], -oo[3]);
	Pos->x = Pos->y = Pos->z = 0.0;
}
void geoviewpoint_fetch_LCS(struct X3D_GeoViewpoint *node, Quaternion *Quat, struct point_XYZ *Pos){
	//returns LCS/LCA - should be similar to prep_geoViewpoint
	//
	//LCS - local coordinate system - a shared euclidean system for a planet's data
	//UCS - user coordinate system, what the scene author specifies in the scene file
	//      might be GC, GD (degrees or radians, lat or long first), XTM (UTM/3TM, easting or northing first) and w/wo geoid
	//LCA/GCA/UCA - alignment - the orientation part
	struct Planet *planet;
	struct SFVec3d LCpos;;
	struct SFVec4d lo;
	Quaternion qoo, qlo, qao, qgc;
	//ppComponent_Geospatial p = (ppComponent_Geospatial)gglobal()->Component_Geospatial.prv;
	
	double oo[4], gd[3];

	planet = current_planet();
	//step 1 convert user coordinates UCS  to LCS coordinates 
	// GC = f(UCS)   //function depends on user coordinate system
	// LCS = (GC - autoOffset) x autoOrient^
	moveCoords3d(GEOSYS(node->__geoSystem),&planet->autoOrigin,&planet->autoOrient,&node->position,1,&LCpos,&node->__movedgd);
	vecnegated(LCpos.c,LCpos.c); //like prep_viewpoint?
	double2pointxyz(Pos,LCpos.c);

	//step 2 convert user alignement UCA to local coordinate alignement LCA
	//step 2a convert UCA to GCA
	//GCA = f(UCA)
	//    = LO^ x UCA (for GD and XTM)
	float2double(oo,node->orientation.c,4);
	oo[3] = -oo[3]; //like prep_viewpoint?
	vrmlrot_to_quaternion(&qoo,oo[0],oo[1],oo[2], oo[3]);
	GeoOrient(X3D_NODE(node->geoOrigin), GEOSYS(node->__geoSystem), &node->__movedgd, &lo);
	vrmlrot_to_quaternion(&qlo,lo.c[0],lo.c[1],lo.c[2], -lo.c[3]);
	quaternion_multiply(&qgc,&qoo,&qlo);
	//step 2b convert from GCA to LCA
	//LCA = AO^ x GCA
	vrmlrot_to_quaternion(&qao,planet->autoOrient.c[0],planet->autoOrient.c[1],planet->autoOrient.c[2], -planet->autoOrient.c[3]);
	quaternion_multiply(Quat,&qgc,&qao);
	quaternion_normalize(Quat);

}

void geoviewpoint_update_LCS(struct X3D_GeoViewpoint *node, Quaternion *Quat, struct point_XYZ *Pos){
	struct Planet *planet;
	double pos[3], oo[4];
	struct SFVec3d GCpos, gdCoord;
	struct SFVec4d lo;
	Quaternion qao, qaoi, qgc, qlo, qoo;
	//ppComponent_Geospatial p = (ppComponent_Geospatial)gglobal()->Component_Geospatial.prv;
	
	planet = current_planet();
	//step 1 convert LCS to UCS
	//step 1.a converte LCS to GC
	// GC = (autoOrient x LCPos) + autoOffset
	vrmlrot_to_quaternion(&qao,planet->autoOrient.c[0],planet->autoOrient.c[1],planet->autoOrient.c[2],planet->autoOrient.c[3]);
	pointxyz2double(pos,Pos);
	vecnegated(pos,pos); //like prep_viewpoint?
	quaternion_rotationd(pos,&qao,pos);
	vecaddd(GCpos.c,planet->autoOrigin.c,pos);

	//step 1.b UCS = f(GC)
	CONVERT_BACK_TO_GD_OR_UTMC(GEOSYS(node->__geoSystem), node->geoOrigin, &GCpos, &node->__movedgd, &node->position);
	MARK_EVENT(X3D_NODE(node),offsetof(struct X3D_GeoViewpoint,position));
	//step 2 convert LCA to UCA
	//step 2a. convert LCA to GCA
	//GCA = AO x LCA
	quaternion_inverse(&qaoi,&qao);
	quaternion_multiply(&qgc,Quat,&qao);
	//step 2.b convert GCA to UCA
	// UCA = f(GCA)
	//     = LO x GCA for GD and XTM
	GeoOrient(X3D_NODE(node->geoOrigin), GEOSYS(node->__geoSystem), &node->__movedgd, &lo);
	vrmlrot_to_quaternion(&qlo,lo.c[0],lo.c[1],lo.c[2], lo.c[3]);
	quaternion_multiply(&qoo,&qgc,&qlo);
	quaternion_normalize(&qoo);
	quaternion_to_vrmlrot(&qoo,&oo[0],&oo[1],&oo[2],&oo[3]);
	oo[3] = -oo[3];
	double2float(node->orientation.c,oo,4);
	MARK_EVENT(X3D_NODE(node),offsetof(struct X3D_GeoViewpoint,orientation));
	
}

//void geoviewpoint_fetch_LCS_testing(struct X3D_GeoViewpoint *node, Quaternion *Quat, struct point_XYZ *Pos){
//	//testing geoviewpoint_fetch_LCS0(node,Quat,Pos);
//	
//	if(0){
//		Quaternion q2;
//		struct point_XYZ p2;
//		printf("gvp fetch LCS cycle test\n");
//		printf("fetch Pos %lf %lf %lf\n",Pos->x,Pos->y,Pos->z);
//		printf("fetch Quat %lf %lf %lf %lf\n",Quat->w,Quat->x,Quat->y,Quat->z);
//		geoviewpoint_update_LCS(node, Quat, Pos);
//		geoviewpoint_fetch_LCS0(node,&q2,&p2);
//		printf("updat Pos %lf %lf %lf\n",p2.x,p2.y,p2.z);
//		printf("updat Quat %lf %lf %lf %lf\n",q2.w,q2.x,q2.y,q2.z);
//		printf("\n");
//	}
//}
void prep_GeoViewpoint (struct X3D_GeoViewpoint *node) {
	double a1;
	GLint viewPort[10];
	if (!renderstate()->render_vp) return;

	if((struct X3D_Node*)node == getActiveLayerBoundViewpoint() && !node->_donethispass){
		X3D_Viewer *viewer = Viewer();
		node->_donethispass = 1; //if the vp id DEF/USED multiple places in the scengraph, 
		COMPILE_IF_REQUIRED

		#ifdef VERBOSE
		printf ("prep_GeoViewpoint called\n");
		#endif

		/* perform GeoViewpoint translations */
		{

			//goal: same as above except Torvaldsian
			//works for demo utm, world33 airdrie and austria vps
			struct point_XYZ Pos;
			struct SFVec4d lo;
			double oo[4], pp[3];
			struct SFVec3d LCSpos;
			struct Planet *planet;
			//ppComponent_Geospatial p = (ppComponent_Geospatial)gglobal()->Component_Geospatial.prv;
			planet = current_planet();

			//we render in 'LCS' Local coordinate system, relative to shared origin aka geoOrigin aka autoOrigin
			GeoOrient(X3D_NODE(node->geoOrigin), GEOSYS(node->__geoSystem), &node->__movedgd, &lo);

			//1. convert current .position (relative to geosystem) into LCS
			moveCoords3d(GEOSYS(node->__geoSystem),&planet->autoOrigin,&planet->autoOrient,&node->position,1,&LCSpos,&node->__movedgd);

			vecnegated(pp,LCSpos.c);
			//2. convert .orientation (relative to geosystem) into LCS
			float2double(oo,node->orientation.c,4);

			oo[3] = -oo[3];
			{
				Quaternion qlo, qao, qoo, q1, q2;
				vrmlrot_to_quaternion(&qlo,lo.c[0],lo.c[1],lo.c[2], -lo.c[3]);
				vrmlrot_to_quaternion(&qao,planet->autoOrient.c[0],planet->autoOrient.c[1],planet->autoOrient.c[2],planet->autoOrient.c[3]);
				vrmlrot_to_quaternion(&qoo,oo[0],oo[1],oo[2],oo[3]);
				// right way up and right yaw pitch axes for Austria
				quaternion_multiply(&q1,&qlo,&qao);
				quaternion_multiply(&q2,&qoo,&q1);
				quaternion_to_vrmlrot(&q2,&oo[0],&oo[1],&oo[2],&oo[3]);
			}
			FW_GL_ROTATE_RADIANS(oo[3],oo[0],oo[1],oo[2]);
			FW_GL_TRANSLATE_D(pp[0],pp[1],pp[2]);

		}
		/* we have  a new currentPosInModel now... */
		/* printf ("currentPosInModel was %lf %lf %lf\n", Viewer.currentPosInModel.x, Viewer.currentPosInModel.y, Viewer.currentPosInModel.z); */

		/* the AntiPos has been applied in the trans and rots above, so we do not need to do it here */
		//getCurrentPosInModel(FALSE); 


		/* now, lets work on the GeoViewpoint fieldOfView. 
			Q why now, here? 
			A.the window can be resized on any frame. so can't do it once in compile_geoviewpoint 
			 -and analogously we do it in prep_viewpoint and prep_orthoviewpoint
		*/
		FW_GL_GETINTEGERV(GL_VIEWPORT, viewPort);
		if(viewPort[2] > viewPort[3]) {
			a1=0;
			viewer->fieldofview = node->fieldOfView/3.1415926536*180;
		} else {
			a1 = node->fieldOfView;
			a1 = atan2(sin(a1),viewPort[2]/((float)viewPort[3]) * cos(a1));
			viewer->fieldofview = a1/3.1415926536*180;
		}
		if(viewer->type != VIEWER_WALK){
			//adjust target walk height in FLY mode
			calculateViewingSpeedB();
			node->_resetRelativeHeight = !node->relativeHeight;
		}
		#ifdef VERBOSE
		printf ("prep_GeoViewpoint, fieldOfView %f \n",node->fieldOfView); 
		#endif
	}
}

/* GeoViewpoint speeds and avatar sizes are depenent on elevation above WGS_84. These are calculated here */
void calculateViewingSpeedB() {
	/* the current position is the GC coordinate */
	ttglobal tg = gglobal();
	struct X3D_Node *boundvp = vector_back(struct X3D_Node*,getActiveBindableStacks(tg)->viewpoint);
		
	if(boundvp->_nodeType == NODE_GeoViewpoint){
		double height;
		int resetHeight;
		struct SFVec3d *gdCoords;
		struct X3D_GeoViewpoint *node = (struct X3D_GeoViewpoint*)boundvp;
		X3D_Viewer *viewer = Viewer();

        //INITIALIZE_GEOSPATIAL(node)
		COMPILE_IF_REQUIRED(X3D_NODE(node));
		gdCoords = &node->__movedgd;
		height = gdCoords->c[2];
		viewer->speed  = height * node->speedFactor;
		if(0){
			static int count = 0;
			count++;
			if(count % 20 == 0)
				printf("height %lf speedFactor %lf speed %lf\n",height,node->speedFactor,viewer->speed);
		}
		if (viewer->speed < 1.0) viewer->speed=1.0;


		/* set the navigation info - use the GeoVRML algorithms */
		set_naviWidthHeightStep(
			height/1.6 *0.25,
			height,
			height/1.6 *0.25);

	}
}

static void calculateExamineModeDistance(void) {
/*
	printf ("bind_GeoViewpoint - calculateExamineModeDistance\n");
*/
Viewer()->doExamineModeDistanceCalculations = TRUE;

}
void bind_GeoViewpoint (struct X3D_GeoViewpoint *node) {
	X3D_Viewer *viewer;

	/* did bind_node tell us we could bind this guy? */
	if (!(node->isBound)) return;

	viewer = ViewerByLayerId(node->_layerId);

	COMPILE_IF_REQUIRED

	/* set Viewer position and orientation */

	if(!node->_initializedOnce) {
		veccopyd(node->_position.c,node->position.c);
		veccopy4f(node->_orientation.c,node->orientation.c);
		node->_initializedOnce = TRUE;
	}
	if(!node->retainUserOffsets){
		veccopyd(node->position.c,node->_position.c);
		veccopy4f(node->orientation.c,node->_orientation.c);
	}


	if (viewer->transitionType != VIEWER_TRANSITION_TELEPORT && viewer->wasBound) { 
		//save the previous vp pose, in root space, for future slerps
		viewer->vp2rnSaved = TRUE; //we bind after prep_viewpoint > setup_viewpoint in rendersceneupdatescene0
		//we bind from the root, so this would be setup_viewpoint_1() and _2() 
		//- the viewmatrix including .position,.orientation,.Pos,.Quat, stereo
		{
			bindablestack* bstack = getActiveBindableStacks(gglobal());
			matcopy(viewer->slerp_viewmatrix,bstack->viewtransformmatrix);
			matcopy(viewer->slerp_posorimatrix,bstack->posorimatrix);
			
		}

        viewer->SLERPing = FALSE;
        viewer->startSLERPtime = TickTime(); 
		/* slerp Mark II */
		viewer->SLERPing2 = TRUE;
		viewer->SLERPing2justStarted = TRUE;
		//printf("binding\n");

	} else { 
		viewer->SLERPing = FALSE; 
		viewer->SLERPing2 = FALSE;
	}
	
	viewer->wasBound = TRUE;

	#ifdef VERBOSE
	printf ("bind_GeoViewpoint, setting Viewer to %lf %lf %lf orient %f %f %f %f\n",node->__movedPosition.c[0],node->__movedPosition.c[1],
	node->__movedPosition.c[2],node->orientation.c[0],node->orientation.c[1],node->orientation.c[2],
	node->orientation.c[3]);
	printf ("	node %u fieldOfView %f\n",node,node->fieldOfView);
	#endif

	vrmlrot_to_quaternion (&viewer->Quat,node->__movedOrientation.c[0],
		node->__movedOrientation.c[1],node->__movedOrientation.c[2],node->__movedOrientation.c[3]);

	calculateViewingSpeedB();
	node->_resetRelativeHeight = !node->relativeHeight;

	calculateExamineModeDistance();
	setMenuStatusVP (node->description->strptr);

}


/************************************************************************/
/* GeoTransform								*/
/************************************************************************/

void compile_GeoTransform (struct X3D_GeoTransform * node) {
	int specversion;

	#ifdef VERBOSE
	printf ("compiling GeoLocation\n");
	#endif

	if(1)
	{
		//step 1 compute origin
		geoOffsetInfo ggi, *gi;
		struct SFVec3d gdCoord, gcCoord;
		struct SFVec4d offsetOrient;
		compile_geoSystem(X3D_NODE(node),node->_nodeType,&node->geoSystem,&node->__geoSystem);
		gi = &ggi;
		gi->node = X3D_NODE(node);
		gi->geoOrigin = X3D_GEOORIGIN(node->geoOrigin);
		gi->geoSystem = GEOSYS(node->__geoSystem);
		gi->position = &node->geoCenter;
		gi->offsetCoord = &node->__movedCoords; //__localCoords; //__autoOffset;
		gi->localOrient = &offsetOrient; //&node->__localOrient;
		gi->offsetOrient = &node->__localOrient;
		gi->gdCoord = &gdCoord;
		gi->gcCoord = &gcCoord;
		printf("GT:\n");
		origin_offsets(gi);

	}else{
		MF_SF_TEMPS

		/* work out the position */
		INITIALIZE_GEOSPATIAL(node)
		COMPILE_GEOSYSTEM(node)
		INIT_MF_FROM_SF(node, geoCenter)
		MOVE_TO_ORIGIN(node)
		COPY_MF_TO_SF(node, __movedCoords)

		/* work out the local orientation */
		specversion = X3D_PROTO(node->_executionContext)->__specversion;
		GeoOrient(node->geoOrigin, GEOSYS(node->__geoSystem), &gdCoords.p[0], &node->__localOrient);
		FREE_MF_SF_TEMPS

	}

	MARK_SFVEC3D_INOUT_EVENT(node->geoCenter, node->__oldGeoCenter,offsetof (struct X3D_GeoTransform, geoCenter))
	MARK_MFNODE_INOUT_EVENT(node->children, node->__oldChildren, offsetof (struct X3D_GeoTransform, children))


	/* re-figure out which modifiers are actually in use */
	/* printf ("re-rendering for %d\n",node);*/
	node->__do_trans = verify_translate ((GLfloat *)node->translation.c);
	if (node->__do_trans) MARK_EVENT(X3D_NODE(node), offsetof (struct X3D_GeoTransform, translation));

	node->__do_scale = verify_scale ((GLfloat *)node->scale.c);
	if (node->__do_scale) MARK_EVENT(X3D_NODE(node), offsetof (struct X3D_GeoTransform, scale));

	node->__do_rotation = verify_rotate ((GLfloat *)node->rotation.c);
	if (node->__do_rotation) MARK_EVENT(X3D_NODE(node), offsetof (struct X3D_GeoTransform, rotation));

	node->__do_scaleO = verify_rotate ((GLfloat *)node->scaleOrientation.c);
	if (node->__do_scaleO) MARK_EVENT(X3D_NODE(node), offsetof (struct X3D_GeoTransform, scaleOrientation));



	#ifdef VERBOSE
	printf ("compile_GeoTransform, orig coords %lf %lf %lf, moved %lf %lf %lf\n", node->geoCoords.c[0], node->geoCoords.c[1], node->geoCoords.c[2], node->__movedCoords.c[0], node->__movedCoords.c[1], node->__movedCoords.c[2]);
	printf ("	rotation is %lf %lf %lf %lf\n",
			node->__localOrient.c[0],
			node->__localOrient.c[1],
			node->__localOrient.c[2],
			node->__localOrient.c[3]);
	#endif

	REINITIALIZE_SORTED_NODES_FIELD(node->children,node->_sortedChildren);
	MARK_NODE_COMPILED
	
	/* events */
	/* MARK_SFNODE_INOUT_EVENT(node->metadata, node->__oldmetadata, offsetof (struct X3D_GeoTransform, metadata)) */


	#ifdef VERBOSE
	printf ("compiled GeoTransform\n\n");
	#endif
}


/* do transforms, calculate the distance */
void prep_GeoTransform (struct X3D_GeoTransform *node) {

	//done above INITIALIZE_GEOSPATIAL(node)
	COMPILE_IF_REQUIRED

        /* rendering the viewpoint means doing the inverse transformations in reverse order (while poping stack),
         * so we do nothing here in that case -ncoder */

	/* printf ("prep_Transform, render_hier vp %d geom %d light %d sens %d blend %d prox %d col %d\n",
	 render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision); */

	/* do we have any geometry visible, and are we doing anything with geometry? */
	OCCLUSIONTEST

	if(!renderstate()->render_vp) {
		FW_GL_PUSH_MATRIX();


		/* TRANSLATION */
		if (node->__do_trans)
			FW_GL_TRANSLATE_F(node->translation.c[0],node->translation.c[1],node->translation.c[2]);

        /* GeoTransform TRANSLATION */
        FW_GL_TRANSLATE_D(node->__movedCoords.c[0], node->__movedCoords.c[1], node->__movedCoords.c[2]);
                
        //printf ("prep_GeoLoc trans to %lf %lf %lf\n",node->__movedCoords.c[0],node->__movedCoords.c[1],node->__movedCoords.c[2]);
        FW_GL_ROTATE_RADIANS(node->__localOrient.c[3], node->__localOrient.c[0],node->__localOrient.c[1],node->__localOrient.c[2]);
                
		/* ROTATION */
		if (node->__do_rotation) {
			FW_GL_ROTATE_RADIANS(node->rotation.c[3], node->rotation.c[0],node->rotation.c[1],node->rotation.c[2]);
		}

		/* SCALEORIENTATION */
		if (node->__do_scaleO) {
			FW_GL_ROTATE_RADIANS(node->scaleOrientation.c[3], node->scaleOrientation.c[0],
				node->scaleOrientation.c[1],node->scaleOrientation.c[2]);
		}

		/* SCALE */
		if (node->__do_scale)
			FW_GL_SCALE_F(node->scale.c[0],node->scale.c[1],node->scale.c[2]);

		/* REVERSE SCALE ORIENTATION */
		if (node->__do_scaleO)
			FW_GL_ROTATE_RADIANS(-node->scaleOrientation.c[3], node->scaleOrientation.c[0],
				node->scaleOrientation.c[1],node->scaleOrientation.c[2]);

		/* REVERSE CENTER */
		FW_GL_TRANSLATE_D(-node->__movedCoords.c[0], -node->__movedCoords.c[1], -node->__movedCoords.c[2]);

		RECORD_DISTANCE
        }
}

void prep_GeoTransform_WRONG_DUG9 (struct X3D_GeoTransform *node) {
	//dug9 had the wrong mental model, was thinking like geoLocation, its going the other way, children are geo
	//done above INITIALIZE_GEOSPATIAL(node)
	COMPILE_IF_REQUIRED

        /* rendering the viewpoint means doing the inverse transformations in reverse order (while poping stack),
         * so we do nothing here in that case -ncoder */

	/* printf ("prep_Transform, render_hier vp %d geom %d light %d sens %d blend %d prox %d col %d\n",
	 render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision); */

	/* do we have any geometry visible, and are we doing anything with geometry? */
	OCCLUSIONTEST

	if(!renderstate()->render_vp) {
		FW_GL_PUSH_MATRIX();

        /* GeoTransform TRANSLATION */
        FW_GL_TRANSLATE_D(node->__movedCoords.c[0], node->__movedCoords.c[1], node->__movedCoords.c[2]);
                
        //printf ("prep_GeoLoc trans to %lf %lf %lf\n",node->__movedCoords.c[0],node->__movedCoords.c[1],node->__movedCoords.c[2]);
        FW_GL_ROTATE_RADIANS(node->__localOrient.c[3], node->__localOrient.c[0],node->__localOrient.c[1],node->__localOrient.c[2]);

		/* TRANSLATION */
		if (node->__do_trans)
			FW_GL_TRANSLATE_F(node->translation.c[0],node->translation.c[1],node->translation.c[2]);

                
		/* ROTATION */
		if (node->__do_rotation) {
			FW_GL_ROTATE_RADIANS(node->rotation.c[3], node->rotation.c[0],node->rotation.c[1],node->rotation.c[2]);
		}

		/* SCALEORIENTATION */
		if (node->__do_scaleO) {
			FW_GL_ROTATE_RADIANS(node->scaleOrientation.c[3], node->scaleOrientation.c[0],
				node->scaleOrientation.c[1],node->scaleOrientation.c[2]);
		}

		/* SCALE */
		if (node->__do_scale)
			FW_GL_SCALE_F(node->scale.c[0],node->scale.c[1],node->scale.c[2]);

		/* REVERSE SCALE ORIENTATION */
		if (node->__do_scaleO)
			FW_GL_ROTATE_RADIANS(-node->scaleOrientation.c[3], node->scaleOrientation.c[0],
				node->scaleOrientation.c[1],node->scaleOrientation.c[2]);

		///* REVERSE CENTER */
		//FW_GL_TRANSLATE_D(-node->__movedCoords.c[0], -node->__movedCoords.c[1], -node->__movedCoords.c[2]);
		if(fwl_getDrawBoundingBoxes()) extent6f_draw(node->_extent);

		RECORD_DISTANCE
        }
}



void fin_GeoTransform (struct X3D_GeoTransform *node) {
	// done in compile INITIALIZE_GEOSPATIAL(node)
	COMPILE_IF_REQUIRED
	OCCLUSIONTEST

        if(!renderstate()->render_vp) {
            FW_GL_POP_MATRIX();
        } else {
           /*Rendering the viewpoint only means finding it, and calculating the reverse WorldView matrix.*/
            if((node->_renderFlags & VF_Viewpoint) == VF_Viewpoint) {
                FW_GL_ROTATE_RADIANS(node->scaleOrientation.c[3],node->scaleOrientation.c[0],node->scaleOrientation.c[1],node->scaleOrientation.c[2]);
                FW_GL_SCALE_F((float)1.0/(((node->scale).c[0])),(float)1.0/(((node->scale).c[1])),(float)1.0/(((node->scale).c[2]))
                );
                FW_GL_ROTATE_RADIANS(-node->scaleOrientation.c[3],node->scaleOrientation.c[0],node->scaleOrientation.c[1],node->scaleOrientation.c[2]);
                FW_GL_ROTATE_RADIANS(-(((node->rotation).c[3])),((node->rotation).c[0]),((node->rotation).c[1]),((node->rotation).c[2])
                );
                FW_GL_TRANSLATE_F(-(((node->translation).c[0])),-(((node->translation).c[1])),-(((node->translation).c[2]))
                );

		        FW_GL_ROTATE_RADIANS(node->__localOrient.c[3], node->__localOrient.c[0],node->__localOrient.c[1],-node->__localOrient.c[2]);

                FW_GL_TRANSLATE_D(-(((node->__movedCoords).c[0])),-(((node->__movedCoords).c[1])),-(((node->__movedCoords).c[2]))
                );
            }
        }
} 

void fin_GeoTransform_WRONG_DUG9 (struct X3D_GeoTransform *node) {
	// done in compile INITIALIZE_GEOSPATIAL(node)
	COMPILE_IF_REQUIRED
	OCCLUSIONTEST

        if(!renderstate()->render_vp) {
            FW_GL_POP_MATRIX();
        } else {
           /*Rendering the viewpoint only means finding it, and calculating the reverse WorldView matrix.*/
            if((node->_renderFlags & VF_Viewpoint) == VF_Viewpoint) {
                FW_GL_TRANSLATE_D(((node->__movedCoords).c[0]),((node->__movedCoords).c[1]),((node->__movedCoords).c[2])
                );
                FW_GL_ROTATE_RADIANS(node->scaleOrientation.c[3],node->scaleOrientation.c[0],node->scaleOrientation.c[1],node->scaleOrientation.c[2]);
                FW_GL_SCALE_F((float)1.0/(((node->scale).c[0])),(float)1.0/(((node->scale).c[1])),(float)1.0/(((node->scale).c[2]))
                );
                FW_GL_ROTATE_RADIANS(-node->scaleOrientation.c[3],node->scaleOrientation.c[0],node->scaleOrientation.c[1],node->scaleOrientation.c[2]);
                FW_GL_ROTATE_RADIANS(-(((node->rotation).c[3])),((node->rotation).c[0]),((node->rotation).c[1]),((node->rotation).c[2])
                );
                FW_GL_TRANSLATE_D(-(((node->__movedCoords).c[0])),-(((node->__movedCoords).c[1])),-(((node->__movedCoords).c[2]))
                );
                FW_GL_TRANSLATE_F(-(((node->translation).c[0])),-(((node->translation).c[1])),-(((node->translation).c[2]))
                );
            }
        }
} 


void child_GeoTransform (struct X3D_GeoTransform *node) {
	CHILDREN_COUNT
	//LOCAL_LIGHT_SAVE
	INITIALIZE_GEOSPATIAL(node)
	COMPILE_IF_REQUIRED
	OCCLUSIONTEST
	RETURN_FROM_CHILD_IF_NOT_FOR_ME

	/* any children at all? */
	if (nc==0) return;

	/* {
		int x;
		struct X3D_Node *xx;

		printf ("child_Transform, this %d \n",node);
		for (x=0; x<nc; x++) {
			xx = X3D_NODE(node->children.p[x]);
			printf ("	ch %d type %s dist %f\n",node->children.p[x],stringNodeType(xx->_nodeType),xx->_dist);
		}
	} */


	/* do we have a local light for a child? */
	//LOCAL_LIGHT_CHILDREN(node->children);
	prep_sibAffectors((struct X3D_Node*)node,&node->__sibAffectors);

	/* now, just render the non-directionalLight children */

	/* printf ("Transform %d, flags %d, render_sensitive %d\n",
			node,node->_renderFlags,render_sensitive); */

	#ifdef CHILDVERBOSE
		printf ("transform - doing normalChildren\n");
	#endif

	normalChildren(node->children);

	#ifdef CHILDVERBOSE
		printf ("transform - done normalChildren\n");
	#endif

	//LOCAL_LIGHT_OFF
	fin_sibAffectors((struct X3D_Node*)node,&node->__sibAffectors);

}

//CONVERT_BACK_TO_GD_OR_UTMB(geoSystem, geoOrigin, thisField);
void CONVERT_BACK_TO_GD_OR_UTMC(Geosys *targetGeoSystem, struct X3D_Node *geoorigin, 
		struct SFVec3d *LCSpos, struct SFVec3d *gdCoords, struct SFVec3d *thisField) {
	//assumes incoming thisField is in GC system
	//outputs thisField in targetGeoSystem

/* compileGeosystem - encode the return value such that srf->p[x] is... 
	0:	spatial reference frame (GEOSP_UTM, GEOSP_GC, GEOSP_GD); 
	1:	ellipsoid index (defaults to GEOSP_WE) 
	2:	UTM zone number, 1..60. INT_ID_UNDEFINED = not specified 
	3:	UTM:    if "northing_first" TRUE, if "easting_first", FALSE 
	4:	UTM:    if "S" - value is FALSE, not S, value is TRUE
	5:	GD:     if "latitude_first" TRUE, if "longitude_first", FALSE 
	6:	GD: true if geoid height
	7:	GD: TRUE: decimal degrees, FALSE radians
*/
 
	/* do we need to change this from a GCC? */ 
	Geosys *geoSystem = targetGeoSystem;
	struct X3D_GeoOrigin *geoOrigin = (struct X3D_GeoOrigin*)geoorigin;
	ppComponent_Geospatial p = (ppComponent_Geospatial)gglobal()->Component_Geospatial.prv;
	veccopyd(thisField->c,LCSpos->c);
	// already in GC system //if(0) vecaddd(thisField->c,thisField->c,p->autoOrigin.c);

	if (geoSystem != NULL) { /* do we have a GeoSystem specified?? if not, dont do this! */ 
 
		if (geoSystem->spatial_system != GEOSP_GC) { 
			/* have to convert to GD or UTM. Go to GD first */ 
			gccToGdc (geoSystem, thisField, gdCoords);
			if(geoSystem->geoid_height || geoSystem->relativeHeight)
				gdCoords->c[2] -= userHeight2ellipsoidHeight(geoSystem,gdCoords);
			veccopyd(thisField->c,gdCoords->c);

			/* printf ("changed as a GDC, %lf %lf %lf\n", thisField.c[0], thisField.c[1], thisField.c[2]); */ 
		 
			/* is this a GD? if so, go no further */ 
			if (geoSystem->spatial_system == GEOSP_UTM || geoSystem->spatial_system == GEOSP_3TM ) { 
				/* convert this to UTM  or 3TM */ 
				double dtemp[3];

				if(geoSystem->spatial_system == GEOSP_UTM){
					gdToUtm3d(geoSystem,thisField->c, dtemp); 
					veccopyd(thisField->c,dtemp);
				}else if(geoSystem->spatial_system == GEOSP_3TM) {
					gdTo3tm3d(geoSystem,thisField->c, dtemp);
					veccopyd(thisField->c,dtemp);
				} 
 
			/* printf ("changed as a UTM, %lf %lf %lf\n", thisField[0], thisField[1], thisField[2]); */ 
			}  
		} 
	}
}
void CONVERT_BACK_TO_GD_OR_UTMB(Geosys *targetGeoSystem, struct X3D_Node *geoOrigin, 
		struct SFVec3d *thisField)
{
	struct SFVec3d LCSpos, gdCoord;
	veccopyd(LCSpos.c,thisField->c);
	CONVERT_BACK_TO_GD_OR_UTMC(targetGeoSystem,geoOrigin,&LCSpos,&gdCoord,thisField);
}
/*
WALK navigation:
(VPbindPose) +  userOffsets[ (cumulative navigation) + (camera tilts/orientation) ]
The gravity height adustment (and general collision) goes into cumulative navigation
The mouse navigation goes into cumulative navigation
The LEVEL and FLY tilts go into orientation

GEO WALK navigation:
almost everything is related to ellipsoid / GD coordinates
- LEVEL - relative to current gdCoords/GD position/GD up vector
- orientation (camera tilts) relative to GD vertical
- gravity correction - along GD vertical
- SPEED - relative to GD height above GEG terrain, ellipsoid or GD radius from GC center, along GD vertical
retainedUserOffsets are stored as absolute GD postion + orientation:
 userOffsets = retainedGDposition - vpBind + orientation
except: GD pose transformed into SLSLA for rendering, picking and extents

*/

/* compileGeosystem - encode the return value such that srf->p[x] is... 
	0:	spatial reference frame (GEOSP_UTM, GEOSP_GC, GEOSP_GD); 
	1:	ellipsoid index (defaults to GEOSP_WE) 
	2:	UTM zone number, 1..60. INT_ID_UNDEFINED = not specified 
	3:	UTM:    if "northing_first" TRUE, if "easting_first", FALSE 
	4:	UTM:    if "S" - value is FALSE, not S, value is TRUE
	5:	GD:     if "latitude_first" TRUE, if "longitude_first", FALSE 
	6:	GD: true if geoid height
	7:	GD: TRUE: decimal degrees, FALSE radians
*/
int geoelevationgrid_getGDHeight0(struct X3D_GeoElevationGrid *node, struct SFVec3d *gdCoord, Geosys *geoSystem, double *gridHeight){
	int hit;
	//we'll work in gd coord.
	struct point_XYZ result;
	float centerf[3],bottomf[3];
	double centerd[3], bottomd[3], spined[3], vertvecd[3];
	struct SFVec3d xxCoord;
	double cosine;
	double tmin[3],tmax[3]; /* MBB for facet */
	struct sNaviInfo *naviinfo;
	Geosys *nodeSystem;
	GLDOUBLE awidth, atop, abottom, astep;
	ttglobal tg = gglobal();
	//naviinfo = (struct sNaviInfo *)tg->Bindable.naviinfo;

	nodeSystem = GEOSYS(node->__geoSystem);
	hit = -1; //caller: watch out, this can be -1 on return. only 1 means true hit
	//get target node's gdCoord into GEG's gdcoord 
	veccopyd(xxCoord.c,gdCoord->c); 
	//printf("gdCoord %lf %lf %lf\n",gdCoord->c[0],gdCoord->c[1],gdCoord->c[2]);
	if(geoSystem->gd_latitude_first != GEOSYS(node->__geoSystem)->gd_latitude_first) 
		vecswizzle2d(xxCoord.c);
	if(geoSystem->gd_degrees != GEOSYS(node->__geoSystem)->gd_degrees) 
		if(geoSystem->gd_degrees == TRUE)
			vecscale2d(xxCoord.c,xxCoord.c,RADIANS_PER_DEGREE);
		else
			vecscale2d(xxCoord.c,xxCoord.c,DEGREES_PER_RADIAN);


	//get target nodes' gd coord into GEG's geosystem if not gd
	int XTM = FALSE;
	if(nodeSystem->spatial_system == GEOSP_UTM || nodeSystem->spatial_system == GEOSP_3TM ) { 
		// convert GVP's gdCoord to UTM  or 3TM, in GEGs user order
		XTM = TRUE;
		double dtemp[3];
		if(nodeSystem->spatial_system == GEOSP_UTM){
			gdToUtm3d(GEOSYS(node->__geoSystem),xxCoord.c, dtemp); 
			veccopyd(xxCoord.c,dtemp);
		}else if(nodeSystem->spatial_system == GEOSP_3TM) {
			gdTo3tm3d(GEOSYS(node->__geoSystem),xxCoord.c, dtemp);
			veccopyd(xxCoord.c,dtemp);
		} 
 	} else if(nodeSystem->spatial_system == GEOSP_GC) {
		//no such thing as GC GEG
		return hit;
	}


	int inside;
	double emin[2],emax[2];
	//not sure what space the GEG's node->_extent is in, so will recalculate here in its user coordinates
	double size[2], spacing[2];
	int idimension[2];
	//we'll put dimension and spacing into x/easting/longitude-first order, 
	// to capture xDimension,zDimension naming, then swizzle as needed
	idimension[0] = node->xDimension; //assume x is longitude/easting
	idimension[1] = node->zDimension; //assume z is latitude/northing
	spacing[0] = node->xSpacing; //x long/east
	spacing[1] = node->zSpacing; //z lat/north
	//GEGs Geosystem
	//to do some extent math, we'll swizzle into user order
	int swizzle_xzdimension = (XTM && nodeSystem->xtm_northing_first) || (!XTM && nodeSystem->gd_latitude_first);
	if(swizzle_xzdimension) {
		//swizzle spacing,dimension into GEG's user order
		//printf("early dimension swizzle\n");
		int itmp = idimension[0];
		idimension[0] = idimension[1];
		idimension[1] = itmp;
		vecswizzle2d(spacing);
	}
	size[0] = spacing[0]*(idimension[0] -1); //take off one column and one row to get cells (vs points)
	size[1] = spacing[1]*(idimension[1] -1);
	emin[0] = min(node->geoGridOrigin.c[0],node->geoGridOrigin.c[0]+size[0]);
	emax[0] = max(node->geoGridOrigin.c[0],node->geoGridOrigin.c[0]+size[0]);
	emin[1] = min(node->geoGridOrigin.c[1],node->geoGridOrigin.c[1]+size[1]);
	emax[1] = max(node->geoGridOrigin.c[1],node->geoGridOrigin.c[1]+size[1]);
	//printf("xxCoord= %lf %lf %lf\n",xxCoord.c[0],xxCoord.c[1],xxCoord.c[2]);
	//printf("emin= %lf %lf emax= %lf %lf\n",emin[0],emin[1],emax[0],emax[1]);
	inside  = xxCoord.c[0] <= emax[0] && xxCoord.c[0] >= emin[0];
	inside &= xxCoord.c[1] <= emax[1] && xxCoord.c[1] >= emin[1];
	//printf("b");
	if(inside){
		double spinelength, vcenterd[3], pp[2];
		//double x,z,
		double cx,cz;
		double deltah, gridpointf[3];
		//printf("c\n");
		hit = 0;
		//see if grid height is below, between or above avatar
		pp[0] = xxCoord.c[0] - node->geoGridOrigin.c[0]; //latitude first/northing first default? or x == 0, z == 1?
		pp[1] = xxCoord.c[1] - node->geoGridOrigin.c[1];
		
		//get pp from user order into x-first, z-second order - our old math below assumes x-first order
		if(swizzle_xzdimension) {
			//printf("swizzling user into xz\n");
			vecswizzle2d(pp);
		}
		//printf("x,z= %lf %lf\n",pp[0],pp[1]);
		//node->xDimension
		// z h2  h3
		// ^ h0  h1
		// |-->x
		//(ix,iz)
		double hh[4],gridheight;
		int i0,i1,i2,i3, ix, iz;
		ix = (int)(pp[0]/node->xSpacing);
		iz = (int)(pp[1]/node->zSpacing);
		//int nh = node->height.n;
		//printf("total h = %d\n",nh);

		//printf("xspacing,zspacing,xdimension zdimension= %lf %lf %d %d\n",node->xSpacing,node->zSpacing,node->xDimension, node->zDimension);
		//printf("ix,iz= %d %d\n",ix,iz);
		i0 = iz * node->xDimension + ix;
		i1 = i0 + 1;
		i2 = i0 + node->xDimension;
		i3 = i2 + 1;
		//printf("i0-i3 = %d %d %d %d\n",i0,i1,i2,i3); //should all be < height.n
		hh[0] = node->height.p[i0];
		hh[1] = node->height.p[i1];
		hh[2] = node->height.p[i2];
		hh[3] = node->height.p[i3];
		//normalize cell x and z
		cx = (pp[0] - ix*node->xSpacing)/node->xSpacing;
		cz = (pp[1] - iz*node->zSpacing)/node->zSpacing;
		//height interpolation by finite elements > bilinear interpolotion of height
		// (could do cubic using 3x3 chunks)
		//printf("cx %lf cz %lf\n",cx,cz);
		gridheight =  hh[0]*(1.0f - cz)*(1.0f - cx)
					+ hh[1]*(1.0f - cz)*cx 
					+ hh[2]*cz*(1.0f - cx) 
					+ hh[3]*cz*cx;
		gridheight *= node->yScale;
		//printf("_");
		*gridHeight = gridheight;
		//printf("\tgridheight=%lf\n",gridheight);
		hit = 1;
	}
	return hit;
}

int geoelevationgrid_disp2(struct X3D_GeoElevationGrid *node, struct X3D_GeoViewpoint *gvp){
	// general polyrep collision does a few ugly things:
	// 1. transforms all the points into collision/avatar space
	// 2. iterates over all the triangles (a few times)
	// this geoElevationGrid optimization will take a few shortcuts:
	// a) only do gravity, if enabled
	// b) transform avatar gravity vector into grid space
	// c) use geoElevationGrid rows, columns, xspace,zspace, to look up heights by avatar position N,E or lat,lon in grid space
	// d) see if its a collision
	// e) update the climing / falling parameters (and skip wall penetration detection and bump collision testing)
	// if for some reason it can't handle it, it returns -1, and then the generic collision can be called
	struct sFallInfo *fi;
	int hit,i;

	hit = -1; //0 handled, and no colliision, 1=handled and collision, -1=not handled (not woaking, or gravity vector not perpendicular to grid)
	fi = FallInfo();

	if(fi->walking)
	{
		struct point_XYZ result;
		float centerf[3],bottomf[3];
		double centerd[3], bottomd[3], spined[3], vertvecd[3], gridheight;
		struct SFVec3d *gdCoord, xxCoord;
		Geosys *geoSystem;
		double cosine;
		double tmin[3],tmax[3]; /* MBB for facet */
		struct sNaviInfo *naviinfo;
		GLDOUBLE awidth, atop, abottom, astep;
		ttglobal tg = gglobal();
		naviinfo = (struct sNaviInfo *)tg->Bindable.naviinfo;

		//GVP gdcoord and geosys
		gdCoord = &gvp->__movedgd;
		geoSystem = GEOSYS(node->__geoSystem);
		if( geoelevationgrid_getGDHeight0(node, gdCoord, geoSystem, &gridheight) == 1){
			hit = 1;
			// scraped from:
			//	accumulateFallingClimbing(abottom,atop,astep,p,num,n,tmin,tmax); //y1, y2, p, num, n);
			if(gvp->_resetRelativeHeight){
				naviinfo->height = gdCoord->c[2] - gridheight;
				gvp->_resetRelativeHeight = FALSE; //we do just once per WALK 'session' (WALK turned on, or bind with WALK on)
				//printf("+");
			}
			//printf("=\n");
			double abottom = gdCoord->c[2] - naviinfo->height; //100; // - avatar height?
			double hhh = gridheight - abottom;
			//printf("\ngridHeight %lf avatarHeight %lf\n",hhh,abottom);
			double hhbelowfoot = hhh; //hhh - abottom;
			//fi->fallHeight = 1000000.0;
			if( hhh < 0.0 )
			{
				//printf("V");
				/* falling */
				if( hhh < abottom && hhh > -fi->fallHeight) 
				{
					//printf("v");
					/* FALLING */
					if(fi->hits ==0)
						fi->hfall = hhbelowfoot; //hh - y1;
					else
						if(hhbelowfoot > fi->hfall) fi->hfall = hhbelowfoot; //hh - y1;
					fi->hits++;
					fi->isFall = 1;
					//printf("hfall %lf\n",fi->hfall);
				}else{
					//printf("~");
					/* regular below / nadir collision - below avatar center but above avatar's feet which are at 0.0 - avatar.height*/
					if( hhh >= abottom  ) /* && hh <= (y1-ystep) ) //no step height implementation */
					{
						/* CLIMBING. handled elsewhere for displacements, except annihilates any fall*/
						fi->canFall = 0;

						if( fi->isClimb == 0 )
							fi->hclimb = hhbelowfoot; //hh - y1;
						else
							fi->hclimb = DOUBLE_MAX(fi->hclimb,hhbelowfoot);
						fi->isClimb = 1;
					}
				}
			}
			double head = 0.0;
			double hhabovehead = hhh - head;
			abottom = 0.0;
			if( hhabovehead > 0.0 )
			{
				//printf("H");
				/* climbing from undergound */
				if( hhabovehead < fi->climbHeight) 
				{
					//printf("^");
					/* CLIMBING */
					fi->canFall = 0;

					if( fi->isClimb == 0 ){
						fi->hclimb = hhabovehead + abottom; //hh - y1;
					}else{
						fi->hclimb = DOUBLE_MAX(fi->hclimb,hhabovehead + abottom);
					}
					fi->isClimb = 1;
					//printf("hclimb %lf abottom %lf hhabovehead %lf\n",fi->hclimb,abottom,hhabovehead);
				}
			}
		}
	}

	return hit;
}

void collide_GeoElevationGrid(struct X3D_GeoElevationGrid *node){
	/* 
	For examine and fly navigation modes, there's no gravity direction. 
	Specifications say for geo walk navigation mode gravity vector is down along GD ellipsoid vertical 
	with respect to (wrt) the current GD (geodetic/ellipsoidal latitude, longitude, height)
	posistion of the viewpoint, not including the viewpoint's orientation 
	field. 
	When you collide in geo walk mode, the avatar collision
	volume is aligned to current viewpoint GD vertical. 
	*/

	int ihit = -1;
	struct Vector *vpstack;
	struct X3D_Node *boundvp = NULL;
	ttglobal tg = gglobal();
	vpstack = getActiveBindableStacks(tg)->viewpoint;
	if(vpstack && vpstack->n)
		boundvp = vector_back(struct X3D_Node*,getActiveBindableStacks(tg)->viewpoint);

	if(node->_nodeType == NODE_GeoElevationGrid && boundvp->_nodeType == NODE_GeoViewpoint){
		ihit = geoelevationgrid_disp2((struct X3D_GeoElevationGrid*)node, (struct X3D_GeoViewpoint *)boundvp);
		//if(ihit==0) printf("0");
		//if(ihit==1) printf("1");
		//if(ihit==-1) printf(".");
	}
	if(0) if(ihit == -1){
		//above couldn't handle it, thunking to generic 
		//problem: if its a really dense grid, the framerate plummets
		collide_genericfaceset ((struct X3D_IndexedFaceSet *)node );
	}

}
double getTerrainHeight(int planetID, Geosys *geoSystem, struct SFVec3d *gdCoord){
	int i,j,nfound;
	struct Planet *planet;
	double highest;
	//find planet
	ppComponent_Geospatial p = (ppComponent_Geospatial)gglobal()->Component_Geospatial.prv;
	highest = 0.0; //this means we can't go below ground. It also means if no GEG found, then we are relative to ellipsoid
	if(!p->planet_stack) return highest; //no GEGs registered, stick to absolute height
	nfound = 0;
	planet = NULL;
	for(i=0;i<vectorSize(p->planet_stack);i++){
		planet = vector_get_ptr(struct Planet,p->planet_stack,i);
		if(planet && planet->ID == planetID) {
			if(!planet->gegs) return highest; //no gegs registered
			for(j=0;j<vectorSize(planet->gegs);j++){
				double gridheight;
				struct X3D_GeoElevationGrid *geg = vector_get(struct X3D_GeoElevationGrid*,planet->gegs,j);
				if(geg)
				if( geoelevationgrid_getGDHeight0(geg,gdCoord,geoSystem,&gridheight) == 1){
					//make a list of hits, and pick the highest one, in case there are grid overlays etc.
					nfound++;
					if(nfound == 1) highest = gridheight;
					highest = max(highest,gridheight);
				}
			}
		}
	}
	return highest;
}
//double adjust_geoLocationRelativeHeight(struct X3D_GeoLocation *node,int planetID){
//	//call from prep or compile_ geoLocation if the height is supposed to be a relative height 
//	// ie height above ellipsoid.
//	// this searchse through all the GeoElevationGrids registered for the same planet, 
//	// to find the highest one under this GL if any, and adjust the height as needed
//	double highest = 0.0;
//	if(node && node->_nodeType == NODE_GeoLocation){
//		int i,j,nfound;
//		struct Planet *planet;
//		//find planet
//		ppComponent_Geospatial p = (ppComponent_Geospatial)gglobal()->Component_Geospatial.prv;
//		if(!p->planet_stack) return highest; //no GEGs registered, stick to absolute height
//		nfound = 0;
//		planet = NULL;
//
//		for(i=0;i<vectorSize(p->planet_stack);i++){
//			planet = vector_get_ptr(struct Planet,p->planet_stack,i);
//			if(planet && planet->ID == planetID) {
//				if(!planet->gegs) return highest; //no gegs registered
//				for(j=0;j<vectorSize(planet->gegs);j++){
//					double gridheight;
//					struct X3D_GeoElevationGrid *geg = vector_get(struct X3D_GeoElevationGrid*,planet->gegs,j);
//					if(geg)
//					if( geoelevationgrid_getGDHeight0(geg,&node->__movedgd,GEOSYS(node->__geoSystem),&gridheight) == 1){
//						//make a list of hits, and pick the highest one, in case there are grid overlays etc.
//						nfound++;
//						if(nfound == 1) highest = gridheight;
//						highest = max(highest,gridheight);
//					}
//				}
//			}
//		}
//		
//	}
//	return highest;
//}


void RegisterGeoElevationGrid(struct X3D_Node *node, int planetID){
	//call this from render_geoelevationgrid, or collide_?, so we get the planet from
	// a) geoSystem "P#"
	// b) X3DGeoPlanet.planetID="#" which is pushed and popped, so DEF/USE can put get GEG in different planets
	// this implies you can have DEF/USE multiple USEs of GEGs for different planets (but just once per planet?), 
	// so {planet,geg} 
	// should be the unique index
	if(node && node->_nodeType == NODE_GeoElevationGrid){
		int i,j,ifound;
		struct Planet *planet;
		//add to planet
		ppComponent_Geospatial p = (ppComponent_Geospatial)gglobal()->Component_Geospatial.prv;
		if(!p->planet_stack) p->planet_stack = newStack(struct Planet);
		ifound = -1;
		planet = NULL;
		for(i=0;i<vectorSize(p->planet_stack);i++){
			planet = vector_get_ptr(struct Planet,p->planet_stack,i);
			if(planet->ID == planetID) {
				//right planet
				ifound = i;
				break;
			}
		}
		if(ifound == -1){
			struct Planet newplanet;
			memset(&newplanet,0,sizeof(struct Planet));
			newplanet.ID = planetID;
			printf("adding planet # %d\n",planetID);
			vector_pushBack(struct Planet,p->planet_stack,newplanet);
			ifound = p->planet_stack->n -1;
			planet = vector_get_ptr(struct Planet,p->planet_stack,ifound);
		}
		if(planet->gegs == NULL) planet->gegs = newStack(struct X3D_Node*);
		printf("adding GEG %x to planet # %d\n",node,planetID);
		vector_pushBack(struct X3D_Node*,planet->gegs,node);
	}
}
void unRegisterGeoElevationGrid(struct X3D_Node *node){
	//call this from unRegisterX3DAnyNode, which is called from unload_broto, which is called 
	// when unloading an Inline (including GeoLOD inlines)
	if(node && node->_nodeType == NODE_GeoElevationGrid){
		int i,j;
		//remove all instances from all planets
		ppComponent_Geospatial p = (ppComponent_Geospatial)gglobal()->Component_Geospatial.prv;
		if(!p->planet_stack) return;
		for(i=0;i<vectorSize(p->planet_stack);i++){
			struct Planet *planet = vector_get_ptr(struct Planet,p->planet_stack,i);
			if(planet->gegs)
			for(j=0;j<vectorSize(planet->gegs);j++){
				int k = vectorSize(planet->gegs) - j -1;
				struct X3D_Node *geg = vector_get(struct X3D_Node*,planet->gegs,k);
				if(geg == node){
					vector_set(struct X3D_Node*,planet->gegs,k,NULL);
				}
			}
		}
	}

}


void compile_GeoPlanet(struct X3D_GeoPlanet *node){
	{
		struct Planet *planet = current_planet();
		if(planet == NULL){
			planet = add_planet(node->planetId);
			printf("planet=%x\n",planet);
		}
	}
	REINITIALIZE_SORTED_NODES_FIELD(node->children,node->_sortedChildren);
	MARK_NODE_COMPILED
	
	/* events */
	/* MARK_SFNODE_INOUT_EVENT(node->metadata, node->__oldmetadata, offsetof (struct X3D_GeoLocation, metadata)) */

	INITIALIZE_EXTENT;

}

void prep_GeoPlanet(struct X3D_GeoPlanet *node){
	push_planetId(node->planetId);
	COMPILE_IF_REQUIRED
	if(!renderstate()->render_vp) {
		double aoo[4],ao[3];
		struct Planet *planet;
		//ppComponent_Geospatial p = (ppComponent_Geospatial)gglobal()->Component_Geospatial.prv;

		planet = current_planet();
		//we need to get the LCS to GC transform on the stack
		FW_GL_PUSH_MATRIX();
		veccopyd(ao,planet->autoOrigin.c);
		veccopy4d(aoo,planet->autoOrient.c);
		FW_GL_TRANSLATE_D(ao[0], ao[1], ao[2]);
		FW_GL_ROTATE_RADIANS(aoo[3], aoo[0],aoo[1],aoo[2]);


		/* did either we or the Viewpoint move since last time? */
		RECORD_DISTANCE
		if(renderstate()->render_boxes) extent6f_draw(node->_extent);
	}

}
	
void child_GeoPlanet(struct X3D_GeoPlanet *node){
	CHILDREN_COUNT
	//LOCAL_LIGHT_SAVE
	//INITIALIZE_GEOSPATIAL(node)
	COMPILE_IF_REQUIRED
//	OCCLUSIONTEST
	RETURN_FROM_CHILD_IF_NOT_FOR_ME

	//LOCAL_LIGHT_CHILDREN(node->children);
	prep_sibAffectors((struct X3D_Node*)node,&node->__sibAffectors);

	normalChildren(node->children);

	fin_sibAffectors((struct X3D_Node*)node,&node->__sibAffectors);
}
void fin_GeoPlanet(struct X3D_GeoPlanet *node){
	//pop LCS to GC transform
	COMPILE_IF_REQUIRED
	OCCLUSIONTEST

	if(!renderstate()->render_vp) {
		FW_GL_POP_MATRIX();
	} else {
		if ((node->_renderFlags & VF_Viewpoint) == VF_Viewpoint) {
			double aoo[4],ao[3];
			struct Planet *planet;
			//ppComponent_Geospatial p = (ppComponent_Geospatial)gglobal()->Component_Geospatial.prv;
			planet = current_planet();
			veccopyd(ao,planet->autoOrigin.c);
			veccopy4d(aoo,planet->autoOrient.c);

			FW_GL_ROTATE_RADIANS(-aoo[3], aoo[0],aoo[1],aoo[2]);
			FW_GL_TRANSLATE_D(-ao[0], -ao[1], -ao[2]);

		}
	}
	pop_planetId();

}

//by 'user' coordinates we mean as authored in the scene file and specfied by geosystem by the scene author
// when converting from GC to user, we might find the user _is_ GC. 
// with these functions you don't need to know or care about shortcuts.
void user2gc(Geosys * geoSystem, struct SFVec3d *geo, int n, struct SFVec3d *gc){
	//UNTESTED
	int i;
	struct SFVec3d gdCoord;
	for(i=0;i<n;i++){
		moveCoords3d(geoSystem,NULL,NULL,&geo[i],1,&gc[i],&gdCoord);
	}
}
void gc2user(Geosys * geoSystem, struct SFVec3d *gc,  int n, struct SFVec3d *geo){
	//UNTESTED
	int i;
	struct SFVec3d gdCoord;
	for(i=0;i<n;i++){
		CONVERT_BACK_TO_GD_OR_UTMC(geoSystem,NULL,&gc[i],&gdCoord,&geo[i]);
	}
}
/*
//as with geoConvert, its more reliable to go user2gc gc2anything and vice versa, rather 
// than user2gd. That's because ideally we go geo2geo(source_geosystem,dest_geosystem).
// and when we do user2gd its confusing which geosystem we are using for the gd
// and are the gd lat first, radians, or are they what the user are?
void user2gd(struct Multi_Int32* geoSystem, struct SFVec3d *geo, int n, struct SFVec3d *gd){
	//UNTESTED
	int i;
	struct SFVec3d gcCoord;
	for(i=0;i<n;i++)
		moveCoords3d(geoSystem,NULL,NULL,&geo[i],1,&gcCoord,&gd[i]);
}
void gd2user(struct Multi_Int32* geoSystem, struct SFVec3d *gd,  int n, struct SFVec3d *geo){
	//UNTESTED
	int i;
	struct SFVec3d gdCoord, gcCoord;
	for(i=0;i<n;i++){
		Gd_Gc3d(geoSystem,&gd[i],1,&gcCoord);
		gc2user(geoSystem,&gcCoord,1,&geo[i]);
	}
}
*/
void gd2gc(Geosys * geoSystem, struct SFVec3d *gd,  int n, struct SFVec3d *gc){
	//UNTESTED
	int i;
	for(i=0;i<n;i++){
		Gd_Gc3d(geoSystem,&gd[i],1,&gc[i]);
	}
}
void gc2gd(Geosys * geoSystem, struct SFVec3d *gc,  int n, struct SFVec3d *gd){
	//UNTESTED
	int i;
	for(i=0;i<n;i++){
		gccToGdc (geoSystem, &gc[i], &gd[i]);
	}
}
void do_GeoConvert (void *px){
	// web3d v3.3 specs missing a converter node - you can route between
	// geoNodes, but what if 2 nodes have different geoSystem?
	// this geoConvert node solves that, you create 2 of these and chain them:
	// myGeoNode1 -> set_geoCoord (geoConvert1) gcCoord_changed -> set_gcCoord (geoConvert2) geoCoord_changed -> myGeoNode2
	// where geoConvert1.geoSystem == myGeoNode1.geoSystem
	// and geoConvert2.geoSystem == myGeoNode2.geoSystem
	//
	// If we've done above nodes well, then any scene routing of geoCoords should be 
	// in so-called 'user coords' -as specified by the scene designer in geoSystem
	// for example longitude first, degrees etc.
	// That means we shouldn't see any LCS - the Local Coordinate System you get after taking off geoOrigin or AutoOrigin 
	// we'll just see full geo coords and full gc coords in routing

	struct X3D_GeoConvert *node;
	node = (struct X3D_GeoConvert *) px;
	if (!node) return;
	if(node->__geoSystem == NULL)
		compile_geoSystem (X3D_NODE(node),node->_nodeType, &node->geoSystem, &node->__geoSystem);

	if (!vecsamed(node->__oldgeoCoords.c,node->set_geoCoords.c)) {
		struct SFVec3d gdCoord;
		moveCoords3d(GEOSYS(node->__geoSystem),NULL,NULL,&node->set_geoCoords,1,&node->gcCoords_changed,&gdCoord);
		MARK_EVENT (px, offsetof (struct X3D_GeoConvert, gcCoords_changed));
		veccopyd(node->__oldgeoCoords.c,node->set_geoCoords.c);
	} 
	if (!vecsamed(node->__oldgcCoords.c,node->set_gcCoords.c)){
		struct SFVec3d gdCoord;
		CONVERT_BACK_TO_GD_OR_UTMC(GEOSYS(node->__geoSystem),NULL,&node->set_gcCoords,&gdCoord,&node->geoCoords_changed);
		MARK_EVENT (px, offsetof (struct X3D_GeoConvert, geoCoords_changed));
		veccopyd(node->__oldgcCoords.c,node->set_gcCoords.c);
	}
}