#ifndef LIQUID_RESIZE
#define LIQUID_RESIZE

#include <string>
#include <vector>

class LiquidResize {
public:
    explicit LiquidResize(std::string const& path);
    void do_horizontal_resize(int perc);
private:
    int height_;
    int width_;
    int nchannels_;
    std::vector<unsigned char> pixel_data_;
    void writeImage();
    void removeHorizontalSeam(const std::vector<int>& seam);
    std::vector<int> findHorizontalSeam() const;
    int energy(int x, int y) const;
    int getIndexFromWrappedCoords(int x, int y) const;
};

#endif /* LIQUID_RESIZE */
