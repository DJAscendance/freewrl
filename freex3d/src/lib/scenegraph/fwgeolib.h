#ifndef __FREEWRL_FWGEOLIB_H__
#define __FREEWRL_FWGEOLIB_H__


void * fgeo_initializeTM(double a, double f, double scaleFactor);
void fgeo_tm2gd(void *fgeo,double easting, double northing, double lon0, double *lat, double *lon);
void fgeo_gd2tm(void *fgeo,double lat, double lon, double lon0, double *easting, double *northing);
void * fgeo_initializeGC(double a, double f);
void fgeo_gc2gd(void *fgeo,double X, double Y, double Z, double *dlat, double *dlon, double *dh);
void fgeo_gd2gc(void *fgeo, double dlat, double dlon, double dh, double *X, double *Y, double *Z);
#endif /* __FREEWRL_FWGEOLIB_H__ */