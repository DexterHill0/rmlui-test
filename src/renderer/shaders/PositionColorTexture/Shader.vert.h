const char* POS_COL_TEX_VERT = R"(
#version 330 core
uniform mat4 matTransform;
uniform vec2 translation;

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aTexCoord;

out vec4 vertexColor;
out vec2 texCoord;

void main()
{
    vec2 translatedPos = vec2(aPos.x, -aPos.y) + vec2(translation.x, -translation.y);
    gl_Position = matTransform * vec4(translatedPos, aPos.z, 1.0);

    vertexColor = aColor;
    texCoord = vec2(aTexCoord.x, aTexCoord.y);
}
)";