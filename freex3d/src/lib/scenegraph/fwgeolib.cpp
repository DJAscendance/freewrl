

// wrapper for geographicLib by Karney
/*
Karney/geographicLib could be a good way to verify independently that I didn't break something
From what I've read, Karney got accuracy in the nanometer range for UTM/GD GD/UTM
- his recent ie 2010 methods are refered to by others as the best way
- and he has a MIT lib with .hpp/.cpp (C++) conversions, called geographicLib, on sourceforge
- or do the whole geographicLib, which looks like it also has gravity and likely GD-GC,
-- and windows/linux, x86/x64 configs

KARNEY UTM
https://arxiv.org/pdf/1002.1417
https://www.uwgb.edu/dutchs/UsefulData/UTMFormulas.HTM
https://geographiclib.sourceforge.io
Karneys nanometer utm Geo converters
https://geographiclib.sourceforge.io/html/classGeographicLib_1_1TransverseMercator.html
/KARNEY UTM
TransverseMercator (real a, real f, real k0) 
void  Forward (real lon0, real lat, real lon, real &x, real &y, real &gamma, real &k) const 
void  Reverse (real lon0, real x, real y, real &lat, real &lon, real &gamma, real &k) const 
void  Forward (real lon0, real lat, real lon, real &x, real &y) const 
void  Reverse (real lon0, real x, real y, real &lat, real &lon) const 

*/

#ifdef HAVE_GEOLIB

#include <GeographicLib/TransverseMercator.hpp>
#include <GeographicLib/DMS.hpp>
#include <GeographicLib/Utility.hpp>
#include <GeographicLib/Geocentric.hpp>
using namespace GeographicLib;

extern "C" {
#include "fwgeolib.h"

void * fgeo_initializeTM(double a, double f, double scaleFactor){
	TransverseMercator * TMS = new TransverseMercator(a, f, scaleFactor);
	return (void*)TMS;
}
void fgeo_tm2gd(void *fgeo,double easting, double northing, double dlon0, double *dlat, double *dlon){
	double ddlat, ddlon;
	TransverseMercator * TMS = (TransverseMercator *)fgeo;
	TMS->Reverse(dlon0,easting,northing,ddlat,ddlon);
	*dlat = ddlat;
	*dlon = ddlon;
}
void fgeo_gd2tm(void *fgeo,double dlat, double dlon, double dlon0, double *easting, double *northing){
	double dx, dy;
	TransverseMercator * TMS = (TransverseMercator *)fgeo;
	TMS->Forward(dlon0,dlat,dlon,dx,dy);
	*easting = dx;
	*northing = dy;
}

void * fgeo_initializeGC(double a, double f){
	Geocentric * GC = new Geocentric(a, f);
	return (void*)GC;
}
void fgeo_gc2gd(void *fgeo,double X, double Y, double Z, double *dlat, double *dlon, double *dh){
	double ddlat, ddlon, ddh;
	Geocentric * GC = (Geocentric *)fgeo;
	GC->Reverse(X,Y,Z,ddlat,ddlon,ddh);
	*dlat = ddlat;
	*dlon = ddlon;
	*dh = ddh;
}
void fgeo_gd2gc(void *fgeo, double dlat, double dlon, double dh, double *X, double *Y, double *Z){
	double dx, dy, dz;
	Geocentric * GC = (Geocentric *)fgeo;
	GC->Forward(dlat, dlon, dh, dx,dy,dz);
	*X = dx;
	*Y = dy;
	*Z = dz;
}


}  //extern C

#endif //HAVE_GEOLIB