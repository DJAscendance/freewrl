/*


X3D Texturing Component

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
#include "../main/headers.h"
#include "../opengl/OpenGL_Utils.h"
#include "../opengl/Textures.h"
#include "../scenegraph/Component_Shape.h"
#include "../scenegraph/RenderFuncs.h"
#include "LinearAlgebra.h"


void render_PixelTexture (struct X3D_PixelTexture *node) {
	loadTextureNode(X3D_NODE(node),NULL);
	gglobal()->RenderFuncs.textureStackTop=1; /* not multitexture - should have saved to boundTextureStack[0] */
}


void render_ImageTexture (struct X3D_ImageTexture *node) {
	loadTextureNode(X3D_NODE(node),NULL); //이미지 텍스처
	
	gglobal()->RenderFuncs.textureStackTop=1; /* not multitexture - should have saved to boundTextureStack[0] */
}

//void render_ProjectiveTexture (struct X3D_ProjectiveTexture *node) {
//	struct projective_Texdata *data;
//	ttglobal tg = gglobal();
//	data = (struct projective_Texdata*)tg->ProjectiveTextures.data;
//	int i;
//	if(node->value)
//	{
//		for(i=0;i<4;i++)
//		{
//			//if(!strcmp(node->projectorName->strptr,tg->ProjectiveTextures.data[i].des->strptr))
//			if(!strcmp(node->projectorName->strptr,data[i].des->strptr))
//			{
//				
//				if(tg->ProjectiveTextures._projTexGenMatCam0_Location != 0 || tg->ProjectiveTextures._projViewMat_Location != 0 || tg->ProjectiveTextures._projMap_forCam1_Location != 0)
//				{
//					//convertDbtoFl(tg->ProjectiveTextures.data[i].TenLinearGexMat, TenLinearGexMatCam0);
//					convertDbtoFl(data[i].TenLinearGexMat, TenLinearGexMatCam0);
//					GLUNIFORMMATRIX4FV (tg->ProjectiveTextures._projTexGenMatCam0_Location,1,GL_FALSE, TenLinearGexMatCam0);
//				}
//				break;
//			}
//		}
//		tg->ProjectiveTextures.ProjActive = true;
//		loadTextureNode(X3D_NODE(node),NULL); //이미지 텍스처
//
//		gglobal()->RenderFuncs.textureStackTop=1; /* not multitexture - should have saved to boundTextureStack[0] */
//	}
//}

void render_MultiTexture (struct X3D_MultiTexture *node) {

	loadMultiTexture(node);
}

/*
void render_MultipleProjectiveTexture (struct X3D_MultipleProjectiveTexture *node) {

	struct projective_Texdata *data;
	ttglobal tg = gglobal();
	data = (struct projective_Texdata*)tg->ProjectiveTextures.data;
	int i;

	for(i=0;i<4;i++)
	{
		if(tg->ProjectiveTextures._MultiprojTexGenMatCam_Location[0] != 0 || 
			tg->ProjectiveTextures._MultiprojTexGenMatCam_Location[1] != 0 || 
			tg->ProjectiveTextures._MultiprojTexGenMatCam_Location[2] != 0 || 
			tg->ProjectiveTextures._MultiprojTexGenMatCam_Location[3] != 0
			)
		{
			//if(tg->ProjectiveTextures.data[i].des != NULL)
			if(data[i].des != NULL)
			{

				//convertDbtoFl(tg->ProjectiveTextures.data[i].TenLinearGexMat, TenLinearGexMatCam0);
				convertDbtoFl(data[i].TenLinearGexMat, TenLinearGexMatCam0);
				GLUNIFORMMATRIX4FV (tg->ProjectiveTextures._MultiprojTexGenMatCam_Location[i],1,GL_FALSE, TenLinearGexMatCam0);
			}
		}
		
	}

	loadMultiTexture(node);
}
*/
void render_AudioClip(struct X3D_AudioClip * node);
void render_MovieTexture (struct X3D_MovieTexture *node) {
	//july 2016 movietexture fields put in same order as audioclip, so can up-caste and delegate
	struct X3D_AudioClip *anode = (struct X3D_AudioClip*)node;
	render_AudioClip(anode); //just checks if loaded, schedules if not
	loadTextureNode(X3D_NODE(node),NULL);
	gglobal()->RenderFuncs.textureStackTop=1; /* not multitexture - should have saved to boundTextureStack[0] */

}

