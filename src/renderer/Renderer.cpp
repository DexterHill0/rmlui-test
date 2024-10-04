#include "Renderer.h"
#include "Geode/cocos/shaders/CCGLProgram.h"
#include "Geode/loader/Log.hpp"
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/FileInterface.h>
#include <RmlUi/Core/Log.h>
#include <RmlUi/Core/Platform.h>
#include <string.h>

#include <Geode/Geode.hpp>
#include <cocos2d.h>

#if defined RMLUI_PLATFORM_WIN32
	#include "RmlUi_Include_Windows.h"
	#include <gl/Gl.h>
	#include <gl/Glu.h>
#elif defined RMLUI_PLATFORM_MACOSX
	#include <AGL/agl.h>
	#include <OpenGL/gl.h>
	#include <OpenGL/glext.h>
	#include <OpenGL/glu.h>
#elif defined RMLUI_PLATFORM_UNIX
	#include "RmlUi_Include_Xlib.h"
	#include <GL/gl.h>
	#include <GL/glext.h>
	#include <GL/glu.h>
	#include <GL/glx.h>
#endif

#define GL_CLAMP_TO_EDGE 0x812F

RenderInterface_GD::RenderInterface_GD() {}

void RenderInterface_GD::SetViewport(int in_viewport_width, int in_viewport_height)
{
	viewport_width = in_viewport_width;
	viewport_height = in_viewport_height;
}

void RenderInterface_GD::BeginFrame()
{

	RMLUI_ASSERT(viewport_width >= 0 && viewport_height >= 0);

    // GLint previous_shader_program;
    // glGetIntegerv(GL_CURRENT_PROGRAM, &previous_shader_program);  // Save the current shader program
    // shader_program = previous_shader_program;
    // glUseProgram(0);

    auto* shader = cocos2d::CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTextureColor);
	shader->use();
	shader->setUniformsForBuiltins();

	glViewport(0, 0, viewport_width, viewport_height);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);  // Disable face culling for 2D rendering

    Rml::Matrix4f projection = Rml::Matrix4f::ProjectOrtho(0, (float)viewport_width, (float)viewport_height, 0, -10000, 10000);
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity(); // < i add this but it do nothing
    glLoadMatrixf(projection.data());
    // glOrtho(0.0f, (float)viewport_width, (float)viewport_height, 0.0f, -1.0f, 1.0f);
    
    // glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	transform_enabled = false;
}

