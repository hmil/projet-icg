#version 400 core
// TessEval

layout(quads, equal_spacing, cw) in;
in vec2 tcPosition[];
in vec2 tcUV[];
out vec3 tePosition;
out vec2 teUV;
out vec3 teNormal;

out vec3 tePatchDistance;

uniform mat4 projection;
uniform mat4 MV;
uniform mat4 model_i; // prebaked inverse
uniform sampler2D tex;
uniform int resolution;

const float WATER_HEIGHT = 0.3f;
// Used to scale the step according to heightmap normals amplification
#define NORMAL_AMP  25.0

void main()
{
  vec2 u1 = mix(tcUV[0], tcUV[1], gl_TessCoord.x);
  vec2 u2 = mix(tcUV[2], tcUV[3], gl_TessCoord.x);
  teUV = mix(u1, u2, gl_TessCoord.y);
  vec2 p0 = mix(tcPosition[0], tcPosition[1], gl_TessCoord.x);
  vec2 p1 = mix(tcPosition[2], tcPosition[3], gl_TessCoord.x);
  vec2 point = mix(p0, p1, gl_TessCoord.y);

  // Unpacking heightmap data
  vec3 sampled = texture(tex, teUV).rgb;
  float height = sampled.r;
  vec2 diffs = sampled.gb;
  // Reconstruct normal
  float dstep = NORMAL_AMP/resolution;
  vec3 dx = normalize(vec3(dstep, (diffs.x - 0.5)/25, 0));
  vec3 dz = normalize(vec3(0, (diffs.y - 0.5)/25, dstep));
  vec3 vnormal = normalize(cross(dz, dx));


  // Phong stuff
  teNormal = normalize(vec3(mat3(model_i) * vec3(vnormal)));

  tePosition = vec3(point.x, height, -point.y);
  gl_Position = projection * MV * vec4(tePosition, 1);

  gl_ClipDistance[0] = tePosition.y - WATER_HEIGHT;
  tePatchDistance = gl_TessCoord;
}
