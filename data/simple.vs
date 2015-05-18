#version 150

// Uniforms
uniform mat4 transform;

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
output;

void main()
{
    output.normal = normal;
    output.uv     = uv;
    output.bc     = vec3(1.0);
    gl_Position   = transform * vec4(position.xyz, 1);
}
