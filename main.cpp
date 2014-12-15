#include <stdexcept>

#include "application.h"
#include "log.h"

int main(int /*argc*/, char** /*argv*/)
{
    try
    {
        hc::Application app;
        app.run();
    }
    catch (const std::exception& e)
    {
        HCLOG(Fatal) << "An exception occurred: " << e.what();
    }
    return 0;
}
