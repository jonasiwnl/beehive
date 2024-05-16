#include <iostream>
#include <string>
#include <vector>
#include <atomic>
#include <chrono>
#include <thread>

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>
#elif _WIN32 || _WIN64
#include <stdio.h> // _popen, _pclose
#endif

using std::cout, std::cin, std::cerr; // IO
using std::string, std::to_string, std::vector, std::pair; // Container
using std::this_thread::sleep_for, std::chrono::milliseconds, std::atomic, std::thread; // Multithreading


constexpr auto FPS = "60";
constexpr auto MPEG4_OUTPUT = "-vcodec mpeg4 output/video.mp4";
// constexpr auto MPEGTS_OUTPUT = "-vcodec libx264 -preset ultrafast -f mpegts output/video.ts";
// constexpr auto STREAM_OUTPUT = "rtsp://localhost:8554/stream";

struct ScreenCapture {
    // ---- CROSS PLATFORM CODE ---- //
    atomic<bool> server_up_flag;
    size_t window_height;
    size_t window_width;

    ScreenCapture(): server_up_flag{true}, window_height{0}, window_width{0} {}

    /*
     * user_select_window_idx makes the user pick an window from [1, max_window_count]
     * and subtracts 1 (for 0-based indexing)
     *
     * Returns: index from [0, max_window_count)
     */
    size_t user_select_window_idx(size_t max_window_count)
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

    /*
     * Allows for easy program termination without SIGINT.
     */
    void wait_for_quit_input()
    {
        string input;
        while (server_up_flag) {
            cin >> input;
            if (input == "q" || input == "quit") server_up_flag = false;
        }
    }

    /*
     * capture starts capturing video from selected window using ffmpeg.
     * it runs until 'q' or 'quit' is typed.
     */
    bool capture()
    {
        string dimensions = to_string(window_width) + 'x' + to_string(window_height);
        string ffmpeg_cmd =
            "ffmpeg -hide_banner -y -f rawvideo -video_size " + dimensions +
            " -pix_fmt bgra -re -i - -r " + FPS + ' ' + MPEG4_OUTPUT;
    #ifdef _WIN32 || _WIN64
        FILE* streampipe = _popen(ffmpeg_cmd.c_str(), "wb");
    #else
        FILE* streampipe = popen(ffmpeg_cmd.c_str(), "w");
    #endif

        thread quit_listener(&ScreenCapture::wait_for_quit_input, this);

        while (server_up_flag) {
        #ifdef __APPLE__
            osx_pipe_image(streampipe);
        #endif
            sleep_for(milliseconds(100)); // TODO: how long should this be
        }

        quit_listener.join();
    #ifdef _WIN32 || _WIN64
        _pclose(streampipe);
    #else
        pclose(streampipe);
    #endif

        return true;
    }
    // ---- END CROSS PLATFORM CODE ---- //

#ifdef __APPLE__ // ---- OSX ONLY ---- //
    CGWindowID selected_window_id;

    /*
     * osx_pipe_image captures a single image on OSX using the CoreGraphics.h library
     * and pipes it to the `streampipe` argument.
     * 
     * Returns: boolean indicating whether the image could be created
     */
    bool osx_pipe_image(FILE* streampipe)
    {
        CGImageRef img = CGWindowListCreateImage(
            CGRectNull, // CGRectNull means capture minimum size needed in order to capture window
            kCGWindowListExcludeDesktopElements | kCGWindowListOptionIncludingWindow,
            selected_window_id,
            kCGWindowImageBestResolution | kCGWindowImageBoundsIgnoreFraming
        );

        if (CGImageGetBitsPerPixel(img) != 32 || CGImageGetBitsPerComponent(img) != 8) {
            CGImageRelease(img);
            return false;
        }

        CFDataRef img_data = CGDataProviderCopyData(CGImageGetDataProvider(img));
        const UInt8 * img_data_ptr = (const UInt8 *) CFDataGetBytePtr(img_data);

        size_t data_length = static_cast<size_t>(CFDataGetLength(img_data));

        fwrite(img_data_ptr, 1, data_length, streampipe);
        fflush(streampipe);

        CGImageRelease(img);
        CFRelease(img_data);

        return true;
    }

