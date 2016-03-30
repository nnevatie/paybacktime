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
    vec4 a = texture(texAlbedo, ib.uv);
    vec4 c = texture(texColor,  ib.uv);
    vec4 b = smoothstep(vec4(1.0), vec4(5.0), c);
    color  = b + a * pow(texture(texLight, ib.uv).b * 4.0, 1.0);
    //color = vec4(0);
}
