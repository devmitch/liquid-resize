#include "liquid-resize.hpp"
#include <OpenImageIO/imageio.h>
#include <iostream>

using namespace OIIO;

LiquidResize::LiquidResize(std::string const& path) {
    auto inp = ImageInput::open(path);
    if (! inp)
        return;
    const ImageSpec &spec = inp->spec();
    int xres = spec.width;
    int yres = spec.height;
    int channels = spec.nchannels;
    std::cout << "w = " << xres << " h = " << yres << " channels = " << channels << "\n";
    std::vector<unsigned char> pixels(xres * yres * channels);
    inp->read_image(TypeDesc::UINT8, &pixels[0]);
    inp->close();
}