/*
  GLCoreCompat.c - run FreeWRL on a macOS OpenGL 4.1 core profile context.

  The library was written against compatibility profiles (desktop GL on Windows/Linux)
  and GLES2. A core profile additionally rejects:
  - vertex attribute and index data in client memory (no buffer bound): the HUD, Text,
    cursor, Box, lines, points, 2D geometry and particles still draw that way. We keep the
    client pointers and stream them into buffer objects at draw time.
  - a few GL 4.3+/4.5 calls macOS does not have (it stops at 4.1).
  - GL_LUMINANCE_ALPHA textures (font and HUD atlases): uploaded as GL_RG8 with a swizzle.

  display.h redirects the gl* names to the fw_core_* functions below when
  FW_GL_CORE_PROFILE is defined (macOS desktop only), so this file does not include it.
*/
#include <config.h>
#if defined(AQUA) && !defined(IPHONE)
#include <OpenGL/gl3.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FW_CORE_MAX_ATTRIBS 16

/* state checks that cost a glGet (a pipeline sync on macOS): Debug builds only */
#ifdef DEBUG
#define core_check(cond) assert(cond)
#else
#define core_check(cond) ((void)0)
#endif

typedef struct {
	const void *pointer; /* client memory, or NULL when the attribute is sourced from a VBO */
	GLint size;
	GLenum type;
	GLboolean normalized;
	GLsizei stride;
	int integer;         /* set with glVertexAttribIPointer */
	int enabled;
	GLuint stream_vbo;
} client_attrib;

/* one GL context (the frontend's) uses the library at a time on macOS */
static client_attrib attribs[FW_CORE_MAX_ATTRIBS];
static GLuint stream_ibo = 0;

static GLsizei type_size(GLenum type){
	switch(type){
	case GL_BYTE: case GL_UNSIGNED_BYTE: return 1;
	case GL_SHORT: case GL_UNSIGNED_SHORT: case GL_HALF_FLOAT: return 2;
	case GL_DOUBLE: return 8;
	default: return 4; /* GL_FLOAT, GL_INT, GL_UNSIGNED_INT, GL_FIXED */
	}
}
static GLuint bound(GLenum binding){
	GLint b = 0;
	glGetIntegerv(binding, &b);
	return (GLuint)b;
}

static void set_pointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer, int integer){
	if(index >= FW_CORE_MAX_ATTRIBS) return;
	client_attrib *a = &attribs[index];
	if(bound(GL_ARRAY_BUFFER_BINDING) || pointer == NULL){
		/* a buffer object is bound: pointer is an offset, core handles it */
		a->pointer = NULL;
		if(integer) glVertexAttribIPointer(index, size, type, stride, pointer);
		else glVertexAttribPointer(index, size, type, normalized, stride, pointer);
		return;
	}
	/* client memory: remember it, upload when we know how many vertices a draw reads */
	a->pointer = pointer;
	a->size = size;
	a->type = type;
	a->normalized = normalized;
	a->stride = stride;
	a->integer = integer;
}
void fw_core_glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer){
	set_pointer(index, size, type, normalized, stride, pointer, 0);
}
void fw_core_glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer){
	set_pointer(index, size, type, GL_FALSE, stride, pointer, 1);
}
void fw_core_glEnableVertexAttribArray(GLuint index){
	if(index < FW_CORE_MAX_ATTRIBS) attribs[index].enabled = 1;
	glEnableVertexAttribArray(index);
}
void fw_core_glDisableVertexAttribArray(GLuint index){
	if(index < FW_CORE_MAX_ATTRIBS) attribs[index].enabled = 0;
	glDisableVertexAttribArray(index);
}

/* stream every enabled client-memory attribute covering vertices [0, nverts) into a VBO */
static void upload_client_attribs(GLsizei nverts){
	GLuint prev = 0;
	int any = 0;
	for(int i=0;i<FW_CORE_MAX_ATTRIBS;i++){
		client_attrib *a = &attribs[i];
		if(!a->enabled || !a->pointer || nverts <= 0) continue;
		if(!any){ prev = bound(GL_ARRAY_BUFFER_BINDING); any = 1; }
		GLsizei elem = a->size * type_size(a->type);
		GLsizei stride = a->stride ? a->stride : elem;
		GLsizeiptr bytes = (GLsizeiptr)stride * (nverts - 1) + elem;
		if(!a->stream_vbo) glGenBuffers(1, &a->stream_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, a->stream_vbo);
		glBufferData(GL_ARRAY_BUFFER, bytes, a->pointer, GL_STREAM_DRAW);
		if(a->integer) glVertexAttribIPointer(i, a->size, a->type, a->stride, 0);
		else glVertexAttribPointer(i, a->size, a->type, a->normalized, a->stride, 0);
	}
	if(any) glBindBuffer(GL_ARRAY_BUFFER, prev);
}
static int have_client_attribs(void){
	for(int i=0;i<FW_CORE_MAX_ATTRIBS;i++)
		if(attribs[i].enabled && attribs[i].pointer) return 1;
	return 0;
}

