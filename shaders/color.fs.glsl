#version 150

// Uniforms
uniform mat4 mv;
uniform vec4 albedo;

// Input
in Block
{
    vec3 eye;
    vec3 normal;
    vec2 uv;
    vec3 bc;
}
ib;

// Output
out vec4 color;
out vec4 normal;

void main()
{
    mat3 nm = transpose(inverse(mat3(mv)));
    vec3 n  = normalize(nm * ib.normal);
    normal  = vec4(n.x * 0.5 + 0.5, n.y * 0.5 + 0.5, n.z * 0.5 + 0.5, 1.0);
    color   = albedo;
}
