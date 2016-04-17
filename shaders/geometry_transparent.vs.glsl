#version 150

// Uniforms
uniform mat4 m;
uniform mat4 v;
uniform mat4 p;

// Input
in vec3 position;
in vec3 normal;
in vec2 uv;

// Output
out Block
{
    vec3 worldPos;
    vec3 normal;
    vec2 uv;
}
ob;

void main()
{
    vec4 pos       = vec4(position, 1.0);
    mat3 normalMat = transpose(inverse(mat3(m)));
    ob.worldPos    = vec3(m * pos);
    ob.normal      = normalMat * normal;
    ob.uv          = uv;
    gl_Position    = p * v * m * pos;
}
