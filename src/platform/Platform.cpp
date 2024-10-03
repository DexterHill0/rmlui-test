#include "Geode/loader/Log.hpp"
#include <RmlUi/Core/Input.h>
#include <RmlUi/Core/SystemInterface.h>
#include <RmlUi/Core/Types.h>
#include <string>
#include <Geode/Geode.hpp>

#include "Platform.h"

SystemInterface_GD::SystemInterface_GD() {}

bool SystemInterface_GD::LogMessage(Rml::Log::Type type, const Rml::String& message) {
    geode::log::debug("[SystemInterface] {}", message);
        return true;
}
