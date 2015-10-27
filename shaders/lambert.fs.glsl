#version 150

// Const
const vec3 ldir = vec3(0, -1, -1);

// Uniforms
uniform mat4 mvp;
uniform mat4 mv;
uniform mat4 p;

// Input
in Block
{
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
    vec3 c  = vec3(1, 1, 1) * (0.25 + max(dot(n, ldir), 0.0));
    color   = vec4(c, 1);
    normal  = vec4(n.x * 0.5 + 0.5, n.y * 0.5 + 0.5, n.z * 0.5 + 0.5, 1.0);
}
