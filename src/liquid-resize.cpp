#include "liquid-resize.hpp"
#include <OpenImageIO/imageio.h>
#include <iostream>
#include <stdexcept>

using namespace OIIO;

LiquidResize::LiquidResize(std::string const& path) {
    auto inp = ImageInput::open(path);
    if (not inp) {
        auto err = "Failed to open image: " + path + "\n";
        throw std::runtime_error(err);
    }
    auto const& spec = inp->spec();
    std::cout << "w = " << spec.width << " h = " << spec.height << " channels = " << spec.nchannels << "\n";
    auto pixels = std::vector<unsigned char>(spec.width * spec.height * spec.nchannels);
    inp->read_image(TypeDesc::UINT8, &pixels[0]);
    inp->close();
}