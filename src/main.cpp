
#include <Core/Application.hpp>

#include <cstdlib>


int WINAPI wWinMain(
    _In_     HINSTANCE hInstance,
    _In_opt_ HINSTANCE ,
    _In_     LPWSTR    ,
    _In_     int       nCmdShow)
{
    DragonOS::Core::Application app;

    if (!app.Initialize(hInstance, nCmdShow))
    {
        return EXIT_FAILURE;
    }

    const int exitCode = app.Run();
    app.Shutdown();
    return exitCode;
}
