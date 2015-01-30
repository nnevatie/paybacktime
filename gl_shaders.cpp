#include <GL/glew.h>

#include "gl_shaders.h"
#include "gl_uniforms.h"

#include "common.h"
#include "log.h"

namespace hc
{
namespace gl
{
namespace
{
    GLenum shaderType(Shader::Type type)
    {
        switch (type)
        {
            case Shader::Type::Vertex:   return GL_VERTEX_SHADER;
            case Shader::Type::Fragment: return GL_FRAGMENT_SHADER;
            case Shader::Type::Geometry: return GL_GEOMETRY_SHADER;
            case Shader::Type::Compute:  return GL_COMPUTE_SHADER;
        }
    }
}

struct Shader::Data
{
    Data(Shader::Type type, const std::string& source) :
        id(0),
        type(type),
        source(source)
    {
        id = glCreateShader(shaderType(type));

        const GLchar* src = source.c_str();
        glShaderSource(id, 1, &src, 0);
        glCompileShader(id);

        GLint success = 0;
        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            GLint logSize = 0;
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logSize);

            std::string infoLog(logSize, 0);
            glGetShaderInfoLog(id, logSize, &logSize, &infoLog[0]);

            HCLOG(Error) << infoLog;
            throw std::runtime_error("Shader compilation error");
        }
    }
    ~Data()
    {
        glDeleteShader(id);
    }

    GLuint id;
    Shader::Type type;
    std::string source;
};

Shader::Shader(Type type, const std::string& s) :
    d(new Data(type, s))
{
}

Shader::Shader(Type type, const filesystem::path& path) :
    d(new Data(type, readFile(path)))
{
}

GLuint Shader::id() const
{
    return d->id;
}

ShaderProgram::ShaderProgram(const std::vector<Shader>& shaders) :
    shaders(shaders)
{
    id = glCreateProgram();
    for (const Shader& shader : shaders)
        glAttachShader(id, shader.id());

    glLinkProgram(id);
}

ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(id);
}

ShaderProgram& ShaderProgram::bind()
{
    glUseProgram(id);
    return *this;
}

} // namespace gl
} // namespace hc
