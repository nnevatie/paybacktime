#version 150

// Uniforms
uniform sampler2D tex;
uniform sampler2D outline;
uniform vec4      fillColor;

// Input
in Block
{
    vec2               uv;
    noperspective vec3 viewRay;
}
ib;

// Output
out vec4 color;

float s(int dx, int dy, sampler2D t, vec2 uv)
{
    vec2 tsi = 1.0 / textureSize(t, 0);
    return texture(outline, uv + vec2(dx * tsi.x, dy * tsi.y)).r;
}

float sobel(sampler2D t, vec2 uv)
{
    float hc = s(-1, -1, t, uv) *  1.0 + s( 0, -1, t, uv) *  2.0 +
               s( 1, -1, t, uv) *  1.0 + s(-1,  1, t, uv) * -1.0 +
               s( 0,  1, t, uv) * -2.0 + s( 1,  1, t, uv) * -1.0;
    float vc = s(-1, -1, t, uv) *  1.0 + s(-1,  0, t, uv) *  2.0 +
               s(-1,  1, t, uv) *  1.0 + s( 1, -1, t, uv) * -1.0 +
               s( 1,  0, t, uv) * -2.0 + s( 1,  1, t, uv) * -1.0;
    return pow(sqrt(vc * vc + hc * hc), 0.5);
}

void main(void)
{
    float o = texture(outline, ib.uv).r;
    float e = sobel(outline, ib.uv);
    vec4 c  = texture(tex, ib.uv);
    color   = c + (0.1 * o + e * 0.2) * fillColor;
}
