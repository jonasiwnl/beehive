#include <iostream>

#include "streamserver.cpp"
#include "screencapture.cpp"

using std::cerr;


int main()
{
    /* TODO 1. Select a window to stream from */

    ScreenCapture capturer;
    bool success;

#ifdef __APPLE__
    success = capturer.osx_display_active_windows();
#elif _WIN32
    success = capturer.windows_display_active_windows();
#endif

    if (!success) {
        cerr << "ERROR: couldn't select window to stream from.\n";
        return 1;
    }

    capturer.capture();

    return 0; // This is the end of screen capture code. Stuff beyond is under construction

    /* TODO 2. Expose a port */
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        cerr << "ERROR: couldn't create socket.\n";
        return 1;
    }

    /* TODO 3. Listen for connections and accept them (as well as for interrupt) */
    StreamServer ss = StreamServer(client_socket);
    ss.listen_and_accept();
#ifdef _WIN32
    closesocket(client_socket);
#else
    close(client_socket);
#endif

    if (!ss.error_message.empty()) {
        cerr << ss.error_message;
        return 1;
    }

    return 0;
}
