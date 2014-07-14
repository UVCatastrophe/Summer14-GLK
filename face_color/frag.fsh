#version 130

uniform sampler2D TextureUniform;
uniform vec3 color1;
uniform vec3 color2;

in vec2 tex_coord;
out vec4 fcol;


void main(void) {
    vec4 txt = texture(TextureUniform,tex_coord);
    if(txt.x > .9 && txt.y > .9 && txt.z > .9){
       fcol = vec4(color1,1.0);
}
    else
       fcol = vec4(color2,1.0);
}
