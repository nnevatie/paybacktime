 #pragma once

#include <memory>
#include <vector>

#include <glad/glad.h>

#include <glm/vec3.hpp>

#include "geom/size.h"
#include "geom/grid.h"
#include "img/image.h"
#include "buffers.h"

namespace pt
{
namespace gl
{

struct Texture
{
    enum class Type
    {
        Texture1d,
        Texture2d,
        Texture2dMs,
        Texture3d,
        Array1d,
        Array2d,
        Buffer
    };

    Texture(Type type = Type::Texture2d, int sampleCount = 1);

    operator bool();

    GLuint id() const;
    Type type() const;

    glm::ivec3 size();
    int dimensions() const;
    int sampleCount() const;

    Image image();

    Texture& bind();
    Texture& bindAs(GLenum unit);
    Texture& unbind();

    static void unbind(GLenum target, GLenum unit);

    Texture& set(GLenum name, GLenum param);
    Texture& set(GLenum name, GLint param);
    Texture& set(GLenum name, GLfloat param);

    Texture& alloc(GLenum internalFormat, const Buffer& buffer);

    Texture& alloc(const std::vector<int>& dim,
                   GLenum internalFormat, GLenum format,
                   GLenum type = GL_UNSIGNED_BYTE, const GLvoid* data = nullptr);

    Texture& alloc(int level, const std::vector<int>& dim,
                   GLenum internalFormat, GLenum format,
                   GLenum type = GL_UNSIGNED_BYTE, const GLvoid* data = nullptr);

    Texture& alloc(const Image& image, bool srgb = true);
    Texture& alloc(const Grid<float>& grid);
    Texture& alloc(const Grid<glm::vec3>& grid);
    Texture& alloc(const Grid<glm::vec4>& grid);

    static float anisotropyMax();

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace gl
} // namespace pt
