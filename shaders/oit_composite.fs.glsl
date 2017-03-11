#version 150

// Uniforms
uniform sampler2D texOpq;
uniform sampler2D texEnv;
uniform sampler2D texOit0;
uniform sampler2D texOit1;

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
    vec4 accum  = texture(texOit0, ib.uv);
    float alpha = 1.0 - clamp(accum.a, 0.0, 1.0);
    accum.a     = texture(texOit1, ib.uv).r;
    vec3 opaque = alpha > 0.0 ? textureLod(texEnv, ib.uv, 2 * alpha).rgb :
                                texture(texOpq, ib.uv).rgb;
    vec3 trans  = vec3(accum.rgb / clamp(accum.a, 1e-4, 5e4));
    color       = vec4(mix(opaque, trans, alpha), 1.0);
}
