#include "../../utils/stringify.h"

constexpr auto POS_TEX_COL_FRAG = STRINGIFY(
#ifdef GL_ES
precision lowp float;
#endif

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;
uniform sampler2D CC_Texture0;

void main() {
  gl_FragColor = v_fragmentColor * texture2D(CC_Texture0, v_texCoord);
}
);