#version 150

// Uniforms
uniform sampler2D texAlbedo;
uniform sampler2D texColor;
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
    float b = smoothstep(1.25, 3.0, length(texture(texColor, ib.uv)));
    color   = b + texture(texAlbedo, ib.uv) *
                  pow(texture(texLight, ib.uv).b * 2.0, 2.0);
}
