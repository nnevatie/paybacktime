#version 150

// Uniforms
uniform mat4 mvp;

// Input
in vec3 position;
in vec3 normal;
in vec2 uv;

// Output
out Block
{
    vec3 normal;
    vec2 uv;
    vec3 bc;
}
ob;

void main()
{
    ob.normal   = normal;
    ob.uv       = uv;
    ob.bc       = vec3(1.0);
    gl_Position = mvp * vec4(position.xyz, 1);
}
