#pragma once

#include <string>

namespace hc
{

struct Application
{
    Application();
    virtual ~Application();

    bool run(const std::string& input);
};

} // namespace
