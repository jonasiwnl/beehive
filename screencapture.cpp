#include <iostream>
#include <string>
#include <vector>

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>
#endif

#define OUTPUT_PATH "output/output.jpg"

using std::cout, std::cin, std::cerr;
using std::string, std::vector;


struct ScreenCapture {
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

#ifdef __APPLE__ /* OSX only code. */
    CGWindowID selected_window_id;

    /*
     * This function is just a placeholder for streaming images.
     */
    bool osx_save_image()
    {
        CGImageRef img = CGWindowListCreateImage(
            CGRectNull, /* CGRectNull means capture minimum size needed in order to capture window */
            kCGWindowListExcludeDesktopElements | kCGWindowListOptionIncludingWindow,
            selected_window_id,
            kCGWindowImageBestResolution | kCGWindowImageBoundsIgnoreFraming
        );

        /* Now, we set up disk IO. */
        CFStringRef output_path =
            CFStringCreateWithCString(kCFAllocatorDefault, OUTPUT_PATH, kCFStringEncodingUTF8);
        CFURLRef url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, output_path, kCFURLPOSIXPathStyle, false);
        CGImageDestinationRef dest = CGImageDestinationCreateWithURL(url, kUTTypeJPEG, 1, NULL);

        /* The NULL is addl properties, might be good to look into */
        CGImageDestinationAddImage(dest, img, NULL);
        bool write_success = CGImageDestinationFinalize(dest); /* Write to disk. */

        /* Clean up memory */
        CFRelease(dest);
        CGImageRelease(img);

        if (!write_success) {
            cerr << "ERROR: couldn't create or write image.\n";
            return false;
        }

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

        /* Window IDs that can actually be streamed from */
        vector<CFNumberRef> active_window_ids;

        if (window_infolist_ref == NULL) {
            cerr << "ERROR: couldn't get window list.\n";
            return false;
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

        /* Clean up memory */
        CFRelease(window_infolist_ref);

        /* Get ID of selected window. */
        size_t selected_window_idx = user_select_window_idx(active_window_ids.size());
        CFNumberRef window_id_ref = active_window_ids[selected_window_idx];
        CFNumberGetValue(window_id_ref, kCGWindowIDCFNumberType, &selected_window_id);

        return true;
    }
#endif /* __APPLE__ */

};
