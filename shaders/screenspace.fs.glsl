#version 150

// Uniforms
uniform vec4 color;

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

float edge(vec3 bc)
{
    vec3 d  = fwidth(bc);
    vec3 a3 = smoothstep(vec3(0.0), d * 1.5, bc);
    return min(min(a3.x, a3.y), a3.z);
}

void main()
{
    vec3 n  = normalize(vec3(dFdx(gl_FragCoord.w), dFdy(gl_FragCoord.w), 1));
    vec3 l  = normalize(vec3(1, 10, 0));
    vec3 c  = vec3(color.xyz + 1000 * dot(n, l));
    frag    = vec4(mix(c + 0.25, c, edge(ib.bc)), color.a);
}
