#version 150

// Uniforms
uniform sampler2D texAlbedo;
uniform sampler2D texLight;

// Input
in Block
{
    vec3 viewPos;
    vec3 normal;
    vec2 uv;
    vec3 bc;
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
    normal   = normalize(ib.normal);    
    color    = alb;
    //color    = vec4(mix(alb + 0.25, alb, edge(ib.bc)).rgb, 1.0);
    light    = texture(texLight, ib.uv).rgb;
}
