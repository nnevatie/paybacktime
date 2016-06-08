#version 150

// Uniforms
uniform sampler2D texAlbedo;
uniform sampler2D texNormal;
uniform sampler2D texLight;

// Input
in Block
{
    vec3 viewPos;
    vec2 uv;
    vec3 bc;
    mat3 tbn;
}
ib;

// Output
out vec3 normal;
out vec4 color;
out vec3 light;

float edge(vec3 bc)
{
    vec3 d  = fwidth(bc);
    vec3 a3 = smoothstep(vec3(0.0), d * 1.0, bc);
    return min(min(a3.x, a3.y), a3.z);
}

void main()
{
    vec4 alb = texture(texAlbedo, ib.uv);
    vec3 n   = texture(texNormal, ib.uv).rgb;
    normal   = normalize(ib.tbn * normalize(n * 2.0 - 1.0));
    color    = alb;
    //color    = vec4(mix(alb + 0.25, alb, edge(ib.bc)).rgb, 1.0);
    light    = texture(texLight, ib.uv).rgb;
}
