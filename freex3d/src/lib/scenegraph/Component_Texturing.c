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
#include "../scenegraph/Polyrep.h"
#include "LinearAlgebra.h"


void render_PixelTexture (struct X3D_PixelTexture *node) {
	loadTextureNode(X3D_NODE(node),NULL);
	gglobal()->RenderFuncs.textureStackTop=1; /* not multitexture - should have saved to boundTextureStack[0] */
}
void* set_TextureRep(void* _texrep) {
	struct X3D_TextureRep* texrep = NULL;
	//to be called from compile_BufferTexture
	if (!_texrep) {
		_texrep = MALLOC(struct X3D_TextureRep*, sizeof(struct X3D_TextureRep));
		memset(_texrep, 0, sizeof(struct X3D_TextureRep));
	}
	texrep = (struct X3D_TextureRep*)_texrep;
	texrep->itype = 4; //0 meshrep 1 linerep 2 polyrep 3 meshrep 4 texturerep
	return texrep;
}
void render_BufferTexture(struct X3D_BufferTexture* node) {
	//check if buffer loaded
	if (node->_intern) {
		struct X3D_TextureRep* tr = (struct X3D_TextureRep*)node->_intern;
		if (tr->buffer->loaded) {
			if (1) {
				loadTextureNode(X3D_NODE(node), NULL);
				gglobal()->RenderFuncs.textureStackTop = 1; /* not multitexture - should have saved to boundTextureStack[0] */
			}
		}
	}
}

void render_ImageTexture (struct X3D_ImageTexture *node) {
	if (node->autoRefresh > 0.0) {
		double dtime = TickTime();
		double elapsedTime = dtime - node->__lasttime;
		double runtime = dtime - BrowserStartTime();
		if (elapsedTime > node->autoRefresh && runtime < node->autoRefreshTimeLimit) {
			node->__lasttime = dtime;
			textureTableIndexStruct_s* tti;
			tti = getTableTableFromTextureNode(X3D_NODE(node));
			tti->status = TEX_NOTLOADED;
		}
	}

	loadTextureNode(X3D_NODE(node),NULL); //이미지 텍스처
	
	gglobal()->RenderFuncs.textureStackTop=1; /* not multitexture - should have saved to boundTextureStack[0] */
}

void render_MultiTexture (struct X3D_MultiTexture *node) {

	loadMultiTexture(node);
}

void render_AudioClip(struct X3D_AudioClip * node);
void render_MovieTexture (struct X3D_MovieTexture *node) {
	//july 2016 movietexture fields put in same order as audioclip, so can up-caste and delegate
	struct X3D_AudioClip *anode = (struct X3D_AudioClip*)node;
	render_AudioClip(anode); //just checks if loaded, schedules if not
	if (node->autoRefresh > 0.0) {
		double dtime = TickTime();
		double elapsedTime = dtime - node->__lasttime;
		double runtime = dtime - BrowserStartTime();
		if (elapsedTime > node->autoRefresh && runtime < node->autoRefreshTimeLimit) {
			node->__lasttime = dtime;
			textureTableIndexStruct_s* tti;
			tti = getTableTableFromTextureNode(X3D_NODE(node));
			tti->status = TEX_NOTLOADED;
		}
	}

	loadTextureNode(X3D_NODE(node),NULL);
	gglobal()->RenderFuncs.textureStackTop=1; /* not multitexture - should have saved to boundTextureStack[0] */

}

