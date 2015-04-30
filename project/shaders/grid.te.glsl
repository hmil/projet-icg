#version 400 core
// TessEval

layout(triangles, equal_spacing, cw) in;
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
  vec2 u1 = gl_TessCoord.x * tcUV[0];
  vec2 u2 = gl_TessCoord.y * tcUV[1];
  vec2 u3 = gl_TessCoord.z * tcUV[2];
  teUV = u1 + u2 + u3;
  vec2 p0 = gl_TessCoord.x * tcPosition[0];
  vec2 p1 = gl_TessCoord.y * tcPosition[1];
  vec2 p2 = gl_TessCoord.z * tcPosition[2];
  vec2 point = p0 + p1 + p2;

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
