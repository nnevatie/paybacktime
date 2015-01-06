#pragma once

#include <memory>
#include <vector>

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

private:
    struct Data;
    std::shared_ptr<Data> d;
};

struct ShaderProgram
{
    ShaderProgram(const std::vector<Shader>& shaders);
};

} // namespace gl
} // namespace hc
