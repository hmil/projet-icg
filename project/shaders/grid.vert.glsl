#version 330 core

uniform sampler2D tex;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 light_pos;

in vec2 position;

out vec2 uv;
out vec3 normal;
out vec3 light_dir;
out vec3 view_dir;

uniform int resolution;

void main() {
  mat4 MV = view * model;
  mat4 MVP = projection * MV;


  uv = (position + vec2(1.0, 1.0)) * 0.5;

  float height = texture(tex, uv).r;
  vec3 vpoint = vec3(position.x, height, -position.y);

  vec2 diffs = texture(tex, uv).gb;
  float dstep = 1.0/resolution;
  vec3 dx = normalize(vec3(dstep, diffs.x * 2 - 1, 0));
  vec3 dz = normalize(vec3(0, diffs.y * 2 - 1, dstep));
  vec3 vnormal = normalize(cross(dz, dx));

  gl_Position = MVP * vec4(vpoint, 1.0);

  // phong

  vec4 vpoint_mv = MV * vec4(vpoint, 1.0);
  gl_Position = projection * vpoint_mv;
  vec3 world_point = vec3(model * vec4(vpoint, 1.0));

  /// 1) compute normal_mv using the model_view matrix.
  normal = normalize(vec3(inverse(transpose(model)) * vec4(vnormal, 1.0)));

  /// 2) compute the light direction light_dir.
  light_dir = normalize(vec3(vec4(light_pos, 1.0) - vec4(world_point, 1.0)));

  /// 3) compute the view direction view_dir.
  vec3 camera_pos = vec3(inverse(view) * vec4(0, 0, 0, 1));
  view_dir = normalize(camera_pos - world_point);

}
