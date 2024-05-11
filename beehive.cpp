#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <sys/socket.h>

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>
#endif

#define MAX_CONNECTIONS 10
#define OUTPUT_PATH "output/output.jpg"

using std::cout, std::cin, std::cerr; // IO
using std::string, std::vector; // Containers
using std::thread, std::atomic; // Multithreading


size_t select_window(size_t max_window_count)
{
    size_t window_idx = max_window_count+1;

    while (window_idx < 1 || window_idx > max_window_count) { // While invalid input
        cout << "Enter window index to stream from [1 - " << max_window_count << "]: ";
        if (cin >> window_idx) continue;
        // Bad input. Clear error and input buffer
        cin.clear();
        string dmy;
        cin >> dmy;
    }

    return window_idx-1; // Convert to 0-based index
}

struct ClientSocket {
    atomic<bool> server_up_flag;
    int client_socket;
    vector<thread> handlers;
    string error_message;

    ClientSocket(int client_socket_in): server_up_flag(true), client_socket(client_socket_in)
    {
        handlers.reserve(MAX_CONNECTIONS);
    }

    /* Listen for new connections, accept them, and handle with a separate thread. */
    void listen_and_accept()
    {
        thread quit_listener(&ClientSocket::wait_for_quit_input, this);
        while (server_up_flag) {
            int client = accept(client_socket, NULL, NULL);
            if (client < 0) {
                error_message = "ERROR: couldn't accept connection.\n";
                server_up_flag = false;
                break;
            }

            /* Spin up a thread for each client. */
            handlers.emplace_back(thread(&ClientSocket::handle_client, this, client));
        }
        quit_listener.join();
        cleanup_handler_threads();
    }

    /* when the user types "q" or "quit", end execution of the server (stop streaming.) */
    void wait_for_quit_input()
    {
        string input;
        while (server_up_flag) {
            cin >> input;
            if (input == "q" || input == "quit") server_up_flag = false;
        }
    }

    void handle_client(int client_socket)
    {
        string message = "What up\n";
        while (server_up_flag) {
            /* TODO (jonas): how can client disconnects be handled */
            send(client_socket, message.c_str(), message.size(), 0);
            sleep(1);
        }
    }

    void cleanup_handler_threads()
    {
        for (auto& handler : handlers)
            // These threads should be done executing, as server_up_flag is false
            // So join *should be* safe here
            handler.join();
    }
};

int main()
{
    /* TODO 1. Select a window to stream from */

#ifdef __APPLE__ /* OSX only code. */
    CFArrayRef window_infolist_ref = CGWindowListCopyWindowInfo(
        kCGWindowListOptionOnScreenOnly |
        kCGWindowListExcludeDesktopElements |
        kCGWindowListOptionIncludingWindow,
        kCGNullWindowID
    );
    CFIndex window_count = CFArrayGetCount(window_infolist_ref);

    /* Window IDs that can actually be streamed from */
    vector<CFNumberRef> active_window_ids;

    if (window_infolist_ref == NULL) {
        cerr << "ERROR: couldn't get window list.\n";
        return 1;
    }

    /* Iterate through window info list and print the name if it exists. */
    for (CFIndex idx = 0; idx < window_count; ++idx) {
        CFDictionaryRef window_info_ref = (CFDictionaryRef) CFArrayGetValueAtIndex(window_infolist_ref, idx);
        /* If the window doesn't have a name, skip */
        if (!CFDictionaryContainsKey(window_info_ref, kCGWindowOwnerName)) continue;

        int window_layer;
        CFNumberRef window_layer_ref = (CFNumberRef) CFDictionaryGetValue(window_info_ref, kCGWindowLayer);
        CFNumberGetValue(window_layer_ref, kCFNumberIntType, &window_layer);
        if (window_layer > 1) continue; /* Skip windows that are not at the top layer. */

        CFStringRef window_name_ref = (CFStringRef) CFDictionaryGetValue(window_info_ref, kCGWindowOwnerName);
        /* This window can be streamed from. */
        active_window_ids.push_back((CFNumberRef) CFDictionaryGetValue(window_info_ref, kCGWindowNumber));
        cout << active_window_ids.size() << ". "
            << CFStringGetCStringPtr(window_name_ref, kCFStringEncodingUTF8) << "      ";
    }
    cout << '\n';

    /* Get ID of selected window. */
    size_t selected_window_idx = select_window(active_window_ids.size());
    CFNumberRef window_id_ref = active_window_ids[selected_window_idx];
    CGWindowID selected_window_id;
    CFNumberGetValue(window_id_ref, kCGWindowIDCFNumberType, &selected_window_id);

    CGImageRef img = CGWindowListCreateImage(
        CGRectNull, /* CGRectNull means capture minimum size needed in order to capture window */
        kCGWindowListExcludeDesktopElements | kCGWindowListOptionIncludingWindow,
        selected_window_id,
        kCGWindowImageBestResolution
    );

    /* Now, we set up disk IO. */
    CFStringRef output_path =
        CFStringCreateWithCString(kCFAllocatorDefault, OUTPUT_PATH, kCFStringEncodingUTF8);
    CFURLRef url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, output_path, kCFURLPOSIXPathStyle, false);
    CGImageDestinationRef dest = CGImageDestinationCreateWithURL(url, kUTTypeJPEG, 1, NULL);

    /* The NULL is addl properties, might be good to look into */
    CGImageDestinationAddImage(dest, img, NULL);
    bool write_success = CGImageDestinationFinalize(dest); /* Write to disk. */

    // Clean up memory
    CFRelease(window_infolist_ref);
    CFRelease(dest);
    CGImageRelease(img);

    if (!write_success) {
        cerr << "ERROR: couldn't create or write image.\n";
        return 1;
    }
#endif /* __APPLE__ */

    /* TODO 2. Expose a port */
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        cerr << "ERROR: couldn't create socket.\n";
        return 1;
    } /* TODO continue this */

    /* TODO 3. Listen for connections and accept them (as well as for interrupt) */
    ClientSocket cs = ClientSocket(client_socket);
    cs.listen_and_accept();
    close(client_socket);

    if (!cs.error_message.empty()) {
        cerr << cs.error_message;
        return 1;
    }

    /* TODO 4. Stream window to connections */

    return 0;
}
