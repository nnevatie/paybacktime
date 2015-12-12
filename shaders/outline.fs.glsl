#version 150

// Uniforms
uniform sampler2D tex;
uniform sampler2D outline;

// Input
in Block
{
    vec2               uv;
    noperspective vec3 viewRay;
}
ib;

// Output
out vec4 color;

vec3 s(int dx, int dy, sampler2D t, vec2 uv)
{
    vec2 tsi = 1.0 / textureSize(t, 0);
    return texture2D(outline, uv + vec2(dx * tsi.x, dy * tsi.y)).rgb;
}

vec3 sobel(sampler2D t, vec2 uv)
{
    vec3 hc = s(-1, -1, t, uv) *  1.0 + s( 0, -1, t, uv) *  2.0 +
              s( 1, -1, t, uv) *  1.0 + s(-1,  1, t, uv) * -1.0 +
              s( 0,  1, t, uv) * -2.0 + s( 1,  1, t, uv) * -1.0;
    vec3 vc = s(-1, -1, t, uv) *  1.0 + s(-1,  0, t, uv) *  2.0 +
              s(-1,  1, t, uv) *  1.0 + s( 1, -1, t, uv) * -1.0 +
              s( 1,  0, t, uv) * -2.0 + s( 1,  1, t, uv) * -1.0;
    return vec3(0, 0.5, 0.75) * pow(length(vc * vc + hc * hc), 0.5);
}

void main(void)
{
    float o = texture(outline, ib.uv).r;
    vec4 c  = texture(tex, ib.uv);
    vec4 e  = vec4(sobel(outline, ib.uv), 1);
    color   = 0.1 * o * vec4(0, 0.5, 0.75, 1) + c + e;
}
