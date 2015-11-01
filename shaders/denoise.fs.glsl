#version 150

// Uniforms
uniform sampler2D tex;

// Input
in Block
{
    vec2 uv;
}
ib;

// Output
out vec4 color;

void main(void)
{
    vec2 ts  = 1.0 / vec2(textureSize(tex, 0));
    vec4 s   = texture(tex, ib.uv);
    vec4 avg = 0.25 * (texture(tex, vec2(ib.uv.s - ts.s, ib.uv.t))        +
                       texture(tex, vec2(ib.uv.s + ts.s, ib.uv.t))        +
                       texture(tex, vec2(ib.uv.s,        ib.uv.t - ts.t)) +
                       texture(tex, vec2(ib.uv.s,        ib.uv.t + ts.t)));
    vec4 d   = abs(s - avg);
    color    = dot(d, d) > 0.5 ? avg : s;
}
