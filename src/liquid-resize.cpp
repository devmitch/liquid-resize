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
    //std::cout << "w = " << spec.width << " h = " << spec.height << " channels = " << spec.nchannels << "\n";
    pixel_data_ = std::vector<unsigned char>(spec.width * spec.height * spec.nchannels);
    inp->read_image(TypeDesc::UINT8, &pixel_data_[0]);
    inp->close();
}

// (0, 0) on top left to (width-1, height-1) bottom right
int LiquidResize::getIndex(int x, int y) const {
    return y * width_ + x;
}

int LiquidResize::energy(int x, int y) const {
    // use the pixel itself rather than neighbour if on edge
    auto aboveIdx = y == 0 ? getIndex(x, y) : getIndex(x, y - 1);
    auto belowIdx = y == height_ - 1 ? getIndex(x, y) : getIndex(x, y + 1);
    auto leftIdx = x == 0 ? getIndex(x, y) : getIndex(x - 1, y);
    auto rightIdx = x == width_ - 1 ? getIndex(x, y) : getIndex(x + 1, y);

    auto sum = 0;
    for (int i = 0; i < 3; ++i) {
        sum += std::pow(abs(pixel_data_[leftIdx*nchannels_ + i] - pixel_data_[rightIdx*nchannels_ + i]), 2);
    }
    for (int i = 0; i < 3; ++i) {
        sum += std::pow(abs(pixel_data_[belowIdx*nchannels_ + i] - pixel_data_[aboveIdx*nchannels_ + i]), 2);
    }
    return sum;
}

void LiquidResize::energyImage() {
    auto new_pixel_data = std::vector<unsigned char>();
    auto max_energy = 0;
    for (auto x = 0; x < width_; x++) {
        for (auto y = 0; y < height_; y++) {
            if (energy(x, y) > max_energy) {
                max_energy = energy(x, y);
            }
        }
    }
    for (auto y = 0; y < height_; y++) {
        for (auto x = 0; x < width_; x++) {
            auto energy_ratio = ((float)energy(x, y) / (float)max_energy) * 255;
            new_pixel_data.push_back(energy_ratio);
            new_pixel_data.push_back(energy_ratio);
            new_pixel_data.push_back(energy_ratio);
        }
    }
    pixel_data_ = std::move(new_pixel_data);
    writeImage();
}

// new_perc between 0 and 100 - 40 means 40% of the size of the original image
void LiquidResize::resizeImage(int new_perc) {
    int seams_to_remove = ((100-new_perc)*width_)/100;
    for (auto i = 0; i < seams_to_remove; i++) {
        std::cout << "Progress: " << i*100/seams_to_remove << "%\r";
        std::cout.flush();
        removeHorizontalSeam();
    }
    writeImage();
}

std::vector<int> LiquidResize::getHorizontalSeam() const {
    auto rows = std::vector<std::vector<int>>();
    auto curr_row = 0;
    auto first_row = std::vector<int>(width_);
    for (auto x = 0; x < width_; x++) {
        first_row[x] = energy(x, curr_row);
    }
    rows.push_back(first_row);
    curr_row++;

    while (curr_row < height_) {
        auto new_row = std::vector<int>(width_);
        for (auto x = 0; x < width_; x++) {
            auto min_seam_energy = std::numeric_limits<int>::max();
            if (x != 0) {
                min_seam_energy = std::min(min_seam_energy, rows[curr_row-1][x-1]);
            }
            if (x != width_ - 1) {
                min_seam_energy = std::min(min_seam_energy, rows[curr_row-1][x+1]);
            }
            min_seam_energy = std::min(min_seam_energy, rows[curr_row-1][x]);
            new_row[x] = min_seam_energy + energy(x, curr_row);
        }
        rows.push_back(new_row);
        curr_row++;
    }

    auto indices_to_remove = std::vector<int>(height_);
    auto seam_end_idx = (int)std::distance(rows[curr_row-1].begin(), std::min_element(rows[curr_row-1].begin(), rows[curr_row-1].end()));
    indices_to_remove[height_-1] = seam_end_idx;
    for (auto row = height_ - 2; row >= 0; row--) {
        auto new_seam_end_idx = 0;
        auto min_seam_energy = std::numeric_limits<int>::max();
        if (seam_end_idx != 0 and rows[row][seam_end_idx-1] < min_seam_energy) {
            new_seam_end_idx = seam_end_idx-1;
            min_seam_energy = rows[row][seam_end_idx-1];
        }
        if (seam_end_idx != width_ - 1 and rows[row][seam_end_idx+1] < min_seam_energy) {
            new_seam_end_idx = seam_end_idx+1;
            min_seam_energy = rows[row][seam_end_idx+1];
        }
        if (rows[row][seam_end_idx] < min_seam_energy) {
            new_seam_end_idx = seam_end_idx;
        }
        indices_to_remove[row] = new_seam_end_idx;
        seam_end_idx = new_seam_end_idx;
    }

    return indices_to_remove;
}

void LiquidResize::removeHorizontalSeam() {
    auto const& indices_to_remove = getHorizontalSeam();
    auto new_pixel_data = std::vector<unsigned char>();
    for (auto y = 0; y < height_; y++) {
        for (auto x = 0; x < width_; x++) {
            if (indices_to_remove[y] == x) {
//                new_pixel_data.push_back(255);
//                new_pixel_data.push_back(0);
//                new_pixel_data.push_back(0);
                continue;
            } else {
                new_pixel_data.push_back(pixel_data_[getIndex(x, y) * nchannels_]);
                new_pixel_data.push_back(pixel_data_[getIndex(x, y) * nchannels_ + 1]);
                new_pixel_data.push_back(pixel_data_[getIndex(x, y) * nchannels_ + 2]);
            }
        }
    }
    pixel_data_ = std::move(new_pixel_data);
    width_--;
}

void LiquidResize::showHorizontalSeam() {
    auto const& indices_to_remove = getHorizontalSeam();
    auto new_pixel_data = std::vector<unsigned char>();
    for (auto y = 0; y < height_; y++) {
        for (auto x = 0; x < width_; x++) {
            if (indices_to_remove[y] == x) {
                new_pixel_data.push_back(255);
                new_pixel_data.push_back(0);
                new_pixel_data.push_back(0);
            } else {
                new_pixel_data.push_back(pixel_data_[getIndex(x, y) * nchannels_]);
                new_pixel_data.push_back(pixel_data_[getIndex(x, y) * nchannels_ + 1]);
                new_pixel_data.push_back(pixel_data_[getIndex(x, y) * nchannels_ + 2]);
            }
        }
    }
    pixel_data_ = std::move(new_pixel_data);
    writeImage();
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
    outp->close();
    std::cout << "Image written to output.jpg.\n";
}

