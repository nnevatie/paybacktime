#version 150

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

// Uniforms
uniform vec2 winSize;

// Input
in Block
{
    vec3 normal;
    vec2 uv;
    vec3 bc;
}
input[];

// Output
out Block
{
    vec3 normal;
    vec2 uv;
    vec3 bc;
}
output;

void main(void)
{
    vec2 p0 = winSize * gl_in[0].gl_Position.xy / gl_in[0].gl_Position.w;
    vec2 p1 = winSize * gl_in[1].gl_Position.xy / gl_in[1].gl_Position.w;
    vec2 p2 = winSize * gl_in[2].gl_Position.xy / gl_in[2].gl_Position.w;

    vec2 v0 = p2 - p1;
    vec2 v1 = p2 - p0;
    vec2 v2 = p1 - p0;

    float area = abs(v1.x * v2.y - v1.y * v2.x);

    output.normal = input[0].normal;
    output.uv     = input[0].uv;
    output.bc     = vec3(area / length(v0), 0, 0);
    gl_Position   = gl_in[0].gl_Position;
    EmitVertex();

    output.normal = input[1].normal;
    output.uv     = input[1].uv;
    output.bc     = vec3(0, area / length(v1), 0);
    gl_Position   = gl_in[1].gl_Position;
    EmitVertex();

    output.normal = input[2].normal;
    output.uv     = input[2].uv;
    output.bc     = vec3(0, 0, area / length(v2));
    gl_Position   = gl_in[2].gl_Position;
    EmitVertex();

    EndPrimitive();
}
