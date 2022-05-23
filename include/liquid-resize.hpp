#ifndef LIQUID_RESIZE
#define LIQUID_RESIZE

#include <string>
#include <vector>

class LiquidResize {
public:
    explicit LiquidResize(std::string const& path);
    void resizeImage(int new_perc);
    void energyImage();
private:
    int height_;
    int width_;
    int nchannels_;
    std::vector<unsigned char> pixel_data_;
    void removeHorizontalSeam();
    void writeImage();
    int energy(int x, int y) const;
    int getIndex(int x, int y) const;
};

#endif /* LIQUID_RESIZE */
