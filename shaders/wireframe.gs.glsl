#version 150

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

// Uniforms
uniform vec2 size;

// Input
in Block
{
    vec3 viewPos;
    vec3 normal;
    vec2 uv;
    vec3 bc;
}
ib[];

// Output
out Block
{
    vec3 viewPos;
    vec3 normal;
    vec2 uv;
    vec3 bc;
}
ob;

void main(void)
{
    vec2 p0 = size * gl_in[0].gl_Position.xy / gl_in[0].gl_Position.w;
    vec2 p1 = size * gl_in[1].gl_Position.xy / gl_in[1].gl_Position.w;
    vec2 p2 = size * gl_in[2].gl_Position.xy / gl_in[2].gl_Position.w;

    vec2 v0 = p2 - p1;
    vec2 v1 = p2 - p0;
    vec2 v2 = p1 - p0;

    float area = abs(v1.x * v2.y - v1.y * v2.x);

    ob.viewPos  = ib[0].viewPos;
    ob.normal   = ib[0].normal;
    ob.uv       = ib[0].uv;
    ob.bc       = vec3(area / length(v0), 0, 0);
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();

    ob.viewPos  = ib[1].viewPos;
    ob.normal   = ib[1].normal;
    ob.uv       = ib[1].uv;
    ob.bc       = vec3(0, area / length(v1), 0);
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();

    ob.viewPos  = ib[2].viewPos;
    ob.normal   = ib[2].normal;
    ob.uv       = ib[2].uv;
    ob.bc       = vec3(0, 0, area / length(v2));
    gl_Position = gl_in[2].gl_Position;
    EmitVertex();

    EndPrimitive();
}
