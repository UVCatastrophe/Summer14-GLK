#version 130

uniform mat3 orientMat;

in vec3 position;
out vec2 tex_coord;

void main(void) { 
    vec3 dPos = vec3(position.xy,1.0);
    dPos = vec3((orientMat*dPos).xy,0.0);
    gl_Position = vec4(dPos,1.0);
    tex_coord = (position.xy + 1.0) /2.0;
}
