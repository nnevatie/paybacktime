#version 150

// Uniforms
uniform mat4 mvp;
uniform mat4 mv;
uniform mat4 p;

// Input
in vec3 position;
in vec3 normal;
in vec2 uv;

// Output
out Block
{
    vec3 viewPos;
    vec3 normal;
    vec2 uv;
    vec3 bc;
}
ob;

void main()
{
    mat3 normalMat = transpose(inverse(mat3(mv)));
    vec4 viewPos   = mv * vec4(position, 1.0);
    ob.viewPos     = viewPos.xyz;
    ob.normal      = normalMat * normal;
    ob.uv          = uv;
    ob.bc          = vec3(1.0);
    gl_Position    = p * viewPos;
}
