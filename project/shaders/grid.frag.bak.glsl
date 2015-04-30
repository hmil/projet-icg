#version 330 core

#define M_PI 3.14159265359

uniform vec3 Ia, Id;
uniform sampler2D tex;
uniform sampler2D grass_tex;
uniform sampler2D rock_tex;
uniform sampler2D sand_tex;
uniform sampler2D snow_tex;
uniform vec3 light_dir;

uniform vec3 fogColor;

in vec2 uv;
in vec3 normal;
in vec3 view_dir;
in vec4 gl_FragCoord;

out vec3 color;

const float SNOW_INTERP_FACTOR = 100;
const float GRASS_INTERP_FACTOR = 100;
const float SAND_INTERP_FACTOR = 200;
const float ROCK_INTERP_FACTOR = 40;

const float GRASS_MIN_HEIGHT = 0.3; // Also water level
const float SNOW_MIN_HEIGHT = 0.65;
const float SNOW_VARIANCE = 0.09;

const float SLOPE_THRESHOLD = 0.25;

void main() {
  // color scale at height depending on heightmap
  vec3 sampled = texture(tex, uv).rgb;
  float height = sampled.r;
  float slope = abs(sampled.b - 0.5) + abs(sampled.g - 0.5);

  float grassAmount = 0, rockAmount = 0, sandAmount = 0, snowAmount = 0;

  // add some distortion to snow level for realism
  float snow_spatial_dist = sin((uv.x+uv.y)*20*M_PI + slope);
  float snow_slope_dist = (slope * slope * 10);
  float snowActualHeight = SNOW_MIN_HEIGHT + SNOW_VARIANCE * snow_spatial_dist * snow_slope_dist;


  float slopeActualThreshold = (1 - 0.2*height) * SLOPE_THRESHOLD;

  rockAmount = clamp(ROCK_INTERP_FACTOR*(slope - slopeActualThreshold), 0, 1);
  snowAmount = clamp(SNOW_INTERP_FACTOR*(height-snowActualHeight), 0, 1);
  grassAmount = clamp(GRASS_INTERP_FACTOR*(height-GRASS_MIN_HEIGHT), 0, 1);
  sandAmount = 1 - grassAmount;
  rockAmount = clamp(rockAmount - sandAmount, 0, 1); // There's no rock underwater
  // In case rock is at snow level, we "freeze" it with some snow:
  rockAmount = clamp(rockAmount - clamp(height - snowActualHeight, 0, 1), 0, 1);
  // no snow on rock
  snowAmount = clamp(snowAmount - rockAmount, 0, 1);
  grassAmount = clamp(grassAmount - snowAmount - rockAmount, 0, 1); // There's no grass on rock


  vec3 baseColor =
            grassAmount* texture(grass_tex, uv*100).rgb
          + rockAmount * texture(rock_tex, uv*100).rgb
          + sandAmount * texture(sand_tex, uv*60).rgb
          + snowAmount* texture(snow_tex, uv*30).rgb;


  vec3 finalColor = Ia*baseColor + clamp(Id*baseColor*dot(normalize(normal), normalize(light_dir)), vec3(0,0,0), vec3(1,1,1));

  // Adapted from http://www.iquilezles.org/www/articles/fog/fog.htm
  //float distance = gl_FragCoord.z;
  //float fogAmount = pow(distance, 120);
  /* Must render to framebuffer to do that properly:
  float sunAmount = max( dot( -view_dir, -light_dir ), 0.0 );

  vec3  fogFinal  = mix( vec3(0.5,0.6,0.7), // bluish
                         vec3(1.0,0.95,0.85), // yellowish
                         pow(sunAmount,8.0) );
  */

  // Underwater effect (in testing)
  float water_coeff = 0;
  if(height < GRASS_MIN_HEIGHT) water_coeff = 1 - height*2;
  finalColor = mix(finalColor, vec3(0, 0, 0.2), water_coeff);

  color = finalColor;
  //color = mix(finalColor, fogColor, fogAmount);
  //color = vec3(0, texture(tex, uv).gb);
}


