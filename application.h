#pragma once

#include <string>

namespace pt
{

struct Application
{
    Application();
    virtual ~Application();

    bool run(const std::string& input);
};

} // namespace
