#version 150

// Uniforms
uniform sampler2D texColor;
uniform sampler2D texEnv;
uniform sampler2D texSsr;
uniform sampler2D texLight;
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
    float g  = scale * 2 * (1 - l.g);
    vec4 col = texture(texColor, ib.uv);
    vec4 ssr = texture(texSsr, ib.uv);
    vec4 ref = textureLod(texEnv, ssr.xy, g);
    color    = vec4(mix(col.rgb, ref.rgb, r * ssr.b * l.a), 1.0);
}
