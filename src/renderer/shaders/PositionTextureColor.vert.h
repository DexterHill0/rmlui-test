#include "../../utils/stringify.h"

constexpr auto POS_TEX_COL_VERT = STRINGIFY(
attribute vec4 a_position;
attribute vec2 a_texCoord;
attribute vec4 a_color;

#ifdef GL_ES
varying lowp vec4 v_fragmentColor;
varying mediump vec2 v_texCoord;
#else
varying vec4 v_fragmentColor;
varying vec2 v_texCoord;
#endif

void main() {
    gl_Position = a_position;
    v_fragmentColor = a_color;
    v_texCoord = vec2(a_texCoord.x, a_texCoord.y);
}
);