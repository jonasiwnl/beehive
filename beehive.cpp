#include <iostream>
#include <string>

#ifdef __APPLE__
#include <CoreGraphics/CoreGraphics.h>
#endif

using std::cout, std::string;


int main()
{
    /* TODO 1. Select a window to stream from */

#ifdef __APPLE__ /* OSX only code. */
    CFArrayRef window_infolist_ref = CGWindowListCopyWindowInfo(
        kCGWindowListOptionOnScreenOnly + kCGWindowListExcludeDesktopElements + kCGWindowListOptionIncludingWindow, kCGNullWindowID);
    CFIndex window_count = CFArrayGetCount(window_infolist_ref);

    if (window_infolist_ref == NULL) {
        cout << "ERROR: couldn't get window list.\n";
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
        cout << CFStringGetCStringPtr(window_name_ref, kCFStringEncodingUTF8) << '\n';
    }

    CFRelease(window_infolist_ref);
#endif /* __APPLE__ */

    /* TODO 2. Expose a port */

    /* TODO 3. Listen for connections and accept them (as well as for interrupt) */

    /* TODO 4. Stream window to connections */

    return 0;
}
