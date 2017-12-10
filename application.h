#pragma once

#include <memory>
#include <string>

#include <boost/program_options.hpp>

namespace pt
{

struct Application
{
    Application() = default;

    bool run(const boost::program_options::variables_map& args);
};

} // namespace
