#version 130

uniform vec3 light_dir;

in vec4 color_frag;
in vec3 norm_frag;
out vec4 fcol;

void main(void) {
/* bp-shading model */

  float a = max(0,dot( normalize(light_dir),normalize(norm_frag)) );
  fcol = color_frag * min(1.5,(1 + a));
  fcol.a = color_frag.a;

}
