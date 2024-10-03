

#include "./Backend.h"
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Input.h>
#include <RmlUi/Core/Profiling.h>
#include <Geode/Geode.hpp>

#include "Backend.h"
#include "platform/Platform.h"


void Backend::Initialize()
{
	RMLUI_ASSERT(!data);

	data = Rml::MakeUnique<BackendData>();
}

void Backend::Shutdown()
{
	RMLUI_ASSERT(data);

	data.reset();
}

SystemInterface_GD* Backend::GetSystemInterface()
{
	RMLUI_ASSERT(data);
	return &data->system_interface;
}

RenderInterface_GD* Backend::GetRenderInterface()
{
	RMLUI_ASSERT(data);
	return &data->render_interface;
}


void Backend::BeginFrame()
{
	RMLUI_ASSERT(data);

    data->render_interface.BeginFrame();
}

void Backend::PresentFrame()
{
	RMLUI_ASSERT(data);

	data->render_interface.EndFrame();

	RMLUI_FrameMark;
}