void fw_core_glDrawArrays(GLenum mode, GLint first, GLsizei count){
	if(have_client_attribs()) upload_client_attribs(first + count);
	glDrawArrays(mode, first, count);
}

static GLuint max_index(GLenum type, const void *indices, GLsizei count){
	GLuint m = 0;
	for(GLsizei i=0;i<count;i++){
		GLuint v = type == GL_UNSIGNED_BYTE ? ((const GLubyte*)indices)[i]
			: type == GL_UNSIGNED_SHORT ? ((const GLushort*)indices)[i]
			: ((const GLuint*)indices)[i];
		if(v > m) m = v;
	}
	return m;
}
void fw_core_glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices){
	GLuint ebo = bound(GL_ELEMENT_ARRAY_BUFFER_BINDING);
	if(ebo){
		/* indices already in a buffer object; client attributes need the index range */
		if(have_client_attribs()){
			GLsizei isz = type_size(type);
			void *tmp = malloc((size_t)count * isz);
			glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER, (GLintptr)indices, (GLsizeiptr)count * isz, tmp);
			upload_client_attribs(max_index(type, tmp, count) + 1);
			free(tmp);
		}
		glDrawElements(mode, count, type, indices);
		return;
	}
	if(!indices || count <= 0) return;
	if(have_client_attribs()) upload_client_attribs(max_index(type, indices, count) + 1);
	if(!stream_ibo) glGenBuffers(1, &stream_ibo);
	/* GL_ELEMENT_ARRAY_BUFFER binding is VAO state: put 0 back so the caller sees no change */
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stream_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)count * type_size(type), indices, GL_STREAM_DRAW);
	glDrawElements(mode, count, type, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/* A core profile rejects a draw (GL_INVALID_OPERATION) when samplers of different types
   (sampler2D, samplerCube, ...) in the program point at the same texture unit. The
   ubershader declares sampler2D textureUnit[16] and samplerCube textureUnitCube[8]; all start
   at unit 0 and FreeWRL only sets the ones a shape uses, leaving the rest at 0 or at the
   previous shape's units. Compatibility drivers ignore that. So at the start of each shape
   (clear_textureUnit_used), park every sampler of the current program on a spare unit
   reserved for its type, from the top unit down; the shape then assigns units from 0 up. */
#define FW_CORE_MAX_PROGRAMS 256
#define FW_CORE_MAX_SAMPLERS 64
typedef struct {
	GLuint program;
	int n;
	GLint location[FW_CORE_MAX_SAMPLERS];
	GLint unit[FW_CORE_MAX_SAMPLERS];
} program_samplers;
static program_samplers sampler_cache[FW_CORE_MAX_PROGRAMS];
static int nsampler_cache = 0;

static int is_sampler(GLenum type){
	switch(type){
	case GL_SAMPLER_1D: case GL_SAMPLER_2D: case GL_SAMPLER_3D: case GL_SAMPLER_CUBE:
	case GL_SAMPLER_1D_SHADOW: case GL_SAMPLER_2D_SHADOW: case GL_SAMPLER_CUBE_SHADOW:
	case GL_SAMPLER_2D_ARRAY: case GL_SAMPLER_2D_ARRAY_SHADOW: case GL_SAMPLER_2D_RECT:
	case GL_SAMPLER_BUFFER: case GL_INT_SAMPLER_2D: case GL_UNSIGNED_INT_SAMPLER_2D:
		return 1;
	}
	return 0;
}
static program_samplers *samplers_of(GLuint program){
	for(int i=0;i<nsampler_cache;i++)
		if(sampler_cache[i].program == program) return &sampler_cache[i];
	if(nsampler_cache == FW_CORE_MAX_PROGRAMS){
		static int warned = 0;
		if(!warned++) fprintf(stderr, "GLCoreCompat: more than %d shader programs, samplers of the rest are not parked (draws may fail)\n", FW_CORE_MAX_PROGRAMS);
		return NULL;
	}
	program_samplers *ps = &sampler_cache[nsampler_cache++];
	GLenum types[16]; int ntypes = 0;
	GLint nuniforms = 0, maxunits = 16;
	ps->program = program;
	ps->n = 0;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxunits);
	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &nuniforms);
	for(GLint u=0;u<nuniforms;u++){
		char name[256];
		GLint size; GLenum type; int t;
		glGetActiveUniform(program, u, sizeof(name), NULL, &size, &type, name);
		if(!is_sampler(type)) continue;
		for(t=0;t<ntypes;t++) if(types[t] == type) break;
		if(t == ntypes && ntypes < 16) types[ntypes++] = type;
		char *bracket = strchr(name, '[');
		if(bracket) *bracket = 0;
		if(ps->n + size > FW_CORE_MAX_SAMPLERS){
			static int warned = 0;
			if(!warned++) fprintf(stderr, "GLCoreCompat: program %u has more than %d samplers, the rest are not parked\n", program, FW_CORE_MAX_SAMPLERS);
		}
		for(GLint e=0;e<size && ps->n<FW_CORE_MAX_SAMPLERS;e++){
			char elem[300];
			if(size > 1) snprintf(elem, sizeof(elem), "%s[%d]", name, e);
			else snprintf(elem, sizeof(elem), "%s", name);
			GLint loc = glGetUniformLocation(program, elem);
			if(loc < 0) continue;
			ps->location[ps->n] = loc;
			ps->unit[ps->n] = maxunits - 1 - t;
			ps->n++;
		}
	}
	return ps;
}
void fw_core_park_samplers(void){
	GLint program = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &program);
	if(!program) return;
	program_samplers *ps = samplers_of((GLuint)program);
	if(!ps) return;
	for(int i=0;i<ps->n;i++) glUniform1i(ps->location[i], ps->unit[i]);
}