    /*
     * osx_display_active_windows displays a list of user windows that can be streamed from on OSX,
     * using the CoreGraphics.h library.
     *
     * Returns: bool indicating whether or not this function succeeded.
     */
    bool osx_display_active_windows()
    {
        CFArrayRef window_infolist_ref = CGWindowListCopyWindowInfo(
            kCGWindowListOptionOnScreenOnly |
            kCGWindowListExcludeDesktopElements |
            kCGWindowListOptionIncludingWindow,
            kCGNullWindowID
        );
        CFIndex window_count = CFArrayGetCount(window_infolist_ref);

        // Window IDs that can actually be streamed from and their bounds, as a dict
        vector<pair<CFNumberRef, CFDictionaryRef>> active_window_ids;

        if (window_infolist_ref == NULL) {
            cerr << "ERROR: couldn't get window list.\n";
            return false;
        }

        // Iterate through window info list and print the name if it exists.
        for (CFIndex idx = 0; idx < window_count; ++idx) {
            CFDictionaryRef window_info_ref = (CFDictionaryRef) CFArrayGetValueAtIndex(window_infolist_ref, idx);
            // If the window doesn't have a name, skip
            if (!CFDictionaryContainsKey(window_info_ref, kCGWindowOwnerName)) continue;

            int window_layer;
            CFNumberRef window_layer_ref = (CFNumberRef) CFDictionaryGetValue(window_info_ref, kCGWindowLayer);
            CFNumberGetValue(window_layer_ref, kCFNumberIntType, &window_layer);
            if (window_layer > 1) continue; // Skip windows that are not at the top layer.

            CFStringRef window_name_ref = (CFStringRef) CFDictionaryGetValue(window_info_ref, kCGWindowOwnerName);
            // This window can be streamed from.
            active_window_ids.push_back({ (CFNumberRef) CFDictionaryGetValue(window_info_ref, kCGWindowNumber),
                                          (CFDictionaryRef) CFDictionaryGetValue(window_info_ref, kCGWindowBounds) });
            cout << active_window_ids.size() << ". "
                << CFStringGetCStringPtr(window_name_ref, kCFStringEncodingUTF8) << "      ";
        }
        cout << '\n';

        // Get ID of selected window.
        size_t selected_window_idx = user_select_window_idx(active_window_ids.size());
        pair<CFNumberRef, CFDictionaryRef> window_id_and_bounds = active_window_ids[selected_window_idx];
        CFNumberGetValue(window_id_and_bounds.first, kCGWindowIDCFNumberType, &selected_window_id);

        CGRect window_bounds;
        CGRectMakeWithDictionaryRepresentation(window_id_and_bounds.second, &window_bounds);

        window_height = static_cast<size_t>(CGRectGetHeight(window_bounds)*2);
        window_width = static_cast<size_t>(CGRectGetWidth(window_bounds)*2);

        // Clean up memory
        CFRelease(window_infolist_ref);

        return true;
    }

    /*
    // Using CGDisplayStream might be more efficient. Test this!
    void record_for_three_seconds()
    {
        CGRect mainMonitor = CGDisplayBounds(CGMainDisplayID());
        CGFloat monitorHeight = CGRectGetHeight(mainMonitor);
        CGFloat monitorWidth = CGRectGetWidth(mainMonitor);
        const void *keys[1] = { kCGDisplayStreamSourceRect };
        const void *values[1] = { CGRectCreateDictionaryRepresentation(CGRectMake(0, 0, 100, 100)) };

        CFDictionaryRef properties = CFDictionaryCreate(NULL, keys, values, 1, NULL, NULL);

        CGDisplayStreamRef stream = CGDisplayStreamCreate(CGMainDisplayID(), monitorWidth, monitorHeight, '420f' , properties,  ^(CGDisplayStreamFrameStatus status, uint64_t displayTime, IOSurfaceRef frameSurface, CGDisplayStreamUpdateRef updateRef){});

        CGDirectDisplayID displayID = CGMainDisplayID();
        CGImageRef image_create = CGDisplayCreateImage(displayID);

        CFRunLoopSourceRef runLoop = CGDisplayStreamGetRunLoopSource(stream);

    // CFRunLoopAddSource(<#CFRunLoopRef rl#>, runLoop, <#CFRunLoopMode mode#>);

        CGError err = CGDisplayStreamStart(stream);
        if (err == CGDisplayNoErr)
            sleep(5);
        else
            std::cout<<"Error: "<<err<<std::endl;
    } */
#endif // ---- END OSX ONLY ---- //

};
