#version 330 core
uniform sampler2D color_tex;
uniform sampler2D depth_tex;
uniform sampler2D clouds_tex;

uniform float tex_width;
uniform float tex_height;

uniform mat4 VP_i;
uniform vec3 cam_pos;

in vec2 uv;
in vec2 vpos;
out vec3 color;

#define M_PI 3.1415

const float CLOUD_DENSITY = 15;
const float CLOUD_FLOOR = 0.6;
const float CLOUD_AMPLITUDE = 6; // determines cloud top height
const float CLOUD_CUTOFF = 0.8;
// Nyquist stuff going on here: larger clouds allow smaller sampling frequency
// and are therefore less expensive to render.
// If it laggs: turn sampling down. If it flickers: turn scale up.
const int CLOUD_SCALE = 30;
const int CLOUD_SAMPLING = 50;

float rgb_2_luma(vec3 c){ return .3*c[0] + .59*c[1] + .11*c[2]; }

float getCloudDensity(vec3 p) {
  //return sin(p.x*2)*max(0, sin(min(p.y - CLOUD_FLOOR, 2)*M_PI/2))*sin(p.z*2);

  vec2 tc = abs(mod(vec2(p.x, p.z), CLOUD_SCALE)) * 2 / CLOUD_SCALE - 1;
  float noiseval = max(0, (CLOUD_CUTOFF - texture(clouds_tex, tc).r));
  float freq = M_PI / (noiseval*CLOUD_AMPLITUDE);

  float cFloor = CLOUD_FLOOR - noiseval*CLOUD_AMPLITUDE/5;

  if (p.y < cFloor) return 0;

  // make a symetric cloud wrt xz-plane
  return sin(min(M_PI, (p.y - cFloor) * freq)) * noiseval;
  //return noiseval*max(0, sin(min(p.y - CLOUD_FLOOR, 2)*M_PI/2)) - noiseval * p.y;
}

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
  float step = horizon_distance / CLOUD_SAMPLING;
  vec3 direction = (farPoint - nearPoint) / horizon_distance;

  float cutoff_value = min(distance(nearPoint, maxPoint), horizon_distance);

  for (float iteration_value = 0 ; iteration_value < cutoff_value ; iteration_value += step) {

    vec3 world_point = nearPoint + direction * iteration_value + vec3(cam_pos.x, 0, cam_pos.z);
    float coeff = min(iteration_value + step, cutoff_value) - iteration_value;


    float density = getCloudDensity(world_point);

    current_color = mix(current_color, vec3(0.85, 0.90, 0.95), min(max(density, 0)*coeff*CLOUD_DENSITY, 1));
  }

  color = current_color;
  //color = vec3(1 - texture(clouds_tex, uv).r, 0, 0);

  /*vec4 world_point = VP_i * vec4(vpos, 0.9, 1);
  world_point /= world_point.w;

  color = vec3(world_point.x, world_point.y, world_point.z);*/
}

