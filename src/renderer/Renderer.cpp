#include "Renderer.h"
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
	glViewport(0, 0, viewport_width, viewport_height);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    // not sure what this does lol!!!!!!!!
    // it changes nothing with or without it 
    
	// Rml::Matrix4f projection = Rml::Matrix4f::ProjectOrtho(0, (float)viewport_width, (float)viewport_height, 0, -10000, 10000);
    // glMatrixMode(GL_PROJECTION);
    // glLoadMatrixf(projection.data());
    // glMatrixMode(GL_TEXTURE);
    // glLoadIdentity();
    // glMatrixMode(GL_MODELVIEW);
    // glLoadIdentity();

	transform_enabled = false;
}

void RenderInterface_GD::EndFrame() {}

void RenderInterface_GD::Clear()
{
	glClearStencil(0);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void RenderInterface_GD::RenderGeometry(Rml::Vertex* vertices, int /*num_vertices*/, int* indices, int num_indices, const Rml::TextureHandle texture,
	const Rml::Vector2f& translation)
{
	glPushMatrix();
	glTranslatef(translation.x, translation.y, 0);

	glVertexPointer(2, GL_FLOAT, sizeof(Rml::Vertex), &vertices[0].position);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Rml::Vertex), &vertices[0].colour);

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
		glTexCoordPointer(2, GL_FLOAT, sizeof(Rml::Vertex), &vertices[0].tex_coord);
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

cocos2d::CCPoint frameToCocos(int x, int y) {
	auto* director = cocos2d::CCDirector::sharedDirector();
	const auto frameSize = director->getOpenGLView()->getFrameSize() * geode::utils::getDisplayFactor();
	const auto winSize = director->getWinSize();

	return {
		x / frameSize.width * winSize.width,
		(1.f - y / frameSize.height) * winSize.height
	};
}

void RenderInterface_GD::SetScissorRegion(int x, int y, int width, int height)
{
    // i took this one from mats imgui cocos
    // dont know if this is doing the correct things
    const auto orig = frameToCocos(x, y);
    const auto end = frameToCocos(width, height);
    cocos2d::CCDirector::sharedDirector()->getOpenGLView()->setScissorInPoints(orig.x, end.y, end.x - orig.x, orig.y - end.y);

    // this is the original code

	// if (!transform_enabled)
	// {
	// 	glScissor(x, viewport_height - (y + height), width, height);
	// }
	// else
	// {
	// 	// clear the stencil buffer
	// 	glStencilMask(GLuint(-1));
	// 	glClear(GL_STENCIL_BUFFER_BIT);

	// 	// fill the stencil buffer
	// 	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	// 	glDepthMask(GL_FALSE);
	// 	glStencilFunc(GL_NEVER, 1, GLuint(-1));
	// 	glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);

	// 	float fx = (float)x;
	// 	float fy = (float)y;
	// 	float fwidth = (float)width;
	// 	float fheight = (float)height;

	// 	// draw transformed quad
	// 	GLfloat vertices[] = {fx, fy, 0, fx, fy + fheight, 0, fx + fwidth, fy + fheight, 0, fx + fwidth, fy, 0};
	// 	glDisableClientState(GL_COLOR_ARRAY);
	// 	glVertexPointer(3, GL_FLOAT, 0, vertices);
	// 	GLushort indices[] = {1, 2, 0, 3};
	// 	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, indices);
	// 	glEnableClientState(GL_COLOR_ARRAY);

	// 	// prepare for drawing the real thing
	// 	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	// 	glDepthMask(GL_TRUE);
	// 	glStencilMask(0);
	// 	glStencilFunc(GL_EQUAL, 1, GLuint(-1));
	// }
}

void RenderInterface_GD::SetTransform(const Rml::Matrix4f* transform)
{
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



bool RenderInterface_GD::LoadTexture(Rml::TextureHandle& texture_handle, Rml::Vector2i& texture_dimensions, const Rml::String& source)
{
	return true;
}

bool RenderInterface_GD::GenerateTexture(Rml::TextureHandle& texture_handle, const Rml::byte* source, const Rml::Vector2i& source_dimensions)
{
	return true;
}

void RenderInterface_GD::ReleaseTexture(Rml::TextureHandle texture_handle)
{
}
