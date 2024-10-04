

#include "./Backend.h"
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Input.h>
#include <RmlUi/Core/Profiling.h>
#include <Geode/Geode.hpp>

#include "Backend.h"


void Backend::Initialize(float viewWidth, float viewHeight)
{
	RMLUI_ASSERT(!data);

	data = Rml::MakeUnique<BackendData>();

    data->window_dimensions.x = viewWidth;
    data->window_dimensions.y = viewHeight;

    geode::log::debug("VW {}, VH {}", viewWidth, viewHeight);

    GLuint rmlui_fbo, rmlui_color_texture, rmlui_depth_buffer;

    // Generate the framebuffer object
    glGenFramebuffers(1, &rmlui_fbo);

    // Bind the framebuffer so you can attach textures or renderbuffers to it
    glBindFramebuffer(GL_FRAMEBUFFER, rmlui_fbo);

    // Generate the texture that will store the color output
    glGenTextures(1, &rmlui_color_texture);
    glBindTexture(GL_TEXTURE_2D, rmlui_color_texture);

    // Allocate texture memory (set texture size according to your UI's size, e.g., window size)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewWidth, viewHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Set texture parameters (important for proper rendering)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Attach the texture to the framebuffer's color attachment
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rmlui_color_texture, 0);

    // Generate a renderbuffer object for depth and stencil
    glGenRenderbuffers(1, &rmlui_depth_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, rmlui_depth_buffer);

    // Allocate storage for the depth buffer (match the size to your framebuffer)
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, viewWidth, viewHeight);

    // Attach the renderbuffer to the framebuffer for depth and stencil
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rmlui_depth_buffer);

    // Check if the framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer is incomplete!" << std::endl;
    }

    // Unbind the framebuffer for now
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GLint gdFbo;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &gdFbo);

    data->rmluiFbo = rmlui_fbo;
    data->gdFbo = gdFbo;
}

void Backend::Shutdown()
{
	RMLUI_ASSERT(data);

	data.reset();
}

// Rml::SystemInterface* Backend::GetSystemInterface()
// {
// 	RMLUI_ASSERT(data);
// 	return &data->system_interface;
// }

RenderInterface_GD* Backend::GetRenderInterface()
{
	RMLUI_ASSERT(data);
	return &data->render_interface;
}


void Backend::BeginFrame()
{
	RMLUI_ASSERT(data);

    glBindFramebuffer(GL_FRAMEBUFFER, data->rmluiFbo);
    data->render_interface.Clear();
    data->render_interface.BeginFrame();
}

void Backend::PresentFrame()
{
	RMLUI_ASSERT(data);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, data->rmluiFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, data->gdFbo); 

	data->render_interface.EndFrame();
    glBindFramebuffer(GL_FRAMEBUFFER, data->gdFbo);

    glViewport(0, 0, data->window_dimensions.x, data->window_dimensions.y);

    data->render_interface.FinaliseFrame();

	RMLUI_FrameMark;
}