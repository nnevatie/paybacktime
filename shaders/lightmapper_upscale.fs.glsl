#version 150

// Uniforms
uniform sampler3D texGi;
uniform sampler3D texInc;
uniform float     z;

// Const
vec3 sizeTexGi = textureSize(texGi, 0);

// Input
in Block
{
    vec2               uv;
    noperspective vec3 viewRay;
}
ib;

// Outputs
out vec3 light;
out vec3 incidence;

// Externals
vec4 textureTricubic(sampler3D, vec3, vec3);

void main(void)
{
    vec3 uvw  = vec3(ib.uv, z);
    light     = textureTricubic(texGi,  uvw, sizeTexGi).rgb;
    incidence = textureTricubic(texInc, uvw, sizeTexGi).rgb;
}
