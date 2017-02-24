#version 150

// Uniforms
uniform sampler2D texColor;
uniform sampler2D texSsr;
uniform sampler2D texLight;
uniform sampler2D texNormal;
uniform float     scale;

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
    float g  = 1 * (1 - l.g);
    vec4 col = texture(texColor, ib.uv);
    vec4 ssr = textureLod(texSsr, ib.uv, scale * g);
    color    = ib.uv.x < 0.5 ? vec4(mix(col.rgb, ssr.rgb, r * ssr.a), 1.0) : col;
}
