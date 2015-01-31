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

Image::Image(const Image& image)
{
    HCTIME("Copy constructor");
    surface_ = SDL_CreateRGBSurface(
        0,
        image.surface_->w,
        image.surface_->h,
        image.surface_->format->BitsPerPixel,
        image.surface_->format->Rmask,
        image.surface_->format->Gmask,
        image.surface_->format->Bmask,
        image.surface_->format->Amask
    );
    SDL_Rect rect = {0, 0, image.surface_->w, image.surface_->h};
    SDL_LowerBlit(image.surface_, &rect, surface_, &rect);
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
