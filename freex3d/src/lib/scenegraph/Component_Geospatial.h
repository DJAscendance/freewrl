/*


Proximity sensor macro.

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



#ifndef __FREEWRL_SCENEGRAPH_GEOSPATIAL_H__
#define __FREEWRL_SCENEGRAPH_GEOSPATIAL_H__



int checkX3DGeoElevationGridFields (struct X3D_GeoElevationGrid *node, float **points, int *npoints);
void compile_geoSystem (struct X3D_Node *, int nodeType, struct Multi_String *args, struct X3D_Node **srf);
typedef struct _geosys Geosys;
#define GEOSYS( geosystem ) ((Geosys *)geosystem)
void update_origin(Geosys *geoSystem, struct X3D_Node *node, struct SFVec3d *userCoord, struct X3D_GeoOrigin *geoOrigin);

//void user2gd(Geosys * geoSystem, struct SFVec3d *geo, int n, struct SFVec3d *gd);
//void gd2user(Geosys * geoSystem, struct SFVec3d *gd,  int n, struct SFVec3d *geo);
void user2gc(Geosys * geoSystem, struct SFVec3d *geo, int n, struct SFVec3d *gc);
void gc2user(Geosys * geoSystem, struct SFVec3d *gc,  int n, struct SFVec3d *geo);
void  gc2lcs(Geosys * geoSystem, struct SFVec3d *gc,  int n, struct SFVec3d *lcs);
void  lcs2gc(Geosys * geoSystem, struct SFVec3d *lcs, int n, struct SFVec3d *gc);
void   gd2gc(Geosys * geoSystem, struct SFVec3d *gd,  int n, struct SFVec3d *gc);
void   gc2gd(Geosys * geoSystem, struct SFVec3d *gc,  int n, struct SFVec3d *gd);
void  gc2tcs(Geosys * geoSystem, struct SFVec3d *gdcenter, struct SFVec3d *gc,  int n, struct SFVec3d *tcs);
void  tcs2gc(Geosys * geoSystem, struct SFVec3d *gdcenter, struct SFVec3d *tcs, int n, struct SFVec3d *gc);
void lcs2gc_transform(struct SFVec4d *rotation, struct SFVec3d *translation);
void gc2lcs_transform(struct SFVec3d *translate, struct SFVec4d *rotate);
void  gc2tcs_transform(Geosys * geoSystem, struct SFVec3d *gdcenter, struct SFVec3d *translate, struct SFVec4d *rotate);
void  tcs2gc_transform(Geosys * geoSystem, struct SFVec3d *gdcenter, struct SFVec4d *rotate, struct SFVec3d *translate);
void geoprep(Geosys *geoSystem, struct SFVec3d *userCoord);
void geofin(Geosys *geoSystem, struct SFVec3d *userCoord);
void geoprepT(Geosys *geoSystem, struct SFVec3d *userCoord);
void geofinT(Geosys *geoSystem, struct SFVec3d *userCoord);

#endif /* __FREEWRL_SCENEGRAPH_GEOSPATIAL_H__ */
