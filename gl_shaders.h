#pragma once

#include <memory>
#include <vector>

#include <GL/gl.h>

namespace hc
{
namespace gl
{

struct Shader
{


private:
    struct Data
    {
        Data(GLenum shaderType);
        ~Data();

        GLuint id;
    };

    std::shared_ptr<Data> data;
};

struct ShaderProgram
{
    ShaderProgram(const std::vector<Shader>& shaders);
};

} // namespace gl
} // namespace hc
