# Beehive (WIP)
A screen recording tool for macOS (hopefully for windows soon), with the capability to save to disk or stream to a server.

## Usage
Type `make release` to build the executable, then type `./beehive_release` to run it.

## Known Issues
- 2 window instances for fullscreen VSCode
- Deprecation issues with CGWindowListCreateImage and kUTTypeJPEG

## Performance stuff to test
- CGDisplayStream vs individual CGWindowListCreateImage calls
- kCGWindowImageNominalResolution vs kCGWindowImageBestResolution
