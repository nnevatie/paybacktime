#pragma once

#include <memory>
#include <vector>

#include <glad/glad.h>

#include "common/file_system.h"

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
        Mesh
    };

    Shader(Type type, const std::string& s);
    Shader(Type type, const filesystem::path& path);
    Shader(const filesystem::path& path);

    GLuint id() const;

    static filesystem::path path(const std::string& filename);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

struct ShaderProgram
{
    typedef std::pair<int, std::string> AttribLocation;

    ShaderProgram(const std::vector<Shader>& shaders,
                  const std::vector<AttribLocation>& attribs =
                  std::vector<AttribLocation>());

    GLuint id() const;
    ShaderProgram& bind();

    template<typename T>
    ShaderProgram& setUniform(const char* name, const T& v);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace gl
} // namespace hc