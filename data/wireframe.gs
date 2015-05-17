#version 150
#extension GL_ARB_separate_shader_objects : enable

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform vec2 winSize;

in vec3 normalV[];
in vec2 uvV[];

out vec3 normalG;
out vec2 uvG;
out vec3 distG;

void main(void)
{
    vec2 p0 = winSize * gl_in[0].gl_Position.xy / gl_in[0].gl_Position.w;
    vec2 p1 = winSize * gl_in[1].gl_Position.xy / gl_in[1].gl_Position.w;
    vec2 p2 = winSize * gl_in[2].gl_Position.xy / gl_in[2].gl_Position.w;

    vec2 v0 = p2 - p1;
    vec2 v1 = p2 - p0;
    vec2 v2 = p1 - p0;

    float area = abs(v1.x * v2.y - v1.y * v2.x);

    normalG     = normalV[0];
    uvG         = uvV[0];
    distG       = vec3(area / length(v0), 0, 0);
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();

    normalG     = normalV[1];
    uvG         = uvV[1];
    distG       = vec3(0, area / length(v1), 0);
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();

    normalG     = normalV[2];
    uvG         = uvV[2];
    distG       = vec3(0, 0, area / length(v2));
    gl_Position = gl_in[2].gl_Position;
    EmitVertex();

    EndPrimitive();
}
