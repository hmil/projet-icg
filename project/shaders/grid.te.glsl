#version 400 core
// TessEval

layout(triangles, equal_spacing, cw) in;
in vec2 tcPosition[];
in vec2 tcUV[];
out vec3 tePosition;
out vec2 teUV;
out vec3 tePatchDistance;
out float gl_ClipDistance[1];
uniform mat4 projection;
uniform mat4 MV;
uniform sampler2D tex;

void main()
{
    vec2 u1 = gl_TessCoord.x * tcUV[0];
    vec2 u2 = gl_TessCoord.y * tcUV[1];
    vec2 u3 = gl_TessCoord.z * tcUV[2];
    teUV = u1 + u2 + u3;
    vec2 p0 = gl_TessCoord.x * tcPosition[0];
    vec2 p1 = gl_TessCoord.y * tcPosition[1];
    vec2 p2 = gl_TessCoord.z * tcPosition[2];
    tePatchDistance = gl_TessCoord;
    vec2 point = p0 + p1 + p2;
    tePosition = vec3(point.x, texture(tex, teUV).r, -point.y);
    gl_Position = projection * MV * vec4(tePosition, 1);

    gl_ClipDistance[0] = -1;
}
