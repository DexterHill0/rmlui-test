#ifndef TEST_SHADER_H
#define TEST_SHADER_H

#include "../../Shader.h"

#include "./Test.frag.h"
#include "./Test.vert.h"
#include <optional>

namespace Test {
    // static const char* Name = "TEST";

    namespace Uniform {
        // static const char* Color = "uColor";
    }

    inline std::optional<Shader*> compile() {
        auto shader = Shader::compile(TEST_VERT, TEST_FRAG);
        if (!shader.has_value()) return std::nullopt;
        // shader.value()->cacheUniformLocations(std::array{ Test::Uniform::Color });
        return shader;
    }
}

#endif