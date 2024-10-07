const char* POS_COL_TEX_FRAG = R"(
#version 330 core
out vec4 gl_FragColor;

in vec4 vertexColor;
in vec2 texCoord;

uniform sampler2D textureSampler;

void main()
{
    gl_FragColor = vertexColor * texture(textureSampler, texCoord);
} 
)";