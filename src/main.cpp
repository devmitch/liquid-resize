#include <iostream>
#include <cmath>

#include "liquid-resize.hpp"



int main(int argc, char *argv[]) {
    if (argc <= 1) {
        std::cout << "Usage: " << argv[0] << " [options] <filename>\n";
        std::cout << "Use --help for help\n";
    } else if (argc == 2) {
        auto arg2 = std::string(argv[1]);
        if (arg2 == "--help") {

        } else {
            auto resizer = LiquidResize(arg2);
            resizer.resizeImage(50);
        }

    } else if (argc >= 3) {
        auto arg2 = std::string(argv[1]);
        auto arg3 = std::string(argv[2]);
        auto resizer = LiquidResize(arg3);
        if (arg2 == "-e") {
            resizer.energyImage();
        } else {

        }
    }

    return 0;
}