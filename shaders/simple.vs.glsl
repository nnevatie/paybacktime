#version 150

// Uniforms
uniform mat4 mvp;
uniform mat4 mv;

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
    vec4 vertex = mv * vec4(position, 1.0);
    ob.normal   = normal;
    ob.uv       = uv;
    ob.bc       = vec3(1.0);
    gl_Position = mvp * vec4(position, 1.0);
}
