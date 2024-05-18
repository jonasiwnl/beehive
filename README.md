# Beehive (WIP)
A screen recording tool with the capability to save to disk (MP4 or MPTS) or stream to a server (RTMP).

## Dependencies
`ffmpeg` is the only dependency. It needs to be installed and should be in PATH (beehive will run commands like `ffmpeg ...`).

## Usage
Type `make release` to build the executable, then type `./beehive_release` to run it.

## Output Formats
with my minimal testing, i've found that mpeg4 generally has lower overhead, but with worse quality output (color, bitrate, etc). mpegts (using h264) has better quality with more overhead.

## Known Issues
- 2 window instances for fullscreen VSCode
- Sometimes get bad packets error when quitting ffmpeg

## Performance stuff to test
- CGDisplayStream vs individual CGWindowListCreateImage calls
- kCGWindowImageNominalResolution vs kCGWindowImageBestResolution
