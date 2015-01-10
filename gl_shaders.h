#pragma once

#include <memory>
#include <vector>

#include <GL/glew.h>

#include "file_system.h"

namespace hc
{
namespace gl
{

struct Shader
{
    enum class Type
    {
        Vertex,
        Fragment,
        Geometry,
        Compute
    };

    Shader(Type type, const std::string& s);
    Shader(Type type, const filesystem::path& path);

    GLuint id() const;

private:
    struct Data;
    std::shared_ptr<Data> d;
};

struct ShaderProgram
{
    ShaderProgram(const std::vector<Shader>& shaders);
    ~ShaderProgram();

    ShaderProgram& bind();

    template<typename T>
    ShaderProgram& setUniform(const char* name, const T& v);

    GLuint id;
    std::vector<Shader> shaders;
};

} // namespace gl
} // namespace hc