/* fixed-function capabilities the library still toggles (texturing, fog, texgen);
   shaders do that work, and a core profile rejects the enums with GL_INVALID_ENUM */
static int fixed_function_cap(GLenum cap){
	switch(cap){
	case 0x0DE1: /* GL_TEXTURE_2D */
	case 0x8513: /* GL_TEXTURE_CUBE_MAP */
	case 0x0B60: /* GL_FOG */
	case 0x0C60: case 0x0C61: case 0x0C62: /* GL_TEXTURE_GEN_S/T/R */
		return 1;
	}
	return 0;
}
void fw_core_glEnable(GLenum cap){
	if(!fixed_function_cap(cap)) glEnable(cap);
}
void fw_core_glDisable(GLenum cap){
	if(!fixed_function_cap(cap)) glDisable(cap);
}

/* LineProperties asks for widths above 1 (linewidthScaleFactor, and 10 for its patterned
   line types, whose shader discards fragments to shape the line). A core profile only
   guarantees width 1 and raises GL_INVALID_VALUE above its range, so clamp to what the
   driver reports: lines draw 1 pixel wide and keep their pattern. Warn once. */
void fw_core_glLineWidth(GLfloat width){
	static GLfloat range[2] = { 0.0f, 0.0f };
	static int warned = 0;
	if(range[1] <= 0.0f){
		GLint flags = 0;
		glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, range);
		/* a forward-compatible context (macOS core is one) rejects any width above 1,
		   whatever range it reports */
		glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
		if(flags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT) range[1] = 1.0f;
		if(range[1] < 1.0f) range[1] = 1.0f;
		if(range[0] <= 0.0f || range[0] > 1.0f) range[0] = 1.0f;
	}
	if(width > range[1]){
		if(!warned++) fprintf(stderr, "GLCoreCompat: line width %g requested, this OpenGL core context supports up to %g; wide lines draw at %g\n", width, range[1], range[1]);
		width = range[1];
	}else if(width < range[0]){
		width = range[0];
	}
	glLineWidth(width);
}

/* GL 4.5 glBindTextureUnit(unit, 0): unbind every target on the unit, active unit unchanged.
   The library only calls it with 0 and binds only 2D, cube map and 3D textures. A non-zero
   texture would need its target, which GL 4.1 cannot query, so that is not supported. */
