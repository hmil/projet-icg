#version 330 core
uniform sampler2D color_tex;
uniform sampler2D depth_tex;
uniform float tex_width;
uniform float tex_height;

uniform mat4 VP_i;
uniform vec3 cam_pos;

in vec2 uv;
in vec2 vpos;
out vec3 color;

#define M_PI 3.1415

const float CLOUD_DENSITY = 10;
const float CLOUD_FLOOR = 0.8;

const int nb_values = 100;
float rgb_2_luma(vec3 c){ return .3*c[0] + .59*c[1] + .11*c[2]; }

/*
float linearizeDepth(float depth) {
  return pow(depth, 20);
}
*/

float f(float depth) {
  return sqrt(depth);
}

vec3 unproject(float depth) {
  vec4 vect = VP_i * vec4(vpos, depth, 1.0);
  return vec3(vect) / vect.w;
}

void main() {
  vec3 current_color = texture(color_tex, uv).rgb;
  float current_depth = texture(depth_tex, uv).r;

  vec2 vertex_pos = vpos;
  vertex_pos.y = -vertex_pos.y; // Inverse y coordinate, figure out why after

  vec3 nearPoint = unproject(0);
  vec3 farPoint = unproject(1);
  vec3 maxPoint = unproject(current_depth);
  float horizon_distance = distance(farPoint, nearPoint);
  float step = horizon_distance / nb_values;
  vec3 direction = (farPoint - nearPoint) / horizon_distance;

  float cutoff_value = min(distance(nearPoint, maxPoint), horizon_distance);


  for (float iteration_value = 0 ; iteration_value < cutoff_value ; iteration_value += step) {

    vec3 world_point = nearPoint + direction * iteration_value + cam_pos;
    float coeff = min(iteration_value + step, cutoff_value) - iteration_value;


    float l = sin(world_point.x)*max(0, sin(min(world_point.y - CLOUD_FLOOR, 2)*M_PI/2))*sin(world_point.z); //distance(world_point, vec3(2103, 1, 2125));
    //if (l < 1) {
      current_color = mix(current_color, vec3(0.85, 0.90, 0.95), min(max(l, 0)*coeff*CLOUD_DENSITY, 1));
      //current_color = mix(current_color, vec3(1,0,0), 0.1);
    //}
  }

  color = current_color;

  /*vec4 world_point = VP_i * vec4(vpos, 0.9, 1);
  world_point /= world_point.w;

  color = vec3(world_point.x, world_point.y, world_point.z);*/
}

