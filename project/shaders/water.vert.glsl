#version 330 core
uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;
in vec3 vpoint;
in vec2 vtexcoord;
out vec2 uv;
out vec4 view_dir;

void main() {
    vec4 pos = MVP * vec4(vpoint, 1.0);
    uv = vtexcoord;

    gl_Position = pos;
    view_dir = normalize(inverse(V) * vec4(0, 0, 0, 1) - M * vec4(vpoint, 1.0));
}


 /*   mat4 MV = view * model;
    vec4 vpoint_mv = MV * vec4(vpoint, 1.0);
    gl_Position = projection * vpoint_mv;
    vec3 world_point = model * vec4(vpoint, 1.0);

    /// 1) compute normal_mv using the model_view matrix.
    normal = normalize(inverse(transpose(model)) * vec4(vnormal, 1.0));

    /// 2) compute the light direction light_dir.
    light_dir = normalize(vec4(light_pos, 1.0) - vec4(world_point, 1.0));

    /// 3) compute the view direction view_dir.
    vec3 camera_pos = inverse(view) * vec4(0, 0, 0, 1);
    view_dir = normalize(camera_pos - world_point);*/