void fw_core_glBindTextureUnit(GLuint unit, GLuint texture){
	GLint prev = GL_TEXTURE0;
	assert(texture == 0);
	glGetIntegerv(GL_ACTIVE_TEXTURE, &prev);
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glBindTexture(GL_TEXTURE_3D, 0);
	glActiveTexture(prev);
	core_check(bound(GL_ACTIVE_TEXTURE) == (GLuint)prev);
}
/* GL 4.5 direct-state query. GL 4.1 cannot ask a texture name for its target, so this is
   not implementable in general. The library only calls it inside if(0) debug prints; if a
   caller ever becomes live, say so instead of silently answering. */
void fw_core_glGetTextureParameteriv(GLuint texture, GLenum pname, GLint *params){
	static int warned = 0;
	if(!warned++) fprintf(stderr, "GLCoreCompat: glGetTextureParameteriv(%u, 0x%x) is GL 4.5, unsupported on macOS\n", texture, pname);
	core_check(!"glGetTextureParameteriv reached on a GL 4.1 core context");
	*params = 0;
}
/* GL 4.5: check an FBO without disturbing the current bindings. GL_FRAMEBUFFER binds both
   the draw and the read framebuffer, so both are put back. */
GLenum fw_core_glCheckNamedFramebufferStatus(GLuint framebuffer, GLenum target){
	GLuint prev_draw = bound(GL_DRAW_FRAMEBUFFER_BINDING);
	GLuint prev_read = bound(GL_READ_FRAMEBUFFER_BINDING);
	GLenum status;
	glBindFramebuffer(target, framebuffer);
	status = glCheckFramebufferStatus(target);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prev_draw);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, prev_read);
	core_check(bound(GL_DRAW_FRAMEBUFFER_BINDING) == prev_draw && bound(GL_READ_FRAMEBUFFER_BINDING) == prev_read);
	return status;
}
/* GL 4.3; invalidation is a hint, so doing nothing is a valid implementation.
   Only reached from HAnim GPU skinning cleanup, which needs SSBOs and is off on macOS. */
void fw_core_glInvalidateBufferData(GLuint buffer){
	(void)buffer;
}
/* GL_ALPHA, GL_LUMINANCE and GL_LUMINANCE_ALPHA are not core texture formats (font and
   HUD atlases, grey images). Store them as GL_R8 / GL_RG8 and swizzle so shaders sample
   the same values as before: alpha (0,0,0,A), luminance (L,L,L,1), lum-alpha (L,L,L,A). */
#define FW_LEGACY_ALPHA            0x1906
#define FW_LEGACY_LUMINANCE        0x1909
#define FW_LEGACY_LUMINANCE_ALPHA  0x190A
void fw_core_glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height,
	GLint border, GLenum format, GLenum type, const void *pixels){
	static const GLint sw_alpha[4] = { GL_ZERO, GL_ZERO, GL_ZERO, GL_RED };
	static const GLint sw_lum[4] = { GL_RED, GL_RED, GL_RED, GL_ONE };
	static const GLint sw_lumalpha[4] = { GL_RED, GL_RED, GL_RED, GL_GREEN };
	static const GLint sw_identity[4] = { GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA };
	const GLint *swizzle = sw_identity;
	GLenum swizzle_target = 0;
	switch(format){
	case FW_LEGACY_ALPHA:           swizzle = sw_alpha;    internalformat = GL_R8;  format = GL_RED; break;
	case FW_LEGACY_LUMINANCE:       swizzle = sw_lum;      internalformat = GL_R8;  format = GL_RED; break;
	case FW_LEGACY_LUMINANCE_ALPHA: swizzle = sw_lumalpha; internalformat = GL_RG8; format = GL_RG;  break;
	}
	glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
	/* the swizzle is texture-object state: set it on every level-0 upload, identity for
	   normal formats, so a texture name reused for RGBA data does not keep a legacy swizzle */
	if(target == GL_TEXTURE_2D) swizzle_target = GL_TEXTURE_2D;
	else if(target >= GL_TEXTURE_CUBE_MAP_POSITIVE_X && target <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z) swizzle_target = GL_TEXTURE_CUBE_MAP;
	if(swizzle_target && level == 0)
		glTexParameteriv(swizzle_target, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
}
#endif /* AQUA && !IPHONE */
