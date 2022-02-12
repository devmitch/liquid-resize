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

// returns pixel_data_ index given wrapped coords (ie if coordinates are negative go backwards from the other side)
int LiquidResize::getIndexFromWrappedCoords(int x, int y) const {
    // (b + (a%b)) % b for algebraic mod (a mod b), ie -2 mod 7 = 5 (% operator doesn't do this)
    return ((height_ + ((y) % height_)) % height_) * (height_ - 1) + (width_ + ((x) % width_)) % width_;
}


/* calculate the energy of a pixel using the dual-gradient method
- maybe cache these results (and test if cache {ie reading} is faster than a few math/branch operations)
- define x to be along the width, y to be along the height
- it would be nice to work in x,y coords for the find function, just means i do the conversion here from
 coords -> pixel_data_ index here */
int LiquidResize::energy(int x, int y) const {
    auto aboveIdx = getIndexFromWrappedCoords(x, y - 1);
    auto belowIdx = getIndexFromWrappedCoords(x, y + 1);
    auto leftIdx = getIndexFromWrappedCoords(x - 1, y);
    auto rightIdx = getIndexFromWrappedCoords(x + 1, y);

    auto sum = 0;
    for (int i = 0; i < 3; ++i) {
        sum += std::pow(abs(pixel_data_[leftIdx*nchannels_ + i] - pixel_data_[rightIdx*nchannels_ + i]), 2);
    }
    for (int i = 0; i < 3; ++i) {
        sum += std::pow(abs(pixel_data_[belowIdx*nchannels_ + i] - pixel_data_[aboveIdx*nchannels_ + i]), 2);
    }
    return sum;
}

// Dynamic programming (bottom-up) approach to find minimum total energy path from top to bottom of image
// returns the x coordinates of the pixels on each row to be remove
std::vector<int> LiquidResize::findHorizontalSeam() const {
    std::cout << "start of find horizontal seam\n";
    // build cumulative energy matrix through DP approach
    auto cumulative_energy = std::vector<int>(width_ * height_);
    for (int x = 0; x < width_; ++x) {
        for (int y = 0; y < height_; ++y) {
            auto idx = getIndexFromWrappedCoords(x, y);
            if (y == 0) {
                cumulative_energy[idx] = energy(x, y); // dealing with indexes and coords again is sus
            } else {
                auto min = energy(x, y-1);
                if (x > 0) { // if there is a pixel available above and to the left
                    min = std::min(min, energy(x-1, y-1));
                }
                if (x < width_ - 1) { // if there is a pixel available above and to the right
                    min = std::min(min, energy(x+1, y-1));
                }
                cumulative_energy[idx] = energy(x, y) + min;
            }
        }
    }

    // find end of shortest path in matrix (when multiple exist, we pick the first one from the left)
    auto minPathEndX = 0;
    auto minPathSum = std::numeric_limits<int>::max();
    for (int x = 0; x < width_; ++x) {
        auto idx = (height_ - 1) * width_ + x; // maybe a function for this? if we need idx from coords in future
        if (cumulative_energy[idx] < minPathSum) {
            minPathSum = cumulative_energy[idx];
            minPathEndX = x;
        }
    }

    // now traverse back up this path and record the x coordinates of each pixel
    auto minPathXCoords = std::vector<int>(height_);
    minPathXCoords[height_ - 1] = minPathEndX;
    for (int y = height_ - 2; y >= 0; --y) {
        auto prevX = minPathXCoords[y+1];
        auto pathXCoordToAdd = prevX;
        auto min = cumulative_energy[getIndexFromWrappedCoords(prevX, y)];
        if (prevX > 0 and cumulative_energy[getIndexFromWrappedCoords(prevX - 1, y)] < min) {
            pathXCoordToAdd = prevX - 1;
            min = cumulative_energy[getIndexFromWrappedCoords(prevX - 1, y)]; // i can do better than doing this twice
        }
        if (prevX < width_ - 1 and cumulative_energy[getIndexFromWrappedCoords(prevX + 1, y)] < min) {
            pathXCoordToAdd = prevX + 1;
            //min = cumulative_energy[getIndexFromWrappedCoords(prevX + 1, y)];
        }
        minPathXCoords[y] = pathXCoordToAdd;
    }
    std::cout << "end of find horizontal seam\n";
    return minPathXCoords;
}

void LiquidResize::removeHorizontalSeam(const std::vector<int> &seam) {
    std::cout << "start of remove horizontal seam\n";
    // 1 less width since we are removing a horizontal seam
    auto new_pixel_data = std::vector<unsigned char>((width_ - 1) * height_ * nchannels_);
    for (int x = 0; x < width_; ++x) {
        for (int y = 0; y < height_; ++y) {
            if (seam[y] == x) continue;
            auto old_idx = getIndexFromWrappedCoords(x, y);
            new_pixel_data.push_back(pixel_data_[old_idx*nchannels_ + 0]);
            new_pixel_data.push_back(pixel_data_[old_idx*nchannels_ + 1]);
            new_pixel_data.push_back(pixel_data_[old_idx*nchannels_ + 2]);
        }
    }
    width_ = width_ - 1;
    pixel_data_ = new_pixel_data; // i think this is better? since no copy
    std::cout << "end of remove horizontal seam\n";
}

void LiquidResize::writeImage() {
    auto filename = "output.jpg";
    auto outp = ImageOutput::create(filename);
    if (not outp) {
        auto err = "Failed to create output image\n";
        throw std::runtime_error(err);
    }
    auto spec = ImageSpec(width_, height_, nchannels_, TypeDesc::UINT8);
    outp->open(filename, spec);
    outp->write_image(TypeDesc::UINT8, pixel_data_.data());
    outp->close ();
}

// main loop over findHorizontalSeam() and removeHorizontalSeam()
void LiquidResize::do_horizontal_resize(int perc) {
    // the loops in the above functions probably fuck up if we try to remove too many seams idk
    if (perc > 100 or perc < 10) {
        auto err = "Error: resize percentage must be in range [0, 90]\n";
        throw std::runtime_error(err);
    }
    // 1 - perc = seams_to_remove*100/width_
    auto seams_to_remove = ((100-perc)*width_) / 100;
    std::cout << "seams to remove: " << seams_to_remove << "\n";
    for (int i = 0; i < seams_to_remove; ++i) {
        std::cout << "starting removal of seam number " << i << "\n";
        auto const& seam = findHorizontalSeam();
        removeHorizontalSeam(seam);
    }
    writeImage();
}
