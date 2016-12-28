#pragma once

#include <memory>
#include <string>

#include "common/file_system.h"
#include "img/image.h"
#include "gl/texture.h"

namespace pt
{

struct Horizon
{
    Horizon();
    Horizon(const Image& image, const std::string& name);
    Horizon(const fs::path& path);

    operator bool() const;

    operator==(const Horizon& other) const;
    operator!=(const Horizon& other) const;

    std::string name() const;
    gl::Texture texture() const;

    static Horizon none();

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace pt
