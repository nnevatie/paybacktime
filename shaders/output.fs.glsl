#version 150

// Uniforms
uniform sampler2D tex0;
uniform sampler2D tex1;

// Input
in Block
{
    vec2               uv;
    noperspective vec3 viewRay;
}
ib;

// Output
out vec4 color;

float A = 0.15;
float B = 0.50;
float C = 0.10;
float D = 0.20;
float E = 0.02;
float F = 0.30;
float W = 11.2;

vec3 tonemap(vec3 x)
{
   return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

void main(void)
{
   vec3 c0 = 12 * (texture(tex0, ib.uv) + texture(tex1, ib.uv)).rgb;
   vec3 c1 = tonemap(c0 * (vec3(1.0) / tonemap(vec3(W))));
   vec3 c2 = pow(c1, vec3(1.0 / 2.2));
   color   = vec4(c2, dot(c2.rgb, vec3(0.299, 0.587, 0.114)));
}
