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

void render_GeneratedTexture(struct X3D_GeneratedTexture* node) {
	// GeneratedTexture doesn't work (not finished) as of Sep 2023
	// not sure its needed. You could render your sub-scene as a separate scene, 
	// do a screenshot, save, trim and use as imagetexture.
	// It would take some work to get this working properly. 
	// References:
	// mainloop > contenttype_fbostage - simple plain method of rendering fbo creation and push/pop
	// Layout/layering > rendering a subscene with its own bindables stack (viewpoint etc), lights
	// GeneratedCubeMap - done with whole scene / from rootnode, but manages camera pose in scene
	// shadow mapping

	int count, iface;

	if (!strcmp(node->update->strptr, "ALWAYS") || !strcmp(node->update->strptr, "NEXT_FRAME_ONLY")) {
		ttrenderstate rs;
		rs = renderstate();
		if (rs->render_geom) {
			double modelviewmatrix[16];
			textureTableIndexStruct_s* tti;
			float vp[4] = { 0.0f,1.0f,0.0f,1.0f }; //arbitrary

			//compile_generatedcubemap - creates framebufferobject fbo
			tti = getTableIndex(node->__textureTableIndex);
			//tti->x = node->size.p[0];
			//tti->y = node->size.p[1];
			//tti->z = 1;
			//set in compile_
			static int once = 0;
			if (!once) {
				//loadTextureNode(X3D_NODE(node), NULL);
				tti->x = node->size.p[0];
				tti->y = node->size.p[1];
				glGenTextures(1, &tti->OpenGLTexture);
				//bind to set some parameters
				glBindTexture(GL_TEXTURE_2D, tti->OpenGLTexture);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tti->x, tti->y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
				//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
				//unbind - will rebind during render to reset width, height as needed
				glBindTexture(GL_TEXTURE_2D, 0);

				glGenFramebuffers(1, &tti->ifbobuffer);
				glBindFramebuffer(GL_FRAMEBUFFER, tti->ifbobuffer);

				// create a renderbuffer object to store depth info
				// NOTE: A depth renderable image should be attached the FBO for depth test.
				// If we don't attach a depth renderable image to the FBO, then
				// the rendering output will be corrupted because of missing depth test.
				// If you also need stencil test for your rendering, then you must
				// attach additional image to the stencil attachement point, too.
				glGenRenderbuffers(1, &tti->idepthbuffer);
				//bind to set some parameters
				glBindRenderbuffer(GL_RENDERBUFFER, tti->idepthbuffer);
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, tti->x, tti->y);
				//unbind
				glBindRenderbuffer(GL_RENDERBUFFER, 0);


				//pushnset_viewport(node->viewpoint); //something to push so we can pop-and-set below, so any mainloop GL_BACK viewport is restored
				glViewport(0, 0, tti->x, tti->y); //viewport we want 
				glEnable(GL_TEXTURE_GEN_S);
				glEnable(GL_TEXTURE_GEN_T);
				//glEnable(GL_TEXTURE_GEN_R);
				PRINT_GL_ERROR_IF_ANY("render_GeneratedTexture, after set framebuffer");

				//create fbo or fbo tiles collection for generatedcubemap
				//method: we draw each face to a single framebuffer texture, 
				if (tti->OpenGLTexture == 0) glGenTextures(1, &tti->OpenGLTexture);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tti->OpenGLTexture, 0);
				// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glFramebufferTexture.xhtml
				PRINT_GL_ERROR_IF_ANY("render_GeneratedTexture, after glFramebufferTexture2D");

				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, tti->idepthbuffer);
				//unbind framebuffer till render
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				once = 1;
			}
			pushnset_framebuffer(tti->ifbobuffer); //binds framebuffer. we push here, in case higher up we are already rendering the whole scene to an fbo

			//glClearColor(1.0f,0.0f,0.0f,1.0f); //red, for diagnostics during debugging
			glClearColor(1.0f, 0.0f, 0.0f, 0.0f); //transparent, so results can be blended
			FW_GL_CLEAR(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//set viewpoint matrix for side
			//setup_projection(); 
			FW_GL_MATRIX_MODE(GL_PROJECTION);
			FW_GL_LOAD_IDENTITY();
			//fw_gluPerspective(90.0, 1.0, .1,10000.0);
			fw_gluPerspective_2(0.0, 90.0, 1.0, .1, 10000.0);

			FW_GL_MATRIX_MODE(GL_MODELVIEW);
			FW_GL_LOAD_IDENTITY();
			//fw_glSetDoublev(GL_MODELVIEW_MATRIX, modelviewmatrix);
			//fw_glRotated(sideangle[j].angle, sideangle[j].x, sideangle[j].y, sideangle[j].z);
			//fw_glScaled(1.0, -1.0, 1.0);
			//fw_glGetDoublev(GL_MODELVIEW_MATRIX, bstack->viewmatrix);

//			lightTable_clear();

//			render_bound_background();

			/*  Other lights*/
			PRINT_GL_ERROR_IF_ANY("render_GeneratedTexture, before render_hier");

			//render_hier(X3D_NODE(node), VF_globalLight);
			PRINT_GL_ERROR_IF_ANY("XEvents::render, after render_hier(VF_globalLight)");
			//render_hier(X3D_NODE(node), VF_Other);

			/*  4. Nodes (not the blended ones)*/
			profile_start("hier_geom");
			push_globalRenderFlags();
			//render_hier(X3D_NODE(node), VF_Geom);
			//render_node(X3D_NODE(node));
			normalChildren(node->children);
			pop_globalRenderFlags();

			profile_end("hier_geom");
			PRINT_GL_ERROR_IF_ANY("render_GeneratedTexture, after render_hier(VF_Geom)");

			///*  5. Blended Nodes*/
			//if (tg->RenderFuncs.have_transparency) {
			//	/*  render the blended nodes*/
			//	render_hier(rootNode(), VF_Geom | VF_Blend );
			//	PRINT_GL_ERROR_IF_ANY("XEvents::render, render_hier(VF_Geom)");
			//}

			//popnset_viewport();
			int status = glCheckNamedFramebufferStatus(tti->ifbobuffer, GL_FRAMEBUFFER);
			printFramebufferStatusIfNotComplete(status);
			popnset_framebuffer();
			if (0) {
				//set_debug_quad(1, tti->OpenGLTexture);
				set_debug_quad(2, tti->ifbobuffer);
			}

		}
	}
	//render what we have now
	gglobal()->RenderFuncs.textureStackTop = 1;
	gglobal()->RenderFuncs.texturenode = node;

}


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

