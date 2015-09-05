#version 150

// Const
const vec3 ldir = vec3(0, -1, -1);

// Uniforms
uniform mat4 mv;

// Input
in Block
{
    vec3 normal;
    vec2 uv;
    vec3 bc;
}
ib;

// Output
out vec4 frag;

void main()
{
    mat3 nm = transpose(inverse(mat3(mv)));
    vec3 n  = normalize(nm * ib.normal);
    vec3 c  = vec3(1, 1, 1) * (0.25 + max(dot(n, ldir), 0.0));
    frag    = vec4(c, 1);
}
