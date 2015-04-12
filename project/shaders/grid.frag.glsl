#version 330 core

uniform vec3 Ia, Id;
uniform sampler2D tex;
uniform sampler1D color_tex;

in vec2 uv;
in vec3 normal;
in vec3 light_dir;
in vec3 view_dir;

out vec3 color;

void main() {
  // color scale at height depending on heightmap
  vec3 baseColor = texture(color_tex, texture(tex, uv).r).rgb;
  color = Ia*baseColor + clamp(Id*baseColor*dot(normal, light_dir), vec3(0,0,0), vec3(1,1,1));
}