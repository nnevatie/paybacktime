#pragma once

#include <memory>
#include <vector>

#include <glad/glad.h>

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
        Texture3d,
        Array1d,
        Array2d,
        Buffer
    };

    Texture(Type type = Type::Texture2d);

    GLuint id() const;
    Type type() const;

    Size<int> size();
    int dimensions() const;

    Image image();

    Texture& bind();
    Texture& bindAs(GLenum unit);
    Texture& unbind();

    static void unbind(GLenum target, GLenum unit);

    Texture& set(GLenum name, GLint param);
    Texture& set(GLenum name, GLfloat param);

    Texture& alloc(GLint internalFormat, const Buffer& buffer);

    Texture& alloc(const std::vector<int>& dim,
                   GLint internalFormat, GLenum format,
                   GLenum type = GL_UNSIGNED_BYTE, const GLvoid* data = 0);

    Texture& alloc(int level, const std::vector<int>& dim,
                   GLint internalFormat, GLenum format,
                   GLenum type = GL_UNSIGNED_BYTE, const GLvoid* data = 0);

    Texture& alloc(const Image& image);
    Texture& alloc(const Grid<glm::vec3>& grid);

    static float anisotropyMax();

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace gl
} // namespace pt
