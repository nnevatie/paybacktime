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
    Shader(const std::string& s);
    Shader(const filesystem::path& path);

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
