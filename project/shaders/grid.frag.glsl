#version 330 core

uniform vec3 Ia, Id;
uniform sampler2D tex;
uniform sampler1D color_tex;

uniform vec3 fogColor;
const float b = 2;

in vec2 uv;
in vec3 normal;
in vec3 light_dir;
in vec3 view_dir;
in vec4 gl_FragCoord;

out vec3 color;

void main() {
  // color scale at height depending on heightmap
  vec3 baseColor = texture(color_tex, texture(tex, uv).r).rgb;
  vec3 finalColor = Ia*baseColor + clamp(Id*baseColor*dot(normal, light_dir), vec3(0,0,0), vec3(1,1,1));


  // Adapted from http://www.iquilezles.org/www/articles/fog/fog.htm
  float distance = gl_FragCoord.z;
  float fogAmount = pow(distance, 10);
  /* Must render to framebuffer to do that properly:
  float sunAmount = max( dot( -view_dir, -light_dir ), 0.0 );

  vec3  fogFinal  = mix( vec3(0.5,0.6,0.7), // bluish
                         vec3(1.0,0.95,0.85), // yellowish
                         pow(sunAmount,8.0) );
  */

  color = mix(finalColor, fogColor, fogAmount);
}