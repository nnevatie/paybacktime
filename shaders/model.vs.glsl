#version 150

// Uniforms
uniform mat4 mvp;

// Input
in vec3 position;
in vec3 normal;
in vec3 tangent;
in vec2 uv;

// Output
out Block
{
    vec3 normal;
    vec2 uv;
}
ob;

void main()
{
    ob.normal   = normal;
    ob.uv       = uv;
    gl_Position = mvp * vec4(position, 1.0);
}
