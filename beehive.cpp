#include <iostream>

#include "screencapture.cpp"

using std::cerr;


int main()
{
    ScreenCapture capturer(false);
    bool success;

    // Select a screen to stream from
#ifdef __APPLE__
    success = capturer.osx_display_active_windows();
#elif _WIN32
    success = capturer.windows_display_active_windows();
#endif

    if (!success) {
        cerr << "ERROR: couldn't select window to stream from.\n";
        return 1;
    }

    // Start capturing
    capturer.capture();

    return 0;
}
