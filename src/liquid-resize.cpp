#include "liquid-resize.hpp"
#include <OpenImageIO/imageio.h>
#include <iostream>
#include <stdexcept>
#include <cmath>

using namespace OIIO;

// maybe define a macro for getting r/g/b of a pixel
// or even a flexible class?

// Load image spec and pixels into memory
LiquidResize::LiquidResize(const std::string& path) {
    auto inp = ImageInput::open(path);
    if (not inp) {
        auto err = "Failed to open image: " + path + "\n";
        // should this be a runtime error or should we close nicely?
        throw std::runtime_error(err);
    }
    auto const& spec = inp->spec();
    height_ = spec.height;
    width_ = spec.width;
    nchannels_ = spec.nchannels;
    if (nchannels_ < 3) {
        auto err = "image must be RBG, no support for grayscale (yet)";
        // should this be a runtime error or should we close nicely?
        throw std::runtime_error(err);
    }
    std::cout << "w = " << spec.width << " h = " << spec.height << " channels = " << spec.nchannels << "\n";
    pixel_data_ = std::vector<unsigned char>(spec.width * spec.height * spec.nchannels);
    inp->read_image(TypeDesc::UINT8, &pixel_data_[0]);
    inp->close();
}

// calculate number of vertical seams to remove and loop over findHorizontalSeam() and removeHorizontalSeam()
void LiquidResize::do_horizontal_resize(int perc) {

}

/* calculate the energy of a pixel using the dual-gradient method
- maybe cache these results (and test if cache {ie reading} is faster than a few math/branch operations)
- define x to be along the width, y to be along the height
- it would be nice to work in x,y coords for the find function, just means i do the conversion here from
 coords -> pixel_data_ index here */
int LiquidResize::energy(int x, int y) const {
    // (b + (a%b)) % b for algebraic mod (a mod b)
    // holy moly test this asap, also use auto you ape
    // REMEMBER to multiply by nchannels_ when accessing the data vector
    int aboveIdx = ((height_ + ((y - 1) % height_)) % height_) * (height_ - 1) + x;
    int belowIdx = ((height_ + ((y + 1) % height_)) % height_) * (height_ - 1) + x;
    int leftIdx = (y * (height_ - 1)) + (width_ + ((x - 1) % width_)) % width_;
    int rightIdx = (y * (height_ - 1)) + (width_ + ((x + 1) % width_)) % width_;

    int sum = 0;
    for (int i = 0; i < 3; ++i) {
        sum += std::pow(abs(pixel_data_[leftIdx*nchannels_ + i] - pixel_data_[rightIdx*nchannels_ + i]), 2);
    }
    for (int i = 0; i < 3; ++i) {
        sum += std::pow(abs(pixel_data_[belowIdx*nchannels_ + i] - pixel_data_[aboveIdx*nchannels_ + i]), 2);
    }
    return sum;
}

