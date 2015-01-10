#pragma once

#include <GL/glew.h>
#include <glm/vec4.hpp>

namespace hc
{
namespace gl
{

template<>
ShaderProgram& ShaderProgram::setUniform<glm::vec4>(
    const char* name, const glm::vec4& v)
{
    glUniform4f(glGetUniformLocation(id, name), v[0], v[1], v[2], v[3]);
    return *this;
}

} // namespace gl
} // namespace hc
