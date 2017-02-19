#version 150

// Uniforms
uniform sampler2D tex;
uniform float     weight;

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
    vec2 ts     = 1.0 / vec2(textureSize(tex, 0));
    vec4 eroded = vec4(1);

    for (int y = -2; y <= 2; ++y)
        for (int x = -2; x <= 2; ++x)
            eroded = min(eroded,
                         texture(tex, ib.uv + vec2(x * ts.x, y * ts.y)));

    color = vec4(mix(texture(tex, ib.uv).rgb, eroded.rgb, weight), 1.0);
}
