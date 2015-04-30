#version 330 core

uniform sampler2D tex;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 MV;
uniform mat4 MVP;
uniform mat4 view_i; // prebaked inverse
uniform mat4 model_i; // prebaked inverse

in vec2 position;
out float gl_ClipDistance[1];

out vec2 uv;
out vec3 normal;
out vec3 view_dir;

const float WATER_HEIGHT = 0.3f;

// Used to scale the step according to heightmap normals amplification
#define NORMAL_AMP  25.0

#define SEAM_LENGTH 0.005

uniform int resolution;

void main() {


  uv = (position + vec2(2.0, 2.0)) * 0.25;

  float height = texture(tex, uv).r;

  vec2 diffs = texture(tex, uv).gb;
  float dstep = NORMAL_AMP/resolution;

  vec3 dx = normalize(vec3(dstep, (diffs.x - 0.5)/25, 0));
  vec3 dz = normalize(vec3(0, (diffs.y - 0.5)/25, dstep));
  vec3 vnormal = normalize(cross(dz, dx));

  // phong

  /// 1) compute normal_mv using the model_view matrix.
  normal = normalize(vec3(mat3(model_i) * vec3(vnormal)));

  vec3 vpoint = vec3(position.x, height, -position.y);

  // Dirty hack to avoid appearence of gaps between tiles
  if (position.x >= 1.999999) {
    vpoint += vec3(SEAM_LENGTH, -SEAM_LENGTH, 0);
  } else if (position.x <= -1.999999) {
    vpoint += vec3(-SEAM_LENGTH, -SEAM_LENGTH, 0);
  }
  if (position.y >= 1.999999) {
    vpoint += vec3(0, -SEAM_LENGTH, -SEAM_LENGTH);
  } else if (position.y <= -1.999999) {
    vpoint += vec3(0, -SEAM_LENGTH, SEAM_LENGTH);
  }

  gl_Position = MVP * vec4(vpoint, 1.0);

  vec3 world_point = vec3(model * vec4(vpoint, 1.0));

  /// 3) compute the view direction view_dir.
  vec3 camera_pos = vec3(view_i * vec4(0, 0, 0, 1));
  view_dir = normalize(camera_pos - world_point);


  gl_ClipDistance[0] = vpoint.y - WATER_HEIGHT;
}
