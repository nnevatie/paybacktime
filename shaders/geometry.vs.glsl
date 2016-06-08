#version 150

// Uniforms
uniform mat4 m;
uniform mat4 v;
uniform mat4 p;

// Input
in vec3 position;
in vec3 normal;
in vec3 tangent;
in vec2 uv;

// Output
out Block
{
    vec3 viewPos;
    vec2 uv;
    vec3 bc;
    mat3 tbn;
}
ob;

void main()
{
    mat4 mv      = v * m;
    vec3 t       = normalize(mat3(mv) * tangent);
    vec3 n       = normalize(mat3(mv) * normal);
    vec3 b       = normalize(cross(t, n));
    vec4 viewPos = mv * vec4(position, 1.0);
    ob.viewPos   = viewPos.xyz;
    ob.uv        = uv;
    ob.bc        = vec3(1);
    ob.tbn       = mat3(t, b, n);
    gl_Position  = p * viewPos;
}
