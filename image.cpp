#include "image.h"

#include "clock.h"
#include "log.h"

namespace hc
{

Image::Image() :
    surface_(nullptr)
{
    HCTIME("Default constructor");
}

Image::Image(const Image& /*image*/)
{
    HCTIME("Copy constructor");
}

Image::Image(const std::string& filename)
{
    HCTIME("I/O constructor");
    surface_ = IMG_Load(filename.c_str());
    if (!surface_)
        HCLOG(Warn) << "Could not read image file " << filename;
}

Image::~Image()
{
    HCTIME("Destructor");
    SDL_FreeSurface(surface_);
}

SDL_Surface* Image::surface() const
{
    return surface_;
}

} // namespace
