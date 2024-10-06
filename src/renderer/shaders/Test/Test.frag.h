const char* TEST_FRAG = R"(
#version 330 core
out vec4 gl_FragColor;

in vec3 vertexColor;
in vec2 texCoord;

uniform sampler2D textureSampler;

void main()
{
    // gl_FragColor = vec4(vertexColor, 1.0f);
    gl_FragColor = texture(textureSampler, texCoord);
} 
)";