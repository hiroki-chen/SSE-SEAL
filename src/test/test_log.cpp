#include <plog/Initializers/RollingFileInitializer.h>
#include <plog/Log.h>

int main(int argc, const char** argv)
{
    plog::init(plog::debug, "ok.txt");
    PLOG(plog::info) << "ok";

    return 0;
}