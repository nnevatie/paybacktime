#version 150

// Uniforms
uniform sampler2D texColor;
uniform sampler2D texSsr;
uniform sampler2D texLight;

// Input
in Block
{
    vec2               uv;
    noperspective vec3 viewRay;
}
ib;

// Output
out vec4 color;

void main(void)
{
    vec4  l  = texture(texLight, ib.uv);
    float r  = l.r;
    float g  = 2 * (1 - l.g);
    vec4 col = texture(texColor, ib.uv);
    vec4 ssr = textureLod(texSsr, ib.uv, g);
    color    = vec4(mix(col.rgb, ssr.rgb, r * ssr.a), 1.0);
}
