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
out vec4 light;
out vec4 incidence;

// Externals
vec4 textureTricubic(sampler3D, vec3, vec3);

void main(void)
{
    vec3 uvw  = vec3(ib.uv, z);
    light     = textureTricubic(texGi,  uvw, sizeTexGi);
    incidence = textureTricubic(texInc, uvw, sizeTexGi);
}
