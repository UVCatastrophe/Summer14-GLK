#version 130

in vec4 color_frag;
in vec3 norm_frag;
out vec4 fcol;

void main(void) {
/* bp-shading model */

  vec3 light_dir = vec3(-1.0,-1.0,0.0);
  float a = max(0,dot( normalize(light_dir),normalize(norm_frag)) );
  fcol = color_frag * min(1,(.5 + a));
  fcol.w = 1.0;
}
