#include <GL/glew.h>
#include "gl_shaders.h"

namespace hc
{
namespace gl
{

ShaderProgram::ShaderProgram(const std::vector<Shader>& shaders)
{
}

Shader::Data::Data(GLenum shaderType)
{
    id = glCreateShader(shaderType);
}

Shader::Data::~Data()
{
    glDeleteShader(id);
}

} // namespace gl
} // namespace hc
