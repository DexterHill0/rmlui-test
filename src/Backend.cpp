

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Input.h>
#include <RmlUi/Core/Profiling.h>
#include <Geode/Geode.hpp>
#include "Geode/cocos/shaders/CCGLProgram.h"
#include "Geode/cocos/shaders/CCShaderCache.h"
#include "Geode/loader/Log.hpp"

#include "./Backend.h"

void Backend::Initialize(float viewWidth, float viewHeight)
{
	RMLUI_ASSERT(!data);

	data = Rml::MakeUnique<BackendData>();
}

void Backend::Shutdown()
{
	RMLUI_ASSERT(data);

    // auto shader = cocos2d::CCShaderCache::sharedShaderCache()->programForKey("rmlui_shader");
    // shader->release();

	data.reset();
}

// Rml::SystemInterface* Backend::GetSystemInterface()
// {
// 	RMLUI_ASSERT(data);
// 	return &data->system_interface;
// }

// FileInterface_GD* Backend::GetFileInterface() {
//     RMLUI_ASSERT(data);
// 	return &data->file_interface;
// }

RenderInterface_GD* Backend::GetRenderInterface()
{
	RMLUI_ASSERT(data);
	return &data->render_interface;
}


void Backend::BeginFrame()
{
	RMLUI_ASSERT(data);

    data->render_interface.Clear();
    data->render_interface.BeginFrame();
}

void Backend::PresentFrame()
{
	RMLUI_ASSERT(data);

	data->render_interface.EndFrame();

    // glViewport(0, 0, data->window_dimensions.x, data->window_dimensions.y);

	RMLUI_FrameMark;
}