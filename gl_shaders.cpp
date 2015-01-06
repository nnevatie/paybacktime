#include "gl_shaders.h"

#include <GL/glew.h>
#include <GL/gl.h>

#include "common.h"

namespace hc
{
namespace gl
{

struct Shader::Data
{
    Data(GLenum shaderType, const std::string& source) :
        id(glCreateShader(shaderType)),
        source(source)
    {
    }
    ~Data()
    {
        glDeleteShader(id);
    }

    GLuint id;
    std::string source;
};


Shader::Shader(const std::string& s) :
    d(new Data(0, s))
{
}

Shader::Shader(const filesystem::path& path) :
    d(new Data(0, read(path)))
{
}

ShaderProgram::ShaderProgram(const std::vector<Shader>& shaders)
{
}

} // namespace gl
} // namespace hc
