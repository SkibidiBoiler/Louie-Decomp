#pragma once
// Flash stage parameters. The real values come from the SWF header (tools/swf_info.py);
// these are placeholders until the owner's copy has been inspected.

namespace engine {

struct Stage {
    int width = 640;      // SKELETON: replace with the SWF frame size (twips / 20)
    int height = 480;     // SKELETON: replace with the SWF frame size (twips / 20)
    double frameRate = 30.0; // SKELETON: replace with the SWF frame rate
};

} // namespace engine