void RenderInterface_GD::EndFrame() {
    glBlitFramebuffer(0, 0, viewport_width, viewport_height, 0, 0, viewport_width, viewport_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void RenderInterface_GD::FinaliseFrame() {
    // glUseProgram(shader_program);
}

void RenderInterface_GD::Clear()
{
	glClearStencil(0);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

Rml::CompiledGeometryHandle RenderInterface_GD::CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices)
{
	GeometryView* data = new GeometryView{vertices, indices};
	return reinterpret_cast<Rml::CompiledGeometryHandle>(data);
}

void RenderInterface_GD::ReleaseGeometry(Rml::CompiledGeometryHandle geometry)
{
	delete reinterpret_cast<GeometryView*>(geometry);
}


void RenderInterface_GD::RenderGeometry(Rml::CompiledGeometryHandle handle, Rml::Vector2f translation, Rml::TextureHandle texture)
{
    const GeometryView* geometry = reinterpret_cast<GeometryView*>(handle);
	const Rml::Vertex* vertices = geometry->vertices.data();
	const int* indices = geometry->indices.data();
	const int num_indices = (int)geometry->indices.size();

    // glDisable(GL_TEXTURE_2D);

	glPushMatrix();
	glTranslatef(translation.x, translation.y, 0);

    // 	glVertexPointer(2, GL_FLOAT, sizeof(Rml::Vertex), &vertices[0].position);
	// glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Rml::Vertex), &vertices[0].colour);


    glEnableVertexAttribArray(cocos2d::kCCVertexAttrib_Position);
	glVertexAttribPointer(cocos2d::kCCVertexAttrib_Position, 2, GL_FLOAT, GL_FALSE, sizeof(Rml::Vertex), &vertices[0].position);

	glEnableVertexAttribArray(cocos2d::kCCVertexAttrib_Color);
	glVertexAttribPointer(cocos2d::kCCVertexAttrib_Color, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Rml::Vertex), &vertices[0].colour);

	if (!texture)
	{
		glDisable(GL_TEXTURE_2D);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
	else
	{
		glEnable(GL_TEXTURE_2D);

		if (texture != TextureEnableWithoutBinding)
			glBindTexture(GL_TEXTURE_2D, (GLuint)texture);

		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		// glTexCoordPointer(2, GL_FLOAT, sizeof(Rml::Vertex), &vertices[0].tex_coord);

        glEnableVertexAttribArray(cocos2d::kCCVertexAttrib_TexCoords);
	    glVertexAttribPointer(cocos2d::kCCVertexAttrib_TexCoords, 2, GL_FLOAT, GL_FALSE, sizeof(Rml::Vertex), &vertices[0].tex_coord);
	}

	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, indices);

	glPopMatrix();
}

void RenderInterface_GD::EnableScissorRegion(bool enable)
{
	if (enable)
	{
		if (!transform_enabled)
		{
			glEnable(GL_SCISSOR_TEST);
			glDisable(GL_STENCIL_TEST);
		}
		else
		{
			glDisable(GL_SCISSOR_TEST);
			glEnable(GL_STENCIL_TEST);
		}
	}
	else
	{
		glDisable(GL_SCISSOR_TEST);
		glDisable(GL_STENCIL_TEST);
	}
}

void RenderInterface_GD::EnableClipMask(bool enable)
{
	if (enable)
		glEnable(GL_STENCIL_TEST);
	else
		glDisable(GL_STENCIL_TEST);
}

void RenderInterface_GD::RenderToClipMask(Rml::ClipMaskOperation operation, Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation)
{
    geode::log::debug("RenderInterface_GD::RenderToClipMask");

	RMLUI_ASSERT(glIsEnabled(GL_STENCIL_TEST));
	using Rml::ClipMaskOperation;

	const bool clear_stencil = (operation == ClipMaskOperation::Set || operation == ClipMaskOperation::SetInverse);
	if (clear_stencil)
	{
		// @performance Increment the reference value instead of clearing each time.
		glClear(GL_STENCIL_BUFFER_BIT);
	}

	GLint stencil_test_value = 0;
	glGetIntegerv(GL_STENCIL_REF, &stencil_test_value);

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glStencilFunc(GL_ALWAYS, GLint(1), GLuint(-1));

	switch (operation)
	{
	case ClipMaskOperation::Set:
	{
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		stencil_test_value = 1;
	}
	break;
	case ClipMaskOperation::SetInverse:
	{
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		stencil_test_value = 0;
	}
	break;
	case ClipMaskOperation::Intersect:
	{
		glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
		stencil_test_value += 1;
	}
	break;
	}

	RenderGeometry(geometry, translation, {});

	// Restore state
	// @performance Cache state so we don't toggle it unnecessarily.
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glStencilFunc(GL_EQUAL, stencil_test_value, GLuint(-1));
}

cocos2d::CCPoint frameToCocos(int x, int y) {
	auto* director = cocos2d::CCDirector::sharedDirector();
	const auto frameSize = director->getOpenGLView()->getFrameSize() * geode::utils::getDisplayFactor();
	const auto winSize = director->getWinSize();

	return {
		x / frameSize.width * winSize.width,
		(1.f - y / frameSize.height) * winSize.height
	};
}


void RenderInterface_GD::SetScissorRegion(Rml::Rectanglei region)
{
    // i took this one from mats imgui cocos
    // dont know if this is doing the correct things
    // const auto orig = frameToCocos(x, y);
    // const auto end = frameToCocos(width, height);
    // cocos2d::CCDirector::sharedDirector()->getOpenGLView()->setScissorInPoints(orig.x, end.y, end.x - orig.x, orig.y - end.y);

    // this is the original code
    glScissor(region.Left(), viewport_height - region.Bottom(), region.Width(), region.Height());
}

void RenderInterface_GD::SetTransform(const Rml::Matrix4f* transform)
{
    geode::log::debug("RenderInterface_GD::SetTransform");

	transform_enabled = (transform != nullptr);

	if (transform)
	{
		if (std::is_same<Rml::Matrix4f, Rml::ColumnMajorMatrix4f>::value)
			glLoadMatrixf(transform->data());
		else if (std::is_same<Rml::Matrix4f, Rml::RowMajorMatrix4f>::value)
			glLoadMatrixf(transform->Transpose().data());
	}
	else
		glLoadIdentity();
}

// Set to byte packing, or the compiler will expand our struct, which means it won't read correctly from file
#pragma pack(1)
struct TGAHeader {
	char idLength;
	char colourMapType;
	char dataType;
	short int colourMapOrigin;
	short int colourMapLength;
	char colourMapDepth;
	short int xOrigin;
	short int yOrigin;
	short int width;
	short int height;
	char bitsPerPixel;
	char imageDescriptor;
};
// Restore packing
#pragma pack()

Rml::TextureHandle RenderInterface_GD::LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source)
{   
    geode::log::debug("RenderInterface_GD::LoadTexture");

	Rml::FileInterface* file_interface = Rml::GetFileInterface();
	Rml::FileHandle file_handle = file_interface->Open(source);
	if (!file_handle)
	{
		return false;
	}

	file_interface->Seek(file_handle, 0, SEEK_END);
	size_t buffer_size = file_interface->Tell(file_handle);
	file_interface->Seek(file_handle, 0, SEEK_SET);

	if (buffer_size <= sizeof(TGAHeader))
	{
		Rml::Log::Message(Rml::Log::LT_ERROR, "Texture file size is smaller than TGAHeader, file is not a valid TGA image.");
		file_interface->Close(file_handle);
		return false;
	}

	using Rml::byte;
	Rml::UniquePtr<byte[]> buffer(new byte[buffer_size]);
	file_interface->Read(buffer.get(), buffer_size, file_handle);
	file_interface->Close(file_handle);

	TGAHeader header;
	memcpy(&header, buffer.get(), sizeof(TGAHeader));

	int color_mode = header.bitsPerPixel / 8;
	const size_t image_size = header.width * header.height * 4; // We always make 32bit textures

	if (header.dataType != 2)
	{
		Rml::Log::Message(Rml::Log::LT_ERROR, "Only 24/32bit uncompressed TGAs are supported.");
		return false;
	}

	// Ensure we have at least 3 colors
	if (color_mode < 3)
	{
		Rml::Log::Message(Rml::Log::LT_ERROR, "Only 24 and 32bit textures are supported.");
		return false;
	}

	const byte* image_src = buffer.get() + sizeof(TGAHeader);
	Rml::UniquePtr<byte[]> image_dest_buffer(new byte[image_size]);
	byte* image_dest = image_dest_buffer.get();

	// Targa is BGR, swap to RGB, flip Y axis, and convert to premultiplied alpha.
	for (long y = 0; y < header.height; y++)
	{
		long read_index = y * header.width * color_mode;
		long write_index = ((header.imageDescriptor & 32) != 0) ? read_index : (header.height - y - 1) * header.width * 4;
		for (long x = 0; x < header.width; x++)
		{
			image_dest[write_index] = image_src[read_index + 2];
			image_dest[write_index + 1] = image_src[read_index + 1];
			image_dest[write_index + 2] = image_src[read_index];
			if (color_mode == 4)
			{
				const byte alpha = image_src[read_index + 3];
				for (size_t j = 0; j < 3; j++)
					image_dest[write_index + j] = byte((image_dest[write_index + j] * alpha) / 255);
				image_dest[write_index + 3] = alpha;
			}
			else
				image_dest[write_index + 3] = 255;

			write_index += 4;
			read_index += color_mode;
		}
	}

	texture_dimensions.x = header.width;
	texture_dimensions.y = header.height;

	return GenerateTexture({image_dest, image_size}, texture_dimensions);
}

Rml::TextureHandle RenderInterface_GD::GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions)
{
    geode::log::debug("RenderInterface_GD::GenerateTexture");
    
	GLuint texture_id = 0;
	glGenTextures(1, &texture_id);
	if (texture_id == 0)
	{
		Rml::Log::Message(Rml::Log::LT_ERROR, "Failed to generate texture.");
		return {};
	}

    geode::log::debug("RenderInterface_GD::GenerateTexture texture id: {}, x: {}, y: {}", texture_id, source_dimensions.x, source_dimensions.y);

	glBindTexture(GL_TEXTURE_2D, texture_id);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, source_dimensions.x, source_dimensions.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, source.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	return (Rml::TextureHandle)texture_id;
}

void RenderInterface_GD::ReleaseTexture(Rml::TextureHandle texture_handle)
{
    geode::log::debug("RenderInterface_GD::ReleaseTexture");

	glDeleteTextures(1, (GLuint*)&texture_handle);
}
