#pragma once

#include <string>

#include "platform/context.h"

namespace hc
{

struct Application
{
    Application();
    virtual ~Application();

    bool run(const std::string& input);

    platform::Context context;
};

} // namespace
