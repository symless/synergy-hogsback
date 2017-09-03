#include <App.h>

bool installServiceHelper();

// HACK: stub mac function if not mac
#ifndef __APPLE__
bool installServiceHelper()
{
    return false;
}
#endif

int
main(int argc, char* argv[])
{
    App app(installServiceHelper);
    return app.run(argc, argv);
}
