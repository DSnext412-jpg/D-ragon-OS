/**
 * @file    main.cpp
 * @brief   DragonOS bootstrap entry point.
 *
 * The sole responsibility of this file is to create the Application,
 * initialise it, run the message loop, and perform a clean shutdown.
 * No Win32 API calls appear here — all platform details are
 * encapsulated in the Window and Application classes.
 */

#include <Core/Application.hpp>

#include <cstdlib>

/**
 * @brief  Application entry point (wide-character version).
 *
 * The project defines UNICODE/_UNICODE, so the CRT's startup routine
 * calls wWinMain (not WinMain).  Using wWinMain with LPWSTR avoids
 * a "redefinition; different exception specifications" linker error.
 *
 * @return Exit code returned to the operating system.
 */
int WINAPI wWinMain(
    _In_     HINSTANCE hInstance,
    _In_opt_ HINSTANCE /*hPrevInstance*/,
    _In_     LPWSTR    /*lpCmdLine*/,
    _In_     int       nCmdShow) noexcept
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
