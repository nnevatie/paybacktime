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
    const std::array<GLenum, 4> types =
    {
        GL_VERTEX_SHADER,
        GL_FRAGMENT_SHADER,
        GL_GEOMETRY_SHADER,
        GL_COMPUTE_SHADER
    };
    const int index = int(type);

    if (index < 0 || index >= int(types.size()))
        throw std::runtime_error("Invalid shader type " +
                                 std::to_string(index));

    return types.at(index);
}

Shader::Type typeFromExt(const filesystem::path& path)
{
    typedef std::pair<std::string, Shader::Type> Type;
    const Type types[] =
    {
        {".vs", Shader::Type::Vertex},
        {".fs", Shader::Type::Fragment},
        {".gs", Shader::Type::Geometry},
        {".cs", Shader::Type::Compute}
    };
    for (const Type& type : types)
        if (path.extension().string()        == type.first ||
            path.stem().extension().string() == type.first)
            return type.second;

    throw std::runtime_error("Cannot determine shader "
                             "type for path: " + path.string());
}

} // namespace

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

Shader::Shader(const filesystem::path& path) :
    d(new Data(typeFromExt(path), readFile(path)))
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
