// Copyright (C) 2014 - LGG EPFL
#version 330 core
uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
uniform vec3 light_pos;

in vec3 vpoint;
in vec3 vnormal;

out vec3 normal;
out vec3 light_dir;
out vec3 view_dir;



/// 1.1: Phong shading.
void main() {
    mat4 MV = view * model;
    vec4 vpoint_mv = MV * vec4(vpoint, 1.0);
    gl_Position = projection * vpoint_mv;
    vec3 world_point = model * vec4(vpoint, 1.0);

    /// 1) compute normal_mv using the model_view matrix.
    normal = normalize(inverse(transpose(model)) * vec4(vnormal, 1.0));

    /// 2) compute the light direction light_dir.
    light_dir = normalize(vec4(light_pos, 1.0) - vec4(world_point, 1.0));

    /// 3) compute the view direction view_dir.
    vec3 camera_pos = inverse(view) * vec4(0, 0, 0, 1);
    view_dir = normalize(camera_pos - world_point);

}
