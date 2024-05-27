# Beehive (WIP)
A screen recording tool that can save to disk (MP4 or MPTS) or stream to a server (RTP).

## Dependencies
`ffmpeg` is the only dependency. It needs to be installed and should be in PATH (beehive will run commands like `ffmpeg ...`).

## Usage
Type `make release` to build the executable, then type `./beehive_release` to run it. Type `q` or `quit` into stdin to quit.

## Output Formats
with my minimal testing, i've found that mpeg4 has lower overhead, but with much worse quality output (color, bitrate, etc). mpegts (using h264) has better quality with more overhead.

## Known Issues
- On Windows, other windows can overlay streaming one
- 2 window instances for fullscreen VSCode on OSX
- Deprecated functions (GetProcessForPID, SetFrontProcess, CGWindowListCreateImage) on OSX

## Performance stuff to test
- producer and consumer threads for creating images and piping them
- stream with RTSP instead of RTP?
- CGDisplayStream vs individual CGWindowListCreateImage calls
- kCGWindowImageNominalResolution vs kCGWindowImageBestResolution
