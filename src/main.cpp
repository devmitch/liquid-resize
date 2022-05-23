#include <iostream>
#include <cmath>

#include "liquid-resize.hpp"



int main(int argc, char *argv[]) {
    if (argc <= 1) {
        std::cout << "Usage: " << argv[0] << " [option] [scale-percentage] <filename>\n";
        std::cout << "Use << --help for more help and options\n";
    } else if (argc == 2) {
        auto arg2 = std::string(argv[1]);
        if (arg2 == "--help") {
            std::cout << "Usage: " << argv[0] << " [options] <filename>\n";
            std::cout << "Current options include:\n";
            std::cout << "\t-e : produces a visual representation of the pixel energies\n";
            std::cout << "\t-s : produces a visual representation of the lowest energy seam\n";
            std::cout << "\t[0, 100] : a number that determines the new scale as a percentage of the original image. By default it is 50.\n";
            std::cout << "Output will be at the file output.jpg in the same directory as the program itself. \n";
        } else {
            auto resizer = LiquidResize(arg2);
            resizer.resizeImage(50);
        }
    } else if (argc == 3) {
        auto arg2 = std::string(argv[1]);
        auto arg3 = std::string(argv[2]);
        auto resizer = LiquidResize(arg3);
        if (arg2 == "-e") {
            resizer.energyImage();
        } else if (arg2 == "-s") {
            resizer.showHorizontalSeam();
        } else {
            try {
                int new_perc = std::stoi(arg2);
                resizer.resizeImage(new_perc);
            } catch(...) {
                std::cout << "scale-percentage must be a number between 0 and 100 inclusive.\n";
            }
        }
    } else {
        std::cout << "Usage: " << argv[0] << " [option] [scale-percentage] <filename>\n";
        std::cout << "Use << --help for more help and options\n";
    }

    return 0;
}