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
    vec4 c = texture(texColor, ib.uv);
    vec4 b = smoothstep(vec4(0.75), vec4(2.5), c);
    color  = b + c * pow(texture(texLight, ib.uv).b * 8.0, 2.0);
}
