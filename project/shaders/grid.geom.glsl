#version 400 core

// Geometry

uniform mat4 model_i;
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;
in vec3 tePosition[3];
in vec2 teUV[3];
in vec3 teNormal[3];
in vec3 tePatchDistance[3];

out vec3 gFacetNormal;
out vec3 gPatchDistance;
out vec3 gTriDistance;
out vec2 gUV;
out vec3 gNormal;



in vec2 uv[3];
out vec2 g_uv[3];

void main()
{
    g_uv = uv;
    vec3 A = tePosition[2] - tePosition[0];
    vec3 B = tePosition[1] - tePosition[0];
    gFacetNormal = mat3(model_i) * normalize(cross(A, B));

    gPatchDistance = tePatchDistance[0];
    gTriDistance = vec3(1, 0, 0);
    gUV = teUV[0];
    gNormal = teNormal[0];
    gl_ClipDistance[0] = gl_in[0].gl_ClipDistance[0];
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();

    gPatchDistance = tePatchDistance[1];
    gTriDistance = vec3(0, 1, 0);
    gUV = teUV[1];
    gNormal = teNormal[1];
    gl_ClipDistance[0] = gl_in[1].gl_ClipDistance[0];
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();

    gPatchDistance = tePatchDistance[2];
    gTriDistance = vec3(0, 0, 1);
    gUV = teUV[2];
    gNormal = teNormal[2];
    gl_ClipDistance[0] = gl_in[2].gl_ClipDistance[0];
    gl_Position = gl_in[2].gl_Position;
    EmitVertex();

    EndPrimitive();
}
