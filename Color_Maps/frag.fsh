#version 150 core

uniform sampler2D TextureUniform;
uniform float ctrl_pts[10];
uniform vec3 ctrl_colors[10];
uniform int max_pt;

in vec2 tex_coord;
out vec4 fcol;

void main(void) {
   float sample = texture(TextureUniform,tex_coord).x;
   if ( max_pt < 1 ){ //Not enough control points to make a map
       fcol = vec4(sample,sample,sample,1.0);
   }
   else{
      float last = ctrl_pts[0];
      int i = 1;
      for(i = 1; i < max_pt; i++){
          if(sample >= last && sample <= ctrl_pts[i]){
	      break;
	   }
          else {
              last = ctrl_pts[i];
	      fcol = vec4(ctrl_colors[0],1);
          }

      }
	fcol = vec4(mix(ctrl_colors[i-1],ctrl_colors[i],
		        (sample - last) / ctrl_pts[i] ),1.0);
   }

}
