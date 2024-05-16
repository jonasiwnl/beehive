# Beehive (WIP)
A screen recording tool for macOS (hopefully for windows soon), with the capability to save to disk or stream to a server.

## Dependencies
`ffmpeg` is the only dependency. It needs to be installed and should be in PATH (beehive will run commands like `ffmpeg ...`.

## Usage
Type `make release` to build the executable, then type `./beehive_release` to run it.

## Output Formats
mpeg4 generally has lower overhead, but with worse quality output (color, bitrate, etc). mpegts (using h264) has better quality with more overhead.

## Known Issues
- output video is at a different speed than recording
- 2 window instances for fullscreen VSCode
- Sometimes get bad packets error when quitting ffmpeg

## Performance stuff to test
- CGDisplayStream vs individual CGWindowListCreateImage calls
- kCGWindowImageNominalResolution vs kCGWindowImageBestResolution
