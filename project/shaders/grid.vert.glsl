#version 400 core

in vec2 position;
out vec2 vPosition;
out vec2 vUV;
out int vLOD;

uniform mat4 model;
uniform sampler2D tex;

void main()
{
  // Texturing
  vUV = (position + vec2(2.0, 2.0)) * 0.25;
  vPosition = position;

  // LOD
  vec4 pp = model * vec4(position.x, 0, -position.y, 1.0 );
  pp /= pp.w;
  float dist = length(vec2(pp.x, pp.z));

  vLOD = 1;
  if (dist < 5) vLOD = 4;
  if (dist < 2) vLOD = 6;
  if (dist < 1.5) vLOD = 8;

  //vLOD = int((1 - clamp(length(vec2(pp.x, pp.z)) / 2.5 - 0.4, 0, 1)) * 10 + 1);
}
