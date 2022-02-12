#include <iostream>
#include <cmath>

#include "liquid-resize.hpp"



int main() {
    int width_ = 3;
    int height_ = 4;
    int nchannels_ = 3;
    auto pixel_data_ = std::vector<unsigned char>(width_ * height_ * nchannels_);
    for (int i = 0; i < pixel_data_.size(); ++i) {
        pixel_data_[i] = i;
    }

    int x = 2;
    int y = 3;

    auto testfn = ((height_ + ((y) % height_)) % height_) * (height_ - 1) + (width_ + ((x) % width_)) % width_;
    std::cout << "indx at coords " << x << ", " << y << " is " << testfn << "\n";

    int aboveIdx = ((height_ + ((y - 1) % height_)) % height_) * (height_ - 1) + x;
    int belowIdx = ((height_ + ((y + 1) % height_)) % height_) * (height_ - 1) + x;
    int leftIdx = (y * (height_ - 1)) + (width_ + ((x - 1) % width_)) % width_;
    int rightIdx = (y * (height_ - 1)) + (width_ + ((x + 1) % width_)) % width_;

    std::cout << "above: " << aboveIdx << ", below: " << belowIdx << ", left: " << leftIdx << ", right: " << rightIdx << "\n";

    int sum = 0;
    for (int i = 0; i < 3; ++i) {
        sum += std::pow(abs(pixel_data_[leftIdx*nchannels_ + i] - pixel_data_[rightIdx*nchannels_ + i]), 2);
    }
    for (int i = 0; i < 3; ++i) {
        sum += std::pow(abs(pixel_data_[belowIdx*nchannels_ + i] - pixel_data_[aboveIdx*nchannels_ + i]), 2);
    }
    return 0;
}