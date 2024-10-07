#ifndef POS_COL_TEX_SHADER_H
#define POS_COL_TEX_SHADER_H

#include <optional>

#include "../../Shader.h"
#include "./Shader.frag.h"
#include "./Shader.vert.h"

namespace PositionColorTexture {
namespace Attribute {
    constexpr int Position = 0;
    constexpr int Color = 1;
    constexpr int TexCoord = 2;
} // namespace Attribute

namespace Uniform {
    constexpr const char* MatTransform = "matTransform";
    constexpr const char* Translation = "translation";
} // namespace Uniform

inline std::optional<Shader*> compile() {
    auto shader = Shader::compile(POS_COL_TEX_VERT, POS_COL_TEX_FRAG);
    if (!shader.has_value())
        return std::nullopt;
    shader.value()->cacheUniformLocations(
        std::array {Uniform::MatTransform, Uniform::Translation}
    );
    return shader;
}
} // namespace PositionColorTexture

#endif