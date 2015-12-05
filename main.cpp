#include <stdexcept>

#include <boost/program_options.hpp>

#include "common/log.h"
#include "application.h"

int main(int argc, char** argv)
{
    try
    {
        using namespace boost::program_options;
        options_description desc("Allowed options");
        desc.add_options()
            ("input,i",
             value<std::string>()->default_value("box"),
             "Input name");

        variables_map args;
        store(parse_command_line(argc, argv, desc), args);

        hc::Application app;
        app.run(args["input"].as<std::string>());
    }
    catch (const std::exception& e)
    {
        HCLOG(Fatal) << "An exception occurred: " << e.what();
        return 1;
    }
    return 0;
}
