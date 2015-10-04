#pragma once

#include <string>

namespace hc
{

struct Application
{
    Application();
    ~Application();

    bool run(const std::string& input);
};

} // namespace
