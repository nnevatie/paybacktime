#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "platform/gl.h"

namespace pt
{
namespace gl
{

template<>
ShaderProgram& ShaderProgram::setUniform<bool>(
    const char* name, const bool& v)
{
    glUniform1i(glGetUniformLocation(id(), name), v);
    return *this;
}

template<>
ShaderProgram& ShaderProgram::setUniform<int>(
    const char* name, const int& v)
{
    glUniform1i(glGetUniformLocation(id(), name), v);
    return *this;
}

template<>
ShaderProgram& ShaderProgram::setUniform<float>(
    const char* name, const float& v)
{
    glUniform1f(glGetUniformLocation(id(), name), v);
    return *this;
}

template<>
ShaderProgram& ShaderProgram::setUniform<glm::vec2>(
    const char* name, const glm::vec2& v)
{
    glUniform2fv(glGetUniformLocation(id(), name),
                 1, glm::value_ptr(v));
    return *this;
}

template<>
ShaderProgram& ShaderProgram::setUniform<glm::vec3>(
    const char* name, const glm::vec3& v)
{
    glUniform3fv(glGetUniformLocation(id(), name),
                 1, glm::value_ptr(v));
    return *this;
}

template<>
ShaderProgram& ShaderProgram::setUniform<glm::vec4>(
    const char* name, const glm::vec4& v)
{
    glUniform4fv(glGetUniformLocation(id(), name),
                 1, glm::value_ptr(v));
    return *this;
}

template<>
ShaderProgram& ShaderProgram::setUniform<glm::mat3>(
    const char* name, const glm::mat3& v)
{
    glUniformMatrix3fv(glGetUniformLocation(id(), name),
                       1, GL_FALSE, glm::value_ptr(v));
    return *this;
}

template<>
ShaderProgram& ShaderProgram::setUniform<glm::mat4>(
    const char* name, const glm::mat4& v)
{
    glUniformMatrix4fv(glGetUniformLocation(id(), name),
                       1, GL_FALSE, glm::value_ptr(v));
    return *this;
}

template<>
ShaderProgram& ShaderProgram::setUniform<std::vector<glm::vec3>>(
    const char* name, const std::vector<glm::vec3>& v)
{
    glUniform3fv(glGetUniformLocation(id(), name),
                 v.size(), (const GLfloat*) v.data());
    return *this;
}

} // namespace gl
} // namespace pt
