#version 150

// Uniforms
uniform vec4 color;

// Input
in Block
{
    vec3 normal;
    vec2 uv;
    vec3 dist;
}
input;

// Output
out vec4 frag;

void main()
{
    vec3 n     = normalize(vec3(dFdx(gl_FragCoord.w), dFdy(gl_FragCoord.w), 1));
    vec3 l     = normalize(vec3(1, 10, 0));
    float d    = min(min(input.dist[0], input.dist[1]), input.dist[2]);
    float edge = exp2(-0.25 * d * d);

    vec4 c      = vec4(color.xyz + 1000 * dot(n, l), 1);
    frag        = mix(c, c + 0.5, edge);
}
