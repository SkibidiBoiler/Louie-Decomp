#pragma once
// Flash stage parameters, from the SWF headers (tools/swf_info.py).
// Defaults are Papa Louie 2 / 3 (700x416 @ 30 fps); Papa Louie 1 uses 550x400 @ 24 fps.

namespace engine {

struct Stage {
    int width = 700;
    int height = 416;
    double frameRate = 30.0;
};

} // namespace engine
