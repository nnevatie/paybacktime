#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace hc
{
namespace gl
{

template<>
ShaderProgram& ShaderProgram::setUniform<glm::vec4>(
    const char* name, const glm::vec4& v)
{
    glUniform4fv(glGetUniformLocation(id, name),
                 1, glm::value_ptr(v));
    return *this;
}

template<>
ShaderProgram& ShaderProgram::setUniform<glm::mat4>(
    const char* name, const glm::mat4& v)
{
    glUniformMatrix4fv(glGetUniformLocation(id, name),
                       1, GL_FALSE, glm::value_ptr(v));
    return *this;
}

} // namespace gl
} // namespace hc
