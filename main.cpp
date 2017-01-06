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
            ("fullscreen,f", "Full screen mode");

        variables_map args;
        store(parse_command_line(argc, argv, desc), args);

        pt::Application app;
        app.run(args);
    }
    catch (const std::exception& e)
    {
        PTLOG(Fatal) << "An exception occurred: " << e.what();
        return 1;
    }
    return 0;
}
